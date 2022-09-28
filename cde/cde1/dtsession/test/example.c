/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        example.c
 **
 **   Project:     HP DT Developer's Kit
 **
 **   Description: An example to be included in development kit documentation
 ** 
 ****************************************************************************
 ************************************<+>*************************************/

/*
 *	Include Files
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Protocols.h>

/*
 * HP DT Developer Kit includes
 */
#include <Dt/Init.h>
#include <Dt/Session.h>
#include <Dt/Help.h>

void main ();			/*  main logic for application	*/
void RestoreAppState();

Widget CreateApplication ();	/*  create main window		*/
Widget CreateVersionID (); 

void InstallBitmapDataIntoXmCache();
void ShowCB();
void ClearCB();
void QuitCB ();
void OnVersionCB();
void OnItemCB();
void OnApplicationCB();
void OnHelpCB();
void OnKeysCB();

void SaveStateCB();

extern void _XmSelectColorDefault();

/*
 * Global Variables
 */
#define MAX_ARGS 20
#define charset       XmSTRING_DEFAULT_CHARSET


Display		*display;	
Screen         	*screen;
Widget		appShell;
Widget   	displayArea;
Widget		actionBtn1, actionBtn2;
Pixmap	 	ballPix, noBallPix;

Atom		XaWmSaveYourself;
char		ballState[10];

/*
 * Parse table for command line options
 */

typedef struct
{
    String	session;
} ApplicationData, *ApplicationDataPtr;

static XrmOptionDescRec opTable[] =
{
    {"-session", "session", XrmoptionSepArg, (caddr_t) NULL}
};

static XtResource resources[] =
{
    {"session", "Session", XtRString, sizeof(XtRString),
	 XtOffset(ApplicationDataPtr, session), (caddr_t) NULL},
};


#define ball_width 32
#define ball_height 32
static char ball_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x1c, 0x70, 0x00,
   0x00, 0x82, 0xc8, 0x00, 0x80, 0x09, 0x01, 0x03, 0x40, 0x00, 0x50, 0x04,
   0x40, 0x11, 0x02, 0x04, 0xa0, 0x24, 0x00, 0x0c, 0x30, 0x00, 0x20, 0x11,
   0x30, 0x03, 0x02, 0x19, 0x10, 0x54, 0x80, 0x18, 0x88, 0x01, 0x01, 0x30,
   0x28, 0x00, 0x10, 0x34, 0x38, 0x91, 0x04, 0x21, 0xc8, 0x00, 0x40, 0x34,
   0x68, 0x46, 0x01, 0x32, 0xd8, 0x0d, 0x88, 0x3b, 0xb8, 0x15, 0x42, 0x3e,
   0xf0, 0xbf, 0x50, 0x15, 0x70, 0xdd, 0x9d, 0x1b, 0xf0, 0x77, 0x77, 0x1e,
   0xe0, 0xfe, 0xd9, 0x0f, 0xc0, 0x57, 0xfe, 0x07, 0xc0, 0xfd, 0xbb, 0x07,
   0x80, 0xdf, 0xef, 0x03, 0x00, 0x7e, 0xff, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static char no_ball_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};



