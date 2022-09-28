/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)help.h	1.1 96/07/30 SMI"


#ifndef _SPELL_HELP_H
#define _SPELL_HELP_H


/* Help volume associated with Spelling Checker. */
#define	SDTSPELL_HELP_VOLUME	"Textedit"

/* Help ID locations for the various help topics. */
#define	SDTSPELL_HELP_TOP_LEVEL		"SDtSpellHelp"
#define SDTSPELL_HELP_OPTIONS		"SDtSpellOptionsDialog"
#define	SDTSPELL_HELP_MAIN_D		"SDtSpellMainDictDialog"
#define	SDTSPELL_HELP_PERSONAL_D	"SDtSpellMainDictDialog"
#define	SDTSPELL_HELP_IMPORT		"SDtSpellImportDialog"
#define	SDTSPELL_HELP_EXPORT		"SDtSpellExportDialog"


/*
 * Spell help item consists of the parent window of the help dialog, and
 * the location id to display in the help dialog.
 */
typedef struct _SpellHelpItem {
	Widget     help_parent_win;
	char      *help_location_id;
} SpellHelpR, *SpellHelpP;



/* Callback to display help. */
extern void	_SDtSpellHelpCB(Widget, XtPointer, XtPointer);

#endif		/* _SPELL_HELP_H */
