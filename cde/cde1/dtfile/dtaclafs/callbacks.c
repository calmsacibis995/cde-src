/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           callbacks.c
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    Callback routines
 *
 *   FUNCTIONS: MIN
 *		addCB
 *		addCancelCB
 *		addOkCB
 *		applyCB
 *		cancelCB
 *		copyAclLines
 *		copyVerticalScrollPos
 *		deleteCB
 *		deleteCancelCB
 *		deleteOkCB
 *		deleteSelectCB
 *		destroyCB
 *		getUserNames
 *		modeCB
 *		questionDialogCB
 *		removeNoRightsUsers
 *		resetCB
 *		scrollToUser
 *		showNegCB
 *		updateRights
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdlib.h>
#include <errno.h>

#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include "dtaclafs.h"

/* error message buffer */
#define EMSGMAX 256
char errorMessage[EMSGMAX];

/* local functions */
#ifdef _NO_PROTO
static void updateRights();
static void removeNoRightsUsers();
static void getUserNames();
static void copyVerticalScrollPos();
static void scrollToUser();
static void copyAclLines();
#else
static void updateRights(const AclDisplay *aclDisplay, AclDef *displayedRights);
static void removeNoRightsUsers(int *nRights, AclEntry entry[]);
static void getUserNames(XmString users[], int numUsers, AclLine *aclLine);
static void copyVerticalScrollPos(Widget toScrolledWindow, Widget fromScrolledWindow);
static void scrollToUser(String user, DialogContext *context);
static void copyAclLines(AclLine *to[], AclLine *from[], int n);
#endif

/*---------------------------------------------------------------------
 *
 *  modeCB
 *
 *  callback for the mode (advanced vs basic) toggle button
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
modeCB(w, context, cbs)
   Widget                         w;
   DialogContext                * context;
   XmToggleButtonCallbackStruct * cbs;
#else
modeCB(Widget w, DialogContext *context, XmToggleButtonCallbackStruct *cbs)
#endif

{     

   /* make sure any newly-enetered rights are reflected in the new mode */
   updateRights(&(context->aclDisplay), &(context->displayedAcl));
   setDisplayedAcl(context, context->displayedAcl); 


   if (cbs->set)
   {   
      /* change to advanced mode */ 
      context->aclDisplay.currentMode = ADVANCED;
      
      /* maintain vertical scroll position between modes */
      copyVerticalScrollPos(context->advNorm.scroll, context->basicNorm.scroll);
      copyVerticalScrollPos(context->advNeg.scroll,  context->basicNeg.scroll); 

      XtUnmanageChild (aclMgr(context->basicNorm));  
      XtUnmanageChild (aclMgr(context->basicNeg));  
      if (context->showNeg) XtManageChild   (aclMgr(context->advNeg));
      XtManageChild   (aclMgr(context->advNorm));
   }
   else
   {
      /* change to basic mode */
      context->aclDisplay.currentMode = BASIC;
      
      /* maintain vertical scroll position between modes */
      copyVerticalScrollPos(context->basicNorm.scroll, context->advNorm.scroll);
      copyVerticalScrollPos(context->basicNeg.scroll,  context->advNeg.scroll);   

      XtUnmanageChild (aclMgr(context->advNorm));
      XtUnmanageChild (aclMgr(context->advNeg));
      if (context->showNeg) XtManageChild   (aclMgr(context->basicNeg));
      XtManageChild   (aclMgr(context->basicNorm));
   }
   


}  /* end modeCB */


/*---------------------------------------------------------------------
 *
 *  cancelCB
 *
 *  callback for the cancel action, called when cancel button is pressed
 *     or close was selected from the window menu (in the latter case,
 *     cbs->reason != XmCR_ACTIVATE
 *  
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
cancelCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
cancelCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{


   /* record any changes made to permissions */
   updateRights(&(context->aclDisplay), &(context->displayedAcl));
   
   /* have any changes been made but not applied? */
   if (aclsMatch(context->displayedAcl, context->initialAcl)) 
      exit(EXIT_SUCCESS);

   else
      if (yesOrNo(GETMESSAGE(1, 49, "Do you want to ignore the changes that\nyou've made to the AFS Permissions?")) == YES)
         exit(EXIT_SUCCESS);
      else
         return;
   

}  /* end cancelCB */

  

