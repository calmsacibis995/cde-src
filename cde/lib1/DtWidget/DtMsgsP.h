/* $XConsortium: DtMsgsP.h /main/cde1_maint/1 1995/07/17 18:27:54 drk $ */
/*
 * DtWidget/DtMsgsP.h
 */
/*
 * DtMsgsP.h:
 *
 * Private header file for libDtWidget
 */
#ifndef _DtMessages_h
#define _DtMessages_h

/*** const causes the HP compiler to complain.  Remove reference until   ***
 *** the build and integration people can figure out how to get this to  ***
 *** work.                                                               ***/

#define CONST	const
/* #define CONST */

#ifdef I18N_MSG
#include <nl_types.h>
#define DTWIDGET_GETMESSAGE(set, number, string)\
    (char *) _DtWidgetGetMessage(set, number, (char*)string)

#else /* I18N_MSG */
#define DTWIDGET_GETMESSAGE(set, number, string) (char*)string
#endif /* I18N_MSG */

extern CONST char _DtMsgCommon_0000[];
extern CONST char _DtMsgCommon_0001[];
extern CONST char _DtMsgComboBox_0000[];
extern CONST char _DtMsgComboBox_0001[];
extern CONST char _DtMsgComboBox_0002[];
extern CONST char _DtMsgComboBox_0003[];
extern CONST char _DtMsgComboBox_0004[];
extern CONST char _DtMsgComboBox_0005[];
extern CONST char _DtMsgComboBox_0006[];
extern CONST char _DtMsgComboBox_0007[];
extern CONST char _DtMsgComboBox_0008[];
extern CONST char _DtMsgComboBox_0009[];
extern CONST char _DtMsgComboBox_0010[];
extern CONST char _DtMsgComboBox_0011[];
extern CONST char _DtMsgComboBox_0012[];
extern CONST char _DtMsgComboBox_0013[];
extern CONST char _DtMsgComboBox_0014[];
extern CONST char _DtMsgDialogBox_0000[];
extern CONST char _DtMsgDialogBox_0001[];
extern CONST char _DtMsgEditor_0000[];
extern CONST char _DtMsgEditor_0001[];
extern CONST char _DtMsgEditor_0002[];
extern CONST char _DtMsgEditor_0003[];
extern CONST char _DtMsgEditor_0004[];
extern CONST char _DtMsgEditor_0005[];
extern CONST char _DtMsgEditor_0006[];
extern CONST char _DtMsgEditor_0007[];
extern CONST char _DtMsgEditor_0008[];
extern CONST char _DtMsgEditor_0009[];
extern CONST char _DtMsgEditor_0010[];
extern CONST char _DtMsgEditor_0011[];
extern CONST char _DtMsgEditor_0012[];
extern CONST char _DtMsgEditor_0013[];
extern CONST char _DtMsgEditor_0014[];
extern CONST char _DtMsgEditor_0015[];
extern CONST char _DtMsgEditor_0016[];
extern CONST char _DtMsgEditor_0017[];
extern CONST char _DtMsgEditor_0018[];
extern CONST char _DtMsgEditor_0019[];
extern CONST char _DtMsgEditor_0020[];
extern CONST char _DtMsgEditor_0021[];
extern CONST char _DtMsgEditor_0022[];
extern CONST char _DtMsgEditor_0050[];
extern CONST char _DtMsgEditor_0051[];
extern CONST char _DtMsgEditor_0052[];
extern CONST char _DtMsgEditor_0053[];
extern CONST char _DtMsgEditor_0054[];
extern CONST char _DtMsgEditor_0055[];
extern CONST char _DtMsgEditor_0056[];
extern CONST char _DtMsgEditor_0057[];
extern CONST char _DtMsgEditor_0058[];
extern CONST char _DtMsgEditor_0059[];
extern CONST char _DtMsgEditor_0060[];
extern CONST char _DtMsgEditor_0061[];
extern CONST char _DtMsgEditor_0062[];
extern CONST char _DtMsgEditor_0063[];
extern CONST char _DtMsgEditor_0064[];
extern CONST char _DtMsgEditor_0065[];
extern CONST char _DtMsgEditor_0066[];
extern CONST char _DtMsgEditor_0067[];
extern CONST char _DtMsgEditor_0068[];
extern CONST char _DtMsgEditor_0069[];
extern CONST char _DtMsgEditor_0070[];
extern CONST char _DtMsgEditor_0071[];
extern CONST char _DtMsgEditor_0072[];
extern CONST char _DtMsgEditor_0073[];
extern CONST char _DtMsgEditor_0074[];
extern CONST char _DtMsgEditor_0075[];
extern CONST char _DtMsgEditor_0076[];
extern CONST char _DtMsgEditor_0077[];
extern CONST char _DtMsgEditor_0078[];
extern CONST char _DtMsgEditor_0079[];
extern CONST char _DtMsgEditor_0080[];
extern CONST char _DtMsgEditor_0081[];
extern CONST char _DtMsgEditor_0082[];
extern CONST char _DtMsgEditor_0083[];
extern CONST char _DtMsgEditor_0084[];
extern CONST char _DtMsgEditor_0085[];
extern CONST char _DtMsgEditor_0086[];
extern CONST char _DtMsgEditor_0087[];
extern CONST char _DtMsgEditor_0088[];
extern CONST char _DtMsgEditor_0089[];
extern CONST char _DtMsgEditor_0090[];
extern CONST char _DtMsgEditor_0091[];
extern CONST char _DtMsgEditor_0092[];
extern CONST char _DtMsgEditor_0093[];
extern CONST char _DtMsgIcon_0000[];
extern CONST char _DtMsgIcon_0001[];
extern CONST char _DtMsgIcon_0002[];
extern CONST char _DtMsgIcon_0003[];
extern CONST char _DtMsgIcon_0004[];
extern CONST char _DtMsgIcon_0005[];
extern CONST char _DtMsgIndicator_0000[];
extern CONST char _DtMsgMenuButton_0000[];
extern CONST char _DtMsgMenuButton_0001[];
extern CONST char _DtMsgMenuButton_0002[];
extern CONST char _DtMsgSpinBox_0000[];
extern CONST char _DtMsgSpinBox_0001[];
extern CONST char _DtMsgSpinBox_0002[];
extern CONST char _DtMsgSpinBox_0003[];
extern CONST char _DtMsgSpinBox_0004[];
extern CONST char _DtMsgSpinBox_0005[];
extern CONST char _DtMsgSpinBox_0006[];
extern CONST char _DtMsgSpinBox_0007[];
extern CONST char _DtMsgSpinBox_0008[];
extern CONST char _DtMsgSpinBox_0009[];
extern CONST char _DtMsgSpinBox_0010[];
extern CONST char _DtMsgSpinBox_0011[];
extern CONST char _DtMsgSpinBox_0012[];
extern CONST char _DtMsgSpinBox_0013[];
extern CONST char _DtMsgSpinBox_0014[];
extern CONST char _DtMsgTitleBox_0000[];
extern CONST char _DtMsgTitleBox_0001[];
extern CONST char _DtMsgTitleBox_0002[];
extern CONST char _DtMsgTitleBox_0003[];
extern CONST char _DtMsgTitleBox_0004[];

