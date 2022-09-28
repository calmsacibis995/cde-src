#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermViewMenu.c /main/cde1_maint/2 1995/09/29 12:09:48 lehors $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#define	PULLDOWN_ACCELERATORS
#define WINDOW_SIZE_TOGGLES

#include "TermHeader.h"
#include <string.h>		/* for strdup				*/
#include <errno.h>		/* for errno and sys_errlist[]		*/

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Xm/SelectioB.h>
#include <X11/keysym.h>
#include "TermViewMenu.h"
#include "TermViewP.h"
#include "TermPrimFunction.h"
#include "TermViewGlobalDialog.h"
#include "TermViewTerminalDialog.h"
#include "TermPrimMessageCatI.h"

/* widget for current menu context */
static Widget currentWidget = (Widget ) 0;
static Widget currentTermWidget = (Widget ) 0;

static Widget scrollBarToggle;
static Widget menuBarToggle;
static Widget *fontSizeToggles;
int fontSizeTogglesDefault = -1;

int     savedSetFontIndex  = 0;
Boolean needToSetFontIndex = 0;

#ifdef	WINDOW_SIZE_TOGGLES
static Widget *windowSizeToggles;
extern int windowSizeTogglesDefault = -1;
/*extern int currentWindowToggleButtonIndex = -1; */
#endif	/* WINDOW_SIZE_TOGGLES */
static Widget newButton;
#define	numPulldowns	5
static Widget pulldown[numPulldowns];
static Widget cascade[numPulldowns];
static int optionsPC;
static int editPC;
static int windowPC;

/* forward declarations... */
static void makeFontArrayFromTermView(Widget w);
static Widget CreateMenu(Widget termView, Widget parent, Boolean menuBar,
	Arg menuArglist[], int menuArgcount);
static void setContext(Widget w, XtPointer client_data, XtPointer call_data);
static void exitCallback(Widget w, XtPointer client_data, XtPointer call_data);
static void scrollBarToggleCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void menuBarToggleCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void cloneCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
#ifdef	HPVUE
static void helpVersionCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpIntroCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpTasksCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpReferenceCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpOnHelpCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
#else	/* HPVUE */
static void helpOverviewCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpTableOfContentsCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpTasksCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpReferenceCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpKeyboardCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpUsingHelpCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void helpAboutDttermCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
#endif	/* HPVUE */
static void globalOptionsCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void terminalOptionsCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void sizeChangeCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void defaultSizeCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void fontChangeCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void defaultFontCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void softResetCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void hardResetCallback(Widget w, XtPointer client_data,
	XtPointer call_data);
static void copyClipboardCallback(Widget w, XtPointer client_data,
        XtPointer call_data);
static void pasteClipboardCallback(Widget w, XtPointer client_data,
        XtPointer call_data);

static void
setCurrentWidgets(Widget w, XtPointer client_data, XEvent *xe, Boolean *cont);

/**************************************************************************/
#ifdef	TOKEN_CALLBACK
static void
tokenCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (client_data) {
	 printf("%s: button pressed\n", (char *) client_data);
    }
}
#endif	/* TOKEN_CALLBACK */

static void
setCurrentWidgets(Widget w, XtPointer client_data, XEvent *xe, Boolean *cont)
{
  currentWidget = (Widget) client_data;
  currentTermWidget = w;
  return;
}

static void
postPopupMenu(Widget w, XtPointer client_data, XEvent *event, Boolean *cont)
{
    Widget popupMenu = (Widget) client_data;

    if (event->xbutton.button == 3) {
      /* Update term for setContext   */
      currentTermWidget = w;
      /* now position and manage the menu... */
      XmMenuPosition(popupMenu, &event->xbutton);
      XtManageChild(popupMenu);
    }
}

static void
postMenuBar(Widget w, XtPointer client_data, XEvent *event, Boolean *cont)
{
  DtTermViewWidget tw = (DtTermViewWidget) client_data;

  /* Update term for setContext   */
  if (event->xbutton.button == 1)
    currentTermWidget = tw->termview.term;
}

static Widget
createMenuWidget(WidgetClass widgetClass, char *callbackName,
	Widget parent, Widget subMenuId, char *label, KeySym mnemonic,
	char *accelerator, char *acceleratorText,
	XtCallbackProc callback, XtPointer clientData)
{
    Widget w;
    XmString string;
    XmString acceleratorString = (XmString) 0;
    Arg arglist[20];
    int i;

    i = 0;

    /* create the label string and stuff it... */
    string = XmStringCreateLtoR(label, XmFONTLIST_DEFAULT_TAG);
     XtSetArg(arglist[i], XmNlabelString, string); i++;

    /* if a mnemonic was specified, set the mnemonic and mnemonic charset... */
    if (mnemonic != NoSymbol) {
	 XtSetArg(arglist[i], XmNmnemonic, mnemonic); i++;
	 XtSetArg(arglist[i], XmNmnemonicCharSet, XmFONTLIST_DEFAULT_TAG);
		i++;
    }

    /* if an accelerator was specified, stuff it... */
    if (accelerator && *accelerator) {
	XtSetArg(arglist[i], XmNaccelerator, accelerator); i++;
    }

    /* if acceleratorText was specified, create the XmString and stuff it... */
    if (acceleratorText && *acceleratorText) {
	acceleratorString = XmStringCreateLtoR(acceleratorText,
		XmFONTLIST_DEFAULT_TAG);
	XtSetArg(arglist[i], XmNacceleratorText, acceleratorString); i++;
    }

    /* if subMenuId was specified, stuff it... */
    if (subMenuId) {
	XtSetArg(arglist[i], XmNsubMenuId, subMenuId); i++;
    }

    /* create the widget... */
    w = XtCreateManagedWidget(label, widgetClass, parent, arglist, i);
    if (callback) {
	XtAddCallback(w, callbackName, callback, clientData);
    }

#ifdef	TOKEN_CALLBACK
    /*DKS -- this is to help debug the menu stuff... */
    if (!strcmp(callbackName, XmNactivateCallback)) {
	XtAddCallback(w, callbackName, tokenCallback, strdup(label));
    }
#endif	/* TOKEN_CALLBACK */

    /* free up storage... */
    XmStringFree(string);
    if (acceleratorString)
	XmStringFree(acceleratorString);

    /* return the widget... */
    return(w);
}

Widget
_DtTermViewCreateCascadeButton(Widget parent, Widget subMenuId, char *label,
	KeySym mnemonic, char *accelerator, char *acceleratorText,
	XtCallbackProc callback, XtPointer clientData)
{
    return(createMenuWidget(xmCascadeButtonGadgetClass, XmNactivateCallback,
	parent, subMenuId, label, mnemonic, accelerator, acceleratorText,
	callback, clientData));
}

