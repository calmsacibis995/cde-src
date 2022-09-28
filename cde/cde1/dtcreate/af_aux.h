/* $XConsortium: af_aux.h /main/cde1_maint/3 1995/10/02 18:00:30 lehors $ */
/*****************************************************************************/
/*                                                                           */
/*  af_aux.h                                                                 */
/*                                                                           */
/*  Header file for af_aux.c                                                 */
/*                                                                           */
/*****************************************************************************/

#ifndef _AF_AUX_H_INCLUDED
#define _AF_AUX_H_INCLUDED

#include "UxXt.h"

#ifndef STORAGECLASS
#ifdef  NOEXTERN
#define STORAGECLASS
#else
#define STORAGECLASS extern
#endif
#endif

/*****************************************************************************/
/*                                                                           */
/*  Constants                                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Swidget Macro Definitions                                                */
/*                                                                           */
/*****************************************************************************/
#define FILE_CHARACTERISTICS_SWID (FileCharacteristics)
#define FA_TINY_ICON_BUTTON_SWID (AF_TinyIconButton)
#define FA_MED_ICON_BUTTON_SWID (AF_MedIconButton)

/*****************************************************************************/
/*                                                                           */
/*  Widget Macro Definitions                                                 */
/*                                                                           */
/*****************************************************************************/
#define FILE_CHARACTERISTICS (UxGetWidget(FILE_CHARACTERISTICS_SWID))
#define FA_TINY_ICON_BUTTON (UxGetWidget(FA_TINY_ICON_BUTTON_SWID))
#define FA_MED_ICON_BUTTON (UxGetWidget(FA_MED_ICON_BUTTON_SWID))

/*****************************************************************************/
/*                                                                           */
/*  Function Declarations                                                    */
/*                                                                           */
/*****************************************************************************/

#ifdef _NO_PROTO

void load_filetype_icons ();
void clear_filetype_icon ();
Widget get_selected_filetype_icon ();
void activateCB_filetype_icon ();

void readAFFromGUI ();
void getAF_FiletypeName ();
void getAF_IDChars ();
void getAF_HelpText ();
void getAF_Icons ();
void getAF_OpenCmd ();
void getAF_PrintCmd ();

void init_AddFiletype ();
void free_Filetypedata();
void clear_AddFiletype_dialog_fields();
void AddFiletypeToList();
void UpdateFiletypeDataArray();
Boolean AddFiletypeCheckFields();

#else /* _NO_PROTO */

void load_filetype_icons (Widget, XtPointer, XmFileSelectionBoxCallbackStruct *);
void clear_filetype_icon (void);
Widget get_selected_filetype_icon (void);
void activateCB_filetype_icon (Widget, XtPointer, DtIconCallbackStruct *);

void readAFFromGUI (FiletypeData *);
void getAF_FiletypeName (FiletypeData *);
void getAF_IDChars (FiletypeData *);
void getAF_HelpText (FiletypeData *);
void getAF_Icons (FiletypeData *);
void getAF_OpenCmd (FiletypeData *);
void getAF_PrintCmd (FiletypeData *);

void init_AddFiletype (FiletypeData *);
void free_Filetypedata(FiletypeData *);
void clear_AddFiletype_dialog_fields(void);
void AddFiletypeToList();
void UpdateFiletypeDataArray();
Boolean AddFiletypeCheckFields(void);

#endif /* _NO_PROTO */

#endif /* _AF_AUX_H_INCLUDED */
