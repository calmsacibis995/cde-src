/* $XConsortium: FileListUtilsI.h /main/cde1_maint/1 1995/07/17 17:27:09 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        FileListUtilsI.h
 **
 **   Project:     DtHelp Project
 **
 **   Description: File locating and handling utilities
 ** 
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _FileListUtilsI_h
#define _FileListUtilsI_h

#include "DisplayAreaP.h"

typedef struct _DtHelpFileRec {
  struct _DtHelpFileRec *  next;           /* next file in list */
  char *                fileName;          /* basename plus extensions */
  char *                fullFilePath;      /* full path of the file */
  int                   nameKey;           /* used for quick identity compare*/
  char *                fileTitle;         /* comparable title of file */
  XmString              fileTitleXmStr;    /* displayable title of file */
  char *                docId;             /* string identifying doc */
  char *                timeStamp;         /* string with time of doc */
  void *                clientData;        /* data assoc with the file */
  unsigned int          fileSelected:1;    /* file selected for activity */
  unsigned int          reservedFlag1:1; 
  unsigned int          reservedFlag2:1;
  unsigned int          reservedFlag3:1;
  unsigned int          clientFlag1:1;
  unsigned int          clientFlag2:1;
  unsigned int          clientFlag3:1;
  unsigned int          clientFlag4:1;
} _DtHelpFileRec, * _DtHelpFileEntry, * _DtHelpFileList;

typedef void (*_DtHelpFileScanProcCB)(
          int           count,
          XtPointer     clientData);

typedef Boolean (*_DtHelpFileInfoProc)(
    DtHelpDispAreaStruct * pDisplayArea, /* in: display area in use */
    char *     volumePath,    /* in: full path to file */
    char **    ret_title,     /* out: mallocd doc title string */
    XmString * ret_titleXmStr,/* out: mallocd doc title XmStr */
    char **    ret_docId,     /* out: mallocd doc Id string */
    char **    ret_timeStamp, /* out: mallocd doc time string */
    int *      ret_nameKey,   /* out: hash value for fast discimination */
    XmFontList * io_fontList, /* io: fontList for title */
    Boolean *  ret_mod);      /* out: has font list been changed */


/* Bitwise-ORable flags that affect the comparisons 
   done when comparing two files. */
typedef enum {
  _DtHELP_FILE_NAME = 0x01,
  _DtHELP_FILE_DIR = 0x02,
  _DtHELP_FILE_TITLE = 0x04,
  _DtHELP_FILE_TIME = 0x08,
  _DtHELP_FILE_IDSTR = 0x10,
  _DtHELP_FILE_LOCALE = 0x20
  } _DtHelpFileAttributes;

#ifdef _NO_PROTO
Boolean _DtHelpFileListAddFile ();
_DtHelpFileList _DtHelpFileListGetMatch ();
Boolean _DtHelpFileIsSameP();
_DtHelpFileList _DtHelpFileListGetNext ();
int _DtHelpFileListScanPaths ();
void _DtHelpFileFreeEntry ();
#else
Boolean _DtHelpFileListAddFile (
        _DtHelpFileList *      in_out_list,
        XmFontList *           io_fontList,
        Boolean *              ret_mod,
        char *                 fullFilePath,
        char *                 fileName,
        _DtHelpFileInfoProc    infoProc,
        int                    compareFlags,
        int                    sortFlags,
        DtHelpDispAreaStruct * pDisplayArea);
_DtHelpFileList _DtHelpFileListGetMatch ( 
        _DtHelpFileList     fileListHead,
        char *              fullFilePath,
        _DtHelpFileInfoProc infoProc,
        int                 compareFlags,
        DtHelpDispAreaStruct * pDisplayArea);
Boolean _DtHelpFileIsSameP(
        char *              fileName1,
        char *              fileName2,
        _DtHelpFileInfoProc infoProc,
        int                 compareFlags,
        DtHelpDispAreaStruct * pDisplayArea);
_DtHelpFileList _DtHelpFileListGetNext (
        _DtHelpFileList fileListHead,
        _DtHelpFileList curFile);
int _DtHelpFileListScanPaths (
        _DtHelpFileList * in_out_list,
        XmFontList *      io_fontList,
        Boolean *         ret_mod,
        char *            type,
        const char *      suffixList[],
        Boolean           searchHomeDir,
        _DtHelpFileInfoProc infoProc,
        DtHelpDispAreaStruct * pDisplayArea,
        int               sysPathCompareFlags,
        int               otherPathCompareFlags,
        int               sortFlags,
        _DtHelpFileScanProcCB scanProc,
        XtPointer         clientData);
void _DtHelpFileFreeEntry (
        _DtHelpFileEntry entry);
#endif 

#endif /* _FileListUtilsI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

