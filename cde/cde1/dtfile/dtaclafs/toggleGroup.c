/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           toggleGroup.c
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    create and manage the collections of toggle buttons used
 *                   to represent AFS access rights while the toggleGroup 
 *                   structure is not hidden, it should be treated as such
 *                   and only accessed through these routines.
 *
 *   FUNCTIONS: TGNew
 *		TGdesensitize
 *		TGgetRights
 *		TGmanager
 *		TGsetToggles
 *		toggleSet
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/


#include <Xm/Form.h>
#include <Xm/ToggleBG.h>
#include <Xm/LabelG.h>

#include "toggleGroup.h"


/*
 * Defines
 */

#define ADVREAD       0        /* indices into array of toggles */
#define ADVLOOKUP     1
#define ADVINSERT     2
#define ADVDELETE     3
#define ADVWRITE      4
#define ADVLOCK       5
#define ADVADMINISTER 6

#define BASREAD       0
#define BASWRITE      1
#define BASADMINISTER 2
#define BASADVANCED   3

/* convenience macro to test toggles ... returns Boolean */
#define toggleSet(TG,RIGHT)    (XmToggleButtonGadgetGetState((TG)->rights[(RIGHT)]))

/*---------------------------------------------------------
 *
 *  TGNew
 *
 *  Return a pointer to a new group of toggles.
 *  The states of the toggles are set to False.
 *
 *--------------------------------------------------------*/

ToggleGroup *
#ifdef _NO_PROTO
TGNew(parent, mode)
   Widget      parent;
   DialogMode  mode;
#else
TGNew(Widget parent, DialogMode mode)
#endif

{
   ToggleGroup  * myTG      = XtNew(ToggleGroup);
   XmString       noString, xs;
   int            i, nRights;
   const String   advName[] =
      {
         "TGread",
         "TGinsert",
         "TGinsert",
         "TGdelete",
         "TGwrite",
         "TGlock",
         "TGadminister"
      };
   const String basName[] =
      {
         "TGSHread",
         "TGSHwrite",
         "TGSHadminister"
      };
   const String advLabels[] =
      {
         GETMESSAGE(1, 29, "read"),
         GETMESSAGE(1, 30, "lookup"),
         GETMESSAGE(1, 31, "insert"),
         GETMESSAGE(1, 32, "delete"),
         GETMESSAGE(1, 33, "write"),
         GETMESSAGE(1, 34, "lock"),
         GETMESSAGE(1, 35, "administer")
      };
   const String basicLabels[] =
      {
         GETMESSAGE(1, 36, "read"),
         GETMESSAGE(1, 37, "write"),
         GETMESSAGE(1, 38, "administer")
      };


   noString = XmStringCreateLtoR ("", XmFONTLIST_DEFAULT_TAG);

   myTG->mode    = mode;
   nRights       = (mode == ADVANCED) ? NADVRIGHTS : NBASRIGHTS;
   myTG->manager = XtVaCreateWidget("TGform",
                                    xmFormWidgetClass, parent,
                                    XmNfractionBase,   2*nRights+1,
                                    NULL);

   for (i = 0; i < nRights; i++)
   {
      if (mode == ADVANCED)
         xs = XmStringCreateLtoR (advLabels[i], XmFONTLIST_DEFAULT_TAG);
      else
         xs = XmStringCreateLtoR (basicLabels[i], XmFONTLIST_DEFAULT_TAG);

      myTG->rights[i]  = XtVaCreateManagedWidget((mode == ADVANCED) ? advName[i] : basName[i],
                                    xmToggleButtonGadgetClass, myTG->manager,
                                    XmNbottomAttachment, XmATTACH_FORM,
                                    XmNtopAttachment,    XmATTACH_FORM,
                                    XmNlabelString,      xs,
                                    XmNalignment,        XmALIGNMENT_BEGINNING,
                                    XmNleftOffset,       10,
                                    XmNset,              FALSE,
                                    NULL);
      if (i == 0)
         XtVaSetValues(myTG->rights[i],
                       XmNleftAttachment, XmATTACH_FORM,
                       NULL);
      else
         XtVaSetValues(myTG->rights[i],
                       XmNleftAttachment, XmATTACH_WIDGET,
                       XmNleftWidget,     myTG->rights[i-1],
                       NULL);
      XmStringFree(xs);

   }

   if (mode == BASIC)
   {
      xs = XmStringCreateLtoR (GETMESSAGE(1, 10, "Use advanced mode"), XmFONTLIST_DEFAULT_TAG);
      myTG->advLabel = XtVaCreateWidget("advLabel",
                                       xmLabelGadgetClass, myTG->manager,
                                       XmNlabelString, xs,
                                       XmNalignment,         XmALIGNMENT_BEGINNING,
                                       XmNtopAttachment,     XmATTACH_FORM,
                                       XmNbottomAttachment,  XmATTACH_FORM,
                                       XmNleftAttachment,    XmATTACH_FORM,
                                       NULL);
      XmStringFree(xs);
   }

   XtManageChild(myTG->manager);

   XmStringFree(noString);

   return (myTG);

}  /* end TGNew */