Widget
_DtTermViewCreatePushButton(Widget parent, char *label, KeySym mnemonic,
	char *accelerator, char *acceleratorText,
	XtCallbackProc callback, XtPointer clientData)
{
    return(createMenuWidget(xmPushButtonGadgetClass, XmNactivateCallback,
	parent, NULL, label, mnemonic, accelerator, acceleratorText,
	callback, clientData));
}

Widget
_DtTermViewCreateToggleButton(Widget parent, char *label, KeySym mnemonic,
	char *accelerator, char *acceleratorText,
	XtCallbackProc callback, XtPointer clientData)
{
    return(createMenuWidget(xmToggleButtonGadgetClass, XmNvalueChangedCallback,
	parent, NULL, label, mnemonic, accelerator, acceleratorText,
	callback, clientData));
}

Widget
_DtTermViewCreateLabel(Widget parent, char *label)
{
    return(createMenuWidget(xmLabelGadgetClass, NULL,
	parent, NULL, label, NULL, NULL, NULL, NULL, NULL));
}

Widget
_DtTermViewCreateSeparator(Widget parent, char *label)
{
    return(createMenuWidget(xmSeparatorGadgetClass, NULL,
	parent, NULL, label, NULL, NULL, NULL, NULL, NULL));
}

static Widget
createPulldown(Widget parent, char *name, Arg *arglist, int argcnt)
{
    Widget w;

    w = XmCreatePulldownMenu(parent, name, arglist, argcnt);
    XtAddCallback(w, XmNmapCallback, setContext, (XtPointer) NULL);
    return(w);
}

static char *
mallocGETMESSAGE(int msgset, int msg, char *defaultString)
{
    char *c1;
    char *c2;

    c2 = GETMESSAGE(msgset, msg, defaultString);
    c1 = XtMalloc(strlen(c2) + 1);
    strcpy(c1, c2);
    return(c1);
}

static void
createSizeMenu
(
    Widget		  w,
    Widget		  menu
)
{
    DtTermViewWidget	  tw = (DtTermViewWidget) w;
    Widget		  submenu;
    Widget		  button;
    int			  i1;
    char		 *c1;
    char		 *c2;
#ifdef	NOTDEF
    char		  mnemonics[BUFSIZ];
#endif	/* NOTDEF */
    char		  buffer[BUFSIZ];
    KeySym		  ks;
    Arg			  al[20];
    int			  ac;

    ac = 0;
    XtSetArg(al[ac], XmNradioBehavior, True); ac++;
    submenu = createPulldown(menu, "Window Size", al, ac);

#ifdef	NOTDEF
    /* clear out mnemonics string... */
    *mnemonics = '\0';
    c1 = mnemonics;
#endif	/* NOTDEF */

    /* create the size buttons... */
#ifdef	WINDOW_SIZE_TOGGLES
    windowSizeToggles = (Widget *)
	    XtMalloc((1 + tw->termview.sizeList.numSizes) *sizeof(Widget));
#endif	/* WINDOW_SIZE_TOGGLES */
    for (i1 = 0; i1 < tw->termview.sizeList.numSizes; i1++) {
	*buffer = '\0';
	if (tw->termview.sizeList.sizes[i1].columns > 0) {
	    sprintf(buffer + strlen(buffer), "%hd",
		    tw->termview.sizeList.sizes[i1].columns);
	}
	strcat(buffer, "x");
	if (tw->termview.sizeList.sizes[i1].rows > 0) {
	    sprintf(buffer + strlen(buffer), "%hd",
		    tw->termview.sizeList.sizes[i1].rows);
	}
#ifdef NOTDEF
	for (c2 = buffer; *c2; c2++) {
	    if (!strchr(mnemonics, *c2) && !isspace(*c2)) {
		break;
	    }
	}
	if (*c2) {
	    /* add it to the mnemonics list... */
	    *c1++ = *c2;
	    ks = XK_A + *c2 - 'A';
	} else {
	    ks = NULL;
	}
#endif
#ifdef	WINDOW_SIZE_TOGGLES
	windowSizeToggles[i1] = _DtTermViewCreateToggleButton(submenu, buffer,
		NULL, NULL, NULL, sizeChangeCallback, (XtPointer) i1);
	ac = 0;
	XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
	XtSetValues(windowSizeToggles[i1], al, ac);
#else	/* WINDOW_SIZE_TOGGLES */
	button = _DtTermViewCreatePushButton(submenu, buffer,
		NULL, NULL, NULL, sizeChangeCallback, (XtPointer) i1);
	ac = 0;
	XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
	XtSetValues(button, al, ac);
#endif	/* WINDOW_SIZE_TOGGLES */

    }

    /* get a mnemonic for "Default"... */
    strcpy(buffer, (GETMESSAGE(NL_SETN_ViewMenu,1, "Default")));
#ifdef NOTDEF
    for (c2 = buffer; *c2; c2++) {
	if (!strchr(mnemonics, *c2) && !isspace(*c2))
	    break;
    }
    if (*c2) {
	/* add it to the mnemonics list... */
	*c1++ = *c2;
	ks = XK_A + *c2 - 'A';
    } else {
	ks = NULL;
    }
#endif
#ifdef	WINDOW_SIZE_TOGGLES
    windowSizeToggles[i1] = _DtTermViewCreateToggleButton(submenu, buffer,
	    NULL, NULL, NULL, defaultSizeCallback, NULL);
	ac = 0;

	XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
	XtSetValues(windowSizeToggles[i1], al, ac);
        windowSizeTogglesDefault = i1;
#else	/* WINDOW_SIZE_TOGGLES */
    button = _DtTermViewCreatePushButton(submenu, buffer,
	    NULL, NULL, NULL, defaultSizeCallback, NULL);
	ac = 0;
	XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
	XtSetValues(button, al, ac);
#endif	/* WINDOW_SIZE_TOGGLES */

    ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,3, "W"));
    _DtTermViewCreateCascadeButton(menu,
	    submenu, (GETMESSAGE(NL_SETN_ViewMenu,2, "Window Size")),
	              ks,
		      NULL, NULL, NULL, NULL);
}

extern char _DtTermViewMenuDefaultFonts[];

typedef struct _fontArrayType
  {
      char *labelName;
      char *fontName;
      XmFontList fontList;
  } fontArrayType;

static fontArrayType *fontArray = (fontArrayType *) 0;
static short fontArrayCount = 0;

static void makeFontArrayFromTermView(Widget w)

