/* $XConsortium: PopupMenu.c /main/cde1_maint/5 1995/10/11 18:11:37 montyb $ */
/*****************************************************************************
 *****************************************************************************
 **
 **   File:         PopupMenu.c
 **
 **   Project:      CDE
 **
 **   Description:  This file contains the functions for creating and managing
 **                 the main panel and subpanel popup menus.
 **
 **
 ****************************************************************************
 *****************************************************************************/


#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/RowColumn.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Dt/Control.h>
#include <Dt/ControlP.h>

#include <Dt/SharedProcs.h>

#include "DataBaseLoad.h"
#include "WmGlobal.h"
#include "WmFunction.h"
#include "WmResNames.h"

#define POPUP_CHILDREN 7

#ifdef _NO_PROTO

extern void ToggleDefaultControl ();
extern void SwitchRenameLabel ();
extern Boolean CheckControlTypeFile ();

#else  /*  _NO_PROTO  */

extern void ToggleDefaultControl (ControlData *, SubpanelData *, ControlData *);
extern void SwitchRenameLabel (Widget, BoxData *);
extern Boolean CheckControlTypeFile (ControlData *);

#endif  /*  _NO_PROTO  */






/************************************************************************
 *
 *  ActionCB
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
void
ActionCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else
void
ActionCB (Widget    w,
          XtPointer client_data,
          XtPointer call_data)

#endif /* _NO_PROTO */

{
   ControlData * control_data;
   int           control_type;
   DtControlCallbackStruct * callback = (DtControlCallbackStruct *) call_data;
   int indx = (int) client_data;
   Arg args[1];
   Boolean push_recall;
   PanelActionData * push_action;
   SubpanelData * subpanel_data;
   ControlData  * main_control_data;
   Boolean           unpost_subpanel = True;
   int i, j;


   XtSetArg (args[0], XmNuserData, &control_data);
   XtGetValues(w, args, 1);

   if (CheckControlTypeFile (control_data) == False)
      return;

   if (control_data->parent_type == SUBPANEL)
   {
      subpanel_data = (SubpanelData *) control_data->parent_data;
      main_control_data = subpanel_data->parent_control_data;

      /*  This control may have been toggled from the subpanel.  If  */
      /*  so, make sure the subpanel is not reposted.                */


      if (XtParent (w) != subpanel_data->form)
         unpost_subpanel = False;
   }
   else
   {
      /*  The control may be the main panel icon copy.  If so, find  */
      /*  the subpanel that contains it a set the subpanel data      */

      subpanel_data = NULL;

      for (i = 0; i < panel.box_data_count; i++)
      {
         for (j = 0; j < panel.box_data[i]->control_data_count; j++)
         {
            ControlData * box_control;

            box_control = panel.box_data[i]->control_data[j];

            if (box_control->subpanel_data != NULL &&
                box_control->subpanel_data->main_panel_icon_copy == w)
            {
               subpanel_data = box_control->subpanel_data;
               main_control_data = subpanel_data->parent_control_data;
               break;
            }
         }
         if (subpanel_data != NULL)
            break;
      }
   }


    push_recall = (Boolean) (control_data->element_values[CONTROL_PUSH_RECALL].parsed_value);
      push_action = (PanelActionData *) (control_data->element_values[CONTROL_PUSH_ACTION].parsed_value);
   control_type = (int) (control_data->element_values[CONTROL_TYPE].parsed_value);		  

 if (push_recall == False)
  {
   if (control_type == CONTROL_FILE && !control_data->is_action)
   {
      DtActionArg * aap;

      aap = (DtActionArg *) XtCalloc(1,sizeof(DtActionArg));

      aap->argClass = DtACTION_FILE;

      aap->u.file.name = (char *)
               control_data->element_values[CONTROL_FILE_NAME].parsed_value;

      DtActionInvoke (control_data->icon,
	              control_data->actions[indx]->action_name,
                      aap, 1, NULL, NULL, NULL, 1, NULL, NULL);

      XtFree ((char *) aap);
   }
   else
   {
      if (strcmp (push_action->action_name, "FPOnItemHelp") == 0)
      {
         WmDtHelpMode();
         unpost_subpanel = False;
      }
      else
      {
         DtActionInvoke (panel.shell, push_action->action_name,
                                        NULL, 0, NULL, NULL, NULL,
                                        1, NULL, NULL);
      }
   }

  }
 else 
  {
    WmFunction wm_func;
    int        function_index;
    String     func_arg = NULL;
    String     s1, s1save;
    char     * client_name;
    char     * client_title;
    WmGlobalData    * wm_global_data = (WmGlobalData *) panel.global_data;
    ClientData      * wm_client_data;
    WmPushRecallArg   push_argument;
                  /*  Parse out the function and arguments used by the  */
                  /*  window manager to invoke the client.              */

                  s1save = s1 = XtNewString ("f.action");

                  function_index =
                    ParseWmFunction ((unsigned char **) &s1, CRS_ANY, &wm_func);
                  XtFree (s1save);


                  client_name =
                     (char *) control_data->element_values[CONTROL_CLIENT_NAME].
parsed_value;

                  if (client_name == NULL)
                     client_name =
                        (char *) control_data->element_values[CONTROL_LABEL].parsed_value;

                  client_title =
                     (char *) control_data->element_values[CONTROL_LABEL].parsed_value;

                  if (control_type == CONTROL_FILE && ! control_data->is_action)
                  {
                     char * file_name;

                     file_name = (char *) control_data->element_values[CONTROL_FILE_NAME].parsed_value;

                     s1 = XtMalloc (strlen (push_action->action_name) + strlen (file_name) + 2);
                     strcpy (s1, push_action->action_name);
                     strcat (s1, " ");
                     strcat (s1, file_name);
                  }
                  else
                     s1 = XtNewString (push_action->action_name);

                  ParseWmFunctionArg ((unsigned char **) &s1, function_index,
                                      wm_func, (void **) &func_arg,
                                      client_name, client_name);


                  /*  Set the push argument to the parsed data  */

                  push_argument.ixReg = PushRecallGetData (client_name);
                  push_argument.wmFunc = wm_func;
                  push_argument.pArgs = func_arg;


                  /*  Get the window manager client data for the panel and  */
                  /*  call the push recall function to get the client run.  */

                  if (subpanel_data != NULL)
                     XFindContext (wm_global_data->display,
                                   XtWindow (subpanel_data->shell),
                                   wm_global_data->windowContextType,
                                   (XtPointer) &wm_client_data);
                  else
                     XFindContext (wm_global_data->display,
                                   XtWindow (panel.shell),
                                   wm_global_data->windowContextType,
                                   (XtPointer) &wm_client_data);

                  F_Push_Recall ((String) &push_argument,
                                 wm_client_data, callback->event);

                  FreeFunctionArg (wm_func, func_arg);
                  XtFree (s1);
  }
}


/************************************************************************
 *
 *  DeleteWorkspaceCB
 *	The callback function is called off of a menu pick to delete
 *	a specific workspace.  The function identifies the index of the
 *	workspace and calls the workspace manager API to perform the delete.
 *	All frontpanel UI processing is performed through a notification 
 *      callback issued by the workspace manager after a successful delete.
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
DeleteWorkspaceCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else
void
DeleteWorkspaceCB (Widget    w,
                   XtPointer client_data,
	           XtPointer call_data)

#endif /* _NO_PROTO */

