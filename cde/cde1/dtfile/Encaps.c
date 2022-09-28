/* $XConsortium: Encaps.c /main/cde1_maint/3 1995/10/09 14:47:24 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           Encaps.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    Source file for the dialog encapsulation functions.
 *
 *   FUNCTIONS: DataChangeCallback
 *		DataCloseCallback
 *		DialogStructureNotifyHandler
 *		IntDialogGetResources
 *		IntDialogPutResources
 *		SetIconifyState
 *		TimerEvent
 *		_DtBooleanToString
 *		_DtBuildDialog
 *		_DtChildPosition
 *		_DtDialogGetResources
 *		_DtDialogPutResources
 *		_DtDimensionToString
 *		_DtEncapSetWorkSpaceHints
 *		_DtFreeDialogData
 *		_DtGetDefaultDialogData
 *		_DtGetDialogInstance
 *		_DtGetDialogShell
 *		_DtGetInstanceData
 *		_DtGetResourceDialogData
 *		_DtHideDialog
 *		_DtInitializeEncapsulation
 *		_DtInstallDialog
 *		_DtIntToString
 *		_DtIsDialogShowing
 *		_DtPositionToString
 *		_DtShowBuiltDialog
 *		_DtShowDialog
 *		_DtStringToString
 *		_DtWriteDialogData
 *		_DtXmStringTableToString
 *		_DtXmStringToString
 *		_DtmapCB
 *		_DtChangeTo
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/


#include <string.h>
#include <Xm/XmP.h>
#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/AtomMgr.h>
#include <Xm/MwmUtil.h>
#include <Xm/VendorSEP.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>

#include <Dt/Wsm.h>
#include <Dt/DtNlUtils.h>

#include "Encaps.h"
#include "Filter.h"
#include "ModAttr.h"

#ifdef _TT_MULTI_SCREEN 
#include "Prefs.h"
#include "FileMgr.h"
#include "Desktop.h"
#include "Main.h" /* For gTTScreenNum */
#endif 

#define MAX_NAME_LIST_SIZE	25
#define MAX_RESOURCE_LENGTH	256


/*  Cache array handling defines, structure, and global statics  */

#define INCREMENT_SIZE	10


typedef struct _Dialog
{
   Boolean                 in_use;
#ifdef _TT_MULTI_SCREEN
   int                     screenNum;
#endif
   Widget                  dialog_widget;
   XtPointer               dialog;
   DialogData            * dialog_data;
   DialogChangedProc       change;
   XtPointer               change_client_data;
   DialogClosedProc        close;
   XtPointer               close_client_data;
   struct _Dialog        * next;
} Dialog;


typedef struct
{
   DialogClass * class;
   Dialog      * dialog_list;
   Boolean       cache;
   Boolean       destroyPopups;
} ClassSet;

extern int filter_dialog,mod_attr_dialog;

static ClassSet * class_set = NULL;
static int class_set_size = 0;
static int num_classes = 0;
#ifdef _TT_MULTI_SCREEN
static int sMultiInitialized = False;
#ifdef _TT_TURNED_OFF /* No TimerEvent for dialog caching */
static int* sNumberOfDialogMapped;
#endif /* _TT_TURNED_OFF */
#else
static int NumberOfDialogMapped = 0;
#endif

static char * resourceBuf = NULL;
static int commonResourceCount = 5;
static DialogResource commonResources[] =
{
   { "displayed", XmRBoolean, sizeof(Boolean),
     XtOffset(DialogInstanceDataPtr, displayed),
     (caddr_t) False, _DtBooleanToString },

   { "x", XmRPosition, sizeof(Position),
     XtOffset(DialogInstanceDataPtr, x),
     (caddr_t) 0, _DtPositionToString},

   { "y", XmRPosition, sizeof(Position),
      XtOffset(DialogInstanceDataPtr, y),
      (caddr_t) 0, _DtPositionToString },

   { "width", XmRDimension, sizeof(Dimension),
     XtOffset(DialogInstanceDataPtr, width),
     (caddr_t) 0, _DtDimensionToString },

   { "height", XmRDimension, sizeof(Dimension),
      XtOffset(DialogInstanceDataPtr, height),
      (caddr_t) 0, _DtDimensionToString },
};

#ifdef _TT_MULTI_SCREEN
static Widget* sParentShell;
#else
static Widget encap_parent_shell;
#endif

extern int file_mgr_dialog;

#ifdef _TT_TURNED_OFF /* No TimerEvent for dialog caching */

/*  Timer globals */
/*
 * initialTimeoutLength is used to specify how long to wait before initially
 *    kicking of the building of the dialog cache.
 * activeTimeoutLength is used to specify how long to wait after adding
 *    a dialog to the cache, before adding the next dialog.
 * idleTimeoutLength is used to specify how long to wait after all dialogs
 *    have been built, and the cache is full.
 */
int initialTimeoutLength = 180000;	/* 3 minutes, in milliseconds */
int activeTimeoutLength = 30000;	/* 30 seconds, in milliseconds */
int idleTimeoutLength = 900000;		/* 15 minutes, in milliseconds */

#define TIMER_STARTUP_STATE  0
#define TIMER_ACTIVE_STATE   1
#define TIMER_IDLE_STATE     2

#ifdef _TT_MULTI_SCREEN

static int* 		sTimerState;
static XtIntervalId* 	sTimerId;

#else

static int timerState = TIMER_STARTUP_STATE;
static XtIntervalId timerId = 0;

#endif /* _TT_MULTI_SCREEN */

#endif /* _TT_TURNED_OFF */

/*
 * This global is used when positioning a dialog ABOVE (not below) its
 * parent window.  The 'y' position is defined to be the 'y' position
 * of the parent, minus the 'height' of the dialog, minus the value
 * assigned to 'topPositionOffset'; this allows the application to control
 * how much, if any, of the parent's title bar is covered.
 */
int topPositionOffset = 20;


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void DialogStructureNotifyHandler();
static void SetIconifyState() ;
static void DataChangeCallback() ;
static void DataCloseCallback() ;
static void TimerEvent() ;
static void IntDialogGetResources() ;
static void IntDialogPutResources() ;

#else

static void DialogStructureNotifyHandler(
                        Widget w,
                        XtPointer client_data,
                        XEvent *event );
static void SetIconifyState( 
                        Widget shell,
                        Boolean iconify,
                        Boolean UseIconX,
                        int IconX,
                        Boolean UseIconY,
                        int IconY);