{
  char  *c1, *c2, **fontNames;
  int    i1, i2, ac, dpi;
  char   mnemonics[BUFSIZ], buffer[BUFSIZ], fontName[BUFSIZ];
  float  pointSize;
  Arg    al[20];
  KeySym ks;
  XFontSet      fontSet;
  XFontStruct   **fonts;
  char          **miss_char_list;
  char          *miss_string;
  int           miss_count, num_fonts;
  int           preFontHeight, curFontHeight;

  DtTermViewWidget tw = (DtTermViewWidget) w;

  /* calculate dots per inch... */
  dpi = HeightOfScreen(XtScreen(w)) / (HeightMMOfScreen(XtScreen(w)) / 25.4);

  if (!tw->termview.userFontList || !*tw->termview.userFontList)
    tw->termview.userFontList = _DtTermViewMenuDefaultFonts;

  /* find out how many newlines there are in the userFontList so
   * that we can build an array big enough to hold them... */

  for (i1 = 1, c1 = tw->termview.userFontList; *c1; )
    if (*c1++ == '\n')
      i1++;

  fontArray = (fontArrayType *) XtMalloc(sizeof(fontArrayType) * i1);
  for (i1 = 0, c1 = tw->termview.userFontList; *c1; )
    {
      /* copy over the userFontList up to the end, or a newline... */
      for (c2 = buffer; *c1 && (*c1 != '\n'); )
        *c2++ = *c1++;

      /* null term the copy and skip over the newline... */
      *c2++ = '\0';
      if (*c1 == '\n')
        c1++;

      /* if this entry is empty, skip it... */
      if (!*buffer)
        continue;

      fontArray[i1].labelName = XtMalloc(strlen(buffer) + 1);
      strcpy(fontArray[i1].labelName, buffer);
      /* look for a separating '/'... */
      if (c2 = strchr(fontArray[i1].labelName, '/'))
        {
          /* found, null it out... */
          *c2++ = '\0';
          /* and assign it to the fontName... */
          fontArray[i1].fontName = c2;
          strcpy(fontName, c2);
        }
      else
        {
          /* calculate the pixelsize for the font... */
          fontArray[i1].fontName = fontArray[i1].labelName;
          strcpy(fontName, fontArray[i1].labelName);
	}

      /* clear out the labelName... */
      fontArray[i1].labelName = (char *) 0;

      /* is it a fontset?... */
      if (fontName[strlen(fontName) - 1] == ':')
	{
	  /* strip off ':'  */
	  fontName[strlen(fontName) - 1] = '\0';
	  /* strip off a '-' before the ':' (should not have been
	   * one, but)...  */
	  if (fontName[strlen(fontName) - 1] == '-')
	    fontName[strlen(fontName) - 1] = '\0';
	}

      if (fontSet=XCreateFontSet(XtDisplay(w), fontName,
				 &miss_char_list, &miss_count, &miss_string))
	{

	  num_fonts = XFontsOfFontSet(fontSet, &fonts, &fontNames);
	  c2 = fontNames[0];

	  for (i2 = 0; i2 < 7; i2++)
	    {
	      while (*c2 && (*c2 != '-'))
		c2++;

	      if (!*c2)
		break;

	      c2++; /* skip over the '-'... */
	    }

	  if (i2 == 7)
	    {

              /* get the font height in pixel  */
	      curFontHeight = strtol(c2, (char **) 0, 0);

	      if (i1 == 0)
		preFontHeight = curFontHeight;

	      if ( i1 && preFontHeight == curFontHeight){

                /* For repeated font size, mark it so that it can
                   be eliminated later for the fontSizeToggles */
		fontArray[i1].labelName = XtMalloc(strlen("repeat") + 1);
		(void) strcpy(fontArray[i1].labelName, "repeat");

	      } else {

                /* For non-repeated font size, generate label  */
		pointSize = ((float)curFontHeight) * 72 / dpi;
		/* this was taken from the style manager... */
		if (dpi <= 72)  /* whole points... */
		  sprintf(fontName,
			  (GETMESSAGE(NL_SETN_ViewMenu,5, "%d point")),
			  (int) (pointSize + 0.5));
		else if (dpi <= 144) /* half points... */
		  sprintf(fontName,
			  (GETMESSAGE(NL_SETN_ViewMenu,6, "%.1f point")),
			  ((int) (pointSize * 2.0 + 0.5)) / 2.0);
		else if (dpi <= 720) /* tenth point... */
		  sprintf(fontName,
			  (GETMESSAGE(NL_SETN_ViewMenu,7, "%.1f point")),
			  ((int) (pointSize * 10.0 + 0.5)) / 10.0);
		else    /* hundredth point... */
		  sprintf(fontName,
			  (GETMESSAGE(NL_SETN_ViewMenu,8, "%.2f point")),
			  ((int) (pointSize * 100.0 + 0.5)) / 100.0);

		fontArray[i1].labelName = XtMalloc(strlen(fontName) + 1);
		strcpy(fontArray[i1].labelName, fontName);
		preFontHeight = curFontHeight;

	      }

	    }

	  /* free the fontSet including fontNames, ... */
	  XFreeFontSet(XtDisplay(w), fontSet);

	}

      if (!fontArray[i1].labelName)
	fontArray[i1].labelName = fontArray[i1].fontName;

      if (!strcmp(fontArray[i1].labelName,"repeat")){
	XtFree(fontArray[i1].labelName);
	fontArray[i1].labelName = NULL;
      }

      i1++;
    }

  fontArrayCount = i1;  /* we have our list... */

  return;
}

static void
createFontMenu(Widget w,
               Widget menu)
{
  DtTermViewWidget	  tw = (DtTermViewWidget) w;
  Widget		  submenu;
  Widget		  button;
  int			  i1;
  char   *c1;
  char   *c2;
  char    mnemonics[BUFSIZ];
  char    buffer[BUFSIZ];
  KeySym  ks;
  Arg     al[20];
  int     ac;

  ac = 0;
  XtSetArg(al[ac], XmNradioBehavior, True); ac++;
  submenu = createPulldown(menu, "Font Size", al, ac);

  if (fontArray == NULL)
    makeFontArrayFromTermView(w);
    if (fontArray == NULL)
      return;

  /* clear out mnemonics string... */
  *mnemonics = '\0';
  c1 = mnemonics;

  /* create the font buttons... */
  fontSizeToggles = (Widget *) XtMalloc((fontArrayCount+1)*sizeof(Widget));

  for (i1 = 0; i1 < fontArrayCount; i1++)
    {
#ifdef NOTDEF
      for (c2 = fontArray[i1].labelName; *c2; c2++)
        if (!strchr(mnemonics, *c2) && !isspace(*c2))
          break;

      if (*c2)
        {
          /* add it to the mnemonics list... */
          *c1++ = *c2;
          ks = XK_A + *c2 - 'A';
        }
      else
        ks = NULL;
#endif
      if (!fontArray[i1].labelName)
	fontSizeToggles[i1] = NULL;
      else{
	fontSizeToggles[i1] =
	  _DtTermViewCreateToggleButton(submenu,
					fontArray[i1].labelName,
					NULL, NULL, NULL, fontChangeCallback,
					(XtPointer) i1);
	ac = 0;
	XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
	XtSetValues(fontSizeToggles[i1], al, ac);
      }
    }

  strcpy(buffer, (GETMESSAGE(NL_SETN_ViewMenu,9, "Default")));

  fontSizeToggles[i1] =
  _DtTermViewCreateToggleButton(submenu, buffer,
                                NULL, NULL, NULL, defaultFontCallback, NULL);
  ac = 0;
  XtSetArg(al[ac], XmNvisibleWhenOff, True); ac++;
  XtSetValues(fontSizeToggles[i1], al, ac);
  fontSizeTogglesDefault = i1;

  ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,11, "F"));
  _DtTermViewCreateCascadeButton(menu, submenu,
                          (GETMESSAGE(NL_SETN_ViewMenu,10, "Font Size")),
                                 ks, NULL, NULL, NULL, NULL);
}

