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

#ifndef PKG_CONFIG_PKG_H
#define PKG_CONFIG_PKG_H

#include <glib.h>

typedef guint8 FlagType; /* bit mask for flag types */

#define LIBS_l       (1 << 0)
#define LIBS_L       (1 << 1)
#define LIBS_OTHER   (1 << 2)
#define CFLAGS_I     (1 << 3)
#define CFLAGS_OTHER (1 << 4)

#define LIBS_ANY     (LIBS_l | LIBS_L | LIBS_OTHER)
#define CFLAGS_ANY   (CFLAGS_I | CFLAGS_OTHER)
#define FLAGS_ANY    (LIBS_ANY | CFLAGS_ANY)

typedef enum
{
  LESS_THAN,
  GREATER_THAN,
  LESS_THAN_EQUAL,
  GREATER_THAN_EQUAL,
  EQUAL,
  NOT_EQUAL,
  ALWAYS_MATCH
} ComparisonType;

typedef struct Flag_ Flag;
typedef struct Package_ Package;
typedef struct RequiredVersion_ RequiredVersion;

struct Flag_
{
  FlagType type;
  char *arg;
};

struct RequiredVersion_
{
  char *name;
  ComparisonType comparison;
  char *version;
  Package *owner;
};

struct Package_
{
  char *key;  /* filename name */
  char *name; /* human-readable name */
  char *version;
  char *description;
  char *url;
  char *pcfiledir; /* directory it was loaded from */
  GList *requires_entries;
  GList *requires;
  GList *requires_private_entries;
  GList *requires_private;
  GList *libs;
  GList *cflags;
  GHashTable *vars;
  GHashTable *required_versions; /* hash from name to RequiredVersion */
  GList *conflicts; /* list of RequiredVersion */
  gboolean uninstalled; /* used the -uninstalled file */
  int path_position; /* used to order packages by position in path of their .pc file, lower number means earlier in path */
  int libs_num; /* Number of times the "Libs" header has been seen */
  int libs_private_num;  /* Number of times the "Libs.private" header has been seen */
  char *orig_prefix; /* original prefix value before redefinition */
};

Package *get_package               (const char *name);
Package *get_package_quiet         (const char *name);
char *   packages_get_flags        (GList      *pkgs,
                                    FlagType   flags);
char *   package_get_var           (Package    *pkg,
                                    const char *var);
char *   packages_get_var          (GList      *pkgs,
                                    const char *var);

void add_search_dir (const char *path);
void add_search_dirs (const char *path, const char *separator);
void package_init (gboolean want_list);
int compare_versions (const char * a, const char *b);
gboolean version_test (ComparisonType comparison,
                       const char *a,
                       const char *b);

const char *comparison_to_str (ComparisonType comparison);

void print_package_list (void);

void define_global_variable (const char *varname,
                             const char *varval);

void debug_spew (const char *format, ...);
void verbose_error (const char *format, ...);

gboolean name_ends_in_uninstalled (const char *str);

void enable_private_libs(void);
void disable_private_libs(void);
void enable_requires(void);
void disable_requires(void);
void enable_requires_private(void);
void disable_requires_private(void);

/* If TRUE, do not automatically prefer uninstalled versions */
extern gboolean disable_uninstalled;

extern char *pcsysrootdir;

/* pkg-config default search path. On Windows the current pkg-config install
 * directory is used. Otherwise, the build-time defined PKG_CONFIG_PC_PATH.
 */
extern char *pkg_config_pc_path;

/* Exit on parse errors if TRUE. */
extern gboolean parse_strict;

/* If TRUE, define "prefix" in .pc files at runtime. */
extern gboolean define_prefix;

/* The name of the variable that acts as prefix, unless it is "prefix" */
extern char *prefix_variable;

#ifdef G_OS_WIN32
/* If TRUE, output flags in MSVC syntax. */
extern gboolean msvc_syntax;
#endif

#endif
