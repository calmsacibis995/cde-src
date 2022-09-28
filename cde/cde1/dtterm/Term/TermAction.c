#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermAction.c /main/cde1_maint/4 1995/11/27 17:47:35 gtsang $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "TermHeader.h"
#include "TermPrimDebug.h"
#include "TermPrimI.h"
#include "TermP.h"
#include "TermPrimData.h"
/* #include "TermData.h" */
#include "TermPrimAction.h"
#include "TermAction.h"
#include "TermFunction.h"
#include "TermPrimSetPty.h"
#include "TermSendEsc.h"
#ifdef _AIX
#include <X11/keysym.h>
#endif /* _AIX */

static int v_write(int f, char *d, int len);

static char  *KeypadKey[] = { DT_KP_Space, DT_KP_Tab, DT_KP_Enter, DT_KP_F1,
                              DT_KP_F2, DT_KP_F3, DT_KP_F4, DT_KP_Equal,
                              DT_KP_Multiply, DT_KP_Add, DT_KP_Separator,
                              DT_KP_Subtract, DT_KP_Decimal, DT_KP_Divide,
                              DT_KP_0, DT_KP_1, DT_KP_2, DT_KP_3, DT_KP_4,
                              DT_KP_5, DT_KP_6, DT_KP_7, DT_KP_8, DT_KP_9};

static char  *AppKeypadKey[] ={KP_APP_Space, KP_APP_Tab, KP_APP_Enter,
                               KP_APP_F1, KP_APP_F2, KP_APP_F3, KP_APP_F4,
                               KP_APP_Equal, KP_APP_Multiply, KP_APP_Add,
                               KP_APP_Separator, KP_APP_Subtract,
                               KP_APP_Decimal, KP_APP_Divide, KP_APP_0,
                               KP_APP_1, KP_APP_2, KP_APP_3, KP_APP_4,
                               KP_APP_5, KP_APP_6, KP_APP_7, KP_APP_8,
                               KP_APP_9};
#ifdef	OBSOLETE
static char  *EditKey[] = {ESC_FIND, ESC_INSERT_HERE, ESC_DELETE,
                           ESC_SELECT, ESC_PREV_SCREEN, ESC_NEXT_SCREEN};

static char  *SunEditKey[] = {ESC_FIND_SUN, ESC_INSERT_HERE_SUN,
                              ESC_DELETE_SUN, ESC_SELECT_SUN,
                              ESC_PREV_SCREEN_SUN, ESC_NEXT_SCREEN_SUN};
#endif	/* OBSOLETE */

void _DtTermWriteEscSeq(Widget w, char *transmitString)
{
  DtTermWidget    tw  = (DtTermWidget)w;
  DtTermData      td = tw->vt.td;

  if (KEYBOARD_LOCKED(td->tpd->keyboardLocked))
    _DtTermPrimBell(w); /* keyboard locked -- ring the bell... */
  else if ( td->S8C1TMode )
    {
      char *cbuf = malloc(strlen(transmitString)+1);
      strcpy(cbuf, transmitString);
      cbuf[1] = '\x9B';
      _DtTermPrimSendInput(w, (unsigned char *) (cbuf+1), strlen(cbuf+1));
      free(cbuf);
    }
  else
    _DtTermPrimSendInput(w,
                         (unsigned char *) transmitString,
                         strlen(transmitString));

    return;
}

typedef struct
  {
      const char *string;
      char value;
  } EnumType;

static int
stringToEnum(char *c, EnumType *enumTypes, int numEnumTypes)
{
    int i;

    for (i = 0; i < numEnumTypes; i++) {
	if (!strcmp(enumTypes[i].string, c))
	    return(i);
    }

    return(-1);
}


/*** BREAK ********************************************************************
 *
 *  #####   #####   ######    ##    #    #
 *  #    #  #    #  #        #  #   #   #
 *  #####   #    #  #####   #    #  ####
 *  #    #  #####   #       ######  #  #
 *  #    #  #   #   #       #    #  #   #
 *  #####   #    #  ######  #    #  #    #
 */

void
_DtTermActionBreak(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    int msec = 0;

    /* check for keyboardLock... */
    if (KEYBOARD_LOCKED(tpd->keyboardLocked)) {
	(void) _DtTermPrimBell(w);
	return;
    }

    /* tcsendbreak() , which eventually gets called, has the msec
       parameter but it never actually gets used, so for now, its a dont care
     */

    if (tw->term.pty >= 0)
	(void) _DtTermPrimPtySendBreak(tw->term.pty, msec);
    return;
}


