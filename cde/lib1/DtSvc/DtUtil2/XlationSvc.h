/* $XConsortium: XlationSvc.h /main/cde1_maint/1 1995/07/17 17:56:26 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        XlationSvc.h
 **
 **   Project:     DtXlate
 **
 **   Description: table-based translation services
 **
 **   (c) Copyright 1993, 1994 Hewlett-Packard Company
 **   (c) Copyright 1993, 1994 International Business Machines Corp.
 **   (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **   (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/


#ifndef _DtXLATE_XLATION_SVC_I
#define _DtXLATE_XLATION_SVC_I

#include <sys/utsname.h>      /* for UTSLEN, SYS_NMLN */

#ifdef __cplusplus
extern "C" {
#endif

/*=================================================================
$SHAREDBEG$: This header appears in all appropriate DtXlate topics
$INCLUDE$
#include <XlationSvc.h>
=$END$==========================================================*/


#if DOC
/*========================================================*/
$TYPEBEG$: _DtXlateDb
$1LINER$:  An opaque object used to represent translation dbs
$SUMMARY$:
_DtXlateDb is the type of a translation database object.
The database object must be opened before use and closed
after use.  The definition of the object is opaque to users.
$ARGS$:
/*================================================$SKIP$==*/
#endif
/*$DEF$*/
typedef struct __DtXlateDbRec * _DtXlateDb;
/*$END$*/


#if DOC
/*========================================================*/
$CONSTBEG$: _DtXLATE_OPER_xxx
$1LINER$:  Constants for specifying operations 
$SUMMARY$:
The _DtXLATE_OPER_xxx are constants that produce strings 
used in the translation specifications when specifying
the operation of a translation.

The operation string name must be identical both in the
source code and in the translation table.
These constants should be used whenever referencing
operations as part of a translation.
/*================================================$SKIP$==*/
#endif
/* $DEF$, Operation constants */
#define _DtXLATE_OPER_VERSION  "version"
/*$END$*/

#if DOC
/*========================================================*/
$CONSTBEG$: _DtPLATFORM_xxx
$1LINER$:  Constants for specifying platforms strings
$SUMMARY$:
The _DtPLATFORM_xxx are constants that produce strings 
used in the translation specifications and when performing
a translation using the API.  Recall that the platform name must
be an exact match if specified as translation criteria.
These names are the same strings returned by 'uname(1) -s'
and uname(2):utsname.sysname.

The operation string name must be identical both in the
source code and in the translation table.
These constants should be used whenever referencing
platforms as part of a translation.
/*================================================$SKIP$==*/
#endif
/* $DEF$, Platform constants */
#if defined(SVR4) || defined(_AIX)
#define _DtPLATFORM_MAX_LEN SYS_NMLN
#else
#if defined(SYS_NMLN)
#define _DtPLATFORM_MAX_LEN SYS_NMLN
#else
#define _DtPLATFORM_MAX_LEN UTSLEN
#endif
#endif

#define _DtPLATFORM_UNKNOWN ((const char *)0)
#define _DtPLATFORM_CURRENT ((const char *)0)
#define _DtPLATFORM_CDE     "CDE"
#define _DtPLATFORM_HPUX    "HP-UX"
#define _DtPLATFORM_AIX     "AIX"
#define _DtPLATFORM_SUNOS   "SunOS"
#define _DtPLATFORM_SOLARIS "Solaris"      /* verify */
#define _DtPLATFORM_USL     "USL"          /* verify */
#define _DtPLATFORM_SCO     "SCO"          /* verify */
#define _DtPLATFORM_XENIX   "Xenix"        /* verify */
/*$END$*/


/* Functions */
#ifdef _NO_PROTO
int _DtXlateOpenDb();
int _DtXlateOpenAndMergeDbs();
int  _DtXlateMergeDbs();
int _DtXlateOpenAllDbs();
int _DtXlateCloseDb();
int _DtXlateStdToOpValue();
int _DtXlateOpToStdValue();
int _DtXlateGetXlateEnv();
int _DtMBStrrchr ();
int _DtMBStrchr ();
#else
int _DtXlateOpenDb(
       const char *  databaseName,
       _DtXlateDb *  ret_db);

int _DtXlateOpenAndMergeDbs(
       const char *   databaseName,
       _DtXlateDb *   io_db);

int  _DtXlateMergeDbs(
       _DtXlateDb *  io_dbToMerge,
       _DtXlateDb *  io_mergeIntoDb);

int _DtXlateOpenAllDbs(
         const char * searchPaths,
         const char * databaseName,
         _DtXlateDb * ret_db);

int _DtXlateCloseDb(
       _DtXlateDb * io_db);

int _DtXlateStdToOpValue(
       _DtXlateDb        db,
       const char *      platform,
       const int         version,
       const char *      operation,
       const char *      stdValue,
       char * *          ret_opValue,
       void *            ret_reserved);

int _DtXlateOpToStdValue(
       _DtXlateDb        db,
       const char *      platform,
       const int         version,
       const char *      operation,
       const char *      opValue,
       char * *          ret_stdValue,
       void *            ret_reserved);

int _DtXlateGetXlateEnv(
         _DtXlateDb db,
         char *     ret_AppExecEnvPlatform,
         int *      ret_AppExecEnvVersion,
         int *      ret_XlateCompiledForOSVersion);

/* Non DtXlate functions currently in XlationSvc.c */
int _DtMBStrrchr (
    const char *   s1,
    int            value,
    int            max_len,
    const char * * ret_ptr );

int _DtMBStrchr (
    const char * s1,
    int          value,
    int          max_len,
    const char * * ret_ptr );
#endif  /* _NO_PROTO */

#ifdef __cplusplus
}
#endif

#endif /*_DtXLATE_XLATION_SVC_I*/
/********* do not put anything below this line ********/
