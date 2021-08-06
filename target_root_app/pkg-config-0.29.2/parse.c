/* 
 * Copyright (C) 2006-2011 Tollef Fog Heen <tfheen@err.no>
 * Copyright (C) 2001, 2002, 2005-2006 Red Hat Inc.
 * Copyright (C) 2010 Dan Nicholson <dbn.lists@gmail.com>
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

#include "parse.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <sys/types.h>

gboolean parse_strict = TRUE;
gboolean define_prefix = ENABLE_DEFINE_PREFIX;
char *prefix_variable = "prefix";

#ifdef G_OS_WIN32
gboolean msvc_syntax = FALSE;
#endif

/**
 * Read an entire line from a file into a buffer. Lines may
 * be delimited with '\n', '\r', '\n\r', or '\r\n'. The delimiter
 * is not written into the buffer. Text after a '#' character is treated as
 * a comment and skipped. '\' can be used to escape a # character.
 * '\' proceding a line delimiter combines adjacent lines. A '\' proceding
 * any other character is ignored and written into the output buffer
 * unmodified.
 * 
 * Return value: %FALSE if the stream was already at an EOF character.
 **/
static gboolean
read_one_line (FILE *stream, GString *str)
{
  gboolean quoted = FALSE;
  gboolean comment = FALSE;
  int n_read = 0;

  g_string_truncate (str, 0);
  
  while (1)
    {
      int c;
      
      c = getc (stream);

      if (c == EOF)
	{
	  if (quoted)
	    g_string_append_c (str, '\\');
	  
	  goto done;
	}
      else
	n_read++;

      if (quoted)
	{
	  quoted = FALSE;
	  
	  switch (c)
	    {
	    case '#':
	      g_string_append_c (str, '#');
	      break;
	    case '\r':
	    case '\n':
	      {
		int next_c = getc (stream);

		if (!(c == EOF ||
		      (c == '\r' && next_c == '\n') ||
		      (c == '\n' && next_c == '\r')))
		  ungetc (next_c, stream);
		
		break;
	      }
	    default:
	      g_string_append_c (str, '\\');	      
	      g_string_append_c (str, c);
	    }
	}
      else
	{
	  switch (c)
	    {
	    case '#':
	      comment = TRUE;
	      break;
	    case '\\':
	      if (!comment)
		quoted = TRUE;
	      break;
	    case '\n':
	      {
		int next_c = getc (stream);

		if (!(c == EOF ||
		      (c == '\r' && next_c == '\n') ||
		      (c == '\n' && next_c == '\r')))
		  ungetc (next_c, stream);

		goto done;
	      }
	    default:
	      if (!comment)
		g_string_append_c (str, c);
	    }
	}
    }

 done:

  return n_read > 0;
}

static char *
trim_string (const char *str)
{
  int len;

  g_return_val_if_fail (str != NULL, NULL);
  
  while (*str && isspace ((guchar)*str))
    str++;

  len = strlen (str);
  while (len > 0 && isspace ((guchar)str[len-1]))
    len--;

  return g_strndup (str, len);
}

static char *
trim_and_sub (Package *pkg, const char *str, const char *path)
{
  char *trimmed;
  GString *subst;
  char *p;
  
  trimmed = trim_string (str);

  subst = g_string_new ("");

  p = trimmed;
  while (*p)
    {
      if (p[0] == '$' &&
          p[1] == '$')
        {
          /* escaped $ */
          g_string_append_c (subst, '$');
          p += 2;
        }
      else if (p[0] == '$' &&
               p[1] == '{')
        {
          /* variable */
          char *var_start;
          char *varname;
          char *varval;
          
          var_start = &p[2];

          /* Get up to close brace. */
          while (*p && *p != '}')
            ++p;

          varname = g_strndup (var_start, p - var_start);

          ++p; /* past brace */
          
          varval = package_get_var (pkg, varname);
          
          if (varval == NULL)
            {
              verbose_error ("Variable '%s' not defined in '%s'\n",
                             varname, path);
              if (parse_strict)
                exit (1);
            }

          g_free (varname);

          g_string_append (subst, varval);
          g_free (varval);
        }
      else
        {
          g_string_append_c (subst, *p);

          ++p;          
        }
    }

  g_free (trimmed);
  p = subst->str;
  g_string_free (subst, FALSE);

  return p;
}

