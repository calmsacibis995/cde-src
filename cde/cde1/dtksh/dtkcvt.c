/* $XConsortium: dtkcvt.c /main/cde1_maint/2 1995/10/13 10:08:47 drk $ */

/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF          */
/*	UNIX System Laboratories, Inc.			        */
/*	The copyright notice above does not evidence any        */
/*	actual or intended publication of such source code.     */

/* X includes */

#include	"shell.h" 
#include <signal.h>
#include <fcntl.h>
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/StringDefs.h>
#include <Xm/XmStrDefs.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <Dt/Service.h>
#include <Dt/Wsm.h>
#include <Dt/HourGlass.h>
#include <Dt/Help.h>
#include <Dt/EnvControl.h>
#include "hash.h"
#include "stdio.h"
#define NO_AST
#include "dtksh.h"
#undef NO_AST
#include "xmksh.h"
#include "XtCvtrs.h"
#include "dtkcmds.h"
#include "xmcvt.h"
#include "widget.h"
#include "extra.h"
#include "xmwidgets.h"
#include "msgs.h"

#if defined(SYSV) || defined(SVR4_0) || defined(SVR4) || defined(DEC)
#define lsprintf sprintf
#endif

extern Hashtab_t * Wclasses;


#ifdef _NO_PROTO 

static void CvtMWMValueToString();
static void CvtStringToMWMValue( );

#else

static void CvtStringToMWMValue( 
        MWMTable * table, 
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data ) ;
static void CvtMWMValueToString(
	MWMTable * table,
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data ) ;
#endif

/*
 * Converters for dtksh
 */

