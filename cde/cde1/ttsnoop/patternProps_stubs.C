/*** DTB_USER_CODE_START vvv Add file header below vvv ***/
//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
//%%  (c) Copyright 1993, 1994 Novell, Inc.
//%%  $Revision: $
/*** DTB_USER_CODE_END   ^^^ Add file header above ^^^ ***/

/*
 * File: patternProps_stubs.c
 * Contains: Module callbacks and connection functions
 *
 * This file was generated by dtcodegen, from module patternProps
 *
 * Any text may be added between the DTB_USER_CODE_START and
 * DTB_USER_CODE_END comments (even non-C code). Descriptive comments
 * are provided only as an aid.
 *
 *  ** EDIT ONLY WITHIN SECTIONS MARKED WITH DTB_USER_CODE COMMENTS.  **
 *  ** ALL OTHER MODIFICATIONS WILL BE OVERWRITTEN. DO NOT MODIFY OR  **
 *  ** DELETE THE GENERATED COMMENTS!                                 **
 */

#include <stdio.h>
#include <Xm/Xm.h>
#include "dtb_utils.h"
#include "patternProps_ui.h"

/*
 * Header files for cross-module connections
 */
#include "ttsnoop_ui.h"


/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** All necessary header files have been included.
 ***
 *** Add include files, types, macros, externs, and user functions here.
 ***/

#include <strstream.h>
#include <Tt/tttk.h>
#include "DtTt.h"

const int		DtTtPatWidgetKey	= (int)DtTtPatternWidget;
const int		DtTtPatInfoKey		= DtTtPatWidgetKey + 1;

Tt_pattern
patternProps2Pat(
	DtbPatternPropsPatternPropsInfo instance
)
{
	Tt_pattern pat;
	XtVaGetValues( instance->patternProps_shellform,
		       XmNuserData, &pat, 0 );
	return pat;
}

static Boolean
_DtTtPatternUpdating(
	Tt_pattern pat
)
{
	void *updating = tt_pattern_user( pat, (int)_DtTtPatternUpdating );
	if (tt_is_err( tt_ptr_error( updating ))) {
		return False;
	}
	return updating == (void *)True;
}

static void
_DtTtPatternSetUpdating(
	Tt_pattern			pat,
	Boolean				updating
)
{
	tt_pattern_user_set( pat, (int)_DtTtPatternUpdating,
			     (void *)updating );
}

void
_DtTtPatternWidgetUpdate(
	DtbPatternPropsPatternPropsInfo	instance,
	Tt_pattern			pat
)
{
	char *printout = tt_pattern_print( pat ); // XXX when to free?
	Tt_status status = tt_ptr_error( printout );
	if (tt_is_err( status )) {
		printout = tt_status_message( status );
	}
	XtVaSetValues( instance->patternText, XmNvalue, printout, 0 );

	_DtTtPatternSetUpdating( pat, True );

	Widget choice;
	switch (tt_pattern_category( pat )) {
	    case TT_CATEGORY_UNDEFINED:
	    default:
		choice = instance->categoryChoice_items.UNDEFINED_item;
		break;
	    case TT_OBSERVE:
		choice = instance->categoryChoice_items.OBSERVE_item;
		break;
	    case TT_HANDLE:
		choice = instance->categoryChoice_items.HANDLE_item;
		break;
	    case TT_HANDLE_PUSH:
		choice = instance->categoryChoice_items.HANDLE_PUSH_item;
		break;
	    case TT_HANDLE_ROTATE:
		choice = instance->categoryChoice_items.HANDLE_ROTATE_item;
		break;
	}
	XtVaSetValues( instance->categoryChoice, XmNmenuHistory, choice, 0 );

	_DtTtPatternSetUpdating( pat, False );
}

