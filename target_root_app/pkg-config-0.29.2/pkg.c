/* 
 * Copyright (C) 2001, 2002 Red Hat Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pkg.h"
#include "parse.h"
#include "rpmvercmp.h"

#ifdef HAVE_MALLOC_H
# include <malloc.h>
#endif

#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <ctype.h>

static void verify_package (Package *pkg);

static GHashTable *packages = NULL;
static GHashTable *globals = NULL;
static GList *search_dirs = NULL;

gboolean disable_uninstalled = FALSE;
gboolean ignore_requires = FALSE;
gboolean ignore_requires_private = TRUE;
gboolean ignore_private_libs = TRUE;

void
add_search_dir (const char *path)
{
  search_dirs = g_list_append (search_dirs, g_strdup (path));
}

void
add_search_dirs (const char *path, const char *separator)
{
      char **search_dirs;
      char **iter;

      search_dirs = g_strsplit (path, separator, -1);
    
      iter = search_dirs;
      while (*iter)
        {
          debug_spew ("Adding directory '%s' from PKG_CONFIG_PATH\n",
                      *iter);
          add_search_dir (*iter);
          
          ++iter;
        }
      
      g_strfreev (search_dirs);
}

#ifdef G_OS_WIN32
/* Guard against .pc file being installed with UPPER CASE name */
# define FOLD(x) tolower(x)
# define FOLDCMP(a, b) g_ascii_strcasecmp (a, b)
#else
# define FOLD(x) (x)
# define FOLDCMP(a, b) strcmp (a, b)
#endif

#define EXT_LEN 3

static gboolean
ends_in_dotpc (const char *str)
{
  int len = strlen (str);
  
  if (len > EXT_LEN &&
      str[len - 3] == '.' &&
      FOLD (str[len - 2]) == 'p' &&
      FOLD (str[len - 1]) == 'c')
    return TRUE;
  else
    return FALSE;
}

/* strlen ("-uninstalled") */
#define UNINSTALLED_LEN 12

gboolean
name_ends_in_uninstalled (const char *str)
{
  int len = strlen (str);
  
  if (len > UNINSTALLED_LEN &&
      FOLDCMP ((str + len - UNINSTALLED_LEN), "-uninstalled") == 0)
    return TRUE;
  else
    return FALSE;
}

static Package *
internal_get_package (const char *name, gboolean warn);

/* Look for .pc files in the given directory and add them into
 * locations, ignoring duplicates
 */
static void
scan_dir (char *dirname)
{
  GDir *dir;
  const gchar *filename;

  int dirnamelen = strlen (dirname);
  /* Use a copy of dirname cause Win32 opendir doesn't like
   * superfluous trailing (back)slashes in the directory name.
   */
  char *dirname_copy = g_strdup (dirname);

  if (dirnamelen > 1 && dirname[dirnamelen-1] == G_DIR_SEPARATOR)
    {
      dirnamelen--;
      dirname_copy[dirnamelen] = '\0';
    }
#ifdef G_OS_WIN32
    {
      gchar *p;
      /* Turn backslashes into slashes or
       * g_shell_parse_argv() will eat them when ${prefix}
       * has been expanded in parse_libs().
       */
      p = dirname;
      while (*p)
        {
          if (*p == '\\')
            *p = '/';
          p++;
        }
    }
#endif
  dir = g_dir_open (dirname_copy, 0 , NULL);
  g_free (dirname_copy);

  if (!dir)
    {
      debug_spew ("Cannot open directory '%s' in package search path: %s\n",
                  dirname, g_strerror (errno));
      return;
    }

  debug_spew ("Scanning directory '%s'\n", dirname);

  while ((filename = g_dir_read_name(dir)))
    {
      char *path = g_build_filename (dirname, filename, NULL);
      internal_get_package (path, FALSE);
      g_free (path);
    }
  g_dir_close (dir);
}

