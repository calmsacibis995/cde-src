#include <ctype.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Dt/SpinBox.h>
#include <Xm/ScrollBar.h>
#include <Xm/TextF.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/Scale.h>
#include <SDtMail/Sdtmail.hh>
#include <DtVirtArray.hh>
#include <DtMail/options_util.h>

#define	OK	0
#define ERROR	-1

SdmBoolean props_changed;

void
options_checkbox_init(
    Widget		checkbox,
    Boolean		*dirty_bit
)
{
    XtVaSetValues(checkbox, XmNuserData,(XtArgVal)OPTIONS_EDIT , NULL);
    XtAddCallback(checkbox, XmNvalueChangedCallback,
		  options_setting_chgCB, (XtPointer)dirty_bit);
}

int
options_checkbox_set_value(
    Widget		checkbox,
    Boolean		b_value,
    Boolean		set_dirty_bit
)
{
  if (!set_dirty_bit)
    /* Set state so dirty-bit is not triggered */
    XtVaSetValues(checkbox, XmNuserData, (XtArgVal)OPTIONS_LOAD, NULL);

  XmToggleButtonSetState(checkbox, b_value, set_dirty_bit? TRUE : FALSE);
  
  if (!set_dirty_bit)
    /* Reset state */
    XtVaSetValues(checkbox, XmNuserData, (XtArgVal)OPTIONS_EDIT, NULL);

return OK;
}

Boolean
options_checkbox_get_value(Widget checkbox)
{

  return((Boolean)XmToggleButtonGetState(checkbox));

}

void
options_radiobox_init(
    Widget		radiobox,
    int			num_items,
    WidgetList		items,
    int			*item_values,
    Boolean		*dirty_bit
)
{
    int i;

    for (i=0; i < num_items; i++)
    {
	XtVaSetValues(items[i], XmNuserData, (XtArgVal)item_values[i], NULL);
	XtAddCallback(items[i], XmNvalueChangedCallback, 
			options_setting_chgCB, (XtPointer)dirty_bit);
	XtAddCallback(items[i], XmNvalueChangedCallback, 
			options_radiobox_itemCB, (XtPointer)item_values[i]);

	/* Ensure Radiobox has a default Value set */
	if (i == 0)
	{
	    XtVaSetValues(items[i], XmNset, True, NULL);
	    XtVaSetValues(radiobox, XmNuserData, (XtArgVal)item_values[i], NULL);
	}
    }

}

int
options_radiobox_get_value(
    Widget	radiobox
)
{
    XtArgVal value;

    XtVaGetValues(radiobox, XmNuserData, &value, NULL);

    return((int)value);
}

int
options_radiobox_set_value(
    Widget	     radiobox,
    int        	     value,
    Boolean	     set_dirty_bit
)
{
    int         num_children = 0;
    WidgetList  children = NULL;
    XtArgVal    childval = 0;
    Boolean     found = FALSE;
    int         i = 0;

    set_dirty_bit = set_dirty_bit;
    XtVaGetValues(radiobox,
        XtNnumChildren,    &num_children,
        XtNchildren,       &children,
        NULL);

    for (i = 0; i < num_children; i++)
    {
        XtVaGetValues(children[i], XmNuserData, &childval, NULL);
        XmToggleButtonSetState(children[i], childval == value? TRUE : FALSE, FALSE);
        if (childval == value)
        {
            found = TRUE;
            XtVaSetValues(radiobox, XmNuserData, (XtArgVal)value, NULL);
        }
    }    
    if (!found)
        return ERROR;
    
    return OK;
}

/*
 * Callback: choice value has changed...turnon dirty bit
 */
void
options_setting_chgCB(
     Widget,
     XtPointer clientdata,
     XtPointer
)
{
     Boolean	*dirty_bit = (Boolean *)clientdata;

     *dirty_bit = TRUE;
     props_changed = Sdm_True;
}

/*
 * Callback: item in exclusive choice has been set...store it's
 *	     value in the parent radiobox
 */
void
options_radiobox_itemCB(
    Widget item,
    XtPointer clientdata,
    XtPointer calldata 
)
{
    XmToggleButtonCallbackStruct *state = (XmToggleButtonCallbackStruct*)calldata;
    int    value = (int)clientdata;
    Widget excl_setting;

    if (state->set)
    {
    	excl_setting = XtParent(item);
    	XtVaSetValues(excl_setting, XmNuserData, (XtArgVal)value, NULL);
    }

}

