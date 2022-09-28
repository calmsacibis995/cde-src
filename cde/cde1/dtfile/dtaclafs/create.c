/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           copydialog.c
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    Widget creation functions
 *
 *   FUNCTIONS: createAclSkeleton
 *		createActionArea
 *		createAddDialog
 *		createDeleteDialog
 *		createDialog
 *		createWMCB
 *		initAcl
 *		newAclLine
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/ScrolledW.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/TextF.h>
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <X11/Xmu/Editres.h>

#include "dtaclafs.h"

#define MARGIN 5   /* used for component spacing */


/* prototypes for local functions */
#ifdef _NO_PROTO
static void createActionArea();
static void createAclSkeleton();
static void createWMCB();
static void initAcl();
static void createAddDialog();
static void createDeleteDialog();
#else
static void createActionArea   (DialogContext * context);
static void createAclSkeleton  (SkeletonType    type,
                                DialogContext * context)
static void createWMCB         (Widget          shell,
                                DialogContext * context);
static void initAcl            (Acl * acl);
static void createAddDialog    (DialogContext *context);
static void createDeleteDialog (DialogContext *context);
#endif


/*----------------------
 *
 *  createDialog
 *
 *--------------------*/
 
void
#ifdef _NO_PROTO
createDialog(context)
   DialogContext * context;
#else
createDialog(DialogContext *context)
#endif