void
_DtTtPatternUpdate(
	Tt_pattern			pat,
	_DtStringChooserAction		choice,
	char *				string
)
{
	DtbPatternPropsPatternPropsInfo	instance;
	instance = (DtbPatternPropsPatternPropsInfo)
		tt_pattern_user( pat, DtTtPatInfoKey );
	Tt_status status = tt_ptr_error( instance );
	if (tt_is_err( status ) || (instance == 0)) {
		return;
	}
	char *func;
	switch (choice) {
	    case _DtStringChoosePatternOp:
		status = tt_pattern_op_add( pat, string );
		func = "tt_pattern_op_add()";
		break;
	    case _DtStringChoosePatternOtype:
		status = tt_pattern_otype_add( pat, string );
		func = "tt_pattern_otype_add()";
		break;
	    case _DtStringChoosePatternObject:
		status = tt_pattern_object_add( pat, string );
		func = "tt_pattern_object_add()";
		break;
	    case _DtStringChoosePatternSenderPtype:
		status = tt_pattern_sender_ptype_add( pat, string );
		func = "tt_pattern_sender_ptype_add()";
		break;
	    default:
		return;
	}
	DtTtSetLabel( instance->patternProps_label, func, status );
	_DtTtPatternWidgetUpdate( instance, pat );
	return;
}

void
_DtTtPatternUpdate(
	Tt_pattern			pat,
	_DtSessionChooserAction		choice,
	char *				session
)
{
	DtbPatternPropsPatternPropsInfo	instance;
	instance = (DtbPatternPropsPatternPropsInfo)
		tt_pattern_user( pat, DtTtPatInfoKey );
	Tt_status status = tt_ptr_error( instance );
	if (tt_is_err( status ) || (instance == 0)) {
		return;
	}
	char *func;
	switch (choice) {
	    case _DtSessionChoosePattern:
		status = tt_pattern_session_add( pat, session );
		func = "tt_pattern_session_add()";
		break;
	    default:
		return;
	}
	DtTtSetLabel( instance->patternProps_label, func, status );
	_DtTtPatternWidgetUpdate( instance, pat );
	return;
}

void
_DtTtPatternUpdate(
	Tt_pattern			pat,
	_DtFileChooserAction		choice,
	char *				file
)
{
	DtbPatternPropsPatternPropsInfo	instance;
	instance = (DtbPatternPropsPatternPropsInfo)
		tt_pattern_user( pat, DtTtPatInfoKey );
	Tt_status status = tt_ptr_error( instance );
	if (tt_is_err( status ) || (instance == 0)) {
		return;
	}
	char *func;
	switch (choice) {
	    case _DtFileChoosePatternAdd:
		status = tt_pattern_file_add( pat, file );
		func = "tt_pattern_file_add()";
		break;
	    default:
		return;
	}
	DtTtSetLabel( instance->patternProps_label, func, status );
	_DtTtPatternWidgetUpdate( instance, pat );
	return;
}

void
_DtTtPatternUpdate(
	Tt_pattern			pat,
	_DtTtChooserAction		choice,
	char *				string
)
{
	DtbPatternPropsPatternPropsInfo	instance;
	instance = (DtbPatternPropsPatternPropsInfo)
		tt_pattern_user( pat, DtTtPatInfoKey );
	Tt_status status = tt_ptr_error( instance );
	if (tt_is_err( status ) || (instance == 0)) {
		return;
	}
	char *func;
	switch (choice) {
	    case _DtTtChooserPatternOpAdd:
		status = tt_pattern_op_add( pat, string );
		func = "tt_pattern_op_add()";
		break;
	    case _DtTtChooserPatternSenderAdd:
		status = tt_pattern_sender_add( pat, string );
		func = "tt_pattern_sender_add()";
		break;
	    default:
		return;
	}
	DtTtSetLabel( instance->patternProps_label, func, status );
	_DtTtPatternWidgetUpdate( instance, pat );
	return;
}

