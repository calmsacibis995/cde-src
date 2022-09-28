/* $XConsortium: FileCharacteristics.c /main/cde1_maint/1 1995/07/17 20:26:02 drk $ */

/*******************************************************************************
        FileCharacteristics.c

       Associated Header file: FileCharacteristics.h
*******************************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/ToggleBG.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/ScrolledW.h>

#include "UxXt.h"
#include "dtcreate.h"
#include "cmncbs.h"

#define CONTEXT_MACRO_ACCESS 1
#include "FileCharacteristics.h"
#undef CONTEXT_MACRO_ACCESS

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

/*
int     set=1;
*/
Widget  FC_ReadOnToggle, FC_ReadOffToggle, FC_ReadEitherToggle;
Widget  FC_WriteOnToggle, FC_WriteOffToggle, FC_WriteEitherToggle;
Widget  FC_ExecuteOnToggle, FC_ExecuteOffToggle, FC_ExecuteEitherToggle;
Widget  FC_PermissionForm;
Widget  FC_ReadLabel;
Widget  FC_WriteLabel;
Widget  FC_ExecuteLabel;

/******************************************************************************/
/*                                                                            */
/* GetPermissionsPattern                                                      */
/*                                                                            */
/* INPUT: none                                                                */
/* OUTPUT: permissions pattern in database format                             */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void GetPermissionsPattern(ppszPermPattern)
     char **ppszPermPattern;
#else /* _NO_PROTO */
void GetPermissionsPattern(char **ppszPermPattern)
#endif /* _NO_PROTO */

{
  if (*ppszPermPattern) {
     XtFree(*ppszPermPattern);
     *ppszPermPattern = (char *)NULL;
  }

  *ppszPermPattern = (char *)XtMalloc(20);
  memset(*ppszPermPattern, 0, 20);

  if (XmToggleButtonGadgetGetState(FC_DirectoryToggle)) {
    strcat(*ppszPermPattern, "d");
  } else {
    strcat(*ppszPermPattern, "!d");
  }

  if (XmToggleButtonGadgetGetState(FC_PermissionToggle)) {
     /******************************************************************/
     /* Get Read state                                                 */
     /******************************************************************/
     if (XmToggleButtonGadgetGetState(FC_ReadOnToggle)) {
        strcat(*ppszPermPattern, "&r");
     } else {
        if (XmToggleButtonGadgetGetState(FC_ReadOffToggle)) {
           strcat(*ppszPermPattern, "&!r");
        }
     }
     /******************************************************************/
     /* Get Write state                                                */
     /******************************************************************/
     if (XmToggleButtonGadgetGetState(FC_WriteOnToggle)) {
        strcat(*ppszPermPattern, "&w");
     } else {
       if (XmToggleButtonGadgetGetState(FC_WriteOffToggle)) {
          strcat(*ppszPermPattern, "&!w");
       }
     }
     /******************************************************************/
     /* Get Execute state                                              */
     /******************************************************************/
     if (XmToggleButtonGadgetGetState(FC_ExecuteOnToggle)) {
        strcat(*ppszPermPattern, "&x");
     } else {
        if (XmToggleButtonGadgetGetState(FC_ExecuteOffToggle)) {
           strcat(*ppszPermPattern, "&!x");
        }
     }
  }

#if 0
  if (XmToggleButtonGadgetGetState(FC_PermissionToggle)) {
     if (XmToggleButtonGadgetGetState(FC_ReadToggle)) {
       strcat(*ppszPermPattern, "&r");
     } else {
       strcat(*ppszPermPattern, "&!r");
     }
     if (XmToggleButtonGadgetGetState(FC_WriteToggle)) {
       strcat(*ppszPermPattern, "&w");
     } else {
       strcat(*ppszPermPattern, "&!w");
     }
     if (XmToggleButtonGadgetGetState(FC_ExecuteToggle)) {
       strcat(*ppszPermPattern, "&x");
     } else {
       strcat(*ppszPermPattern, "&!x");
     }
  }
#endif
}

/******************************************************************************/
/*                                                                            */
/* GetContentsType                                                            */
/*                                                                            */
/* INPUT: none                                                                */
/* OUTPUT: contents type selected                                             */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
ushort GetContentsType()
#else /* _NO_PROTO */
ushort GetContentsType(void)
#endif /* _NO_PROTO */

{
  if (XmToggleButtonGadgetGetState(FC_StringToggle)) {
    return((ushort)CA_FT_CNTSTRING);
  } else if (XmToggleButtonGadgetGetState(FC_ByteToggle)) {
    return((ushort)CA_FT_CNTBYTE);
  } else if (XmToggleButtonGadgetGetState(FC_ShortToggle)) {
    return((ushort)CA_FT_CNTSHORT);
  } else if (XmToggleButtonGadgetGetState(FC_LongToggle)) {
    return((ushort)CA_FT_CNTLONG);
  }
}

/******************************************************************************/
/*                                                                            */
/* Save_FileChar_Info                                                         */
/*                                                                            */
/* INPUT: none                                                                */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void Save_FileChar_Info()
#else /* _NO_PROTO */
void Save_FileChar_Info(void)
#endif /* _NO_PROTO */

{
  FiletypeData *pFtD;
  char         *ptr = (char *)NULL;
  short        sContentsType;

  /**************************************************************************/
  /* Clear file characteristics text widget on the AddFiletype dialog.      */
  /**************************************************************************/
  XmTextSetString(AF_IdCharacteristicsText, "");

  /**************************************************************************/
  /* Get pointer to FiletypeData structure.                                 */
  /* Also, update the filetypes listbox on the AddFiletype window.          */
  /**************************************************************************/
  XtVaGetValues(FileCharacteristics,
                XmNuserData, &pFtD,
                NULL);

  /**************************************************************************/
  /* Get Name Pattern Info                                                  */
  /**************************************************************************/
  if (XmToggleButtonGadgetGetState(FC_NameOrPathToggle)) {
    GetWidgetTextString(FC_NameOrPathText, &(pFtD->pszPattern));
#if DEBUG
    printf("Pattern is '%s'\n", pFtD->pszPattern);
#endif
  } else {
    if (pFtD->pszPattern) {
       XtFree(pFtD->pszPattern);
       pFtD->pszPattern = (char *)NULL;
    }
  }

  /**************************************************************************/
  /* Get Permission Info                                                    */
  /**************************************************************************/
  GetPermissionsPattern(&(pFtD->pszPermissions));

  /**************************************************************************/
  /* Get Content Pattern Info                                               */
  /**************************************************************************/
  if (XmToggleButtonGadgetGetState(FC_ContentsToggle)) {

    /************************************************************************/
    /* Save data in structure.                                              */
    /************************************************************************/
    GetWidgetTextString(FC_ContentsPatternText, &(pFtD->pszContents));
    ptr = (char *)NULL;
    GetWidgetTextString(FC_StartByteTextField, &ptr);
    if (ptr) {
       pFtD->sStart = atoi(ptr);
       XtFree(ptr);
    }
#if 0
    ptr = (char *)NULL;
    GetWidgetTextString(FC_EndByteTextField, &ptr);
    if (ptr) {
       pFtD->sEnd = atoi(ptr);
       XtFree(ptr);
    }
#endif
    pFtD->fsFlags = GetContentsType();

  } else {
    if (pFtD->pszContents) {
       XtFree(pFtD->pszContents);
       pFtD->pszContents = (char *)NULL;
    }
    pFtD->sStart = 0;
  }

  /**************************************************************************/
  /* Parse the ID fields in the FiletypeData structure and create a text    */
  /* string for the Identifying Characteristics text field in AddFiletype.  */
  /**************************************************************************/
  ParseAndUpdateID(pFtD);

}

