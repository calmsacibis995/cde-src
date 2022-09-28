/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           setval.c
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    set values
 *
 *   FUNCTIONS: aclMgr
 *		checkAuth
 *		checkNumUserLimits
 *		manageAclLines
 *		manageAllAclLines
 *		setAclLine
 *		setAddDialogMode
 *		setDisplayedAcl
 *		setNegSummary
 *		setValues
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include "dtaclafs.h"
#include <Xm/ManagerP.h>  /* for XmManagerWidget, XtHeight */ 
 

/* local functions */
#ifdef _NO_PROTO
static void    setAclLine ();
static void    manageAllAclLines();
static void    manageAclLines();
static void    checkNumUserLimits();
#else
static void    setAclLine (AclLine  * aclLine, AclEntry aclEntry);
static void    manageAllAclLines(DialogContext * context);
static void    manageAclLines(Widget scrollForm, int nLines, AclLine * aclLines[]);
static void    checkNumUserLimits(DialogContext * context);
#endif


/*------------------------------------------------------------------
 *
 *  setValues
 *
 *  define path-dependent resources
 *  this function is called only once per invocation of dtaclafs
 *  subsequent updates to the window are handled by setDisplayedAcl
 *
 *------------------------------------------------------------------*/
 
void
#ifdef _NO_PROTO
setValues(path, directory, context, fileState)
   const String     path, directory;
   DialogContext  * context;
   const AclDef     fileState;
#else
setValues(const String path, const String directory, DialogContext *context, AclDef fileState)
#endif

{  
   char   s[2*FILENAME_MAX];    /* doubling of FILENAME_MAX is arbitrary */
   String file;
          

   /* store the directory name in the context structure and in folderName resource */
   strncpy(context->directory, directory, FILENAME_MAX);
   
   sprintf(s, GETMESSAGE(1, 4, "AFS permissions for folder: %s"), directory);
   XtVaSetValues (context->folderName,
                  XmNlabelString, XmStringCreateLtoR(s, XmFONTLIST_DEFAULT_TAG),
                  NULL);

   
   /* store the file name ... if the path points to a directory instead of */
   /* a file, the filename field is not shown */            
   if (strcmp(path,directory) != 0)
   {
      file = path + strlen(directory) + 1; 
      sprintf(s, GETMESSAGE(1, 3, "File: %s\nis inside\nFolder: %s"), file, directory);
      XtVaSetValues (context->fileName,
                     XmNlabelString, XmStringCreateLtoR(s, XmFONTLIST_DEFAULT_TAG),
                     NULL);
   }
   else
   {
      XtUnmanageChild(context->fileName);
      XtUnmanageChild(context->fileNameSeparator);
      XtVaSetValues(context->folderName,
                    XmNtopWidget, context->warningSeparator,
                    NULL);
   }


   /* set acl */
   setDisplayedAcl(context, fileState);
   
   /* disable acl-editing if no authorization */
   checkAuth(context);

}  /* end setValues */



/*--------------------------------------------------------------------------
 *
 *  setDisplayedAcl
 *
 *  set the displayed ACL so that it matches that represented in fileState;
 *  any existing aclDisplay is overwritten
 *
 *  fileState is the ACL to display
 *
 *-------------------------------------------------------------------------*/


void
#ifdef _NO_PROTO
setDisplayedAcl(context, fileState)
   DialogContext  * context;
   const AclDef     fileState;
#else
setDisplayedAcl(DialogContext *context, const AclDef fileState)
#endif

