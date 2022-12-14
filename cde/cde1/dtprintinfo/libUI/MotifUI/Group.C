/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "Group.h"
#include "Dialog.h"

#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>

Group::Group(MotifUI *parent,
	     char *name,
	     GroupType group_type)
	: MotifUI(parent, name, NULL)
{
   Widget parentW;

   _group_type = group_type;

   parentW = parent->InnerWidget();
   XmString xm_string = StringCreate(name);

   _w = XtVaCreateManagedWidget(name, xmFrameWidgetClass, parentW, NULL);
   if (name)
      _label = XtVaCreateManagedWidget(name, xmLabelWidgetClass, _w, 
				       XmNchildType, XmFRAME_TITLE_CHILD,
				       XmNlabelString, xm_string, NULL);
   else
      _label = XtVaCreateWidget(name, xmLabelWidgetClass, _w, 
				XmNchildType, XmFRAME_TITLE_CHILD,
				XmNlabelString, xm_string, NULL);
   if (group_type == FORM_BOX)
      _rc = XtVaCreateManagedWidget(name, xmFormWidgetClass, _w, NULL);
   else
    {
      boolean flag;
      int orientaion;
      if (group_type == RADIO_GROUP || group_type == HORIZONTAL_RADIO_GROUP)
         flag = true;
      else
         flag = false;
      if (group_type == HORIZONTAL_CHECK_BOX ||
	  group_type == HORIZONTAL_RADIO_GROUP)
         orientaion = XmHORIZONTAL;
      else
         orientaion = XmVERTICAL;
      _rc = XtVaCreateManagedWidget(name, xmRowColumnWidgetClass, _w, 
				    XmNradioBehavior, flag,
				    XmNorientation, orientaion, NULL);
    }
   StringFree(xm_string);
}

boolean Group::SetName(char *name)
{
   if (name)
    {
      XmString xm_string = StringCreate(name);
      XtVaSetValues(_label, XmNlabelString, xm_string, NULL);
      StringFree(xm_string);
      XtManageChild(_label);
    }
   else
      XtUnmanageChild(_label);

   return true;
}