/*---------------------------------------------------------------------
 *
 *  resetCB
 *
 *  callback for the reset action
 *  assume that the acl has not been changed by another process
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
resetCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
resetCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{

   setDisplayedAcl(context, context->initialAcl);

}  /* end resetCB */




/*---------------------------------------------------------------------
 *
 *  applyCB
 *
 *  callback for the apply and ok actions
 *  assume that the acl has not been changed by another process
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
applyCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
applyCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{
   AclDef  trialAcl;
   int     numUsers;

   String message="Possible reasons include:\n \
-File not in AFS\n \
-Too many users on access control list\n \
-Tried to add non-existent user to access control list";

   /* record any changes made to permissions */
   updateRights(&(context->aclDisplay), &(context->displayedAcl));

   /* remove any users whose rights have all been turned off */
   /* use trial-ACL so the version in display is accurate if setACL fails or */
   /*    the user decides this not what is wanted */
   copyAcl(&trialAcl, &(context->displayedAcl));
   numUsers = trialAcl.nNorm + trialAcl.nNeg;
   removeNoRightsUsers(&(trialAcl.nNorm), trialAcl.norm);
   removeNoRightsUsers(&(trialAcl.nNeg),  trialAcl.neg);

   if ( (trialAcl.nNorm + trialAcl.nNeg) < numUsers)
   {
      if (yesOrNo(GETMESSAGE(1, 46, "One or more users (or groups) will be deleted because no permissions are set.\nDo you wish to continue?")) == NO) return;
   }

   /* update the ACL associated with the directory */
   /*    if the OK button was pressed, we're done  */
   /*    if the apply button was pressed, update the display */
   /*    if there is a problem, inform the user */
   if (setFileAcl(context->directory, &trialAcl) == 0)
   {
      if (w == context->okButton)
         exit(EXIT_SUCCESS);
      copyAcl(&(context->displayedAcl), &trialAcl);
      copyAcl(&(context->initialAcl), &(context->displayedAcl));
      checkAuth(context);
   }
   else
   {
      strncpy(errorMessage,
              GETMESSAGE(1, 17, "Cannot set AFS Permissions for "),
              EMSGMAX);
      strncat(errorMessage,context->directory,EMSGMAX);
      strncat(errorMessage,":\n",EMSGMAX);
      strncat(errorMessage,strerror(errno),EMSGMAX);
      strncat(errorMessage,"\n");
      strncat(errorMessage, GETMESSAGE(1, 18, message), EMSGMAX);
      displayError(errorMessage);
   }
   setDisplayedAcl(context, context->displayedAcl);


}  /* end applyCB */


/*---------------------------------------------------------------------
 *
 *  destroyCB
 *
 *  callback for the warning and error dialogs
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
destroyCB(w, dialog, cbs)
   Widget                 w;
   Widget                 dialog;
   XmAnyCallbackStruct  * cbs;
#else
destroyCB(Widget w, Widget dialog, XmAnyCallbackStruct *cbs)
#endif

{

   XtDestroyWidget(dialog);

}  /* end destroyCB */