void
#ifdef _NO_PROTO
DtkshCvtWindowToString( args, nargs, fval, toval )
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
#else
DtkshCvtWindowToString(
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval )
#endif /* _NO_PROTO */
{
   static char result[16];
   Window window;
   char * errmsg;

   if (fval->size != sizeof(Window))
   {
      errmsg = strdup(GETMESSAGE(6,1, 
            "DtkshCvtWindowToString: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   window = ((Window *)(fval->addr))[0];
   if (window == None)
      sprintf(result, "None");
   else
      sprintf(result, "0x%x", window);
   toval->addr = result;
   toval->size = strlen(result)+1;
}

void
#ifdef _NO_PROTO
DtkshCvtScreenToString( args, nargs, fval, toval )
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
#else
DtkshCvtScreenToString(
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval )
#endif /* _NO_PROTO */
{
   static char result[16];
   Screen * screen;
   char * errmsg;

   if (fval->size != sizeof(Screen *))
   {
      errmsg = strdup(GETMESSAGE(6,14, 
            "DtkshCvtScreenToString: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   screen = ((Screen **)(fval->addr))[0];
   sprintf(result, "0x%x", screen);
   toval->addr = result;
   toval->size = strlen(result)+1;
}

void
#ifdef _NO_PROTO
DtkshCvtStringToScreen( args, nargs, fval, toval )
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
#else
DtkshCvtStringToScreen(
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval )
#endif /* _NO_PROTO */
{
   static Screen * screen;
   char * errmsg;
   char * p;

   if (fval->size <= 0 || fval->addr == NULL) 
   {
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
 
   screen = (Screen *)strtoul(fval->addr, &p, 0);
   if (p != fval->addr)
   {
      toval->addr = (XtPointer)&screen;
      toval->size = sizeof(Screen *);
   }
   else
   {
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
}

void
#ifdef _NO_PROTO
DtkshCvtStringToTopItemPosition( args, nargs, fval, toval )
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
#else
DtkshCvtStringToTopItemPosition(
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval )
#endif /* _NO_PROTO */
{
   static int topItemPosition;
   char * p;

   if (fval->size <= 0 || fval->addr == NULL) 
   {
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
 
   topItemPosition = (int)strtoul(fval->addr, &p, 0);
   if (p != fval->addr)
   {
      toval->addr = (XtPointer)&topItemPosition;
      toval->size = sizeof(int);
   }
   else
   {
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
}

void
#ifdef _NO_PROTO
DtkshCvtHexIntToString( args, nargs, fval, toval )
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
#else
DtkshCvtHexIntToString(
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval )
#endif /* _NO_PROTO */
{
   static char result[16];
   char * errmsg;

   if (fval->size != sizeof(long) && fval->size != sizeof(int) && 
       fval->size != sizeof(short) && fval->size != sizeof(char)) 
   {
      errmsg = strdup(GETMESSAGE(6,2, 
           "DtkshCvtHexIntToString: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   if (fval->size == sizeof(long))
      sprintf(result, "0x%lx", ((long *)(fval->addr))[0]);
   else if (fval->size == sizeof(int))
      sprintf(result, "0x%x", ((int *)(fval->addr))[0]);
   else if (fval->size == sizeof(short))
      sprintf(result, "0x%x", (int)(((short *)(fval->addr))[0]));
   else if (fval->size == sizeof(char))
      sprintf(result, "0x%x", (int)(((char *)(fval->addr))[0]));
   toval->addr = result;
   toval->size = strlen(result)+1;
}

void
#ifdef _NO_PROTO
DtkshCvtIntToString( args, nargs, fval, toval )
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
#else
DtkshCvtIntToString(
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval )
#endif /* _NO_PROTO */
{
   static char result[16];
   char * errmsg;

   if (fval->size != sizeof(int) && fval->size != sizeof(short)) 
   {
      errmsg = strdup(GETMESSAGE(6,3, 
             "DtkshCvtIntToString: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   if (fval->size == sizeof(int))
      sprintf(result, "%d", ((int *)(fval->addr))[0]);
   else
      sprintf(result, "%d", (int)(((short *)(fval->addr))[0]));
   toval->addr = result;
   toval->size = strlen(result)+1;
}

void
#ifdef _NO_PROTO
DtkshCvtBooleanToString( args, nargs, fval, toval )
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
#else
DtkshCvtBooleanToString(
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval )
#endif /* _NO_PROTO */
{
   char * errmsg;
   Boolean booleanState;

   if (fval->size != sizeof(int) && fval->size != sizeof(short) &&
       fval->size != sizeof(char)) 
   {
      errmsg = strdup(GETMESSAGE(6,4, 
             "DtkshCvtBooleanToString: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }

   if (fval->size == sizeof(int))
      booleanState = (Boolean) (((int *)(fval->addr))[0]);
   else if (fval->size == sizeof(short))
      booleanState =  (Boolean) (((short *)(fval->addr))[0]);
   else if (fval->size == sizeof(char))
      booleanState = (Boolean) (((char *)(fval->addr))[0]);

   if (booleanState)
      toval->addr = (caddr_t)("true");
   else
      toval->addr = (caddr_t)("false");

   toval->size = strlen(toval->addr)+1;
}


void
#ifdef _NO_PROTO
DtkshCvtStringToPointer( args, nargs, fval, toval )
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
#else
DtkshCvtStringToPointer(
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval )
#endif /* _NO_PROTO */
{
   static XtPointer ret;

   if (fval->size <= 0 || fval->addr == NULL) 
   {
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   ret = (XtPointer)strdup(fval->addr);
   toval->addr = (XtPointer)&ret;
   toval->size = sizeof(XtPointer);
   return;
}

void
#ifdef _NO_PROTO
DtkshCvtStringToWidget( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtStringToWidget(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   char *wname;
   static Widget wid;
   wtab_t *w;
   char * errmsg;

   if (fval->size <= 0) 
   {
      errmsg = strdup(GETMESSAGE(6,5, 
             "DtkshCvtStringToWidget: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   wname = (char *)fval->addr;
   if (wname == NULL || wname[0] == '\0' || strcmp(wname, "NULL") == 0) 
   {
      static Widget NullWidget = NULL;

      toval->addr = (XtPointer)&NullWidget;
      toval->size = sizeof(Widget);
      return;
   }
   if ((w = str_to_wtab("DtkshCvtStringToWidget", wname)) != NULL) 
   {
      wid= w->w;
      toval->addr = (XtPointer)&wid;
      toval->size = sizeof(Widget);
      return;
   }
   /*
    * If we couldn't find it in our table, try looking up the
    * name in standard resource format.
    */
   if ((wid = DtkshNameToWidget(wname)) != NULL) 
   {
      toval->addr = (XtPointer)&wid;
      toval->size = sizeof(Widget);
      return;
   }
   /*
    * We failed completely
    */
   {
      char errbuf[1024];

      errmsg = strdup(GETMESSAGE(6,6, 
               "DtkshCvtStringToWidget: Unable to find a widget named '%s'"));
      sprintf(errbuf, errmsg, wname);
      XtWarning(errbuf);
      free(errmsg);
   }
   toval->addr = NULL;
   toval->size = 0;
}

void
#ifdef _NO_PROTO
DtkshCvtStringToCallback( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtStringToCallback(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   static XtCallbackList cb;
   dtksh_client_data_t *cdata;
   classtab_t *c = DTKSHConversionClass;
   wtab_t *w = DTKSHConversionWidget;
   char * errmsg;

   if (fval->size <= 0) 
   {
      errmsg = strdup(GETMESSAGE(6,7, 
             "DtkshCvtStringToCallback: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }

   cb = (XtCallbackList)XtMalloc(sizeof(XtCallbackRec)*2);
   cb[0].callback = (XtCallbackProc)stdCB;

   cdata = GetNewCBData((String)fval->addr, w, DTKSHConversionResource, NULL);

   cb[0].closure = (caddr_t)cdata;
   cb[1].callback = NULL;
   toval->addr = (XtPointer)&cb;
   toval->size = sizeof(XtCallbackList);
}

void
#ifdef _NO_PROTO
DtkshCvtCallbackToString( display, args, nargs, fval, toval, converterData )
        Display *display ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer converterData ;
#else
DtkshCvtCallbackToString(
        Display *display,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer converterData )
#endif /* _NO_PROTO */
{
   XtCallbackList cb;
   char buf[2048];
   register char *p;
   char * errmsg;
   Boolean count = 0;

   if (fval->size != sizeof(XtCallbackList)) 
   {
      errmsg = strdup(GETMESSAGE(6,8, 
          "DtkshCvtCallbackToString: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   if (fval->addr == NULL) 
   {
      toval->addr = ": ;";
      toval->size = 1;
      return;
   }
   p = &buf[0];
   *p = '\0';
   for (cb = ((XtCallbackList *)(fval->addr))[0]; cb->callback != NULL; cb++) 
   {
      if (cb->callback == (XtCallbackProc)stdCB) 
      {
         dtksh_client_data_t *cdata = (dtksh_client_data_t *)cb->closure;

         if (p + strlen((String)cdata->ksh_cmd) + 1 - buf > sizeof(buf)) 
         {
            errmsg = strdup(GetSharedMsg(DT_CONV_BUF_OVFL));
            XtWarning(errmsg);
            free(errmsg);
            break;
         }
         if (count > 0)
            p += lsprintf(p, "|%s", (String)cdata->ksh_cmd);
         else
            p += lsprintf(p, "%s", (String)cdata->ksh_cmd);
         count++;
      }
   }
   toval->addr = (XtPointer)strdup(buf);
   toval->size = strlen(buf) + 1;
}


void
#ifdef _NO_PROTO
DtkshCvtWidgetToString( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtWidgetToString(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   char *wname;
   Widget widget;
   wtab_t *w;
   char * errmsg;

   if (fval->size != sizeof(Widget) || fval->addr == NULL) 
   {
      errmsg = strdup(GETMESSAGE(6,9, 
           "DtkshCvtWidgetToString: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   widget = ((Widget *)fval->addr)[0];
   if (widget == NULL) 
   {
      toval->addr = (XtPointer)("NULL");
      toval->size = 5;
      return;
   }
   if ((w = widget_to_wtab(widget)) == NULL) 
   {
      errmsg = strdup(GETMESSAGE(6,10, 
          "DtkshCvtWidgetToString: Unable to find a name for the widget"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   toval->addr = (XtPointer)w->widid;
   toval->size = strlen(w->widid) + 1;
}


/***********************************************/

static EventMaskTable eventMasks[] = {
   {"XtAllEvents", XtAllEvents},
   {"NoEventMask", NoEventMask},
   {"KeyPressMask", KeyPressMask},
   {"KeyReleaseMask", KeyReleaseMask},
   {"ButtonPressMask", ButtonPressMask},
   {"ButtonReleaseMask", ButtonReleaseMask},
   {"EnterWindowMask", EnterWindowMask},
   {"LeaveWindowMask", LeaveWindowMask},
   {"PointerMotionMask", PointerMotionMask},
   {"PointerMotionHintMask", PointerMotionHintMask},
   {"Button1MotionMask", Button1MotionMask},
   {"Button2MotionMask", Button2MotionMask},
   {"Button3MotionMask", Button3MotionMask},
   {"Button4MotionMask", Button4MotionMask},
   {"Button5MotionMask", Button5MotionMask},
   {"ButtonMotionMask", ButtonMotionMask},
   {"KeymapStateMask", KeymapStateMask},
   {"ExposureMask", ExposureMask},
   {"VisibilityChangeMask", VisibilityChangeMask},
   {"StructureNotifyMask", StructureNotifyMask},
   {"ResizeRedirectMask", ResizeRedirectMask},
   {"SubstructureNotifyMask", SubstructureNotifyMask},
   {"SubstructureRedirectMask", SubstructureRedirectMask},
   {"FocusChangeMask", FocusChangeMask},
   {"PropertyChangeMask", PropertyChangeMask},
   {"ColormapChangeMask", ColormapChangeMask},
   {"OwnerGrabButtonMask", OwnerGrabButtonMask},
   {NULL, NULL},
};


void
#ifdef _NO_PROTO
DtkshCvtStringToEventMask( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtStringToEventMask(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   static EventMask eventMask = 0;
   int i;
   char * ptr;
   char * eventMaskString;

   toval->addr = (XtPointer)&eventMask;
   toval->size = sizeof(EventMask);

   if (fval->size <= 0 || fval->addr == NULL) 
      return;

   ptr = eventMaskString = strdup(fval->addr);

   /*  Skip any leading whitespace.  */
   while (isspace(*ptr) && (*ptr != '\0'))
      ptr++;

   eventMask = 0;
   ptr = strtok(ptr, "|");
   while (ptr)
   {
      for (i = 0; eventMasks[i].name; i++)
      {
         if (DtCompareISOLatin1(ptr, eventMasks[i].name))
         {
            eventMask |= eventMasks[i].mask;
            break;
         }
      }
      ptr = strtok(NULL, "|");
   }

   XtFree(eventMaskString);
}


void
#ifdef _NO_PROTO
DtkshCvtStringToListItems( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtStringToListItems(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   XtConvert(Toplevel, XtRString, fval, XmRXmStringTable, toval);
}


void
#ifdef _NO_PROTO
DtkshCvtWidgetClassToString( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtWidgetClassToString(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   static char result[16];
   char * errmsg;
   WidgetClass wc;
   int i;

   if (fval->size != sizeof(WidgetClass))
   {
      errmsg = strdup(GETMESSAGE(6,11, 
           "DtkshCvtWidgetClassToString: The 'from' value is an invalid size"));
      XtWarning(errmsg);
      free(errmsg);
      toval->addr = NULL;
      toval->size = 0;
      return;
   }

   if ((wc = ((WidgetClass *)(fval->addr))[0]) == NULL)
   {
      toval->addr = "";
      toval->size = 1;
      return;
   }

   for (i = 0; C[i].cname != NULL; i++)
   {
      if (C[i].class == wc)
      {
         toval->addr = C[i].cname;
         toval->size = strlen(C[i].cname)+1;
         return;
      }
   }

   /* No match found */
   errmsg = strdup(GETMESSAGE(6,12,
          "DtkshCvtWidgetClassToString: Unknown widget class"));
   XtWarning(errmsg);
   free(errmsg);
   toval->addr = NULL;
   toval->size = 0;
   return;
}

void
#ifdef _NO_PROTO
DtkshCvtStringToWidgetClass( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtStringToWidgetClass(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   static char result[16];
   char * errmsg;
   static WidgetClass wc;
   int i;
   char * wcName;
   char * hashInfo;
   classtab_t * classtab;

   if (fval->size <= 0 || fval->addr == NULL) 
   {
      toval->addr = NULL;
      toval->size = 0;
      return;
   }

   wcName = (char *)(fval->addr);
   if ((hashInfo = hashget(Wclasses, wcName)) != NULL)
   {
      classtab = (classtab_t *)hashInfo;
      wc = classtab->class;
      toval->addr = (caddr_t)&wc;
      toval->size = sizeof(WidgetClass);
      return;
   }

   /* No match found */
   errmsg = strdup(GETMESSAGE(6,13,
          "DtkshCvtStringToWidgetClass: Unknown widget class name"));
   XtWarning(errmsg);
   free(errmsg);
   toval->addr = NULL;
   toval->size = 0;
   return;
}


static MWMTable mwmDecorations[] = {
   {"MWM_DECOR_ALL", MWM_DECOR_ALL},
   {"MWM_DECOR_BORDER", MWM_DECOR_BORDER},
   {"MWM_DECOR_RESIZEH", MWM_DECOR_RESIZEH},
   {"MWM_DECOR_TITLE", MWM_DECOR_TITLE},
   {"MWM_DECOR_MENU", MWM_DECOR_MENU},
   {"MWM_DECOR_MINIMIZE", MWM_DECOR_MINIMIZE},
   {"MWM_DECOR_MAXIMIZE", MWM_DECOR_MAXIMIZE},
   {NULL, NULL},
};

static MWMTable mwmFunctions[] = {
   {"MWM_FUNC_ALL", MWM_FUNC_ALL},
   {"MWM_FUNC_RESIZE", MWM_FUNC_RESIZE},
   {"MWM_FUNC_MOVE", MWM_FUNC_MOVE},
   {"MWM_FUNC_MINIMIZE", MWM_FUNC_MINIMIZE},
   {"MWM_FUNC_MAXIMIZE", MWM_FUNC_MAXIMIZE},
   {"MWM_FUNC_CLOSE", MWM_FUNC_CLOSE},
   {NULL, NULL},
};

static void
#ifdef _NO_PROTO
CvtStringToMWMValue( table, dpy, args, nargs, fval, toval, data )
	MWMTable * table ;
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
CvtStringToMWMValue(
	MWMTable * table,
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   static long value;
   int i;
   char * ptr;
   char * valueString;
   char * p;
   int intVal;

   value = 0;
   toval->addr = (XtPointer)&value;
   toval->size = sizeof(int);

   if (fval->size <= 0 || fval->addr == NULL) 
      return;

   ptr = valueString = strdup(fval->addr);

   /*  Skip any leading whitespace.  */
   while (isspace(*ptr) && (*ptr != '\0'))
      ptr++;

   /* Integer values, especially -1, are also supported */
   intVal = strtol(ptr, &p, 0);
   if (p != ptr)
   {
      value = intVal;
      XtFree(valueString);
      return;
   }

   ptr = strtok(ptr, "|");
   while (ptr)
   {
      for (i = 0; table[i].name; i++)
      {
         if (DtCompareISOLatin1(ptr, table[i].name))
         {
            value |= table[i].value;
            break;
         }
      }
      ptr = strtok(NULL, "|");
   }

   XtFree(valueString);
}


static void
#ifdef _NO_PROTO
CvtMWMValueToString( table, dpy, args, nargs, fval, toval, data )
	MWMTable * table ;
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
CvtMWMValueToString(
	MWMTable * table,
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   static char *string = NULL;
   int value;
   int i = 0;
   Boolean firstOne = True;

   value = ((int *)(fval->addr))[0];

   if (value == -1)
   {
      string = XtRealloc(string, 5);
      strcpy(string, "-1");
   }
   else
   {
      string = XtRealloc(string, 1);
      string[0] = '\0';

      while (table[i].name)
      {
         if (value & table[i].value)
         {
            if (!firstOne)
            {
               string = XtRealloc(string,
                     strlen(string) + strlen(table[i].name) + 2);
               strcat(string, "|");
               strcat(string, table[i].name);
            }
            else
            {
               firstOne = False;
               string = XtRealloc(string,
                     strlen(string) + strlen(table[i].name) + 1);
               strcat(string, table[i].name);
            }
         }
         i++;
      }
   }

   toval->addr = (caddr_t)string;
   toval->size = strlen(string) + 1;
}

void
#ifdef _NO_PROTO
DtkshCvtStringToMWMDecoration( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtStringToMWMDecoration(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   CvtStringToMWMValue(mwmDecorations, dpy, args, nargs, fval, toval, data);
}


void
#ifdef _NO_PROTO
DtkshCvtMWMDecorationToString( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtMWMDecorationToString(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   CvtMWMValueToString(mwmDecorations, dpy, args, nargs, fval, toval, data);
}

void
#ifdef _NO_PROTO
DtkshCvtStringToMWMFunctions( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtStringToMWMFunctions(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   CvtStringToMWMValue(mwmFunctions, dpy, args, nargs, fval, toval, data);
}


void
#ifdef _NO_PROTO
DtkshCvtMWMFunctionsToString( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtMWMFunctionsToString(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   CvtMWMValueToString(mwmFunctions, dpy, args, nargs, fval, toval, data);
}

void
#ifdef _NO_PROTO
DtkshCvtStringToPanedWinPosIndex ( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtStringToPanedWinPosIndex(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   static short value;
   char * pos;
   char * p;

   if (fval->size <= 0 || fval->addr == NULL) 
   {
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
 
   pos = (char *)fval->addr;
   if (DtCompareISOLatin1(pos, "LAST_POSITION"))
      value = XmLAST_POSITION;
   else
      value = strtol(pos, &p, 0);

   toval->addr = (XtPointer)&value;
   toval->size = sizeof(short);
}

void
#ifdef _NO_PROTO
DtkshCvtPanedWinPosIndexToString ( dpy, args, nargs, fval, toval, data )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *nargs ;
        XrmValuePtr fval ;
        XrmValuePtr toval ;
        XtPointer data ;
#else
DtkshCvtPanedWinPosIndexToString(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *nargs,
        XrmValuePtr fval,
        XrmValuePtr toval,
        XtPointer data )
#endif /* _NO_PROTO */
{
   static char result[50];
   short value;

   if (fval->size != sizeof(short))
   {
      toval->addr = NULL;
      toval->size = 0;
      return;
   }
   value = ((short *)(fval->addr))[0];
   if (value == XmLAST_POSITION)
      sprintf(result, "LAST_POSITION");
   else
      sprintf(result, "%d", value);
   toval->addr = result;
   toval->size = strlen(result)+1;
}