{
   int delete_workspace = (int) client_data;
   SwitchData * switch_data;
   int i;
   

   /*  Look up the switch data to get the atom name of the workspace  */
   /*  to delete.                                                     */
   
   for (i = 0; i < panel.box_data_count; i++)
   {
      if (panel.box_data[i]->switch_data != NULL)
      {
         switch_data = panel.box_data[i]->switch_data;
         break;
      }
   }
   
   _DtWsmDeleteWorkspace (panel.shell, 
                          switch_data->atom_names[delete_workspace]);
}




/************************************************************************
 *
 *  RenameWorkspaceCB
 *	The callback function is called off of a menu pick to rename
 *	a specific workspace.  The function identifies the index of the
 *	workspace and calls SwitchRenameLabel() function to change the switch.
 *	button to a text widget for typing the new workspace name and setting.
 *	up the callback to accept the new name upon activation.
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
RenameWorkspaceCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else
void
RenameWorkspaceCB (Widget    w,
                   XtPointer client_data,
	           XtPointer call_data)

#endif /* _NO_PROTO */

{
   int rename_ws = (int) client_data;
   SwitchData * switch_data;
   int i;
   

   /*  Look up the switch data to get the atom name of the workspace  */
   /*  to delete.                                                     */
   
   for (i = 0; i < panel.box_data_count; i++)
   {
      if (panel.box_data[i]->switch_data != NULL)
      {
         switch_data = panel.box_data[i]->switch_data;
         break;
      }
   }
   
   SwitchRenameLabel (switch_data->buttons[rename_ws], panel.box_data[i]);
}





/************************************************************************
 *
 *  AddWorkspaceCB
 *	The callback function is called off of a menu pick.  It generates
 *	a new workspace name and calls the workspace manager API to 
 *	create the new workspace.  All frontpanel UI processing is 
 *	performed through a notification callback issued by the workspace
 *	manager after a successful add.
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
void
AddWorkspaceCB (w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;

#else
void
AddWorkspaceCB (Widget    w,
                XtPointer client_data,
                XtPointer call_data)

#endif /* _NO_PROTO */

{
   SwitchData * switch_data;
   char * switch_name;
   char * temp_name;

   int append;
   int i;
   

   /*  Get the beginning part of the new name  */
   
   temp_name = FPGETMESSAGE (82, 35, "New");
   switch_name = XtMalloc (strlen (temp_name) + 5);
   strcpy (switch_name, temp_name);


   /*  Look up the switch data to get the atom name of the workspace  */
   /*  to delete.                                                     */
   
   for (i = 0; i < panel.box_data_count; i++)
   {
      if (panel.box_data[i]->switch_data != NULL)
      {
         switch_data = panel.box_data[i]->switch_data;
         break;
      }
   }
   

   /*  Generate a new title for the workspace.  It should be unique  */
   /*  from the existing title.                                      */

   append = 0;

   while (1)
   {
      Boolean good_name;
      
      good_name = True;
      append++;      

      for (i = 0; i < switch_data->switch_count; i++)
      {
         if (strcmp (switch_name, switch_data->switch_names[i]) == 0)
         {
            good_name = False;
            sprintf (switch_name, "%s_%d", temp_name, append);
	    continue;
         }
      }

      if (good_name)
      {
         _DtWsmCreateWorkspace (panel.shell, switch_name);
         break;
      }
   }

   XtFree(switch_name);
}


/************************************************************************
 *
 *  DeleteCancelCB
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
DeleteCancelCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
static void
DeleteCancelCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   XtUnmanageChild((Widget)client_data);
   XmUpdateDisplay((Widget)client_data);
   XtDestroyWidget((Widget)client_data);
}




/************************************************************************
 *
 *  DeleteControlOkCB
 *	Called from the Ok button on the confimation dialog when a 
 *	control deletion is seleted from the popup menu.
 *
 ************************************************************************/

#ifdef _NO_PROTO

static void
DeleteControlOkCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else

static void
DeleteControlOkCB (Widget    w,
                   XtPointer client_data,
                   XtPointer call_data)

#endif /* _NO_PROTO */

{
   ControlData  * control_data;
   SubpanelData * subpanel_data;
   Arg args[1];


   /*  Extract the control data from the user data field of the  */
   /*  widget pointed to by client data.                         */
   
   XtSetArg (args[0], XmNuserData, &control_data);
   XtGetValues((Widget) client_data, args, 1);


   /*  Get the subpanel data for the control.  Check to see if the  */
   /*  control is a real subpanel control or the main panel copy.   */
   
   if (control_data->parent_type == SUBPANEL)
      subpanel_data = (SubpanelData *) control_data->parent_data;
   else
      subpanel_data = (SubpanelData *) control_data->subpanel_data;


   /*  Get rid of the dialog and call the funtion to delete the control  */

   XtUnmanageChild ((Widget) client_data);
   XmUpdateDisplay ((Widget) client_data);

   DeleteSubpanelControl (subpanel_data, control_data);

   XtDestroyWidget ((Widget) client_data);
}




/************************************************************************
 *
 *  DeleteSubpanelOkCB
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
DeleteSubpanelOkCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
static void
DeleteSubpanelOkCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   ControlData * control_data;
   Arg args[1];

   XtSetArg (args[0], XmNuserData, &control_data);
   XtGetValues((Widget)client_data, args, 1);

   XtUnmanageChild((Widget)client_data);
   XmUpdateDisplay((Widget)client_data);

   DeleteSubpanel(control_data);

   XtDestroyWidget((Widget)client_data);
}


/************************************************************************
 *
 *  DeleteControlCB
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
DeleteControlCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else
void
DeleteControlCB (Widget    w,
                 XtPointer client_data,
                 XtPointer call_data)

#endif /* _NO_PROTO */