/*** SCROLL *******************************************************************
 *
 *   ####    ####   #####    ####   #       #
 *  #       #    #  #    #  #    #  #       #
 *   ####   #       #    #  #    #  #       #
 *       #  #       #####   #    #  #       #
 *  #    #  #    #  #   #   #    #  #       #
 *   ####    ####   #    #   ####   ######  ######
 */


typedef enum {
    scrollPage,
    scrollHalfPage,
    scrollLine,
    scrollPixel
} ScrollAmount;

static EnumType scrollUnits[] = {
    "page",	(char) scrollPage,
    "halfpage",	(char) scrollHalfPage,
    "line",	(char) scrollLine
};

void
_DtTermActionScroll(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    struct termData *tpd = tw->term.tpd;
    int unitsIndex;
    ScrollAmount units;
    int count = 1;

    if (*num_params >= 1) {
	count = strtol(params[0], (char **) 0, 0);
    }

    if (*num_params >= 2) {
	unitsIndex = stringToEnum(params[1], scrollUnits,
		XtNumber(scrollUnits));
	if (unitsIndex < 0) {
	    /* no match... */
	    return;
	}
	units = (ScrollAmount) scrollUnits[unitsIndex].value;
    } else {
	units = scrollLine;
    }

    /* perform the cursor motion...
     */
    switch(units) {
    case scrollPage:
	    _DtTermFuncScroll(w, count*(tw->term.rows-1), fromAction);
	break;

    case scrollHalfPage:
	if (count > 0) {
	    _DtTermFuncScroll(w, count *
                  (tw->term.rows - tpd->memoryLockRow) / 2, fromAction);
	} else {
	    _DtTermFuncScroll(w,
		    count * (tw->term.rows - tpd->memoryLockRow) / 2,
                    fromAction);
	}
	break;

    case scrollLine:
	    _DtTermFuncScroll(w, count, fromAction);
	break;
    }
    return;
}

void
_DtTermActionBeginningOfBuffer(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    _DtTermFuncBeginningOfBuffer(w,NULL,1) ;
    (void) _DtTermPrimCursorOn(w);
    return;
}

void
_DtTermActionEndOfBuffer(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    _DtTermFuncEndOfBuffer(w,NULL,1) ;
    (void) _DtTermPrimCursorOn(w);
    return;
}



/** HARD/SOFT RESET ***********************************************************
 *                                       #
 *  #    #    ##    #####   #####       #    ####    ####   ######   #####
 *  #    #   #  #   #    #  #    #     #    #       #    #  #          #
 *  ######  #    #  #    #  #    #    #      ####   #    #  #####      #
 *  #    #  ######  #####   #    #   #           #  #    #  #          #
 *  #    #  #    #  #   #   #    #  #       #    #  #    #  #          #
 *  #    #  #    #  #    #  #####  #         ####    ####   #          #
 *
 *
 *  #####   ######   ####   ######   #####
 *  #    #  #       #       #          #
 *  #    #  #####    ####   #####      #
 *  #####   #            #  #          #
 *  #   #   #       #    #  #          #
 *  #    #  ######   ####   ######     #
 */
void
_DtTermActionHardReset(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    (void) _DtTermFuncHardReset(w, 0, fromAction);
}

void
_DtTermActionSoftReset(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    (void) _DtTermFuncSoftReset(w, 0, fromAction);
}


/*** INSERT CHAR/LINE *********************************************************
 *
 *     #    #    #   ####   ######  #####    #####
 *     #    ##   #  #       #       #    #     #
 *     #    # #  #   ####   #####   #    #     #
 *     #    #  # #       #  #       #####      #
 *     #    #   ##  #    #  #       #   #      #
 *     #    #    #   ####   ######  #    #     #
 *
 *                                       #
 *   ####   #    #    ##    #####       #   #          #    #    #  ######
 *  #    #  #    #   #  #   #    #     #    #          #    ##   #  #
 *  #       ######  #    #  #    #    #     #          #    # #  #  #####
 *  #       #    #  ######  #####    #      #          #    #  # #  #
 *  #    #  #    #  #    #  #   #   #       #          #    #   ##  #
 *   ####   #    #  #    #  #    # #        ######     #    #    #  ######
 */