{ 
   String    s;            
   XmString  xs, negTitle, normTitle;
   Arg       args[20];
   int       n;
   Dimension height1, height2;
   String   warnMessage="AFS permissions can only be set for folders.\n\
All files in a folder inherit that folder's permissions.";

   /* shell widget is stored as a global variable */
   /*    to be available as the parent for popups */
   extern Widget dtAclAfsShell;

   /* local copies of Widgets */
   Widget  outerFrame;
   Widget  outerForm;
   Widget  fileName;
   Widget  fileNameSeparator;
   Widget  folderName;
   Widget  warningMessage;
   Widget  warningSeparator;
   Widget  modeRadio;
   Widget  modeRadioTitle;
   Widget  advancedToggle;
   Widget  basicToggle;
   Widget  aclForm;
   Widget  showNegRadio;
   Widget  showNegRadioTitle;
   Widget  showNegToggle;
   Widget  hideNegToggle;
   Widget  summary;


   
   

   /* add support for editres debugging tool */
   XtAddEventHandler(dtAclAfsShell, (EventMask) 0, True, _XEditResCheckMessages, NULL);
   
   /* have use of window manager's close invoke the dialog's cancel callback */
   createWMCB(dtAclAfsShell, context);

   /* create dialog for adding new users/groups to ACL */
   createAddDialog(context);

   /* create dialog for deleting users/groups from ACL */
   createDeleteDialog(context);

                                         
   /* create a frame around the dialog for better separation from window border */
   outerFrame = XtVaCreateManagedWidget("outerFrame",
                                        xmFrameWidgetClass, dtAclAfsShell,
                                        XmNshadowThickness, 3,
                                        XmNshadowType, XmSHADOW_ETCHED_IN,
                                        NULL);

   /* create dialog's top-level form */
   outerForm = XtVaCreateWidget("outerForm",
                                 xmFormWidgetClass, outerFrame,
                                 NULL);


   /* create warning message */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 14, warnMessage)), XmFONTLIST_DEFAULT_TAG);
   warningMessage = XtVaCreateManagedWidget("warningMessage",
                                            xmLabelGadgetClass, outerForm,
                                            XmNlabelString,    xs,
                                            XmNalignment,      XmALIGNMENT_BEGINNING,
                                            XmNtopAttachment,  XmATTACH_FORM,
                                            XmNtopOffset,      MARGIN,
                                            XmNleftAttachment, XmATTACH_FORM,
                                            XmNleftOffset,     MARGIN,
                                            NULL);
   XmStringFree(xs);

   /* put a seperator after the warning message */
   warningSeparator = XtVaCreateManagedWidget("warningSeparator",
                                              xmSeparatorGadgetClass, outerForm,
                                              XmNleftAttachment,  XmATTACH_FORM,
                                              XmNrightAttachment, XmATTACH_FORM,
                                              XmNtopAttachment,   XmATTACH_WIDGET,
                                              XmNtopWidget,       warningMessage,
                                              XmNtopOffset,       MARGIN,
                                              NULL);

   /* create file name field ... if the input file is a directory, this */
   /*    widget may end up being un-managed                             */
   /* the text for the label is generated when the filename is known    */
   fileName = XtVaCreateManagedWidget("fileName",
                                      xmLabelGadgetClass, outerForm,
                                      XmNalignment,      XmALIGNMENT_BEGINNING,
                                      XmNtopAttachment,  XmATTACH_WIDGET,
                                      XmNtopWidget,      warningSeparator,
                                      XmNtopOffset,      MARGIN,
                                      XmNleftAttachment, XmATTACH_FORM,
                                      XmNleftOffset,     MARGIN,
                                      XmNrightOffset,    MARGIN,
                                      NULL);


   /* put a seperator after the file name */
   fileNameSeparator = XtVaCreateManagedWidget("fileNameSeparator",
                                               xmSeparatorGadgetClass, outerForm,
                                               XmNleftAttachment,  XmATTACH_FORM,
                                               XmNrightAttachment, XmATTACH_FORM,
                                               XmNtopAttachment,   XmATTACH_WIDGET,
                                               XmNtopWidget,       fileName,
                                               XmNtopOffset,       MARGIN,
                                               NULL);


   /* create folder name field ... the text will be supplied when the name is known */
   folderName = XtVaCreateManagedWidget("folderName",
                                        xmLabelGadgetClass, outerForm,
                                        XmNalignment,      XmALIGNMENT_BEGINNING,
                                        XmNtopAttachment,  XmATTACH_WIDGET,
                                        XmNtopWidget,      fileNameSeparator,
                                        XmNtopOffset,      MARGIN,
                                        XmNleftAttachment, XmATTACH_FORM,
                                        XmNleftOffset,     MARGIN,
                                        XmNrightOffset,    MARGIN,
                                        NULL);

   /* create dialog mode (advanced/basic) radio box */
   /* so modeRadioTitle and modeRadio were centered, it is neccessary */
   /*    to insure they have the same height, thus manipulations of   */
   /*    height1 and height2                                          */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 19, "Permissions List:")), XmFONTLIST_DEFAULT_TAG);
   modeRadioTitle = XtVaCreateManagedWidget("modeRadioTitle",
                                   xmLabelGadgetClass, outerForm,
                                   XmNlabelString,     xs,
                                   XmNleftAttachment,  XmATTACH_FORM,
                                   XmNleftOffset,      MARGIN,
                                   XmNtopAttachment,   XmATTACH_WIDGET,
                                   XmNtopWidget,       folderName,
                                   XmNtopOffset,       2*MARGIN,
                                   NULL);
   XmStringFree(xs);

   n = 0;
   XtSetArg(args[n], XmNorientation,      XmHORIZONTAL);		n++;
   XtSetArg(args[n], XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET);	n++;
   XtSetArg(args[n], XmNtopWidget,        modeRadioTitle);		n++;
   XtSetArg(args[n], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET);	n++;
   XtSetArg(args[n], XmNbottomWidget,     modeRadioTitle);		n++;
   XtSetArg(args[n], XmNleftAttachment,   XmATTACH_WIDGET);		n++;
   XtSetArg(args[n], XmNleftWidget,       modeRadioTitle);		n++;
   modeRadio = XmCreateRadioBox(outerForm, "modeRadio", args, n);

   xs = XmStringCreateLtoR ((GETMESSAGE(1, 21, "Basic")), XmFONTLIST_DEFAULT_TAG);
   basicToggle    = XtVaCreateManagedWidget("basicToggle",
                                            xmToggleButtonGadgetClass, modeRadio,
                                            XmNalignment,   XmALIGNMENT_BEGINNING,
                                            XmNlabelString, xs,
                                            NULL);
   XmStringFree(xs);

   xs = XmStringCreateLtoR ((GETMESSAGE(1, 20, "Advanced")), XmFONTLIST_DEFAULT_TAG);
   advancedToggle = XtVaCreateManagedWidget("advancedToggle",
                                            xmToggleButtonGadgetClass, modeRadio,
                                            XmNalignment,   XmALIGNMENT_BEGINNING,
                                            XmNlabelString, xs,
                                            NULL);
   XmStringFree(xs);
   XtAddCallback(advancedToggle, XmNvalueChangedCallback, modeCB, context);

   /* set the heights of the radio box and its label */
   /* this is done twice because once didn't work (Motif bug??) */
   /*    after the first call to SetValues, the height was      */
   /*    neither height1 or height2 but some different value    */
   XtVaGetValues(modeRadioTitle, XmNheight, &height1, NULL);
   XtVaGetValues(modeRadio,      XmNheight, &height2, NULL);
   if (height1 > height2)
      XtVaSetValues(modeRadio,      XmNheight, height1, NULL);
   else
      XtVaSetValues(modeRadioTitle, XmNheight, height2, NULL);

   XtVaGetValues(modeRadioTitle, XmNheight, &height1, NULL);
   XtVaGetValues(modeRadio,      XmNheight, &height2, NULL);
   if (height1 > height2)
      XtVaSetValues(modeRadio,      XmNheight, height1, NULL);
   else
      XtVaSetValues(modeRadioTitle, XmNheight, height2, NULL);

   XtManageChild(modeRadio);



   /* create "show negative permissions" (yes/no) radio box */
   /* so showNegRadioTitle and showNegRadio were centered, it is neccessary */
   /*    to insure they have the same height, thus manipulations of   */
   /*    height1 and height2                                          */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 55, "Negative Permissions:")), XmFONTLIST_DEFAULT_TAG);
   showNegRadioTitle = XtVaCreateManagedWidget("showNegTitle",
                                               xmLabelGadgetClass, outerForm,
                                               XmNlabelString,     xs,
                                               XmNleftAttachment,  XmATTACH_FORM,
                                               XmNleftOffset,      MARGIN,
                                               XmNtopAttachment,   XmATTACH_WIDGET,
                                               XmNtopWidget,       modeRadioTitle,
                                               XmNtopOffset,       MARGIN,
                                               NULL);
   XmStringFree(xs);

   n = 0;
   XtSetArg(args[n], XmNorientation,      XmHORIZONTAL);		n++;
   XtSetArg(args[n], XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET);	n++;
   XtSetArg(args[n], XmNtopWidget,        showNegRadioTitle);		n++;
   XtSetArg(args[n], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET);	n++;
   XtSetArg(args[n], XmNbottomWidget,     showNegRadioTitle);		n++;
   XtSetArg(args[n], XmNleftAttachment,   XmATTACH_WIDGET);		n++;
   XtSetArg(args[n], XmNleftWidget,       showNegRadioTitle);		n++;
   showNegRadio = XmCreateRadioBox(outerForm, "showNegRadio", args, n);

   xs = XmStringCreateLtoR ((GETMESSAGE(1, 57, "Hide")), XmFONTLIST_DEFAULT_TAG);
   hideNegToggle = XtVaCreateManagedWidget("hideNegToggle",
                                            xmToggleButtonGadgetClass, showNegRadio,
                                            XmNalignment,   XmALIGNMENT_BEGINNING,
                                            XmNlabelString, xs,
                                            NULL);
   XmStringFree(xs);

   xs = XmStringCreateLtoR ((GETMESSAGE(1, 56, "Show")), XmFONTLIST_DEFAULT_TAG);
   showNegToggle = XtVaCreateManagedWidget("showNegToggle",
                                            xmToggleButtonGadgetClass, showNegRadio,
                                            XmNalignment,   XmALIGNMENT_BEGINNING,
                                            XmNlabelString, xs,
                                            XmNset,         FALSE,
                                            NULL);
   XmStringFree(xs);
   XtAddCallback(showNegToggle, XmNvalueChangedCallback, showNegCB, context);

   /* set the heights of the radio box and its label */
   /* this is done twice because once didn't work (Motif bug??) */
   /*    after the first call to SetValues, the height was      */
   /*    neither height1 or height2 but some different value    */
   XtVaGetValues(showNegRadioTitle,    XmNheight, &height1, NULL);
   XtVaGetValues(showNegRadio,         XmNheight, &height2, NULL);
   if (height1 > height2)
      XtVaSetValues(showNegRadio,      XmNheight, height1, NULL);
   else
      XtVaSetValues(showNegRadioTitle, XmNheight, height2, NULL);

   XtVaGetValues(showNegRadioTitle,    XmNheight, &height1, NULL);
   XtVaGetValues(showNegRadio,         XmNheight, &height2, NULL);
   if (height1 > height2)
      XtVaSetValues(showNegRadio,      XmNheight, height1, NULL);
   else
      XtVaSetValues(showNegRadioTitle, XmNheight, height2, NULL);

   XtManageChild(showNegRadio);

   /* create the summary label used in lieu of negative acl form when the */
   /*    user chooses to hide negative permissions */
   summary = XtVaCreateManagedWidget("summary",
                              xmLabelGadgetClass,  outerForm,
                              XmNalignment,        XmALIGNMENT_BEGINNING,
                              XmNtopAttachment,    XmATTACH_OPPOSITE_WIDGET,
                              XmNtopWidget,        showNegRadio,
                              XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
                              XmNbottomWidget,     showNegRadio,
                              XmNleftAttachment,   XmATTACH_WIDGET,
                              XmNleftWidget,       showNegRadio,
                              XmNleftOffset,       2*MARGIN,
                              NULL);

   /* copy local widgets into context structure */
   context->outerFrame         = outerFrame;
   context->outerForm          = outerForm;
   context->fileName           = fileName;
   context->fileName           = fileName;
   context->fileNameSeparator  = fileNameSeparator;
   context->folderName         = folderName;
   context->warningMessage     = warningMessage;
   context->warningSeparator   = warningSeparator;
   context->modeRadio          = modeRadio;
   context->modeRadioTitle     = modeRadioTitle;
   context->advancedToggle     = advancedToggle;
   context->basicToggle        = basicToggle;
   context->showNegRadio       = showNegRadio;
   context->showNegRadioTitle  = showNegRadioTitle;
   context->showNegToggle      = showNegToggle;
   context->hideNegToggle      = hideNegToggle;
   context->summary            = summary;

   /* create action area -- done before Advanced Mode and Basic Mode forms */
   /*    so they be be attached to it                                      */
   createActionArea(context);

   /* create widgets for Acl that do not depend on mode (advanced vs basic) */
   /* the frames and titles for normal and negative acl's don't depend on   */
   /*    mode per se, but have to be in the skeleton because the frame      */
   /*    can have only one working-area child  */
   aclForm = XtVaCreateWidget("aclForm",
                              xmFormWidgetClass, outerForm,
                              XmNleftAttachment,    XmATTACH_FORM,
                              XmNrightAttachment,   XmATTACH_FORM,
                              XmNtopAttachment,     XmATTACH_WIDGET,
                              XmNtopWidget,         context->showNegRadioTitle,
                              XmNtopOffset,         MARGIN,
                              XmNbottomAttachment,  XmATTACH_WIDGET,
                              XmNbottomWidget,      context->actionForm,
                              XmNverticalSpacing,   MARGIN,
                              XmNhorizontalSpacing, 2*MARGIN,
                              NULL);

   /* copy local widgets into context structure */
   context->aclForm    = aclForm;

   /* initialize gui representation of ACL */
   initAcl(&(context->aclDisplay));


   /* create skeleton forms */
   createAclSkeleton(ADVNEG,   context);
   XtVaSetValues(aclMgr(context->advNeg),
                 XmNtopAttachment,    XmATTACH_NONE,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNbottomOffset,     2*MARGIN,
                 NULL);
   createAclSkeleton(ADVNORM,  context);
   XtVaSetValues(aclMgr(context->advNorm),
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 XmNbottomWidget,     aclMgr(context->advNeg),
                 XmNbottomOffset,     2*MARGIN,
                 XmNtopAttachment,    XmATTACH_FORM,
                 NULL);

   createAclSkeleton(BASICNEG,   context);
   XtVaSetValues(aclMgr(context->basicNeg),
                 XmNtopAttachment,    XmATTACH_NONE,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNbottomOffset,     2*MARGIN,
                 NULL);
   createAclSkeleton(BASICNORM,  context);
   XtVaSetValues(aclMgr(context->basicNorm),
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 XmNbottomWidget,     aclMgr(context->basicNeg),
                 XmNbottomOffset,     2*MARGIN,
                 XmNtopAttachment,    XmATTACH_FORM,
                 NULL);



   /* now manage the form */
   XtManageChild(aclForm);

   /* now that the children have been created ... */
   XtManageChild(outerForm);


}  /* end createDialog */


