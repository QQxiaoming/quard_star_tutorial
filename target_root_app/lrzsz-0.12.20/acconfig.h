/* define this if you have a reliable ftime function */
#undef HAVE_FTIME

/* define this if you have the timezone variable */
#undef HAVE_TIMEZONE_VAR

/* Define to 1 if NLS is requested.  */
#undef ENABLE_NLS

/* Define as 1 if you have catgets and don't want to use GNU gettext.  */
#undef HAVE_CATGETS

/* Define as 1 if you have gettext and don't want to use GNU gettext.  */
#undef HAVE_GETTEXT

/* Define if your locale.h file contains LC_MESSAGES.  */
#undef HAVE_LC_MESSAGES

/* Define to the name of the distribution.  */
#undef PACKAGE

/* The concatenation of the strings PACKAGE, "-", and VERSION.  */
#undef PACKAGE_VERSION

/* Define to the version of the distribution.  */
#undef VERSION

/* Define to 1 if you have the stpcpy function.  */
#undef HAVE_STPCPY

/* Define to 1 if your utime() takes struct utimbuf as second argument */
#undef HAVE_STRUCT_UTIMBUF

/* Define to 1 if ANSI function prototypes are usable.  */
#undef PROTOTYPES

/* Define to LOG_xxx (a syslog facility) if syslog() shall be used */
#undef ENABLE_SYSLOG

/* Define to 1 if syslogging shall be forced */
#undef ENABLE_SYSLOG_FORCE

/* Define to 1 if syslogging shall be default */
#undef ENABLE_SYSLOG_DEFAULT

/* Define to 1 if lrz shall create directories if needed */
#undef ENABLE_MKDIR

/* Define to public writable directory if you want this. Leave out the "'s */
#undef PUBDIR

/* Define to 1 if you want support for the timesync protocol */
#undef ENABLE_TIMESYNC

/* define to 1. we have a replacement function for it. */
#undef HAVE_STRERROR

/* define to 1 if you want strict ANSI prototypes. will remove some 
   extern x(); declarations. */
#undef STRICT_PROTOTYPES

/* where the localedata hides */
/* #undef LOCALEDIR */

/* do your system libraries declare errno? */
#undef HAVE_ERRNO_DECLARATION

/* define to type of speed_t (long?) */
#undef speed_t

/* define this if you headers conflict */
#undef SYS_TIME_WITHOUT_SYS_SELECT