{
   int i;



   /* set the name and toggles for each user/group in the ACL */
   context->aclDisplay.nNormal = fileState.nNorm;
   XtUnmanageChild(context->advNorm.scrollForm);
   XtUnmanageChild(context->basicNorm.scrollForm);
   for (i = 0; i < fileState.nNorm; i++)
   {
      if (context->aclDisplay.advNormAcl[i] == NULL)
      {
         context->aclDisplay.advNormAcl[i]   = newAclLine(context->advNorm.scrollForm,   ADVANCED);
         context->aclDisplay.basicNormAcl[i] = newAclLine(context->basicNorm.scrollForm, BASIC);
      }
      setAclLine(context->aclDisplay.advNormAcl[i],   fileState.norm[i]);
      setAclLine(context->aclDisplay.basicNormAcl[i], fileState.norm[i]);
   }

   context->aclDisplay.nNegative = fileState.nNeg;
   for (i = 0; i < fileState.nNeg; i++)
   {
      if (context->aclDisplay.advNegAcl[i] == NULL)
      {
         context->aclDisplay.advNegAcl[i]   = newAclLine(context->advNeg.scrollForm,   ADVANCED);
         context->aclDisplay.basicNegAcl[i] = newAclLine(context->basicNeg.scrollForm, BASIC);
      }
      setAclLine(context->aclDisplay.advNegAcl[i],   fileState.neg[i]);
      setAclLine(context->aclDisplay.basicNegAcl[i], fileState.neg[i]);
   }
   
   /* update the summary of negative entries */
   setNegSummary(context, fileState.nNeg);


   manageAllAclLines(context);

   copyAcl(&(context->displayedAcl),&fileState);

   checkNumUserLimits(context);

}  /* setDisplayedAcl */


/*--------------------------------------------------------------------------
 *
 *  setAclLine
 *
 *  initalize data for one line (i.e. one user/group) in acl display
 *
 *-------------------------------------------------------------------------*/


static void
#ifdef _NO_PROTO
setAclLine(aclLine, aclEntry)
   AclLine   * aclLine;
   AclEntry    aclEntry;
#else
setAclLine (AclLine *aclLine, AclEntry aclEntry)
#endif

{
   XmString xs;


   /* set the acl-dependent resources */
   xs = XmStringCreateLtoR(aclEntry.user, XmFONTLIST_DEFAULT_TAG);
   XtVaSetValues (aclLine->name,
                  XmNlabelString, xs,
                  NULL);
   XmStringFree(xs);
   TGsetToggles(aclLine->TG, aclEntry.rights);


}  /* end setAclLine */


/*--------------------------------------------------------------------
 *
 *  setAddDialogMode
 *
 *  manage the advanced or basic group of toggles, depending on the
 *     current mode, for the add user/gruop dialog
 *
 *-------------------------------------------------------------------*/


void
#ifdef _NO_PROTO
setAddDialogMode(context)
   DialogContext  * context;
#else
setAddDialogMode(DialogContext  * context)
#endif

{
   Widget advanced=TGmanager(context->addAdvNewuserTG);
   Widget basic=TGmanager(context->addBasNewuserTG);
   Widget TGMgr;

   if (context->aclDisplay.currentMode == BASIC)
   {
      XtUnmanageChild(advanced);
      XtManageChild(basic);
      TGMgr = basic;
   }
   else
   {
      XtUnmanageChild(basic);
      XtManageChild(advanced);
      TGMgr = advanced;
   }


   XtVaSetValues(context->addNewuserText,
                 XmNrightAttachment,  XmATTACH_WIDGET,
                 XmNrightWidget,      TGMgr,
                 NULL);


}  /* end setAddDialogMode */


/*--------------------------------------------------------------------------
 *
 *  setNegSummary
 *
 *  build the string summarizing the number of negative entries in the ACL
 *     and set the appropriate resource(s)
 *
 *-------------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
setNegSummary(context, nNeg)
   DialogContext * context;
   int             nNeg;
#else
setNegSummary(DialogContext * context, int nNeg)
#endif

{  
   String   s, buf = malloc(256);

   
   if (nNeg == 0)
      s = GETMESSAGE(1, 58, "(currently no entries)");
   else if (nNeg == 1)
      s = GETMESSAGE(1, 59, "(currently 1 entry)");
   else       
   {
      sprintf(buf, 
              GETMESSAGE(1, 60, "(currently %d entries)"),
              nNeg);
      s = buf;
   }
 
   XtVaSetValues (context->summary,
                  XmNlabelString, XmStringCreateLtoR (s, XmFONTLIST_DEFAULT_TAG),
                  NULL);
                 
   free(buf);
   
}  /*  end setNegSummary */
 



