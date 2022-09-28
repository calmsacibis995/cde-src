/* $XConsortium: cmncbs.c /main/cde1_maint/2 1995/08/29 19:30:05 gtsang $ */
/*****************************************************************************/
/*                                                                           */
/*  cmncbs.c                                                                 */
/*                                                                           */
/*   Common callbacks                                                        */
/*                                                                           */
/*****************************************************************************/
#include <stdio.h>
#include <sys/signal.h>
#include <X11/cursorfont.h>

#include <Dt/Icon.h>
#include <Dt/Help.h>
#include <Dt/HelpDialog.h>

#include "dtcreate.h"
#include "OpenFile.h"
#include "cmncbs.h"
#include "cmnrtns.h"
#include "CreateActionAppShell.h"
#include "ca_aux.h"
#include "icon_selection_dialog.h"


/******************************************************************************/
/* Initialize global values.                                                  */
/******************************************************************************/
Boolean bRegisteredSignal = FALSE;
int     (*sigchildRoutine)();

/******************************************************************************/
/* activateCB_open_FindSet - Callback for the "Find Set" button on            */
/*                           CreateActionAppShell and AddFiletype windows.    */
/*                           This callback will pop up the Icon Selection     */
/*                           dialog box.                                      */
/*                                                                            */
/* INPUT:  Widget filesel - file selection box widget id                      */
/*         XtPointer cdata - client data                                      */
/*         XmFileSelectionBoxCallbackStruct *cbstruct - callback data         */
/*                                                                            */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void activateCB_open_FindSet (find_set_button, cdata, cbstruct)
                              Widget find_set_button;
                              XtPointer cdata;
                              XmFileSelectionBoxCallbackStruct *cbstruct;
#else /* _NO_PROTO */
void activateCB_open_FindSet (Widget find_set_button, XtPointer cdata,
                              XmFileSelectionBoxCallbackStruct *cbstruct)
#endif /* _NO_PROTO */
{
  Widget filter;
  Boolean use_bm = False;
  static char *use_bm_filter = "*.m.bm";
  static char *use_pm_filter = "*.m.pm";
  char *search_path;
  char *filter_field_title = NULL;
  int use_filter_field = False;
  char *file_filter;

  char **directories_list = GetIconSearchPathList();
  int numberOfTopButtons = 0;
  void *top_button_one_cb = NULL;
  void *top_button_two_cb = NULL;
  void *top_button_three_cb = NULL;
  char *top_button_one_label = NULL;
  char *top_button_two_label = NULL;
  char *top_button_three_label = NULL;
  int use_icon_name_field = True;
  int numberOfBottomButtons = 3;
  void *bottom_button_one_cb = (void *)load_icons;
  void *bottom_button_two_cb = NULL;
  void *bottom_button_three_cb = NULL;
  void *bottom_button_four_cb = NULL;
  char *bottom_button_one_label = NULL;   /* use default */
  char *bottom_button_two_label = NULL;   /* use default */
  char *bottom_button_three_label = NULL; /* use default */
  char *bottom_button_four_label = NULL;  /* use default */

  char name_field_title[MAXBUFSIZE];
  char container_title[MAXBUFSIZE];
  char directory_title[MAXBUFSIZE];
  char dialog_title[MAXBUFSIZE];

  strcpy(name_field_title, GETMESSAGE(11, 12, "Enter Icon Filename"));
  strcpy(container_title, GETMESSAGE(11, 11, "Icon Files"));
  strcpy(directory_title, GETMESSAGE(11, 30, "Icon Folders"));
  strcpy(dialog_title, GETMESSAGE(6, 18, "Find Set"));

#ifdef DEBUG
  printf("activate Find Set button; client_data = %d\n", cdata); /* debug */
#endif

  XtVaGetValues (COLOR_MONO_OPTION, XmNset, &use_bm, NULL);

  if (use_bm) file_filter = use_bm_filter;
  else file_filter = use_pm_filter;

#ifndef _ICONSELECTOR_DESTROY_ENABLED
  if (!IconSelector) {
#endif  /* _ICONSELECTOR_DESTROY_ENABLED */
     IconSelector = create_icon_selection_dialog ((swidget) CreateActionAppShell,
                                               (unsigned char *)dialog_title,
                                               (unsigned char *)filter_field_title,
                                               use_filter_field,
                                               (unsigned char *)file_filter,
                                               (unsigned char *)directory_title,
                                               (unsigned char **)directories_list,
                                               (unsigned char *)container_title,
                                               numberOfTopButtons,
                                               (unsigned char *)top_button_one_label,
                                               (void (*)())top_button_one_cb,
                                               (unsigned char *)top_button_two_label,
                                               (void (*)())top_button_two_cb,
                                               (unsigned char *)top_button_three_label,
                                               (void (*)())top_button_three_cb,
                                               use_icon_name_field,
                                               (unsigned char *)name_field_title,
                                               numberOfBottomButtons,
                                               (unsigned char *)bottom_button_one_label,
                                               (void (*)())bottom_button_one_cb,
                                               (unsigned char *)bottom_button_two_label,
                                               (void (*)())bottom_button_two_cb,
                                               (unsigned char *)bottom_button_three_label,
                                               (void (*)())bottom_button_three_cb,
                                               (unsigned char *)bottom_button_four_label,
                                               (void (*)())bottom_button_four_cb);
#ifndef _ICONSELECTOR_DESTROY_ENABLED
  }
#endif  /* _ICONSELECTOR_DESTROY_ENABLED */
  FreeIconSearchPathList(directories_list);
  XtVaSetValues (IconSelector, XmNuserData, cdata, NULL);
  UxPopupInterface (IconSelector, no_grab);
  XmProcessTraversal(ISD_SelectedIconTextField, XmTRAVERSE_CURRENT);
  return;
}