static Package *
add_virtual_pkgconfig_package (void)
{
  Package *pkg = NULL;

  pkg = g_new0 (Package, 1);

  pkg->key = g_strdup ("pkg-config");
  pkg->version = g_strdup (VERSION);
  pkg->name = g_strdup ("pkg-config");
  pkg->description = g_strdup ("pkg-config is a system for managing "
			       "compile/link flags for libraries");
  pkg->url = g_strdup ("http://pkg-config.freedesktop.org/");

  if (pkg->vars == NULL)
    pkg->vars = g_hash_table_new (g_str_hash, g_str_equal);
  g_hash_table_insert (pkg->vars, "pc_path", pkg_config_pc_path);

  debug_spew ("Adding virtual 'pkg-config' package to list of known packages\n");
  g_hash_table_insert (packages, pkg->key, pkg);

  return pkg;
}

void
package_init (gboolean want_list)
{
  if (packages)
    return;
      
  packages = g_hash_table_new (g_str_hash, g_str_equal);

  if (want_list)
    g_list_foreach (search_dirs, (GFunc)scan_dir, NULL);
  else
    /* Should not add virtual pkgconfig package when listing to be
     * compatible with old code that only listed packages from real
     * files */
    add_virtual_pkgconfig_package ();
}

static Package *
internal_get_package (const char *name, gboolean warn)
{
  Package *pkg = NULL;
  char *key = NULL;
  char *location = NULL;
  unsigned int path_position = 0;
  GList *iter;
  GList *dir_iter;
  
  pkg = g_hash_table_lookup (packages, name);

  if (pkg)
    return pkg;

  debug_spew ("Looking for package '%s'\n", name);
  
  /* treat "name" as a filename if it ends in .pc and exists */
  if ( ends_in_dotpc (name) )
    {
      debug_spew ("Considering '%s' to be a filename rather than a package name\n", name);
      location = g_strdup (name);
      key = g_strdup (name);
    }
  else
    {
      /* See if we should auto-prefer the uninstalled version */
      if (!disable_uninstalled &&
          !name_ends_in_uninstalled (name))
        {
          char *un;

          un = g_strconcat (name, "-uninstalled", NULL);

          pkg = internal_get_package (un, FALSE);

          g_free (un);
          
          if (pkg)
            {
              debug_spew ("Preferring uninstalled version of package '%s'\n", name);
              return pkg;
            }
        }
      
      for (dir_iter = search_dirs; dir_iter != NULL;
           dir_iter = g_list_next (dir_iter))
        {
          path_position++;
          location = g_strdup_printf ("%s%c%s.pc", (char*)dir_iter->data,
                                      G_DIR_SEPARATOR, name);
          if (g_file_test (location, G_FILE_TEST_IS_REGULAR))
            break;
          g_free (location);
          location = NULL;
        }

    }
  
  if (location == NULL)
    {
      if (warn)
        verbose_error ("Package %s was not found in the pkg-config search path.\n"
                       "Perhaps you should add the directory containing `%s.pc'\n"
                       "to the PKG_CONFIG_PATH environment variable\n",
                       name, name);

      return NULL;
    }

  if (key == NULL)
    key = g_strdup (name);
  else
    {
      /* need to strip package name out of the filename */
      key = g_path_get_basename (name);
      key[strlen (key) - EXT_LEN] = '\0';
    }

  debug_spew ("Reading '%s' from file '%s'\n", name, location);
  pkg = parse_package_file (key, location, ignore_requires,
                            ignore_private_libs, ignore_requires_private);
  g_free (key);

  if (pkg != NULL && strstr (location, "uninstalled.pc"))
    pkg->uninstalled = TRUE;

  g_free (location);

  if (pkg == NULL)
    {
      debug_spew ("Failed to parse '%s'\n", location);
      return NULL;
    }

  pkg->path_position = path_position;

  debug_spew ("Path position of '%s' is %d\n",
              pkg->key, pkg->path_position);
  
  debug_spew ("Adding '%s' to list of known packages\n", pkg->key);
  g_hash_table_insert (packages, pkg->key, pkg);

  /* pull in Requires packages */
  for (iter = pkg->requires_entries; iter != NULL; iter = g_list_next (iter))
    {
      Package *req;
      RequiredVersion *ver = iter->data;

      debug_spew ("Searching for '%s' requirement '%s'\n",
                  pkg->key, ver->name);
      req = internal_get_package (ver->name, warn);
      if (req == NULL)
        {
          verbose_error ("Package '%s', required by '%s', not found\n",
                         ver->name, pkg->key);
          exit (1);
        }

      if (pkg->required_versions == NULL)
        pkg->required_versions = g_hash_table_new (g_str_hash, g_str_equal);

      g_hash_table_insert (pkg->required_versions, ver->name, ver);
      pkg->requires = g_list_prepend (pkg->requires, req);
    }

  /* pull in Requires.private packages */
  for (iter = pkg->requires_private_entries; iter != NULL;
       iter = g_list_next (iter))
    {
      Package *req;
      RequiredVersion *ver = iter->data;

      debug_spew ("Searching for '%s' private requirement '%s'\n",
                  pkg->key, ver->name);
      req = internal_get_package (ver->name, warn);
      if (req == NULL)
        {
          verbose_error ("Package '%s', required by '%s', not found\n",
			 ver->name, pkg->key);
          exit (1);
        }

      if (pkg->required_versions == NULL)
        pkg->required_versions = g_hash_table_new (g_str_hash, g_str_equal);

      g_hash_table_insert (pkg->required_versions, ver->name, ver);
      pkg->requires_private = g_list_prepend (pkg->requires_private, req);
    }

  /* make requires_private include a copy of the public requires too */
  pkg->requires_private = g_list_concat (g_list_copy (pkg->requires),
                                         pkg->requires_private);

  pkg->requires = g_list_reverse (pkg->requires);
  pkg->requires_private = g_list_reverse (pkg->requires_private);

  verify_package (pkg);

  return pkg;
}