/*------------------------------------------------------------------------
 *
 *  createActionArea
 *
 *  create a separator and form with the action buttons (ok, reset ...)
 *
 *-----------------------------------------------------------------------*/
 
static void
#ifdef _NO_PROTO
createActionArea(context)
   DialogContext *context;
#else
createActionArea(DialogContext *context)
#endif

{  
   
#define LEFTPOSITION(i)   (1 + (i) * ((spacingRatio)+1))
#define RIGHTPOSITION(i)  ((LEFTPOSITION((i))) + (spacingRatio))

   XmString xs;

   /* local widgets */
   Widget actionSeparator;
   Widget actionForm;
   Widget okButton;
   Widget applyButton;
   Widget cancelButton;
   Widget resetButton;
   Widget optionsButton;
   Widget helpButton;
   
   const int numButtons=6;
   const int spacingRatio=4;     /* width of buttons / space between buttons */ 
   int       buttonNumber;       /* number buttons left to right: 0, 1, ..., numButtons-1 */
     
   
   /* create action-button form at bottom of window */ 
   /* with this definition of fractionBase, button width=spacingRatio, */
   /*    and buttons are spaced 1 unit apart */
   actionForm = XtVaCreateWidget("actionForm",
                                 xmFormWidgetClass, context->outerForm,
                                 XmNrightAttachment,  XmATTACH_FORM,
                                 XmNleftAttachment,   XmATTACH_FORM,
                                 XmNbottomAttachment, XmATTACH_FORM,
                                 XmNbottomOffset,     MARGIN,
                                 XmNfractionBase,     numButtons*(spacingRatio+1)+1,
                                 NULL); 
                                        
   /* put a separator above the action area */
   actionSeparator = XtVaCreateManagedWidget("actionSeparator",
                                             xmSeparatorGadgetClass, actionForm,
                                             XmNleftAttachment,   XmATTACH_FORM,
                                             XmNrightAttachment,  XmATTACH_FORM,
                                             XmNtopAttachment,    XmATTACH_FORM,
                                             NULL);
        
   /* create OK button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 5, "OK")), XmFONTLIST_DEFAULT_TAG);
   buttonNumber = 0;
   okButton = XtVaCreateManagedWidget("okButton",
                                      xmPushButtonGadgetClass, actionForm,
                                      XmNlabelString,      xs,
                                      XmNtopAttachment,    XmATTACH_WIDGET,
                                      XmNtopWidget,        actionSeparator,
                                      XmNtopOffset,        MARGIN,
                                      XmNbottomAttachment, XmATTACH_FORM,
                                      XmNleftAttachment,   XmATTACH_POSITION,
                                      XmNleftPosition,     LEFTPOSITION(buttonNumber),
                                      XmNrightAttachment,  XmATTACH_POSITION,
                                      XmNrightPosition,    RIGHTPOSITION(buttonNumber),
                                      NULL);
   XtAddCallback(okButton, XmNactivateCallback, applyCB, context);
   XmStringFree(xs);
        
   /* create Apply button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 6, "Apply")), XmFONTLIST_DEFAULT_TAG);
   buttonNumber = 1;
   applyButton = XtVaCreateManagedWidget("applyButton",
                                         xmPushButtonGadgetClass, actionForm,
                                         XmNlabelString,      xs,
                                         XmNtopAttachment,    XmATTACH_WIDGET,
                                         XmNtopWidget,        actionSeparator,
                                         XmNtopOffset,        MARGIN,
                                         XmNbottomAttachment, XmATTACH_FORM,
                                         XmNleftAttachment,   XmATTACH_POSITION,
                                         XmNleftPosition,     LEFTPOSITION(buttonNumber),
                                         XmNrightAttachment,  XmATTACH_POSITION,
                                         XmNrightPosition,    RIGHTPOSITION(buttonNumber),
                                         NULL);
   XmStringFree(xs);
   XtAddCallback(applyButton, XmNactivateCallback, applyCB, context);

   /* create Cancel button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 8, "Cancel")), XmFONTLIST_DEFAULT_TAG);
   buttonNumber = 2;
   cancelButton = XtVaCreateManagedWidget("cancelButton",
                                          xmPushButtonGadgetClass, actionForm,
                                          XmNlabelString,      xs,
                                          XmNtopAttachment,    XmATTACH_WIDGET,
                                          XmNtopWidget,        actionSeparator,
                                          XmNtopOffset,        MARGIN,
                                          XmNbottomAttachment, XmATTACH_FORM,
                                          XmNleftAttachment,   XmATTACH_POSITION,
                                          XmNleftPosition,     LEFTPOSITION(buttonNumber),
                                          XmNrightAttachment,  XmATTACH_POSITION,
                                          XmNrightPosition,    RIGHTPOSITION(buttonNumber),
                                          NULL);
   XmStringFree(xs);
   XtAddCallback(cancelButton, XmNactivateCallback, cancelCB, context);

   /* create Reset button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 7, "Reset")), XmFONTLIST_DEFAULT_TAG);
   buttonNumber = 3;
   resetButton = XtVaCreateManagedWidget("resetButton",
                                         xmPushButtonGadgetClass, actionForm,
                                         XmNlabelString,      xs,
                                         XmNtopAttachment,    XmATTACH_WIDGET,
                                         XmNtopWidget,        actionSeparator,
                                         XmNtopOffset,        MARGIN,
                                         XmNbottomAttachment, XmATTACH_FORM,
                                         XmNleftAttachment,   XmATTACH_POSITION,
                                         XmNleftPosition,     LEFTPOSITION(buttonNumber),
                                         XmNrightAttachment,  XmATTACH_POSITION,
                                         XmNrightPosition,    RIGHTPOSITION(buttonNumber),
                                         NULL);
   XmStringFree(xs);
   XtAddCallback(resetButton, XmNactivateCallback, resetCB, context);
        
   /* create Options button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 61, "Options")), XmFONTLIST_DEFAULT_TAG);
   buttonNumber = 4;
   optionsButton = XtVaCreateManagedWidget("optionsButton",
                                           xmPushButtonGadgetClass, actionForm,
                                           XmNlabelString,      xs,
                                           XmNtopAttachment,    XmATTACH_WIDGET,
                                           XmNtopWidget,        actionSeparator,
                                           XmNtopOffset,        MARGIN,
                                           XmNbottomAttachment, XmATTACH_FORM,
                                           XmNleftAttachment,   XmATTACH_POSITION,
                                           XmNleftPosition,     LEFTPOSITION(buttonNumber),
                                           XmNrightAttachment,  XmATTACH_POSITION,
                                           XmNrightPosition,    RIGHTPOSITION(buttonNumber),
                                           NULL);
   XmStringFree(xs); 
/*   XtAddCallback(resetButton, XmNactivateCallback, optionsCB, context); */
        
   /* create Help button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 9, "Help")), XmFONTLIST_DEFAULT_TAG);
   buttonNumber = 5;
   helpButton = XtVaCreateManagedWidget("helpButton",
                                         xmPushButtonGadgetClass, actionForm,
                                         XmNlabelString,      xs,
                                         XmNtopAttachment,    XmATTACH_WIDGET,
                                         XmNtopWidget,        actionSeparator,
                                         XmNtopOffset,        MARGIN,
                                         XmNbottomAttachment, XmATTACH_FORM,
                                         XmNleftAttachment,   XmATTACH_POSITION,
                                         XmNleftPosition,     LEFTPOSITION(buttonNumber),
                                         XmNrightAttachment,  XmATTACH_POSITION,
                                         XmNrightPosition,    RIGHTPOSITION(buttonNumber),
                                         NULL);
   XmStringFree(xs); 
/*   XtAddCallback(resetButton, XmNactivateCallback, resetCB, context); */
     
   /* define the default action, so the Enter (kActivate) key will do something */
   XtVaSetValues(actionForm,
                 XmNdefaultButton, cancelButton,
                 NULL);

   /* now manage the form */
   XtManageChild(actionForm);
   
   
   /* copy local widgets to dialog context */
   context->actionSeparator = actionSeparator;
   context->actionForm      = actionForm;
   context->okButton        = okButton;
   context->applyButton     = applyButton;
   context->cancelButton    = cancelButton;
   context->resetButton     = resetButton;
   context->optionsButton   = optionsButton;
   context->helpButton      = helpButton;


}   /* end createActionArea */


