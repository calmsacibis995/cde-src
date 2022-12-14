/*** DTB_USER_CODE_START vvv Add file header below vvv ***/
//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
//%%  (c) Copyright 1993, 1994 Novell, Inc.
//%%  $Revision: $
/*** DTB_USER_CODE_END   ^^^ Add file header above ^^^ ***/

/*
 * File: stringChooser_stubs.c
 * Contains: Module callbacks and connection functions
 *
 * This file was generated by dtcodegen, from module stringChooser
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
#include "stringChooser_ui.h"

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

#include <Xm/TextF.h>
#include <fstream.h>
#include "DtTt.h"

extern ofstream snoopStream;

void
_DtStringChooserSet(
	_DtStringChooserAction	choice,
	void *			entity,
	const char *		val
)
{
	DtbStringChooserStringChooserInfo instance =
		&dtb_string_chooser_string_chooser;
	if (! instance->initialized) {
		dtb_string_chooser_string_chooser_initialize(
			instance, dtb_ttsnoop_ttsnoop_win.ttsnoopWin );
	}
	if ((! tt_is_err( tt_ptr_error( val ))) && (val != 0)) {
		XmTextFieldSetString( instance->stringText, (char *)val );
	}
	_DtStringChooserSet( choice, entity );
}

void
_DtStringChooserSet(
	_DtStringChooserAction	choice,
	void *			entity
)
{
	char *okString;
	char *title;
	char *valueLabel;
	switch (choice) {
	    case _DtStringChoosePatternOp:
		okString = "Add";
		title = "tt_pattern_op_add";
		valueLabel = "Op:";
		break;
	    case _DtStringChoosePatternOtype:
		okString = "Add";
		title = "tt_pattern_otype_add";
		valueLabel = "Otype:";
		break;
	    case _DtStringChoosePatternObject:
		okString = "Add";
		title = "tt_pattern_object_add";
		valueLabel = "Object:";
		break;
	    case _DtStringChoosePatternSenderPtype:
		okString = "Add";
		title = "tt_pattern_sender_ptype_add";
		valueLabel = "Ptype:";
		break;
	    case _DtStringChooseMessageOtype:
		okString = "Set";
		title = "tt_message_otype_set";
		valueLabel = "Otype:";
		break;
	    case _DtStringChooseMessageObject:
		okString = "Set";
		title = "tt_message_object_set";
		valueLabel = "Object:";
		break;
	    case _DtStringChooseMessageSenderPtype:
		okString = "Set";
		title = "tt_message_sender_ptype_set";
		valueLabel = "Ptype:";
		break;
	    case _DtStringChooseMessageHandlerPtype:
		okString = "Set";
		title = "tt_message_handler_ptype_set";
		valueLabel = "Ptype:";
		break;
	    case _DtStringChooseMessageStatusString:
		okString = "Set";
		title = "tt_message_status_string_set";
		valueLabel = "String:";
		break;
	    case _DtStringChooseMessageArgValSet:
		okString = "Set...";
		title = "tt_message_arg_val_set";
		valueLabel = "Arg:";
		break;
	    case _DtStringChoosePtype2Declare:
		okString = "Declare";
		title = "tt_ptype_declare";
		valueLabel = "Ptype:";
		break;
	    case _DtStringChooseMediaPtype2Declare:
		okString = "Undeclare";
		title = "ttmedia_ptype_undeclare";
		valueLabel = "Ptype:";
		break;
	    case _DtStringChoosePtype2UnDeclare:
		okString = "Undeclare";
		title = "tt_ptype_undeclare";
		valueLabel = "Ptype:";
		break;
	    case _DtStringChoosePtype2SetDefault:
		okString = "Set";
		title = "tt_default_ptype_set";
		valueLabel = "Ptype:";
		break;
	    case _DtStringChoosePtype2Exists:
		okString = "Exists?";
		title = "tt_ptype_exists";
		valueLabel = "Ptype:";
		break;
	    case _DtStringChooseSystem:
		okString = "system()";
		title = "system(3C)";
		valueLabel = "Command:";
		break;
	    case _DtStringChoosePutenv:
		okString = "putenv()";
		title = "putenv(3C)";
		valueLabel = "variable=value:";
		break;
	    case _DtStringChooseNetfile2File:
		okString = "File";
		title = "tt_netfile_file";
		valueLabel = "Netfile:";
		break;
	    default:
		return;
	}
	DtbStringChooserStringChooserInfo instance =
		&dtb_string_chooser_string_chooser;
	if (! instance->initialized) {
		dtb_string_chooser_string_chooser_initialize(
			instance, dtb_ttsnoop_ttsnoop_win.ttsnoopWin );
	}
	DtTtSetLabel( instance->stringOkButton, okString );
	DtTtSetLabel( instance->stringText_label, valueLabel );
	XtVaSetValues( instance->stringChooser, XmNtitle, title, 0 );
	// Remember dialog mode, entity
	XtVaSetValues( instance->stringOkButton, XmNuserData, choice, 0 );
	XtVaSetValues( instance->stringCancelButton, XmNuserData, entity, 0 );
	XmTextPosition last = XmTextFieldGetLastPosition( instance->stringText );
	Time when = XtLastTimestampProcessed( XtDisplay( instance->stringText ));
	XmTextFieldSetSelection( instance->stringText, 0, last, when );
	XtManageChild( instance->stringChooser_shellform );
}

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/



void 
stringChooser_stringCancelButton_CB1(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    DtbStringChooserStringChooserInfo	instance = (DtbStringChooserStringChooserInfo)clientData;
    
    if (!(instance->initialized))
    {
        dtb_string_chooser_string_chooser_initialize(instance, dtb_ttsnoop_ttsnoop_win.ttsnoopWin);
    }
    XtUnmanageChild(instance->stringChooser_shellform);
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
stringOkayed(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbStringChooserStringChooserInfo instance =
	    (DtbStringChooserStringChooserInfo)clientData;
    char *string = XmTextFieldGetString( instance->stringText );
    if ((string != 0) && (string[0] == '\0')) {
	    XtFree( string );
	    string = 0;
    }
    XtPointer val;
    XtVaGetValues( instance->stringOkButton, XmNuserData, &val, 0 );
    _DtStringChooserAction choice = (_DtStringChooserAction)val;
    XtVaGetValues( instance->stringCancelButton, XmNuserData, &val, 0 );
    Widget label = dtb_ttsnoop_ttsnoop_win.ttsnoopWin_label;
    switch (choice) {
	    Tt_status status;
	    int ival;
	    char *file;
	case _DtStringChoosePatternOp:
	case _DtStringChoosePatternOtype:
	case _DtStringChoosePatternObject:
	case _DtStringChoosePatternSenderPtype:
	    _DtTtPatternUpdate( (Tt_pattern)val, choice, string );
	    break;
	case _DtStringChooseMessageOtype:
	case _DtStringChooseMessageObject:
	case _DtStringChooseMessageSenderPtype:
	case _DtStringChooseMessageHandlerPtype:
	case _DtStringChooseMessageStatusString:
	case _DtStringChooseMessageArgValSet:
	    _DtTtMessageUpdate( (Tt_message)val, choice, string );
	    break;
	case _DtStringChoosePtype2Declare:
	    status = tt_ptype_declare( string );
	    DtTtSetLabel( label, "tt_ptype_declare()", status );
	    break;
	case _DtStringChooseMediaPtype2Declare:
	    // XXX prompt for base_opnum
	    status = ttmedia_ptype_declare( string, 0, _DtTtMediaLoadPatCb,
					    0, 1 );
	    DtTtSetLabel( label, "ttmedia_ptype_declare()", status );
	    break;
	case _DtStringChoosePtype2UnDeclare:
	    status = tt_ptype_undeclare( string );
	    DtTtSetLabel( label, "tt_ptype_undeclare()", status );
	    break;
	case _DtStringChoosePtype2SetDefault:
	    status = tt_default_ptype_set( string );
	    DtTtSetLabel( label, "tt_default_ptype_set()", status );
	    break;
	case _DtStringChoosePtype2Exists:
	    status = tt_ptype_exists( string );
	    DtTtSetLabel( label, "tt_ptype_exists()", status );
	    break;
	case _DtStringChooseSystem:
	    ival = system( string );
	    DtTtSetLabel( label, "system()", ival );
	    break;
	case _DtStringChoosePutenv:
	    ival = putenv( string );
	    DtTtSetLabel( label, "putenv()", ival );
	    break;
	case _DtStringChooseNetfile2File:
	    file = tt_netfile_file( string );
	    DtTtSetLabel( label, "tt_netfile_file()", file );
	    if (! tt_is_err( tt_ptr_error( file ))) {
		    snoopStream << "tt_netfile_file( \"" << string
			    << "\" ) = \"" << file << "\"" << endl;
	    }
	    break;
	default:
	    break;
    }
    if (string != 0) {
	    XtFree( string );
    }
    XtUnmanageChild( instance->stringChooser_shellform );
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
stringHelp(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    DtbStringChooserStringChooserInfo instance =
	    (DtbStringChooserStringChooserInfo)clientData;
    XtPointer val;
    XtVaGetValues( instance->stringOkButton, XmNuserData, &val, 0 );
    _DtStringChooserAction choice = (_DtStringChooserAction)val;
    Widget label = dtb_ttsnoop_ttsnoop_win.ttsnoopWin_label;
    switch (choice) {
	case _DtStringChoosePatternOp:
	    _DtMan( label, "tt_pattern_op_add" );
	    break;
	case _DtStringChoosePatternOtype:
	    _DtMan( label, "tt_pattern_otype_add" );
	    break;
	case _DtStringChoosePatternObject:
	    _DtMan( label, "tt_pattern_object_add" );
	    break;
	case _DtStringChoosePatternSenderPtype:
	    _DtMan( label, "tt_pattern_sender_ptype_add" );
	    break;
	case _DtStringChooseMessageOtype:
	    _DtMan( label, "tt_message_otype_set" );
	    break;
	case _DtStringChooseMessageObject:
	    _DtMan( label, "tt_message_object_set" );
	    break;
	case _DtStringChooseMessageSenderPtype:
	    _DtMan( label, "tt_message_sender_ptype_set" );
	    break;
	case _DtStringChooseMessageHandlerPtype:
	    _DtMan( label, "tt_message_handler_ptype_set" );
	    break;
	case _DtStringChooseMessageStatusString:
	    _DtMan( label, "tt_message_status_string_set" );
	    break;
	case _DtStringChooseMessageArgValSet:
	    _DtMan( label, "tt_message_arg_val_set" );
	    break;
	case _DtStringChoosePtype2Declare:
	    _DtMan( label, "tt_ptype_declare" );
	    break;
	case _DtStringChooseMediaPtype2Declare:
	    _DtMan( label, "ttmedia_ptype_declare" );
	    break;
	case _DtStringChoosePtype2UnDeclare:
	    _DtMan( label, "tt_ptype_undeclare" );
	    break;
	case _DtStringChoosePtype2SetDefault:
	    _DtMan( label, "tt_default_ptype_set" );
	    break;
	case _DtStringChoosePtype2Exists:
	    _DtMan( label, "tt_ptype_exists" );
	    break;
	case _DtStringChooseSystem:
	    _DtMan( label, "system" );
	    break;
	case _DtStringChoosePutenv:
	    _DtMan( label, "putenv" );
	    break;
	default:
	    break;
    }
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