Package *
get_package (const char *name)
{
  return internal_get_package (name, TRUE);
}

Package *
get_package_quiet (const char *name)
{
  return internal_get_package (name, FALSE);
}

/* Strip consecutive duplicate arguments in the flag list. */
static GList *
flag_list_strip_duplicates (GList *list)
{
  GList *tmp;

  /* Start at the 2nd element of the list so we don't have to check for an
   * existing previous element. */
  for (tmp = g_list_next (list); tmp != NULL; tmp = g_list_next (tmp))
    {
      Flag *cur = tmp->data;
      Flag *prev = tmp->prev->data;

      if (cur->type == prev->type && g_strcmp0 (cur->arg, prev->arg) == 0)
        {
          /* Remove the duplicate flag from the list and move to the last
           * element to prepare for the next iteration. */
          GList *dup = tmp;

          debug_spew (" removing duplicate \"%s\"\n", cur->arg);
          tmp = g_list_previous (tmp);
          list = g_list_remove_link (list, dup);
        }
    }

  return list;
}

static char *
flag_list_to_string (GList *list)
{
  GList *tmp;
  GString *str = g_string_new ("");
  char *retval;
  
  tmp = list;
  while (tmp != NULL) {
    Flag *flag = tmp->data;
    char *tmpstr = flag->arg;

    if (pcsysrootdir != NULL && flag->type & (CFLAGS_I | LIBS_L)) {
      /* Handle non-I Cflags like -isystem */
      if (flag->type & CFLAGS_I && strncmp (tmpstr, "-I", 2) != 0) {
        char *space = strchr (tmpstr, ' ');

        /* Ensure this has a separate arg */
        g_assert (space != NULL && space[1] != '\0');
        g_string_append_len (str, tmpstr, space - tmpstr + 1);
        g_string_append (str, pcsysrootdir);
        g_string_append (str, space + 1);
      } else {
        g_string_append_c (str, '-');
        g_string_append_c (str, tmpstr[1]);
        g_string_append (str, pcsysrootdir);
        g_string_append (str, tmpstr+2);
      }
    } else {
      g_string_append (str, tmpstr);
    }
    g_string_append_c (str, ' ');
    tmp = g_list_next (tmp);
  }

  retval = str->str;
  g_string_free (str, FALSE);

  return retval;
}