/*------------------------------------------------------------------------
 *
 *  createAclSkeleton
 *
 *  create the form and enclosed widgets which hold an ACL
 *  the manager widget for the skeleton will not be managed
 *  the labels & toggles for users/groups in an acl are created as needed
 *     in setDisplayedAcl
 *
 *-----------------------------------------------------------------------*/
 
static void
#ifdef _NO_PROTO
createAclSkeleton(myType, context)
   SkeletonType     myType;
   DialogContext  * context;
#else
createAclSkeleton  (SkeletonType     myType,
                    DialogContext  * context);
#endif

{   
   XmString       xs;
   String         name;
   String         names[4] = {"advNorm", "advNeg", "basicNorm", "basicNeg"};
   DialogMode     mode;
   AclSkeleton  * skeleton;
   XmString       titleString;
   Widget         parent;

   /* local widgets */
   Widget         frame;
   Widget         title;
   Widget         form;
   Widget         scroll;
   Widget         scrollForm;
   Widget         addButton;
   Widget         deleteButton;

   Widget         TGMgr;



   parent = context->aclForm;

   /* initialize skeleton-type specific variables */
   switch (myType)
   {
      case ADVNORM:
         name              = names[0];
         skeleton          = &context->advNorm;
         mode              = ADVANCED;
         titleString       = XmStringCreateLtoR ((GETMESSAGE(1, 11, "Positive Permissions")),
                                                 XmFONTLIST_DEFAULT_TAG);
         break;
      case ADVNEG:
         name              = names[1];
         skeleton          = &context->advNeg;
         mode              = ADVANCED;
         titleString       = XmStringCreateLtoR ((GETMESSAGE(1, 12, "Negative Permissions")),
                                                 XmFONTLIST_DEFAULT_TAG);
         break;
      case BASICNORM:
         name              = names[2];
         skeleton          = &context->basicNorm;
         mode              = BASIC;
         titleString       = XmStringCreateLtoR ((GETMESSAGE(1, 11, "Positive Permissions")),
                                                 XmFONTLIST_DEFAULT_TAG);
         break;
      case BASICNEG:
         name              = names[3];
         skeleton          = &context->basicNeg;
         mode              = BASIC;
         titleString       = XmStringCreateLtoR ((GETMESSAGE(1, 12, "Negative Permissions")),
                                                 XmFONTLIST_DEFAULT_TAG);
         break;

   }

   /* create the enclosing frame and form */
   /* leave top and bottom attachments for the calling function */
   frame = XtVaCreateWidget(name,
                            xmFrameWidgetClass, parent,
                            XmNrightAttachment,  XmATTACH_FORM,
                            XmNleftAttachment,   XmATTACH_FORM,
                            XmNshadowType,       XmSHADOW_OUT,
                            NULL);

   title = XtVaCreateManagedWidget("skeletonTitle",
                                   xmLabelGadgetClass, frame,
                                   XmNlabelString, titleString,
                                   XmNchildType,   XmFRAME_TITLE_CHILD,
                                   NULL);

   form = XtVaCreateWidget("skeletonForm",
                           xmFormWidgetClass, frame,
                           XmNchildType,         XmFRAME_WORKAREA_CHILD,
                           XmNverticalSpacing,   MARGIN,
                           XmNhorizontalSpacing, MARGIN,
                           NULL);

   /* create add and delete buttons */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 39, "Add user ...")), XmFONTLIST_DEFAULT_TAG);
   addButton = XtVaCreateManagedWidget("addButton",
                                       xmPushButtonGadgetClass, form,
                                       XmNlabelString,      xs,
                                       XmNbottomAttachment, XmATTACH_FORM,
                                       XmNleftAttachment,   XmATTACH_FORM,
                                       NULL);
   XtAddCallback(addButton, XmNactivateCallback, addCB, context);
   XmStringFree(xs);

   xs = XmStringCreateLtoR ((GETMESSAGE(1, 40, "Delete user ...")), XmFONTLIST_DEFAULT_TAG);
   deleteButton = XtVaCreateManagedWidget("deleteButton",
                                       xmPushButtonGadgetClass, form,
                                       XmNlabelString,      xs,
                                       XmNbottomAttachment, XmATTACH_FORM,
                                       XmNleftAttachment,   XmATTACH_WIDGET,
                                       XmNleftWidget,       addButton,
                                       XmNleftOffset,       4*MARGIN,
                                       NULL);
   XtAddCallback(deleteButton, XmNactivateCallback, deleteCB, context); 
   XmStringFree(xs);

   /* create scrolled window to hold acl */
   scroll = XtVaCreateManagedWidget("skeletonScroll",
                                    xmScrolledWindowWidgetClass, form,
                                    XmNscrollingPolicy,        XmAUTOMATIC,
                                    XmNscrollBarDisplayPolicy, XmSTATIC,
                                    XmNrightAttachment,        XmATTACH_FORM,
                                    XmNleftAttachment,         XmATTACH_FORM,
                                    XmNtopAttachment,          XmATTACH_FORM,
                                    XmNbottomAttachment,       XmATTACH_WIDGET,
                                    XmNbottomWidget,           addButton,
                                    NULL);

   scrollForm = XtVaCreateManagedWidget("skeletonScrollForm",
                                        xmFormWidgetClass, scroll,
                                        XmNhorizontalSpacing, MARGIN,
                                        XmNverticalSpacing,   MARGIN,
                                        NULL);


   XtManageChild(form);

   XmStringFree(titleString);


   /* copy local widgets to context structure */
   skeleton->type         = myType;
   skeleton->frame        = frame;
   skeleton->title        = title;
   skeleton->form         = form;
   skeleton->scroll       = scroll;
   skeleton->scrollForm   = scrollForm;
   skeleton->addButton    = addButton;
   skeleton->deleteButton = deleteButton;

}  /* end createAclSkeleton */


