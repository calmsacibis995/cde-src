/* $XConsortium: GlobSearchP.h /main/cde1_maint/2 1995/10/08 17:19:15 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        GlobSearchP.h
 **
 **   Project:     DtHelp Project
 **
 **   Description: Builds and displays an instance of a DtHelp GlobSearch
 **                Dialog.
 ** 
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _GlobSearchP_h
#define _GlobSearchP_h

#include "AccessP.h"
#include "FileListUtilsI.h"
#include "StringFuncsI.h"  /* for _CEStrcollProc */

typedef enum {
  _DtHelpGlobSrchVolumeUndef = 0,
  _DtHelpGlobSrchCurVolume = 1,
  _DtHelpGlobSrchSelectedVolumes,
  _DtHelpGlobSrchAllVolumes
} _DtHelpGlobSrchSources;

typedef enum {
  _DtHelpGlobSrchSearchThisVolume = 32,
  _DtHelpGlobSrchNothingDone = 0,
  _DtHelpGlobSrchTopicSearchInProgress = 1,
  _DtHelpGlobSrchTopicSearchDone = 2,
  _DtHelpGlobSrchIndexSearchInProgress = 4,
  _DtHelpGlobSrchIndexSearchDone = 8,
  _DtHelpGlobSrchEverySearchDone = 15
} _DtHelpGlobSrchState;

/* maintained for each volume searched thus far */
/* this data is stored in a struct pointed to by 'clientData' in _DtHelpFileList */
typedef struct _DtHelpGlobSrchVol {
  unsigned int              searchThisVolume:1;
  unsigned int              nothingDone:1;
  unsigned int              topicSearchInProgress:1;
  unsigned int              topicSearchDone:1;
  unsigned int              indexSearchInProgress:1;
  unsigned int              indexSearchDone:1;
  unsigned int              searchCompleted:1;
  unsigned int              showVolInList:1;   /* show vol in srchResultList? */
  unsigned int              zeroHitsOk:1;      /* show vol with zero hits? */
  unsigned int              hitsDisplayed:1;   /* are hits shown in list? */
  unsigned int              showHitsWithVol:1; /* show hits of the volume? */
  unsigned int              gatheredFullIndex:1; /* full index was gathered */
  unsigned int              gotLocale:1;         /* got locale already */
  struct _DtHelpGlobSrchHit *  hitListHead;
  struct _DtHelpGlobSrchHit *  hitListTail;
  int                       hitCnt;          /* number hits for this volume */
  int                       startPosition;   /* position of volume item */
  int                       nextVolPosition; /* pos of next vol with hits */
  size_t                    searchedCnt;     /* number items searched thus far */
  char * *                  indexEntriesList;/* entries for searching */
  XmString *                indexXmStrsList; /* entries for display */
  XmString *                curIndexXmStr;   /* position in indexXmStrsList */
  VolumeHandle              volHandle;
  char *                    stdLocale;       /* CDE standard locale of the volume */
  char *                    iconv3Codeset;   /* iconv-compatible codeset of volume locale */
  _CEStrcollProc            strcollProc;     /* proc to do string coll */
} _DtHelpGlobSrchVol;

/* maintained for each location hit encountered */
typedef struct _DtHelpGlobSrchHit {
  struct _DtHelpGlobSrchHit *  next;
  _DtHelpFileEntry             volume;
  int                       indexKey;     /* hash value for quick compares */
  XmString                  indexTitle;   /* title used for display */
  char *                    indexEntry;   /* string used for retrieval */
  char **                   topicIdList;  /* list of topic ids of this index */
  char **                   topicFileList;/* list of files of the topics */
  XmString *                topicTitles;  /* list of titles of the topics */
  short                     topicCnt;     /* number of topics in the list */
  unsigned int              topicsLoaded:1;     /* loaded topics of this index*/
  unsigned int              topicsDisplayed:1;  /* are topics shown in list */
  unsigned int              showTopicsWithHit:1;/* show topics if showing hit?*/
} _DtHelpGlobSrchHit;

#endif /* _GlobSearchP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