void
_DtTtPatternUpdate(
	Tt_pattern		pat,
	_DtArgChooserAction	choice,
	Tt_mode			mode,
	char *			vtype,
	char *			val,
	int			ival
)
{
	DtbPatternPropsPatternPropsInfo	instance;
	instance = (DtbPatternPropsPatternPropsInfo)
		tt_pattern_user( pat, DtTtPatInfoKey );
	Tt_status status = tt_ptr_error( instance );
	if (tt_is_err( status ) || (instance == 0)) {
		return;
	}
	char *func;
	switch (choice) {
	    case _DtArgChoosePatternArgAdd:
		status = tt_pattern_arg_add( pat, mode, vtype, val );
		func = "tt_pattern_arg_add()";
		break;
	    case _DtArgChoosePatternIArgAdd:
		status = tt_pattern_iarg_add( pat, mode, vtype, ival );
		func = "tt_pattern_iarg_add()";
		break;
	    case _DtArgChoosePatternContextAdd:
		status = tt_pattern_context_add( pat, vtype, val );
		func = "tt_pattern_context_set()";
		break;
	    case _DtArgChoosePatternIContextAdd:
		status = tt_pattern_icontext_add( pat, vtype, ival );
		func = "tt_pattern_icontext_add()";
		break;
	    default:
		return;
	}
	DtTtSetLabel( instance->patternProps_label, func, status );
	_DtTtPatternWidgetUpdate( instance, pat );
	return;
}

void
_DtTtPatternUpdate(
	Tt_pattern			pat,
	Tt_message_callback		cb
)
{
	DtbPatternPropsPatternPropsInfo	instance;
	instance = (DtbPatternPropsPatternPropsInfo)
		tt_pattern_user( pat, DtTtPatInfoKey );
	Tt_status status = tt_ptr_error( instance );
	if (tt_is_err( status ) || (instance == 0)) {
		return;
	}
	status = tt_pattern_callback_add( pat, cb );
	DtTtSetLabel( instance->patternProps_label,
		      "tt_pattern_callback_add()", status );
	return;
}

Widget
DtTtPatternWidget(
	Tt_pattern pat
)
{
	// Function address is cast to int and used as clientdata key
	return (Widget)tt_pattern_user( pat, DtTtPatWidgetKey );
}

Widget
DtTtPatternWidgetCreate(
	Widget				parent,
	Tt_pattern			pat,
	Tt_message_callback		callback
)
{
	DtbPatternPropsPatternPropsInfoRec *instance;
	Widget widget = DtTtPatternWidget( pat );
	Tt_status status = tt_ptr_error( widget );
	if (tt_is_err( status ) || (widget != 0)) {
		instance = (DtbPatternPropsPatternPropsInfoRec *)
			tt_pattern_user( pat, DtTtPatInfoKey );
		status = tt_ptr_error( instance );
		if (tt_is_err( status ) || (instance != 0)) {
			XtManageChild( instance->patternProps_shellform );
		}
		return widget;
	}
	instance = new DtbPatternPropsPatternPropsInfoRec;
	if (instance == 0) {
		return (Widget)tt_error_pointer( TT_ERR_NOMEM );
	}
	// XXX delete instance in widget destroy proc
	status = tt_pattern_user_set( pat, DtTtPatInfoKey, instance );
	if (tt_is_err( status )) {
		delete instance;
		return (Widget)tt_error_pointer( status );
	}
	dtbPatternPropsPatternPropsInfo_clear( instance );
	dtb_pattern_props_pattern_props_initialize( instance, parent );
	widget = instance->patternProps;

	XtVaSetValues( instance->patternProps_shellform, XmNuserData, pat, 0 );
	status = tt_pattern_user_set( pat, DtTtPatWidgetKey, widget );
	if (tt_is_err( status )) {
		XtDestroyWidget( widget );
		delete instance;
		return (Widget)tt_error_pointer( status );
	}
	if (callback != 0) {
		status = tt_pattern_callback_add( pat, callback );
		if (tt_is_err( status )) {
			XtDestroyWidget( widget );
			return (Widget)tt_error_pointer( status );
		}
	}

	ostrstream labelStream;
	labelStream << "Tt_pattern " << (void *)pat << ends;
	XtVaSetValues( instance->patternProps,
		       XmNtitle, labelStream.str(),
		       0 );
	delete labelStream.str();

	_DtTtPatternWidgetUpdate( instance, pat );

	XtManageChild( instance->patternProps_shellform );
	return widget;
}