/*---------------------------------------------------------------
 *
 *   createWMCB
 *
 *   invoke cancel callback if window manager close is invoked
 *
 *--------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
createWMCB(shell, context)
   Widget shell;
   DialogContext *context;
#else
createWMCB(Widget shell, DialogContext *context)
#endif

{
   Atom WM_DELETE_WINDOW;

   XtVaSetValues(shell,
                 XmNdeleteResponse, XmDO_NOTHING,
                 NULL);
   WM_DELETE_WINDOW = XmInternAtom(XtDisplay(shell), "WM_DELETE_WINDOW", FALSE);
   XmAddWMProtocolCallback(shell, WM_DELETE_WINDOW, cancelCB, context);


}  /* end createWMCB */


/*--------------------------------------------------------------
 *
 *  initAcl
 *
 *  make the Acl strucute ready for users/groups to be added
 *
 *------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
initAcl(aclDisplay)
   AclDisplay  * aclDisplay;
#else
initAcl(AclDisplay * aclDisplay)
#endif

{
   int i;


   aclDisplay->nNormal   = 0;
   aclDisplay->nNegative = 0;
   for (i = 0; i < maxAclEntries(); i++)
   {
      aclDisplay->advNormAcl[i]   = NULL;
      aclDisplay->advNegAcl[i]    = NULL;
      aclDisplay->basicNormAcl[i] = NULL;
      aclDisplay->basicNegAcl[i]  = NULL;
   }

}  /* end initAcl */