static void
delayWindowCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget		pulldown = (Widget) client_data;
    KeySym		ks;
    char		*accelerator;
    char		*acceleratorText;
    Widget		button;
    static Boolean 	first = True;

	if (!first)
		return;

	first = False;
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,13, "N"));
	newButton = _DtTermViewCreatePushButton(pulldown,
	        (GETMESSAGE(NL_SETN_ViewMenu,12, "New")),
		ks,
		NULL, NULL, cloneCallback, NULL);
#ifdef	NOTDEF
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,15, "P"));
	button = _DtTermViewCreatePushButton(pulldown,
	        (GETMESSAGE(NL_SETN_ViewMenu,14, "Print")),
		ks,
		NULL, NULL, NULL, NULL);
	XtSetSensitive(button, False);
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,17, "r"));
	button = _DtTermViewCreatePushButton(pulldown,
	        (GETMESSAGE(NL_SETN_ViewMenu,16, "Print...")),
		ks,
		NULL, NULL, NULL, NULL);
	XtSetSensitive(button, False);
#endif	/* NOTDEF */
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,19, "C"));
	accelerator = mallocGETMESSAGE(NL_SETN_ViewMenu,62, "Alt F4");
	acceleratorText = mallocGETMESSAGE(NL_SETN_ViewMenu,62, "Alt+F4");
	button = _DtTermViewCreatePushButton(pulldown,
	        (GETMESSAGE(NL_SETN_ViewMenu,18, "Close")),
		ks,
		accelerator,
		acceleratorText,
		exitCallback, NULL);
	XtFree(accelerator);
	XtFree(acceleratorText);
}

static void
delayEditCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget		pulldown = (Widget) client_data;
    KeySym		ks;
    char		*accelerator;
    char		*acceleratorText;
    Widget		button;
    static Boolean 	first = True;

	if (!first)
		return;

	first = False;
        accelerator = mallocGETMESSAGE(NL_SETN_ViewMenu,24,
		"Ctrl <Key>osfInsert");
	acceleratorText = mallocGETMESSAGE(NL_SETN_ViewMenu,25, "Ctrl+Insert");
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,23, "C"));
	button = _DtTermViewCreatePushButton(pulldown,
		  (GETMESSAGE(NL_SETN_ViewMenu,22, "Copy")),
		  ks,
		  accelerator,
		  acceleratorText,
                  copyClipboardCallback, NULL);
	XtFree(accelerator);
	XtFree(acceleratorText);

        ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,27, "P"));
        accelerator = mallocGETMESSAGE(NL_SETN_ViewMenu,28,
		"Shift <Key>osfInsert");
        acceleratorText = mallocGETMESSAGE(NL_SETN_ViewMenu,29, "Shift+Insert");
	button = _DtTermViewCreatePushButton(pulldown,
	          (GETMESSAGE(NL_SETN_ViewMenu,26, "Paste")),
		  ks,
		  accelerator,
		  acceleratorText,
                  pasteClipboardCallback, NULL);
	XtFree(accelerator);
	XtFree(acceleratorText);
}

static void
delayOptionsCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget		termView = (Widget) client_data;
    KeySym		ks;
    char		*accelerator;
    char		*acceleratorText;
    Widget		button;
    Widget submenu;
    Arg arglist[20];
    Arg *newArglist;
    int i;
    static Boolean 	first = True;

	if (!first)
		return;

	first = False;

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,33, "M"));
	menuBarToggle = _DtTermViewCreateToggleButton(pulldown[optionsPC],
	                (GETMESSAGE(NL_SETN_ViewMenu,32, "Menu Bar")),
		         ks,
			 NULL, NULL, menuBarToggleCallback, NULL);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,35, "S"));
	scrollBarToggle = _DtTermViewCreateToggleButton(pulldown[optionsPC],
	                 (GETMESSAGE(NL_SETN_ViewMenu,34, "Scroll Bar")),
		          ks,
			  NULL, NULL, scrollBarToggleCallback, NULL);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,37, "G"));
	button = _DtTermViewCreatePushButton(pulldown[optionsPC],
	         (GETMESSAGE(NL_SETN_ViewMenu,36, "Global...")),
		  ks,
		  NULL, NULL, globalOptionsCallback, NULL);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,39, "T"));
	button = _DtTermViewCreatePushButton(pulldown[optionsPC],
	          (GETMESSAGE(NL_SETN_ViewMenu,38, "Terminal...")),
		  ks,
		  NULL, NULL, terminalOptionsCallback, NULL);

	createSizeMenu(termView, pulldown[optionsPC]);

	createFontMenu(termView, pulldown[optionsPC]);

	i = 0;
	submenu = createPulldown(pulldown[optionsPC], "Reset", arglist, i);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,41, "S"));
	button = _DtTermViewCreatePushButton(submenu,
	         (GETMESSAGE(NL_SETN_ViewMenu,40, "Soft Reset")),
		 ks,
		NULL, NULL, softResetCallback, NULL);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,43, "H"));
	button = _DtTermViewCreatePushButton(submenu,
	         (GETMESSAGE(NL_SETN_ViewMenu,42, "Hard Reset")),
		  ks,
		  NULL, NULL, hardResetCallback, NULL);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,45, "R"));
	_DtTermViewCreateCascadeButton(pulldown[optionsPC], submenu,
	        (GETMESSAGE(NL_SETN_ViewMenu,44, "Reset")),
		ks,
		NULL, NULL, NULL, NULL);
}

#ifdef	HPVUE
static void
delayHelpCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget		pulldown = (Widget) client_data;
    KeySym		ks;
    char		*accelerator;
    char		*acceleratorText;
    Widget		button;
    static Boolean 	first = True;

	if (!first)
		return;

	first = False;
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,49, "O"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,48, "Overview")),
		 ks,
		 NULL, NULL, helpIntroCallback, NULL);

	button = _DtTermViewCreateSeparator(pulldown, "Separator");

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,51,"T"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,50, "Tasks")),
		 ks,
		 NULL, NULL, helpTasksCallback, NULL);
	XtSetSensitive(button, False);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,53,"R"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,52, "Reference")),
		 ks,
		 NULL, NULL, helpReferenceCallback, NULL) ;

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,55,"O"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,54, "On Item")),
		 ks,
		 NULL, NULL, NULL, NULL);
	XtSetSensitive(button, False);

	button = _DtTermViewCreateSeparator(pulldown, "Separator");

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,57,"U"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,56, "Using Help")),
		ks,
		NULL, NULL, helpOnHelpCallback, NULL);

	button = _DtTermViewCreateSeparator(pulldown, "Separator");

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,59,"A"));
	button = _DtTermViewCreatePushButton(pulldown,
	        (GETMESSAGE(NL_SETN_ViewMenu,82, "About Terminal")),
		ks,
		NULL, NULL, helpVersionCallback, NULL);
}
#else  /* HPVUE */