/*---------------------------------------------------------------------
 *
 *  showNegCB
 *
 *  callback to turn display of negative permissions on and off
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
showNegCB(w, context, cbs)
   Widget                          w;
   DialogContext                 * context;
   XmToggleButtonCallbackStruct  * cbs;
#else
showNegCB(Widget w, DialogContext * context, XmAnyCallbackStruct *cbs)
#endif

{

   if (cbs->set)
   {
      context->showNeg = TRUE;

      XtUnmanageChild(aclMgr(context->basicNorm));
      XtUnmanageChild(aclMgr(context->advNorm));
      XtVaSetValues(aclMgr(context->basicNorm),
                    XmNbottomAttachment, XmATTACH_WIDGET,
                    XmNbottomWidget,     aclMgr(context->basicNeg),
                    NULL);
      XtVaSetValues(aclMgr(context->advNorm),
                    XmNbottomAttachment, XmATTACH_WIDGET,
                    XmNbottomWidget,     aclMgr(context->advNeg),
                    NULL);

      if (context->aclDisplay.currentMode == ADVANCED)
      {
         XtManageChild(aclMgr(context->advNeg));
         XtManageChild(aclMgr(context->advNorm));
      }
      else
      {
         XtManageChild(aclMgr(context->basicNeg));
         XtManageChild(aclMgr(context->basicNorm));
      }
   }
   else
   {
      context->showNeg = FALSE;

      XtVaSetValues(aclMgr(context->basicNorm),
                    XmNbottomAttachment, XmATTACH_FORM,
                    NULL);
      XtVaSetValues(aclMgr(context->advNorm),
                    XmNbottomAttachment, XmATTACH_FORM,
                    NULL);

      XtUnmanageChild(aclMgr(context->basicNeg));
      XtUnmanageChild(aclMgr(context->advNeg));

   }

}  /* end showNegCB */

/*---------------------------------------------------------------------
 *
 *  addCB
 *
 *  callback for the add button
 *  displays a dialog for adding new users/groups to the ACL
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
addCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
addCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{
   /* determine which add button was pushed */
   context->addNormalAcl =  ( w == context->advNorm.addButton ||
                              w == context->basicNorm.addButton );

   if (context->addNormalAcl)
      XtVaSetValues(context->addDialogShell,
                    XmNtitle, GETMESSAGE(1, 47, "Add User or Group to Positive Permissions"),
                    NULL);
   else
      XtVaSetValues(context->addDialogShell,
                    XmNtitle, GETMESSAGE(1, 48, "Add User or Group to Negative Permissions"),
                    NULL);


   /* clear the dialog fields before displaying ... this dialog gets re-used */
   XmTextSetString (context->addNewuserText,      "");
   TGsetToggles    (context->addAdvNewuserTG,      0); 
   TGsetToggles    (context->addBasNewuserTG,      0);
 
   setAddDialogMode(context);
   XtManageChild(context->addForm);

}  /* end addCB */



/*---------------------------------------------------------------------
 *
 *  deleteCB
 *
 *  callback for the delete button
 *  displays a dialog for deleting new users/groups from the ACL
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
deleteCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
deleteCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{  

   int       numUsers;
   XmString  users[ACL_MAXENTRIES];
   

   /* determine which add button was pushed */
   context->deleteNormalAcl =  ( w == context->advNorm.deleteButton ||
                                 w == context->basicNorm.deleteButton );

   if (context->deleteNormalAcl)
      XtVaSetValues(context->deleteDialogShell,
                    XmNtitle, GETMESSAGE(1, 50, "Delete User or Group from Positive Permissions"),
                    NULL);
   else
      XtVaSetValues(context->deleteDialogShell,
                    XmNtitle, GETMESSAGE(1, 51, "Delete User or Group from Negative Permissions"),
                    NULL);
                    
   /* create the list of users/groups */
   XmListDeleteAllItems(context->deleteList);
   
   numUsers = context->deleteNormalAcl ? context->aclDisplay.nNormal : context->aclDisplay.nNegative; 
   
   if (numUsers == 0)
      displayWarning(GETMESSAGE(1, 52, "There are no users to delete"));

   else
   {
      
      if (context->deleteNormalAcl)
         if (context->aclDisplay.currentMode == ADVANCED)
            getUserNames(users, numUsers, context->aclDisplay.advNormAcl);
         else
            getUserNames(users, numUsers, context->aclDisplay.basicNormAcl); 
      else
         if (context->aclDisplay.currentMode == ADVANCED)
            getUserNames(users, numUsers, context->aclDisplay.advNegAcl);
         else
            getUserNames(users, numUsers, context->aclDisplay.basicNegAcl);
    
      /* the 0 indicates that items should be appended to any existing list */
      XmListAddItemsUnselected(context->deleteList, users, numUsers, 0); 
      XtVaSetValues(context->deleteList, XmNvisibleItemCount, numUsers, NULL);
      
      /* until something is selected, leave Ok unsensitive */
      XtSetSensitive(context->deleteOkButton, FALSE);
      XtAddCallback(context->deleteList, XmNextendedSelectionCallback, deleteSelectCB, context); 


      XtManageChild(context->deleteForm);
   }


}  /* end deleteCB */