static int
pathposcmp (gconstpointer a, gconstpointer b)
{
  const Package *pa = a;
  const Package *pb = b;
  
  if (pa->path_position < pb->path_position)
    return -1;
  else if (pa->path_position > pb->path_position)
    return 1;
  else
    return 0;
}

static void
spew_package_list (const char *name,
                   GList     *list)
{
  GList *tmp;

  debug_spew (" %s:", name);

  tmp = list;
  while (tmp != NULL)
    {
      Package *pkg = tmp->data;
      debug_spew (" %s", pkg->key);
      tmp = tmp->next;
    }
  debug_spew ("\n");
}


static GList *
packages_sort_by_path_position (GList *list)
{
  return g_list_sort (list, pathposcmp);
}

/* Construct a topological sort of all required packages.
 *
 * This is a depth first search starting from the right.  The output 'listp' is
 * in reverse order, with the first node reached in the depth first search at
 * the end of the list.  Previously visited nodes are skipped.  The result is
 * a list of packages such that each packages is listed once and comes before
 * any package that it depends on.
 */
static void
recursive_fill_list (Package *pkg, gboolean include_private,
                     GHashTable *visited, GList **listp)
{
  GList *tmp;

  /*
   * If the package has already been visited, then it is already in 'listp' and
   * we can skip it. Additionally, this allows circular requires loops to be
   * broken.
   */
  if (g_hash_table_lookup_extended (visited, pkg->key, NULL, NULL))
    {
      debug_spew ("Package %s already in requires chain, skipping\n",
                  pkg->key);
      return;
    }
  /* record this package in the dependency chain */
  else
    {
      g_hash_table_replace (visited, pkg->key, pkg->key);
    }

  /* Start from the end of the required package list to maintain order since
   * the recursive list is built by prepending. */
  tmp = include_private ? pkg->requires_private : pkg->requires;
  for (tmp = g_list_last (tmp); tmp != NULL; tmp = g_list_previous (tmp))
    recursive_fill_list (tmp->data, include_private, visited, listp);

  *listp = g_list_prepend (*listp, pkg);
}

/* merge the flags from the individual packages */
static GList *
merge_flag_lists (GList *packages, FlagType type)
{
  GList *last = NULL;
  GList *merged = NULL;

  /* keep track of the last element to avoid traversing the whole list */
  for (; packages != NULL; packages = g_list_next (packages))
    {
      Package *pkg = packages->data;
      GList *flags = (type & LIBS_ANY) ? pkg->libs : pkg->cflags;

      /* manually copy the elements so we can keep track of the end */
      for (; flags != NULL; flags = g_list_next (flags))
        {
          Flag *flag = flags->data;

          if (flag->type & type)
            {
              if (last == NULL)
                {
                  merged = g_list_prepend (NULL, flags->data);
                  last = merged;
                }
              else
                last = g_list_next (g_list_append (last, flags->data));
            }
        }
    }

  return merged;
}

static GList *
fill_list (GList *packages, FlagType type,
           gboolean in_path_order, gboolean include_private)
{
  GList *tmp;
  GList *expanded = NULL;
  GList *flags;
  GHashTable *visited;

  /* Start from the end of the requested package list to maintain order since
   * the recursive list is built by prepending. */
  visited = g_hash_table_new (g_str_hash, g_str_equal);
  for (tmp = g_list_last (packages); tmp != NULL; tmp = g_list_previous (tmp))
    recursive_fill_list (tmp->data, include_private, visited, &expanded);
  g_hash_table_destroy (visited);
  spew_package_list ("post-recurse", expanded);

  if (in_path_order)
    {
      spew_package_list ("original", expanded);
      expanded = packages_sort_by_path_position (expanded);
      spew_package_list ("  sorted", expanded);
    }

  flags = merge_flag_lists (expanded, type);
  g_list_free (expanded);

  return flags;
}

