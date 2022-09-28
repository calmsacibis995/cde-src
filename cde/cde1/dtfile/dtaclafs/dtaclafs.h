/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           dtaclafs.h
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    header file for dtaclafs.c ... Motif program to
 *                   display and edit AFS Access Control Lists.
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef DTACLAFS_H
#define DTACLAFS_H


/*-----------------
 * Include Section
 *-----------------*/

#include <Xm/Xm.h>

#include "toggleGroup.h"


/*-----------------
 * Defines Section
 *-----------------*/

/* macro to get message catalog string */
#ifdef MESSAGE_CAT
#ifdef __ultrix
#define _CLIENT_CAT_NAME "dtaclafs.cat"
#else  /* __ultrix */
#define _CLIENT_CAT_NAME "dtaclafs"
#endif /* __ultrix */
#define GETMESSAGE(set, number, string)\
    ((char *) _DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */


/*-----------------
 * Typedef Section
 *-----------------*/

typedef enum {NONE, YES, NO} Answer;   /* used for yesOrNo() */

typedef enum {ADVNORM, BASICNORM, ADVNEG, BASICNEG} SkeletonType;

/* a Motif form with label and toggles for one user/group */
typedef struct {
   Widget         aclForm;
   Widget         name;
   ToggleGroup  * TG;
} AclLine;  

/* Widgets representing all users/groups in an ACL */
typedef struct {
   DialogMode  currentMode;
   int         nNormal, nNegative;
   AclLine   * advNormAcl[ACL_MAXENTRIES];
   AclLine   * advNegAcl[ACL_MAXENTRIES];
   AclLine   * basicNormAcl[ACL_MAXENTRIES];
   AclLine   * basicNegAcl[ACL_MAXENTRIES];
} AclDisplay;

/* skeleton in which an ACL is displayed */
typedef struct {
   SkeletonType  type;
   Widget        frame;
   Widget        title;
   Widget        form;
   Widget        scroll;
   Widget        scrollForm;
   Widget        addButton;
   Widget        deleteButton;
} AclSkeleton;


/* Dialog context structure */
typedef struct {

   char     directory[FILENAME_MAX];

   Boolean  showNeg;   /* TRUE ==> show neg've permissions, FALSE ==> show a summary */

   /* widgets which do not depend on the ACL */
   /* note that the shell widget is not here */
   /*    but is stored in a global variable  */
   Widget   outerFrame;
   Widget   outerForm;
   Widget   fileName;
   Widget   fileNameSeparator;
   Widget   folderName;
   Widget   warningMessage;
   Widget   warningSeparator;

   /* widgets for setting options */
   Widget   modeRadio;
   Widget   modeRadioTitle;
   Widget   advancedToggle;
   Widget   basicToggle;

   Widget   showNegRadio;
   Widget   showNegRadioTitle;
   Widget   showNegToggle;
   Widget   hideNegToggle;

   /* widget for summary of number of entries in negative ACL */
   Widget   summary;

   /* Acl widgets present in both advanced and basic mode */
   Widget   aclForm;

   /* widgets to display ACL */
   AclSkeleton    advNorm;
   AclSkeleton    advNeg;
   AclSkeleton    basicNorm;
   AclSkeleton    basicNeg;
   AclDisplay     aclDisplay;

   /* action area widgets */
   Widget   actionSeparator;
   Widget   actionForm;
   Widget   okButton;
   Widget   applyButton;
   Widget   cancelButton;
   Widget   resetButton;
   Widget   optionsButton;
   Widget   helpButton;

   /* widgets for add (users/groups) dialog */
   Widget   addDialogShell;
   Widget   addForm;
   Widget   addActionForm;
   Widget   addActionSeparator;
   Widget   addOkButton;
   Widget   addCancelButton;
   Widget   addHelpButton;
   Widget   addWorkForm;
   Widget   addNewuserText;
   ToggleGroup  * addAdvNewuserTG;
   ToggleGroup  * addBasNewuserTG;
   Boolean  addNormalAcl;   /* was add pushed for normal ACL as opposed to negative? */


   /* widgets for delete (users/groups) dialog */
   Widget   deleteDialogShell;
   Widget   deleteForm;
   Widget   deleteActionForm;
   Widget   deleteActionSeparator;
   Widget   deleteOkButton;
   Widget   deleteCancelButton;
   Widget   deleteHelpButton;
   Widget   deleteList;
   Widget   deleteInstructions;
   Widget   deleteInstSeparator;
   Boolean  deleteNormalAcl;   /* was delete pushed for normal ACL as opposed to negative? */

   /* file state information (afs acl state only, not Unix state) */
   AclDef   initialAcl;      /* upon entry or after an apply action */
   AclDef   displayedAcl;    /* updated after certain actions by user */

} DialogContext;



/* -----------------------------
 * Function Declaration Section
 *------------------------------*/

#ifdef _NO_PROTO
/* old-style prototypes from dtaclafs.c */
extern void   displayError();
extern void   displayInfo();
extern void   displayWarning();
extern Answer yesOrNo();

/* old-style prototypes from callbacks.c */
extern void modeCB();
extern void cancelCB();
extern void resetCB();
extern void applyCB();
extern void destroyCB();
extern void showNegCB();
extern void addCB();
extern void addCancelCB();
extern void addOkCB();
extern void deleteCB();
extern void deleteCancelCB();
extern void deleteSelectCB();
extern void deleteOkCB();
extern void questionDialogCB();

/* old-style prototypes from create.c */
extern void       createDialog();
extern AclLine  * newAclLine();  

/* old-style prototypes from setval.c */
extern void    setValues();
extern void    setDisplayedAcl();
extern void    checkAuth();
extern Widget  aclMgr();
extern void    setAddDialogMode();
extern void    setNegSummary();

#else
/* new-style prototypes from dtaclafs.c */
extern void   displayError(const String s);
extern void   displayInfo(const String s);
extern void   displayWarning(const String s);
extern Answer yesOrNo(const String s);

/* new-style prototypes from callbacks.c */
extern void modeCB           (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void cancelCB         (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void resetCB          (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void applyCB          (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void destroyCB        (Widget w, Widget dialog,          XmAnyCallbackStruct *cbs);
extern void showNegCB        (Widget w, Widget dialog,          XmAnyCallbackStruct *cbs);
extern void addCB            (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void addCancelCB      (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void addOkCB          (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void deleteCB         (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void deleteCancelCB   (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void deleteSelectCB   (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void deleteOkCB       (Widget w, DialogContext *context, XmAnyCallbackStruct *cbs);
extern void questionDialogCB (Widget w, Answer, answerButton,   XmAnyCallbackStruct *cbs);

/* new-style prototypes from create.c */
extern void       createDialog  (DialogContext *context);
extern AclLine  * newAclLine    (Widget parent);

/* new-style prototypes from setval.c */
extern void    setValues        (const String     path,
                                 const String     directory,
                                 DialogContext  * context,
                                 const AclDef   * fileState);
extern void    setDisplayedAcl  (DialogContext  * context,
                                 const  AclDef  * fileState);
extern void    displayWarning   (Widget parent);
extern void    checkAuth        (DialogContext * context);
extern Widget  aclMgr           (AclSkeleton skeleton);
extern void    setAddDialogMode (DialogContext  * context);
extern void    setNegSummary    (DialogContext  * context, int nNeg);

#endif


#endif /* DTACLAFS_H */