/*---------------------------------------------------------------
 *
 *  newAclLine
 *
 *  create widgets needed to display user/group name and
 *     toggles for the rights
 *  the enclosing form widget is not managed
 *
 *--------------------------------------------------------------*/


AclLine *
#ifdef _NO_PROTO
newAclLine (parent, mode)
   Widget      parent;
   DialogMode  mode;
#else
newAclLine (Widget parent, DialogMode mode)
#endif

{

   AclLine * myAclLine = XtNew(AclLine);

   myAclLine->aclForm = XtVaCreateWidget("aclLineForm",
                                         xmFormWidgetClass, parent,
                                         XmNleftAttachment,    XmATTACH_FORM,
                                         XmNrightAttachment,   XmATTACH_FORM,
                                         NULL);

   myAclLine->TG = TGNew(myAclLine->aclForm, mode);
   XtVaSetValues(TGmanager(myAclLine->TG),
                 XmNrightAttachment,   XmATTACH_FORM,
                 XmNbottomAttachment,  XmATTACH_FORM,
                 XmNtopAttachment,     XmATTACH_FORM,
                 NULL);

   myAclLine->name    = XtVaCreateManagedWidget("aclName",
                                         xmLabelGadgetClass, myAclLine->aclForm,
                                         XmNalignment,         XmALIGNMENT_BEGINNING,
                                         XmNtopAttachment,     XmATTACH_FORM,
                                         XmNbottomAttachment,  XmATTACH_FORM,
                                         XmNleftAttachment,    XmATTACH_FORM,
                                         XmNrightAttachment,   XmATTACH_WIDGET,
                                         XmNrightWidget,       TGmanager(myAclLine->TG),
                                         NULL);

   return myAclLine;


}  /* end newAclLine */