{
    ControlData * control_data;
#ifndef	IBM_163763
    SubpanelData *subpanel_data;
#endif
    Widget dialog;
    String title, del_ctrl, ctrl_name, ctrl_label, del_msg, message;
    int indx = (int) client_data;
    Arg args[2];

    XtSetArg (args[0], XmNuserData, &control_data);
    XtGetValues (w, args, 1);

    del_ctrl = FPGETMESSAGE(82, 31, "Delete Control:");
    del_ctrl = XtNewString (del_ctrl);

    ctrl_label = (char *)
                control_data->element_values[CONTROL_LABEL].parsed_value;
    ctrl_name = (char *)
                   control_data->element_values[CONTROL_NAME].parsed_value;

    if (ctrl_label == NULL)
       ctrl_label = ctrl_name;

    if ((Boolean)control_data->element_values[CONTROL_LOCKED].parsed_value)
    {
       title = FPGETMESSAGE(82,1, "Workspace Manager - Delete Locked Control Error");
       title = XtNewString (title);

       del_msg = FPGETMESSAGE(82,5, "This control cannot be deleted because it is locked.");
       del_msg = XtNewString (del_msg);

       message = XtMalloc (sizeof(char) *
                           (strlen(del_ctrl) + strlen(ctrl_label) +
                            strlen(del_msg) + 4));

       sprintf(message, "%s %s\n\n%s", del_ctrl, ctrl_label, del_msg);

       _DtMessage (XtParent(w), title, message, NULL, NULL);

       XtFree (title);
       XtFree (del_ctrl);
       XtFree (del_msg);
       XtFree (message);
    }
    else
    {
       title = FPGETMESSAGE(82,2, "Workspace Manager - Delete Control");
       title = XtNewString (title);

       if (SessionFileNameLookup (ctrl_name, CONTROL, (char *) control_data->element_values[CONTROL_CONTAINER_NAME].parsed_value, SUBPANEL) == NULL)
          del_msg = FPGETMESSAGE(82,32, "If you delete this control, you cannot restore it using\nthe \"Install Icon\" drop zone.\n\nYou can use \"RestorePanel\" in the Application Manager\nto put back this control at a later time.\n\nAre you sure you want to delete this control?");
       else
          del_msg = FPGETMESSAGE(82,6, "Are you sure you want to delete this control?");

       del_msg = XtNewString (del_msg);

       message = XtMalloc (sizeof(char) *
                           (strlen(del_ctrl) + strlen(ctrl_label) +
                            strlen(del_msg) + 4));

       sprintf(message, "%s %s\n\n%s", del_ctrl, ctrl_label, del_msg);

#ifndef	IBM_163763
       /*  Get the subpanel data for the control.  Check to see if the  */
       /*  control is a real subpanel control or the main panel copy.   */

       if (control_data->parent_type == SUBPANEL)
          subpanel_data = (SubpanelData *) control_data->parent_data;
       else
          subpanel_data = (SubpanelData *) control_data->subpanel_data;

       dialog = _DtMessageDialog(subpanel_data->shell,title,message,NULL,TRUE,
#else
       dialog = _DtMessageDialog(panel.shell, title, message, NULL, TRUE,
#endif
                           DeleteCancelCB, DeleteControlOkCB, NULL, NULL, False,
                           QUESTION_DIALOG);

       XtSetArg (args[0], XmNuserData, control_data);
       XtSetArg (args[1], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
       XtSetValues (dialog, args, 2);

       XtFree (title);
       XtFree (del_ctrl);
       XtFree (del_msg);
       XtFree (message);
    }

}




/************************************************************************
 *
 *  ToggleControlCB
 *      This callback initiates a subpanel control being toggled into
 *      the main panel.
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
ToggleControlCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else
void
ToggleControlCB (Widget    w,
                 XtPointer client_data,
                 XtPointer call_data)

#endif /* _NO_PROTO */

{
    SubpanelData * subpanel_data = (SubpanelData *) client_data;
    ControlData * control_data;
    int indx = (int) client_data;
    Arg args[1];

    XtSetArg (args[0], XmNuserData, &control_data);
    XtGetValues(w, args, 1);

    ToggleDefaultControl (subpanel_data->parent_control_data,
                          subpanel_data, control_data);
}




/************************************************************************
 *
 *  AddSubpanelCB
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
AddSubpanelCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else
void
AddSubpanelCB (Widget w,
               XtPointer client_data,
               XtPointer call_data)

#endif /* _NO_PROTO */

{
    ControlData * control_data;
    int indx = (int) client_data;
    Arg args[1];

    XtSetArg (args[0], XmNuserData, &control_data);
    XtGetValues(w, args, 1);

    AddSubpanel(control_data);
}




/************************************************************************
 *
 *  DeleteSubpanelCB
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
void
DeleteSubpanelCB (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else
void
DeleteSubpanelCB (Widget    w,
                  XtPointer client_data,
                  XtPointer call_data)

#endif /* _NO_PROTO */

{
    ControlData * control_data;
    SubpanelData * subpanel_data;
    Widget dialog;
    String title, message, del_spanel, spanel_name, del_msg;
    Arg args[2];

    XtSetArg (args[0], XmNuserData, &control_data);
    XtGetValues(w, args, 1);

    subpanel_data = (SubpanelData *) control_data->subpanel_data;
    if (!subpanel_data)
	subpanel_data = (SubpanelData *) control_data->parent_data;

    del_spanel = FPGETMESSAGE(82, 33, "Delete Subpanel:");
    del_spanel = XtNewString (del_spanel);

    spanel_name = (char *)
                  subpanel_data->element_values[SUBPANEL_NAME].parsed_value;

    if ((Boolean)subpanel_data->element_values[SUBPANEL_LOCKED].parsed_value)
    {
       title = FPGETMESSAGE(82,3, "Workspace Manager - Delete Locked Subpanel Error");
       title = XtNewString (title);

       del_msg = FPGETMESSAGE(82,7, "This subpanel cannot be deleted because it is locked.");
       del_msg = XtNewString (del_msg);

       message = XtMalloc (sizeof(char) *
                           (strlen(del_spanel) + strlen(spanel_name) +
                            strlen(del_msg) + 4));

       sprintf(message, "%s %s\n\n%s", del_spanel, spanel_name, del_msg);

       _DtMessage (XtParent(w), title, message, NULL, NULL);

       XtFree (title);
       XtFree (del_spanel);
       XtFree (del_msg);
       XtFree (message);
    }
    else
    {
       title = FPGETMESSAGE(82,4, "Workspace Manager - Delete Subpanel");
       title = XtNewString (title);

       if (subpanel_data->control_data_count > 0)
       {
         if (SessionFileNameLookup (spanel_name, SUBPANEL, (char *)
            subpanel_data->element_values[SUBPANEL_CONTAINER_NAME].parsed_value,
            CONTROL) == NULL)
             del_msg = FPGETMESSAGE(82,34,
"If you delete this subpanel, you cannot restore all\nof the controls contained in the subpanel using the\n\"Install Icon\" drop zone.\n\nYou can use \"RestorePanel\" in the Application Manager\nto put back this subpanel and its controls at a later time.\n\nAre you sure you want to delete this subpanel?");
         else
             del_msg = FPGETMESSAGE(82,8, "This subpanel contains more than one control.\n\nAre you sure you want to delete this subpanel?");
       }
       else
          del_msg = FPGETMESSAGE(82,9, "Are you sure you want to delete this subpanel?");

       del_msg = XtNewString (del_msg);

       message = XtMalloc (sizeof(char) *
                           (strlen(del_spanel) + strlen(spanel_name) +
                            strlen(del_msg) + 4));

       sprintf(message, "%s %s\n\n%s", del_spanel, spanel_name, del_msg);

#ifndef	IBM_163763
       dialog = _DtMessageDialog(subpanel_data->shell,title,message,NULL,TRUE,
#else
       dialog = _DtMessageDialog(panel.shell, title, message, NULL, TRUE,
#endif
                          DeleteCancelCB, DeleteSubpanelOkCB, NULL, NULL, False,
                          QUESTION_DIALOG);

       XtSetArg (args[0], XmNuserData, control_data);
       XtSetArg (args[1], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
       XtSetValues(dialog, args, 2);

       XtFree (title);
       XtFree (del_spanel);
       XtFree (del_msg);
       XtFree (message);
    }
}





/************************************************************************
 *
 *  PostWorkspacePopupMenu
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
void
PostWorkspacePopupMenu (w, client_data, event)
Widget      w;
XtPointer   client_data;
XEvent    * event;
#else

void
PostWorkspacePopupMenu (Widget      w,
                        XtPointer   client_data,
                        XEvent    * event)

#endif /* _NO_PROTO */

{
   int i;
   BoxData * box_data;
   SwitchData * switch_data = NULL;
   XButtonEvent * b_event = (XButtonEvent *) event;

   /* only use BMENU press events */

   if (b_event->button != wmGD.bMenuButton) return;

   for (i = 0; i < panel.box_data_count; i++)
   {
      box_data = panel.box_data[i];

      if (box_data->switch_data != NULL)
         switch_data = box_data->switch_data;
   }

   if (switch_data != NULL)
      XtManageChild(switch_data->popup_data->popup);
}




/************************************************************************
 *
 *  PostPopupMenu
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
void
PostPopupMenu (w, client_data, event)
Widget      w;
XtPointer   client_data;
XEvent    * event;

#else
void
PostPopupMenu (Widget      w,
               XtPointer   client_data,
               XEvent    * event)

#endif /* _NO_PROTO */

{
   XButtonEvent * b_event = (XButtonEvent *) event;

   /* only use BMENU press events */

   if (b_event->button != wmGD.bMenuButton) return;

   XtManageChild(panel.popup_data->popup);
}




/************************************************************************
 *
 *  GetBoxDataFromForm
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
static void
GetBoxDataFromForm (form, box_data_rtn)
Widget     form;
BoxData ** box_data_rtn;

#else
static void
GetBoxDataFromForm (Widget     form,
                    BoxData ** box_data_rtn)

#endif /* _NO_PROTO */

{
   BoxData * box_data;
   int i;

   for (i = 0; i < panel.box_data_count; i++)
   {
      box_data = panel.box_data[i];

      if (box_data == *box_data_rtn) continue;

      if (box_data->left_control_form == form ||
         box_data->right_control_form == form)
      {
         *box_data_rtn = box_data;
         break;
      }
   }
}




/************************************************************************
 *
 *  GetSubpanelDataFromForm
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
static void
GetSubpanelDataFromForm (form, subpanel_data_rtn)
Widget          form;
SubpanelData ** subpanel_data_rtn;

#else
static void
GetSubpanelDataFromForm (Widget          form,
                         SubpanelData ** subpanel_data_rtn)

#endif /* _NO_PROTO */

{
   BoxData * box_data;
   ControlData * control_data;
   SubpanelData * subpanel_data;
   int i, j;

   for (i = 0; i < panel.box_data_count; i++)
   {
      box_data = panel.box_data[i];

      for (j = 0; j < box_data->control_data_count; j++)
      {
         control_data = box_data->control_data[j];

         if (control_data->subpanel_data != NULL)
         {
            subpanel_data = control_data->subpanel_data;

            if (subpanel_data == *subpanel_data_rtn) continue;

            if (subpanel_data->form == form)
            {
               *subpanel_data_rtn = subpanel_data;
               break;
            }
         }
      }
   }
}





/************************************************************************
 *
 *  KeyMenuPosition
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
static void
KeyMenuPosition (popup, focus_widget)
Widget popup;
Widget focus_widget;

#else
static void
KeyMenuPosition (Widget popup, Widget focus_widget)

#endif /* _NO_PROTO */

{
   XButtonPressedEvent bevent;  /* synthetic button press event */
   Widget parent;
   Position root_x, root_y;
   Position x, y;
   Arg args[2];


   /* get the gadget's x and y */

   XtSetArg (args[0], XmNx, &x);
   XtSetArg (args[1], XmNy, &y);
   XtGetValues (focus_widget, args, 2);


   /* get the focus widget's parent since the focus widget is a gadget */

   parent = XtParent (focus_widget);


   /* get the root coordinates of the gadget */

   XtTranslateCoords (parent, x, y, &root_x, &root_y);


   /*  XmMenuPosition() only uses the root_x and root_y  */
   /*  of the event for positioning                      */

   bevent.x_root = root_x;
   bevent.y_root = root_y;


   /* set the menu positioning */

   XmMenuPosition (popup, &bevent);
}





/************************************************************************
 *
 *  GetMainControlData
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
static ControlData *
GetMainControlData (box_data, focus_widget)
BoxData * box_data;
Widget    focus_widget;

#else
static ControlData *
GetMainControlData (BoxData * box_data,
                    Widget    focus_widget)

#endif /* _NO_PROTO */

{
   int i;
   ControlData * control_data;
   Boolean found = False;

   for (i = 0; i < box_data->control_data_count; i++)
   {
      control_data = box_data->control_data[i];

      if (control_data->icon == focus_widget)
      {
         found = True;
         break;
      }
   }

   if (!found) return NULL;

   return (control_data);
}




/************************************************************************
 *
 *  GetSubpanelControlData
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
static ControlData *
GetSubpanelControlData (subpanel_data, focus_widget)
SubpanelData * subpanel_data;
Widget         focus_widget;

#else
static ControlData *
GetSubpanelControlData (SubpanelData * subpanel_data,
	                Widget         focus_widget)

#endif /* _NO_PROTO */

{
   int i;
   ControlData * control_data;
   Boolean found = False;

   if ( subpanel_data->dropzone == focus_widget )
      return NULL;

   if (subpanel_data->main_panel_icon_copy == focus_widget ||
       subpanel_data->parent_control_data->indicator == focus_widget)
   {
      return (subpanel_data->parent_control_data);
   }


   for (i = 0; i < subpanel_data->control_data_count; i++)
   {
      control_data = subpanel_data->control_data[i];
      if (control_data->icon == focus_widget ||
          control_data->indicator == focus_widget)
      {
         found = True;
         break;
      }
   }

   if (!found) return NULL;

   return (control_data);
}




/************************************************************************
 *
 *  SetupActionMenuItems
 *	Set up the action list for display in the popup menu for a 
 *	control.  This may require creating additional menu items is
 *	the action list for this control is greater that previously
 *	encountered or it may require umanaging menu item is there are
 *	less actions.
 *
 *	Inputs:
 *	   w: The parent of the menu items.
 *	   control_data: The control the menu is being set up for.
 *	   action_item: A pointer to the array of menu items for the actions.
 *	   action_count: A pointer to the total number of action items.
 *
 ************************************************************************/

#ifdef _NO_PROTO

static void
SetupActionMenuItems (w, control_data, popup_data, action_item, action_count)
Widget        w;
ControlData * control_data;
PopupData * popup_data;
Widget      ** action_item;
int         * action_count;

#else

static void
SetupActionMenuItems (Widget        w,
                      ControlData * control_data,
		      PopupData  * popup_data,
                      Widget      ** action_item,
                      int         * action_count)

#endif /* _NO_PROTO */

{
   XmString label_string;
   int i;
   int num_actions = 0;
   Arg args[5];
   int new_actions = 0;


   /*  Count the action list for the control in order to adjust  */
   /*  the number of action menu items within the menu.          */

   if (control_data != NULL)
      while (control_data->actions[num_actions] != NULL)
         num_actions++;

   /* This is a special case when there are no action items to create.
    * For e.g. the clock icon has no actions specified.
    */
   if (num_actions == 0)
    {
     XtUnmanageChild(popup_data->separator1);
     if (*action_count)
       for(i=0;i< (*action_count);i++)
         XtDestroyWidget((*action_item)[i]);
     *action_count = 0; 
     return;
    }

/* If we have less number of actions compared to last time 
   destroy those actions, so they don't show up in our list.
   Unmanaging would be better here, but when we need more actions
   we create new ones, rather than managing the unused old and then
   checking if we still need more action buttons and only then creating
   new actions. This logic is more complicated so we use create and
   destroy approach.
   Note:!!! we are not freeing the widget pointers from the action_item
   list when we destroy the widget. That gets taken care of when we do
   XtRealloc next time.
*/
  if(*action_count)
   {
     new_actions = num_actions - (*action_count);
     if (new_actions < 0)
      {
       new_actions = -1 * new_actions;
       for (i=num_actions; i<= new_actions; i++)
  	  XtDestroyWidget((*action_item)[i]);
      }
   }

  /* We have more action items then the last time, so create push buttons
   * for these new actions and put their pointers in action_item list.
   */
  if (num_actions > (*action_count) )
   {
      *action_item = (Widget *) XtRealloc((char *) *action_item,
					 sizeof(Widget* ) * num_actions);
      for (i = (*action_count); i < num_actions ; i++)
      {    
	  (*action_item)[i] = 
	     XmCreatePushButtonGadget (w, "action_button", args, 0);

          XtManageChild ((*action_item)[i]);
	  XtAddCallback ((*action_item)[i], XmNactivateCallback,
                         ActionCB, (XtPointer)i);
      }
   }
  /* update action_count for the next time we come here */
  (*action_count) = num_actions;

  /* Set the labels on the action items, and appropriate control data.
   * The logic used in this func is create new push buttons when there are more
   * actions items then the last time, and destroy push buttons if there
   * are less number of action items than last time. Do not create or destroy
   * pushbuttons if there are same number of action items compared to last
   * time. Once appropriate number of push buttons are there, then we worry
   * about the correct label and control data to put on them.
   */
  for (i = 0; i < num_actions; i++)
   {
      if (control_data->actions[i]->action_label != NULL)
         label_string = 
               XmStringCreateLocalized (control_data->actions[i]->action_label);
      else
         label_string = 
               XmStringCreateLocalized (control_data->actions[i]->action_name);

     if((*action_item)[i])
      XtVaSetValues ((*action_item)[i], XmNlabelString, label_string,
				     XmNuserData, control_data,
		     NULL);

      XmStringFree (label_string);
   }

   if (num_actions > 0)
      XtManageChild(popup_data->separator1);
   else
      XtUnmanageChild(popup_data->separator1);

}

/************************************************************************
 *
 *  UpdateMainControlMenu
 *	Set up the label and the add/delete subpanel menu item for the
 *	popup menu.
 *
 *	Inputs:
 *	   control_data: The control overwhich the menu is being posted.
 *
 ************************************************************************/
 
#ifdef _NO_PROTO

static void
UpdateMainControlMenu (control_data)
ControlData * control_data;

#else

static void
UpdateMainControlMenu (ControlData * control_data)

#endif /* _NO_PROTO */

{
   XmString   label_string;
   char     * mnemonic;
   char     * control_label;

   int control_type;
   Arg args[5];

   ElementValue * old_element_values;

   /*  Save element values of control then load values of default control */

   if (control_data->subpanel_data != NULL && 
	control_data != control_data->subpanel_data->default_control)
   {
	old_element_values = control_data->element_values;
	control_data->element_values = 
		control_data->subpanel_data->default_control->element_values;
   }

   /*  Set the title of the menu  */
   
   control_label =
	 (char *) control_data->element_values[CONTROL_LABEL].parsed_value;

   if (control_label == NULL)
      control_label = 
	       (char *) control_data->element_values[CONTROL_NAME].parsed_value;

   label_string = XmStringCreateLocalized (control_label);
   XtSetArg (args[0], XmNlabelString, label_string);
   XtSetValues (panel.popup_data->popup_title, args, 1);
   XmStringFree (label_string);


   /*  If the control is blank, remove the add/delete subpanel item.    */
   /*  Otherwise, if there is a subpanel, activate the delete subpanel  */
   /*  item.  Else activate the add subpanel item.                      */

   control_type = (int) control_data->element_values[CONTROL_TYPE].parsed_value;

   if (control_type != CONTROL_BLANK)
   {
      XtManageChild (panel.popup_data->modify_subpanel_item);

      if (control_data->subpanel_data == NULL)
      {
         if (panel.popup_data->subpanel_add_state != True)
         {
            label_string = 
	       XmStringCreateLocalized ((FPGETMESSAGE(82, 10, "Add Subpanel")));
            XtSetArg (args[0], XmNlabelString, label_string);
            mnemonic = ((char *)FPGETMESSAGE(82, 11, "A"));
            XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
            XtSetArg (args[2], XmNuserData, control_data);
            XtSetValues (panel.popup_data->modify_subpanel_item, args, 3);

            XmStringFree (label_string);

            XtRemoveCallback (panel.popup_data->modify_subpanel_item,
                              XmNactivateCallback, DeleteSubpanelCB, NULL);
            XtAddCallback (panel.popup_data->modify_subpanel_item,
                           XmNactivateCallback, AddSubpanelCB, NULL);

            panel.popup_data->subpanel_add_state = True;
         }
         else
         {
            XtSetArg (args[0], XmNuserData, control_data);
            XtSetValues (panel.popup_data->modify_subpanel_item, args, 1);
         }
      }
      else
      {
         if (panel.popup_data->subpanel_add_state == True)
         {
            label_string = 
	      XmStringCreateLocalized ((FPGETMESSAGE(82, 12, "Delete Subpanel")));
            XtSetArg (args[0], XmNlabelString, label_string);
            mnemonic = ((char *)FPGETMESSAGE(82, 13, "D"));
            XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
            XtSetArg (args[2], XmNuserData, control_data);
            XtSetValues (panel.popup_data->modify_subpanel_item, args, 3);

            XmStringFree (label_string);

            XtRemoveCallback (panel.popup_data->modify_subpanel_item,
                              XmNactivateCallback, AddSubpanelCB, NULL);
            XtAddCallback (panel.popup_data->modify_subpanel_item,
                           XmNactivateCallback, DeleteSubpanelCB, NULL);

            panel.popup_data->subpanel_add_state = False;
         }
         else
         {
            XtSetArg (args[0], XmNuserData, control_data);
            XtSetValues (panel.popup_data->modify_subpanel_item, args, 1);
         }
      }
   }
   else
   {
      XtUnmanageChild (panel.popup_data->modify_subpanel_item);
   }

   XtRemoveAllCallbacks (panel.popup_data->help_item, XmNactivateCallback);

   XtAddCallback(panel.popup_data->help_item, XmNactivateCallback,
	         ControlTopicHelpCB, (XtPointer)control_data);

   XtUnmanageChild (panel.popup_data->deinstall_item);
   XtUnmanageChild (panel.popup_data->toggle_item);

   /* Restore element values to control. */

   if (control_data->subpanel_data != NULL && 
	control_data != control_data->subpanel_data->default_control)
   {
	control_data->element_values = old_element_values;
   }
}





/************************************************************************
 *
 *  UpdateSubpanelControlMenu
 *	Set the elements of the popup menu for a subpanel control
 *	item to its proper values before it is made visible.
 *
 *	Inputs: 
 *	  subpanel_data:  The subpanel in which the menu is to be
 *                        posted.
 *
 *        control_data:  The control that the menu is being brought up for.
 *
 ************************************************************************/
 
#ifdef _NO_PROTO

static void
UpdateSubpanelControlMenu (subpanel_data, control_data)
SubpanelData * subpanel_data;
ControlData  * control_data;

#else

static void
UpdateSubpanelControlMenu (SubpanelData * subpanel_data,
			   ControlData  * control_data)

#endif /* _NO_PROTO */

{
   XmString label_string;
   Arg args[5];
   char     * control_label = NULL;

   /*  Set the title of the menu  */
   
   if (control_data)
   {
      control_label =
	 (char *) control_data->element_values[CONTROL_LABEL].parsed_value;

      if (control_label == NULL)
         control_label = 
	       (char *) control_data->element_values[CONTROL_NAME].parsed_value;
   } 
   else
   {
      control_label = FPGETMESSAGE(82, 36, "Install Icon");
   }

   label_string = XmStringCreateLocalized (control_label);
   XtSetArg (args[0], XmNlabelString, label_string);
   XtSetValues (panel.popup_data->popup_title, args, 1);
   XmStringFree (label_string);


   /*  Set the sensitivity of the control deinstall and toggle menu item  */
   
   if (control_data == NULL)
   {
      XtSetSensitive (panel.popup_data->deinstall_item, False);
      XtSetSensitive (panel.popup_data->toggle_item, False);

      XtRemoveAllCallbacks (panel.popup_data->help_item, XmNactivateCallback);

      XtAddCallback(panel.popup_data->help_item, XmNactivateCallback,
		    GeneralTopicHelpCB, (XtPointer)INSTALL_ZONE);
   }
   else
   {
      XtSetSensitive (panel.popup_data->deinstall_item, True);
      XtSetArg (args[0], XmNuserData, control_data);
      XtSetValues(panel.popup_data->deinstall_item, args, 1);

      XtRemoveAllCallbacks (panel.popup_data->deinstall_item, 
                            XmNactivateCallback);

      XtAddCallback (panel.popup_data->deinstall_item, XmNactivateCallback,
                     DeleteControlCB, (XtPointer) NULL);

      if ( control_data ==  subpanel_data->default_control)
         XtSetSensitive (panel.popup_data->toggle_item, False);
      else
         XtSetSensitive (panel.popup_data->toggle_item, True);

      XtSetArg (args[0], XmNuserData, control_data);
      XtSetValues(panel.popup_data->toggle_item, args, 1);

      XtRemoveAllCallbacks (panel.popup_data->toggle_item,
                            XmNactivateCallback);

      XtAddCallback (panel.popup_data->toggle_item, XmNactivateCallback,
                     ToggleControlCB, (XtPointer) subpanel_data);

      XtRemoveAllCallbacks (panel.popup_data->help_item, XmNactivateCallback);

      XtAddCallback(panel.popup_data->help_item, XmNactivateCallback,
		    ControlTopicHelpCB, (XtPointer)control_data);
   }

   XtManageChild (panel.popup_data->deinstall_item);

   if ((Boolean) (subpanel_data->parent_control_data->
                  element_values[CONTROL_LOCKED].parsed_value) != True)
      XtManageChild (panel.popup_data->toggle_item);
   else
      XtUnmanageChild (panel.popup_data->toggle_item);
   
   XtUnmanageChild (panel.popup_data->modify_subpanel_item);
}




/************************************************************************
 *
 *  PopupMenu
 *	Callback function which updates the popup menu for main panel
 *	and subpanel controls before the menu is posted.
 *
 ************************************************************************/

#ifdef _NO_PROTO
static void
PopupMenu (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else

static void
PopupMenu (Widget    w,
           XtPointer client_data,
           XtPointer call_data)

#endif /* _NO_PROTO */

{
   XmAnyCallbackStruct * callback;
   XEvent * event;
   Widget form, focus_widget;
   XmManagerWidget mgr;
   SubpanelData * subpanel_data = NULL;
   ControlData * control_data;
   BoxData * box_data = NULL;
   XmString label_string;
   Arg args[5];
   int control_type, action_count = panel.popup_data->action_count;

   callback = (XmAnyCallbackStruct *) call_data;
   event = (XEvent *) callback->event;


   /*  Get the form where the menu was posted from.  */

   form = XmGetPostedFromWidget(w);

   XtSetArg (args[0], XmNuserData, &control_type);
   XtGetValues (form, args, 1);

   if (control_type == CONTROL)
   {
      GetBoxDataFromForm (form, &box_data);
      subpanel_data = NULL;
   }
   else if (control_type == SUBPANEL)
   {
      GetSubpanelDataFromForm (form, &subpanel_data);
      box_data = NULL;
   }

   if (event->type == ButtonPress)
   {
      /* Change to use new Xme function when porting to Motif 2.0 */
      /* Get gadget at x, y position */

      focus_widget = (Widget) _XmInputInGadget (form, event->xbutton.x,
					       event->xbutton.y);
      if (focus_widget == NULL)
         return;

      /* set the menu positioning */

      XmMenuPosition (panel.popup_data->popup, (XButtonPressedEvent *)event);
   }
   else
   {
      /* get the widget that has the traversal focus */

      focus_widget = XmGetFocusWidget (form);


      /* set the menu positioning for non button events */

      KeyMenuPosition (panel.popup_data->popup, focus_widget);
   }

   if (control_type == CONTROL)
   {
      control_data = GetMainControlData (box_data, focus_widget);

      /* Find the right subpanel data if main control is not default */
      /* control.						     */

      UpdateMainControlMenu (control_data);

      if (control_data->subpanel_data != NULL &&			
          control_data != control_data->subpanel_data->default_control)
      {
         subpanel_data = control_data->subpanel_data;
	 control_data = control_data->subpanel_data->default_control;
         control_data->subpanel_data = subpanel_data;
         UpdateMainControlMenu (control_data);
      }

      SetupActionMenuItems (w, control_data, panel.popup_data,
                            &(panel.popup_data->action_item), &action_count);

   }
   else if (control_type == SUBPANEL)
   {
      control_data = GetSubpanelControlData (subpanel_data, focus_widget);

      UpdateSubpanelControlMenu (subpanel_data, control_data);

      SetupActionMenuItems (w, control_data, panel.popup_data,
			    &(panel.popup_data->action_item), &action_count);
   }

   panel.popup_data->action_count = action_count;
}




/************************************************************************
 *
 *  CreatePopupMenu
 *
 ************************************************************************/

#ifdef _NO_PROTO

void
CreatePopupMenu (w)
Widget w;

#else

void
CreatePopupMenu (Widget w)

#endif /* _NO_PROTO */

{
   Widget children[20];
   char * mnemonic;
   Arg args[5];
   PopupData * popup_data;
   int i, child_num = 0;
   XmString label_string;

   popup_data = panel.popup_data = (PopupData *) XtMalloc(sizeof(PopupData));

   i = 0;
   XtSetArg (args[i], XmNwhichButton, wmGD.bMenuButton); i++;

   popup_data->popup = XmCreatePopupMenu(w, "FPPopup", args, i);

   popup_data->add_ws_item = NULL;

   XtAddCallback (popup_data->popup, XmNmapCallback, PopupMenu,
		  (XtPointer) NULL);

   popup_data->popup_title = children[child_num++] =
		 XmCreateLabelGadget(popup_data->popup, "title", NULL, 0);

   children[child_num++] = XmCreateSeparatorGadget(popup_data->popup,
						   "sep1", NULL, 0);
   children[child_num++] = XmCreateSeparatorGadget(popup_data->popup,
						   "sep2", NULL, 0);


   popup_data->action_count = 0;
   popup_data->action_item =  (Widget *) NULL; 

   popup_data->separator1 = children[child_num++] =
   		XmCreateSeparatorGadget (popup_data->popup, "sep3", NULL, 0);

   label_string =
           XmStringCreateLocalized((FPGETMESSAGE(82, 14, "Copy To Main Panel")));
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)FPGETMESSAGE(82, 15, "C"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   popup_data->toggle_item = children[child_num] =
      XmCreatePushButtonGadget (popup_data->popup, "Toggle_Control", args, 2);
   XmStringFree (label_string);

   label_string = XmStringCreateLocalized((FPGETMESSAGE(82, 16, "Delete")));
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)FPGETMESSAGE(82, 17, "D"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   popup_data->deinstall_item = children[child_num] =
	 XmCreatePushButtonGadget(popup_data->popup, "Delete_Control",
				  args, 2);
   XmStringFree (label_string);

   label_string =
              XmStringCreateLocalized ((FPGETMESSAGE(82, 10, "Add Subpanel")));
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)FPGETMESSAGE(82, 11, "A"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   popup_data->modify_subpanel_item = children[child_num] =
	 XmCreatePushButtonGadget (popup_data->popup, "Add_Subpanel", args, 2);
   XmStringFree (label_string);

   XtAddCallback(children[child_num++], XmNactivateCallback, AddSubpanelCB,
			(XtPointer)NULL);
   popup_data->subpanel_add_state = True;

   popup_data->separator2 = children[child_num++] =
   		XmCreateSeparatorGadget(popup_data->popup, "sep4", NULL, 0);

   label_string = XmStringCreateLocalized ((FPGETMESSAGE(82, 18, "Help")));
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)FPGETMESSAGE(82, 19, "H"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   popup_data->help_item = children[child_num++] =
   		XmCreatePushButtonGadget (popup_data->popup, "Help", args, 2);
   XmStringFree (label_string);

   XtManageChildren(children, child_num);
}




/************************************************************************
 *
 *  CreateActionButtons
 *     Show the the action menu items and hide workspace menu items since
 *     we are over a control.
 *
 *
 ************************************************************************/
 
#ifdef _NO_PROTO

static void
CreateActionButtons (w, switch_data, control_data)
Widget        w;
SwitchData *  switch_data;
ControlData * control_data;

#else

static void
CreateActionButtons (Widget        w,
	             SwitchData  * switch_data,
	             ControlData * control_data)

#endif /* _NO_PROTO */

{
    PopupData * popup_data = switch_data->popup_data;
    int action_count = popup_data->action_count;
    Arg args[1];
    XmString label_string;
    char     * control_label;

   /*  Set the title of the menu  */
   
    control_label =
	  (char *) control_data->element_values[CONTROL_LABEL].parsed_value;

    if (control_label == NULL)
       control_label = 
	       (char *) control_data->element_values[CONTROL_NAME].parsed_value;

    label_string = XmStringCreateLocalized (control_label);
    XtSetArg (args[0], XmNlabelString, label_string);
    XtSetValues (popup_data->popup_title, args, 1);
    XmStringFree (label_string);

   /* show actions items */
    SetupActionMenuItems(w, control_data, popup_data,
			 &(popup_data->action_item), &action_count);
    popup_data->action_count = action_count;

   /* hide add and delete workspace items */
    XtUnmanageChild(popup_data->add_ws_item);
    XtUnmanageChild(popup_data->delete_ws_item);
    XtUnmanageChild(popup_data->rename_ws_item);

   /* reset the help callback to reflect switch control help */
    XtRemoveAllCallbacks (popup_data->help_item, XmNactivateCallback);
    XtAddCallback(popup_data->help_item, XmNactivateCallback,
                  ControlTopicHelpCB, control_data);
}





/************************************************************************
 *
 *  ShowWorkspaceItems
 *     Don't show the the action menu items since we are not over a control.
 *     Reset the activate callbacks with the latest data.
 *
 ************************************************************************/
 
#ifdef _NO_PROTO

static void
ShowWorkspaceItems (switch_data, focus_widget)
SwitchData * switch_data;
Widget       focus_widget;

#else

static void
ShowWorkspaceItems (SwitchData * switch_data,
                           Widget       focus_widget)

#endif /* _NO_PROTO */

{
   PopupData * popup_data = switch_data->popup_data;
   int i;
   Arg args[1];
   XmString label_string;
   char     * control_label;
   char     * ws;
   char     * ws_name;
   char     * format;


   /* hide actions items */

   for (i = 0; i < popup_data->action_count; i++)
       XtUnmanageChild (popup_data->action_item[i]);

   /* show add and delete workspace items */

   XtManageChild (popup_data->add_ws_item);
   XtManageChild (popup_data->delete_ws_item);
   XtManageChild (popup_data->rename_ws_item);


   /* reset the activate callback to reflect current button index */

   XtRemoveAllCallbacks (popup_data->delete_ws_item, XmNactivateCallback);
   for (i = 0; i < switch_data->switch_count &&
               switch_data->buttons[i] != NULL; i++)
   {
      if (switch_data->buttons[i] == focus_widget) break;
   }


   /*  If we didn't find the switch button to post the menu for, it   */
   /*  must be in edit mode so the correct menu is the active switch  */

   if (i >= switch_data->switch_count)
      i = switch_data->active_switch;


   /*  Set the title of the menu  */
   
   format = strdup (FPGETMESSAGE(82, 20, "%1$s %2$s"));
   ws = strdup (FPGETMESSAGE(82, 21, "Workspace"));
   ws_name = switch_data->switch_names[i];

   control_label = XtMalloc (sizeof(char) * (strlen(ws) + strlen(ws_name) + 4));

   sprintf (control_label, format, ws, ws_name);
   free (format);
   free (ws);

   label_string = XmStringCreateLocalized (control_label);
   XtSetArg (args[0], XmNlabelString, label_string);
   XtSetValues (popup_data->popup_title, args, 1);
   XmStringFree (label_string);
   XtFree(control_label);

   XtAddCallback (popup_data->delete_ws_item, XmNactivateCallback,
	          DeleteWorkspaceCB, (XtPointer) i);

   /* reset the activate callback to reflect current button index */
   XtRemoveAllCallbacks (popup_data->rename_ws_item, XmNactivateCallback);
   XtAddCallback (popup_data->rename_ws_item, XmNactivateCallback,
	          RenameWorkspaceCB, (XtPointer) i);

   /* reset the help callback to reflect switch button help */
   XtRemoveAllCallbacks (popup_data->help_item, XmNactivateCallback);
   XtAddCallback(popup_data->help_item, XmNactivateCallback,
                 GeneralTopicHelpCB, (XtPointer) SWITCH_BUTTON);

   XtManageChild(popup_data->separator1);
}




/************************************************************************
 *
 *  ShowPartialWorkspaceItems
 *     Don't show the delete workspace item since we are not over
 *     a workspace button and don't show the action menu items since we are
 *     not over a control.  Reset the activate callback with the latest data.
 *
 ************************************************************************/
 
#ifdef _NO_PROTO

static void
ShowPartialWorkspaceItems (switch_data)
SwitchData * switch_data;

#else
static void
ShowPartialWorkspaceItems(SwitchData * switch_data)

#endif /* _NO_PROTO */

{
   PopupData * popup_data = switch_data->popup_data;
   int i;
   Arg args[1];
   XmString label_string;

   label_string = XmStringCreateLocalized((FPGETMESSAGE(82, 22, "Switch Area")));
   XtSetArg (args[0], XmNlabelString, label_string);
   XtSetValues (popup_data->popup_title, args, 1);
   XmStringFree (label_string);

   /* hide actions items */
   for (i = 0; i < popup_data->action_count; i++)
       XtUnmanageChild (popup_data->action_item[i]);

   /* show add workspace item */
   XtManageChild (popup_data->add_ws_item);

   /* hide delete and rename workspace items */
   XtUnmanageChild (popup_data->delete_ws_item);
   XtUnmanageChild (popup_data->rename_ws_item);

   /* reset the help callback to reflect switch area help */
   XtRemoveAllCallbacks (popup_data->help_item, XmNactivateCallback);
   XtAddCallback(popup_data->help_item, XmNactivateCallback,
		 SwitchTopicHelpCB, (XtPointer)switch_data);

   XtManageChild(popup_data->separator1);
}





/************************************************************************
 *
 *  GetSwitchDataFromForm
 *
 ************************************************************************/
 

#ifdef _NO_PROTO

static void
GetSwitchDataFromForm (form, switch_data_rtn)
Widget        form;
SwitchData ** switch_data_rtn;

#else

static void
GetSwitchDataFromForm (Widget        form,
                       SwitchData ** switch_data_rtn)

#endif /* _NO_PROTO */

{
   BoxData * box_data;
   int i;

   for (i = 0; i < panel.box_data_count; i++)
   {
      box_data = panel.box_data[i];

      if (box_data->switch_form == form)
      {
         *switch_data_rtn = box_data->switch_data;
         break;
      }
   }
}




/************************************************************************
 *
 *  GetSwtichControlData
 *
 ************************************************************************/
 
#ifdef _NO_PROTO

static ControlData *
GetSwitchControlData (switch_data, focus_widget)
SwitchData * switch_data;
Widget       focus_widget;

#else

static ControlData *
GetSwitchControlData (SwitchData * switch_data,
	              Widget       focus_widget)

#endif /* _NO_PROTO */

{
   int i;
   ControlData * control_data;
   Boolean found = False;

   for (i = 0; i < switch_data->control_data_count; i++)
   {
      control_data = switch_data->control_data[i];

      if (control_data->icon == focus_widget)
      {
         found = True;
         break;
      }
   }

   if (!found) return NULL;

   return (control_data);
}





/************************************************************************
 *
 *  WSPopupMenu
 *
 ************************************************************************/
 
#ifdef _NO_PROTO

static void
WSPopupMenu (w, client_data, call_data)
Widget    w;
XtPointer client_data;
XtPointer call_data;

#else
static void
WSPopupMenu (Widget     w,
             XtPointer client_data,
             XtPointer call_data)

#endif /* _NO_PROTO */

{
   XmManagerWidget mgr = (XmManagerWidget) w;
   int num_children = mgr->composite.num_children;
   XmAnyCallbackStruct * callback;
   XEvent * event;
   Widget form, focus_widget;
   SwitchData * switch_data;
   ControlData * control_data = NULL;
   Boolean is_button = False;
   Position x, y;
   Arg args[2];
   int control_type;

   callback = (XmAnyCallbackStruct *) call_data;
   event = (XEvent *) callback->event;


   /* get the form where the menu was posted from */

   form = XmGetPostedFromWidget (w);

   GetSwitchDataFromForm (form, &switch_data);


   if (event->type == ButtonPress)
   {

     /*  Change to use new Xme function when porting to Motif 2.0  */
     /*  Get gadget at x, y position                               */

     focus_widget = (Widget) _XmInputInGadget (form, event->xbutton.x,
					       event->xbutton.y);

     if (focus_widget == NULL)
     {
        XtSetArg (args[0], XmNx, &x);
        XtSetArg (args[1], XmNy, &y);
        XtGetValues(switch_data->rc, args, 2);

        focus_widget = (Widget) _XmInputInGadget (switch_data->rc,
					          event->xbutton.x - x,
					          event->xbutton.y - y);
        is_button = True;
     }
     else
        control_data = GetSwitchControlData (switch_data, focus_widget);


     /* set the menu positioning */

     XmMenuPosition (switch_data->popup_data->popup,
                     (XButtonPressedEvent *) event);
   }
   else
   {
 
      /* get the widget that has the traversal focus */
  
      focus_widget = XmGetFocusWidget (form);

      control_data = GetSwitchControlData (switch_data, focus_widget);

      if (control_data == NULL)
         is_button = True;

      /* set the menu positioning for non button events */

      KeyMenuPosition (switch_data->popup_data->popup, focus_widget);
   }

   if (focus_widget != NULL)
   {
      if (is_button)
        /* The mouse is over a button in the switch area */
	 ShowWorkspaceItems (switch_data, focus_widget);
      else
      {
         if (control_data != NULL)
         {
            control_type =
		 (int) control_data->element_values[CONTROL_TYPE].parsed_value;

            if (control_type != CONTROL_BLANK)
              /* The mouse is over a control in the switch area */
	       CreateActionButtons (w, switch_data, control_data);
            else
              /* The mouse is over a blank control in the switch area */
               ShowPartialWorkspaceItems (switch_data);
         }
         else
         {
              /*
               * The mouse is over area other than the buttons and control
               * in the switch area
               */
      	       ShowPartialWorkspaceItems (switch_data);
         }
      }
   } 
   else
      /*
       * The mouse is over area other than the buttons and control
       * in the switch area
       */
      ShowPartialWorkspaceItems (switch_data);
}




/************************************************************************
 *
 *  CreateWorkspacePopupMenu
 *
 ************************************************************************/
 
 
#ifdef _NO_PROTO

void
CreateWorkspacePopupMenu (w, switch_data)
Widget        w;
SwitchData * switch_data;

#else
void
CreateWorkspacePopupMenu (Widget       w,
                          SwitchData * switch_data)

#endif /* _NO_PROTO */

{
   Widget children[20];
   XmString label_string;
   char * mnemonic;
   Arg args[5];
   int i, child_num = 0;
   PopupData * popup_data;

   popup_data = (PopupData *) XtMalloc(sizeof(PopupData));

   i = 0;
   XtSetArg (args[i], XmNwhichButton, wmGD.bMenuButton); i++;

   popup_data->popup = XmCreatePopupMenu(w, "WSPopup", args, i);

   popup_data->deinstall_item = NULL;
   popup_data->toggle_item = NULL;
   popup_data->modify_subpanel_item = NULL;
   popup_data->subpanel_add_state = True;

   XtAddCallback(popup_data->popup, XmNmapCallback, WSPopupMenu,
		 (XtPointer) switch_data);

   label_string = XmStringCreateLocalized((FPGETMESSAGE(82, 22, "Switch Area")));
   XtSetArg (args[0], XmNlabelString, label_string);
   popup_data->popup_title = children[child_num++] =
   		XmCreateLabelGadget (popup_data->popup, "title", args, 1);
   XmStringFree (label_string);

   children[child_num++] = XmCreateSeparatorGadget (popup_data->popup, "sep1",
						    NULL, 0);
   children[child_num++] = XmCreateSeparatorGadget (popup_data->popup, "sep2",
						    NULL, 0);

   popup_data->action_count = 0;
   popup_data->action_item =  (Widget *) NULL; 

   label_string =
          XmStringCreateLocalized ((FPGETMESSAGE(82, 23, "Add Workspace")));
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)FPGETMESSAGE(82, 24, "A"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   popup_data->add_ws_item = children[child_num] =
	 XmCreatePushButtonGadget (popup_data->popup, "Add_Workspace", args, 2);
   XmStringFree (label_string);

   XtAddCallback(children[child_num++], XmNactivateCallback, AddWorkspaceCB,
			(XtPointer)NULL);

   label_string = XmStringCreateLocalized((FPGETMESSAGE(82, 25, "Delete")));
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)FPGETMESSAGE(82, 26, "D"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   popup_data->delete_ws_item = children[child_num] = 
 	                XmCreatePushButtonGadget (popup_data->popup,
						  "Delete_Workspace", args, 2);
   XmStringFree (label_string);

   XtAddCallback(children[child_num++], XmNactivateCallback,
	         DeleteWorkspaceCB, (XtPointer) 0);

   label_string = XmStringCreateLocalized((FPGETMESSAGE(82, 27, "Rename")));
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)FPGETMESSAGE(82, 28, "R"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   popup_data->rename_ws_item = children[child_num] = 
 	                XmCreatePushButtonGadget (popup_data->popup,
						  "Rename_Workspace", args, 2);
   XmStringFree (label_string);

   XtAddCallback(children[child_num++], XmNactivateCallback,
	         RenameWorkspaceCB, (XtPointer) 0);

   popup_data->separator1 = children[child_num++] =
      XmCreateSeparatorGadget (popup_data->popup, "sep3", NULL, 0);

   popup_data->separator2 = NULL;

   label_string = XmStringCreateLocalized((FPGETMESSAGE(82, 29, "Help")));
   XtSetArg (args[0], XmNlabelString, label_string);
   mnemonic = ((char *)FPGETMESSAGE(82, 30, "H"));
   XtSetArg (args[1], XmNmnemonic, mnemonic[0]);
   popup_data->help_item = children[child_num++] =
   		XmCreatePushButtonGadget (popup_data->popup, "Help", args, 2);
   XmStringFree (label_string);

   XtManageChildren(children, child_num);

   switch_data->popup_data = popup_data;
}