/*----------------------
 *
 *  manageAllAclLines
 *
 *---------------------*/

static void
#ifdef _NO_PROTO
manageAllAclLines(context)
   DialogContext * context;
#else
manageAllAclLines(DialogContext * context)
#endif

{
   manageAclLines(context->advNorm.scrollForm,
                  context->aclDisplay.nNormal,
                  context->aclDisplay.advNormAcl);

   manageAclLines(context->basicNorm.scrollForm, 
                  context->aclDisplay.nNormal,
                  context->aclDisplay.basicNormAcl);

   manageAclLines(context->advNeg.scrollForm,
                  context->aclDisplay.nNegative,
                  context->aclDisplay.advNegAcl);

   manageAclLines(context->basicNeg.scrollForm,
                  context->aclDisplay.nNegative,
                  context->aclDisplay.basicNegAcl);

}  /* end manageAllAclLines */


/*--------------------
 *
 *  manageAclLines
 *
 *------------------*/

static void
#ifdef _NO_PROTO
manageAclLines(parent, nLines, aclLines)
   Widget     parent;
   int        nLines;
   AclLine  * aclLines[];
#else
manageAclLines(Widget parent, int nLines, AclLine * aclLines[])
#endif

{
   int    i;
   Widget *children;
   
                      
   XtUnmanageChildren(
     ((XmManagerWidget)parent)->composite.children,
     ((XmManagerWidget)parent)->composite.num_children);

   if (nLines > 0)
   {                
      XtUnmanageChild(parent);
      children = (Widget *)XtMalloc(nLines * sizeof(Widget *));

      /* attach first line to top of form */
      XtVaSetValues(aclLines[0]->aclForm,
                    XmNtopAttachment,    XmATTACH_FORM,
                    XmNbottomAttachment, XmATTACH_NONE,
                    NULL);
      children[0] = aclLines[0]->aclForm;
 
      /* attach each remaining ACL line to the one above it */
      for (i = 1; i < nLines; i++)
      {
         XtVaSetValues(aclLines[i]->aclForm,
                       XmNtopAttachment,    XmATTACH_WIDGET,
                       XmNtopWidget,        aclLines[i-1]->aclForm,
                       XmNbottomAttachment, XmATTACH_NONE,
                       NULL);
         children[i] = aclLines[i]->aclForm;
      }

      XtManageChildren(children, nLines); 
      
      /* This is a hack ... when new users were added to an ACL, the height */
      /* of the aclLine was set to 1. This sets the height of the last line */
      /* in the form to be the same as the penultimate line.                */
      if (nLines > 1) XtVaSetValues(aclLines[nLines-1]->aclForm, 
                                    XmNheight, XtHeight(aclLines[nLines-2]->aclForm), 
                                    NULL); 

      XtManageChild(parent);
      XtFree((char *)children);
   }
   

}  /* end manageAclLines */



/*--------------------------------------------------------------
 *
 *  checkAuth
 *
 *  Check to see if the user is authorized to change the ACL
 *    in the current dirrectory and set the sensitivity of
 *    some controls accordingly. This is called at the beginning
 *    and whenever Apply is pressed. Once you are unauthorized
 *    there is nothing you can do with the dialog to authorize
 *    yourself, so there is no reason to have code to make
 *    things sensitive again.
 *
 *-------------------------------------------------------------*/

void
#ifdef _NO_PROTO
checkAuth(context)
   DialogContext * context;