#ifdef I18N_MSG

/* 
 * Message set for shared messages 
 */
#define	MS_Common		2

/* Message IDs for shared messages */
#define	MSG_CLOSE		1
#define	MSG_HELP		2

/* 
 * Message set for Editor widget
 */
#define	MS_Editor		3

/* Message IDs for Editor */
#define	EDITOR_FORMAT_SETTINGS	1
#define	EDITOR_RIGHT_MARGIN	2
#define	EDITOR_LEFT_MARGIN	3
#define	EDITOR_LEFT_ALIGN	4
#define	EDITOR_RIGHT_ALIGN	5
#define	EDITOR_JUSTIFY		6
#define	EDITOR_CENTER		7
#define	EDITOR_PARAGRAPH	8
#define	EDITOR_ALL		9
#define	EDITOR_SPELL_TITLE	20
#define	EDITOR_FIND_TITLE	21
#define	EDITOR_MISSPELLED	22
#define	EDITOR_FIND_LABEL	23
#define	EDITOR_CHANGE_LABEL	24
#define	EDITOR_FIND_BUTTON	25
#define	EDITOR_CHANGE_BUTTON	26
#define	EDITOR_CHNG_ALL_BUTTON	27
#define	EDITOR_NO_FIND		30
#define	EDITOR_INFO_TITLE	31
#define	EDITOR_LINE		40
#define	EDITOR_TOTAL		41
#define	EDITOR_OVR		42
#define	EDITOR_INS		43
#define EDITOR_FILE_MENU        50
#define EDITOR_FILE_MNEMONIC    51
#define EDITOR_INCLUDE_MENU     52
#define EDITOR_INCLUDE_MNEMONIC 53
#define EDITOR_EDIT_MENU        54
#define EDITOR_EDIT_MNEMONIC    55
#define EDITOR_UNDO_MENU        56
#define EDITOR_UNDO_MNEMONIC    57
#define EDITOR_CUT_MENU         58
#define EDITOR_CUT_MNEMONIC     59
#define EDITOR_COPY_MENU        60
#define EDITOR_COPY_MNEMONIC    61
#define EDITOR_PASTE_MENU       62
#define EDITOR_PASTE_MNEMONIC   63
#define EDITOR_CLEAR_MENU       64
#define EDITOR_CLEAR_MNEMONIC   65
#define EDITOR_DELETE_MENU      66
#define EDITOR_DELETE_MNEMONIC  67
#define EDITOR_SALL_MENU        68
#define EDITOR_SALL_MNEMONIC    69
#define EDITOR_FC_MENU          70
#define EDITOR_FC_MNEMONIC      71
#define EDITOR_SPELL_MENU       72
#define EDITOR_SPELL_MNEMONIC   73
#define EDITOR_FORMAT_MENU      74
#define EDITOR_FORMAT_MNEMONIC  75
#define EDITOR_SETTINGS_MENU    76
#define EDITOR_SETTINGS_MNEMONIC 77
#define EDITOR_PARAG_MENU       78
#define EDITOR_PARAG_MNEMONIC   79
#define EDITOR_ALL_MENU         80
#define EDITOR_ALL_MNEMONIC     81
#define EDITOR_OPTIONS_MENU     82
#define EDITOR_OPTIONS_MNEMONIC 83
#define EDITOR_OSTRIKE_MENU     84
#define EDITOR_OSTRIKE_MNEMONIC 85
#define EDITOR_WRAP_MENU        86
#define EDITOR_WRAP_MNEMONIC    87
#define EDITOR_STATUS_MENU      88
#define EDITOR_STATUS_MNEMONIC  89
#define EDITOR_EXTRAS_MENU      90
#define EDITOR_EXTRAS_MNEMONIC  91
#define EDITOR_WARNING_1        92
#define EDITOR_WARNING_TITLE    93