static GList *
add_env_variable_to_list (GList *list, const gchar *env)
{
  gchar **values;
  gint i;

  values = g_strsplit (env, G_SEARCHPATH_SEPARATOR_S, 0);
  for (i = 0; values[i] != NULL; i++)
    {
      list = g_list_append (list, g_strdup (values[i]));
    }
  g_strfreev (values);

  return list;
}

/* Well known compiler include path environment variables. These are
 * used to find additional system include paths to remove. See
 * https://gcc.gnu.org/onlinedocs/gcc/Environment-Variables.html. */
static const gchar *gcc_include_envvars[] = {
  "CPATH",
  "C_INCLUDE_PATH",
  "CPP_INCLUDE_PATH",
  NULL
};

#ifdef G_OS_WIN32
/* MSVC include path environment variables. See
 * https://msdn.microsoft.com/en-us/library/73f9s62w.aspx. */
static const gchar *msvc_include_envvars[] = {
  "INCLUDE",
  NULL
};
#endif

static void
verify_package (Package *pkg)
{
  GList *requires = NULL;
  GList *conflicts = NULL;
  GList *system_directories = NULL;
  GList *iter;
  GList *requires_iter;
  GList *conflicts_iter;
  GList *system_dir_iter = NULL;
  GHashTable *visited;
  int count;
  const gchar *search_path;
  const gchar **include_envvars;
  const gchar **var;

  /* Be sure we have the required fields */

  if (pkg->key == NULL)
    {
      fprintf (stderr,
               "Internal pkg-config error, package with no key, please file a bug report\n");
      exit (1);
    }
  
  if (pkg->name == NULL)
    {
      verbose_error ("Package '%s' has no Name: field\n",
                     pkg->key);
      exit (1);
    }

  if (pkg->version == NULL)
    {
      verbose_error ("Package '%s' has no Version: field\n",
                     pkg->key);
      exit (1);
    }

  if (pkg->description == NULL)
    {
      verbose_error ("Package '%s' has no Description: field\n",
                     pkg->key);
      exit (1);
    }
  
  /* Make sure we have the right version for all requirements */

  iter = pkg->requires_private;

  while (iter != NULL)
    {
      Package *req = iter->data;
      RequiredVersion *ver = NULL;

      if (pkg->required_versions)
        ver = g_hash_table_lookup (pkg->required_versions,
                                   req->key);

      if (ver)
        {
          if (!version_test (ver->comparison, req->version, ver->version))
            {
              verbose_error ("Package '%s' requires '%s %s %s' but version of %s is %s\n",
                             pkg->key, req->key,
                             comparison_to_str (ver->comparison),
                             ver->version,
                             req->key,
                             req->version);
              if (req->url)
                verbose_error ("You may find new versions of %s at %s\n",
                               req->name, req->url);

              exit (1);
            }
        }
                                   
      iter = g_list_next (iter);
    }

  /* Make sure we didn't drag in any conflicts via Requires
   * (inefficient algorithm, who cares)
   */
  visited = g_hash_table_new (g_str_hash, g_str_equal);
  recursive_fill_list (pkg, TRUE, visited, &requires);
  g_hash_table_destroy (visited);
  conflicts = pkg->conflicts;

  requires_iter = requires;
  while (requires_iter != NULL)
    {
      Package *req = requires_iter->data;
      
      conflicts_iter = conflicts;

      while (conflicts_iter != NULL)
        {
          RequiredVersion *ver = conflicts_iter->data;

	  if (strcmp (ver->name, req->key) == 0 &&
	      version_test (ver->comparison,
			    req->version,
			    ver->version))
            {
              verbose_error ("Version %s of %s creates a conflict.\n"
                             "(%s %s %s conflicts with %s %s)\n",
                             req->version, req->key,
                             ver->name,
                             comparison_to_str (ver->comparison),
                             ver->version ? ver->version : "(any)",
                             ver->owner->key,
                             ver->owner->version);

              exit (1);
            }

          conflicts_iter = g_list_next (conflicts_iter);
        }
      
      requires_iter = g_list_next (requires_iter);
    }
  
  g_list_free (requires);

  /* We make a list of system directories that compilers expect so we
   * can remove them.
   */

  search_path = g_getenv ("PKG_CONFIG_SYSTEM_INCLUDE_PATH");

  if (search_path == NULL)
    {
      search_path = PKG_CONFIG_SYSTEM_INCLUDE_PATH;
    }

  system_directories = add_env_variable_to_list (system_directories, search_path);

#ifdef G_OS_WIN32
  include_envvars = msvc_syntax ? msvc_include_envvars : gcc_include_envvars;
#else
  include_envvars = gcc_include_envvars;
#endif
  for (var = include_envvars; *var != NULL; var++)
    {
      search_path = g_getenv (*var);
      if (search_path != NULL)
        system_directories = add_env_variable_to_list (system_directories, search_path);
    }

  count = 0;
  for (iter = pkg->cflags; iter != NULL; iter = g_list_next (iter))
    {
      gint offset = 0;
      Flag *flag = iter->data;

      if (!(flag->type & CFLAGS_I))
        continue;

      /* Handle the system cflags. We put things in canonical
       * -I/usr/include (vs. -I /usr/include) format, but if someone
       * changes it later we may as well be robust.
       *
       * Note that the -i* flags are left out of this handling since
       * they're intended to adjust the system cflags behavior.
       */
      if (((strncmp (flag->arg, "-I", 2) == 0) && (offset = 2))||
          ((strncmp (flag->arg, "-I ", 3) == 0) && (offset = 3)))
        {
	  if (offset == 0)
	    {
	      iter = iter->next;
	      continue;
	    }

	  system_dir_iter = system_directories;
	  while (system_dir_iter != NULL)
	    {
	      if (strcmp (system_dir_iter->data,
                          ((char*)flag->arg) + offset) == 0)
		{
                  debug_spew ("Package %s has %s in Cflags\n",
			      pkg->key, (gchar *)flag->arg);
		  if (g_getenv ("PKG_CONFIG_ALLOW_SYSTEM_CFLAGS") == NULL)
		    {
                      debug_spew ("Removing %s from cflags for %s\n",
                                  flag->arg, pkg->key);
		      ++count;
		      iter->data = NULL;

		      break;
		    }
		}
	      system_dir_iter = system_dir_iter->next;
	    }
        }
    }

  while (count)
    {
      pkg->cflags = g_list_remove (pkg->cflags, NULL);
      --count;
    }

  g_list_foreach (system_directories, (GFunc) g_free, NULL);
  g_list_free (system_directories);

  system_directories = NULL;

  search_path = g_getenv ("PKG_CONFIG_SYSTEM_LIBRARY_PATH");

  if (search_path == NULL)
    {
      search_path = PKG_CONFIG_SYSTEM_LIBRARY_PATH;
    }

  system_directories = add_env_variable_to_list (system_directories, search_path);

  count = 0;
  for (iter = pkg->libs; iter != NULL; iter = g_list_next (iter))
    {
      GList *system_dir_iter = system_directories;
      Flag *flag = iter->data;

      if (!(flag->type & LIBS_L))
        continue;

      while (system_dir_iter != NULL)
        {
          gboolean is_system = FALSE;
          const char *linker_arg = flag->arg;
          const char *system_libpath = system_dir_iter->data;

          if (strncmp (linker_arg, "-L ", 3) == 0 &&
              strcmp (linker_arg + 3, system_libpath) == 0)
            is_system = TRUE;
          else if (strncmp (linker_arg, "-L", 2) == 0 &&
              strcmp (linker_arg + 2, system_libpath) == 0)
            is_system = TRUE;
          if (is_system)
            {
              debug_spew ("Package %s has -L %s in Libs\n",
                          pkg->key, system_libpath);
              if (g_getenv ("PKG_CONFIG_ALLOW_SYSTEM_LIBS") == NULL)
                {
                  iter->data = NULL;
                  ++count;
                  debug_spew ("Removing -L %s from libs for %s\n",
                              system_libpath, pkg->key);
                  break;
                }
            }
          system_dir_iter = system_dir_iter->next;
        }
    }
  g_list_free (system_directories);

  while (count)
    {
      pkg->libs = g_list_remove (pkg->libs, NULL);
      --count;
    }
}