static void
_DtTtPatternCategorySet(
	DtbPatternPropsPatternPropsInfo instance,
	Tt_category category
)
{
    Tt_pattern pat = patternProps2Pat( instance );
    if ((pat == 0) || _DtTtPatternUpdating( pat )) {
	    return;
    }
    Tt_status status = tt_pattern_category_set( pat, category );
    DtTtSetLabel( instance->patternProps_label, 
		  "tt_pattern_category_set()", status );
    _DtTtPatternWidgetUpdate( instance, pat );
}

static void
_DtTtPatternScopeAdd(
	DtbPatternPropsPatternPropsInfo instance,
	Tt_scope scope
)
{
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    Tt_status status = tt_pattern_scope_add( pat, scope );
    DtTtSetLabel( instance->patternProps_label,
		  "tt_pattern_scope_add()", status );
    _DtTtPatternWidgetUpdate( instance, pat );
}

static void
_DtTtPatternAddressAdd(
	DtbPatternPropsPatternPropsInfo instance,
	Tt_address address
)
{
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    Tt_status status = tt_pattern_address_add( pat, address );
    DtTtSetLabel( instance->patternProps_label, 
		  "tt_pattern_address_add()", status );
    _DtTtPatternWidgetUpdate( instance, pat );
}

static void
_DtTtPatternStateAdd(
	DtbPatternPropsPatternPropsInfo instance,
	Tt_state state
)
{
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    Tt_status status = tt_pattern_state_add( pat, state );
    DtTtSetLabel( instance->patternProps_label, 
		  "tt_pattern_state_add()", status );
    _DtTtPatternWidgetUpdate( instance, pat );
}

static void
_DtTtPatternClassAdd(
	DtbPatternPropsPatternPropsInfo instance,
	Tt_class theClass
)
{
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    Tt_status status = tt_pattern_class_add( pat, theClass );
    DtTtSetLabel( instance->patternProps_label, 
		  "tt_pattern_class_add()", status );
    _DtTtPatternWidgetUpdate( instance, pat );
}

static void
_DtTtPatternDispositionAdd(
	DtbPatternPropsPatternPropsInfo instance,
	Tt_disposition disposition
)
{
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    Tt_status status = tt_pattern_disposition_add( pat, disposition );
    DtTtSetLabel( instance->patternProps_label, 
		  "tt_pattern_disposition_add()", status );
    _DtTtPatternWidgetUpdate( instance, pat );
}

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/