/*---------------------------------------------------------------------
 *
 *  addCancelCB
 *
 *  callback for the cancel button of add dialog
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
addCancelCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
addCancelCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{

   XtUnmanageChild(context->addForm);

}  /* end addCancelCB */


/*---------------------------------------------------------------------
 *
 *  addOkCB
 *
 *  callback for the OK button of add dialog
 *  update the internal acl with name & rights from add user/group dialog
 *  the file is not updated until apply or ok are pressed in main dialog
 *  updating the internal acl will result in the new user/group being 
 *     added to the list of users/groups displayed for the user
 *  if the user did not provide a name or did not set any permissions,
 *     a warning is displayed and the add dialog is not dismissed
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
addOkCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
addOkCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{ 
   String  newUser;
   RightsL newRights;

   
   newUser   = XmTextFieldGetString(context->addNewuserText);
   
   if (context->aclDisplay.currentMode == BASIC)
      newRights = TGgetRights(context->addBasNewuserTG);
   else
      newRights = TGgetRights(context->addAdvNewuserTG);
   
   if (newRights != 0  &&  strlen(newUser) > 0)
   {
      if (context->addNormalAcl)
         addNormalUser(context->directory, &(context->displayedAcl), newUser, newRights);
      else
         addNegativeUser(context->directory, &(context->displayedAcl), newUser, newRights);

      setDisplayedAcl(context, context->displayedAcl);

      /* make sure the added user is visible in the scroll window */
      scrollToUser(newUser, context);

      XtFree(newUser);
      XtUnmanageChild(context->addForm);
   }
   else /* user did not provide user/group name or did not set any permissions */
   {
      strncpy(errorMessage,
              GETMESSAGE(1, 42, "You pressed OK but either no name was entered or no permissions were set"),
              EMSGMAX);
      displayError(errorMessage);
   }
      

}  /* end addOkCB */


/*---------------------------------------------------------------------
 *
 *  deleteOkB
 *
 *  callback for the OK button of delete dialog
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
deleteOkCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
deleteOkCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{
   XmStringTable  selectedUsers;
   int            numSelectedUsers;
   int          * selectedUserPos;
   int            i;

   
   XmListGetSelectedPos(context->deleteList, &selectedUserPos, &numSelectedUsers);
   
   if (numSelectedUsers != 0)
   {
      deleteUsers(context->deleteNormalAcl, 
                  selectedUserPos, 
                  numSelectedUsers, 
                  &(context->displayedAcl));

      setDisplayedAcl(context, context->displayedAcl);
   }
   
   XtFree(selectedUserPos);

   XtUnmanageChild(context->deleteForm);

}  /* end deleteOkCB */


/*---------------------------------------------------------------------
 *
 *  deleteCancelCB
 *
 *  callback for the cancel button of delete dialog
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
deleteCancelCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
deleteCancelCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{

   XtUnmanageChild(context->deleteForm);

}  /* end deleteCancelCB */


/*---------------------------------------------------------------------
 *
 *  deleteSelectCB
 *
 *  callback for selection of an item in the list of the delete dialog
 *  The ok button is left insensitive until an item is selected. 
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
deleteSelectCB(w, context, cbs)
   Widget                 w;
   DialogContext        * context;
   XmAnyCallbackStruct  * cbs;
#else
deleteSelectCB(Widget w, DialogContext *context, XmAnyCallbackStruct *cbs)
#endif

{
                               
   XtSetSensitive(context->deleteOkButton,TRUE);

}  /* end deleteSelectCB */