void
_DtTermActionInsertLine(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    _DtTermFuncInsertLine(w,1,1) ;
    (void) _DtTermPrimCursorOn(w);
}



/*** CURSOR MOTION ************************************************************
 *
 *   ####   #    #  #####    ####    ####   #####
 *  #    #  #    #  #    #  #       #    #  #    #
 *  #       #    #  #    #   ####   #    #  #    #
 *  #       #    #  #####        #  #    #  #####
 *  #    #  #    #  #   #   #    #  #    #  #   #
 *   ####    ####   #    #   ####    ####   #    #
 *
 *
 *  #    #   ####    #####     #     ####   #    #
 *  ##  ##  #    #     #       #    #    #  ##   #
 *  # ## #  #    #     #       #    #    #  # #  #
 *  #    #  #    #     #       #    #    #  #  # #
 *  #    #  #    #     #       #    #    #  #   ##
 *  #    #   ####      #       #     ####   #    #
 */

typedef enum {
    cursorForward,
    cursorBackward,
    cursorUp,
    cursorDown
} CursorDirection;

static EnumType cursorDirections[] = {
    "forward",	(char) cursorForward,
    "backward",	(char) cursorBackward,
    "up",	(char) cursorUp,
    "down",	(char) cursorDown
};

void
_DtTermActionMoveCursor(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    int i;

    if (*num_params < 1) {
	return;
    }

    /* figure out the direction... */
    i = stringToEnum(params[0], cursorDirections, XtNumber(cursorDirections));
    if (i < 0) {
	/* no match... */
	return;
    }

    switch((CursorDirection) cursorDirections[i].value) {
    case cursorUp:
        if (((DtTermWidget)w)->vt.td->applicationMode)
	    (void) _DtTermWriteEscSeq(w, ESC_CURSOR_UP_APP);
        else
	    (void) _DtTermWriteEscSeq(w, ESC_CURSOR_UP);
	break;

    case cursorDown:
        if (((DtTermWidget)w)->vt.td->applicationMode)
	   (void) _DtTermWriteEscSeq(w, ESC_CURSOR_DOWN_APP);
        else
	   (void) _DtTermWriteEscSeq(w, ESC_CURSOR_DOWN);
	break;

    case cursorForward:
        if (((DtTermWidget)w)->vt.td->applicationMode)
	 (void) _DtTermWriteEscSeq(w, ESC_CURSOR_RIGHT_APP);
        else
	 (void) _DtTermWriteEscSeq(w, ESC_CURSOR_RIGHT);
	break;

    case cursorBackward:
        if (((DtTermWidget)w)->vt.td->applicationMode)
	 (void) _DtTermWriteEscSeq(w, ESC_CURSOR_LEFT_APP);
        else
	 (void) _DtTermWriteEscSeq(w, ESC_CURSOR_LEFT);
	break;
    }
    return;
}


void
_DtTermActionTab(Widget w, XEvent *event,
	String *params, Cardinal *num_params)
{
    (void) _DtTermPrimSendInput(w, (unsigned char *) "\t", 1);
    return;
}


/*** FUNCTION KEYS ************************************************************
 *
 *  ######  #    #  #    #   ####    #####     #     ####   #    #
 *  #       #    #  ##   #  #    #     #       #    #    #  ##   #
 *  #####   #    #  # #  #  #          #       #    #    #  # #  #
 *  #       #    #  #  # #  #          #       #    #    #  #  # #
 *  #       #    #  #   ##  #    #     #       #    #    #  #   ##
 *  #        ####   #    #   ####      #       #     ####   #    #
 *
 *
 *  #    #  ######   #   #   ####
 *  #   #   #         # #   #
 *  ####    #####      #     ####
 *  #  #    #          #         #
 *  #   #   #          #    #    #
 *  #    #  ######     #     ####
 */