/******************************************************************************/
/*                                                                            */
/* createCB_IconGadget                                                        */
/*                                                                            */
/* INPUT:  Widget wid  - icon gadget windet id                                */
/*         Boolean bActionIcons - is this an action widget                    */
/*         enum icon_size_range - size of icon this widget represents         */
/*                                                                            */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
        void    createCB_IconGadget(wid, bActionIcons, IconSize)
        Widget               wid;
        Boolean              bActionIcons;
        enum icon_size_range IconSize;
#else /* _NO_PROTO */
        void    createCB_IconGadget(Widget wid,
                                    Boolean bActionIcons,
                                    enum icon_size_range IconSize)
#endif /* _NO_PROTO */

{
  SetIconData(wid,
              ((bActionIcons) ? ca_full_icon_default : af_full_icon_default),
              IconSize);

}

/******************************************************************************/
/* activateCB_edit_icon - brings up icon editor with selected icon            */
/*                                                                            */
/* INPUT:  Widget wid - widget id                                             */
/*         XtPointer cd - client data                                         */
/*         XmPushButtonCallbackStruct *cbs - callback data                    */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void activateCB_edit_icon (wid, client_data, cbs)
     Widget wid;
     XtPointer client_data;
     XmPushButtonCallbackStruct *cbs;
#else /* _NO_PROTO */
void activateCB_edit_icon (Widget wid, XtPointer client_data,
                           XmPushButtonCallbackStruct *cbs)
#endif /* _NO_PROTO */
{
  char    *pszIconToEdit;
  Boolean IsActionIcons;

  if (bIconEditorDisplayed) return;

  if ((int)client_data == CA_ACTION_ICONS) {
     IsActionIcons = True;
     widSelectedIcon = get_selected_action_icon();
     widEditSource = CreateActionAppShell;
  } else {
     IsActionIcons = False;
     widSelectedIcon = (Widget)get_selected_filetype_icon();
     widEditSource = AddFiletype;
  }
  XtVaGetValues (widSelectedIcon, XmNimageName, &pszIconToEdit, NULL);

  /***************************************************************/
  /* Call routine to send tooltalk message to start icon editor. */
  /***************************************************************/
  UxDoEditPixmap(widSelectedIcon, pszIconToEdit);

  return;
}

/******************************************************************************/
/*                                                                            */
/* closeCB_mainHelpDialog                                                     */
/*                                                                            */
/* INPUT:  Widget wid - widget id                                             */
/*         XtPointer cd - client data                                         */
/*         XtPointer cbs - callback data                                      */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void closeCB_mainHelpDialog(wid, client_data, cbs)
     Widget wid;
     XtPointer client_data;
     XtPointer *cbs;
#else /* _NO_PROTO */
void closeCB_mainHelpDialog(Widget wid, XtPointer client_data,
                           XtPointer *cbs)
#endif /* _NO_PROTO */
{
  XtDestroyWidget(wid);
}

/******************************************************************************/
/*                                                                            */
/* DisplayHelpDialog                                                          */
/*                                                                            */
/* INPUT:  Widget wid - widget id                                             */
/*         XtPointer cd - client data                                         */
/*         XtPointer cbs - callback data                                      */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void DisplayHelpDialog(wid, client_data, cbs)
     Widget wid;
     XtPointer client_data;
     XtPointer cbs;