static void
delayHelpCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget		pulldown = (Widget) client_data;
    KeySym		ks;
    char		*accelerator;
    char		*acceleratorText;
    Widget		button;
    static Boolean 	first = True;

	if (!first)
		return;

	first = False;
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,65, "v"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,64, "Overview")),
		 ks,
		 NULL, NULL, helpOverviewCallback, NULL);

	button = _DtTermViewCreateSeparator(pulldown, "Separator");

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,69,"C"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,68, "Table Of Contents")),
		 ks,
		 NULL, NULL, helpTableOfContentsCallback, NULL);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,71,"T"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,70, "Tasks")),
		 ks,
		 NULL, NULL, helpTasksCallback, NULL);

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,73,"R"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,72, "Reference")),
		 ks,
		 NULL, NULL, helpReferenceCallback, NULL) ;

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,75,"K"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,74, "Keyboard")),
		 ks,
		 NULL, NULL, helpKeyboardCallback, NULL) ;

	button = _DtTermViewCreateSeparator(pulldown, "Separator");

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,77,"U"));
	button = _DtTermViewCreatePushButton(pulldown,
	         (GETMESSAGE(NL_SETN_ViewMenu,76, "Using Help")),
		ks,
		NULL, NULL, helpUsingHelpCallback, NULL);

	button = _DtTermViewCreateSeparator(pulldown, "Separator");

	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,79,"A"));
	button = _DtTermViewCreatePushButton(pulldown,
	        (GETMESSAGE(NL_SETN_ViewMenu,83, "About Terminal")),
		ks,
		NULL, NULL, helpAboutDttermCallback, NULL);
}
#endif /* HPVUE */


static Widget
CreateMenu(Widget termView, Widget parent, Boolean menuBar,
	Arg menuArglist[], int menuArgcount)
{
    Widget menu;
    static Widget topLevel = (Widget) 0;
    static Boolean first = True;
    static Boolean firstPopup = True;
    static Widget popupMenu = (Widget) 0;
    int pc;
    Widget button;
    Widget submenu;
    Arg arglist[20];
    Arg *newArglist;
    int i;
    KeySym		  ks;
    char		 *accelerator;
    char		 *acceleratorText;

    /* if this is the first time, we will need a topLevel widget... */
    if (first) {
	i = 0;
	topLevel = XtAppCreateShell((char *) 0, "Dtterm",
		applicationShellWidgetClass, XtDisplay(parent), arglist, i);
    }

    newArglist = (Arg *) XtMalloc((menuArgcount + 1) * sizeof(Arg));
    for (i = 0; i < menuArgcount; i++) {
	newArglist[i].name = menuArglist[i].name;
	newArglist[i].value = menuArglist[i].value;
    }
    if (menuBar) {
	/* we will create the menubar unmanaged so that our parent can mange
	 * it when it wants to...
	 */
	/*DKS
	XtSetArg(newArglist[i], XmNmenuAccelerator, "Ctrl<Key>F10"); i++;
	DKS*/
	menu = XmCreateMenuBar(parent, "menu_pulldown", newArglist, i);
    } else {
	if (firstPopup) {
	    /*DKS
	    XtSetArg(newArglist[i], XmNmenuAccelerator,
		    "Shift Ctrl<Key>F10"); i++;
	    DKS*/
	    popupMenu = XmCreatePopupMenu(topLevel, "menu_popup", newArglist,
		    i);
	}
	menu = popupMenu;
	button = _DtTermViewCreateLabel(menu,
	          (GETMESSAGE(NL_SETN_Main, 3, "Terminal")));
	button = _DtTermViewCreateSeparator(menu, "Separator");
    }
    XtFree((char *) newArglist);

    /*****************************************
     * create the "Window" pulldown...
     *****************************************/
    pc = 0;
    if (first) {
	i = 0;
	pulldown[pc] = createPulldown(topLevel, "Window", arglist, i);
    }

    if (menuBar || (!menuBar && firstPopup))
    {
#ifdef	PULLDOWN_ACCELERATORS
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,21, "W"));
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	          (GETMESSAGE(NL_SETN_ViewMenu,20, "Window")),
		  ks,
		  NULL, NULL, NULL, NULL);
#else	/* PULLDOWN_ACCELERATORS */
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	        (GETMESSAGE(NL_SETN_ViewMenu,20, "Window")),
		NoSymbol, NULL, NULL, NULL, NULL);
#endif	/* PULLDOWN_ACCELERATORS */
	windowPC = pc;
    	XtAddCallback(cascade[pc], XmNcascadingCallback,
	    delayWindowCallback, (XtPointer) pulldown[pc]);
    }

    /*****************************************
     * create the "Edit" pulldown...
     *****************************************/
    pc++;
    if (first) {
	i = 0;
	pulldown[pc] = createPulldown(topLevel, "Edit", arglist, i);
    }
    if (menuBar || (!menuBar && firstPopup))
    {
#ifdef	PULLDOWN_ACCELERATORS
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,31, "E"));
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	          (GETMESSAGE(NL_SETN_ViewMenu,30, "Edit")),
		  ks,
		  NULL, NULL, NULL, NULL);
#else	/* PULLDOWN_ACCELERATORS */
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	        (GETMESSAGE(NL_SETN_ViewMenu,30, "Edit")),
		NoSymbol, NULL, NULL, NULL, NULL);
#endif	/* PULLDOWN_ACCELERATORS */
	editPC = pc;
    	XtAddCallback(cascade[pc], XmNcascadingCallback,
	    delayEditCallback, (XtPointer) pulldown[pc]);
	/*
	 * create the edit pulldown early because it's the easiest
	 * way of dealing with the accelerators
	 */
	XtCallCallbacks (cascade[pc], XmNcascadingCallback, pulldown[pc]);
    }

    /*****************************************
     * create the "Options" pulldown...
     *****************************************/
    pc++;
    if (first) {
	i = 0;
	pulldown[pc] = createPulldown(topLevel, "Options", arglist, i);
    }

    if (menuBar || (!menuBar && firstPopup))
    {
#ifdef	PULLDOWN_ACCELERATORS
        ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,47, "O"));
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	          (GETMESSAGE(NL_SETN_ViewMenu,46, "Options")),
		   ks,
		   NULL, NULL, NULL, NULL);
#else	/* PULLDOWN_ACCELERATORS */
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	          (GETMESSAGE(NL_SETN_ViewMenu,46, "Options")),
		NoSymbol, NULL, NULL, NULL, NULL);