/*************************************<->*************************************
 *
 *  void main(unsigned int argc,
 *            char **argv)
 *
 *
 *  Description:
 *  -----------
 *  Main program for developer kit example.  Controls initialization - and
 *  then enters client event loop.
 *
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments program is called with
 *  argv = pointer to the command line arguments
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void main (argc,argv)
unsigned int argc;
char **argv;
{
    Widget		top;
    Widget		mainWindow;	/*  MainWindow		*/
    XtAppContext      	app;
    char		hostName [20];  
    Arg			args[10];	
    int			n;
    ApplicationData	commLineArgs;

    /*
     * Set up a communication with the toolkit
     */
    XtToolkitInitialize();
    app = XtCreateApplicationContext();
    display = XtOpenDisplay(app, NULL, "example", "Example",
			    opTable, XtNumber(opTable),
			    &argc, argv);
    if (!display)
    {
	XtWarning ("Example: Can't open display, exiting...");
	exit (-1);
    }

       
    /*
     * Create application shell
     */
    n = 0;
    XtSetArg(args[n], XmNallowShellResize, TRUE); n++;
    XtSetArg(args[n], XmNtitle, "Developer Kit Example"); n++; 
    appShell = XtAppCreateShell ("example", "Example",
				 applicationShellWidgetClass,
				 display, args, n);

    /*
     * Initialize connection to HP DT
     */
    if(DtAppInit(app,display, argv[0]) == False)
    {
	XtWarning ("Example: Can't connect to HP DT, exiting...");
	exit (-1);
    }

    /*
     * Get the command line options and restore the session if need be
     */
    XtGetApplicationResources(appShell, &commLineArgs, resources,
			      XtNumber(resources), NULL, 0);

    /*
     * If no session is being restored, but application in its default state
     */
    if(commLineArgs.session != NULL)
    {
	RestoreAppState(commLineArgs.session);
    }
    else
    {
	strcpy(ballState, "on");
    }

    /*
     *  Tell the Session Manager that to send an event when
     *  a Save Session is happening
     */
    XaWmSaveYourself = XInternAtom(XtDisplay(appShell), "WM_SAVE_YOURSELF",
				   False);
    XmAddWMProtocolCallback(appShell, XaWmSaveYourself, SaveStateCB,
			    (caddr_t) argv[0]);
    
      
    /*
     * Create and realize main application window.
     */
    mainWindow = CreateApplication(app, display);
    XtRealizeWidget (appShell);

    /*
     * Get and dispatch events
     */
    XtAppMainLoop (app);
}