void
_DtTermActionFunctionKeyExecute(Widget w, XEvent *event,
                                String *params, Cardinal *num_params)
{
  Boolean shift = False;
  long keyNumber;
  char *ret;
  int i;

  /* must have a key number, may have a shift/unshift as well... */
  if (*num_params < 1)
    return;

  /* get a key number... */
  keyNumber = strtol(params[0], &ret, 0);

  /* if we had anything left in the string, the number is bogus... */
  if (*ret)
    return;

  if (*num_params >= 2)
    {
      if (!strcmp(params[1], "UDK"))
        shift = True;
      else if (!strcmp(params[1], "function"))
        shift = False;
      else
        return;
    }

  /* execute the key... */
  _DtTermFunctionKeyExecute(w, (short) keyNumber, shift);

  return;
}


/**************************************************************************
 *
 *  KEYPAD
 *
 */

static char *kpTypes[] =
{ "space", "tab",  "enter", "f1",  "f2", "f3", "f4",
  "equal", "multiply", "add",  "separator", "subtract", "decimal",
  "divide", "0",  "1",  "2", "3", "4", "5", "6",  "7", "8", "9", "help"};

static int no_kptypes=sizeof(kpTypes)/sizeof(char *) ;

void
_DtTermActionKeypadKeyExecute(Widget w, XEvent *event, String *params,
        Cardinal *num_params)
{
    DtTermWidget    tw  = (DtTermWidget)w;
    Boolean shift = False;
    long keyNumber;
    char *ret;
    int type;
    DtTermData  td = tw->vt.td;

    if (KEYBOARD_LOCKED(td->tpd->keyboardLocked)) {
        /* keyboard locked -- ring the bell...
         */
        (void) _DtTermPrimBell(w);
               return ;
     }
    /* must have an edit key type... */
    if (*num_params < 1) {
        return;
    }
#ifdef _AIX
    /*
     * IBM JP kbd specific code. IBM JP kbd requires IM functionality
     * to recoginze NumLock state + (*,/) key
     *
     * The following code is derived from TermPrim/TermPrim.c's
     * _DtTermPrimActionKeyInput()
     *
     */
    {
	XKeyEvent     *keyEvent = (XKeyEvent *)event;
	KeySym        keysym;
	Status        status;
	unsigned char string[BUFSIZ];
        int           nbytes;

	nbytes = XmImMbLookupString(w, keyEvent, (char *) string,
			sizeof(string), &keysym, &status);

	/*
	 * Usually we have to take care of the status. Yet, this function is
	 * called only when KP_XX is pressed. So in this case, ignore any
	 * error case..... Please keep your fingers crossed !
	 */

	/*
	 * Hack code........
	 */
	if ( ( keysym == XK_KP_Add ) || ( keysym == XK_KP_Multiply ) ) {
	    if ( string[0] == '*' )
		params[0] = "multiply";
	    else if ( string[0] == '+' )
		params[0] = "add";
	}
    }
#endif /* _AIX */

    type=0;
    while( strcmp(params[0],kpTypes[type]) && type<= no_kptypes )
      type++;

    if (type > no_kptypes) return ;

    if ( tw->vt.td->applicationKPMode )
      _DtTermWriteEscSeq(w,AppKeypadKey[type]);
    else if ( type == 3 || type == 24)
      {
        w = _DtTermFindHelpViewWidget(w);
        if ( w == NULL)
          w = (Widget) DtCreateTermView(w, "dtTermView", NULL, 0);

        if (w != NULL)
          _DtTermViewMapHelp(w, "Terminal", "_hometopic");
      }
    else
      {
        _DtTermPrimSendInput(w,(unsigned char *)KeypadKey[type],
                             strlen(KeypadKey[type]));
        if ( type == 2 && tw->term.tpd->autoLineFeed )
          _DtTermPrimSendInput(w,(unsigned char *)"\012",1);/* newline */
      }
}

/***********************************************************************
 *
 *  Edit Keys (Find, Insert Here, Remove, Select, Prev Screen, Next Screen)
 *
 */

typedef enum {
    findType,
    insertType,
    selectType,
    priorType,
    nextType,
    deleteType,
    removeType,
    helpType,
    menuType,
    doType,
    MetaXType
} EditType;

static EnumType editTypes[] = {
    "find",     (char) findType,
    "insert", (char) insertType,
    "select",   (char) selectType,
    "next",    (char) nextType,
    "prior",   (char) priorType,
    "delete",  (char) deleteType,
    "remove",  (char) removeType,
    "help",    (char) helpType,
    "menu",    (char) menuType,
    "do",      (char) doType,
    "MetaX",     (char) MetaXType
};