/*------------------------------------------------------------------------
 *
 *  createAddDialog
 *
 *  create the dialog for adding new users/groups to the ACL
 *
 *-----------------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
createAddDialog(context)
   DialogContext *context;
#else
createAddDialog(DialogContext *context)
#endif

{

   const int  nActionButtons=3;
   XmString   xs;

   /* the global, top-level shell */
   extern Widget dtAclAfsShell;

   /* local widgets */
   Widget addDialogShell;
   Widget addForm;
   Widget addActionForm;
   Widget addActionSeparator;
   Widget addOkButton;
   Widget addCancelButton;
   Widget addHelpButton;
   Widget addWorkForm;
   Widget addNewuserText;
   ToggleGroup  * addAdvNewuserTG;
   ToggleGroup  * addBasNewuserTG;


   /* allow resize requests from children to account for basic vs advanced modes */
   /*   when the dialog is popped up */
   /* the title will be set when the dialog is popped up to indicate normal vs neg ACL */
   addDialogShell = XtVaCreatePopupShell("addDialogShell",
                                         xmDialogShellWidgetClass, dtAclAfsShell,
                                         XmNallowShellResize,      True,
                                         NULL);
  
   /* create top-level form for dialog */
   addForm = XtVaCreateWidget("addForm",
                              xmFormWidgetClass, addDialogShell,
                              XmNdialogStyle,  XmDIALOG_FULL_APPLICATION_MODAL,
                              NULL);

   /* create action-button form at bottom of window */
   addActionForm = XtVaCreateWidget("addActionForm",
                                    xmFormWidgetClass, addForm,
                                    XmNrightAttachment,  XmATTACH_FORM,
                                    XmNleftAttachment,   XmATTACH_FORM,
                                    XmNbottomAttachment, XmATTACH_FORM,
                                    XmNtopOffset,        MARGIN,
                                    XmNfractionBase,     2*nActionButtons + nActionButtons + 1,
                                    NULL);

   /* put a separator above the action area */
   addActionSeparator = XtVaCreateManagedWidget("addActionSeparator",
                                                xmSeparatorGadgetClass, addActionForm,
                                                XmNleftAttachment,   XmATTACH_FORM,
                                                XmNrightAttachment,  XmATTACH_FORM,
                                                XmNtopAttachment,    XmATTACH_FORM,
                                                NULL);

   /* create OK button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 5, "OK")), XmFONTLIST_DEFAULT_TAG);
   addOkButton = XtVaCreateManagedWidget("addOkButton",
                                         xmPushButtonGadgetClass, addActionForm,
                                         XmNlabelString,      xs,
                                         XmNtopAttachment,    XmATTACH_WIDGET,
                                         XmNtopWidget,        addActionSeparator,
                                         XmNtopOffset,        MARGIN,
                                         XmNbottomAttachment, XmATTACH_FORM,
                                         XmNleftAttachment,   XmATTACH_POSITION,
                                         XmNleftPosition,     1,
                                         XmNrightAttachment,  XmATTACH_POSITION,
                                         XmNrightPosition,    3,
                                         NULL);
   XtAddCallback(addOkButton, XmNactivateCallback, addOkCB, context);
   XmStringFree(xs);

   /* create Cancel button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 8, "Cancel")), XmFONTLIST_DEFAULT_TAG);
   addCancelButton = XtVaCreateManagedWidget("addCancelButton",
                                         xmPushButtonGadgetClass, addActionForm,
                                         XmNlabelString,      xs,
                                         XmNtopAttachment,    XmATTACH_WIDGET,
                                         XmNtopWidget,        addActionSeparator,
                                         XmNtopOffset,        MARGIN,
                                         XmNbottomAttachment, XmATTACH_FORM,
                                         XmNleftAttachment,   XmATTACH_POSITION,
                                         XmNleftPosition,     4,
                                         XmNrightAttachment,  XmATTACH_POSITION,
                                         XmNrightPosition,    6,
                                         NULL);
   XtAddCallback(addCancelButton, XmNactivateCallback, addCancelCB, context);
   XmStringFree(xs);


   /* create Help button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 9, "Help")), XmFONTLIST_DEFAULT_TAG);
   addHelpButton = XtVaCreateManagedWidget("addHelpButton",
                                         xmPushButtonGadgetClass, addActionForm,
                                         XmNlabelString,      xs,
                                         XmNtopAttachment,    XmATTACH_WIDGET,
                                         XmNtopWidget,        addActionSeparator,
                                         XmNtopOffset,        MARGIN,
                                         XmNbottomAttachment, XmATTACH_FORM,
                                         XmNleftAttachment,   XmATTACH_POSITION,
                                         XmNleftPosition,     7,
                                         XmNrightAttachment,  XmATTACH_POSITION,
                                         XmNrightPosition,    9,
                                         NULL);
/*   XtAddCallback(addHelpButton, XmNactivateCallback, , context); */
   XmStringFree(xs);

   /* define the default action, so the Enter (kActivate) key will do something */
   XtVaSetValues(addActionForm,
                 XmNdefaultButton, addOkButton,
                 NULL);

   /* create the text field and toggle groups for adding new users */
   /* since basic vs. advanced mode is decided when the dialog is popped up, */
   /*    some geometry constraints are left until we know advanced vs. basic */
   /*    and are set in setAddDialogMode */
   addWorkForm = XtVaCreateWidget("addWorkForm",
                                  xmFormWidgetClass, addForm,
                                  XmNtopAttachment,    XmATTACH_FORM,
                                  XmNtopOffset,        MARGIN,
                                  XmNleftAttachment,   XmATTACH_FORM,
                                  XmNleftOffset,       MARGIN,
                                  XmNrightAttachment,  XmATTACH_FORM,
                                  XmNrightOffset,      MARGIN,
                                  XmNbottomAttachment, XmATTACH_WIDGET,
                                  XmNbottomWidget,     addActionSeparator,
                                  XmNbottomOffset,     MARGIN,
                                  NULL);

   addAdvNewuserTG = TGNew(addWorkForm, ADVANCED);
   addBasNewuserTG = TGNew(addWorkForm, BASIC);

   XtVaSetValues(TGmanager(addAdvNewuserTG),
                 XmNrightAttachment,   XmATTACH_FORM,
                 XmNbottomAttachment,  XmATTACH_FORM,
                 XmNtopAttachment,     XmATTACH_FORM,
                 NULL);
   XtVaSetValues(TGmanager(addBasNewuserTG),
                 XmNrightAttachment,   XmATTACH_FORM,
                 XmNbottomAttachment,  XmATTACH_FORM,
                 XmNtopAttachment,     XmATTACH_FORM,
                 NULL);

   addNewuserText = XtVaCreateManagedWidget("addNewuserText",
                                            xmTextFieldWidgetClass, addWorkForm,
                                            XmNmaxLength,        aclMaxNameLen(),
                                            XmNleftAttachment,   XmATTACH_FORM,
                                            XmNbottomAttachment, XmATTACH_FORM,
                                            XmNtopAttachment,    XmATTACH_FORM,
                                            NULL);


   /* we manage the forms here, but not the child of the shell which is managed */
   /*    when the user presses an add button, causing the window to appear */
   XtManageChild(addWorkForm);
   XtManageChild(addActionForm);

   /* copy add Dialog elements to context structure */
   context->addDialogShell     = addDialogShell;
   context->addForm            = addForm;
   context->addActionForm      = addActionForm;
   context->addActionSeparator = addActionSeparator;
   context->addOkButton        = addOkButton;
   context->addCancelButton    = addCancelButton;
   context->addHelpButton      = addHelpButton;
   context->addWorkForm        = addWorkForm;
   context->addNewuserText     = addNewuserText;
   context->addAdvNewuserTG    = addAdvNewuserTG;
   context->addBasNewuserTG    = addBasNewuserTG;

}  /* end createAddDialog */