/*-----------------------------------------------------------------
 *
 *  TGmanager
 *
 *  return the manager widget for the toggle group
 *
 *----------------------------------------------------------------*/

Widget
#ifdef _NO_PROTO
TGmanager (TG)
   ToggleGroup * TG;
#else
TGmanager (ToggleGroup * TG)
#endif

{

   return TG->manager;

}  /* end TGmanager */


/*------------------------------------------------------------------------------
 *
 *   TGsetToggles
 *
 *   the rights input is always the longhand rights, not the shorthand version
 *
 *-----------------------------------------------------------------------------*/
 
void
#ifdef _NO_PROTO
TGsetToggles(TG, advRights)
   ToggleGroup * TG;
   RightsL       advRights;
#else
TGsetToggles(ToggleGroup * TG, RightsL advRights)
#endif                                   

{                        

   Boolean  notify=FALSE;
   RightsS  basicRights;
   int      i;

   if (TG->mode == ADVANCED)
   {
      XmToggleButtonSetState (TG->rights[ADVREAD],       ifRead(advRights),       notify);
      XmToggleButtonSetState (TG->rights[ADVLOOKUP],     ifLookup(advRights),     notify);
      XmToggleButtonSetState (TG->rights[ADVINSERT],     ifInsert(advRights),     notify);
      XmToggleButtonSetState (TG->rights[ADVDELETE],     ifDelete(advRights),     notify);
      XmToggleButtonSetState (TG->rights[ADVWRITE],      ifWrite(advRights),      notify);
      XmToggleButtonSetState (TG->rights[ADVLOCK],       ifLock(advRights),       notify);
      XmToggleButtonSetState (TG->rights[ADVADMINISTER], ifAdminister(advRights), notify);
   }
   else
   {
      basicRights = shRights(advRights);
      if (ifSHAdvanced(basicRights))
      {
         for (i = 0; i < NBASRIGHTS; i++)
            XtUnmanageChild(TG->rights[i]);
         XtManageChild(TG->advLabel);
      }
      else
      {
         XtUnmanageChild(TG->advLabel);
         for (i = 0; i < NBASRIGHTS; i++)
            XtManageChild(TG->rights[i]);
         XmToggleButtonSetState (TG->rights[BASREAD],       ifSHRead(basicRights),       notify);
         XmToggleButtonSetState (TG->rights[BASWRITE],      ifSHWrite(basicRights),      notify);
         XmToggleButtonSetState (TG->rights[BASADMINISTER], ifSHAdminister(basicRights), notify);
      }
   }
   
}  /* end TGsetToggles */


/*---------------------------------------------------------------------
 *
 *  TGgetRights
 *
 *  always returns longhand rights, even for basic-mode toggle groups
 *  if advanced mode is set for basic rights, return -1
 *
 *--------------------------------------------------------------------*/

RightsL
#ifdef _NO_NOPROTO
TGgetRights(TG)
   ToggleGroup * TG;
#else
TGgetRights(ToggleGroup *TG)
#endif

{
   RightsL  myRights = 0;

   if (TG->mode == ADVANCED)
   {
      if(toggleSet(TG,ADVREAD))        setRead(myRights);
      if(toggleSet(TG,ADVLOOKUP))      setLookup(myRights);
      if(toggleSet(TG,ADVINSERT))      setInsert(myRights);
      if(toggleSet(TG,ADVDELETE))      setDelete(myRights);
      if(toggleSet(TG,ADVWRITE))       setWrite(myRights);
      if(toggleSet(TG,ADVLOCK))        setLock(myRights);
      if(toggleSet(TG,ADVADMINISTER))  setAdminister(myRights);
      return (myRights);
   }
   else if ( ! XtIsManaged(TG->advLabel))
   {
      if(toggleSet(TG,BASREAD))        setSHRead(myRights);
      if(toggleSet(TG,BASWRITE))       setSHWrite(myRights);
      if(toggleSet(TG,BASADMINISTER))  setSHAdminister(myRights);
      return (lhRights(myRights));
   }
   else
      return (-1);


}  /* end TGgetRights */


/*--------------------
 *
 *  TGdesensitize
 *
 *------------------*/

void
#ifdef _NO_PROTO
TGdesensitize(TG)
   ToggleGroup * TG;
#else
TGdesensitize(ToggleGroup * TG)
#endif

{
   int i, nRights;

   nRights = (TG->mode == ADVANCED) ? NADVRIGHTS : NBASRIGHTS;
   for (i = 0; i < nRights; i++)
      XtSetSensitive(TG->rights[i],FALSE);

}  /* end TGdesensitize */