/******************************************************************************/
/*                                                                            */
/* ParseAndUpdateID                                                           */
/*                                                                            */
/* INPUT: none                                                                */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void ParseAndUpdateID(pFtD)
     FiletypeData *pFtD;
#else /* _NO_PROTO */
void ParseAndUpdateID(FiletypeData *pFtD)
#endif /* _NO_PROTO */

{
  char    *ptr = (char *)NULL;
  int     size;
  char    buffer[MAXBUFSIZE];

  /**************************************************************************/
  /* Init size variable.  This will be used to indicate size of buffer to   */
  /* allocate to hold text string that will be displayed in FC text widget. */
  /**************************************************************************/
  size = 0;

  /**************************************************************************/
  /* Get Name Pattern Info                                                  */
  /**************************************************************************/
  if (pFtD->pszPattern) {
     /*size += strlen("Name: ");*/
     size += strlen(GETMESSAGE(9, 20, "Name Pattern:")) + 1; /* 1 for space */
     size += strlen(pFtD->pszPattern);
     size += 1;                                    /* For newline character */
  }

  /**************************************************************************/
  /* Get Permission Info                                                    */
  /**************************************************************************/
  if (pFtD->pszPermissions) {
     size += strlen(GETMESSAGE(9, 21, "Permission Pattern:")) + 1; /* space */
     size += strlen(pFtD->pszPermissions);
     size += 1;                                    /* For newline character */
  }

  /**************************************************************************/
  /* Get Content Pattern Info                                               */
  /**************************************************************************/
  if (pFtD->pszContents) {
     size += strlen(GETMESSAGE(9, 28, "Contents:")) + 1;     /* 1 for space */
     size += strlen(pFtD->pszContents);
     size += 1;                                    /* For newline character */
  }

  /**************************************************************************/
  /* Allocate buffer and fill with ID info.                                 */
  /**************************************************************************/
  if (size > 0) {
    ptr = XtMalloc(size);
    if (ptr) {
       memset(ptr, 0, size);
       if (pFtD->pszPattern) {
          sprintf(buffer, "%s %s\n", GETMESSAGE(9, 20, "Name Pattern:"), pFtD->pszPattern);
          strcat(ptr, buffer);
       }
       if (pFtD->pszPermissions) {
          sprintf(buffer, "%s %s\n", GETMESSAGE(9, 21, "Permission Pattern:"), pFtD->pszPermissions);
          strcat(ptr, buffer);
       }
       if (pFtD->pszContents) {
          sprintf(buffer, "%s %s\n", GETMESSAGE(9, 28, "Contents:"), pFtD->pszContents);
          strcat(ptr, buffer);
       }
       XmTextSetString(AF_IdCharacteristicsText, ptr);
       XtFree(ptr);
    }
  }
}