/*---------------------------------------------------------------------
 *
 *  questionDialogCB
 *
 *  callback for the Yes and No buttons of question dialog
 *
 *---------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
questionDialogCB(w, answerButton, cbs)
   Widget                 w;
   Answer                 answerButton;
   XmAnyCallbackStruct  * cbs;
#else
questionDialogCB(Widget w, Answer answerButton, XmAnyCallbackStruct *cbs)
#endif

{
   extern Answer answer;

   answer = answerButton;


}  /* end questionDialogCB */



/*-------------------------------------------------------------
 *
 *  updateRights
 *
 *  get the status of all displayed toggles and update the
 *    rights stored in displayedAcl
 *  no interaction with the file is performed
 *  note that basic mode cannot be used to represent all 
 *     combinations of rights, thus it may be necessary 
 *     to query the advanced-mode toggles even if the 
 *     current mode is basic
 *
 *----------------------------------------------------------*/

static void
#ifdef _NO_PROTO
updateRights(aclDisplay, displayedAcl)
   const AclDisplay  * aclDisplay;
   AclDef            * displayedAcl;
#else
updateRights(const AclDisplay *aclDisplay, AclDef *displayedAcl)
#endif

{

   int i;

   for (i = 0; i < aclDisplay->nNormal; i++)
   {  
      if (aclDisplay->currentMode == ADVANCED)
         displayedAcl->norm[i].rights = TGgetRights(aclDisplay->advNormAcl[i]->TG);
      else
      {
         displayedAcl->norm[i].rights = TGgetRights(aclDisplay->basicNormAcl[i]->TG);
         if (displayedAcl->norm[i].rights < 0)
            displayedAcl->norm[i].rights = TGgetRights(aclDisplay->advNormAcl[i]->TG);
      }
      displayedAcl->norm[i].shRights = shRights(displayedAcl->norm[i].rights);
   }

   for (i = 0; i < aclDisplay->nNegative; i++)
   {
      if (aclDisplay->currentMode == ADVANCED)
         displayedAcl->neg[i].rights   = TGgetRights(aclDisplay->advNegAcl[i]->TG);
      else
      {
         displayedAcl->neg[i].rights   = TGgetRights(aclDisplay->basicNegAcl[i]->TG);
         if (displayedAcl->neg[i].rights < 0)
            displayedAcl->neg[i].rights = TGgetRights(aclDisplay->advNegAcl[i]->TG);
      }
      displayedAcl->neg[i].shRights = shRights(displayedAcl->neg[i].rights);
   }

}   /* end updateRights */


/*-------------------------------------------------------------
 *
 *  removeNoRightsUsers
 *
 *  The pioctl interface will allow you to add users with no
 *  rights to an acl. As this is inconsistent with the
 *  command-line behavior (indeed, a subsequent fs setacl
 *  on a directory with a no-rights user/group will remove
 *  that user/group) any such users are removed here.
 *
 *------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
removeNoRightsUsers(nRights, entry)
   int       * nRights;
   AclEntry    entry[];
#else
removeNoRightsUsers(int *nRights, AclEntry entry[])
#endif

{
   int in, out;

   for (in = out = 0; in < *nRights; in++)
   {
      if (entry[in].rights != 0)
      {
         if (out != in)
         {
            strcpy(entry[out].user,entry[in].user);
            entry[out].rights   = entry[in].rights;
            entry[out].shRights = entry[in].shRights;
         }
         out++;
      }
   }

   *nRights = out;


}  /* end removeNoRightsUsers */


/*----------------------------------------------------------------
 *
 *  getUserNames
 *
 *  build a list of user/group names from the widgets in aclLine
 * 
 *  numUsers and aclLine are input, users is output
 *  users is not affected except for the first numUsers entries
 *
 *---------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
getUserNames(users, numUsers, aclLine)
         XmString     users[];
   const int        numUsers;
         AclLine  * aclLine[];
#else
getUserNames(XmString users[], int numUsers, AclLine *aclLine[])
#endif

{
   int i;
   
   for (i = 0; i < numUsers; i++)
      XtVaGetValues(aclLine[i]->name, XmNlabelString, &users[i], NULL);
   

}  /* end getUserNames */