static void DataChangeCallback( 
                        Widget widget,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void DataCloseCallback( 
                        Widget widget,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void TimerEvent( 
                        Widget widget,
                        XtIntervalId *id) ;
static void IntDialogGetResources(
                        XrmDatabase database,
                        char *base,
                        DialogResource *resource,
                        XrmName *xrmName,
                        int nameCount,
                        XrmQuark stringQuark) ;
static void IntDialogPutResources(
                        int fd,
                        char **nameList,
                        char *dialogName,
                        char *base,
                        DialogResource *resource) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/



/************************************************************************
 *
 *  _DtInitializeEncapsulation
 *	This function is used to initialize the dialog encapsulation.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtInitializeEncapsulation( display, name, class )
        Display *display ;
        char *name ;
        char *class ;
#else
_DtInitializeEncapsulation(
        Display *display,
        char *name,
        char *class )
#endif /* _NO_PROTO */

{
   Arg args[5];

   /*  Create an application shell that will never be    */
   /*  displayed that is used as a parent of all of the  */
   /*  dialog created.                                   */
#ifdef _TT_MULTI_SCREEN
   Screen* 	screen;
   int		scr;
   int		numScreens = XScreenCount(display);
   Widget	encap_parent_shell;

   /* Initialize multiscreen vars */
   /* This probably isn't the best place to do it, but it'll work */
   if (! sMultiInitialized )  
   {
      sParentShell = (Widget*) XtCalloc(numScreens, sizeof(Widget)); 
# ifdef _TT_TURNED_OFF /* No TimerEvent for dialog caching */
      sTimerId = (XtIntervalId*) XtCalloc(numScreens, sizeof(XtIntervalId)); 
      sTimerState = (int*) XtCalloc(numScreens, sizeof(int)); 
      sNumberOfDialogMapped = (int*) XtCalloc(numScreens, sizeof(int)); 
# endif /* _TT_TURNED_OFF */
      sMultiInitialized = True;  	/* Only once */
   }

   /* Do for each screen */
   for (scr = 0; scr < numScreens ; scr++)
   {
      screen = XScreenOfDisplay(display, scr);
      XtSetArg(args[0], XmNscreen, screen);

      encap_parent_shell = sParentShell[scr] = XtAppCreateShell (name, class,
                           applicationShellWidgetClass, 
                           display, args, 1);
#else

   encap_parent_shell = XtAppCreateShell (name, class,
                                    applicationShellWidgetClass, 
                                    display, NULL, 0);
#endif

   /* Supposedly required to be ICCCM complient */
   XtSetArg(args[0], XmNmappedWhenManaged, False);
   XtSetArg(args[1], XmNwidth, 1);
   XtSetArg(args[2], XmNheight, 1);
   XtSetValues(encap_parent_shell, args, 3);
   XtRealizeWidget(encap_parent_shell);

   /*  Get a timer going that is used for auto creation of dialogs.  */

#ifdef _TT_TURNED_OFF /* No TimerEvent for dialog caching */

#ifdef _TT_MULTI_SCREEN
   sTimerState[scr] = TIMER_STARTUP_STATE;
   sTimerId[scr] = XtAppAddTimeOut(
                   XtWidgetToApplicationContext(sParentShell[scr]), 
                                   initialTimeoutLength, 
                                   (XtTimerCallbackProc) TimerEvent, 
                                   (XtPointer) sParentShell[scr]);
#else
   timerState = TIMER_STARTUP_STATE;
   timerId = XtAppAddTimeOut(XtWidgetToApplicationContext(encap_parent_shell), 
                             initialTimeoutLength, (XtTimerCallbackProc) 
                             TimerEvent, (caddr_t) encap_parent_shell);
#endif

#endif /* _TT_TURNED_OFF */

#ifdef _TT_MULTI_SCREEN
   }
#endif

   /* Allocate a buffer for writing out resource values */
   resourceBuf = XtMalloc(20);
}




/************************************************************************
 *
 *  _DtInstallDialog
 *	This function is used to register a dialog class with the
 *	encapsulation functions.
 *
 ************************************************************************/

int
#ifdef _NO_PROTO
_DtInstallDialog( dialog_class, cache, destroyPopups )
        DialogClass *dialog_class ;
        Boolean cache ;
        Boolean destroyPopups ;
#else
_DtInstallDialog(
        DialogClass *dialog_class,
        Boolean cache,
        Boolean destroyPopups )
#endif /* _NO_PROTO */

{
   /*  Allocate additional dialog cache space if needed  */

   if (num_classes == class_set_size)
   {
      class_set_size += INCREMENT_SIZE;
      class_set = 
         (ClassSet *) XtRealloc ((char *)class_set, 
                                 sizeof (ClassSet) * class_set_size);
   }

   class_set[num_classes].class = dialog_class;
   class_set[num_classes].dialog_list = NULL;
   class_set[num_classes].cache = cache;
   class_set[num_classes].destroyPopups = destroyPopups;

   num_classes++;

   return (num_classes - 1);
}

/************************************************************************
 *
 *  _DtGetDefaultDialogData
 *	This function is used to get a structure containing the
 *	default data for a particular dialog type.
 *
 ************************************************************************/

DialogData *
#ifdef _NO_PROTO
_DtGetDefaultDialogData( dialog_type )
        int dialog_type ;
#else
_DtGetDefaultDialogData(
        int dialog_type )
#endif /* _NO_PROTO */


{
   DialogData * dialog_data;

   dialog_data = (DialogData *) XtMalloc (sizeof (DialogData));

   dialog_data->type = dialog_type;
   dialog_data->data = (*(class_set[dialog_type].class->get_default_values))();

   return (dialog_data);
}




/************************************************************************
 *
 *  _DtGetResourceDialogData
 *	This function is used to get a structure containing the
 *	data for a particular dialog type from a resource data base.
 *
 ************************************************************************/

DialogData *
#ifdef _NO_PROTO
_DtGetResourceDialogData( dialog_type, data_base, name_list )
        int dialog_type ;
        XrmDatabase data_base ;
        char **name_list ;
#else
_DtGetResourceDialogData(
        int dialog_type,
        XrmDatabase data_base,
        char **name_list )
#endif /* _NO_PROTO */

{
   DialogData * dialog_data;

   dialog_data = (DialogData *) XtMalloc (sizeof (DialogData));

   dialog_data->type = dialog_type;
   dialog_data->data = (*(class_set[dialog_type].class->get_resource_values))
                          (data_base, name_list);

   return (dialog_data);
}




/************************************************************************
 *
 *  _DtShowDialog
 *	This functions is used to display an instance of a dialog with
 *	the provided data.  The functions and data to be set back to
 *	the application upon change or close of the dialog is also
 *	provided as parameters.
 *
 ************************************************************************/
void
#ifdef _NO_PROTO
_DtShowDialog(parent, map_parent, top_rec, dialog_data, change_proc,
              change_data, close_proc, close_data, workspaces,
              iconify_state, ignoreCache, title, classHints,
              UseIconX, IconX, UseIconY, IconY)
        Widget parent ;
        Widget map_parent ;
        XtPointer top_rec ;
        DialogData *dialog_data ;
        DialogChangedProc change_proc ;
        XtPointer change_data ;
        DialogClosedProc close_proc ;
        XtPointer close_data ;
        char *workspaces ;
        Boolean iconify_state ;
        Boolean ignoreCache ;
        char * title;
        XClassHint * classHints;
        Boolean UseIconX;
        int IconX;
        Boolean UseIconY;
        int IconY;
#else
_DtShowDialog(
        Widget parent,
        Widget map_parent,
        XtPointer top_rec,
        DialogData *dialog_data,
        DialogChangedProc change_proc,
        XtPointer change_data,
        DialogClosedProc close_proc,
        XtPointer close_data,
        char *workspaces,
        Boolean iconify_state,
        Boolean ignoreCache,
        char * title,
        XClassHint * classHints,
        Boolean UseIconX,
        int IconX,
        Boolean UseIconY,
        int IconY)
#endif /* _NO_PROTO */

{
   Dialog * dialog;
   int dialog_type, n;
   DialogInstanceData * instance_data;
   char geometry[40];
   Arg args[5];
   Boolean doCenter = False;
   Boolean doParentRelativePositioning = False;
   int availableDialogCount;

#if defined(_TT_MULTI_SCREEN)
   extern Widget toplevel;
   int display_screen;

   display_screen = (parent) ? 
	XScreenNumberOfScreen(XtScreen(parent)) : gTTScreenNum;
#endif

   /*  See if there is a cached, unused dialog of the correct type.  */

   dialog_type = dialog_data->type;
   dialog = NULL;

   if (!ignoreCache)
   {
      Dialog * availableDialog;

      availableDialog = class_set[dialog_type].dialog_list;
      availableDialogCount = 0;

      /*
       * In addition to looking for an available dialog in the cache to use,
       * we also want to count up the number of unused dialogs in the cache.
       * This lets us know it we need to restart the timer, to again build
       * up the cache.
       */
      while (availableDialog != NULL)
      {
#ifdef _TT_MULTI_SCREEN
         if ((availableDialog->in_use == False) &&
             (availableDialog->screenNum == display_screen))
         {
            if (dialog == NULL)
               dialog = availableDialog;
            else
               availableDialogCount++;
         }
#else
         if (availableDialog->in_use == False)
         {
            if (dialog == NULL)
               dialog = availableDialog;
            else
               availableDialogCount++;
         }
#endif
         availableDialog = availableDialog->next;
      }
   }

   if (dialog == NULL)
   {
      dialog = (Dialog *) XtMalloc (sizeof (Dialog));

#ifdef _TT_MULTI_SCREEN
     dialog->screenNum = display_screen;
      (*(class_set[dialog_type].class->create))
        (XtDisplay (sParentShell[display_screen]),
         sParentShell[display_screen],
         &(dialog->dialog_widget), &dialog->dialog);
#else
      (*(class_set[dialog_type].class->create))
         (XtDisplay (encap_parent_shell), encap_parent_shell,
         &(dialog->dialog_widget), &dialog->dialog);
#endif

      /* if this is a File Manager view we want to update the headers
       * (i.e icon_path, current_directory, status_line), now so that they
       * don't get managed up front.  This is to make the Application
       * Manager (toolbox) happy.  
       */
      if(dialog_type == file_mgr_dialog)
         UpdateHeaders(dialog->dialog, dialog_data->data, False);

      /*  Add the change and close callbacks into the dialog  */

      if (class_set[dialog_type].class->install_change_callback) 
         (*(class_set[dialog_type].class->install_change_callback))
            (dialog->dialog, DataChangeCallback, dialog);

      if (class_set[dialog_type].class->install_close_callback) 
         (*(class_set[dialog_type].class->install_close_callback))
            (dialog->dialog, DataCloseCallback, dialog);

      dialog->next = class_set[dialog_type].dialog_list;
      class_set[dialog_type].dialog_list = dialog;
   }
   else
   {
      if(dialog_type == mod_attr_dialog)
      {
        ModAttrRec *mr = (ModAttrRec *)dialog->dialog;
        ResetFlag(NULL,mr->ok);
        ResetFlag(NULL,mr->cancel);
      }
      else if(dialog_type == filter_dialog)
      {
        FilterRec *fr = (FilterRec *)dialog->dialog;
        ResetFlag(NULL,fr->ok);
        ResetFlag(NULL,fr->close);
      }
   }

#ifdef _TT_TURNED_OFF /* No TimerEvent for dialog caching */

   if ((!ignoreCache) && (class_set[dialog_type].cache) &&
#ifdef _TT_MULTI_SCREEN
       (availableDialogCount < 1) && 
       (sTimerState[display_screen] == TIMER_IDLE_STATE))
#else
       (availableDialogCount < 1) && (timerState == TIMER_IDLE_STATE))
#endif
   {
      /*
       * We need to reset the cache building timer, so that it gets kicked
       * off quickly, instead of after the longer idle delay.
       */

#ifdef _TT_MULTI_SCREEN
   if (sTimerId[display_screen])
      XtRemoveTimeOut(sTimerId[display_screen]);
   sTimerState[display_screen] = TIMER_ACTIVE_STATE;
   sTimerId[display_screen] = XtAppAddTimeOut(
                XtWidgetToApplicationContext(sParentShell[display_screen]),
                      activeTimeoutLength, (XtTimerCallbackProc) TimerEvent,
                      (XtPointer) sParentShell[display_screen]);
#else
      if (timerId)
        XtRemoveTimeOut(timerId);
      timerState = TIMER_ACTIVE_STATE;
      timerId = XtAppAddTimeOut(
                 XtWidgetToApplicationContext(encap_parent_shell),
                 activeTimeoutLength, (XtTimerCallbackProc) TimerEvent,
                 (XtPointer) encap_parent_shell);
#endif


   }
#endif /* _TT_TURNED_OFF */

   /* 
    * Set pointer to top dialog data in child of the shell.
    * This is needed to get help to work.
    */
   if (top_rec == NULL)
      top_rec = dialog->dialog;
   XtSetArg(args[0], XmNuserData, top_rec);
   XtSetValues(dialog->dialog_widget, args, 1);

   /* Need to add the map callback in relation to the parent */
   if (class_set[dialog_type].class->map)
   {
      /*
       * The map_parent parameter gives us the ability to position
       * the dialog relative to a window which is not the transientFor
       * parent.  This is used for the audio preview dialog.
       */
      if (map_parent == NULL)
         map_parent = parent;

      (*(class_set[dialog_type].class->map)) (map_parent, dialog->dialog);
   }


   /*  Set the dialog structure fields to the parameter data.  */

   dialog->in_use = True;
   dialog->dialog_data = dialog_data;
   dialog->change = change_proc;
   dialog->change_client_data = change_data;
   dialog->close = close_proc;
   dialog->close_client_data = close_data;


   instance_data = (DialogInstanceData *) dialog_data->data;

   /* If a special title has been specified, we need to set it now */
   if (title)
   {
      XtSetArg(args[0], XmNtitle, title);
      XtSetValues(XtParent(dialog->dialog_widget), args, 1);
   }

   /*  If this is a top level shell, get it realized  */

   if (XtIsSubclass (XtParent (dialog->dialog_widget), 
                     applicationShellWidgetClass))
   {
      if (XtIsRealized (XtParent (dialog->dialog_widget)) == False)
      {
         if (instance_data->displayed == True)
         {
            (void) sprintf (geometry, "=%dx%d+%d+%d", 
                            instance_data->width, instance_data->height,
                            instance_data->x, instance_data->y);

            XtSetArg (args[0], XmNgeometry, geometry);
            XtSetValues (XtParent (dialog->dialog_widget), args, 1);
         }
         else if ((instance_data->width != 0) && (instance_data->height != 0))
         {
	    n=0;
            XtSetArg (args[n], XmNwidth, instance_data->width); n++;
            XtSetArg (args[n], XmNheight, instance_data->height); n++;
            XtSetValues (XtParent (dialog->dialog_widget), args, n);
         }

         /* Toggle mappedWhenManaged to false */
         XtSetMappedWhenManaged(XtParent (dialog->dialog_widget), False);
         XtRealizeWidget (XtParent(dialog->dialog_widget));

         /* Set the proper workspaces if needed */
         _DtEncapSetWorkSpaceHints(XtParent(dialog->dialog_widget), workspaces);

         /* Set any application-specified class hints for the window */
         if (classHints)
         {
            XSetClassHint(XtDisplay(dialog->dialog_widget),
                          XtWindow(XtParent(dialog->dialog_widget)),
                          classHints);
         }
           
         /* Set the iconify state */
         SetIconifyState(XtParent(dialog->dialog_widget),
                         iconify_state, UseIconX, IconX, UseIconY, IconY);

         /* Map the window */
         XtSetMappedWhenManaged(XtParent (dialog->dialog_widget), True);
         XtPopup (XtParent (dialog->dialog_widget), XtGrabNone);
         XSync(XtDisplay(dialog->dialog_widget), False);
         
      }
      else
      {
         if (instance_data->displayed == True)
         {
            WMShellWidget wm = (WMShellWidget)XtParent(dialog->dialog_widget);

            wm->wm.size_hints.flags |= USPosition;
            XtSetArg (args[0], XmNx, instance_data->x);
            XtSetArg (args[1], XmNy, instance_data->y);
            XtSetArg (args[2], XmNwidth, instance_data->width);
            XtSetArg (args[3], XmNheight, instance_data->height);
            XtSetValues (XtParent (dialog->dialog_widget), args, 4);
         }
         else if ((instance_data->width != 0) && (instance_data->height != 0))
         {
	    n=0;
            XtSetArg (args[n], XmNwidth, instance_data->width); n++;
            XtSetArg (args[n], XmNheight, instance_data->height); n++;
            XtSetValues (XtParent (dialog->dialog_widget), args, n);
         }
       
         /* Set the proper workspaces if needed */
         _DtEncapSetWorkSpaceHints(XtParent(dialog->dialog_widget), workspaces);

         /* Set any application-specified class hints for the window */
         if (classHints)
         {
            XSetClassHint(XtDisplay(dialog->dialog_widget),
                          XtWindow(XtParent(dialog->dialog_widget)),
                          classHints);
         }
           
         /* Set the iconify state */
         SetIconifyState(XtParent(dialog->dialog_widget),
                         iconify_state, UseIconX, IconX, UseIconY, IconY);


         /* Map the window */
         XtPopup (XtParent (dialog->dialog_widget), XtGrabNone);
      }
   }
   else
   {
      if (instance_data->displayed == True)
      {
         XtSetArg (args[0], XmNx, instance_data->x);
         XtSetArg (args[1], XmNy, instance_data->y);
         XtSetArg (args[2], XmNwidth, instance_data->width);
         XtSetArg (args[3], XmNheight, instance_data->height);
         XtSetArg (args[4], XmNdefaultPosition, False);
         XtSetValues (dialog->dialog_widget, args, 5);
         XtRealizeWidget (dialog->dialog_widget);
      }
      else
      {
         XtSetArg (args[0], XmNdefaultPosition, False);
         XtSetValues (dialog->dialog_widget, args, 1);

         XtRealizeWidget (dialog->dialog_widget);

         if (parent)
         {
            /* Position relative to the parent dialog */
            /* 
             * Must be done after the set_values call, since the dialog
             * may get forced to a different size.
             */
            doParentRelativePositioning = True;
         }
         else
         {
            /* Center in the display */
            /* 
             * Must be done after the set_values call, since the dialog
             * may get forced to a different size.
             */
            doCenter = True;
         }
      }

      /*
       * Dialogs with no controlling parent window, need to have their
       * own workspace perperty set, if some workspaces have been requested.
       */
      if ((parent == NULL) && workspaces)
         _DtEncapSetWorkSpaceHints(XtParent(dialog->dialog_widget), workspaces);
   }


   /*  Set Values onto the dialog to set it to the correct data.  */

   (*(class_set[dialog_data->type].class->set_values))
      (dialog->dialog, dialog_data->data);

   /*
    * These two adjustments MUST be done AFTER the dialog's SetValues()
    * procedure is called.  This is due to the fact that the setvalues
    * may cause the dialog size to change, and since both of the following
    * positioning algorithms are dependent upon the dialog size, we want
    * to make sure that the correct size is used.
    */
   if (doCenter)
   {
      XtSetArg (args[0], XmNx,
          (Dimension)(WidthOfScreen(XtScreen(dialog->dialog_widget)) -
           dialog->dialog_widget->core.width) / (Dimension)2);
      XtSetArg (args[1], XmNy,
          (Dimension)(HeightOfScreen(XtScreen(dialog->dialog_widget)) -
           dialog->dialog_widget->core.height) / (Dimension)2);
      XtSetValues (dialog->dialog_widget, args, 2);
   }
   else if (doParentRelativePositioning)
   {
      XtSetArg (args[0], XmNx,
          parent->core.x + 
          (Dimension)(parent->core.width - dialog->dialog_widget->core.width) / (Dimension)2);
      XtSetArg (args[1], XmNy,
          parent->core.y + 
          (Dimension)(parent->core.height - dialog->dialog_widget->core.height) / (Dimension)2);
      XtSetValues (XtParent(dialog->dialog_widget), args, 2);
   }


   /*  Fix up the transient-for windowing information so that  */
   /*  the window manager will shuffle and iconify as a group  */

   if (parent != NULL) 
   {
      if (XtIsRealized(parent))
      {
         XSetTransientForHint (XtDisplay (parent), 
                            XtWindow (XtParent (dialog->dialog_widget)),
                            XtWindow (parent)); 
      }
   }
   else
   {
      if (!XtIsSubclass (XtParent (dialog->dialog_widget), 
                                     applicationShellWidgetClass))
      {
#ifdef _TT_MULTI_SCREEN
         XSetTransientForHint (XtDisplay (sParentShell[display_screen]),
                               XtWindow (XtParent (dialog->dialog_widget)),
                               XtWindow (sParentShell[display_screen]));

#else
         XSetTransientForHint (XtDisplay (encap_parent_shell), 
                               XtWindow (XtParent (dialog->dialog_widget)),
                               XtWindow (encap_parent_shell)); 
#endif
      }
   }


   /*  Display the dialogs, application shells are displayed above.  */

   if (!(XtIsSubclass (XtParent (dialog->dialog_widget), 
                       applicationShellWidgetClass)))
   {
      XtManageChild (dialog->dialog_widget);
   }
   

   /*  Set the dialog instance data to indicate that the dialog  */
   /*  is displayed.                                             */

   ((DialogInstanceData *) (dialog_data->data))->displayed = True;


   /* Give the dialog a chance to set its focus widget, if necessary */

   if (class_set[dialog_data->type].class->set_focus)
   {
      (*(class_set[dialog_data->type].class->set_focus))
         (dialog->dialog, dialog_data->data);
   }


#ifdef _TT_TURNED_OFF /* No TimerEvent for dialog caching */

   XtAddEventHandler( XtParent( dialog->dialog_widget ),
                      StructureNotifyMask,
                      False,
                      (XtEventHandler)DialogStructureNotifyHandler,
                      NULL );
#endif
}




/************************************************************************
 *
 *  _DtHideDialog
 *	This function is used to undisplay a dialog.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtHideDialog( dialog_data, call_callbacks )
        DialogData *dialog_data ;
        Boolean call_callbacks ;
#else
_DtHideDialog(
        DialogData *dialog_data,
        Boolean call_callbacks )
#endif /* _NO_PROTO */

{
   Dialog * dialog;
   DialogData * new_data;
   CorePart * core;
   register int i;


   /*  Find the dialog and then hide it.  */

   dialog = class_set[dialog_data->type].dialog_list;

   while (dialog != NULL)
   {
      if (dialog->dialog_data == dialog_data && dialog->in_use == True)
      {

         /*  Free up any dialogs attached to the dialog widget  */

         if(class_set[dialog_data->type].destroyPopups)
         {
            core = (CorePart *) (XtParent (dialog->dialog_widget));

            for (i = core->num_popups - 1; i >= 0; i--)
               XtDestroyWidget (core->popup_list[i]);
         }


         /*  Get the dialog down, invoke the close callbacks, and  */
         /*  take it out of use.                                   */

         if (XtIsSubclass (XtParent (dialog->dialog_widget), 
                           applicationShellWidgetClass))
         {
            ShellWidget shell_widget;

            /*
             * If we had been iconified, then our popped_up flag will
             * have been cleared by the vendor shell.  When we call
             * XtPopdown(), it will see that we are no longer popped
             * up, and will not notify the window manager that our
             * icon should be removed; this can cause a subsequent
             * core dump if the user later tries to deiconify the window.
             * This fix should not be necessary once the toolkit is
             * fixed to properly track the shell's state.
             */
            shell_widget = (ShellWidget) XtParent(dialog->dialog_widget);
            shell_widget->shell.popped_up = True;

            XtPopdown ((Widget)shell_widget);
         }
         else
         {
            /* 
             * The following is for the condition described above.  However,
             * for a dialog shell, what happens it that the 'managed' flag
             * was set to 'False' when the windows were iconified (apparently
             * by the dialog shell), and when we tell the intrinsics to
             * really unmanage the child, it thinks it already has, so 
             * nothing happens; as a result, the dialog shell is left with
             * its 'popped_up' flag set to 'True'.  The next time we try
             * to post this dialog, the intrinsics think that it is already
             * up, so it does nothing.
             */
            dialog->dialog_widget->core.managed = True;
            XtUnmanageChild (dialog->dialog_widget);
         }


         /*  Set the dialog data to hidden  */

         ((DialogInstanceData *) (dialog->dialog_data->data))->displayed = False;

         
         if (call_callbacks && dialog->close)
         {
            new_data = (DialogData *) XtMalloc (sizeof (DialogData));

            new_data->type = dialog->dialog_data->type;
            new_data->data = NULL;
            if (class_set[new_data->type].class->get_values)
            {
               new_data->data = 
               (*(class_set[new_data->type].class->get_values))(dialog->dialog);
            }

            dialog->in_use = False;
            if (new_data->data)
               ((DialogInstanceData *) (new_data->data))->displayed = False;

            (*(dialog->close))
                  (dialog->close_client_data, dialog->dialog_data, new_data);
         }
         else
         {
            dialog->in_use = False;
            ((DialogInstanceData *) (dialog->dialog_data->data))->displayed = False;
         }

         break;
      }

      dialog = dialog->next;
   }
}




/************************************************************************
 *
 *  _DtGetDialogShell
 *	This function is used return the shell widget of a dialog that
 *	is currently displayed.
 *
 ************************************************************************/

Widget
#ifdef _NO_PROTO
_DtGetDialogShell( dialog_data )
        DialogData *dialog_data ;
#else
_DtGetDialogShell(
        DialogData *dialog_data )
#endif /* _NO_PROTO */

{
   Dialog * dialog;


   /*  Find the dialog and then return the shell.  */

   dialog = class_set[dialog_data->type].dialog_list;

   while (dialog != NULL)
   {
      if (dialog->dialog_data == dialog_data)
         return (dialog->dialog_widget);
      else
         dialog = dialog->next;
   }

   return (NULL);
}




/************************************************************************
 *
 *  _DtGetDialogInstance
 *	This function is used return the dialog instance structure
 *	of a currently in use dialog.
 *
 ************************************************************************/

XtPointer
#ifdef _NO_PROTO
_DtGetDialogInstance( dialog_data )
        DialogData *dialog_data ;
#else
_DtGetDialogInstance(
        DialogData *dialog_data )
#endif /* _NO_PROTO */

{
   Dialog * dialog;


   /*  Find the dialog and then return the instance  */

   dialog = class_set[dialog_data->type].dialog_list;

   while (dialog != NULL)
   {
      if (dialog->dialog_data == dialog_data)
         return (dialog->dialog);
      else
         dialog = dialog->next;
   }

   return (NULL);
}




/************************************************************************
 *
 *  _DtGetInstanceData
 *	This function is used return the dialog data structure contained
 *	within the cache structure referenced by instance.
 *
 ************************************************************************/

DialogData *
#ifdef _NO_PROTO
_DtGetInstanceData( instance )
        XtPointer instance ;
#else
_DtGetInstanceData(
        XtPointer instance )
#endif /* _NO_PROTO */

{
   int i;
   Dialog * dialog;


   /*  Find the dialog and then return the instance  */

   for (i = 0; i < num_classes; i++)
   {
      dialog = class_set[i].dialog_list;

      while (dialog != NULL)
      {
         if (dialog->dialog == instance && dialog->in_use == True)
            return (dialog->dialog_data);
         else
            dialog = dialog->next;
      }
   }

   return (NULL);
}




/************************************************************************
 *
 *  _DtIsDialogShowing
 *	This function is used return a boolean indicating whether the
 *	a dialog is displayed which contains the dialog data.
 *	of a currently in use dialog.
 *
 ************************************************************************/

Boolean
#ifdef _NO_PROTO
_DtIsDialogShowing( dialog_data )
        DialogData *dialog_data ;
#else
_DtIsDialogShowing(
        DialogData *dialog_data )
#endif /* _NO_PROTO */

{
   Dialog * dialog;


   /*  Find the dialog and then return the instance  */

   dialog = class_set[dialog_data->type].dialog_list;

   while (dialog != NULL)
   {
      if (dialog->in_use && dialog->dialog_data == dialog_data)
         return (True);
      else
         dialog = dialog->next;
   }

   return (False);
}




/************************************************************************
 *
 *  _DtWriteDialogData
 *	This function is used to write, as resources, the data
 *	for a dialog contained in dialog_data to the open file fd.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtWriteDialogData( dialog_data, fd, name_list )
        DialogData *dialog_data ;
        int fd ;
        char **name_list ;
#else
_DtWriteDialogData(
        DialogData *dialog_data,
        int fd,
        char **name_list )
#endif /* _NO_PROTO */

{
   if (dialog_data != NULL)
      (*(class_set[dialog_data->type].class->write_resource_values))
         (dialog_data, fd, name_list);
}




/************************************************************************
 *
 *  _DtFreeDialogData
 *	This function is used to free up the data space allocated 
 *	by a dialog.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtFreeDialogData( dialog_data )
        DialogData *dialog_data ;
#else
_DtFreeDialogData(
        DialogData *dialog_data )
#endif /* _NO_PROTO */

{
   if (dialog_data != NULL)
   {
      (*(class_set[dialog_data->type].class->free_values)) (dialog_data->data);
      XtFree ((char *) dialog_data);
       dialog_data = NULL;
   }
}




/************************************************************************
 *
 *  _DtDialogGetResources
 *	This function accesses data_base to extract the resource set
 *	described by the resoruces array.  Resources values are 
 *	converted to the proper type and defaults are used if not
 *	data is found in the resource data base.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtDialogGetResources( database, name_list, dialog_name, base, resources, 
                    resource_count )
        XrmDatabase database ;
        char **name_list ;
        char *dialog_name ;
        char *base ;
        DialogResource *resources ;
        int resource_count ;
#else
_DtDialogGetResources(
        XrmDatabase database,
        char **name_list,
        char *dialog_name,
        char *base,
        DialogResource *resources,
        int resource_count )
#endif /* _NO_PROTO */

{
   XrmName xrmName[MAX_NAME_LIST_SIZE];
   XrmQuark stringQuark;
   int nameCount;
   int i;

   /*  Build the quarkified name list from name_list and dialog_name  */
   /*  provided by the calling procedure.                             */

   nameCount = 0;
   if (name_list != NULL)
   {
      while (name_list[nameCount] != NULL)
      {
         xrmName[nameCount] =  XrmStringToQuark (name_list[nameCount]);
         nameCount++;
      }
   }
    
   if (dialog_name)
      xrmName[nameCount] = XrmStringToQuark (dialog_name);
   else
      nameCount--;

   xrmName[nameCount + 2] = 0;
   stringQuark = XrmStringToQuark (XmRString);


   /* Load the common dialog size/position resources */
   for (i = 0; i < resource_count; i++)
   {
      IntDialogGetResources(database, base, resources + i, xrmName, nameCount,
                            stringQuark);
   }

   /* 
    * Load the dialog specific resources. If no value found, use the default.
    */
   for (i = 0; i < commonResourceCount; i++)
   {
      IntDialogGetResources(database, base, commonResources + i, xrmName, 
                            nameCount, stringQuark);
   }
}


/*
 * This internal function does the real work of loading a single resource
 * value.  If the value is not found in the resource database, then the
 * specified default value is used.
 */

static void
#ifdef _NO_PROTO
IntDialogGetResources( database, base, resource, xrmName, nameCount,
                       stringQuark ) 
        XrmDatabase database ;
        char *base ;
        DialogResource *resource ;
        XrmName *xrmName;
        int nameCount ;
        XrmQuark stringQuark;
#else
IntDialogGetResources(
        XrmDatabase database,
        char *base,
        DialogResource *resource,
        XrmName *xrmName,
        int nameCount,
        XrmQuark stringQuark )
#endif /* _NO_PROTO */

{
   XrmRepresentation repType;
   XrmValue value;
   XrmValue convertedValue;
   char  charVal;
   short shortVal;
   long  longVal;

   {
      xrmName[nameCount + 1] = XrmStringToQuark (resource->name);

      if (XrmQGetResource (database, xrmName, xrmName, &repType, &value))
      {
         if (repType == stringQuark) 
            if (strcmp (resource->type, XmRString) != 0)
            {
#ifdef _TT_MULTI_SCREEN
               XtConvert (sParentShell[0], XmRString, &value,
                                      resource->type, &convertedValue);
#else
               XtConvert (encap_parent_shell, XmRString, &value, 
                                      resource->type, &convertedValue);
#endif
            }
            else
            {
               *((char **)(base + resource->offset)) = (char *)value.addr;
               return;
            }
         else
            convertedValue.addr = NULL;

      }
      else
         convertedValue.addr = NULL;


      /*  Set the converted value address pointer and value to  */
      /*  the proper default value if the addr is NULL.         */

      if (convertedValue.addr == NULL)
      {
         if (resource->size == sizeof(char))
         {
            charVal = (char)resource->default_value;
            convertedValue.addr = (caddr_t) &charVal;
         }
         else if (resource->size == sizeof(short))
         {
            shortVal = (short)resource->default_value;
            convertedValue.addr = (caddr_t) &shortVal;
         }
         else
         {
            longVal = (long)resource->default_value;
            convertedValue.addr = (caddr_t) &longVal;
         }
      }
        

      /*  Stuff the converted value into the calling functions   */
      /*  structure according to the size of the piece of data.  */

      if (resource->size == sizeof(char))
         *((char *)(base + resource->offset)) = *((char *)convertedValue.addr);
      else if (resource->size == sizeof(short))
         *((short *)(base + resource->offset))= *((short *)convertedValue.addr);
      else
         *((long *)(base + resource->offset)) = *((long *)convertedValue.addr);
   }
}



/************************************************************************
 *
 *  _DtDialogPutResources
 *	This function writes a resource set to a file.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtDialogPutResources( fd, nameList, dialogName, base, resources, resourceCount)
        int fd ;
        char **nameList ;
        char *dialogName ;
        char *base ;
        DialogResource *resources ;
        int resourceCount ;
#else
_DtDialogPutResources(
        int fd,
        char **nameList,
        char *dialogName,
        char *base,
        DialogResource *resources,
        int resourceCount )
#endif /* _NO_PROTO */

{
   int i;
   DialogInstanceData * dialogInstanceData;

   /*
    * Write out the common dialog size/position resources, only if the
    * dialog is currently displayed.
    */
   dialogInstanceData = (DialogInstanceData *)base;
   if (dialogInstanceData->displayed)
   {
      for (i = 0; i < commonResourceCount; i++)
      {
         IntDialogPutResources(fd, nameList, dialogName, base, 
                               commonResources + i);
      }
   }

   /*
    *  Loop through the dialog specific resources, write the name list and
    *  resource name.
    */
   for (i = 0; i < resourceCount; i++)
      IntDialogPutResources( fd, nameList, dialogName, base, resources + i);
}


/*
 * This internal function does the real work involved in writing a single
 * resource out to a session file.
 */

static void
#ifdef _NO_PROTO
IntDialogPutResources( fd, nameList, dialogName, base, resource)
        int fd ;
        char **nameList ;
        char *dialogName ;
        char *base ;
        DialogResource *resource ;
#else
IntDialogPutResources(
        int fd,
        char **nameList,
        char *dialogName,
        char *base,
        DialogResource *resource )
#endif /* _NO_PROTO */

{
   static char outBuf[MAX_RESOURCE_LENGTH];
   int nameCount;

   outBuf[0] = '\0';

   (void) strcat (outBuf, "*");

   nameCount = 0;
   if (nameList != NULL)
   {
      while (nameList[nameCount] != NULL)
      {
         (void) strcat (outBuf, nameList[nameCount]);
         (void) strcat (outBuf, ".");
         nameCount++;
      }
   }

   if (dialogName != NULL)
   {
      (void) strcat (outBuf, dialogName);
      (void) strcat (outBuf, ".");
   }

   (void) strcat (outBuf, resource->name);
   (void) strcat (outBuf, ":	");

   /* Bug fix 4024025 */
   if (strcmp(resource->name,"height") == NULL ) { 
	extern int HeightAdjustment ; /* defined in FileMgr.c */
	Dimension * p = (Dimension *)(base + resource->offset);
	*p = *p - HeightAdjustment ;
	(*(resource->write_resource))(fd, (XtPointer) p, outBuf);
   }
   else
        (*(resource->write_resource))
                (fd, (XtPointer) (base + resource->offset), outBuf);
}



/************************************************************************
 ************************************************************************
 *
 *	Internal functions
 *
 ************************************************************************
 ************************************************************************/


/************************************************************************
 *
 *  _DtEncapSetWorkSpaceHints
 *	This function sets a given shell to a given set(s) of
 *   workspaces.
 *
 ************************************************************************/
void
#ifdef _NO_PROTO
_DtEncapSetWorkSpaceHints( shell, workspaces )
        Widget shell ;
        char *workspaces ;
#else
_DtEncapSetWorkSpaceHints(
        Widget shell,
        char *workspaces )
#endif /* _NO_PROTO */

{
   char * ptr;
   Atom * workspace_atoms = NULL;
   int num_workspaces=0;
  
   if (workspaces) 
   {
      do
      {
         ptr = DtStrchr (workspaces, '*');

         if (ptr != NULL) *ptr = '\0';

         workspace_atoms = (Atom *) XtRealloc ((char *)workspace_atoms, 
                           sizeof (Atom) * (num_workspaces + 1));

         workspace_atoms[num_workspaces] = 
                           XmInternAtom (XtDisplay(shell), workspaces, True);

         num_workspaces++;

         if (ptr != NULL)
         {
            *ptr = '*';
            workspaces = ptr + 1;
         }
      } while (ptr != NULL);

      DtWsmSetWorkspacesOccupied (XtDisplay(shell), XtWindow (shell), workspace_atoms,
		           num_workspaces);

      XtFree ((char *) workspace_atoms);
      workspace_atoms = NULL;
   }
   else
   {
      Window   rootWindow;
      Atom     pCurrent;
      Screen   *currentScreen;
      int      screen;
      char     *workspace_name;

      /*
       * Since no specific workspaces were specified, we will force the
       * dialog to the current workspace.
       */
#ifdef _TT_MULTI_SCREEN
      currentScreen = XtScreen(shell);
#else
      screen = XDefaultScreen(XtDisplay(shell));
      currentScreen = XScreenOfDisplay(XtDisplay(shell), screen);
#endif
      rootWindow = RootWindowOfScreen(currentScreen);

      if(DtWsmGetCurrentWorkspace(XtDisplay(shell), rootWindow, &pCurrent) == 
                               Success)
      {
         DtWsmSetWorkspacesOccupied(XtDisplay(shell), XtWindow (shell), &pCurrent, 1);
      }
   }
}

/************************************************************************
 *
 *  SetIconifyState
 *	This function sets a given shell to a given iconify state.
 *  (e.g. Mapped or iconified)
 *
 ************************************************************************/
static void
#ifdef _NO_PROTO
SetIconifyState( shell, iconify, UseIconX, IconX, UseIconY, IconY)
        Widget shell ;
        Boolean iconify ;
        Boolean UseIconX;
        int IconX;
        Boolean UseIconY;
        int IconY;
        
#else
SetIconifyState(
        Widget shell,
        Boolean iconify,
        Boolean UseIconX,
        int IconX,
        Boolean UseIconY,
        int IconY)
#endif /* _NO_PROTO */

{
  int ac;
   Arg         args[10];
   XWMHints   *wmhints;
   
   if (iconify)
   {
     /* add the iconify hint to the current shell */
     ac = 0;
     XtSetArg(args[ac], XmNinitialState, IconicState); ac++;

     if (UseIconX)
       { XtSetArg(args[ac], XmNiconX, IconX); ac++; }

     if (UseIconY)
       { XtSetArg(args[ac], XmNiconY, IconY); ac++; }

     XtSetValues(shell, args, ac);
   }
   else
   {
     /* Remove the iconify hint from the current shell */
     wmhints = XGetWMHints(XtDisplay(shell), XtWindow(shell));
     wmhints->flags |= IconWindowHint;
     wmhints->initial_state = NormalState;
     XSetWMHints(XtDisplay(shell), XtWindow(shell), wmhints);
     XFree(wmhints);
   }  

}
/************************************************************************
 *
 *  DataChangeCallback
 *	This callback is invoked from a dialog upon an action on the
 *	dialog that means that the data within the dialog has been
 *	changed.
 *
 ************************************************************************/

/*ARGSUSED*/
static void
#ifdef _NO_PROTO
DataChangeCallback( widget, client_data, call_data )
        Widget widget ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
DataChangeCallback(
        Widget widget,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */

{
   Dialog * dialog = (Dialog *) client_data;
   DialogData * new_data;

   new_data = (DialogData *) XtMalloc (sizeof (DialogData));

   new_data->type = dialog->dialog_data->type;
   new_data->data = NULL;
   if (class_set[new_data->type].class->get_values)
   {
      new_data->data = (*(class_set[new_data->type].class->get_values))
                          (dialog->dialog);
   }

   if (dialog->change)
      (*(dialog->change))
         (dialog->change_client_data, dialog->dialog_data, new_data, call_data);
}




/************************************************************************
 *
 *  DataCloseCallback
 *	This callback is invoked from a dialog upon an action on the
 *	dialog that means the dialog has been closed.
 *
 ************************************************************************/

/*ARGSUSED*/
static void
#ifdef _NO_PROTO
DataCloseCallback( widget, client_data, call_data )
        Widget widget ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
DataCloseCallback(
        Widget widget,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */


{
   Dialog * dialog = (Dialog *) client_data;

   if(RecheckFlag(NULL,widget)) /* cancel flag already set, just return */
       return;
   _DtHideDialog (dialog->dialog_data, True);
}


#ifdef _TT_TURNED_OFF /* No TimerEvent for dialog caching */


/************************************************************************
 *
 *  TimerEvent
 *	This action function is called upon the encapsulations
 *	timeout going off.  Its function is to precreate and destroy
 *	extra dialogs.
 *
 ************************************************************************/

/* ARGSUSED */
static void
#ifdef _NO_PROTO
TimerEvent( widget, id )
        Widget widget ;
        XtIntervalId *id ;
#else
TimerEvent(
        Widget widget,
        XtIntervalId *id )
#endif /* _NO_PROTO */

{
   register int i;
   Dialog * dialog;
   Dialog * prev_dialog;
   int count;

#ifdef _TT_MULTI_SCREEN
   int display_screen;

   display_screen = XScreenNumberOfScreen(XtScreen(widget));
#endif

   /*  First pass through the dialog set to see if any  */
   /*  need to be created.  This is based on having 0   */
   /*  not in use dialogs of each type.                 */

   for (i = 0; i < num_classes; i++)
   {
      dialog = class_set[i].dialog_list;

      /* Only attempt to cache dialogs requesting this feature */
      if (!class_set[i].cache)
         continue;

      while (dialog != NULL)
#ifdef _TT_MULTI_SCREEN
         if ((dialog->in_use == False) && 
	     (dialog->screenNum == display_screen))
#else
         if (dialog->in_use == False)
#endif
            break;
         else
            dialog = dialog->next;

      if (dialog == NULL)
      {
         dialog = (Dialog *) XtMalloc (sizeof (Dialog));
#ifdef _TT_MULTI_SCREEN
	 dialog->screenNum = display_screen;
#endif

         (*(class_set[i].class->create))
#ifdef _TT_MULTI_SCREEN
            (XtDisplay (widget), widget,
#else
            (XtDisplay (encap_parent_shell), encap_parent_shell, 
#endif
            &(dialog->dialog_widget), &dialog->dialog);


         /*  Add the change and close callbacks into the dialog  */

         if (class_set[i].class->install_change_callback) 
            (*(class_set[i].class->install_change_callback))
               (dialog->dialog, DataChangeCallback, dialog);

         if (class_set[i].class->install_close_callback) 
            (*(class_set[i].class->install_close_callback))
               (dialog->dialog, DataCloseCallback, dialog);


         dialog->next = class_set[i].dialog_list;
         class_set[i].dialog_list = dialog;

         dialog->in_use = False;

#ifdef _TT_MULTI_SCREEN
         sTimerState[display_screen] = TIMER_ACTIVE_STATE;
         sTimerId[display_screen] = 
		XtAppAddTimeOut(XtWidgetToApplicationContext(widget),
#else
         timerState = TIMER_ACTIVE_STATE;
         timerId = XtAppAddTimeOut(XtWidgetToApplicationContext(widget), 
#endif
                                   activeTimeoutLength,
                                   (XtTimerCallbackProc) TimerEvent, 
                                   (XtPointer) widget);

         return;
      }
   }


   /*  Pass through the dialog set to see if any need to be destroyed    */
   /*  This is based on having more than 1 not in use dialog of a type.  */

   for (i = 0; i < num_classes; i++)
   {
      dialog = class_set[i].dialog_list;
      count = 0;

      while (dialog != NULL)
      {
#ifdef _TT_MULTI_SCREEN
         if ((dialog->in_use == False) &&
             (dialog->screenNum == XScreenNumberOfScreen(XtScreen(widget))))
#endif
         if (dialog->in_use == False)
            count++;

         dialog = dialog->next;
      }

#ifdef _TT_ALL
      /* We want to delete any items in the cache */
      if (count > 0)
#else
      if (count > 1)
#endif
      {
         dialog = class_set[i].dialog_list;

         if (dialog->in_use == False)
            class_set[i].dialog_list = dialog->next;
         else
         {
            prev_dialog = class_set[i].dialog_list;
            dialog = dialog->next;

            while (dialog->in_use == True)
            {
              prev_dialog = dialog;
              dialog = dialog->next;
            }

            prev_dialog->next = dialog->next;
         }

         (*(class_set[i].class->destroy)) (dialog->dialog);
         XtFree ((char *) dialog);
          dialog = NULL;


#ifdef _TT_MULTI_SCREEN
          /* Keep the timer active to delete more dialogs */
          sTimerState[display_screen] = TIMER_ACTIVE_STATE;
          sTimerId[display_screen] = 
            XtAppAddTimeOut(XtWidgetToApplicationContext(widget),
                            activeTimeoutLength,
                            (XtTimerCallbackProc) TimerEvent, 
                            (XtPointer) widget);
          return;
#else
          break;
#endif
      }
   }

#ifdef _TT_MULTI_SCREEN
   sTimerState[display_screen] = TIMER_IDLE_STATE;
   sTimerId[display_screen] = 
     XtAppAddTimeOut(XtWidgetToApplicationContext(widget),
#else
   timerState = TIMER_IDLE_STATE;
   timerId = XtAppAddTimeOut(XtWidgetToApplicationContext(widget), 
#endif
                             idleTimeoutLength,
                             (XtTimerCallbackProc)TimerEvent, 
                             (XtPointer)widget);
}

#endif /* _TT_TURNED_OFF */



/************************************************************************
 ************************************************************************
 *
 *  Externed Type to String Converters and writers
 *
 ************************************************************************
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtIntToString( fd, value, out_buf )
        int fd ;
        int *value ;
        char *out_buf ;
#else
_DtIntToString(
        int fd,
        int *value,
        char *out_buf )
#endif /* _NO_PROTO */

{
   (void) sprintf (resourceBuf, "%d", *value);
   _DtStringToString( fd, &resourceBuf, out_buf );
}

void
#ifdef _NO_PROTO
_DtPositionToString( fd, value, out_buf )
        int fd ;
        Position *value ;
        char *out_buf ;
#else
_DtPositionToString(
        int fd,
        Position *value,
        char *out_buf )
#endif /* _NO_PROTO */

{
   (void) sprintf (resourceBuf, "%d", *value);
   _DtStringToString( fd, &resourceBuf, out_buf );
}



void
#ifdef _NO_PROTO
_DtDimensionToString( fd, value, out_buf )
        int fd ;
        Dimension *value ;
        char *out_buf ;
#else
_DtDimensionToString(
        int fd,
        Dimension *value,
        char *out_buf )
#endif /* _NO_PROTO */

{
   (void) sprintf (resourceBuf, "%d", *value);
   _DtStringToString( fd, &resourceBuf, out_buf );
}



void
#ifdef _NO_PROTO
_DtBooleanToString( fd, value, out_buf )
        int fd ;
        Boolean *value ;
        char *out_buf ;
#else
_DtBooleanToString(
        int fd,
        Boolean *value,
        char *out_buf )
#endif /* _NO_PROTO */

{
   char * buf;

   if (*value == True) 
      buf = "True";
   else 
      buf = "False";
   _DtStringToString( fd, (char **)&buf, out_buf );
}


void
#ifdef _NO_PROTO
_DtXmStringToString( fd, value, out_buf )
        int fd ;
        XmString *value ;
        char *out_buf ;
#else
_DtXmStringToString(
        int fd,
        XmString *value,
        char *out_buf )
#endif /* _NO_PROTO */

{
   char * out_value;

   if (*value != NULL)
   {
      if (XmStringGetLtoR (*value, XmFONTLIST_DEFAULT_TAG, &out_value))
      {
         if (strlen (out_value) != 0)
         {
            (void) write (fd, out_value, strlen (out_value));
            _DtStringToString( fd, &out_value, out_buf );
         }

         XtFree ((char *) out_value);
      }
   }
}


void
#ifdef _NO_PROTO
_DtXmStringTableToString( fd, value, out_buf )
        int fd ;
        XmStringTable *value ;
        char *out_buf ;
#else
_DtXmStringTableToString(
        int fd,
        XmStringTable *value,
        char *out_buf )
#endif /* _NO_PROTO */

{
   int i;
   char * out_value;
   Boolean first = True;

   if ((value != NULL) && (*value != NULL))
   {
      i = 0;
      while ((*value)[i] != NULL)
      {
         if (XmStringGetLtoR ((*value)[i], XmFONTLIST_DEFAULT_TAG, &out_value))
         {
            if (first)
            {
               (void) write (fd, out_buf, strlen (out_buf));
               first = False;
            }
            else
               (void) write (fd, ", ", strlen (", "));

            (void) write (fd, out_value, strlen (out_value));

            XtFree ((char *) out_value);
         }

         i++;
      }

      if (first == False)
         (void) write (fd, "\n", strlen ("\n"));
   }
}


void
#ifdef _NO_PROTO
_DtStringToString( fd, value, out_buf )
        int fd ;
        char **value ;
        char *out_buf ;
#else
_DtStringToString(
        int fd,
        char **value,
        char *out_buf )
#endif /* _NO_PROTO */

{
   if (*value == NULL || strlen (*value) == 0)
      ;
   else
   {
      (void) write (fd, out_buf, strlen (out_buf));
      (void) write (fd, *value, strlen (*value));
      (void) write (fd, "\n", strlen ("\n"));
   }
}


/*
 * _DtChildPosition:
 *   Choose a position for a popup window ("child") so that the main
 *   window ("parent") is not obscured.  The child will be positioned
 *   to the right, below, left, or above the parent, depending on where
 *   there is the most space.
 */

void
#ifdef _NO_PROTO
_DtChildPosition(w, parent, newX, newY)
         Widget w;
         Widget parent;
         Position *newX, *newY;
#else
_DtChildPosition(
         Widget w,
         Widget parent,
         Position *newX,
         Position *newY)
#endif /* _NO_PROTO */
{
   Position pY, pX;
   XmVendorShellExtObject vendorExt;
   XmWidgetExtData        extData;
   int xOffset, yOffset;
   int pHeight, myHeight, sHeight;
   int pWidth, myWidth, sWidth;
   enum { posRight, posBelow, posLeft, posAbove } pos;
   int space;

   /* get x, y offsets for the parent's window frame */
   extData = _XmGetWidgetExtData(parent, XmSHELL_EXTENSION);
   if (extData)
   {
     vendorExt = (XmVendorShellExtObject)extData->widget;
     xOffset = vendorExt->vendor.xOffset;
     yOffset = vendorExt->vendor.yOffset;
   }
   else
     xOffset = yOffset = 0;

   /* get size/position of screen, parent, and widget */
   sHeight = HeightOfScreen(XtScreen(parent));;
   sWidth = WidthOfScreen(XtScreen(parent));
   pX = XtX(parent) - xOffset;
   pY = XtY(parent) - yOffset;
   pHeight = XtHeight(parent) + yOffset + xOffset;
   pWidth = XtWidth(parent) + 2*xOffset;
   myHeight = XtHeight(w) + yOffset + xOffset;
   myWidth = XtWidth(w) + 2*xOffset;

   /*
    * Determine how much space would be left if the child was positioned
    * to the right, below, left, or above the parent.  Choose the child
    * positioning so that the maximum space is left.
    */
   pos = posRight;
   space = sWidth - (pX + pWidth + myWidth);

   if (sHeight - (pY + pHeight + myHeight) > space)
   {
      pos = posBelow;
      space = sHeight - (pY + pHeight + myHeight);
   }

   if (pX - myWidth > space)
   {
      pos = posLeft;
      space = pX - myWidth;
   }

   if (pY - myHeight > space)
   {
      pos = posAbove;
      space = pY - myHeight;
   }

   /* Given relative positioning, determine x, y coordinates for the child */

   switch (pos)
   {
     case posRight:
       *newX = pX + pWidth + 5;
       *newY = pY + (pHeight - myHeight)/2;
       break;

     case posBelow:
       *newX = pX + (pWidth - myWidth)/2;
       *newY = pY + pHeight + 5;
       break;

     case posLeft:
       *newX = pX - myWidth - 5;
       *newY = pY + (pHeight - myHeight)/2;
       break;

     case posAbove:
       *newX = pX + (pWidth - myWidth)/2;
       *newY = pY - myHeight - 5;
       break;
   }
}


/* ARGSUSED */
void
#ifdef _NO_PROTO
_DtmapCB(w,client_data,call_data)
         Widget w;
         XtPointer client_data;
         XtPointer call_data;
#else
_DtmapCB(
         Widget w,
         XtPointer client_data,
         XtPointer call_data )
#endif /* _NO_PROTO */
{
   Arg         args[2];
   Widget      parent;
   Position newX, newY;

   parent = (Widget)client_data;

   if (parent)
   {
      _DtChildPosition(w, parent, &newX, &newY);
      XtSetArg(args[0], XmNx, newX);
      XtSetArg(args[1], XmNy, newY);
      XtSetValues(w, args, 2);
   }
}


/************************************************************************
 *
 *  _DtBuildDialog
 *	This functions is used to build an instance of a dialog with
 *	the provided data but not display it.  The functions and data 
 *      to be set back to the application upon change or close of the 
 *      dialog is also provided as parameters.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtBuildDialog(parent, map_parent, top_rec, dialog_data, change_proc, change_data, 
            close_proc, close_data, workspaces, iconify_state, ignoreCache,
            title, classHints )
        Widget parent ;
        Widget map_parent ;
        XtPointer top_rec ;
        DialogData *dialog_data ;
        DialogChangedProc change_proc ;
        XtPointer change_data ;
        DialogClosedProc close_proc ;
        XtPointer close_data ;
        char *workspaces ;
        Boolean iconify_state ;
        Boolean ignoreCache ;
        char * title;
        XClassHint * classHints;
#else
_DtBuildDialog(
        Widget parent,
        Widget map_parent,
        XtPointer top_rec,
        DialogData *dialog_data,
        DialogChangedProc change_proc,
        XtPointer change_data,
        DialogClosedProc close_proc,
        XtPointer close_data,
        char *workspaces,
        Boolean iconify_state,
        Boolean ignoreCache,
        char * title,
        XClassHint * classHints )
#endif /* _NO_PROTO */

{
   Dialog * dialog;
   int dialog_type, n;
   DialogInstanceData * instance_data;
   char geometry[40];
   Arg args[5];
   Boolean doCenter = False;
   Boolean doParentRelativePositioning = False;
   int availableDialogCount;

#ifdef _TT_MULTI_SCREEN
   extern Widget toplevel;
   int	display_screen;

   display_screen = (parent) ? 
	XScreenNumberOfScreen(XtScreen(parent)) : gTTScreenNum;
#endif

   /*  See if there is a cached, unused dialog of the correct type.  */

   dialog_type = dialog_data->type;
   dialog = NULL;

   if (!ignoreCache)
   {
      Dialog * availableDialog;

      availableDialog = class_set[dialog_type].dialog_list;
      availableDialogCount = 0;

      /*
       * In addition to looking for an available dialog in the cache to use,
       * we also want to count up the number of unused dialogs in the cache.
       * This lets us know it we need to restart the timer, to again build
       * up the cache.
       */
      while (availableDialog != NULL)
      {
#ifdef _TT_MULTI_SCREEN
         if ((availableDialog->in_use == False) &&
             (availableDialog->screenNum == display_screen))
         {
            if (dialog == NULL)
               dialog = availableDialog;
            else
               availableDialogCount++;
         }
#else
         if (availableDialog->in_use == False)
         {
            if (dialog == NULL)
               dialog = availableDialog;
            else
               availableDialogCount++;
         }
#endif
         availableDialog = availableDialog->next;
      }
   }

   if (dialog == NULL)
   {
      dialog = (Dialog *) XtMalloc (sizeof (Dialog));

#ifdef _TT_MULTI_SCREEN
      (*(class_set[dialog_type].class->create))
         (XtDisplay (sParentShell[display_screen]),
         sParentShell[display_screen],
         &(dialog->dialog_widget), &dialog->dialog);
#else
      (*(class_set[dialog_type].class->create))
         (XtDisplay (encap_parent_shell), encap_parent_shell,
         &(dialog->dialog_widget), &dialog->dialog);
#endif

      /*  Add the change and close callbacks into the dialog  */

      if (class_set[dialog_type].class->install_change_callback) 
         (*(class_set[dialog_type].class->install_change_callback))
            (dialog->dialog, DataChangeCallback, dialog);

      if (class_set[dialog_type].class->install_close_callback) 
         (*(class_set[dialog_type].class->install_close_callback))
            (dialog->dialog, DataCloseCallback, dialog);

      dialog->next = class_set[dialog_type].dialog_list;
      class_set[dialog_type].dialog_list = dialog;
   }

   /* 
    * Set pointer to top dialog data in child of the shell.
    * This is needed to get help to work.
    */
   if (top_rec == NULL)
      top_rec = dialog->dialog;
   XtSetArg(args[0], XmNuserData, top_rec);
   XtSetValues(dialog->dialog_widget, args, 1);

   /*  Set the dialog structure fields to the parameter data.  */

   dialog->in_use = True;
   dialog->dialog_data = dialog_data;
   dialog->change = change_proc;
   dialog->change_client_data = change_data;
   dialog->close = close_proc;
   dialog->close_client_data = close_data;

   /* If a special title has been specified, we need to set it now */
   if (title)
   {
      XtSetArg(args[0], XmNtitle, title);
      XtSetValues(XtParent(dialog->dialog_widget), args, 1);
   }
}


/************************************************************************
 *
 *  _DtShowBuiltDialog
 *	This functions is used to display an instance of a dialog which
 *      has already been built with _DtBuildDialog.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
_DtShowBuiltDialog(parent, map_parent,
                   dialog_data, workspaces,
                   iconify_state, classHints,
                   UseIconX, IconX, UseIconY, IconY)
        Widget parent ;
        Widget map_parent ;
        DialogData *dialog_data ;
        char *workspaces ;
        Boolean iconify_state ;
        XClassHint * classHints;
        Boolean UseIconX;
        int IconX;
        Boolean UseIconY;
        int IconY;
#else
_DtShowBuiltDialog(
        Widget parent,
        Widget map_parent,
        DialogData *dialog_data,
        char *workspaces,
        Boolean iconify_state,
        XClassHint * classHints,
        Boolean UseIconX,
        int IconX,
        Boolean UseIconY,
        int IconY)
#endif /* _NO_PROTO */

{
   Dialog * dialog;
   int dialog_type, n;
   DialogInstanceData * instance_data;
   char geometry[40];
   Arg args[5];
   Boolean doCenter = False;
   Boolean doParentRelativePositioning = False;
   int availableDialogCount;

#ifdef _TT_MULTI_SCREEN
   extern Widget toplevel;
   int	display_screen = gTTScreenNum;
#endif

   dialog_type = dialog_data->type;

   /*  Find the dialog */
   dialog = class_set[dialog_data->type].dialog_list;
   while (dialog != NULL)
   {
      if (dialog->dialog_data == dialog_data)
         break;
      else
         dialog = dialog->next;
   }

   /* Need to add the map callback in relation to the parent */
   if (class_set[dialog_type].class->map)
   {
      /*
       * The map_parent parameter gives us the ability to position
       * the dialog relative to a window which is not the transientFor
       * parent.  This is used for the audio preview dialog.
       */
      if (map_parent == NULL)
         map_parent = parent;

      (*(class_set[dialog_type].class->map)) (map_parent, dialog->dialog);
   }

   instance_data = (DialogInstanceData *) dialog_data->data;

   /*  If this is a top level shell, get it realized  */
   if (XtIsSubclass (XtParent (dialog->dialog_widget), 
                     applicationShellWidgetClass))
   {
      if (XtIsRealized (XtParent (dialog->dialog_widget)) == False)
      {
         if (instance_data->displayed == True)
         {
            (void) sprintf (geometry, "=%dx%d+%d+%d", 
                            instance_data->width, instance_data->height,
                            instance_data->x, instance_data->y);

            XtSetArg (args[0], XmNgeometry, geometry);
            XtSetValues (XtParent (dialog->dialog_widget), args, 1);
         }
         else if ((instance_data->width != 0) && (instance_data->height != 0))
         {
            n=0;
            XtSetArg (args[n], XmNwidth, instance_data->width); n++;
            XtSetArg (args[n], XmNheight, instance_data->height); n++;
            XtSetValues (XtParent (dialog->dialog_widget), args, n);
         }

         /* Toggle mappedWhenManaged to false */
         XtSetMappedWhenManaged(XtParent (dialog->dialog_widget), False);
         XtRealizeWidget (XtParent(dialog->dialog_widget));

         /* Set the proper workspaces if needed */
         _DtEncapSetWorkSpaceHints(XtParent(dialog->dialog_widget), workspaces);

         /* Set any application-specified class hints for the window */
         if (classHints)
         {
            XSetClassHint(XtDisplay(dialog->dialog_widget),
                          XtWindow(XtParent(dialog->dialog_widget)),
                          classHints);
         }
           
         /* Set the iconify state */
         SetIconifyState(XtParent(dialog->dialog_widget),
                         iconify_state, UseIconX, IconX, UseIconY, IconY);

         /* Map the window */
         XtSetMappedWhenManaged(XtParent (dialog->dialog_widget), True);
         XtPopup (XtParent (dialog->dialog_widget), XtGrabNone);
         XSync(XtDisplay(dialog->dialog_widget), False);
         
      }
      else
      {
         if (instance_data->displayed == True)
         {
            WMShellWidget wm = (WMShellWidget)XtParent(dialog->dialog_widget);

            wm->wm.size_hints.flags |= USPosition;
            XtSetArg (args[0], XmNx, instance_data->x);
            XtSetArg (args[1], XmNy, instance_data->y);
            XtSetArg (args[2], XmNwidth, instance_data->width);
            XtSetArg (args[3], XmNheight, instance_data->height);
            XtSetValues (XtParent (dialog->dialog_widget), args, 4);
         }
         else if ((instance_data->width != 0) && (instance_data->height != 0))
         {
	    n=0;
            XtSetArg (args[n], XmNwidth, instance_data->width); n++;
            XtSetArg (args[n], XmNheight, instance_data->height); n++;
            XtSetValues (XtParent (dialog->dialog_widget), args, n);
         }
       
         /* Set the proper workspaces if needed */
         _DtEncapSetWorkSpaceHints(XtParent(dialog->dialog_widget), workspaces);

         /* Set any application-specified class hints for the window */
         if (classHints)
         {
            XSetClassHint(XtDisplay(dialog->dialog_widget),
                          XtWindow(XtParent(dialog->dialog_widget)),
                          classHints);
         }
           
         /* Set the iconify state */
         SetIconifyState(XtParent(dialog->dialog_widget),
                         iconify_state, UseIconX, IconX, UseIconY, IconY);

         /* Map the window */
         XtPopup (XtParent (dialog->dialog_widget), XtGrabNone);
      }
   }
   else
   {
      if (instance_data->displayed == True)
      {
         XtSetArg (args[0], XmNx, instance_data->x);
         XtSetArg (args[1], XmNy, instance_data->y);
         XtSetArg (args[2], XmNwidth, instance_data->width);
         XtSetArg (args[3], XmNheight, instance_data->height);
         XtSetArg (args[4], XmNdefaultPosition, False);
         XtSetValues (dialog->dialog_widget, args, 5);
         XtRealizeWidget (dialog->dialog_widget);
      }
      else
      {
         XtSetArg (args[0], XmNdefaultPosition, False);
         XtSetValues (dialog->dialog_widget, args, 1);

         XtRealizeWidget (dialog->dialog_widget);

         if (parent)
         {
            /* Position relative to the parent dialog */
            /* 
             * Must be done after the set_values call, since the dialog
             * may get forced to a different size.
             */
            doParentRelativePositioning = True;
         }
         else
         {
            /* Center in the display */
            /* 
             * Must be done after the set_values call, since the dialog
             * may get forced to a different size.
             */
            doCenter = True;
         }
      }

      /*
       * Dialogs with no controlling parent window, need to have their
       * own workspace perperty set, if some workspaces have been requested.
       */
      if ((parent == NULL) && workspaces)
         _DtEncapSetWorkSpaceHints(XtParent(dialog->dialog_widget), workspaces);
   }


   /*  Set Values onto the dialog to set it to the correct data.  */

   (*(class_set[dialog_data->type].class->set_values))
      (dialog->dialog, dialog_data->data);

   /*
    * These two adjustments MUST be done AFTER the dialog's SetValues()
    * procedure is called.  This is due to the fact that the setvalues
    * may cause the dialog size to change, and since both of the following
    * positioning algorithms are dependent upon the dialog size, we want
    * to make sure that the correct size is used.
    */
   if (doCenter)
   {
      XtSetArg (args[0], XmNx,
          (Dimension)(WidthOfScreen(XtScreen(dialog->dialog_widget)) -
           dialog->dialog_widget->core.width) / (Dimension)2);
      XtSetArg (args[1], XmNy,
          (Dimension)(HeightOfScreen(XtScreen(dialog->dialog_widget)) -
           dialog->dialog_widget->core.height) / (Dimension)2);
      XtSetValues (dialog->dialog_widget, args, 2);
   }
   else if (doParentRelativePositioning)
   {
      XtSetArg (args[0], XmNx,
          parent->core.x + 
          (Dimension)(parent->core.width - dialog->dialog_widget->core.width) / (Dimension)2);
      XtSetArg (args[1], XmNy,
          parent->core.y + 
          (Dimension)(parent->core.height - dialog->dialog_widget->core.height) / (Dimension)2);
      XtSetValues (XtParent(dialog->dialog_widget), args, 2);
   }


   /*  Fix up the transient-for windowing information so that  */
   /*  the window manager will shuffle and iconify as a group  */

   if (parent != NULL) 
   {
      if (XtIsRealized(parent))
      {
         XSetTransientForHint (XtDisplay (parent), 
                            XtWindow (XtParent (dialog->dialog_widget)),
                            XtWindow (parent)); 
      }
   }
   else
   {
      if (!XtIsSubclass (XtParent (dialog->dialog_widget), 
                                     applicationShellWidgetClass))
      {
#ifdef _TT_MULTI_SCREEN
         XSetTransientForHint (XtDisplay (sParentShell[display_screen]),
                               XtWindow (XtParent (dialog->dialog_widget)),
                               XtWindow (sParentShell[display_screen]));

#else
         XSetTransientForHint (XtDisplay (encap_parent_shell), 
                               XtWindow (XtParent (dialog->dialog_widget)),
                               XtWindow (encap_parent_shell)); 
#endif
      }
   }


   /*  Display the dialogs, application shells are displayed above.  */

   if (!(XtIsSubclass (XtParent (dialog->dialog_widget), 
                       applicationShellWidgetClass)))
   {
      XtManageChild (dialog->dialog_widget);
   }
   

   /*  Set the dialog instance data to indicate that the dialog  */
   /*  is displayed.                                             */

   ((DialogInstanceData *) (dialog_data->data))->displayed = True;


   /* Give the dialog a chance to set its focus widget, if necessary */

   if (class_set[dialog_data->type].class->set_focus)
   {
      (*(class_set[dialog_data->type].class->set_focus))
         (dialog->dialog, dialog_data->data);
   }
}


#ifdef _TT_TURNED_OFF /* No TimerEvent for dialog caching */

static void
#ifdef _NO_PROTO
DialogStructureNotifyHandler( w, client_data, event )
       Widget w ;
       XtPointer client_data ;
       XEvent *event ;
#else
DialogStructureNotifyHandler(
       Widget w,
       XtPointer client_data,
       XEvent *event )
#endif /* _NO_PROTO */
{
#ifdef _TT_MULTI_SCREEN
  int scr = XScreenNumberOfScreen(XtScreen(w));

  if( event->type == MapNotify )
  {
    if( sNumberOfDialogMapped[scr] == 0 )
    {
      int timeOut;

      if( sTimerState[scr] == TIMER_STARTUP_STATE )
        timeOut = initialTimeoutLength;
      else if( sTimerState[scr] == TIMER_ACTIVE_STATE )
        timeOut = activeTimeoutLength;
      else if( sTimerState[scr] == TIMER_IDLE_STATE )
        timeOut = idleTimeoutLength;
      else
        timeOut = 0;

      if( timeOut )
      {
        if( sTimerId[scr] )
          XtRemoveTimeOut( sTimerId[scr] );

        sTimerId[scr] = XtAppAddTimeOut( XtWidgetToApplicationContext( w ), 
                                   timeOut,
                                   (XtTimerCallbackProc)TimerEvent, 
                                   (XtPointer)sParentShell[scr] );
      }
    }
    ++sNumberOfDialogMapped[scr];
  }
  else if( event->type == UnmapNotify )
  {
    if( sNumberOfDialogMapped[scr] )
      --sNumberOfDialogMapped[scr];

    if( sNumberOfDialogMapped[scr] == 0 )
    {
      if( sTimerId[scr] )
      {
        XtRemoveTimeOut(sTimerId[scr]);
        sTimerId[scr] = 0;
   
      }
    }
  }

#else /* _TT_MULTI_SCREEN */

  if( event->type == MapNotify )
  {
    if( NumberOfDialogMapped == 0 )
    {
      int timeOut;

      if( timerState == TIMER_STARTUP_STATE )
        timeOut = initialTimeoutLength;
      else if( timerState == TIMER_ACTIVE_STATE )
        timeOut = activeTimeoutLength;
      else if( timerState == TIMER_IDLE_STATE )
        timeOut = idleTimeoutLength;
      else
        timeOut = 0;

      if( timeOut )
      {
        if( timerId )
          XtRemoveTimeOut( timerId );

        timerId = XtAppAddTimeOut( XtWidgetToApplicationContext( w ), 
                                   timeOut,
                                   (XtTimerCallbackProc)TimerEvent, 
#ifdef _TT_MULTI_SCREEN
	/* SCW:  TriTeal's won't ever use this since we use the previous 
	 *  ifdef MULTI_SCREEN block, but others might want to make this 
	 *  fix.  The incorrect widget is being used to initiate the timer
	 *  event.  This did cause a problem for us in the multi-screen 
	 *  case, so I'm guessing this one could cause problems also.
	 */
				   (XtPointer) encap_parent_shell);
#else  /* _TT_MULTI_SCREEN */
                                   (XtPointer)w );
#endif /* _TT_MULTI_SCREEN */
      }
    }
    ++NumberOfDialogMapped;
  }
  else if( event->type == UnmapNotify )
  {
    if( NumberOfDialogMapped )
      --NumberOfDialogMapped;

    if( NumberOfDialogMapped == 0 )
    {
      if( timerId )
      {
        XtRemoveTimeOut( timerId );
        timerId = 0;
      }
    }
  }
#endif /* _TT_MULTI_SCREEN */ 
}

#endif /* _TT_TURNED_OFF */

void
#ifdef _NO_PROTO
_DtChangeTo(client_data, directory)
         XtPointer client_data;
         char  *directory;
#else
_DtChangeTo(
         XtPointer client_data,
         char  *directory)
#endif /* _NO_PROTO */
{
   Dialog * dialog = (Dialog *) client_data;

   ChangeDirectoryToParent(dialog->change_client_data, directory);
}

void
#ifdef _NO_PROTO
_DtFreeDialog(dialog_data)
         DialogData  *dialog_data;
#else
_DtFreeDialog(
         DialogData  *dialog_data)
#endif /* _NO_PROTO */
{
   Dialog *dialog,**headptr;
   headptr = &class_set[dialog_data->type].dialog_list;
   dialog = *headptr;

   while (dialog != NULL)
   {
      if (dialog->dialog_data == dialog_data)
      {
        *headptr = dialog->next;
        XtFree((char *)dialog);
        break;
      }
      headptr = &(dialog->next);
      dialog = *headptr;
   }
}

