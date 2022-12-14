/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "Prompt.h"
#include "Dialog.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Text.h>

Prompt::Prompt(MotifUI *parent,
               char *name,
               boolean editable,
	       PromptType prompt_type,
	       char *default_value,
	       ValidationCallback /*CB*/,
               void * /*validation_data*/,
               boolean /*echo_input*/,
	       int n_columns,
	       int n_rows,
	       int captionWidth)
	: MotifUI(parent, name, NULL)
{
   int editMode;
   int wordWrap;
   Widget parentW;
   Pixel bg;

   parentW = parent->InnerWidget();

   _prompt_type = prompt_type;
   _value = NULL;
   if (_default_value)
      _default_value = STRDUP(default_value);
   else
      _default_value = strdup("");
   if (_prompt_type == MULTI_LINE_STRING_PROMPT)
    {
      wordWrap = true;
      editMode = XmMULTI_LINE_EDIT; 
    }
   else
    {
      wordWrap = false;
      editMode = XmSINGLE_LINE_EDIT;
    }
   XtVaGetValues(parentW, XmNbackground, &bg, NULL);
   _w = XtVaCreateManagedWidget(name, xmFormWidgetClass, parentW, NULL);
   XmString xm_string = StringCreate(name);
   _caption = XtVaCreateManagedWidget(name, xmLabelWidgetClass, _w, 
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNalignment, XmALIGNMENT_END,
				      XmNlabelString, xm_string,
				      XmNwidth, captionWidth, NULL);
   StringFree(xm_string);
   if (editMode == XmMULTI_LINE_EDIT && editable == false)
    {
      Widget _frame;
      _frame = XtVaCreateManagedWidget(name, xmFrameWidgetClass, _w, 
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNbottomAttachment, XmATTACH_FORM,
				       XmNleftAttachment,
				       (name ? XmATTACH_WIDGET : XmATTACH_FORM),
				       XmNleftWidget, _caption,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNshadowType, XmSHADOW_ETCHED_IN,
				       XmNshadowThickness, 2, NULL);
      _text = XtVaCreateManagedWidget(name, xmTextWidgetClass, _frame,
				      XmNvalue, default_value,
				      XmNrows, n_rows, XmNcolumns, n_columns,
				      XmNeditMode, editMode, 
				      XmNwordWrap, wordWrap,
				      XmNeditable, editable,
				      XmNbackground, bg,
				      XmNcursorPositionVisible, false,
				      XmNtraversalOn, false,
				      XmNshadowThickness, 0, NULL);
    }
   else
      _text = XtVaCreateManagedWidget(name, xmTextWidgetClass, _w, 
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNleftAttachment,
				       (name ? XmATTACH_WIDGET : XmATTACH_FORM),
				      XmNleftWidget, _caption,
				      XmNrightAttachment, XmATTACH_FORM,
				      XmNvalue, default_value,
				      XmNrows, n_rows, XmNcolumns, n_columns,
				      XmNeditMode, editMode, 
				      XmNwordWrap, wordWrap,
				      editable ? NULL : XmNeditable, editable,
				      XmNbackground, bg,
				      XmNcursorPositionVisible, false,
				      XmNtraversalOn, false,
				      XmNshadowThickness, 0,
				      NULL);
}

Prompt::~Prompt()
{
    delete _default_value;
    XtFree(_value);
}

void Prompt::Reset()
{
    Value(_default_value);
}

void Prompt::DefaultValue(char *value)
{
    delete _default_value;
    _default_value = STRDUP(value);
}

void Prompt::Value(int *number)
{
    XtFree(_value);
    _value = XmTextGetString(_text);
    sscanf(_value, "%d", number);
}

void Prompt::Value(float *number)
{
    XtFree(_value);
    _value = XmTextGetString(_text);
    sscanf(_value, "%f", number);
}

char * Prompt::Value()
{
    XtFree(_value);
    _value = XmTextGetString(_text);
    return _value;
}

boolean Prompt::Value(int number)
{
    char value[50];

    sprintf(value, "%d", number);
    XmTextSetString(_text, value);

    return true;
}

boolean Prompt::Value(float number)
{
    char value[50];

    sprintf(value, "%f", number);
    XmTextSetString(_text, value);

    return true;
}

boolean Prompt::Value(char *string)
{
    XmTextSetString(_text, string);

    return true;
}