void 
patternProps_patternCloseButton_CB1(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    DtbPatternPropsPatternPropsInfo	instance = (DtbPatternPropsPatternPropsInfo)clientData;
    
    if (!(instance->initialized))
    {
        dtb_pattern_props_pattern_props_initialize(instance, dtb_ttsnoop_ttsnoop_win.ttsnoopWin);
    }
    XtUnmanageChild(instance->patternProps_shellform);
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
patOpAdd(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtTtChooserSet( _DtTtChooserPatternOpAdd, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
patRegister(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    Tt_status status = tt_pattern_register( pat );
    DtTtSetLabel( instance->patternProps_label, 
		  "tt_pattern_register()", status );
    _DtTtPatternWidgetUpdate( instance, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
patUnRegister(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    Tt_status status = tt_pattern_unregister( pat );
    DtTtSetLabel( instance->patternProps_label, 
		  "tt_pattern_unregister()", status );
    _DtTtPatternWidgetUpdate( instance, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
patDestroy(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    Tt_status status = tt_pattern_destroy( pat );
    DtTtSetLabel( instance->patternProps_label, 
		  "tt_pattern_destroy()", status );
    switch (status) {
	case TT_OK:
	    DtTtDestroyed( DTTT_PATTERN, pat );
	    XtDestroyWidget( instance->patternProps );
	    delete instance;
	    break;
	default:
	    _DtTtPatternWidgetUpdate( instance, pat );
	    break;
    }
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
patHelp(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    dtb_do_onitem_help();
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addScopeSession(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternScopeAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_SESSION );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addScopeFile(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternScopeAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_FILE );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addScopeBoth(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternScopeAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_BOTH );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addScopeFileInSession(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternScopeAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_FILE_IN_SESSION );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addAddressPROCEDURE(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternAddressAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			    TT_PROCEDURE );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addAddressOBJECT(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternAddressAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			    TT_OBJECT );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addAddressHANDLER(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternAddressAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			    TT_HANDLER );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addAddressOTYPE(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternAddressAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			    TT_OTYPE );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateCREATED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_CREATED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateSENT(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_SENT );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateHANDLED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_HANDLED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateFAILED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_FAILED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateQUEUED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_QUEUED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateSTARTED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_STARTED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateREJECTED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_REJECTED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateRETURNED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_RETURNED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateACCEPTED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_ACCEPTED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addStateABSTAINED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternStateAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_ABSTAINED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addClassNOTICE(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternClassAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_NOTICE );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addClassREQUEST(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternClassAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_REQUEST );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addClassOFFER(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternClassAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			  TT_OFFER );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addDispositionDISCARD(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternDispositionAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			        TT_DISCARD );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addDispositionQUEUE(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternDispositionAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			        TT_QUEUE );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addDispositionSTART(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternDispositionAdd( (DtbPatternPropsPatternPropsInfo)clientData,
			        TT_START );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
setCategoryUNDEFINED(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternCategorySet( (DtbPatternPropsPatternPropsInfo)clientData,
			     TT_CATEGORY_UNDEFINED );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
setCategoryOBSERVE(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternCategorySet( (DtbPatternPropsPatternPropsInfo)clientData,
			     TT_OBSERVE );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
setCategoryHANDLE(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternCategorySet( (DtbPatternPropsPatternPropsInfo)clientData,
			     TT_HANDLE );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
setCategoryHANDLE_PUSH(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternCategorySet( (DtbPatternPropsPatternPropsInfo)clientData,
			     TT_HANDLE_PUSH );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
setCategoryHANDLE_ROTATE(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    _DtTtPatternCategorySet( (DtbPatternPropsPatternPropsInfo)clientData,
			     TT_HANDLE_ROTATE );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addContext(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtArgChooserSet( _DtArgChoosePatternContextAdd, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addOtype(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtStringChooserSet( _DtStringChoosePatternOtype, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addObject(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtStringChooserSet( _DtStringChoosePatternObject, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addSenderPtype(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtStringChooserSet( _DtStringChoosePatternSenderPtype, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addSender(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtTtChooserSet( _DtTtChooserPatternSenderAdd, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
sessionAdd(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtSessionChooserSet( _DtSessionChoosePattern, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addFile(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtFileChooserSet( _DtFileChoosePatternAdd, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
addArg(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtArgChooserSet( _DtArgChoosePatternArgAdd, pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
callbackAdd(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbPatternPropsPatternPropsInfo instance =
	    (DtbPatternPropsPatternPropsInfo)clientData;
    Tt_pattern pat = patternProps2Pat( instance );
    if (pat == 0) {
	    return;
    }
    _DtTtMsgCbChooserSet( pat );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}



/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** All automatically-generated data and functions have been defined.
 ***
 *** Add new functions here, or at the top of the file.
 ***/

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/