/*************************************<->*************************************
 *
 *  Widget BuildMenus(Widget parent)
 *
 *
 *  Description:
 *  ------------
 *  BuildMenus builds up the menu bar to be displayed in the application
 *
 *
 *  Inputs:
 *  ------
 *  parent - mainWindow widget for the application
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Widget BuildMenus(parent)
Widget parent;
{
    Widget       menuBar;
    Widget       c[10];
    Widget	menu_pane;
    Widget	cascadeB1, cascadeB2, cascadeB3;
    Widget       menupane1, menupane2, menupane3;
    Widget       fileBtn1;
    Widget       helpBtn1, helpBtn2, helpBtn3, helpBtn4, helpBtn5;
    Arg		args[10];	
    int          n;
    XmString     accText;
    XmString     labelStr;

    n = 0;
    menuBar = XmCreateMenuBar (parent, "menuBar", args, n); 
    XtManageChild (menuBar);

    /*
     * Create Exit menupain and buttons
     */
    menupane1 = XmCreatePulldownMenu(menuBar, "menupane1", args, 0);
   
    /*
     * Exit menu button
     */
    labelStr = XmStringCreateLtoR("Exit",XmSTRING_DEFAULT_CHARSET);
    accText  = XmStringCreateLtoR("<Ctrl>X",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNaccelerator,"Ctrl <Key>X"); n++;
    XtSetArg(args[n], XmNacceleratorText ,accText); n++;
    XtSetArg(args[n], XmNmnemonic, 'x'); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    c[0] = fileBtn1 = XmCreatePushButtonGadget(menupane1, "fileBtn1", args, n);
    XtAddCallback(fileBtn1, XmNactivateCallback, QuitCB, NULL);
    XtManageChildren(c, 1);
    XmStringFree(accText);
    XmStringFree(labelStr);


    /*
     * Create Actions menupain and related buttons
     */
    menupane2 = XmCreatePulldownMenu(menuBar, "menupane2", args, 0);

    /*
     * Show ball menu item
     */
    labelStr = XmStringCreateLtoR("Show Ball",XmSTRING_DEFAULT_CHARSET);
    accText = XmStringCreateLtoR("<Ctrl>S",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNaccelerator,"Ctrl <Key>C"); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    XtSetArg(args[n], XmNacceleratorText ,accText); n++;
    if(!strcmp(ballState, "on"))
    {
	XtSetArg(args[n], XmNsensitive, False); n++;
    }
    XtSetArg(args[n], XmNmnemonic, 'C'); n++;
    c[0] = actionBtn1 = XmCreatePushButtonGadget(menupane2,
						 "actionBtn1",args, n);
    XtAddCallback(actionBtn1, XmNactivateCallback, ShowCB, NULL);
    XtManageChildren(c, 1);
    XmStringFree(accText);
    XmStringFree(labelStr);

    /*
     * remove ball menu item
     */
    labelStr = XmStringCreateLtoR("Remove Ball",XmSTRING_DEFAULT_CHARSET);
    accText = XmStringCreateLtoR("<Ctrl>R",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNaccelerator,"Ctrl <Key>J"); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    XtSetArg(args[n], XmNacceleratorText ,accText); n++;
    if(!strcmp(ballState, "off"))
    {
	XtSetArg(args[n], XmNsensitive, False); n++;
    }
    XtSetArg(args[n], XmNmnemonic, 'J'); n++;
    c[1] = actionBtn2 = XmCreatePushButtonGadget(menupane2,
						 "actionBtn2",args, n);
    XtAddCallback(actionBtn2, XmNactivateCallback, ClearCB, NULL);
    XtManageChildren(c, 2);
    XmStringFree(accText);
    XmStringFree(labelStr);


    /*
     * Create Help menupain and related buttons
     */
    menupane3 = XmCreatePulldownMenu(menuBar, "menupane3", args, 0);

    /*
     * On Item menu button
     */
    labelStr = XmStringCreateLtoR("On Item",XmSTRING_DEFAULT_CHARSET);
    accText = XmStringCreateLtoR("<Ctrl>I",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNaccelerator,"Ctrl <Key>I"); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    XtSetArg(args[n], XmNacceleratorText ,accText); n++;
    XtSetArg(args[n], XmNmnemonic, 'I'); n++;
    c[0] = helpBtn1 = XmCreatePushButtonGadget(menupane3, "helpBtn1", args, n);
    XtAddCallback(helpBtn1, XmNactivateCallback, OnItemCB, NULL);
    XmStringFree(accText);
    XmStringFree(labelStr);

    /*
     * On Application menu button
     */
    labelStr = XmStringCreateLtoR("On Application",XmSTRING_DEFAULT_CHARSET);
    accText = XmStringCreateLtoR("<Ctrl>A",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNaccelerator,"Ctrl <Key>A"); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    XtSetArg(args[n], XmNacceleratorText ,accText); n++;
    XtSetArg(args[n], XmNmnemonic, 'A'); n++;
    c[1] = helpBtn2 = XmCreatePushButtonGadget(menupane3,
					       "helpBtn2", args, n);
    XtAddCallback(helpBtn2, XmNactivateCallback, OnApplicationCB, NULL);
    XmStringFree(accText);
    XmStringFree(labelStr);

    /*
     * On Help menu button
     */
    labelStr = XmStringCreateLtoR("On Help",XmSTRING_DEFAULT_CHARSET);
    accText = XmStringCreateLtoR("<Ctrl>H",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNaccelerator,"Ctrl <Key>H"); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    XtSetArg(args[n], XmNacceleratorText ,accText); n++;
    XtSetArg(args[n], XmNmnemonic, 'H'); n++;
    c[2] = helpBtn3 = XmCreatePushButtonGadget(menupane3, "helpBtn3", args, n);
    XtAddCallback(helpBtn3, XmNactivateCallback, OnHelpCB, NULL);
    XmStringFree(accText);
    XmStringFree(labelStr);

    /*
     * On Keys menu button
     */
    labelStr = XmStringCreateLtoR("On Keys",XmSTRING_DEFAULT_CHARSET);
    accText = XmStringCreateLtoR("<Ctrl>K",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNaccelerator,"Ctrl <Key>K"); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    XtSetArg(args[n], XmNacceleratorText ,accText); n++;
    XtSetArg(args[n], XmNmnemonic, 'K'); n++;
    c[3] = helpBtn4 = XmCreatePushButtonGadget(menupane3, "helpBtn4", args, n);
    XtAddCallback(helpBtn4, XmNactivateCallback, OnKeysCB, NULL);
    XmStringFree(accText);
    XmStringFree(labelStr);

    /*
     * On Version menu button
     */
    accText = XmStringCreateLtoR("<Ctrl>V",XmSTRING_DEFAULT_CHARSET);
    labelStr = XmStringCreateLtoR("On Version",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNaccelerator,"Ctrl <Key>V"); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    XtSetArg(args[n], XmNacceleratorText ,accText); n++;
    XtSetArg(args[n], XmNmnemonic, 'V'); n++;
    c[4] = helpBtn5 = XmCreatePushButtonGadget(menupane3, "helpBtn5",args,n);
    XtAddCallback(helpBtn5, XmNactivateCallback, OnVersionCB, NULL);
    XtManageChildren(c, 5);
    XmStringFree(accText);
    XmStringFree(labelStr);


    /*
     * Create Cascade buttons for menubar
     */
    labelStr = XmStringCreateLtoR("File",XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNmnemonic, 'F'); n++;
    XtSetArg(args[n], XmNsubMenuId, menupane1); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    c[0] = cascadeB1 = XmCreateCascadeButton(menuBar, "cascadeB1", args, n);
    XmStringFree(labelStr);

    labelStr = XmStringCreateLtoR("Actions", XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNmnemonic, 'A'); n++;
    XtSetArg(args[n], XmNsubMenuId, menupane2); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    c[1] = cascadeB2 = XmCreateCascadeButton(menuBar, "cascadeB2", args, n);
    XmStringFree(labelStr);

    labelStr = XmStringCreateLtoR("Help", XmSTRING_DEFAULT_CHARSET);
    n = 0;
    XtSetArg(args[n], XmNmnemonic, 'H'); n++;
    XtSetArg(args[n], XmNsubMenuId, menupane3); n++;
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    c[2] = cascadeB3 = XmCreateCascadeButton(menuBar, "cascadeB3", args, n);
    XmStringFree(labelStr);

    XtManageChildren(c, 3);
   
    /*
     * Attaches the Help menupane to the right side
     */
    n = 0;
    XtSetArg (args[n], XmNmenuHelpWidget, cascadeB3);  n++;
    XtSetValues (menuBar, args, n);

    return(menuBar);
}