static void
parse_name (Package *pkg, const char *str, const char *path)
{
  if (pkg->name)
    {
      verbose_error ("Name field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }
  
  pkg->name = trim_and_sub (pkg, str, path);
}

static void
parse_version (Package *pkg, const char *str, const char *path)
{
  if (pkg->version)
    {
      verbose_error ("Version field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }
  
  pkg->version = trim_and_sub (pkg, str, path);
}

static void
parse_description (Package *pkg, const char *str, const char *path)
{
  if (pkg->description)
    {
      verbose_error ("Description field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }
  
  pkg->description = trim_and_sub (pkg, str, path);
}


#define MODULE_SEPARATOR(c) ((c) == ',' || isspace ((guchar)(c)))
#define OPERATOR_CHAR(c) ((c) == '<' || (c) == '>' || (c) == '!' || (c) == '=')

/* A module list is a list of modules with optional version specification,
 * separated by commas and/or spaces. Commas are treated just like whitespace,
 * in order to allow stuff like: Requires: @FRIBIDI_PC@, glib, gmodule
 * where @FRIBIDI_PC@ gets substituted to nothing or to 'fribidi'
 */

typedef enum
{
  /* put numbers to help interpret lame debug spew ;-) */
  OUTSIDE_MODULE = 0,
  IN_MODULE_NAME = 1,
  BEFORE_OPERATOR = 2,
  IN_OPERATOR = 3,
  AFTER_OPERATOR = 4,
  IN_MODULE_VERSION = 5  
} ModuleSplitState;

#define PARSE_SPEW 0

static GList *
split_module_list (const char *str, const char *path)
{
  GList *retval = NULL;
  const char *p;
  const char *start;
  ModuleSplitState state = OUTSIDE_MODULE;
  ModuleSplitState last_state = OUTSIDE_MODULE;

  /*   fprintf (stderr, "Parsing: '%s'\n", str); */
  
  start = str;
  p = str;

  while (*p)
    {
#if PARSE_SPEW
      fprintf (stderr, "p: %c state: %d last_state: %d\n", *p, state, last_state);
#endif
      
      switch (state)
        {
        case OUTSIDE_MODULE:
          if (!MODULE_SEPARATOR (*p))
            state = IN_MODULE_NAME;          
          break;

        case IN_MODULE_NAME:
          if (isspace ((guchar)*p))
            {
              /* Need to look ahead to determine next state */
              const char *s = p;
              while (*s && isspace ((guchar)*s))
                ++s;

              if (*s == '\0')
                state = OUTSIDE_MODULE;
              else if (MODULE_SEPARATOR (*s))
                state = OUTSIDE_MODULE;
              else if (OPERATOR_CHAR (*s))
                state = BEFORE_OPERATOR;
              else
                state = OUTSIDE_MODULE;
            }
          else if (MODULE_SEPARATOR (*p))
            state = OUTSIDE_MODULE; /* comma precludes any operators */
          break;

        case BEFORE_OPERATOR:
          /* We know an operator is coming up here due to lookahead from
           * IN_MODULE_NAME
           */
          if (isspace ((guchar)*p))
            ; /* no change */
          else if (OPERATOR_CHAR (*p))
            state = IN_OPERATOR;
          else
            g_assert_not_reached ();
          break;

        case IN_OPERATOR:
          if (!OPERATOR_CHAR (*p))
            state = AFTER_OPERATOR;
          break;

        case AFTER_OPERATOR:
          if (!isspace ((guchar)*p))
            state = IN_MODULE_VERSION;
          break;

        case IN_MODULE_VERSION:
          if (MODULE_SEPARATOR (*p))
            state = OUTSIDE_MODULE;
          break;
          
        default:
          g_assert_not_reached ();
        }

      if (state == OUTSIDE_MODULE &&
          last_state != OUTSIDE_MODULE)
        {
          /* We left a module */
          char *module = g_strndup (start, p - start);
          retval = g_list_prepend (retval, module);

#if PARSE_SPEW
          fprintf (stderr, "found module: '%s'\n", module);
#endif
          
          /* reset start */
          start = p;
        }
      
      last_state = state;
      ++p;
    }

  if (p != start)
    {
      /* get the last module */
      char *module = g_strndup (start, p - start);
      retval = g_list_prepend (retval, module);

#if PARSE_SPEW
      fprintf (stderr, "found module: '%s'\n", module);
#endif
      
    }
  
  retval = g_list_reverse (retval);

  return retval;
}

GList *
parse_module_list (Package *pkg, const char *str, const char *path)
{
  GList *split;
  GList *iter;
  GList *retval = NULL;

  split = split_module_list (str, path);

  for (iter = split; iter != NULL; iter = g_list_next (iter))
    {
      RequiredVersion *ver;
      char *p;
      char *start;
      
      p = iter->data;

      ver = g_new0 (RequiredVersion, 1);
      ver->comparison = ALWAYS_MATCH;
      ver->owner = pkg;
      retval = g_list_prepend (retval, ver);
      
      while (*p && MODULE_SEPARATOR (*p))
        ++p;
      
      start = p;

      while (*p && !isspace ((guchar)*p))
        ++p;

      while (*p && MODULE_SEPARATOR (*p))
        {
          *p = '\0';
          ++p;
        }

      if (*start == '\0')
        {
          verbose_error ("Empty package name in Requires or Conflicts in file '%s'\n", path);
          if (parse_strict)
            exit (1);
          else
            continue;
        }
      
      ver->name = g_strdup (start);

      start = p;

      while (*p && !isspace ((guchar)*p))
        ++p;

      while (*p && isspace ((guchar)*p))
        {
          *p = '\0';
          ++p;
        }
      
      if (*start != '\0')
        {
          if (strcmp (start, "=") == 0)
            ver->comparison = EQUAL;
          else if (strcmp (start, ">=") == 0)
            ver->comparison = GREATER_THAN_EQUAL;
          else if (strcmp (start, "<=") == 0)
            ver->comparison = LESS_THAN_EQUAL;
          else if (strcmp (start, ">") == 0)
            ver->comparison = GREATER_THAN;
          else if (strcmp (start, "<") == 0)
            ver->comparison = LESS_THAN;
          else if (strcmp (start, "!=") == 0)
            ver->comparison = NOT_EQUAL;
          else
            {
              verbose_error ("Unknown version comparison operator '%s' after "
                             "package name '%s' in file '%s'\n", start,
                             ver->name, path);
              if (parse_strict)
                exit (1);
              else
                continue;
            }
        }

      start = p;
      
      while (*p && !MODULE_SEPARATOR (*p))
        ++p;

      while (*p && MODULE_SEPARATOR (*p))
        {
          *p = '\0';
          ++p;
        }
      
      if (ver->comparison != ALWAYS_MATCH && *start == '\0')
        {
          verbose_error ("Comparison operator but no version after package "
                         "name '%s' in file '%s'\n", ver->name, path);
          if (parse_strict)
            exit (1);
          else
            {
              ver->version = g_strdup ("0");
              continue;
            }
        }

      if (*start != '\0')
        {
          ver->version = g_strdup (start);
        }

      g_assert (ver->name);
    }

  g_list_foreach (split, (GFunc) g_free, NULL);
  g_list_free (split);

  retval = g_list_reverse (retval);

  return retval;
}

static void
parse_requires (Package *pkg, const char *str, const char *path)
{
  char *trimmed;

  if (pkg->requires)
    {
      verbose_error ("Requires field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }

  trimmed = trim_and_sub (pkg, str, path);
  pkg->requires_entries = parse_module_list (pkg, trimmed, path);
  g_free (trimmed);
}

static void
parse_requires_private (Package *pkg, const char *str, const char *path)
{
  char *trimmed;

  if (pkg->requires_private)
    {
      verbose_error ("Requires.private field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }

  trimmed = trim_and_sub (pkg, str, path);
  pkg->requires_private_entries = parse_module_list (pkg, trimmed, path);
  g_free (trimmed);
}

static void
parse_conflicts (Package *pkg, const char *str, const char *path)
{
  char *trimmed;
  
  if (pkg->conflicts)
    {
      verbose_error ("Conflicts field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }

  trimmed = trim_and_sub (pkg, str, path);
  pkg->conflicts = parse_module_list (pkg, trimmed, path);
  g_free (trimmed);
}

static char *strdup_escape_shell(const char *s)
{
	size_t r_s = strlen(s)+10, c = 0;
	char *r = g_malloc(r_s);
	while (s[0]) {
		if ((s[0] < '$') ||
		    (s[0] > '$' && s[0] < '(') ||
		    (s[0] > ')' && s[0] < '+') ||
		    (s[0] > ':' && s[0] < '=') ||
		    (s[0] > '=' && s[0] < '@') ||
		    (s[0] > 'Z' && s[0] < '^') ||
		    (s[0] == '`') ||
		    (s[0] > 'z' && s[0] < '~') ||
		    (s[0] > '~')) {
			r[c] = '\\';
			c++;
		}
		r[c] = *s;
		c++;
		if (c+2 >= r_s) {
			r_s *= 2;
			r = g_realloc(r, r_s);
		}
		s++;
	}
	r[c] = 0;
	return r;
}

static void _do_parse_libs (Package *pkg, int argc, char **argv)
{
  int i;
#ifdef G_OS_WIN32
  char *L_flag = (msvc_syntax ? "/libpath:" : "-L");
  char *l_flag = (msvc_syntax ? "" : "-l");
  char *lib_suffix = (msvc_syntax ? ".lib" : "");
#else
  char *L_flag = "-L";
  char *l_flag = "-l";
  char *lib_suffix = "";
#endif

  i = 0;
  while (i < argc)
    {
      Flag *flag = g_new (Flag, 1);
      char *tmp = trim_string (argv[i]);
      char *arg = strdup_escape_shell(tmp);
      char *p;
      p = arg;
      g_free(tmp);

      if (p[0] == '-' &&
          p[1] == 'l' &&
	  /* -lib: is used by the C# compiler for libs; it's not an -l
              flag. */
	  (strncmp(p, "-lib:", 5) != 0))
        {
          p += 2;
          while (*p && isspace ((guchar)*p))
            ++p;

          flag->type = LIBS_l;
          flag->arg = g_strconcat (l_flag, p, lib_suffix, NULL);
          pkg->libs = g_list_prepend (pkg->libs, flag);
        }
      else if (p[0] == '-' &&
               p[1] == 'L')
        {
          p += 2;
          while (*p && isspace ((guchar)*p))
            ++p;

          flag->type = LIBS_L;
          flag->arg = g_strconcat (L_flag, p, NULL);
          pkg->libs = g_list_prepend (pkg->libs, flag);
	}
      else if ((strcmp("-framework", p) == 0 ||
                strcmp("-Wl,-framework", p) == 0) &&
               i+1 < argc)
        {
          /* Mac OS X has a -framework Foo which is really one option,
           * so we join those to avoid having -framework Foo
           * -framework Bar being changed into -framework Foo Bar
           * later
          */
          gchar *framework, *tmp = trim_string (argv[i+1]);

          framework = strdup_escape_shell(tmp);
          flag->type = LIBS_OTHER;
          flag->arg = g_strconcat (arg, " ", framework, NULL);
          pkg->libs = g_list_prepend (pkg->libs, flag);
          i++;
          g_free (framework);
          g_free (tmp);
        }
      else if (*arg != '\0')
        {
          flag->type = LIBS_OTHER;
          flag->arg = g_strdup (arg);
          pkg->libs = g_list_prepend (pkg->libs, flag);
        }
      else
        /* flag wasn't used */
        g_free (flag);

      g_free (arg);

      ++i;
    }

}


static void
parse_libs (Package *pkg, const char *str, const char *path)
{
  /* Strip out -l and -L flags, put them in a separate list. */
  
  char *trimmed;
  char **argv = NULL;
  int argc = 0;
  GError *error = NULL;
  
  if (pkg->libs_num > 0)
    {
      verbose_error ("Libs field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }
  
  trimmed = trim_and_sub (pkg, str, path);

  if (trimmed && *trimmed &&
      !g_shell_parse_argv (trimmed, &argc, &argv, &error))
    {
      verbose_error ("Couldn't parse Libs field into an argument vector: %s\n",
                     error ? error->message : "unknown");
      if (parse_strict)
        exit (1);
      else
        {
          g_free (trimmed);
          return;
        }
    }

  _do_parse_libs(pkg, argc, argv);

  g_free (trimmed);
  g_strfreev (argv);
  pkg->libs_num++;
}

static void
parse_libs_private (Package *pkg, const char *str, const char *path)
{
  /*
    List of private libraries.  Private libraries are libraries which
    are needed in the case of static linking or on platforms not
    supporting inter-library dependencies.  They are not supposed to
    be used for libraries which are exposed through the library in
    question.  An example of an exposed library is GTK+ exposing Glib.
    A common example of a private library is libm.
    
    Generally, if include another library's headers in your own, it's
    a public dependency and not a private one.
  */
  
  char *trimmed;
  char **argv = NULL;
  int argc = 0;
  GError *error = NULL;
  
  if (pkg->libs_private_num > 0)
    {
      verbose_error ("Libs.private field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }
  
  trimmed = trim_and_sub (pkg, str, path);

  if (trimmed && *trimmed &&
      !g_shell_parse_argv (trimmed, &argc, &argv, &error))
    {
      verbose_error ("Couldn't parse Libs.private field into an argument vector: %s\n",
                     error ? error->message : "unknown");
      if (parse_strict)
        exit (1);
      else
        {
          g_free (trimmed);
          return;
        }
    }

  _do_parse_libs(pkg, argc, argv);

  g_strfreev (argv);
  g_free (trimmed);

  pkg->libs_private_num++;
}

static void
parse_cflags (Package *pkg, const char *str, const char *path)
{
  /* Strip out -I flags, put them in a separate list. */
  
  char *trimmed;
  char **argv = NULL;
  int argc = 0;
  GError *error = NULL;
  int i;
  
  if (pkg->cflags)
    {
      verbose_error ("Cflags field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }
  
  trimmed = trim_and_sub (pkg, str, path);

  if (trimmed && *trimmed &&
      !g_shell_parse_argv (trimmed, &argc, &argv, &error))
    {
      verbose_error ("Couldn't parse Cflags field into an argument vector: %s\n",
                     error ? error->message : "unknown");
      if (parse_strict)
        exit (1);
      else
        {
          g_free (trimmed);
          return;
        }
    }

  i = 0;
  while (i < argc)
    {
      Flag *flag = g_new (Flag, 1);
      char *tmp = trim_string (argv[i]);
      char *arg = strdup_escape_shell(tmp);
      char *p = arg;
      g_free(tmp);

      if (p[0] == '-' &&
          p[1] == 'I')
        {
          p += 2;
          while (*p && isspace ((guchar)*p))
            ++p;

          flag->type = CFLAGS_I;
          flag->arg = g_strconcat ("-I", p, NULL);
          pkg->cflags = g_list_prepend (pkg->cflags, flag);
        }
      else if ((strcmp ("-idirafter", arg) == 0 ||
                strcmp ("-isystem", arg) == 0) &&
               i+1 < argc)
        {
          char *option, *tmp;

          tmp = trim_string (argv[i+1]);
          option = strdup_escape_shell (tmp);

          /* These are -I flags since they control the search path */
          flag->type = CFLAGS_I;
          flag->arg = g_strconcat (arg, " ", option, NULL);
          pkg->cflags = g_list_prepend (pkg->cflags, flag);
          i++;
          g_free (option);
          g_free (tmp);
        }
      else if (*arg != '\0')
        {
          flag->type = CFLAGS_OTHER;
          flag->arg = g_strdup (arg);
          pkg->cflags = g_list_prepend (pkg->cflags, flag);
        }
      else
        /* flag wasn't used */
        g_free (flag);

      g_free (arg);
      
      ++i;
    }

  g_strfreev (argv);
  g_free (trimmed);
}

static void
parse_url (Package *pkg, const char *str, const char *path)
{
  if (pkg->url != NULL)
    {
      verbose_error ("URL field occurs twice in '%s'\n", path);
      if (parse_strict)
        exit (1);
      else
        return;
    }

  pkg->url = trim_and_sub (pkg, str, path);
}

static void
parse_line (Package *pkg, const char *untrimmed, const char *path,
	    gboolean ignore_requires, gboolean ignore_private_libs,
	    gboolean ignore_requires_private)
{
  char *str;
  char *p;
  char *tag;

  debug_spew ("  line>%s\n", untrimmed);
  
  str = trim_string (untrimmed);
  
  if (*str == '\0') /* empty line */
    {
      g_free(str);
      return;
    }
  
  p = str;

  /* Get first word */
  while ((*p >= 'A' && *p <= 'Z') ||
	 (*p >= 'a' && *p <= 'z') ||
	 (*p >= '0' && *p <= '9') ||
	 *p == '_' || *p == '.')
    p++;

  tag = g_strndup (str, p - str);
  
  while (*p && isspace ((guchar)*p))
    ++p;

  if (*p == ':')
    {
      /* keyword */
      ++p;
      while (*p && isspace ((guchar)*p))
        ++p;

      if (strcmp (tag, "Name") == 0)
        parse_name (pkg, p, path);
      else if (strcmp (tag, "Description") == 0)
        parse_description (pkg, p, path);
      else if (strcmp (tag, "Version") == 0)
        parse_version (pkg, p, path);
      else if (strcmp (tag, "Requires.private") == 0)
	{
	  if (!ignore_requires_private)
	    parse_requires_private (pkg, p, path);
	}
      else if (strcmp (tag, "Requires") == 0)
	{
          if (ignore_requires == FALSE)
	    parse_requires (pkg, p, path);
          else
	    goto cleanup;
        }
      else if ((strcmp (tag, "Libs.private") == 0) && 
               ignore_private_libs == FALSE)
        parse_libs_private (pkg, p, path);
      else if (strcmp (tag, "Libs") == 0)
        parse_libs (pkg, p, path);
      else if (strcmp (tag, "Cflags") == 0 ||
               strcmp (tag, "CFlags") == 0)
        parse_cflags (pkg, p, path);
      else if (strcmp (tag, "Conflicts") == 0)
        parse_conflicts (pkg, p, path);
      else if (strcmp (tag, "URL") == 0)
        parse_url (pkg, p, path);
      else
        {
	  /* we don't error out on unknown keywords because they may
	   * represent additions to the .pc file format from future
	   * versions of pkg-config.  We do make a note of them in the
	   * debug spew though, in order to help catch mistakes in .pc
	   * files. */
          debug_spew ("Unknown keyword '%s' in '%s'\n",
		      tag, path);
        }
    }
  else if (*p == '=')
    {
      /* variable */
      char *varname;
      char *varval;
      
      ++p;
      while (*p && isspace ((guchar)*p))
        ++p;

      if (define_prefix && strcmp (tag, prefix_variable) == 0)
	{
	  /* This is the prefix variable. Try to guesstimate a value for it
	   * for this package from the location of the .pc file.
	   */
          gchar *base;
          gboolean is_pkgconfigdir;

          base = g_path_get_basename (pkg->pcfiledir);
          is_pkgconfigdir = g_ascii_strcasecmp (base, "pkgconfig") == 0;
          g_free (base);
          if (is_pkgconfigdir)
            {
              /* It ends in pkgconfig. Good. */
              gchar *q;
              gchar *prefix;
	      
              /* Keep track of the original prefix value. */
              pkg->orig_prefix = g_strdup (p);

              /* Get grandparent directory for new prefix. */
              q = g_path_get_dirname (pkg->pcfiledir);
              prefix = g_path_get_dirname (q);
              g_free (q);
	      
	      /* Turn backslashes into slashes or
	       * g_shell_parse_argv() will eat them when ${prefix}
	       * has been expanded in parse_libs().
	       */
	      q = prefix;
	      while (*q)
		{
		  if (*q == '\\')
		    *q = '/';
		  q++;
		}

	      /* Now escape the special characters so that there's no danger
	       * of arguments that include the prefix getting split.
	       */
	      q = prefix;
	      prefix = strdup_escape_shell (prefix);
	      g_free (q);

	      varname = g_strdup (tag);
	      debug_spew (" Variable declaration, '%s' overridden with '%s'\n",
			  tag, prefix);
	      g_hash_table_insert (pkg->vars, varname, prefix);
	      goto cleanup;
	    }
	}
      else if (define_prefix &&
	       pkg->orig_prefix != NULL &&
	       *(pkg->orig_prefix) != '\0' &&
	       strncmp (p, pkg->orig_prefix, strlen (pkg->orig_prefix)) == 0 &&
	       G_IS_DIR_SEPARATOR (p[strlen (pkg->orig_prefix)]))
	{
	  char *oldstr = str;

	  p = str = g_strconcat (g_hash_table_lookup (pkg->vars, prefix_variable),
				 p + strlen (pkg->orig_prefix), NULL);
	  g_free (oldstr);
	}

      if (g_hash_table_lookup (pkg->vars, tag))
        {
          verbose_error ("Duplicate definition of variable '%s' in '%s'\n",
                         tag, path);
          if (parse_strict)
            exit (1);
          else
            goto cleanup;
        }

      varname = g_strdup (tag);
      varval = trim_and_sub (pkg, p, path);     

      debug_spew (" Variable declaration, '%s' has value '%s'\n",
                  varname, varval);
      g_hash_table_insert (pkg->vars, varname, varval);
  
    }

 cleanup:  
  g_free (str);
  g_free (tag);
}

Package*
parse_package_file (const char *key, const char *path,
                    gboolean ignore_requires,
                    gboolean ignore_private_libs,
                    gboolean ignore_requires_private)
{
  FILE *f;
  Package *pkg;
  GString *str;
  gboolean one_line = FALSE;
  
  f = fopen (path, "r");

  if (f == NULL)
    {
      verbose_error ("Failed to open '%s': %s\n",
                     path, strerror (errno));
      
      return NULL;
    }

  debug_spew ("Parsing package file '%s'\n", path);
  
  pkg = g_new0 (Package, 1);
  pkg->key = g_strdup (key);

  if (path)
    {
      pkg->pcfiledir = g_dirname (path);
    }
  else
    {
      debug_spew ("No pcfiledir determined for package\n");
      pkg->pcfiledir = g_strdup ("???????");
    }

  if (pkg->vars == NULL)
    pkg->vars = g_hash_table_new (g_str_hash, g_str_equal);

  /* Variable storing directory of pc file */
  g_hash_table_insert (pkg->vars, "pcfiledir", pkg->pcfiledir);

  str = g_string_new ("");

  while (read_one_line (f, str))
    {
      one_line = TRUE;
      
      parse_line (pkg, str->str, path, ignore_requires, ignore_private_libs,
		  ignore_requires_private);

      g_string_truncate (str, 0);
    }

  if (!one_line)
    verbose_error ("Package file '%s' appears to be empty\n",
                   path);
  g_string_free (str, TRUE);
  fclose(f);

  pkg->cflags = g_list_reverse (pkg->cflags);
  pkg->libs = g_list_reverse (pkg->libs);
  
  return pkg;
}

/* Parse a package variable. When the value appears to be quoted,
 * unquote it so it can be more easily used in a shell. Otherwise,
 * return the raw value.
 */
char *
parse_package_variable (Package *pkg, const char *variable)
{
  char *value;
  char *unquoted;
  GError *error = NULL;

  value = package_get_var (pkg, variable);
  if (!value)
    return NULL;

  if (*value != '"' && *value != '\'')
    /* Not quoted, return raw value */
    return value;

  /* Maybe too naive, but assume a fully quoted variable */
  unquoted = g_shell_unquote (value, &error);
  if (unquoted)
    {
      g_free (value);
      return unquoted;
    }
  else
    {
      /* Note the issue, but just return the raw value */
      debug_spew ("Couldn't unquote value of \"%s\": %s\n",
                  variable, error ? error->message : "unknown");
      g_clear_error (&error);
      return value;
    }
}