/* Create a merged list of required packages and retrieve the flags from them.
 * Strip the duplicates from the flags list. The sorting and stripping can be
 * done in one of two ways: packages sorted by position in the pkg-config path
 * and stripping done from the beginning of the list, or packages sorted from
 * most dependent to least dependent and stripping from the end of the list.
 * The former is done for -I/-L flags, and the latter for all others.
 */
static char *
get_multi_merged (GList *pkgs, FlagType type, gboolean in_path_order,
                  gboolean include_private)
{
  GList *list;
  char *retval;

  list = fill_list (pkgs, type, in_path_order, include_private);
  list = flag_list_strip_duplicates (list);
  retval = flag_list_to_string (list);
  g_list_free (list);

  return retval;
}

char *
packages_get_flags (GList *pkgs, FlagType flags)
{
  GString *str;
  char *cur;

  str = g_string_new (NULL);

  /* sort packages in path order for -L/-I, dependency order otherwise */
  if (flags & CFLAGS_OTHER)
    {
      cur = get_multi_merged (pkgs, CFLAGS_OTHER, FALSE, TRUE);
      debug_spew ("adding CFLAGS_OTHER string \"%s\"\n", cur);
      g_string_append (str, cur);
      g_free (cur);
    }
  if (flags & CFLAGS_I)
    {
      cur = get_multi_merged (pkgs, CFLAGS_I, TRUE, TRUE);
      debug_spew ("adding CFLAGS_I string \"%s\"\n", cur);
      g_string_append (str, cur);
      g_free (cur);
    }
  if (flags & LIBS_L)
    {
      cur = get_multi_merged (pkgs, LIBS_L, TRUE, !ignore_private_libs);
      debug_spew ("adding LIBS_L string \"%s\"\n", cur);
      g_string_append (str, cur);
      g_free (cur);
    }
  if (flags & (LIBS_OTHER | LIBS_l))
    {
      cur = get_multi_merged (pkgs, flags & (LIBS_OTHER | LIBS_l), FALSE,
                              !ignore_private_libs);
      debug_spew ("adding LIBS_OTHER | LIBS_l string \"%s\"\n", cur);
      g_string_append (str, cur);
      g_free (cur);
    }

  /* Strip trailing space. */
  if (str->len > 0 && str->str[str->len - 1] == ' ')
    g_string_truncate (str, str->len - 1);

  debug_spew ("returning flags string \"%s\"\n", str->str);
  return g_string_free (str, FALSE);
}

