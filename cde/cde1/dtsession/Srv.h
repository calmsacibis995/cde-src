/* $XConsortium: Srv.h /main/cde1_maint/3 1995/10/09 10:58:18 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        Srv.h
 **
 **  Project:     HP DT Style Manager (part of the session manager)
 **
 **  Description:
 **  -----------
 **
 **
 *******************************************************************
 **  (c) Copyright Hewlett-Packard Company, 1990.  All rights are  
 **  reserved.  Copying or other reproduction of this program      
 **  except for archival purposes is prohibited without prior      
 **  written consent of Hewlett-Packard Company.		     
 ********************************************************************
 **
 **
 **
 *****************************************************************************
 *************************************<+>*************************************/
/*  Standard C headers  */
#include <stdio.h>

#ifdef _AIX
#include <sys/types.h>
#include <sys/dir.h>
#else
#include <dirent.h>
#include <limits.h>
#endif /* _AIX */

/*  Dt header  */
#include <Dt/UserMsg.h>

/*  Xm headers  */
#include <Xm/ColorObj.h>

/* #defines */

#define NUM_OF_COLORS        8
#define MAX_NUM_SCREENS      5

/* defines for foreground colors */
#define DYNAMIC    0
#define BLACK      1
#define WHITE      2

#define PALETTE_SUFFIX     ".dp"

#define B_O_W   "BlackWhite"
#define W_ONLY  "White"
#define W_O_B   "WhiteBlack"
#define B_ONLY  "Black"

/*
 * macro to get message catalog strings
 */
#ifdef __osf__
#ifdef _NO_PROTO
extern char *_DtGetMessage();
#else
extern char *_DtGetMessage(
                        char *filename,
                        int set,
                        int n,
                        char *s );
#endif
#endif

#ifdef MESSAGE_CAT
#ifdef __ultrix
#define _CLIENT_CAT_NAME "dtsession.cat"
#else  /* __ultrix */
#define _CLIENT_CAT_NAME "dtsession"
#endif /* __ultrix */
#ifdef __osf__
  #define GETMESSAGE(set, number, string)\
    (_DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#else
#define GETMESSAGE(set, number, string)\
      ((char *) _DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#endif
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

/*******************************************************************************
 * Color Palette data structures
 *
 ******************************************************************************/

/*
**  Define a colorset as foreground, background, topshadow, bottomshadow
**  and selectcolor (also known as arm color).
*/
typedef struct _colorset {
    XColor fg;
    XColor bg;
    XColor ts;
    XColor bs;
    XColor sc;
} ColorSet;

typedef struct _hsv {
    int hue;
    int saturation;
    int value;
} HSVset;

/*
**  Structure which stores the palettes for the customizer
*/
typedef struct _palette {
      char *name;
      char *directory;
      int num_of_colors;
      ColorSet color[MAX_NUM_COLORS];
      short defaultActive;
      short defaultInactive;
      short defaultPrimary;
      short defaultSecondary;
      short defaultRoot;
      char *converted;
      unsigned long converted_len;
      struct _palette *prev;
      struct _palette *next;
} palettes;

/****** Global Variables ********/

typedef struct {
   int                TypeOfMonitor[MAX_NUM_SCREENS];
   int                FgColor[MAX_NUM_SCREENS];
   Bool               UsePixmaps[MAX_NUM_SCREENS];
   Bool               DynamicColor[MAX_NUM_SCREENS];
   int                NumOfScreens;
   Atom               XA_CUSTOMIZE[MAX_NUM_SCREENS];
   Atom               XA_PIXEL_SET;
   Atom               XA_PALETTE_NAME;
   Atom               XA_TYPE_MONITOR;
   Atom               XA_UPDATE;
   struct _palette    *pCurrentPalette[MAX_NUM_SCREENS];
} ColorSrv;
extern ColorSrv colorSrv; /* defined in SrvPalette.c */

#define MAX_STR_LEN   128

/*
 *  Wrappers for system calls
 */
#define     SRV_MALLOC(SIZE)         XtMalloc(SIZE)
#define     SRV_REALLOC(PTR, SIZE)   XtRealloc(PTR, SIZE)
#define     SRV_FREE(PTR)            XtFree(PTR)
#define     SRV_EXIT(STATUS)         exit(status)