/***************************************************************************
 * Function:  VoidCB
 *
 ***************************************************************************/
static void VoidCB(widget, client_data, event)
Widget  widget;
caddr_t client_data;
XEvent  event;
{
 /* this routine does nothing.
  * it simply exists to provide a dummy routine for adding events to the
  * display window so context sensitive help will work.
  */
}


/*****************************************************************************
 *
 *  Widget CreateApplication(XtAppContext app,
 *                           Display *display)
 *
 *
 *  Description:
 *  -----------
 *  After the application shell has been created, this creates the main
 *  window
 *
 *
 *  Inputs:
 *  ------
 *  app = application context for the application
 *  parent = top level shell for application
 *
 * 
 *  Outputs:
 *  -------
 *  mainWindow = main window for the application
 *
 *
 *  Comments:
 *  --------
 * 
 ***************************************************************************/
Widget CreateApplication (app, display) 
XtAppContext 	app;
Display		*display;
{
    Widget	mainWindow;	/*  MainWindow		*/
    Widget	menuBar;	/*  MenuBar		*/
    Widget	frame;		/*  Frame		*/
    Widget	form;
    Arg		args[MAX_ARGS];	/*  arg list		*/
    int      	n;		/*  arg count		*/
    char	name[15];	/*  name string		*/
    int		len;		/*  string length	*/
    Widget	hsb, vsb;	/*  ScrollBars		*/
       
    Pixel	foreground;
    Pixel	background;
       
    Colormap colormap;
    Pixel selectColor;
    Pixel topShadowColor;
    Pixel bottomShadowColor;
    Pixel foregroundColor;

    XImage *tmpImg;

    /*
     * Create MainWindow.
     */
    n = 0;
    mainWindow = XmCreateMainWindow(appShell, "mainwindow1", args, n);
    XtManageChild (mainWindow);

    /*
     * Create MenuBar in MainWindow.
     */ 
    menuBar = BuildMenus(mainWindow);


    /*
     * Create Frame MainWindow and ScrolledWindow in Frame.
     */
    n = 0;
    XtSetArg (args[n], XmNmarginWidth, 2);  n++;
    XtSetArg (args[n], XmNmarginHeight, 2);  n++;
    XtSetArg (args[n], XmNshadowThickness, 1);  n++;
    XtSetArg (args[n], XmNshadowType, XmSHADOW_OUT);  n++;
    frame = XmCreateFrame (mainWindow, "frame", args, n);
    XtManageChild (frame);

        
    /*
     * Create the form to hold the scrolled window
     */
    n = 0;
    form = XmCreateForm(frame, "form", args, n);
    XtManageChild(form);

    /*
     * Get the select color and margin widths and heights
     */
    n = 0;
    XtSetArg(args[n], XmNforeground, &foreground);n++;
    XtSetArg(args[n], XmNbackground, &background);n++;
    XtSetArg (args[n], XmNcolormap,  &colormap);n++;
    XtGetValues(form, args, n);

    XmGetColors (XtScreen (form), colormap, background,
		 &foregroundColor, &topShadowColor,
		 &bottomShadowColor, &selectColor);

    /*
     * Create the label with the default bitmap
     */
    InstallBitmapDataIntoXmCache("ballImg", ball_bits);
    InstallBitmapDataIntoXmCache("noBallImg", no_ball_bits);
    ballPix = XmGetPixmap(XDefaultScreenOfDisplay(display), "ballImg",
			  foreground, background);
    noBallPix = XmGetPixmap(XDefaultScreenOfDisplay(display), "noBallImg",
			    foreground, background);
    n = 0;
    XtSetArg(args[n], XmNlabelType, XmPIXMAP); n++;
    if(!strcmp(ballState, "off"))
    {
	XtSetArg(args[n], XmNlabelPixmap, noBallPix); n++;
	XtSetArg(args[n], XmNlabelInsensitivePixmap, noBallPix); n++;   
    }
    else
    {
	XtSetArg(args[n], XmNlabelPixmap, ballPix); n++;
	XtSetArg(args[n], XmNlabelInsensitivePixmap, ballPix); n++;   
    }
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;   
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;   
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;      
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;      
    XtSetArg(args[n], XmNrecomputeSize, False); n++;      
    displayArea = XmCreateLabel(form, "ballDisplay", args, n);
    XtManageChild(displayArea);

    XtAddEventHandler(displayArea,
		ButtonPressMask | ButtonReleaseMask | KeyPressMask,
			FALSE, VoidCB, (caddr_t) NULL);
    /*
     * Set MainWindow areas 
     */
    XmMainWindowSetAreas (mainWindow, menuBar, NULL, NULL, NULL,
			  frame);
      
    /*
     * Force a size to the form
     */
    n = 0;
    XtSetArg(args[n], XmNwidth, 260); n++;
    XtSetArg(args[n], XmNheight, 100); n++;      
    XtSetValues(form, args, n);
    
    /*
     * Return the main window
     */
    return (mainWindow);
}