#endif	/* PULLDOWN_ACCELERATORS */
	optionsPC = pc;
    	XtAddCallback(cascade[pc], XmNcascadingCallback,
	    delayOptionsCallback, (XtPointer) termView);
    }

    /*****************************************
     * create the "Help" pulldown...
     *****************************************/
    pc++;
    if (first) {
	i = 0;
	pulldown[pc] = createPulldown(topLevel, "Help", arglist, i);
    }

#ifdef HPVUE
    if (menuBar || (!menuBar && firstPopup))
    {
#ifdef	PULLDOWN_ACCELERATORS
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,61,"H"));
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	         (GETMESSAGE(NL_SETN_ViewMenu,60, "Help")),
		ks,
		NULL, NULL, NULL, NULL);
#else	/* PULLDOWN_ACCELERATORS */
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	        (GETMESSAGE(NL_SETN_ViewMenu,60, "Help")),
		NoSymbol, NULL, NULL, NULL, NULL);
    }
#endif	/* PULLDOWN_ACCELERATORS */
#else	/* HPVUE */

    if (menuBar || (!menuBar && firstPopup))
    {
#ifdef	PULLDOWN_ACCELERATORS
	ks = XStringToKeysym(GETMESSAGE(NL_SETN_ViewMenu,81,"H"));
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	         (GETMESSAGE(NL_SETN_ViewMenu,80, "Help")),
		ks,
		NULL, NULL, NULL, NULL);
#else	/* PULLDOWN_ACCELERATORS */
	cascade[pc] = _DtTermViewCreateCascadeButton(menu, pulldown[pc],
	        (GETMESSAGE(NL_SETN_ViewMenu,80, "Help")),
		NoSymbol, NULL, NULL, NULL, NULL);
#endif	/* PULLDOWN_ACCELERATORS */
#endif	/* HPVUE */
    	XtAddCallback(cascade[pc], XmNcascadingCallback,
	    delayHelpCallback, (XtPointer) pulldown[pc]);
    }

    if (menuBar) {
	/* this is the help button... */
	i = 0;
	XtSetArg(arglist[i], XmNmenuHelpWidget, cascade[pc]); i++;
	XtSetValues(menu, arglist, i);
    }

    if (!menuBar && firstPopup)
	firstPopup = False;

    if (first)
	first = False;

    return(menu);
}

Widget
_DtTermViewCreatePulldownMenu
(
    Widget		  termView,
    Widget		  parent,
    Arg		 	  menuArglist[],
    int			  menuArgcount
)
{
    DtTermViewWidget tw = (DtTermViewWidget) termView;
    tw->termview.menuBar.widget =
      CreateMenu(termView, parent, True, menuArglist, menuArgcount);
    /* set up a handler to post menuBar... */
    XtAddEventHandler(tw->termview.menuBar.widget, ButtonPressMask,
            False, postMenuBar, (XtPointer)termView);
    return (tw->termview.menuBar.widget);
}

Widget
_DtTermViewCreatePopupMenu
(
    Widget		  termView,
    Widget		  parent,
    Arg			  menuArglist[],
    int			  menuArgcount
)
{

    static Widget popupMenu = (Widget) 0;	/* popup widget...	*/

    /* Create popup menu only at the first time         */
    if (!popupMenu)
      popupMenu = CreateMenu(termView, parent, False,
                             menuArglist, menuArgcount);

    /* set up a handler to post the menu... */
    XtAddEventHandler(parent, ButtonPressMask, False,
                      postPopupMenu, (XtPointer) popupMenu);

    XtAddEventHandler(parent, KeyPressMask, True,
                      setCurrentWidgets, (XtPointer) termView);

    return (popupMenu);

}

/*ARGSUSED*/
static void
setContext(Widget w, XtPointer client_data, XtPointer call_data)
{
    DtTermViewWidget	  tw;
    Arg arglist[20];
    int i;
    Boolean menuBarState;
    Boolean scrollBarState;
#ifdef	WINDOW_SIZE_TOGGLES
    short rows;
    short columns;
#endif	/* WINDOW_SIZE_TOGGLES */

    /* Get term from XtEevet in postPopupMenu and PostMenuBar
       a workarond for using XmGetPostedFromWidget */
    w = currentTermWidget;

    /* walk up the tree until we find an DtTermView widget... */
    while (w && !XtIsShell(w) && !XtIsSubclass(w, dtTermViewWidgetClass)) {
	w = XtParent(w);
    }
    if (!w) {
	return;
    }
    /* make sure it is an DtTermView widget... */
    if (!XtIsSubclass(w, dtTermViewWidgetClass)) {
	/* problem... */
	fprintf(stderr,
		"unable to find dtTermViewWidgetClass parent for menu\n");
	return;
    }

    tw = (DtTermViewWidget) w;
    currentWidget = w;

    /* update the menu toggle buttons... */
    i = 0;
    XtSetArg(arglist[i], DtNscrollBar, &scrollBarState); i++;
    XtSetArg(arglist[i], DtNmenuBar, &menuBarState); i++;
#ifdef	WINDOW_SIZE_TOGGLES
    XtSetArg(arglist[i], DtNrows, &rows); i++;
    XtSetArg(arglist[i], DtNcolumns, &columns); i++;
#endif	/* WINDOW_SIZE_TOGGLES */
    XtGetValues(w, arglist, i);

    /* set the toggles... */
    if (scrollBarToggle && menuBarToggle) {
    	XmToggleButtonGadgetSetState(scrollBarToggle,
		scrollBarState, False);
    	XmToggleButtonGadgetSetState(menuBarToggle, menuBarState, False);
    }

    /* set the font toggles...
     */
    if (fontSizeToggles)
    {
        for (i = 0; i < fontSizeTogglesDefault; i++) {
	  if (fontSizeToggles[i]){
	    if (tw->termview.currentFontToggleButtonIndex == i) {
	        XmToggleButtonGadgetSetState(fontSizeToggles[i],
		    True, False);
	    } else {
	        XmToggleButtonGadgetSetState(fontSizeToggles[i],
		    False, False);
	    }
          }
        }
        if ((tw->termview.currentFontToggleButtonIndex
		>= fontSizeTogglesDefault) ||
	    (tw->termview.currentFontToggleButtonIndex < 0)) {
	    XmToggleButtonGadgetSetState(fontSizeToggles[i],
		True, False);
        } else {
	    XmToggleButtonGadgetSetState(fontSizeToggles[i],
		False, False);
        }
    }

#ifdef	WINDOW_SIZE_TOGGLES
    if (windowSizeToggles)
    {
        /* set the window size toggles...
        for (i = 0; i < windowSizeTogglesDefault; i++) {
	    if(currentWindowToggleButtonIndex == i) {
	        XmToggleButtonGadgetSetState(windowSizeToggles[i],
		    True, False);
            }
	    else {
	        XmToggleButtonGadgetSetState(windowSizeToggles[i],
		    False, False);
	    }
        }
        if ((currentWindowToggleButtonIndex >= windowSizeTogglesDefault) ||
	    (currentWindowToggleButtonIndex < 0)) {
	    XmToggleButtonGadgetSetState(windowSizeToggles[i],
		True, False);
        } else {
	    XmToggleButtonGadgetSetState(windowSizeToggles[i],
		False, False);
        }
    /**** This part is commented by Tanuja Shah ****/
/* set the window size toggles...
        for (i = 0; i < tw->termview.sizeList.numSizes; i++) {
	    if (((tw->termview.sizeList.sizes[i].rows <= 0) ||
		(rows == tw->termview.sizeList.sizes[i].rows)) &&
		((tw->termview.sizeList.sizes[i].columns <= 0) ||
		(columns == tw->termview.sizeList.sizes[i].columns))) {
	        XmToggleButtonGadgetSetState(windowSizeToggles[i],
		    True, False);
	    } else {
	        XmToggleButtonGadgetSetState(windowSizeToggles[i],
		    False, False);
	    }
        }
        if (((tw->termview.sizeDefault.rows <= 0) ||
	    (tw->termview.sizeDefault.rows == rows)) &&
	    ((tw->termview.sizeDefault.columns <= 0) ||
	    (tw->termview.sizeDefault.columns == columns))) {
	    XmToggleButtonGadgetSetState(windowSizeToggles[i],
		True, False);
        } else {
	    XmToggleButtonGadgetSetState(windowSizeToggles[i],
		False, False);
        }
*/
    }
#endif	/* WINDOW_SIZE_TOGGLES */


    /* set the sensitivity on the new button... */
    if (newButton)
    	XtSetSensitive(newButton, DtTermViewGetCloneEnabled(w));
}