/*****************************************************************************/
/*                                                                           */
/*  CalculateAND                                                             */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
void CalculateAND()
#else /* _NO_PROTO */
void CalculateAND(void)
#endif /* _NO_PROTO */
{
  Boolean set1, set2, set3, set4;
  int     cnt;

  XtVaGetValues(FC_NameOrPathToggle, XmNset, &set1, NULL);
  XtVaGetValues(FC_PermissionToggle, XmNset, &set2, NULL);
  XtVaGetValues(FC_ContentsToggle, XmNset, &set3, NULL);
  XtSetSensitive(FC_AndLabel1, False);
  XtSetSensitive(FC_AndLabel2, False);

  cnt = (int)set1 + (int)set2 + (int)set3;
    if (set1 && (set2 || set3))
          XtSetSensitive(FC_AndLabel1, True);
    if (set2 && set3)
          XtSetSensitive(FC_AndLabel2, True);
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

#ifdef _NO_PROTO
static  void    activateCB_FC_OkButton( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    activateCB_FC_OkButton( Widget  UxWidget,
                                        XtPointer UxClientData,
                                        XtPointer UxCallbackArg)
#endif /* _NO_PROTO */

{
  if (!FileCharCheckFields()) {
     Save_FileChar_Info();
     UxPopdownInterface(FileCharacteristics);
  }
}

#ifdef _NO_PROTO
static  void    activateCB_FC_ClearButton( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    activateCB_FC_ClearButton(Widget UxWidget,
                                          XtPointer UxClientData,
                                          XtPointer UxCallbackArg)
#endif /* _NO_PROTO */

{
  /* Clear_FileChar_Info(); */
  clear_FileCharacteristics_dialog_fields();
}

#ifdef _NO_PROTO
static  void    activateCB_FC_CancelButton( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    activateCB_FC_CancelButton(Widget UxWidget,
                                          XtPointer UxClientData,
                                          XtPointer UxCallbackArg)
#endif /* _NO_PROTO */
{
      /*
        _UxCFileCharacteristics *UxSaveCtx, *UxContext;

        UxSaveCtx = UxFileCharacteristicsContext;
        UxFileCharacteristicsContext = UxContext =
                        (_UxCFileCharacteristics *) UxGetContext( UxWidget );
        {
       */
        UxPopdownInterface(FileCharacteristics);
       /*
        }
        UxFileCharacteristicsContext = UxSaveCtx;
       */
}

#ifdef _NO_PROTO
static  void    valueChangedCB_FC_PermissionToggle( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    valueChangedCB_FC_PermissionToggle(Widget UxWidget,
                                          XtPointer UxClientData,
                                          XtPointer UxCallbackArg)
#endif /* _NO_PROTO */
{
  Boolean set;

  XtVaGetValues(UxWidget, XmNset, &set, NULL);
  XtSetSensitive(FC_PermissionLabel, set);
  XtSetSensitive(FC_PermissionForm, set);
  /*
  XtSetSensitive(FC_ReadLabel, set);
  XtSetSensitive(FC_WriteLabel, set);
  XtSetSensitive(FC_ExecuteLabel, set);

  XtSetSensitive(FC_ReadOnToggle, set);
  XtSetSensitive(FC_WriteOnToggle, set);
  XtSetSensitive(FC_ExecuteOnToggle, set);

  XtSetSensitive(FC_ReadOffToggle, set);
  XtSetSensitive(FC_WriteOffToggle, set);
  XtSetSensitive(FC_ExecuteOffToggle, set);

  XtSetSensitive(FC_ReadEitherToggle, set);
  XtSetSensitive(FC_WriteEitherToggle, set);
  XtSetSensitive(FC_ExecuteEitherToggle, set);
  */
  CalculateAND();
}

#if 0
#ifdef _NO_PROTO
static  void    armCB_FC_StringToggle( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    armCB_FC_StringToggle(Widget UxWidget,
                                          XtPointer UxClientData,
                                          XtPointe`r UxCallbackArg)
#endif /* _NO_PROTO */
{
  /*set=1;*/
}

#ifdef _NO_PROTO
static  void    armCB_FC_ByteToggle( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    armCB_FC_ByteToggle(Widget UxWidget,
                                    XtPointer UxClientData,
                                    XtPointer UxCallbackArg)
#endif /* _NO_PROTO */
{
  /*set=2;*/
}

#ifdef _NO_PROTO
static  void    armCB_FC_ShortToggle( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    armCB_FC_ShortToggle(Widget UxWidget,
                                    XtPointer UxClientData,
                                    XtPointer UxCallbackArg)
#endif /* _NO_PROTO */
{
  /*set=3;*/
}

#ifdef _NO_PROTO
static  void    armCB_FC_LongToggle( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    armCB_FC_LongToggle( Widget UxWidget,
                                    XtPointer UxClientData,
                                    XtPointer UxCallbackArg)
#endif /* _NO_PROTO */
{
  /*set=4;*/
}
#endif

#ifdef _NO_PROTO
static  void    valueChangedCB_FC_ContentsToggle( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    valueChangedCB_FC_ContentsToggle( Widget UxWidget,
                                                 XtPointer UxClientData,
                                                 XtPointer UxCallbackArg)
#endif /* _NO_PROTO */
{
  Boolean set;

  XtVaGetValues(UxWidget, XmNset, &set, NULL);
  XtSetSensitive(FC_ContentsLabel, set);
  XtSetSensitive(FC_ContentsBox, set);
  if (set) {
     XmProcessTraversal(FC_ContentsPatternText, XmTRAVERSE_CURRENT);
  }
  CalculateAND();
}

#ifdef _NO_PROTO
static  void    valueChangedCB_FC_NameOrPathToggle( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    valueChangedCB_FC_NameOrPathToggle(Widget UxWidget,
                                                   XtPointer UxClientData,
                                                   XtPointer UxCallbackArg)
#endif /* _NO_PROTO */

{
  Boolean set;

  XtVaGetValues(UxWidget, XmNset, &set, NULL);
  XtSetSensitive(FC_NamePatternLabel, set);
  XtSetSensitive(FC_NameOrPathText, set);
  if (set) {
     XmProcessTraversal(FC_NameOrPathText, XmTRAVERSE_CURRENT);
  }
  CalculateAND();
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

#ifdef _NO_PROTO
static Widget   _Uxbuild_FileCharacteristics()
#else /* _NO_PROTO */
static Widget   _Uxbuild_FileCharacteristics(void)
#endif /* _NO_PROTO */
{
        Widget          _UxParent;
        Widget          FC_ActionAreaForm;
        Widget          FC_OkButton;
        Widget          FC_ClearButton;
        Widget          FC_CancelButton;
        Widget          FC_HelpButton;
        Widget          FC_MainLabel1;
        Widget          FC_MainLabel2;
        Widget          FC_TypeFileRowColumn;

        Widget          FC_ReadRowColumn;
        Widget          FC_WriteRowColumn;
        Widget          FC_ExecuteRowColumn;
        int             ntotalbuttons = 4;
        int             nbutton = 0;
#define TIGHTNESS       20
        Widget          FC_ScrolledWindow;
        Widget          FC_BigForm;


        /* Creation of FileCharacteristics */
#if 0
        _UxParent = UxParent;
        if ( _UxParent == NULL )
        {
             _UxParent = UxTopLevel;
        }
#endif

        _UxParent = UxTopLevel;

        if (bLowRes) {
           _UxParent = XtVaCreatePopupShell( "FileCharacteristics_shell",
                        xmDialogShellWidgetClass, _UxParent,
                        XmNx, 398,
                        XmNy, 222,
                        XmNwidth, 446,
                        XmNheight, 579,
                        XmNshellUnitType, XmPIXELS,
                        XmNtitle, GETMESSAGE(9, 10, "Identifying Characteristics"),
                        NULL );
           FC_BigForm = XtVaCreateWidget( "FC_BigForm",
                        xmFormWidgetClass,
                        _UxParent,
                        XmNunitType, XmPIXELS,
                        XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
                        RES_CONVERT( XmNdialogTitle, GETMESSAGE(9, 10, "Identifying Characteristics")),
                        XmNautoUnmanage, FALSE,
                        XmNrubberPositioning, FALSE,
                        NULL );
           FC_ScrolledWindow = XtVaCreateManagedWidget( "FC_ScrolledWindow",
                        xmScrolledWindowWidgetClass,
                        FC_BigForm,
                        XmNscrollingPolicy, XmAUTOMATIC,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNbottomAttachment, XmATTACH_FORM,
                        XmNtopAttachment, XmATTACH_FORM,
                        NULL );
           XtVaSetValues(FC_BigForm, XmNuserData, FC_ScrolledWindow, NULL);

           FileCharacteristics = XtVaCreateManagedWidget( "FileCharacteristics",
                           xmFormWidgetClass,
                           FC_ScrolledWindow,
                           NULL );
        } else {
           _UxParent = XtVaCreatePopupShell( "FileCharacteristics_shell",
                        xmDialogShellWidgetClass, _UxParent,
                        XmNx, 398,
                        XmNy, 222,
                        XmNmwmDecorations, MWM_DECOR_ALL | MWM_DECOR_RESIZEH,
                        XmNwidth, 446,
                        XmNheight, 579,
                        XmNshellUnitType, XmPIXELS,
                        XmNtitle, GETMESSAGE(9, 10, "Identifying Characteristics"),
                        NULL );
           FileCharacteristics = XtVaCreateWidget( "FileCharacteristics",
                        xmFormWidgetClass,
                        _UxParent,
                        XmNunitType, XmPIXELS,
                        XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
                        RES_CONVERT( XmNdialogTitle, GETMESSAGE(9, 10, "Identifying Characteristics")),
                        XmNautoUnmanage, FALSE,
                        XmNrubberPositioning, FALSE,
                        NULL );
        }
        XtAddCallback( FileCharacteristics, XmNhelpCallback,
                (XtCallbackProc) helpCB_general,
                (XtPointer) HELP_FILECHAR );

        /* Creation of FC_MainLabel1 */
        FC_MainLabel1 = XtVaCreateManagedWidget( "FC_MainLabel1",
                        xmLabelWidgetClass,
                        FileCharacteristics,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 11, "Include All")),
                        XmNleftOffset, 40,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopOffset, 15,
                        XmNtopAttachment, XmATTACH_FORM,
                        NULL );

        /* Creation of FC_TypeFileRowColumn */
        FC_TypeFileRowColumn = XtVaCreateManagedWidget( "FC_TypeFileRowColumn",
                        xmRowColumnWidgetClass,
                        FileCharacteristics,
                        XmNradioBehavior, TRUE,
                        XmNsensitive, TRUE,
                        XmNnumColumns, 2,
                        XmNorientation, XmVERTICAL,
                        XmNpacking, XmPACK_COLUMN,
                        XmNshadowThickness, 1,
                        XmNleftWidget, FC_MainLabel1,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNtopOffset, 5,
                        XmNtopWidget, FC_MainLabel1,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_FileToggle */
        FC_FileToggle = XtVaCreateManagedWidget( "FC_FileToggle",
                        xmToggleButtonGadgetClass,
                        FC_TypeFileRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 12, "Files")),
                        XmNset, TRUE,
                        NULL );

        /* Creation of FC_DirectoryToggle */
        FC_DirectoryToggle = XtVaCreateManagedWidget( "FC_DirectoryToggle",
                        xmToggleButtonGadgetClass,
                        FC_TypeFileRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 13, "Folders")),
                        NULL );

        /* Creation of FC_MainLabel2 */
        FC_MainLabel2 = XtVaCreateManagedWidget( "FC_MainLabel2",
                        xmLabelWidgetClass,
                        FileCharacteristics,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 14, "That Match the Following:")),
                        XmNleftOffset, 40,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopOffset, 5,
                        XmNtopWidget, FC_TypeFileRowColumn,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_NamePatternLabel */
        FC_NamePatternLabel = XtVaCreateManagedWidget( "FC_NamePatternLabel",
                        xmLabelWidgetClass,
                        FileCharacteristics,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 20, "Name Pattern:")),
                        XmNleftWidget, FC_MainLabel2,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNrightAttachment, XmATTACH_NONE,
                        XmNtopOffset, 20,
                        XmNtopWidget, FC_MainLabel2,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_NameOrPathText */
        FC_NameOrPathText = XtVaCreateManagedWidget( "FC_NameOrPathText",
                        xmTextFieldWidgetClass,
                        FileCharacteristics,
                        XmNx, 59,
                        XmNy, 66,
                        XmNleftWidget, FC_NamePatternLabel,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNtopOffset, 0,
                        XmNtopWidget, FC_NamePatternLabel,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_AndLabel1 */
        FC_AndLabel1 = XtVaCreateManagedWidget( "FC_AndLabel1",
                        xmLabelWidgetClass,
                        FileCharacteristics,
                        XmNx, 420,
                        XmNy, 98,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 27, "AND")),
#ifdef NOT_ILS
                        XmNfontList, UxConvertFontList( "-adobe-times-bold-r-normal--25-180-100-100-p-132-iso8859-1" ),
#endif
                        XmNsensitive, FALSE,
                        XmNrightOffset, 30,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopOffset, 0,
                        XmNtopWidget, FC_NamePatternLabel,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        XtVaSetValues(FC_NameOrPathText,
                      XmNrightOffset, 30,
                      XmNrightWidget, FC_AndLabel1,
                      XmNrightAttachment, XmATTACH_WIDGET,
                      NULL);


        /* Creation of FC_NameOrPathToggle */
        FC_NameOrPathToggle = XtVaCreateManagedWidget( "FC_NameOrPathToggle",
                        xmToggleButtonGadgetClass,
                        FileCharacteristics,
                        RES_CONVERT( XmNlabelString, "" ),
                        XmNset, TRUE,
                        XmNnavigationType, XmTAB_GROUP,
                        XmNindicatorSize, 20,
                        XmNleftAttachment, XmATTACH_NONE,
                        XmNrightOffset, 2,
                        XmNrightWidget, FC_NameOrPathText,
                        XmNrightAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, FC_NameOrPathText,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNbottomAttachment, XmATTACH_NONE,
                        NULL );
        XtAddCallback( FC_NameOrPathToggle, XmNvalueChangedCallback,
                (XtCallbackProc) valueChangedCB_FC_NameOrPathToggle,
                (XtPointer) NULL );

        /******************************************************************/
        /*                                                                */
        /* Permission Area creation        <<<<<<<>>>>>>>>                */
        /*                                                                */
        /******************************************************************/
        /* Creation of FC_PermissionLabel */
        FC_PermissionLabel = XtVaCreateManagedWidget( "FC_PermissionLabel",
                        xmLabelWidgetClass,
                        FileCharacteristics,
                        XmNx, 50,
                        XmNy, 155,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 21, "Permission Pattern:")),
                        XmNsensitive, FALSE,
                        XmNleftWidget, FC_NameOrPathText,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNrightAttachment, XmATTACH_NONE,
                        XmNtopOffset, 15,
                        XmNtopWidget, FC_NameOrPathText,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        FC_PermissionForm = XtVaCreateManagedWidget( "FC_PermissionForm",
                        xmFormWidgetClass,
                        FileCharacteristics,
                        XmNresizePolicy, XmRESIZE_NONE,
                        XmNborderWidth, 1,
                        XmNsensitive, FALSE,
                        XmNleftWidget, FC_PermissionLabel,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNtopOffset, 0,
                        XmNtopWidget, FC_PermissionLabel,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /******************************************************************/
        /* Creation of FC_ReadRowColumn                                   */
        /*                                                                */
        /* This is the read row column - on, off, either radio buttons    */
        /******************************************************************/
        FC_ReadRowColumn = XtVaCreateManagedWidget( "FC_ReadRowColumn",
                        xmRowColumnWidgetClass,
                        FC_PermissionForm,
                        XmNradioBehavior, TRUE,
                        XmNnumColumns, 3,
                        XmNorientation, XmVERTICAL,
                        XmNpacking, XmPACK_COLUMN,
                        XmNshadowThickness, 1,
                        XmNrightOffset, 10,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopOffset, 10,
                        XmNtopAttachment, XmATTACH_FORM,
                        NULL );

        /* Creation of FC_ReadOnToggle */
        FC_ReadOnToggle = XtVaCreateManagedWidget( "FC_ReadOnToggle",
                        xmToggleButtonGadgetClass,
                        FC_ReadRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 43, "On")),
                        XmNset, FALSE,
                        NULL );

        /* Creation of FC_ReadOffToggle */
        FC_ReadOffToggle = XtVaCreateManagedWidget( "FC_ReadOffToggle",
                        xmToggleButtonGadgetClass,
                        FC_ReadRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 44, "Off")),
                        XmNset, FALSE,
                        NULL );

        /* Creation of FC_ReadEitherToggle */
        FC_ReadEitherToggle = XtVaCreateManagedWidget( "FC_ReadEitherToggle",
                        xmToggleButtonGadgetClass,
                        FC_ReadRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 45, "Either")),
                        XmNset, TRUE,
                        NULL );

        /******************************************************************/
        /* Creation of FC_WriteRowColumn                                  */
        /*                                                                */
        /* This is the write row column - on, off, either radio buttons   */
        /******************************************************************/
        FC_WriteRowColumn = XtVaCreateManagedWidget( "FC_WriteRowColumn",
                        xmRowColumnWidgetClass,
                        FC_PermissionForm,
                        XmNradioBehavior, TRUE,
                        XmNnumColumns, 3,
                        XmNorientation, XmVERTICAL,
                        XmNpacking, XmPACK_COLUMN,
                        XmNshadowThickness, 1,
                        XmNrightOffset, 10,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopWidget, FC_ReadRowColumn,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_WriteOnToggle */
        FC_WriteOnToggle = XtVaCreateManagedWidget( "FC_WriteOnToggle",
                        xmToggleButtonGadgetClass,
                        FC_WriteRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 43, "On")),
                        XmNset, FALSE,
                        NULL );

        /* Creation of FC_WriteOffToggle */
        FC_WriteOffToggle = XtVaCreateManagedWidget( "FC_WriteOffToggle",
                        xmToggleButtonGadgetClass,
                        FC_WriteRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 44, "Off")),
                        XmNset, FALSE,
                        NULL );

        /* Creation of FC_WriteEitherToggle */
        FC_WriteEitherToggle = XtVaCreateManagedWidget( "FC_WriteEitherToggle",
                        xmToggleButtonGadgetClass,
                        FC_WriteRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 45, "Either")),
                        XmNset, TRUE,
                        NULL );

        /******************************************************************/
        /* Creation of FC_ExecuteRowColumn                                */
        /*                                                                */
        /* This is the execute row column - on, off, either radio buttons */
        /******************************************************************/
        FC_ExecuteRowColumn = XtVaCreateManagedWidget( "FC_ExecuteRowColumn",
                        xmRowColumnWidgetClass,
                        FC_PermissionForm,
                        XmNradioBehavior, TRUE,
                        XmNnumColumns, 3,
                        XmNorientation, XmVERTICAL,
                        XmNpacking, XmPACK_COLUMN,
                        XmNshadowThickness, 1,
                        XmNrightOffset, 10,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopWidget, FC_WriteRowColumn,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        XmNbottomOffset, 10,
                        XmNbottomAttachment, XmATTACH_FORM,
                        NULL );

        /* Creation of FC_ExecuteOnToggle */
        FC_ExecuteOnToggle = XtVaCreateManagedWidget( "FC_ExecuteOnToggle",
                        xmToggleButtonGadgetClass,
                        FC_ExecuteRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 43, "On")),
                        XmNset, FALSE,
                        NULL );

        /* Creation of FC_ExecuteOffToggle */
        FC_ExecuteOffToggle = XtVaCreateManagedWidget( "FC_ExecuteOffToggle",
                        xmToggleButtonGadgetClass,
                        FC_ExecuteRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 44, "Off")),
                        XmNset, FALSE,
                        NULL );

        /* Creation of FC_ExecuteEitherToggle */
        FC_ExecuteEitherToggle = XtVaCreateManagedWidget( "FC_ExecuteEitherToggle",
                        xmToggleButtonGadgetClass,
                        FC_ExecuteRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 45, "Either")),
                        XmNset, TRUE,
                        NULL );

        /******************************************************************/
        /* Creation of Permission Labels                                  */
        /******************************************************************/
        FC_ReadLabel = XtVaCreateManagedWidget( "FC_ReadLabel",
                        xmLabelWidgetClass,
                        FC_PermissionForm,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 40, "Read:")),
                        XmNrightWidget, FC_ReadRowColumn,
                        XmNrightAttachment, XmATTACH_WIDGET,
                        XmNleftOffset, 10,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNtopWidget, FC_ReadRowColumn,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        NULL );
        FC_WriteLabel = XtVaCreateManagedWidget( "FC_WriteLabel",
                        xmLabelWidgetClass,
                        FC_PermissionForm,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 41, "Write:")),
                        XmNrightWidget, FC_WriteRowColumn,
                        XmNrightAttachment, XmATTACH_WIDGET,
                        XmNleftOffset, 10,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNtopWidget, FC_WriteRowColumn,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        NULL );
        FC_ExecuteLabel = XtVaCreateManagedWidget( "FC_ExecuteLabel",
                        xmLabelWidgetClass,
                        FC_PermissionForm,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 42, "Execute:")),
                        XmNrightWidget, FC_ExecuteRowColumn,
                        XmNrightAttachment, XmATTACH_WIDGET,
                        XmNleftOffset, 10,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNtopWidget, FC_ExecuteRowColumn,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        NULL );

        /******************************************************************/
        /* Creation of the  AND  label for permissions.                   */
        /******************************************************************/
        FC_AndLabel2 = XtVaCreateManagedWidget( "FC_AndLabel2",
                        xmLabelWidgetClass,
                        FileCharacteristics,
                        XmNx, 420,
                        XmNy, 192,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 27, "AND")),
#ifdef NOT_ILS
                        XmNfontList, UxConvertFontList( "-adobe-times-bold-r-normal--25-180-100-100-p-132-iso8859-1" ),
#endif
                        XmNsensitive, FALSE,
                        XmNrightOffset, 30,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopOffset, 0,
                        XmNtopWidget, FC_PermissionLabel,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        XtVaSetValues(FC_PermissionForm,
                      XmNrightOffset, 30,
                      XmNrightWidget, FC_AndLabel2,
                      XmNrightAttachment, XmATTACH_WIDGET,
                      NULL);


        /* Creation of FC_PermissionToggle */
        FC_PermissionToggle = XtVaCreateManagedWidget( "FC_PermissionToggle",
                        xmToggleButtonGadgetClass,
                        FileCharacteristics,
                        RES_CONVERT( XmNlabelString, "" ),
                        XmNnavigationType, XmTAB_GROUP,
                        XmNindicatorSize, 20,
                        XmNleftAttachment, XmATTACH_NONE,
                        XmNrightOffset, 2,
                        XmNrightWidget, FC_PermissionForm,
                        XmNrightAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, FC_PermissionForm,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNbottomAttachment, XmATTACH_NONE,
                        NULL );
        XtAddCallback( FC_PermissionToggle, XmNvalueChangedCallback,
                (XtCallbackProc) valueChangedCB_FC_PermissionToggle,
                (XtPointer) NULL );

        /* Creation of FC_ContentsLabel */
        FC_ContentsLabel = XtVaCreateManagedWidget( "FC_ContentsLabel",
                        xmLabelWidgetClass,
                        FileCharacteristics,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 28, "Contents:")),
                        XmNsensitive, FALSE,
                        XmNleftWidget, FC_PermissionForm,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNrightAttachment, XmATTACH_NONE,
                        XmNtopOffset, 15,
                        XmNtopWidget, FC_PermissionForm,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_ContentsBox */
        FC_ContentsBox = XtVaCreateManagedWidget( "FC_ContentsBox",
                        xmFormWidgetClass,
                        FileCharacteristics,
                        XmNresizePolicy, XmRESIZE_NONE,
                        XmNborderWidth, 1,
                        XmNsensitive, FALSE,
                        XmNleftWidget, FC_ContentsLabel,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNrightOffset, 30,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopOffset, 0,
                        XmNtopWidget, FC_ContentsLabel,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_ContentsToggle */
        FC_ContentsToggle = XtVaCreateManagedWidget( "FC_ContentsToggle",
                        xmToggleButtonGadgetClass,
                        FileCharacteristics,
                        RES_CONVERT( XmNlabelString, "" ),
                        XmNnavigationType, XmTAB_GROUP,
                        XmNindicatorSize, 20,
                        XmNleftAttachment, XmATTACH_NONE,
                        XmNrightOffset, 2,
                        XmNrightWidget, FC_ContentsBox,
                        XmNrightAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, FC_ContentsBox,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNbottomAttachment, XmATTACH_NONE,
                        NULL );
        XtAddCallback( FC_ContentsToggle, XmNvalueChangedCallback,
                (XtCallbackProc) valueChangedCB_FC_ContentsToggle,
                (XtPointer) NULL );

        /* Creation of FC_ContentsPatternLabel */
        FC_ContentsPatternLabel = XtVaCreateManagedWidget( "FC_ContentsPatternLabel",
                        xmLabelWidgetClass,
                        FC_ContentsBox,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 29, "Pattern:")),
                        XmNleftOffset, 10,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNrightAttachment, XmATTACH_NONE,
                        XmNtopOffset, 10,
                        XmNtopAttachment, XmATTACH_FORM,
                        NULL );

        /* Creation of FC_ContentsPatternText */
        FC_ContentsPatternText = XtVaCreateManagedWidget( "FC_ContentsPatternText",
                        xmTextFieldWidgetClass,
                        FC_ContentsBox,
                        XmNwidth, 300,
                        XmNleftOffset, 5,
                        XmNleftWidget, FC_ContentsPatternLabel,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNrightOffset, 10,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNbottomAttachment, XmATTACH_NONE,
                        XmNtopWidget, FC_ContentsPatternLabel,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        NULL );

        /* Creation of FC_TypeLabel */
        FC_TypeLabel = XtVaCreateManagedWidget( "FC_TypeLabel",
                        xmLabelWidgetClass,
                        FC_ContentsBox,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 30, "Type:")),
                        XmNleftWidget, FC_ContentsPatternLabel,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNbottomAttachment, XmATTACH_NONE,
                        XmNtopOffset, 15,
                        XmNtopWidget, FC_ContentsPatternLabel,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_TypeRowColumn */
        FC_TypeRowColumn = XtVaCreateManagedWidget( "FC_TypeRowColumn",
                        xmRowColumnWidgetClass,
                        FC_ContentsBox,
                        XmNnumColumns, 1,
                        XmNpacking, XmPACK_COLUMN,
                        XmNradioBehavior, TRUE,
                        XmNwhichButton, 1,
                        XmNleftWidget, FC_ContentsPatternText,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNrightWidget, FC_ContentsPatternText,
                        XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNbottomAttachment, XmATTACH_NONE,
                        XmNtopWidget, FC_TypeLabel,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        NULL );

        /* Creation of FC_StringToggle */
        FC_StringToggle = XtVaCreateManagedWidget( "FC_StringToggle",
                        xmToggleButtonGadgetClass,
                        FC_TypeRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 31, "String")),
                        XmNset, TRUE,
                        NULL );

        /* Creation of FC_ByteToggle */
        FC_ByteToggle = XtVaCreateManagedWidget( "FC_ByteToggle",
                        xmToggleButtonGadgetClass,
                        FC_TypeRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 32, "Byte (Hex)")),
                        NULL );

        /* Creation of FC_ShortToggle */
        FC_ShortToggle = XtVaCreateManagedWidget( "FC_ShortToggle",
                        xmToggleButtonGadgetClass,
                        FC_TypeRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 33, "Short (Decimal)")),
                        NULL );

        /* Creation of FC_LongToggle */
        FC_LongToggle = XtVaCreateManagedWidget( "FC_LongToggle",
                        xmToggleButtonGadgetClass,
                        FC_TypeRowColumn,
                        XmNindicatorType, XmONE_OF_MANY,
                        XmNindicatorSize, 17,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 34, "Long (Decimal)")),
                        NULL );

        /* Creation of FC_StartByteLabel */
        FC_StartByteLabel = XtVaCreateManagedWidget( "FC_StartByteLabel",
                        xmLabelWidgetClass,
                        FC_ContentsBox,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(9, 35, "Start Byte:")),
                        XmNleftWidget, FC_TypeLabel,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNbottomOffset, 15,
                        XmNbottomAttachment, XmATTACH_FORM,
                        XmNtopOffset, 15,
                        XmNtopWidget, FC_TypeRowColumn,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_StartByteTextField */
        FC_StartByteTextField = XtVaCreateManagedWidget( "FC_StartByteTextField",
                        xmTextFieldWidgetClass,
                        FC_ContentsBox,
#ifdef NOT_ILS
                        XmNcolumns, 6,
#endif
                        XmNcursorPosition, 5,
                        XmNwidth, 80,
                        XmNleftOffset, 5,
                        XmNleftWidget, FC_StartByteLabel,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, FC_StartByteLabel,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        NULL );
#if 0
        /* Creation of FC_EndByteLabel */
        FC_EndByteLabel = XtVaCreateManagedWidget( "FC_EndByteLabel",
                        xmLabelWidgetClass,
                        FC_ContentsBox,
                        XmNx, 248,
                        XmNy, 189,
                        XmNalignment, XmALIGNMENT_BEGINNING,
                        RES_CONVERT( XmNlabelString, "End byte:" ),
                        XmNleftOffset, 10,
                        XmNleftWidget, FC_StartByteTextField,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, FC_StartByteTextField,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        NULL );

        /* Creation of FC_EndByteTextField */
        FC_EndByteTextField = XtVaCreateManagedWidget( "FC_EndByteTextField",
                        xmTextFieldWidgetClass,
                        FC_ContentsBox,
                        XmNcolumns, 6,
                        XmNcursorPosition, 5,
                        XmNwidth, 80,
                        XmNleftOffset, 5,
                        XmNleftWidget, FC_EndByteLabel,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNrightOffset, 10,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNtopWidget, FC_EndByteLabel,
                        XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
                        NULL );
#endif

        FC_ActionAreaForm = XtVaCreateManagedWidget( "FC_ActionAreaForm",
                        xmFormWidgetClass,
                        FileCharacteristics,
                        XmNskipAdjust, TRUE,
                        XmNfractionBase, ((TIGHTNESS * ntotalbuttons) - 1),
                        XmNleftWidget, FC_ContentsBox,
                        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                        XmNrightOffset, 30,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNbottomOffset, 20,
                        XmNbottomAttachment, XmATTACH_FORM,
                        XmNtopOffset, 20,
                        XmNtopWidget, FC_ContentsBox,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        NULL );

        /* Creation of FC_OkButton */
        FC_OkButton = XtVaCreateManagedWidget( "FC_OkButton",
                        xmPushButtonGadgetClass,
                        FC_ActionAreaForm,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(6, 10, "OK")),
                        XmNleftAttachment, nbutton ?
                                            XmATTACH_POSITION : XmATTACH_FORM,
                        XmNleftPosition, TIGHTNESS * nbutton,
                        XmNrightAttachment, nbutton != (ntotalbuttons - 1) ?
                                             XmATTACH_POSITION : XmATTACH_FORM,
                        XmNrightPosition, (TIGHTNESS * nbutton)+(TIGHTNESS - 1),
                        NULL );
        nbutton++;
        XtAddCallback( FC_OkButton, XmNactivateCallback,
                (XtCallbackProc) activateCB_FC_OkButton,
                (XtPointer) NULL );

        /* Creation of FC_CancelButton */
        FC_CancelButton = XtVaCreateManagedWidget( "FC_CancelButton",
                        xmPushButtonGadgetClass,
                        FC_ActionAreaForm,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(6, 12, "Cancel")),
                        XmNleftAttachment, nbutton ?
                                            XmATTACH_POSITION : XmATTACH_FORM,
                        XmNleftPosition, TIGHTNESS * nbutton,
                        XmNrightAttachment, nbutton != (ntotalbuttons - 1) ?
                                             XmATTACH_POSITION : XmATTACH_FORM,
                        XmNrightPosition, (TIGHTNESS * nbutton)+(TIGHTNESS - 1),
                        NULL );
        nbutton++;
        XtAddCallback( FC_CancelButton, XmNactivateCallback,
                (XtCallbackProc) activateCB_FC_CancelButton,
                (XtPointer) NULL );

        /* Creation of FC_ClearButton */
        FC_ClearButton = XtVaCreateManagedWidget( "FC_ClearButton",
                        xmPushButtonGadgetClass,
                        FC_ActionAreaForm,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(6, 14, "Clear")),
                        XmNleftAttachment, nbutton ?
                                            XmATTACH_POSITION : XmATTACH_FORM,
                        XmNleftPosition, TIGHTNESS * nbutton,
                        XmNrightAttachment, nbutton != (ntotalbuttons - 1) ?
                                             XmATTACH_POSITION : XmATTACH_FORM,
                        XmNrightPosition, (TIGHTNESS * nbutton)+(TIGHTNESS - 1),
                        NULL );
        nbutton++;
        XtAddCallback( FC_ClearButton, XmNactivateCallback,
                (XtCallbackProc) activateCB_FC_ClearButton,
                (XtPointer) NULL );

        /* Creation of FC_HelpButton */
        FC_HelpButton = XtVaCreateManagedWidget( "FC_HelpButton",
                        xmPushButtonGadgetClass,
                        FC_ActionAreaForm,
                        RES_CONVERT( XmNlabelString, GETMESSAGE(6, 13, "Help")),
                        XmNuserData, FileCharacteristics,
                        XmNleftAttachment, nbutton ?
                                            XmATTACH_POSITION : XmATTACH_FORM,
                        XmNleftPosition, TIGHTNESS * nbutton,
                        XmNrightAttachment, nbutton != (ntotalbuttons - 1) ?
                                             XmATTACH_POSITION : XmATTACH_FORM,
                        XmNrightPosition, (TIGHTNESS * nbutton)+(TIGHTNESS - 1),
                        NULL );
        nbutton++;
        XtAddCallback( FC_HelpButton, XmNactivateCallback,
                (XtCallbackProc) helpCB_general,
                (XtPointer) HELP_FILECHAR );

        XtVaSetValues(FileCharacteristics,
                        XmNcancelButton, FC_CancelButton,
                        XmNdefaultButton, FC_OkButton,
                        XmNinitialFocus, FC_NameOrPathText,
                        NULL );

        XtAddCallback( FileCharacteristics, XmNdestroyCallback,
                (XtCallbackProc) UxDestroyContextCB,
                (XtPointer) NULL );

        if (bLowRes) {
           return ( FC_BigForm );
        } else {
           return ( FileCharacteristics );
        }
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

#ifdef _NO_PROTO
Widget  create_FileCharacteristics( _UxUxParent )
        swidget _UxUxParent;
#else /* _NO_PROTO */
Widget  create_FileCharacteristics( swidget _UxUxParent )
#endif /* _NO_PROTO */
{
        Widget            rtrn;
        Dimension         dimHeight;
        Dimension         dimWidth;
        Dimension         dimTmp;
        Widget            widScrolledWindow;
        Widget            widTmp;

        rtrn = _Uxbuild_FileCharacteristics();

        if (bLowRes) {
           XtRealizeWidget(rtrn);
           XtVaGetValues (FileCharacteristics,
                          XmNheight, &dimHeight,
                          XmNwidth, &dimWidth,
                          NULL);
           XtVaGetValues (rtrn,
                          XmNuserData, &widScrolledWindow,
                          NULL);
           XtVaGetValues (widScrolledWindow,
                          XmNverticalScrollBar, &widTmp,
                          NULL);
           XtVaGetValues (widTmp,
                          XmNwidth, &dimTmp,
                          NULL);
           dimHeight += dimTmp;
           dimWidth += dimTmp;

           XtVaSetValues (rtrn,
                          XmNheight, dimHeight,
                          XmNwidth, dimWidth,
                          NULL);
        }

        return(rtrn);
}

/******************************************************************************/
/*                                                                            */
/* init_FileCharacteristics_dialog_fields                                     */
/*                                                                            */
/* INPUT:  FiletypeData * - pointer to FiletypeData structure                 */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void init_FileCharacteristics_dialog_fields(pFtD)
     FiletypeData *pFtD;
#else /* _NO_PROTO */
void init_FileCharacteristics_dialog_fields(FiletypeData *pFtD)
#endif /* _NO_PROTO */
{
  char buffer[MAXBUFSIZE];
  char *pszPerms = (char *)NULL;

  /************************************************************************/
  /* Initialize name pattern.                                             */
  /************************************************************************/
  if (pFtD->pszPattern) {
     /*XmToggleButtonSetState(FC_NameOrPathToggle, TRUE);*/
     XmToggleButtonGadgetSetState(FC_NameOrPathToggle, TRUE, TRUE);
     PutWidgetTextString(FC_NameOrPathText, pFtD->pszPattern);
  } else {
     /*XmToggleButtonSetState(FC_NameOrPathToggle, FALSE);*/
     XmToggleButtonGadgetSetState(FC_NameOrPathToggle, FALSE, TRUE);
  }

  /************************************************************************/
  /* Initialize permission pattern.                                       */
  /************************************************************************/
  if (pFtD->pszPermissions) {
     if (strstr(pFtD->pszPermissions, "!d")) {
        XmToggleButtonGadgetSetState(FC_FileToggle, TRUE, TRUE);
     } else {
        XmToggleButtonGadgetSetState(FC_DirectoryToggle, TRUE, TRUE);
     }
     pszPerms = strchr(pFtD->pszPermissions, 'd');
     if (pszPerms) {
        pszPerms++;
        if (!*pszPerms) {
           pszPerms = (char *)NULL;
        }
     }
  }

  if (pszPerms) {
     XmToggleButtonGadgetSetState(FC_PermissionToggle, TRUE, TRUE);

     /******************************************************************/
     /* Initialize Read permission                                     */
     /******************************************************************/
     if (strstr(pFtD->pszPermissions, "!r")) {
        XmToggleButtonGadgetSetState(FC_ReadOffToggle, TRUE, TRUE);
     } else {
        if (strstr(pFtD->pszPermissions, "r")) {
           XmToggleButtonGadgetSetState(FC_ReadOnToggle, TRUE, TRUE);
        } else {
           XmToggleButtonGadgetSetState(FC_ReadEitherToggle, TRUE, TRUE);
        }
     }
     /******************************************************************/
     /* Initialize Write permission                                    */
     /******************************************************************/
     if (strstr(pFtD->pszPermissions, "!w")) {
        XmToggleButtonGadgetSetState(FC_WriteOffToggle, TRUE, TRUE);
     } else {
        if (strstr(pFtD->pszPermissions, "w")) {
           XmToggleButtonGadgetSetState(FC_WriteOnToggle, TRUE, TRUE);
        } else {
           XmToggleButtonGadgetSetState(FC_WriteEitherToggle, TRUE, TRUE);
        }
     }
     /******************************************************************/
     /* Initialize Execute permission                                  */
     /******************************************************************/
     if (strstr(pFtD->pszPermissions, "!x")) {
        XmToggleButtonGadgetSetState(FC_ExecuteOffToggle, TRUE, TRUE);
     } else {
       if (strstr(pFtD->pszPermissions, "x")) {
          XmToggleButtonGadgetSetState(FC_ExecuteOnToggle, TRUE, TRUE);
       } else {
          XmToggleButtonGadgetSetState(FC_ExecuteEitherToggle, TRUE, TRUE);
       }
     }

  } else {
     XmToggleButtonGadgetSetState(FC_PermissionToggle, FALSE, TRUE);
  }

  /************************************************************************/
  /* Initialize contents pattern.                                         */
  /************************************************************************/
  if (pFtD->pszContents) {
     XmToggleButtonGadgetSetState(FC_ContentsToggle, TRUE, TRUE);
     /*XmToggleButtonSetState(FC_ContentsToggle, TRUE);*/
     PutWidgetTextString(FC_ContentsPatternText, pFtD->pszContents);

     if (pFtD->fsFlags & CA_FT_CNTSTRING)
        XmToggleButtonGadgetSetState(FC_StringToggle, TRUE, TRUE);
     else if (pFtD->fsFlags & CA_FT_CNTBYTE)
        XmToggleButtonGadgetSetState(FC_ByteToggle, TRUE, TRUE);
     else if (pFtD->fsFlags & CA_FT_CNTSHORT)
        XmToggleButtonGadgetSetState(FC_ShortToggle, TRUE, TRUE);
     else if (pFtD->fsFlags & CA_FT_CNTLONG)
        XmToggleButtonGadgetSetState(FC_LongToggle, TRUE, TRUE);

     sprintf(buffer, "%d", pFtD->sStart);
     PutWidgetTextString(FC_StartByteTextField,buffer);
#if 0
     sprintf(buffer, "%d", pFtD->sEnd);
     PutWidgetTextString(FC_EndByteTextField, buffer);
#endif
  } else {
     XmToggleButtonGadgetSetState(FC_ContentsToggle, FALSE, TRUE);
     /*XmToggleButtonSetState(FC_ContentsToggle, FALSE);*/
  }
  return;
}

/******************************************************************************/
/*                                                                            */
/* clear_FileCharacteristics_dialog_fields                                    */
/*                                                                            */
/* INPUT:  FiletypeData * - pointer to FiletypeData structure                 */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void clear_FileCharacteristics_dialog_fields()
#else /* _NO_PROTO */
void clear_FileCharacteristics_dialog_fields(void)
#endif /* _NO_PROTO */
{
  /************************************************************************/
  /* Clear Files/Directories - set to default                             */
  /************************************************************************/
  XmToggleButtonGadgetSetState(FC_FileToggle, TRUE, TRUE);

  /************************************************************************/
  /* Clear name pattern.                                                  */
  /************************************************************************/
  XmToggleButtonGadgetSetState(FC_NameOrPathToggle, FALSE, TRUE);
  clear_text_field(FC_NameOrPathText);

  /************************************************************************/
  /* Clear permission pattern - set to default                            */
  /************************************************************************/
  XmToggleButtonGadgetSetState(FC_PermissionToggle, FALSE, TRUE);
  XmToggleButtonGadgetSetState(FC_ReadOnToggle, FALSE, TRUE);
  XmToggleButtonGadgetSetState(FC_WriteOnToggle, FALSE, TRUE);
  XmToggleButtonGadgetSetState(FC_ExecuteOnToggle, FALSE, TRUE);

  XmToggleButtonGadgetSetState(FC_ReadOffToggle, FALSE, TRUE);
  XmToggleButtonGadgetSetState(FC_WriteOffToggle, FALSE, TRUE);
  XmToggleButtonGadgetSetState(FC_ExecuteOffToggle, FALSE, TRUE);

  XmToggleButtonGadgetSetState(FC_ReadEitherToggle, TRUE, TRUE);
  XmToggleButtonGadgetSetState(FC_WriteEitherToggle, TRUE, TRUE);
  XmToggleButtonGadgetSetState(FC_ExecuteEitherToggle, TRUE, TRUE);

  /************************************************************************/
  /* Clear contents pattern - set to default                              */
  /************************************************************************/
  XmToggleButtonGadgetSetState(FC_ContentsToggle, FALSE, TRUE);
  clear_text_field(FC_ContentsPatternText);
  XmToggleButtonGadgetSetState(FC_StringToggle, TRUE, TRUE);
  clear_text_field(FC_StartByteTextField);
#if 0
  clear_text_field(FC_EndByteTextField);
#endif

  return;
}

/******************************************************************************/
/*                                                                            */
/* FileCharCheckFields                                                        */
/*                                                                            */
/* INPUT:  none                                                               */
/* OUTPUT: FALSE  - no errors found                                           */
/*         TRUE   - found errors                                              */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
Boolean FileCharCheckFields()
#else /* _NO_PROTO */
Boolean FileCharCheckFields(void)
#endif /* _NO_PROTO */
{
  char    *ptr = (char *)NULL;
  Boolean bError = FALSE;
  char    buffer[MAXBUFSIZE];

  /**************************************************************************/
  /* Check to make sure at least one characteristic is selected.            */
  /**************************************************************************/
  if (!bError) {
     if ( !(XmToggleButtonGadgetGetState(FC_NameOrPathToggle)) &&
          !(XmToggleButtonGadgetGetState(FC_PermissionToggle)) &&
          !(XmToggleButtonGadgetGetState(FC_ContentsToggle)) ) {
        strcpy(buffer, GETMESSAGE(10, 30, "An identifying characteristic has not been selected.\nSelect one or more of the characteristics\n(Name Pattern, Permissions Pattern, or Contents)."));
        display_error_message(FileCharacteristics, buffer);
        XmProcessTraversal(FC_NameOrPathText, XmTRAVERSE_CURRENT);
        bError = TRUE;
     }
  }

  /**************************************************************************/
  /* Check if name pattern is selected.  If so must have a name pattern.    */
  /**************************************************************************/
  if (!bError) {
     if (XmToggleButtonGadgetGetState(FC_NameOrPathToggle)) {
        ptr = (char *)NULL;
        GetWidgetTextString(FC_NameOrPathText, &ptr);
#ifdef DEBUG
        printf("Name Pattern = '%s'\n", ptr);
#endif
        if (!ptr) {
           strcpy(buffer, GETMESSAGE(10, 10, "Name Pattern is missing.\nIf Name Pattern is selected, a name pattern\nmust be entered."));
           display_error_message(FileCharacteristics, buffer);
           XmProcessTraversal(FC_NameOrPathText, XmTRAVERSE_CURRENT);
           bError = TRUE;
        } else {
           XtFree(ptr);
        }
     }
  }

  /**************************************************************************/
  /* Check if contents is selected.  If so must have a contents pattern.    */
  /**************************************************************************/
  if (!bError) {
     if (XmToggleButtonGadgetGetState(FC_ContentsToggle)) {
        ptr = (char *)NULL;
        GetWidgetTextString(FC_ContentsPatternText, &ptr);
#ifdef DEBUG
        printf("Name Pattern = '%s'\n", ptr);
#endif
        if (!ptr) {
           strcpy(buffer, GETMESSAGE(10, 20, "Contents Pattern is missing.\nIf Contents is selected, then the Pattern field\nunder Contents must be filled in."));
           display_error_message(FileCharacteristics, buffer);
           bError = TRUE;
        } else {
           XtFree(ptr);
           /*****************************************************************/
           /* Now check to make sure there is a start byte value.           */
           /*****************************************************************/
           ptr = (char *)NULL;
           GetWidgetTextString(FC_StartByteTextField, &ptr);
           if (!ptr) {
              strcpy(buffer, GETMESSAGE(10, 25, "Start byte value is missing.\nIf Contents is selected, then a number must be\nentered in the 'Start Byte' field."));
              display_error_message(FileCharacteristics, buffer);
              bError = TRUE;
           } else {
              XtFree(ptr);
           }
        }
     }
  }

  return(bError);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