/*----------------------------------------------------------------
 *
 *  copyVerticalScrollPos
 *
 *  copy the value of a vertical scroll bar from one scrolled
 *     window widget to another
 *
 *---------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
copyVerticalScrollPos(toScrolledWindow, fromScrolledWindow)
   Widget toScrolledWindow;
   Widget fromScrolledWindow;
#else
copyVerticalScrollPos(Widget toScrolledWindow, Widget fromScrolledWindow)
#endif

#define MIN(a,b) ( (a) < (b) ? (a) : (b))
{
   int    scrollPosition, sliderSize, maxSize;
   int    increment, pageIncrement, oldPosition, newPosition;
   Widget fromScrollBar, toScrollBar;


   XtVaGetValues(fromScrolledWindow, XmNverticalScrollBar, &fromScrollBar,  NULL);
   XtVaGetValues(toScrolledWindow,   XmNverticalScrollBar, &toScrollBar,    NULL); 

   XtVaGetValues(fromScrollBar,      XmNvalue,             &scrollPosition, NULL);

   XtVaGetValues(toScrollBar,        XmNmaximum,           &maxSize,        NULL);
   XmScrollBarGetValues(toScrollBar, &oldPosition, &sliderSize, &increment, &pageIncrement);
   newPosition = MIN(scrollPosition, maxSize-sliderSize);

   /* the last argument causes the scroll window's callbacks to be executed */
   /* without this the scroll bar would change but not the data in the clip window */
   XmScrollBarSetValues(toScrollBar, newPosition, sliderSize, increment, pageIncrement, TRUE);
   

}  /* end copyVerticalScrollPos */


/*----------------------------------------------------------------
 *
 *  scrollToUser
 *
 *  scroll in the current permission list so permissions for
 *     the indicated user/group are visible; no change is made
 *     if the user/group is not found
 *
 *---------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
scrollToUser(user, context)
   String           user;
   DialogContext  * context;
#else
scrollToUser(String user, DialogContext *context)
#endif

{

   Widget      scroll;
   AclLine   * myAclLines[ACL_MAXENTRIES];
   int         i, numUsers;
   XmString    xs;
   String      myUser;

   if (context->aclDisplay.currentMode == BASIC)
      if (context->addNormalAcl)
      {
         scroll     = context->basicNorm.scroll;
         numUsers   = context->aclDisplay.nNormal;
         copyAclLines(myAclLines, context->aclDisplay.basicNormAcl, numUsers);
      }
      else
      {
         scroll     = context->basicNeg.scroll;
         numUsers   = context->aclDisplay.nNegative;
         copyAclLines(myAclLines, context->aclDisplay.basicNegAcl, numUsers);
      }
   else
      if (context->addNormalAcl)
      {
         scroll     = context->advNorm.scroll;
         numUsers   = context->aclDisplay.nNormal;
         copyAclLines(myAclLines, context->aclDisplay.advNormAcl, numUsers);
      }
      else
      {
         scroll     = context->advNeg.scroll;
         numUsers   = context->aclDisplay.nNegative;
         copyAclLines(myAclLines, context->aclDisplay.advNegAcl, numUsers);
      }

   for (i = 0; i < numUsers; i++)
   {
      XtVaGetValues(myAclLines[i]->name,
                    XmNlabelString, &xs,
                    NULL);
      XmStringGetLtoR(xs, XmFONTLIST_DEFAULT_TAG, &myUser);
      if (strcmp(user,myUser) == 0)
      {
         XmScrollVisible(scroll, myAclLines[i]->aclForm, 0, 0);
         XmStringFree(xs);
         XtFree(myUser);
         break;
      }
      XmStringFree(xs);
      XtFree(myUser);
   }


}  /* end scrollToUser */


/*----------------------------------------------------------------
 *
 *  copyAclLines
 *
 *  AclLines is an array of pointers ... this fcn copies the ptrs
 *
 *---------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
copyAclLines(to, from, n)
   AclLine *to, *from;
   int      n;
#else
copyAclLines(AclLine *to[], AclLine *from[], int n)
#endif

{
   int i;

   for (i = 0; i < n; i++)
      to[i] = from[i];

}  /* end copyAclLines */