/*************************************************************************
 *
 *  menu callbacks
 */
/*ARGSUSED*/
static void
exitCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    /* walk up the widget tree to find the shell... */

    w = currentWidget;

    /* invoke the callbacks for this terminal emulator... */
    XtCallCallbacks(w, DtNsubprocessTerminationCallback, (XtPointer) 0);

#ifdef	NOTDEF
    /* make sure things are dead by destroying the interface... */
    while (w && !XtIsShell(w)) {
	w = XtParent(w);
    }

    /* destroy the interface... */
    XtDestroyWidget(w);
#endif	/* NOTDEF */
}

/*ARGSUSED*/
static void
scrollBarToggleCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg arglist;
    XmToggleButtonCallbackStruct *cb =
	    (XmToggleButtonCallbackStruct *) call_data;

    XtSetArg(arglist, DtNscrollBar, cb->set);
    XtSetValues(currentWidget, &arglist, 1);
}

void
_DtTermViewMenuToggleMenuBar
(
    Widget		  w
)
{
    Arg al;
    Boolean toggle;

    XtSetArg(al, DtNmenuBar, &toggle);
    XtGetValues(w, &al, 1);

    XtSetArg(al, DtNmenuBar, !toggle);
    XtSetValues(w, &al, 1);
}

/*ARGSUSED*/
static void
menuBarToggleCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg arglist;
    XmToggleButtonCallbackStruct *cb =
	    (XmToggleButtonCallbackStruct *) call_data;

    XtSetArg(arglist, DtNmenuBar, cb->set);
    XtSetValues(currentWidget, &arglist, 1);
}

/*ARGSUSED*/
static void
cloneCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    DtTermViewCloneCallback(currentWidget, client_data, call_data);
}

#ifdef	HPVUE
/*ARGSUSED*/
static void
helpVersionCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "_copyright");
}

/*ARGSUSED*/
static void
helpIntroCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "_hometopic");
}

/*ARGSUSED*/
static void
helpTasksCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "UsingTermEmulators");
}

/*ARGSUSED*/
static void
helpReferenceCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "dtterm1x");
}

/*ARGSUSED*/
static void
helpOnHelpCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Help4Help", "_hometopic");
}
#else	/* HPVUE */

/*ARGSUSED*/
static void
helpOverviewCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "_hometopic");
}

/*ARGSUSED*/
static void
helpTableOfContentsCallback(Widget w, XtPointer client_data,
	XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "TableOfContents");
}

/*ARGSUSED*/
static void
helpTasksCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "Tasks");
}

/*ARGSUSED*/
static void
helpReferenceCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "Reference");
}

/*ARGSUSED*/
static void
helpKeyboardCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "Keyboard");
}

/*ARGSUSED*/
static void
helpUsingHelpCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Help4Help", "_hometopic");
}

/*ARGSUSED*/
static void
helpAboutDttermCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermViewMapHelp(currentWidget, "Terminal", "_copyright");
}
#endif	/* HPVUE */

static void
raiseDialog(Widget w)
{
    while (w && !XtIsShell(w)) {
	w = XtParent(w);
    }

    if (w) {
	XRaiseWindow(XtDisplay(w), XtWindow(w));
    }
}

/*ARGSUSED*/
static void
globalOptionsCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    DtTermViewWidget tw = (DtTermViewWidget) currentWidget;

    if (tw->termview.globalOptionsDialog) {
	XtManageChild(tw->termview.globalOptionsDialog);
	raiseDialog(tw->termview.globalOptionsDialog);
    } else {
	tw->termview.globalOptionsDialog =
		_DtTermViewCreateGlobalOptionsDialog(currentWidget);
	raiseDialog(tw->termview.globalOptionsDialog);
    }
}

/*ARGSUSED*/
static void
terminalOptionsCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    DtTermViewWidget tw = (DtTermViewWidget) currentWidget;

    if (tw->termview.terminalOptionsDialog) {
	XtManageChild(tw->termview.terminalOptionsDialog);
	raiseDialog(tw->termview.terminalOptionsDialog);
    } else {
	tw->termview.terminalOptionsDialog =
		_DtTermViewCreateTerminalOptionsDialog(currentWidget);
	raiseDialog(tw->termview.terminalOptionsDialog);
    }
}

/*ARGSUSED*/
static void
sizeChangeCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    DtTermViewWidget tw = (DtTermViewWidget) currentWidget;
    Arg al[10];
    int ac;
    int i1;

    i1 = (int) client_data;
    ac = 0;
    if (tw->termview.sizeList.sizes[i1].columns > 0) {
	XtSetArg(al[ac], DtNcolumns,
		tw->termview.sizeList.sizes[i1].columns); ac++;
    }
    if (tw->termview.sizeList.sizes[i1].rows > 0) {
	XtSetArg(al[ac], DtNrows,
		tw->termview.sizeList.sizes[i1].rows); ac++;
    }
    if (ac > 0) {
	XtSetValues(currentWidget, al, ac);
    }

    /* commennt ed until a coherent method is implemented to set the
       right toggle button for the corresponding dtterm */

    /* currentWindowToggleButtonIndex = i1; */

    XtVaSetValues(w, XmNset, False, NULL);

    /* End of commented code for common behavior across all of dtterms */
}