/*************************************<->*************************************
 *
 * void QuitCB (Widget w,
 *              caddr_t client_data,
 *              caddr_t call_data) 
 *
 *
 *  Description:
 *  -----------
 *  When a user requests that the application exit, this callback is called,
 *  and it exits the application.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  THIS IS THE APPLICATION'S EXIT POINT
 * 
 *************************************<->***********************************/
void QuitCB (w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from applicaiton   */
caddr_t		call_data;	/*  data from widget class  */
{

    /*
     * Terminate the application.
     */
    exit (0);
}




/*************************************<->*************************************
 *
 * void SaveStatetCB (Widget w,
 *                    caddr_t client_data,
 *                    caddr_t call_data) 
 *
 *
 *  Description:
 *  -----------
 *  When the Session Manager is saving a session, this routine is called
 *  to notify the application to update its state, and then update its
 *  WM_COMMAND property.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void SaveStateCB (w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from applicaiton   */
caddr_t		call_data;	/*  data from widget class  */
{
    char      *fileName, *filePath;
    FILE      *stateFile;
    int       newArgc;
    char      *newArgv[10], sessionOpt[10];

    /*
     * Write out state information to file Session Manager gives
     * name of
     */
    if(DtSessionSavePath(w, &filePath, &fileName))
    {
	stateFile = fopen(filePath, "w");
	if(stateFile != NULL)
	{
	    fprintf(stateFile, "%s", ballState);
	    (void) fclose(stateFile);
	}
	else
	{
	    XtError("Could not save session information");
	    fileName = NULL;
	    filePath = NULL;
	}
	
    }
    else
    {
	XtError("Could not save session information");
	fileName = NULL;
	filePath = NULL;
    }

    /*
     * Update the WM_COMMAND no matter what so that the session
     * manager won't continue to wait for it to be updated
     */
    if(fileName != NULL)
    {
	newArgv[0] = (char *) client_data;
	strcpy(sessionOpt, "-session");
	newArgv[1] = (char *) sessionOpt;
	newArgv[2] = (char *) fileName;
	newArgc = 3;
	XSetCommand(XtDisplay(w), XtWindow(appShell), newArgv, newArgc);
	XtFree(fileName);
	XtFree(filePath);
    }
    else
    {
	newArgv[0] = NULL;
	XSetCommand(XtDisplay(w), XtWindow(appShell), 0, NULL);
    }

    XSync(XtDisplay(w), 0);
}



/*************************************<->*************************************
 *
 * void RestoreAppState (char *restoreFile)
 *
 *
 *  Description:
 *  -----------
 *  When we are starting in a state we wish to restore - this routine
 *  reads the restoration file, and restores the application to its correct
 *  state.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void RestoreAppState(restoreFile)
char *restoreFile;
{
    char      *restorePath, stateInfo[10], *string;
    FILE      *stateFile;
    int	      i;

    /*
     * If path can't be created - we're done
     */
    if(!DtSessionRestorePath(appShell, &restorePath, restoreFile))
    {
	XtError("Could not allocate memory for restore path");
	strcpy(ballState, "on");
	return;
    }

    stateFile = fopen(restorePath, "r");

    /*
     * If the file is not there - print a warning and use default
     */
    if(stateFile == NULL)
    {
	XtWarning("State could not be restored");
	strcpy(ballState, "on");
	XtFree(restorePath);
	return;
    }
     

    string = fgets(stateInfo, 9, stateFile);

    /*
     * Copy all but newlines and NULL to state info
     */
    for(i = 0;((i < 9) && (*string != NULL) && (*string != '\n'));i++) 
    {
	ballState[i] = *string++;
	ballState[i+1] = NULL;
    }

    /*
     * If this is not one of the accepted states -
     * default it to "on"
     */
    if((strcmp(ballState, "on")) && (strcmp(ballState, "off")))
    {
	strcpy(ballState, "on");
    }

    fclose(stateFile);
    XtFree(restorePath);
}



