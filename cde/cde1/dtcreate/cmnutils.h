/* $XConsortium: cmnutils.h /main/cde1_maint/2 1995/10/02 18:01:31 lehors $ */
/***************************************************************************/
/*                                                                         */
/*  cmnutils.h                                                             */
/*                                                                         */
/***************************************************************************/

#ifndef _CMNUTILS_H_INCLUDED
#define _CMNUTILS_H_INCLUDED


#include <Xm/Xm.h>

/********************************************************************************/
/*                                                                              */
/*  Function Declarations                                                       */
/*                                                                              */
/********************************************************************************/

#ifdef _NO_PROTO
XmStringTable TextStringsToXmStrings ();
char *XmStringToText ();
char **XmStringsToTextStrings ();
int countItems ();
void ffree_string_array ();
Widget findOptionMenuButtonID ();
void free_string_array ();
char *getLabelString ();
Widget getOptionMenuHistory ();
void setOptionMenuHistoryByText ();
void setOptionMenuHistoryByWidget ();
void show_warning_dialog ();
Widget show_working_dialog ();
void show_info_dialog ();
void free_XmStringTable ();

#else /* _NO_PROTO */

XmStringTable TextStringsToXmStrings (char **);
char *XmStringToText (XmString);
char **XmStringsToTextStrings (void);
int countItems (char **);
void ffree_string_array (void);
Widget findOptionMenuButtonID (void);
void free_string_array (void);
char *getLabelString (void);
Widget getOptionMenuHistory (void);
void setOptionMenuHistoryByText (void);
void setOptionMenuHistoryByWidget (void);
void show_warning_dialog (void);
Widget show_working_dialog (void);
void show_info_dialog (void);
void free_XmStringTable (XmStringTable table);

#endif /* _NO_PROTO */

#endif /* _CMNUTILS_H_INCLUDED */
