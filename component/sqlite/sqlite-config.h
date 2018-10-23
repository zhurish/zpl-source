/*
 * sqlite-config.h
 *
 *  Created on: Sep 23, 2018
 *      Author: zhurish
 */

#ifndef __SQLITE_SQLITE_CONFIG_H__
#define __SQLITE_SQLITE_CONFIG_H__


#define _REENTRANT 1
//#define SQLITE_THREADSAFE 1
#define SQLITE_ENABLE_FTS4 1
#define SQLITE_ENABLE_FTS5 1
#define SQLITE_ENABLE_JSON1 1
#define SQLITE_ENABLE_RTREE 1
//#define SQLITE_HAVE_ZLIB 1

/*
#define SQLITE_ENABLE_EXPLAIN_COMMENTS 1
#define SQLITE_ENABLE_DBPAGE_VTAB 1
#define SQLITE_ENABLE_STMTVTAB 1
#define SQLITE_ENABLE_DBSTAT_VTAB 1

*/

//#define __linux__
//#define SQLITE_OS_UNIX 1


#define PACKAGE_NAME "sqlite"
#define PACKAGE_TARNAME "sqlite"
#define PACKAGE_VERSION "3.25.1"
#define PACKAGE_STRING "sqlite 3.25.1"
#define PACKAGE_BUGREPORT "http://www.sqlite.org"
#define PACKAGE_URL " "
#define PACKAGE "sqlite"
#define VERSION "3.25.1"

#define LT_OBJDIR ".libs/"


#define STDC_HEADERS       1
#define HAVE_SYS_TYPES_H       1
#define HAVE_SYS_STAT_H       1
#define HAVE_STDLIB_H       1
#define HAVE_STRING_H       1
#define HAVE_MEMORY_H       1
#define HAVE_STRINGS_H       1
#define HAVE_INTTYPES_H       1
#define HAVE_STDINT_H       1
#define HAVE_UNISTD_H       1
#define HAVE_DLFCN_H       1

#define HAVE_FDATASYNC       1
#define HAVE_USLEEP       1
#define HAVE_LOCALTIME_R       1
#define HAVE_GMTIME_R       1
#define HAVE_DECL_STRERROR_R       1
#define HAVE_STRERROR_R       1
//#define HAVE_READLINE_READLINE_H       1
//#define HAVE_READLINE       1
//#define HAVE_POSIX_FALLOCATE       1
//#define HAVE_ZLIB_H       1



/*

#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
*/


#endif /* __SQLITE_SQLITE_CONFIG_H__ */