/*************************************<->*************************************
 *
 *  void ClearCB (Widget pbw, caddr_t client_data, caddr_t call_data)
 *
 *
 *  Description:
 *  -----------
 *  When the Remove Ball option is chosen, clear the screen
 *
 *
 *  Inputs:
 *  ------
 *  actionBtn1 - (global) Show Ball button widget
 *  actionBtn2 - (global) Remove Ball button widget
 *  displayArea - (global) Area to display ball
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void ClearCB (pbw, client_data, call_data)
Widget pbw;
caddr_t client_data;
caddr_t call_data;

{
    Arg	    args[10];	
    int       n;

    /*
     * Turn "Show Ball" option on
     */
    n = 0;
    XtSetArg (args[n], XmNsensitive, True);  n++;
    XtSetValues (actionBtn1, args, n);

    /*
     * Turn "Remove Ball" option off
     */
    n = 0;
    XtSetArg (args[n], XmNsensitive, False);  n++;
    XtSetValues (actionBtn2, args, n);

    /*
     * Remove the ball
     */
    n = 0;
    XtSetArg(args[n], XmNlabelPixmap, noBallPix); n++;   
    XtSetValues (displayArea, args, n);


    /*
     *  Set the global variable to give the state of the ball
     */
    strcpy(ballState, "off");
}