void
_DtTermActionEditKeyExecute(Widget w, XEvent *event, String *params,
        Cardinal *num_params)
{
    DtTermWidget    tw  = (DtTermWidget)w;
    Boolean shift = False;
    long keyNumber;
    char *ret;
    int i;
    DtTermData      td = tw->vt.td;

    if (*num_params < 1) {
        return;
    }

    /* figure out the direction... */
    i = stringToEnum(params[0], editTypes, XtNumber(editTypes));
    if (i < 0) {
        /* no match... */
        return;
    }

    switch( editTypes[i].value) {
            case findType:
                    if ( tw->vt.sunFunctionKeys == False) {
                      (void) _DtTermWriteEscSeq(w, ESC_FIND) ;
                     }
                    else {
                      (void) _DtTermWriteEscSeq(w,ESC_FIND_SUN);
                     }
                    break;
            case insertType:
                    if ( tw->vt.sunFunctionKeys == False) {
                      (void) _DtTermWriteEscSeq(w, ESC_INSERT_HERE) ;
                     }
                    else {
                      (void) _DtTermWriteEscSeq(w,ESC_INSERT_HERE_SUN);
                     }
                    break;
            case selectType:
                    if ( tw->vt.sunFunctionKeys == False) {
                      (void) _DtTermWriteEscSeq(w, ESC_SELECT) ;
                     }
                    else {
                      (void) _DtTermWriteEscSeq(w,ESC_SELECT_SUN);
                     }
                    break;
            case priorType:
                    if ( tw->vt.sunFunctionKeys == False) {
                      (void) _DtTermWriteEscSeq(w, ESC_PREV_SCREEN) ;
                     }
                    else {
                      (void) _DtTermWriteEscSeq(w,ESC_PREV_SCREEN_SUN);
                     }
                    break;
            case nextType:
                    if ( tw->vt.sunFunctionKeys == False) {
                      (void) _DtTermWriteEscSeq(w, ESC_NEXT_SCREEN) ;
                     }
                    else {
                      (void) _DtTermWriteEscSeq(w,ESC_NEXT_SCREEN_SUN);
                     }
                    break;
            case deleteType:
            case removeType:
                    if ( tw->vt.sunFunctionKeys == False) {
                      (void) _DtTermWriteEscSeq(w, ESC_DELETE) ;
                     }
                    else {
                      (void) _DtTermWriteEscSeq(w,ESC_DELETE_SUN);
                     }
                    break;

	    case helpType:
		    if (tw->vt.sunFunctionKeys == False) {
			(void) _DtTermWriteEscSeq(w, ESC_HELP);
		    } else {
			(void) _DtTermWriteEscSeq(w, ESC_HELP_SUN);
		    }
		    break;

	    case menuType:
		    if (tw->vt.sunFunctionKeys == False) {
			(void) _DtTermWriteEscSeq(w, ESC_MENU);
		    } else {
			(void) _DtTermWriteEscSeq(w, ESC_MENU_SUN);
		    }
		    break;

	    case doType:
		    if (tw->vt.sunFunctionKeys == False) {
			(void) _DtTermWriteEscSeq(w, ESC_DO);
		    } else {
			(void) _DtTermWriteEscSeq(w, ESC_DO_SUN);
		    }
		    break;
       }
}

#define STRBUFSIZE 100

void _HandleMetaKeyPressed(Widget w, XEvent *event,
                           String *params, Cardinal *num_params)
{
  DtTermWidget    tw = (DtTermWidget)w;
  DtTermData      td = tw->vt.td;

  char	buf[2];

  /* ---------------------------- */
  /*  Send an Escape -x Sequence  */
  /* ---------------------------- */

  buf[0] = '\033';
  buf[1] = 'x';

  v_write(tw->term.pty, buf, 2);

  return;
}

int pty_mask;
static char *v_buffer;		/* pointer to physical buffer */
static char *v_bufstr = NULL;	/* beginning of area to write */
static char *v_bufptr;		/* end of area to write */
static char *v_bufend;		/* end of physical buffer */
#define	ptymask()	(v_bufptr > v_bufstr ? pty_mask : 0)

/* Write data to the pty as typed by the user, pasted with the mouse,
   or generated by us in response to a query ESC sequence. */