/*------------------------------------------------------------------------
 *
 *  createDeleteDialog
 *
 *  create the dialog for deleting users/groups from the ACL 
 *
 *-----------------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
createDeleteDialog(context)
   DialogContext *context;
#else
createDeleteDialog(DialogContext *context)
#endif

{

   const int  nActionButtons=3;
   XmString   xs;

   /* the global, top-level shell */
   extern Widget dtAclAfsShell;

   /* local widgets */
   Widget deleteDialogShell;
   Widget deleteForm;
   Widget deleteActionForm; 
   Widget deleteInstructions;
   Widget deleteInstSeparator;
   Widget deleteActionSeparator;
   Widget deleteOkButton;
   Widget deleteCancelButton;
   Widget deleteHelpButton;
   Widget deleteList;
  
   /* allow resize requests from children to account for differing list lengths */
   /* the title will be set when the dialog is popped up to indicate normal vs neg ACL */
   deleteDialogShell = XtVaCreatePopupShell("deleteDialogShell",
                                            xmDialogShellWidgetClass, dtAclAfsShell,
                                            XmNallowShellResize,      True,
                                            NULL);  

   /* create top-level form for dialog ... not managed until user presses "Delete ..." button */
   deleteForm = XtVaCreateWidget("deleteForm",
                                 xmFormWidgetClass, deleteDialogShell,
                                 XmNdialogStyle,  XmDIALOG_FULL_APPLICATION_MODAL,
                                 NULL); 

   /* create action-button form at bottom of window */
   deleteActionForm = XtVaCreateWidget("deleteActionForm",
                                       xmFormWidgetClass, deleteForm,
                                       XmNrightAttachment,  XmATTACH_FORM,
                                       XmNleftAttachment,   XmATTACH_FORM,
                                       XmNbottomAttachment, XmATTACH_FORM,
                                       XmNbottomOffset, 10,
                                       XmNfractionBase, 2*nActionButtons + nActionButtons + 1,
                                       NULL);

   /* put a separator above the action area */
   deleteActionSeparator = XtVaCreateManagedWidget("deleteActionSeparator",
                                                   xmSeparatorGadgetClass, deleteActionForm,
                                                   XmNleftAttachment,   XmATTACH_FORM,
                                                   XmNrightAttachment,  XmATTACH_FORM,
                                                   XmNtopAttachment,    XmATTACH_FORM,
                                                   NULL);

   /* create OK button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 5, "OK")), XmFONTLIST_DEFAULT_TAG);
   deleteOkButton = XtVaCreateManagedWidget("deleteOkButton",
                                            xmPushButtonGadgetClass, deleteActionForm,
                                            XmNlabelString,      xs,
                                            XmNtopAttachment,    XmATTACH_WIDGET,
                                            XmNtopWidget,        deleteActionSeparator,
                                            XmNtopOffset,        10,
                                            XmNbottomAttachment, XmATTACH_FORM,
                                            XmNleftAttachment,   XmATTACH_POSITION,
                                            XmNleftPosition,     1,
                                            XmNrightAttachment,  XmATTACH_POSITION,
                                            XmNrightPosition,    3,
                                            NULL);
   XtAddCallback(deleteOkButton, XmNactivateCallback, deleteOkCB, context); 
   XmStringFree(xs);

   /* create Cancel button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 8, "Cancel")), XmFONTLIST_DEFAULT_TAG);
   deleteCancelButton = XtVaCreateManagedWidget("deleteCancelButton",
                                            xmPushButtonGadgetClass, deleteActionForm,
                                            XmNlabelString,      xs,
                                            XmNtopAttachment,    XmATTACH_WIDGET,
                                            XmNtopWidget,        deleteActionSeparator,
                                            XmNtopOffset,        10,
                                            XmNbottomAttachment, XmATTACH_FORM,
                                            XmNleftAttachment,   XmATTACH_POSITION,
                                            XmNleftPosition,     4,
                                            XmNrightAttachment,  XmATTACH_POSITION,
                                            XmNrightPosition,    6,
                                            NULL);
   XtAddCallback(deleteCancelButton, XmNactivateCallback, deleteCancelCB, context); 
   XmStringFree(xs);


   /* create Help button */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 9, "Help")), XmFONTLIST_DEFAULT_TAG);
   deleteHelpButton = XtVaCreateManagedWidget("deleteHelpButton",
                                              xmPushButtonGadgetClass, deleteActionForm,
                                              XmNlabelString,      xs,
                                              XmNtopAttachment,    XmATTACH_WIDGET,
                                              XmNtopWidget,        deleteActionSeparator,
                                              XmNtopOffset,        10,
                                              XmNbottomAttachment, XmATTACH_FORM,
                                              XmNleftAttachment,   XmATTACH_POSITION,
                                              XmNleftPosition,     7,
                                              XmNrightAttachment,  XmATTACH_POSITION,
                                              XmNrightPosition,    9,
                                              NULL);
/*   XtAddCallback(deleteHelpButton, XmNactivateCallback, , context); */
   XmStringFree(xs);

   /* define the default action, so the Enter (kActivate) key will do something */
   XtVaSetValues(deleteActionForm,
                 XmNdefaultButton, deleteCancelButton,
                 NULL);
   
   /* create a label with instructions for the user */
   xs = XmStringCreateLtoR ((GETMESSAGE(1, 52, "Select the Users or Groups\nyou wish to delete and press OK")), XmFONTLIST_DEFAULT_TAG);
   deleteInstructions = XtVaCreateManagedWidget("deleteInstructions",
                                                xmLabelGadgetClass, deleteForm,
                                                XmNlabelString,     xs,
                                                XmNalignment,       XmALIGNMENT_CENTER,
                                                XmNtopAttachment,   XmATTACH_FORM,
                                                XmNleftAttachment,  XmATTACH_FORM,
                                                XmNmarginWidth,     MARGIN,
                                                XmNmarginHeight,    MARGIN,
                                                NULL);
   XmStringFree(xs);

   /* put a separator after the instructions */
   deleteInstSeparator = XtVaCreateManagedWidget("deleteInstSeparator",
                                                   xmSeparatorGadgetClass, deleteForm,
                                                   XmNleftAttachment,   XmATTACH_FORM,
                                                   XmNrightAttachment,  XmATTACH_FORM,
                                                   XmNtopAttachment,    XmATTACH_WIDGET,
                                                   XmNtopWidget,        deleteInstructions,
                                                   NULL);

   /* create the list widget but with no entries ... these will be added whenever */
   /* the delete dialog is invoked */
   deleteList = XtVaCreateManagedWidget("deleteList",
                                        xmListWidgetClass, deleteForm,
                                        XmNselectionPolicy,  XmEXTENDED_SELECT,
                                        XmNtopAttachment,    XmATTACH_WIDGET,
                                        XmNtopOffset,        MARGIN,
                                        XmNtopWidget,        deleteInstSeparator,
                                        XmNleftAttachment,   XmATTACH_FORM,
                                        XmNleftOffset,       MARGIN,
                                        XmNbottomOffset,     MARGIN,
                                        XmNbottomAttachment, XmATTACH_WIDGET,
                                        XmNbottomWidget,     deleteActionForm,
                                        XmNbottomOffset,     MARGIN,
                                        XmNlistMarginWidth,  MARGIN,
                                        XmNlistMarginHeight, MARGIN,
                                        NULL);

   XtManageChild(deleteActionForm);


   /* copy delete Dialog elements to context structure */
   context->deleteDialogShell     = deleteDialogShell;
   context->deleteForm            = deleteForm;
   context->deleteActionForm      = deleteActionForm;
   context->deleteActionSeparator = deleteActionSeparator;
   context->deleteOkButton        = deleteOkButton;
   context->deleteCancelButton    = deleteCancelButton;
   context->deleteHelpButton      = deleteHelpButton;
   context->deleteInstructions    = deleteInstructions;
   context->deleteInstSeparator   = deleteInstSeparator;
   context->deleteList            = deleteList;

}  /* end createDeleteDialog */
