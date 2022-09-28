/* 
** $XConsortium: TermPrimSelectP.h /main/cde1_maint/1 1995/07/15 01:35:12 drk $
*/
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#ifndef   _Dt_TermPrimSelectP_h
#define   _Dt_TermPrimSelectP_h

#include <Xm/Xm.h>
#include "TermPrimSelect.h"

typedef enum { scanLeft, scanRight } TermScanDirection;

typedef struct _termSelectRec
{
    XEvent *event;
    String *params;
    Cardinal *num_params;
} _TermSelectRec;

typedef struct _TermSelectPrimary
{
    Atom    target;
    Time    time;
    int     num_chars;
    int     ref_count;
} _TermSelectPrimaryRec;

typedef int selectPosition;

typedef struct _TermSelectionHint
{
    int x;
    int y;
} TermSelectionHint;

/* 
** Selection specific information.
*/
typedef struct _termSelectInfoRec
{    
    Boolean               ownPrimary;   /* do we own the primary selection? */
    Time                  primaryTime;  /* time primary selection acquired  */
    TermSelectType        selectType;   /* rectanglar selection ?           */
    short                 scanArraySize;
    XmTextScanType       *scanArray;    /* possible values for scanType     */
    XmTextScanType        scanType;     /* line, word, character, all       */
    XtIntervalId          selectID;     /* AppTimeOut ID                    */
    short                 rows;         /* rows,cols current buffer useful  */
    short                 columns;      /* for converting position->row,col */
    XmTextPosition        anchor;       /* anchor for extending selecions   */
    XmTextPosition        begin;        /* upper left corner of selection   */
    XmTextPosition        end;          /* lower right corner of selection  */
    XmTextPosition        origBegin;    /* upper left corner of selection   */
    XmTextPosition        origEnd;      /* lower right corner of selection  */
    TermScanDirection     extendDir;    /* direction to extend selection    */
    TermScanDirection     direction;    /* select right or left?            */
    Time                  lastTime;     /* time of last button press        */
    Boolean               extending;    /* true if extending selection      */
    int                   threshold;    /* minimum # of pixels moved        */
    TermSelectionHint     hint;         /* initial coordinates of selection */
    TermSelectionHint     extend;       /* coordinates of extend event      */
    Boolean               cancel;       /* used to end scroll selection     */
    Boolean               isScrollUp;   /* scroll selection direction       */
    Boolean               sel_start;    /* doing selection (True) or drag   */
} TermSelectInfoRec;

#ifdef	   __cplusplus
extern "C" {
#endif	/* __cplusplus */

static Time
getServerTime
(
     Widget w
);

static void
setScanType
(
    Widget  w,
    XEvent *event,
    Boolean dotoken
);

typedef struct {
    Widget widget;
    XmTextPosition insert_pos;
    int num_chars;
    Time timestamp;
    Boolean move;
} _DtTermDropTransferRec;

#ifndef	_XmConst
#define	_XmConst /**/
#endif

void
_DtTermPrimSelect2ButtonMouse(
        Widget w,
        XEvent *event,
        char **params,
        Cardinal *num_params 
);

#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

#endif /* _Dt_TermPrimSelectP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
