/* $XConsortium: WmHelp.h /main/cde1_maint/1 1995/07/18 01:55:53 drk $ */
#ifdef WSM
/* 
 * (c) Copyright 1987, 1988, 1989, 1990, 1992 HEWLETT-PACKARD COMPANY 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif

#include "WmGlobal.h"
/*
 *    Various types of help available.
 */

#define WM_DT_HELP_VOLUME 			"FPanel"
#define WM_DT_HELP_TOPIC  			"_HOMETOPIC"

#define WM_DT_WSRENAME_HELP_TOPIC  		"WSRENAME"
#define WM_DT_WSRENAMEERROR_HELP_TOPIC 		"WSRENAMEERROR"
#define WM_DT_ICONBOX_TOPIC  			"ICONBOX"
#define WM_DT_WSPRESENCE_TOPIC  		"WSPRESENCE"

typedef struct _WmHelpTopicData
{
   char *helpVolume;
   char *helpTopic;

} WmHelpTopicData;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void SaveHelpResources();
extern Boolean RestoreHelpDialogs();
extern Boolean WmDtHelp ();
extern Boolean WmDtHelpMode ();
extern void WmDtHelpOnVersion ();
extern void WmDtStringHelpCB ();
extern void WmDtWmStringHelpCB ();
extern void WmDtDisplayTopicHelp ();
extern void WmDtDisplayStringHelp ();
extern void WmDtWmTopicHelpCB ();
#else /* _NO_PROTO */

extern Boolean RestoreHelpDialogs(
			      WmScreenData *pSD);
extern void SaveHelpResources(
			      WmScreenData *pSD);
extern Boolean WmDtHelp (
			      String args);
extern Boolean WmDtHelpMode (
			      void);
extern void WmDtHelpOnVersion (
				Widget shell);
extern void WmDtStringHelpCB (Widget  theWidget,
			       XtPointer  client_data,
			       XtPointer  call_data );

extern void WmDtWmStringHelpCB (Widget  theWidget,
			       XtPointer  client_data,
			       XtPointer  call_data );

extern void WmDtDisplayTopicHelp (Widget widget,
                        	char * helpVolume,
                        	char * locationID);

extern void WmDtDisplayStringHelp (Widget widget,
				  char * helpString);

extern void WmDtWmTopicHelpCB (Widget  theWidget,
			       XtPointer  client_data,
			       XtPointer  call_data );

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/****************************   eof    ***************************/
#endif /* WSM */