void
options_field_init(
    Widget		field,
    Boolean		*dirty_bit)
{

    XtAddCallback(field, XmNvalueChangedCallback, 
		options_field_chgCB, (XtPointer)dirty_bit);

    XtVaSetValues(field, XmNuserData, (XtArgVal)OPTIONS_EDIT, NULL);

}

/* NOTE: caller must free memory allocated by XmTextFieldGetString() */
String 
options_field_get_value( 
    Widget    		field
) 
{ 
    String      value; 
 
    value = XmTextFieldGetString(field); 
 
    return value; 
} 

int
options_field_set_value(
    Widget		field,
    String		value,
    Boolean		set_dirty_bit
)
{
    if (!set_dirty_bit)
    	/* Set state so dirty-bit is not triggered */
    	XtVaSetValues(field, XmNuserData, (XtArgVal)OPTIONS_LOAD, NULL);

    /* Set field value */
    XmTextFieldSetString(field, value);

    if (!set_dirty_bit)
    	/* Reset state */
   	XtVaSetValues(field, XmNuserData, (XtArgVal)OPTIONS_EDIT, NULL);

    return OK;

}

void
options_field_chgCB( 
     Widget widget,
     XtPointer clientdata,
     XtPointer calldata
)
{
     Boolean     *dirty_bit = (Boolean *)clientdata;
     XtArgVal    field_mode;

     calldata = calldata;
     XtVaGetValues(widget, XmNuserData, &field_mode, NULL);
     if (field_mode == OPTIONS_EDIT)
	*dirty_bit = TRUE;
     props_changed = Sdm_True;

}
static Boolean
all_digits(char *string) {

        while (*string) {
                if (!isdigit(*string++))
                        return(False);
        }

        return(True);
}

void
options_spinbox_chgCB( 
     Widget widget,
     XtPointer clientdata,
     XtPointer calldata
)
{
     Widget  spinbox = (Widget) clientdata;
     char *textstr = XmTextGetString(widget);
     int             mvalue, value = strtol(textstr, NULL, 10);
     Display         *dpy = XtDisplayOfObject(widget);
     calldata = calldata;
     
     props_changed = Sdm_True;
     
     if (!all_digits(textstr)){
	// We have set it to something other than 1 before setting 
	// the 1 to get around a spinbox bug. Otherwise the textfield
 	// will not clear the invalid entry. Yuk!
	XtVaSetValues(spinbox, DtNposition, 2, NULL);
	XtVaSetValues(spinbox, DtNposition, 1, NULL);
	XBell(dpy, 50);
     	XtFree(textstr);
	return;
     }
     XtVaGetValues(spinbox, DtNmaximumValue, &mvalue, NULL); 
     if (value > mvalue) {
	XtVaSetValues(spinbox, DtNposition, 2, NULL);
	XtVaSetValues(spinbox, DtNposition, mvalue, NULL);
	XBell(dpy, 50);
     	XtFree(textstr);
	return;
     }
     // It could never be less than the minimum value because a 
     // '-' is an invalid entry and it would be cleared and reset
     // if entered. So if we made it here it is a valid entry. We
     // also have to reset the insertion point! Another spinbox bug
     // we have to work around. 
     XtVaSetValues(spinbox, DtNposition, value, NULL);
     int shift = 1;
     while ((value /= 10) > 0)
	shift++; 
     XmTextSetInsertionPosition(widget, shift);
     XtFree(textstr);
}

void
options_spinbox_init(
    Widget		spinbox,
    Boolean		*dirty_bit
)
{
    Widget textfield;
    dirty_bit = dirty_bit;

    XtVaGetValues(spinbox, DtNtextField, &textfield, NULL);
    XtAddCallback(textfield, XmNvalueChangedCallback, 
		options_spinbox_chgCB, spinbox);

    XtVaSetValues(spinbox, XmNuserData, (XtArgVal)OPTIONS_EDIT, NULL);

}


int  
options_spinbox_get_value( 
    Widget    		spinbox
) 
{ 
    int      value; 
 
    XtVaGetValues(spinbox,
		  DtNposition, &value,
		  NULL);

    return value; 
} 