void
define_global_variable (const char *varname,
                        const char *varval)
{
  if (globals == NULL)
    globals = g_hash_table_new (g_str_hash, g_str_equal);

  if (g_hash_table_lookup (globals, varname))
    {
      verbose_error ("Variable '%s' defined twice globally\n", varname);
      exit (1);
    }
  
  g_hash_table_insert (globals, g_strdup (varname), g_strdup (varval));
      
  debug_spew ("Global variable definition '%s' = '%s'\n",
              varname, varval);
}

char *
var_to_env_var (const char *pkg, const char *var)
{
  char *new = g_strconcat ("PKG_CONFIG_", pkg, "_", var, NULL);
  char *p;
  for (p = new; *p != 0; p++)
    {
      char c = g_ascii_toupper (*p);

      if (!g_ascii_isalnum (c))
        c = '_';

      *p = c;
    }

  return new;
}

char *
package_get_var (Package *pkg,
                 const char *var)
{
  char *varval = NULL;

  if (globals)
    varval = g_strdup (g_hash_table_lookup (globals, var));

  /* Allow overriding specific variables using an environment variable of the
   * form PKG_CONFIG_$PACKAGENAME_$VARIABLE
   */
  if (pkg->key)
    {
      char *env_var = var_to_env_var (pkg->key, var);
      const char *env_var_content = g_getenv (env_var);
      g_free (env_var);
      if (env_var_content)
        {
          debug_spew ("Overriding variable '%s' from environment\n", var);
          return g_strdup (env_var_content);
        }
    }


  if (varval == NULL && pkg->vars)
    varval = g_strdup (g_hash_table_lookup (pkg->vars, var));

  return varval;
}