/*ARGSUSED*/
static void
defaultSizeCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  DtTermViewWidget tw = (DtTermViewWidget) currentWidget;
  Arg al[10];
  int ac;

  ac = 0;
  if (tw->termview.sizeDefault.columns > 0)
    {
      XtSetArg(al[ac], DtNcolumns, tw->termview.sizeDefault.columns);
      ac++;
    }

  if (tw->termview.sizeDefault.rows > 0)
    {
      XtSetArg(al[ac], DtNrows, tw->termview.sizeDefault.rows); ac++;
    }

  if(ac > 0)
    XtSetValues(currentWidget, al, ac);

    /* create the label string and stuff it... */

    /* I am commenting this code until a better mechanism for finding
       the correct window size for each dtterm is implemented. Right
       now, many ideas has been partially implemented. For now, all the
       toggle buttons in the WindowSize pulldown menu will be unselected.
       This is just to have a uniform behavior for all dtterms. The popup
       menu belongs to all the dtterms cloned from each other. Therefore,
       the information you set in it from one dtterm might be completely
       wrong for another child (cloned). Better to close this can of
       worms for now. */

    /*
    ac = 0;
    sprintf(buffer,"Default(%hdx%hd)",tw->termview.sizeDefault.columns,
                                        tw->termview.sizeDefault.rows);

    string = XmStringCreateLtoR(GETMESSAGE(NL_SETN_ViewMenu,1,
    buffer), XmFONTLIST_DEFAULT_TAG);
    XtSetArg(al[ac], XmNlabelString, string); ac++;
    XtSetValues(w, al, ac);
    currentWindowToggleButtonIndex = windowSizeTogglesDefault;
    */

    XtVaSetValues(w, XmNset, False, NULL);

    /* End of commented code for common behavior across all of dtterms */
}

/*ARGSUSED*/
static void
fontChangeCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    DtTermViewWidget tw = (DtTermViewWidget) currentWidget;

    XrmValue from;
    XrmValue to;
    Arg al[10];
    int ac;
    int index = (int) client_data;

    if (fontArray == NULL)
      makeFontArrayFromTermView(w);

    if (fontArray == NULL)
      return;

    /* generate the fontlist from the font... */
    from.size = strlen(fontArray[index].fontName);
    from.addr = (XtPointer) fontArray[index].fontName;
    to.size = sizeof(fontArray[index].fontList);
    to.addr = (XtPointer) &fontArray[index].fontList;

    /* Unable to convert to a fontlist.  For now, let's just
     * ignore it and return... */

    if (!XtConvertAndStore((Widget) tw, XmRString, &from, XmRFontList, &to))
      return;

    if (tw->termview.userFontName)
        XtFree(tw->termview.userFontName);

    tw->termview.userFontName = XtNewString(fontArray[index].fontName);

    if (tw->termview.fontList != fontArray[index].fontList)
      {
	ac = 0;
	XtSetArg(al[ac], DtNuserFont, fontArray[index].fontList); ac++;
	XtSetValues(currentWidget, al, ac);
      }

    tw->termview.currentFontToggleButtonIndex = index;

    return;
}

/*ARGSUSED*/
static void
defaultFontCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    DtTermViewWidget tw = (DtTermViewWidget) currentWidget;
    Arg al[10];
    int ac;

    if (tw->termview.userFontName)
	    XtFree(tw->termview.userFontName);
    tw->termview.userFontName = NULL;

    if (tw->termview.fontList != tw->termview.defaultFontList) {
	ac = 0;
	XtSetArg(al[ac], DtNuserFont, tw->termview.defaultFontList);
		ac++;
	XtSetValues(currentWidget, al, ac);
    }
    tw->termview.currentFontToggleButtonIndex = fontSizeTogglesDefault;
}
/*ARGSUSED*/
static void
softResetCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermFuncSoftReset(_DtTermViewGetChild(currentWidget,
	    DtTermTERM_WIDGET), 1, fromMenu);
}

/*ARGSUSED*/
static void
hardResetCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    _DtTermFuncHardReset(_DtTermViewGetChild(currentWidget,
	    DtTermTERM_WIDGET), 1, fromMenu);
}

/*ARGSUSED*/
static void
copyClipboardCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget tw;

    /* since this may have been called from an accelerator, we need
     * to insure that our context was set...
     */
    setContext(w, client_data, call_data);
    tw = _DtTermViewGetChild(currentWidget, DtTermTERM_WIDGET);
    _DtTermPrimSelectCopyClipboard(tw,
                     XtLastTimestampProcessed(XtDisplay(tw)));
}

/*ARGSUSED*/
static void
pasteClipboardCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget tw;

    /* since this may have been called from an accelerator, we need
     * to insure that our context was set...
     */
    setContext(w, client_data, call_data);
    tw = _DtTermViewGetChild(currentWidget, DtTermTERM_WIDGET);
    _DtTermPrimSelectPasteClipboard(tw);
}

int _DtTermViewGetUserFontListIndex(Widget w)
{
  DtTermViewWidget tw = (DtTermViewWidget) w;

  /* use 0 for default, index + 1 for specific buttons... */
  if (tw->termview.currentFontToggleButtonIndex == fontSizeTogglesDefault)
    return(0);
  else
    return(tw->termview.currentFontToggleButtonIndex + 1);
}

void _DtTermViewSetUserFontListIndex(Widget w, int i1)
{
  DtTermViewWidget tw = (DtTermViewWidget) w;

  /* indexing starts at 0 - indexing in resource file starts at 1. */

  if (i1 == 0)
    i1 = fontSizeTogglesDefault;
  else
    i1 -= 1;

  /* set up for the callback... */
  currentWidget = w;
  if (i1 == fontSizeTogglesDefault)
    defaultFontCallback(w, (XtPointer) fontSizeTogglesDefault, NULL);
  else
    XtVaSetValues(w,
                  DtNfontIndexSet,  True,
                  DtNuserFontIndex, i1,
                  NULL);

  return;
}

/* ----------------------------------------------------------- */
/*  Assign the saved userFontListIndex and corresponding Font  */
/* ----------------------------------------------------------- */
void
_DtTermRestoreUserFontIndex(Widget *InstanceList, int NumWidgets)
{
  int i, user_font_index = -1;
  Boolean font_index_set = False;

  for ( i = 0 ; i < NumWidgets ; i++ )
    {
      XtVaGetValues(InstanceList[i],
                    DtNfontIndexSet,  &font_index_set,
                    DtNuserFontIndex, &user_font_index,
                    NULL);

      if (font_index_set == True)
        {
          currentWidget = InstanceList[i];
          fontChangeCallback(InstanceList[i],
                             (XtPointer) user_font_index, NULL);
        }
    }

  return;
}