int
options_spinbox_set_value(
    Widget		spinbox,
    int			value,
    Boolean		set_dirty_bit
)
{
    if (!set_dirty_bit)
      /* Set state so dirty-bit is not triggered */
      XtVaSetValues(spinbox, XmNuserData, (XtArgVal)OPTIONS_LOAD, NULL);

    /* Set spinbox value */
    XtVaSetValues(spinbox, DtNposition, value, NULL);

    if (!set_dirty_bit)
    	/* Reset state */
   	XtVaSetValues(spinbox, XmNuserData, (XtArgVal)OPTIONS_EDIT, NULL);

    return OK;

}
void
options_list_init(Widget list,
		  DtVirtArray<char *> *item_list
		  )
{
  int 		num_items;

  int 		i;
  XmStringTable str_list;

  if(item_list == NULL)
    return;

  num_items = item_list->length();

  str_list = (XmStringTable)XtMalloc(num_items * sizeof(XmString));

  for(i = 0; i < num_items; i++)
    str_list[i] = XmStringCreateLocalized((*item_list)[i]);

  XtVaSetValues(list,
		XmNitemCount, num_items,
		XmNitems, str_list,
		NULL);

  for(i = 0; i < num_items; i++)
    XmStringFree(str_list[i]);
  XtFree((char *)str_list);
  
}

void
options_list_init(Widget list,
		  SdmStringL *item_list
		  )
{
  int 		num_items;

  int 		i;
  XmStringTable str_list;

  if(item_list == NULL)
    return;

  num_items = item_list->ElementCount();

  str_list = (XmStringTable)XtMalloc(num_items * sizeof(XmString));

  for(i = 0; i < num_items; i++) {
    const char *tmpStr = (*item_list)[i];
    str_list[i] = XmStringCreateLocalized((char *)tmpStr);
  }

  XtVaSetValues(list,
		XmNitemCount, num_items,
		XmNitems, str_list,
		NULL);

  for(i = 0; i < num_items; i++)
    XmStringFree(str_list[i]);
  XtFree((char *)str_list);
  
}

void listAddButtonCB(
     Widget widget,
     XtPointer clientdata,
     XtPointer calldata
)
{
  ListUiItem *list = (ListUiItem *)clientdata;
  calldata = calldata;
  widget = widget;

  list->handleAddButtonPress();

}

void listDeleteButtonCB(
     Widget widget,
     XtPointer clientdata,
     XtPointer calldata
)
{
  ListUiItem *list = (ListUiItem *)clientdata;
  calldata = calldata;
  widget = widget;

  list->handleDeleteButtonPress();

}

void listChangeButtonCB(
     Widget widget,
     XtPointer clientdata,
     XtPointer calldata
)
{
  ListUiItem *list = (ListUiItem *)clientdata;
  calldata = calldata;
  widget = widget;

  list->handleChangeButtonPress();

}

void add_cbs_to_list(ListUiItem *list,
		     Widget add_butt,
		     Widget del_butt,
		     Widget chg_butt)
{
  XtAddCallback(add_butt,
		XmNactivateCallback,
		listAddButtonCB,
		list);

  XtAddCallback(del_butt,
		XmNactivateCallback,
		listDeleteButtonCB,
		list);

  XtAddCallback(chg_butt,
		XmNactivateCallback,
		listChangeButtonCB,
		list);
		
}

int 
options_scale_get_value( 
    Widget    		scale
) 
{ 
    int      value; 
 
    XtVaGetValues(scale, XmNvalue, &value, NULL);
 
    return value; 
} 

int
options_scale_set_value(
    Widget		scale,
    char		*value,
    Boolean		set_dirty_bit
)
{
    int val_int;

    if (!set_dirty_bit)
    	/* Set state so dirty-bit is not triggered */
    	XtVaSetValues(scale, XmNuserData, (XtArgVal)OPTIONS_LOAD, NULL);

    val_int = strtol(value, NULL, 10);
    /* Set scale value */

    XtVaSetValues(scale,  XmNvalue, val_int, NULL);

    if (!set_dirty_bit)
    	/* Reset state */
   	XtVaSetValues(scale, XmNuserData, (XtArgVal)OPTIONS_EDIT, NULL);

    return OK;

}