#else /* _NO_PROTO */
void DisplayHelpDialog(Widget wid, XtPointer client_data, XtPointer cbs)
#endif /* _NO_PROTO */
{
#ifdef AIX_ILS
  static Widget  mainHelpDialog = NULL;
#else /* AIX_ILS */
  Widget  mainHelpDialog;
#endif /* AIX_ILS */
  Widget  parent;
  int     i;
  Arg     args[10];
  char    pszTitle[MAXBUFSIZE];
  char    pszTopic[MAXBUFSIZE];

  i = 0;

  /***********************************************************************/
  /* Get help dialog window title.                                       */
  /***********************************************************************/
  /*
  sprintf(pszTitle, "%s  -  %s", GETMESSAGE(3, 10, "Create Action"),
          GETMESSAGE(2, 53, "Help"));
  */
  sprintf(pszTitle, "%s  -  ", GETMESSAGE(3, 10, "Create Action"));
  strcat(pszTitle, GETMESSAGE(2, 53, "Help"));
  XtSetArg(args[i], XmNtitle, pszTitle); i++;

  /***********************************************************************/
  /* Now set the particular helpvolume and topic to view                 */
  /***********************************************************************/
  XtSetArg(args[i], DtNhelpType, DtHELP_TYPE_TOPIC); i++;
  XtSetArg(args[i], DtNhelpVolume, "CreatAct"); i++;
  XtSetArg(args[i], DtNlocationId, pszTopic); i++;

  switch ((int)client_data) {
     case HELP_OVERVIEW:
                         strcpy(pszTopic, "_hometopic");
                         break;
     case HELP_TASKS:
                         strcpy(pszTopic, "Tasks");
                         break;
     case HELP_REFERENCE:
                         strcpy(pszTopic, "Reference");
                         break;
     case HELP_USING:
                         strcpy(pszTopic, "_HOMETOPIC");
                         XtSetArg(args[i], DtNhelpVolume, "Help4Help"); i++;
                         break;
     case HELP_ABOUT:
                         strcpy(pszTopic, "_copyright");
                         break;
     case HELP_ONITEM:
                         strcpy(pszTopic, "CreateActionMainWindow");
                         break;
     case HELP_ADDFILETYPE:
                         strcpy(pszTopic, "AddDatatypeWindow");
                         break;
     case HELP_FILECHAR:
                         strcpy(pszTopic, "DatatypeCriteriaWindow");
                         break;
     case HELP_ICONSELECTOR:
                         strcpy(pszTopic, "FindIconSetDialog");
                         break;
     case HELP_OPENFILE:
                         strcpy(pszTopic, "openDialog");
                         break;
  }

  /***********************************************************************/
  /* Create the Help dialog.                                             */
  /***********************************************************************/
  /*  **** old method of getting parent ****
  XtVaGetValues(wid, XmNuserData, &parent, NULL);
  if (!parent) {
     parent = wid;
  }
  */
  parent = GetTrueToplevel(wid);
  TurnOnHourGlassAllWindows();
#ifdef AIX_ILS
  if ( mainHelpDialog == NULL ) {
      mainHelpDialog = DtCreateHelpDialog(parent, "mainHelpDialog", args, i);
      XtAddCallback(mainHelpDialog, DtNcloseCallback,
                (void (*)())closeCB_mainHelpDialog,
                (XtPointer)NULL);
      XtManageChild(mainHelpDialog);
  } else {
     XtSetValues( mainHelpDialog, args, i );
  }
#else /* AIX_ILS */
  mainHelpDialog = DtCreateHelpDialog(parent, "mainHelpDialog", args, i);
  XtAddCallback(mainHelpDialog, DtNcloseCallback,
                (void (*)())closeCB_mainHelpDialog,
                (XtPointer)NULL);
  XtManageChild(mainHelpDialog);
#endif /* AIX_ILS */
  TurnOffHourGlassAllWindows();
}
/******************************************************************************/
/*                                                                            */
/* helpCB_general                                                             */
/*                                                                            */
/* INPUT:  Widget wid - widget id                                             */
/*         XtPointer cd - client data                                         */
/*         XtPointer cbs - callback data                                      */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void    helpCB_general( UxWidget, UxClientData, UxCallbackArg )
               Widget          UxWidget;
               XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
void    helpCB_general(Widget UxWidget,
                       XtPointer UxClientData,
                       XtPointer UxCallbackArg)
#endif /* _NO_PROTO */

{
  DisplayHelpDialog(UxWidget, UxClientData, UxCallbackArg);
}