/* 
 * Message set for MenuButton widget
 */
#define	MS_MenuButton		4

/* Message IDs for MenuButton*/
#define	MENU_POST		1
#define	MENU_PARENT		2
#define	MENU_SUBMENU		3

/* 
 * Message set for ComboBox widget
 */
#define	MS_ComboBox 		5

/* Message IDs for ComboBox */
#define	COMBO_ALIGNMENT		1 
#define	COMBO_MARGIN_HEIGHT 	2
#define	COMBO_MARGIN_WIDTH	3 
#define	COMBO_HORIZONTAL_SPACING	4
#define	COMBO_VERTICAL_SPACING 	5
#define	COMBO_ORIENTATION 	6
#define	COMBO_ITEM_COUNT 	7
#define	COMBO_VISIBLE_ITEM	8 
#define	COMBO_TEXT 		9
#define	COMBO_SET_ITEM		10
#define	COMBO_SELECT_ITEM 	11
#define	COMBO_RESIZE		12
#define	COMBO_LABEL		13
#define	COMBO_CVTSTRING		14
#define	COMBO_DEL_POS		15

/* 
 * Message set for SpinBox widget
 */
#define	MS_SpinBox 		6

/* Message IDs for SpinBox */
#define	SPIN_ARROW_SENSITIVE	1
#define	SPIN_ALIGNMENT		2 
#define	SPIN_INIT_DELAY		3
#define SPIN_MARGIN_HEIGHT	4
#define SPIN_MARGIN_WIDTH	5
#define SPIN_ARROW_LAYOUT	6
#define SPIN_REPEAT_DELAY	7
#define SPIN_ITEM_COUNT		8
#define SPIN_POSITION_STRING	9
#define SPIN_POSITION_NUMERIC	10
#define SPIN_DECIMAL_POINTS	11
#define SPIN_MIN_MAX		12
#define SPIN_TEXT		13
#define SPIN_SET_ITEM		14	
#define SPIN_LABEL		15	

extern char *	_DtWidgetGetMessage(
			int set,
			int n,
			char *s );

#endif /* I18N_MSG */

#endif /* _DtMessageh */