/*************************************<->*************************************
 *
 *  void ShowCB (Widget pbw, caddr_t client_data, caddr_t call_data)
 *
 *
 *  Description:
 *  -----------
 *  When the Show Ball option is chosen, display the ball
 *
 *
 *  Inputs:
 *  ------
 *  actionBtn1 - (global) Show Ball button widget
 *  actionBtn2 - (global) Remove Ball button widget
 *  displayArea - (global) Area to display ball
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void ShowCB (pbw, client_data, call_data)
Widget pbw;
caddr_t client_data;
caddr_t call_data;

{
    Arg	    args[10];	
    int       n;

    /*
     * Turn "Show Ball" option off
     */
    n = 0;
    XtSetArg (args[n], XmNsensitive, False);  n++;
    XtSetValues (actionBtn1, args, n);

    /*
     * Turn "Remove Ball" option on
     */
    n = 0;
    XtSetArg (args[n], XmNsensitive, True);  n++;
    XtSetValues (actionBtn2, args, n);

    /*
     * Show the ball
     */
    n = 0;
    XtSetArg(args[n], XmNlabelPixmap, ballPix); n++;   
    XtSetValues (displayArea, args, n);

    /*
     *  Set the global variable to give the state of the ball
     */
    strcpy(ballState, "on");
    
}



/*************************************<->*************************************
 *
 *  void OnVersionCB(Widget widget,
 *                   caddr_t clientData,
 *                   caddr_t callData)
 *
 *
 *  Description:
 *  -----------
 *  When the user requests "Help on Version" from the help menu, this
 *  callback is called, so that the help system can be invo
 *
 *
 *  Inputs:
 *  ------
 *  widget - the widget chosen (version button), gives the help system a
 *           "handle" into the application.
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void OnVersionCB( widget, clientData, callData)
Widget widget;
caddr_t clientData, callData;
{
    char     *messageString = "HP DT Example Program: Version 1.0";

    /*
     * Invoke the Help Manager to display version information
     * We send the parent of widget because widget is a gadget
     */
    DtHelpOnVersion(XtParent(widget), messageString);
}




/*************************************<->*************************************
 *
 *  void OnItemCB(Widget widget,
 *                caddr_t clientData,
 *                caddr_t callData)
 *
 *
 *  Description:
 *  -----------
 *  When the user requests "Help On Item" from the Applications help menu,
 *  this callback is called to invoke the Help Manager to show help information
 *  on whatever component the user chooses.
 *
 *
 *  Inputs:
 *  ------
 *  widget - the widget chosen (on item button) gives the help system a
 *           "handle" into the application.
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void OnItemCB( widget, clientData, callData)
Widget widget;
caddr_t clientData, callData;
{
  
    /*
     * Invoke the Help Manager to display item information
     * We send the parent of widget because widget is a gadget
     */
    DtHelpOnItem(XtParent(widget));
}