char *
packages_get_var (GList      *pkgs,
                  const char *varname)
{
  GList *tmp;
  GString *str;

  str = g_string_new (NULL);

  tmp = pkgs;
  while (tmp != NULL)
    {
      Package *pkg = tmp->data;
      char *var;

      var = parse_package_variable (pkg, varname);
      if (var)
        {
          if (str->len > 0)
            g_string_append_c (str, ' ');
          g_string_append (str, var);
          g_free (var);
        }

      tmp = g_list_next (tmp);
    }

  return g_string_free (str, FALSE);
}

int
compare_versions (const char * a, const char *b)
{
  return rpmvercmp (a, b);
}

gboolean
version_test (ComparisonType comparison,
              const char *a,
              const char *b)
{
  switch (comparison)
    {
    case LESS_THAN:
      return compare_versions (a, b) < 0;
      break;

    case GREATER_THAN:
      return compare_versions (a, b) > 0;
      break;

    case LESS_THAN_EQUAL:
      return compare_versions (a, b) <= 0;
      break;

    case GREATER_THAN_EQUAL:
      return compare_versions (a, b) >= 0;
      break;

    case EQUAL:
      return compare_versions (a, b) == 0;
      break;

    case NOT_EQUAL:
      return compare_versions (a, b) != 0;
      break;

    case ALWAYS_MATCH:
      return TRUE;
      break;
      
    default:
      g_assert_not_reached ();
      break;
    }

  return FALSE;
}

const char *
comparison_to_str (ComparisonType comparison)
{
  switch (comparison)
    {
    case LESS_THAN:
      return "<";
      break;

    case GREATER_THAN:
      return ">";
      break;

    case LESS_THAN_EQUAL:
      return "<=";
      break;

    case GREATER_THAN_EQUAL:
      return ">=";
      break;

    case EQUAL:
      return "=";
      break;

    case NOT_EQUAL:
      return "!=";
      break;

    case ALWAYS_MATCH:
      return "(any)";
      break;
      
    default:
      g_assert_not_reached ();
      break;
    }

  return "???";
}

static void
max_len_foreach (gpointer key, gpointer value, gpointer data)
{
  int *mlen = data;

  *mlen = MAX (*mlen, strlen (key));
}

static void
packages_foreach (gpointer key, gpointer value, gpointer data)
{
  Package *pkg = value;
  char *pad;

  pad = g_strnfill (GPOINTER_TO_INT (data) - strlen (pkg->key), ' ');
      
  printf ("%s%s%s - %s\n",
          pkg->key, pad, pkg->name, pkg->description);

  g_free (pad);
}

void
print_package_list (void)
{
  int mlen = 0;

  ignore_requires = TRUE;
  ignore_requires_private = TRUE;

  g_hash_table_foreach (packages, max_len_foreach, &mlen);
  g_hash_table_foreach (packages, packages_foreach, GINT_TO_POINTER (mlen + 1));
}

void
enable_private_libs(void)
{
  ignore_private_libs = FALSE;
}

void
disable_private_libs(void)
{
  ignore_private_libs = TRUE;
}

void
enable_requires(void)
{
  ignore_requires = FALSE;
}

void
disable_requires(void)
{
  ignore_requires = TRUE;
}

void
enable_requires_private(void)
{
  ignore_requires_private = FALSE;
}

void
disable_requires_private(void)
{
  ignore_requires_private = TRUE;
}