int v_write(int f, char *d, int len)
{
  int riten;
  int c = len;

  if (v_bufstr == NULL  &&  len > 0)
    {
      v_buffer = XtMalloc(len);
      v_bufstr = v_buffer;
      v_bufptr = v_buffer;
      v_bufend = v_buffer + len;
    }

  if ((1 << f) != pty_mask)
    return(write(f, d, len));

  /*
   * Append to the block we already have.
   * Always doing this simplifies the code, and
   * isn't too bad, either.  If this is a short
   * block, it isn't too expensive, and if this is
   * a long block, we won't be able to write it all
   * anyway.
   */

	if (len > 0)
          {
	    if (v_bufend < v_bufptr + len)  /* we've run out of room */
              {
                if (v_bufstr != v_buffer)
                  {
                    /* there is unused space, move everything down */
                    /* possibly overlapping bcopy here */
                    bcopy(v_bufstr, v_buffer, v_bufptr - v_bufstr);
                    v_bufptr -= v_bufstr - v_buffer;
                    v_bufstr = v_buffer;
                  }
		if (v_bufend < v_bufptr + len)
                  {
		    /* still won't fit: get more space */
		    /* Don't use XtRealloc because an error is not fatal. */
		    int size = v_bufptr - v_buffer; /* save across realloc */
		    v_buffer = realloc(v_buffer, size + len);
		    if (v_buffer)
                      {
			v_bufstr = v_buffer;
			v_bufptr = v_buffer + size;
			v_bufend = v_bufptr + len;
                      }
                    else
                      {
			/* no memory: ignore entire write request */
			v_buffer = v_bufstr; /* restore clobbered pointer */
			c = 0;
                      }
                  }
              }

	    if (v_bufend >= v_bufptr + len)
              {
		/* new stuff will fit */
		bcopy(d, v_bufptr, len);
		v_bufptr += len;
              }
          }

	/*
	 * Write out as much of the buffer as we can.
	 * Be careful not to overflow the pty's input silo.
	 * We are conservative here and only write
	 * a small amount at a time.
	 *
	 * If we can't push all the data into the pty yet, we expect write
	 * to return a non-negative number less than the length requested
	 * (if some data written) or -1 and set errno to EAGAIN,
	 * EWOULDBLOCK, or EINTR (if no data written).
	 *
	 * (Not all systems do this, sigh, so the code is actually
	 * a little more forgiving.)
	 */

#define MAX_PTY_WRITE 128	/* 1/2 POSIX minimum MAX_INPUT */

	if (v_bufptr > v_bufstr)
          {
            riten = write(f, v_bufstr, v_bufptr - v_bufstr <= MAX_PTY_WRITE ?
                          v_bufptr - v_bufstr : MAX_PTY_WRITE);
            if (riten < 0)
              riten = 0;
            v_bufstr += riten;
            if (v_bufstr >= v_bufptr) /* we wrote it all */
              v_bufstr = v_bufptr = v_buffer;
          }

	/*
	 * If we have lots of unused memory allocated, return it
	 */

	if (v_bufend - v_bufptr > 1024) /* arbitrary hysteresis */
          {
	    /* save pointers across realloc */
	    int start = v_bufstr - v_buffer;
	    int size = v_bufptr - v_buffer;
	    int allocsize = size ? size : 1;

	    v_buffer = realloc(v_buffer, allocsize);
	    if (v_buffer)
              {
                v_bufstr = v_buffer + start;
                v_bufptr = v_buffer + size;
		v_bufend = v_buffer + allocsize;
              }
            else
              /* should we print a warning if couldn't return memory? */
              v_buffer = v_bufstr - start; /* restore clobbered pointer */
          }
	return(c);
}



Widget _DtTermFindHelpViewWidget(Widget w)
{
  char *name;
  int i, num_children;
  WidgetList children;

  /* ---------------------------------------------------------------- */
  /*  Get to the dtTermView widget through the toplevel shell Parent  */
  /* ---------------------------------------------------------------- */

  while(!XtIsTopLevelShell(w)) w = XtParent(w);

  XtVaGetValues(w,
                XmNchildren,    &children,
                XmNnumChildren, &num_children,
                NULL);

  for ( i = 0 ; i < num_children ; i++ )
    {
      name = XtName(children[i]);
      if(strcmp(name, "dtTermView") == 0)
        return(children[i]);
    }

  return(NULL);
}
