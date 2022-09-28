/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)help.c	1.2 96/09/11 SMI"

/*--------------------------------------------------------------*/
/* 			INCLUDE FILES 				*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <nl_types.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif				/* X_NOT_STDC_ENV */

#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Dt/HelpDialog.h>
#include <Dt/Help.h>
#include <Dt/IconFile.h>

#include "globals.h"
#include "help.h"


/* Function:	_SDtSpellHelpCB()
 * 
 * Objective:	Callback invoked when user presses on the help button in
 *		any of the spelling checker's components.
 */
void
_SDtSpellHelpCB(Widget wid, XtPointer client_data, XtPointer call_data)
{
	int             arg_cnt;
	Arg             arg_list[10];
	static Widget   help_dialog = NULL;
	SpellHelpP      helpItem = (SpellHelpP) client_data;
 

	/* Set the requested help information. */
	arg_cnt = 0;
	XtSetArg(arg_list[arg_cnt], DtNhelpType, DtHELP_TYPE_TOPIC);
	arg_cnt++;
	XtSetArg(arg_list[arg_cnt], DtNhelpVolume, SDTSPELL_HELP_VOLUME); 
	arg_cnt++;
	XtSetArg(arg_list[arg_cnt], DtNlocationId, helpItem->help_location_id);
	arg_cnt++;


	/* Create the help dialog, if it is NULL. */
	if (help_dialog == NULL) {
		XtSetArg(arg_list[arg_cnt], XmNtitle, 
			 catgets(_spellCat, 1, 18, "Spelling Checker : Help"));
		arg_cnt++;
		help_dialog = DtCreateHelpDialog(helpItem->help_parent_win, 
						 "_SDtSpellHelpDialog",
                                                 arg_list, arg_cnt);
	} else 
		XtSetValues (help_dialog, arg_list, arg_cnt);


	/* Display the help dialog. */
	XtManageChild(help_dialog);
}