#else
checkAuth(DialogContext * context)
#endif

{
   int i;

   if( ! authToChangeAcl(context->directory))
   {
      for (i = 0; i < context->aclDisplay.nNormal; i++)
      {
         TGdesensitize(context->aclDisplay.advNormAcl[i]->TG);
         TGdesensitize(context->aclDisplay.basicNormAcl[i]->TG);
      }
      for (i = 0; i < context->aclDisplay.nNegative; i++)
      {
         TGdesensitize(context->aclDisplay.advNegAcl[i]->TG);
         TGdesensitize(context->aclDisplay.basicNegAcl[i]->TG);
      }
      XtSetSensitive  (context->okButton,    FALSE);
      XtSetSensitive  (context->applyButton, FALSE);
      XtSetSensitive  (context->resetButton, FALSE);
      XtSetSensitive  (context->advNorm.addButton, FALSE);
      XtSetSensitive  (context->advNeg.addButton, FALSE);
      XtSetSensitive  (context->basicNorm.addButton, FALSE);
      XtSetSensitive  (context->basicNeg.addButton, FALSE);
      XtVaSetValues   (context->advNorm.scroll, XmNshadowThickness, 0, NULL);
      XtVaSetValues   (context->advNeg.scroll,  XmNshadowThickness, 0, NULL);
      XtVaSetValues   (context->basicNorm.scroll, XmNshadowThickness, 0, NULL);
      XtVaSetValues   (context->basicNeg.scroll,  XmNshadowThickness, 0, NULL);
      if (context->aclDisplay.nNegative == 0)
      {
         XtUnmanageChild(aclMgr(context->advNeg));
         XtVaSetValues(aclMgr(context->advNorm),
                       XmNbottomAttachment, XmATTACH_FORM,
                       NULL);
         XtUnmanageChild(aclMgr(context->basicNeg));
         XtVaSetValues(aclMgr(context->basicNorm),
                       XmNbottomAttachment, XmATTACH_FORM,
                       NULL);
      }
   }

}  /* end checkAuth */




/*--------------------------------------------------------------
 *
 *  checkNumUserLimits
 *
 *  Check to see if there are no users or the number of users
 *    is at its maximum and change set the sensitivity of the
 *    Add... and Delete... button accordingly. The number of    
 *    displayed users is tested, not the number of users in the
 *    actual ACL.
 *
 *-------------------------------------------------------------*/

void
#ifdef _NO_PROTO
checkNumUserLimits(context)
   DialogContext * context;
#else
checkNumUserLimits(DialogContext * context)
#endif

{   
   Boolean okToAdd;

   XtSetSensitive(context->advNorm.deleteButton,   (context->aclDisplay.nNormal   > 0));
   XtSetSensitive(context->basicNorm.deleteButton, (context->aclDisplay.nNormal   > 0));
   XtSetSensitive(context->advNeg.deleteButton,    (context->aclDisplay.nNegative > 0));
   XtSetSensitive(context->basicNeg.deleteButton,  (context->aclDisplay.nNegative > 0));
       
   okToAdd = (context->aclDisplay.nNormal + context->aclDisplay.nNegative) < maxAclEntries();
   XtSetSensitive(context->advNorm.addButton,      okToAdd);
   XtSetSensitive(context->basicNorm.addButton,    okToAdd);
   XtSetSensitive(context->advNeg.addButton,       okToAdd);
   XtSetSensitive(context->basicNeg.addButton,     okToAdd);

}  /* end checkNumUserLimits */ 

/*---------------------------------------------
 *
 *  aclMgr
 *
 *  return the manager widget in an AclSkeleton
 *
 *---------------------------------------------*/

Widget
#ifdef _NO_PROTO
aclMgr(skeleton)
   AclSkeleton skeleton;
#else
aclMgr(AclSkeleton skeleton)
#endif

{

   return skeleton.frame;

}  /* end aclMgr */