/*************************************<->*************************************
 *
 *  void OnApplicationCB(Widget widget,
 *                       caddr_t clientData,
 *                       caddr_t callData)
 *
 *
 *  Description:
 *  -----------
 *  When the user requests "Help On Application"
 *  from the Applications help menu,
 *  this callback is called to invoke the Help Manager to show help information
 *  on this application.
 *
 *
 *  Inputs:
 *  ------
 *  widget - the widget chosen (on app button) gives the help system a
 *           "handle" into the application.
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void OnApplicationCB( widget, clientData, callData)
Widget widget;
caddr_t clientData, callData;
{
    char *helpFile="ex_appl";
  
    /*
     * Invoke the Help Manager to display version information
     * We send the parent of widget because widget is a gadget
     */
    DtHelpOnTopic(XtParent(widget), helpFile);
}



/*************************************<->*************************************
 *
 *  void OnKeysCB(Widget widget,
 *                caddr_t clientData,
 *                caddr_t callData)
 *
 *
 *  Description:
 *  -----------
 *  Callback for the Application Help button in the Help Browser.
 *
 *
 *  Inputs:
 *  ------
 *  widget - the widget chosen gives the help system a
 *           "handle" into the application.
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
 void OnKeysCB(widget, clientData, callData)
 Widget widget;
 caddr_t clientData, callData;
{
    char *helpFile="ex_keys";
  
    /*
     * We send the parent of widget because widget is a gadget
     */
    DtHelpOnTopic(XtParent(widget), helpFile);

}



/*************************************<->*************************************
 *
 *  void OnHelpCB(Widget widget,
 *                caddr_t clientData,
 *                caddr_t callData)
 *
 *
 *  Description:
 *  -----------
 *  When the user requests "Help On Help" from the Applications help menu,
 *  this callback is called to invoke the Help Manager to show help information
 *  on whatever component the user chooses.
 *
 *
 *  Inputs:
 *  ------
 *  widget - the widget chosen (on help button) gives the help system a
 *           "handle" into the application.
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void OnHelpCB( widget, clientData, callData)
Widget widget;
caddr_t clientData, callData;
{

    /*
     * Invoke the Help Manager to display help information
     * We send the parent of widget because widget is a gadget
     */
    DtHelpOnHelp(XtParent(widget));
}



/*************************************<->*************************************
 *
 *  InstallBitmapDataIntoXmCache (bmName, bmData)
 *
 *
 *  Description:
 *  -----------
 *  Installs built-in bitmap data into the Xm Pixmap cache. The image
 *  may be retrieved later by a call to XmGetPixmap.
 *
 *  Inputs:
 *  ------
 *  bmName - name of the bitmap used for retrieving pixmaps
 *  bmData - data for the bitmap
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  This is principally for putting built-in pixmap data into the Xm
 *  cache to allow for uniform access to pixmap creation.
 *
 *  ***WARNING***
 *  Do NOT call XmDestroyPixmap on images cached via this routine unless
 *  pData passed in points to malloc'ed memory. XmDestroyPixmap could 
 *  try to free this data.
 *			
 *************************************<->***********************************/
void InstallBitmapDataIntoXmCache(bmName, bmData)
char *bmName;
char *bmData;
{
    XImage *pImage;

    if (pImage = (XImage *) malloc (sizeof (XImage)))
    {
	pImage->width = ball_width; 
	pImage->height = ball_height;
	pImage->xoffset = 0;
	pImage->data = bmData;
	pImage->format = XYBitmap;
	pImage->byte_order = MSBFirst;
	pImage->bitmap_pad = 8;
	pImage->bitmap_bit_order = LSBFirst;
	pImage->bitmap_unit = 8;
	pImage->depth = 1;
	pImage->bytes_per_line = (ball_width/8) +
	    ((ball_width%8) != 0 ? 1 : 0);
	pImage->obdata = NULL;

	XmInstallImage(pImage, bmName);
    }

}
