/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "DtProps.h"
#include "DtMainW.h"
#include "Button.h"
#include "Prompt.h"
#include "LabelObj.h"
#include "Container.h"
#include "BaseObj.h"

#include "dtprintinfomsg.h"

#include <stdio.h>

DtProps::DtProps(AnyUI *parent,
		 char *name,
		 char *location_id,
                 boolean editable,
		 int n_attributes,
		 Attribute **attributes)
	: Dialog(parent, name)
{
   mainw = (DtMainW *) parent;
   _location_id = location_id;
   _has_been_posted = false;
   rc = new Container(this, "rc", VERTICAL_ROW_COLUMN);
   int i, captionWidth = 0, width, columns = 0;
   for (i = 0; i < n_attributes; i++)
    {
      if ((width = StringWidth(attributes[i]->DisplayName)) > captionWidth)
	 captionWidth = width;
      if ((width = strlen(attributes[i]->DisplayValue)) > columns)
	 columns = width;
    }

   for (i = 0; i < n_attributes; i++)
    {
      boolean _editable;
      if (EditableAfterCreate(attributes[i]))
	 _editable = true;
      else
	 _editable = false;
      if (attributes[i]->ValueListType == INFORMATION_LINE)
         new LabelObj(rc, attributes[i]->DisplayName);
      else
         new Prompt(rc, attributes[i]->DisplayName, _editable, STRING_PROMPT,
		    attributes[i]->DisplayValue, NULL,
		    NULL, true, columns, 1, captionWidth + 8);
    }

   ok = new Button(this, MESSAGE(OKL), PUSH_BUTTON, OkCB, this);
   if (editable)
    {
      cancel = new Button(this, MESSAGE(CancelL), PUSH_BUTTON, CancelCB, this);
      CancelButton(cancel);
    }
   else
      CancelButton(ok);
   help = new Button(this, MESSAGE(HelpL), PUSH_BUTTON, HelpCB, this);
   rc->AttachAll();
   DefaultButton(ok);
}

DtProps::~DtProps()
{
   // Empty
}

boolean DtProps::SetVisiblity(boolean flag)
{
   if (_has_been_posted == false)
    {
      Dialog::SetVisiblity(flag);
      Refresh();
      int width = StringWidth(Name()) + 35;
      if (Width() < width)
       {
         Dialog::SetVisiblity(flag);
	 Width(width);
       }
      _has_been_posted = true;
    }
   Dialog::SetVisiblity(flag);
   return true;
}

void DtProps::Reset()
{
}

void DtProps::Apply()
{
}

void DtProps::CloseCB()
{
   Reset();
   Visible(false);
}

void DtProps::OkCB(void *data)
{
   DtProps *obj = (DtProps *) data;
   
   obj->Apply();
   obj->Visible(false);
}

void DtProps::ApplyCB(void *data)
{
   DtProps *obj = (DtProps *) data;
   obj->Apply();
}

void DtProps::CancelCB(void *data)
{
   DtProps *obj = (DtProps *) data;
   
   obj->Reset();
   obj->Visible(false);
}

void DtProps::ResetCB(void *data)
{
   DtProps *obj = (DtProps *) data;
   
   obj->Reset();
}

void DtProps::HelpCB(void *data)
{
   DtProps *obj = (DtProps *) data;
   obj->HandleHelpRequest();
}

boolean DtProps::HandleHelpRequest()
{
   mainw->DisplayHelp(_location_id);
   return true;
}
