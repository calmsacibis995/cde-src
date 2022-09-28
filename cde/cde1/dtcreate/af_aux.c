/* $XConsortium: af_aux.c /main/cde1_maint/2 1995/10/08 00:11:01 pascale $ */
/******************************************************************************/
/*                                                                            */
/* af_aux - auxiliary functions for add filetype                              */
/*                                                                            */
/* Functions to get/set values from/to interface.                             */
/* Callback routines for AF interface.                                        */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <errno.h>
#include <unistd.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Xm/Xm.h>
#include <Xm/TextF.h>
#include <Xm/RowColumnP.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <sys/stat.h>
#ifdef AIXV3
#include <sys/dir.h>
#else
#ifdef __osf__
#include <dirent.h>
#else
#include <sys/dirent.h>
#endif
#endif

#include <Dt/Icon.h>

#include "dtcreate.h"
#include "af_aux.h"
#include "cmnutils.h"
#include "cmnrtns.h"

/******************************************************************************/
/*                                                                            */
/*  External Variables                                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*  Variables                                                                 */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*         CALLBACK FUNCTIONS                                                 */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* get_selected_filetype_icon - determines which filetype icon is currently   */
/*                              selected.                                     */
/*                                                                            */
/* INPUT: none                                                                */
/* OUTPUT: Widget wid - id of selected icon gadget                            */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
Widget get_selected_filetype_icon ()
#else /* _NO_PROTO */
Widget get_selected_filetype_icon (void)
#endif /* _NO_PROTO */
{
  if (!last_filetype_pushed) {
    last_filetype_pushed = AF_MED_IconGadget;
  }
  return (last_filetype_pushed);
}

/******************************************************************************/
/*                                                                            */
/* activateCB_filetype_icon                                                   */
/*                                                                            */
/* INPUT: Widget wid - id of icon gadget                                      */
/*        XtPointer client_data - not used                                    */
/*        XmPushButtonCallbackStruct *cbs - not used                          */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void activateCB_filetype_icon (wid, client_data, cbs)
     Widget wid;
     XtPointer client_data;
     DtIconCallbackStruct *cbs;
#else /* _NO_PROTO */
void activateCB_filetype_icon (Widget wid, XtPointer client_data,
                               DtIconCallbackStruct *cbs)
#endif /* _NO_PROTO */
{
  Time    lts, mct = 0;
  static  Time prev_lts = 0;

  if (cbs->reason == XmCR_ACTIVATE) {

#ifdef DEBUG
    printf("In activate_filetype_icon callback.\n");
#endif
    if (last_filetype_pushed && (last_filetype_pushed != wid)) {
       XtVaSetValues(XtParent(last_filetype_pushed), XmNborderWidth, 0, NULL);
    }
    XtVaSetValues(XtParent(wid), XmNborderWidth, ICON_BORDER_WIDTH, NULL);

    last_filetype_pushed = wid;

    /********************************************************************/
    /* This is the support needed to provide double-click functionality */
    /* to the icon gadgets.  When double-clicked, the icon editor will  */
    /* be launched.                                                     */
    /********************************************************************/
    XtVaSetValues(wid, XmNpushButtonClickTime, 0, NULL);

    lts = XtLastTimestampProcessed(XtDisplay(wid));
    mct = XtGetMultiClickTime(XtDisplay(wid));

    if ((prev_lts + mct) > lts) {
       prev_lts = XtLastTimestampProcessed(XtDisplay(wid));
    } else {
       prev_lts = XtLastTimestampProcessed(XtDisplay(wid));
       return;
    }

#ifdef DEBUG
    printf("DblClick icon callback.\n");
#endif

    activateCB_edit_icon(wid, CA_FILETYPE_ICONS, cbs);

  }

  return;
}

/******************************************************************************/
/*                                                                            */
/*   readAFFromGUI -                                                          */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void readAFFromGUI (pFiletypedata)
       FiletypeData *pFiletypedata;
#else /* _NO_PROTO */
void readAFFromGUI (FiletypeData *pFiletypedata)
#endif /* _NO_PROTO */
{
  getAF_FiletypeName(pFiletypedata);
  getAF_HelpText(pFiletypedata);
  getAF_OpenCmd(pFiletypedata);
  getAF_PrintCmd(pFiletypedata);
  getAF_Icons(pFiletypedata);

  return;
}

/******************************************************************************/
/*                                                                            */
/* getAF_FiletypeName - get name of filetype family                           */
/*                                                                            */
/* INPUT: FiletypeData *pFiletypedata - pointer to filetype data structure    */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void getAF_FiletypeName (pFiletypedata)
     FiletypeData *pFiletypedata;
#else /* _NO_PROTO */
void getAF_FiletypeName (FiletypeData *pFiletypedata)
#endif /* _NO_PROTO */
{
  GetWidgetTextString(AF_FileTypeNameTextField, &(pFiletypedata->pszName));
  return;
}

/******************************************************************************/
/*                                                                            */
/* getAF_HelpText - retrieve the Help text for the filetype                   */
/*                                                                            */
/* INPUT: FiletypeData *pFiletypedata - pointer to filetype data structure    */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void getAF_HelpText (pFiletypedata)
     FiletypeData *pFiletypedata;
#else /* _NO_PROTO */
void getAF_HelpText (FiletypeData *pFiletypedata)
#endif /* _NO_PROTO */
{
  GetWidgetTextString(AF_FiletypeHelpText, &(pFiletypedata->pszHelp));
  return;
}

/******************************************************************************/
/*                                                                            */
/* getAF_Icons - store temporary icon names.                                  */
/*                                                                            */
/* INPUT: FiletypeData *pFiletypedata - pointer to filetype data structure    */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void getAF_Icons(pFiletypedata)
     FiletypeData *pFiletypedata;
#else /* _NO_PROTO */
void getAF_Icons(FiletypeData *pFiletypedata)
#endif /* _NO_PROTO */
{
  IconData  *pIconData;

  /***************************************************************/
  /* Medium Pixmap                                               */
  /***************************************************************/
  pIconData = GetIconDataFromWid(AF_MED_IconGadget);
  if ( (pIconData->pmDirtyBit) &&
       (pIconData->pmFileName) &&
       (strlen(pIconData->pmFileName)) ) {
     pFiletypedata->pszMedPmIcon = XtMalloc(strlen(pIconData->pmFileName) + 1);
     if (pFiletypedata->pszMedPmIcon) {
        strcpy(pFiletypedata->pszMedPmIcon, pIconData->pmFileName);
     }
  }
  pIconData->pmDirtyBit = False;
  strcpy(pIconData->pmFileName, "");

  /***************************************************************/
  /* Medium Bitmap                                               */
  /***************************************************************/
  if ( (pIconData->bmDirtyBit) &&
       (pIconData->bmFileName) &&
       (strlen(pIconData->bmFileName)) ) {
     pFiletypedata->pszMedBmIcon = XtMalloc(strlen(pIconData->bmFileName) + 1);
     if (pFiletypedata->pszMedBmIcon) {
        strcpy(pFiletypedata->pszMedBmIcon, pIconData->bmFileName);
     }
  }
  pIconData->bmDirtyBit = False;
  strcpy(pIconData->bmFileName, "");

  /***************************************************************/
  /* Tiny Pixmap                                                 */
  /***************************************************************/
  pIconData = GetIconDataFromWid(AF_TINY_IconGadget);
  if ( (pIconData->pmDirtyBit) &&
       (pIconData->pmFileName) &&
       (strlen(pIconData->pmFileName)) ) {
     pFiletypedata->pszTinyPmIcon = XtMalloc(strlen(pIconData->pmFileName) + 1);
     if (pFiletypedata->pszTinyPmIcon) {
        strcpy(pFiletypedata->pszTinyPmIcon, pIconData->pmFileName);
     }
  }
  pIconData->pmDirtyBit = False;
  strcpy(pIconData->pmFileName, "");

  /***************************************************************/
  /* Tiny Bitmap                                                 */
  /***************************************************************/
  if ( (pIconData->bmDirtyBit) &&
       (pIconData->bmFileName) &&
       (strlen(pIconData->bmFileName)) ) {
     pFiletypedata->pszTinyBmIcon = XtMalloc(strlen(pIconData->bmFileName) + 1);
     if (pFiletypedata->pszTinyBmIcon) {
        strcpy(pFiletypedata->pszTinyBmIcon, pIconData->bmFileName);
     }
  }
  pIconData->bmDirtyBit = False;
  strcpy(pIconData->bmFileName, "");


#if 0
  Widget    AF_wids[2];
  int       i;


  AF_wids[0] = AF_MED_IconGadget;
  AF_wids[1] = AF_TINY_IconGadget;

  for (i=0; i < 2; i++) {
     /* pixmap */
     pIconData = GetIconDataFromWid(AF_wids[i]);
     if ( (pIconData->pmDirtyBit) &&
          (pIconData->pmFileName) &&
          (strlen(pIconData->pmFileName)) ) {
        pFiletypedata->pszMedPmIcon = XtMalloc(strlen(pIconData->pmFileName) + 1);
        if (pFiletypedata->pszMedPmIcon) {
           strcpy(pFiletypedata->pszMedPmIcon, pIconData->pmFileName);
        }
     }
     pIconData->pmDirtyBit = False;
     strcpy(pIconData->pmFileName, "");

     /* bitmap */
     if ( (pIconData->bmDirtyBit) &&
          (pIconData->bmFileName) &&
          (strlen(pIconData->bmFileName)) ) {
        pFiletypedata->pszMedBmIcon = XtMalloc(strlen(pIconData->bmFileName) + 1);
        if (pFiletypedata->pszMedBmIcon) {
           strcpy(pFiletypedata->pszMedBmIcon, pIconData->bmFileName);
        }
     }
     pIconData->bmDirtyBit = False;
     strcpy(pIconData->bmFileName, "");
  }
#endif


}

/******************************************************************************/
/*                                                                            */
/* getAF_OpenCmd - retrieve the Open command string                           */
/*                                                                            */
/* INPUT: FiletypeData *pFiletypedata - pointer to filetype data structure    */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void getAF_OpenCmd (pFiletypedata)
     FiletypeData *pFiletypedata;
#else /* _NO_PROTO */
void getAF_OpenCmd (FiletypeData *pFiletypedata)
#endif /* _NO_PROTO */
{
  GetWidgetTextString(AF_OpenCmdText, &(pFiletypedata->pszOpenCmd));
  return;
}

/******************************************************************************/
/*                                                                            */
/* getAF_PrintCmd - retrieve Print command string                             */
/*                                                                            */
/* INPUT: FiletypeData *pFiletypedata - pointer to filetype data structure    */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void getAF_PrintCmd (pFiletypedata)
     FiletypeData *pFiletypedata;
#else /* _NO_PROTO */
void getAF_PrintCmd (FiletypeData *pFiletypedata)
#endif /* _NO_PROTO */
{
  GetWidgetTextString(AF_FiletypePrintCmdTextField, &(pFiletypedata->pszPrintCmd));
  return;
}

/******************************************************************************/
/*                                                                            */
/* init_AddFiletype_dialog_fields                                             */
/*                                                                            */
/* INPUT:  FiletypeData * - pointer to FiletypeData structure                 */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void init_AddFiletype_dialog_fields(pFtD)
     FiletypeData *pFtD;
#else /* _NO_PROTO */
void init_AddFiletype_dialog_fields(FiletypeData *pFtD)
#endif /* _NO_PROTO */
{
  char *pszIconFileName = (char *)NULL;
  char *pszTmpFile;

  if (pFtD->pszName) {
     PutWidgetTextString(AF_FileTypeNameTextField, pFtD->pszName);
  }

  ParseAndUpdateID(pFtD);

  if (pFtD->pszHelp) {
     PutWidgetTextString(AF_FiletypeHelpText, pFtD->pszHelp);
  }

  if (pFtD->pszIcon) {

     if (bShowPixmaps) {

        /*************************/
        /* Medium Pixmap         */
        /*************************/
        if (pFtD->pszMedPmIcon) {
           SET_ICONGADGET_ICON(AF_MED_IconGadget, pFtD->pszMedPmIcon);
        } else {
           pszIconFileName = (char *)NULL;
           FIND_ICONGADGET_ICON(pFtD->pszIcon, pszIconFileName, DtMEDIUM);
           pszTmpFile = GetCorrectIconType(pszIconFileName);
           SET_ICONGADGET_ICON(AF_MED_IconGadget, pszTmpFile);
           if (pszIconFileName) XtFree(pszIconFileName);
           if (pszTmpFile) XtFree(pszTmpFile);
        }

        /*************************/
        /* Tiny Pixmap           */
        /*************************/
        if (pFtD->pszTinyPmIcon) {
           SET_ICONGADGET_ICON(AF_TINY_IconGadget, pFtD->pszTinyPmIcon);
        } else {
           pszIconFileName = (char *)NULL;
           FIND_ICONGADGET_ICON(pFtD->pszIcon, pszIconFileName, DtTINY);
           pszTmpFile = GetCorrectIconType(pszIconFileName);
           SET_ICONGADGET_ICON(AF_TINY_IconGadget, pszTmpFile);
           if (pszIconFileName) XtFree(pszIconFileName);
           if (pszTmpFile) XtFree(pszTmpFile);
        }
     } else {
        /*************************/
        /* Medium Pixmap         */
        /*************************/
        if (pFtD->pszMedBmIcon) {
           SET_ICONGADGET_ICON(AF_MED_IconGadget, pFtD->pszMedBmIcon);
        } else {
           pszIconFileName = (char *)NULL;
           FIND_ICONGADGET_ICON(pFtD->pszIcon, pszIconFileName, DtMEDIUM);
           pszTmpFile = GetCorrectIconType(pszIconFileName);
           SET_ICONGADGET_ICON(AF_MED_IconGadget, pszTmpFile);
           if (pszIconFileName) XtFree(pszIconFileName);
           if (pszTmpFile) XtFree(pszTmpFile);
        }

        /*************************/
        /* Tiny Pixmap           */
        /*************************/
        if (pFtD->pszTinyBmIcon) {
           SET_ICONGADGET_ICON(AF_TINY_IconGadget, pFtD->pszTinyBmIcon);
        } else {
           pszIconFileName = (char *)NULL;
           FIND_ICONGADGET_ICON(pFtD->pszIcon, pszIconFileName, DtTINY);
           pszTmpFile = GetCorrectIconType(pszIconFileName);
           SET_ICONGADGET_ICON(AF_TINY_IconGadget, pszTmpFile);
           if (pszIconFileName) XtFree(pszIconFileName);
           if (pszTmpFile) XtFree(pszTmpFile);
        }
     }


#if 0
        /*************************/
        /* Tiny Pixmap           */
        /*************************/
        pszIconFileName = (char *)NULL;
        if (pFtD->pszTinyPmIcon) {
           pszTmpFile = pFtD->pszTinyPmIcon;
        } else {
           FIND_ICONGADGET_ICON(pFtD->pszIcon, pszIconFileName, DtTINY);
           pszTmpFile = pszIconFileName;
        }
        SET_ICONGADGET_ICON(AF_TINY_IconGadget, pszTmpFile);
        if (pszIconFileName) XtFree(pszIconFileName);

     } else {

        /*************************/
        /* Medium Bitmap         */
        /*************************/
        pszIconFileName = (char *)NULL;
        if (pFtD->pszMedBmIcon) {
           pszTmpFile = pFtD->pszMedBmIcon;
        } else {
           FIND_ICONGADGET_ICON(pFtD->pszIcon, pszIconFileName, DtMEDIUM);
           pszTmpFile = pszIconFileName;
        }
        SET_ICONGADGET_ICON(AF_MED_IconGadget, pszTmpFile);
        if (pszIconFileName) XtFree(pszIconFileName);

        /*************************/
        /* Tiny Bitmap           */
        /*************************/
        pszIconFileName = (char *)NULL;
        if (pFtD->pszTinyBmIcon) {
           pszTmpFile = pFtD->pszTinyBmIcon;
        } else {
           FIND_ICONGADGET_ICON(pFtD->pszIcon, pszIconFileName, DtTINY);
           pszTmpFile = pszIconFileName;
        }
        SET_ICONGADGET_ICON(AF_TINY_IconGadget, pszTmpFile);
        if (pszIconFileName) XtFree(pszIconFileName);
     }
#endif

  }

  if (pFtD->pszOpenCmd) {
     PutWidgetTextString(AF_OpenCmdText, pFtD->pszOpenCmd);
  }
  if (pFtD->pszPrintCmd) {
     PutWidgetTextString(AF_FiletypePrintCmdTextField, pFtD->pszPrintCmd);
  }

  return;
}

/******************************************************************************/
/*                                                                            */
/* clear_AddFiletype_dialog_fields                                            */
/*                                                                            */
/* INPUT:  FiletypeData * - pointer to FiletypeData structure                 */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void clear_AddFiletype_dialog_fields()
#else /* _NO_PROTO */
void clear_AddFiletype_dialog_fields(void)
#endif /* _NO_PROTO */
{
  /*char pszFile[MAXBUFSIZE];*/

  clear_text(AF_FileTypeNameTextField);
  clear_text(AF_IdCharacteristicsText);
  clear_text(AF_FiletypeHelpText);
  createCB_IconGadget(AF_MED_IconGadget, FALSE, Medium_Icon);
  createCB_IconGadget(AF_TINY_IconGadget, FALSE, Tiny_Icon);
  /*
  SET_ICONGADGET_ICON_AND_EXT(AF_MED_IconGadget, af_med_icon_default, pszFile);
  SET_ICONGADGET_ICON_AND_EXT(AF_TINY_IconGadget, af_tiny_icon_default, pszFile);
  */
  clear_text(AF_OpenCmdText);
  clear_text(AF_FiletypePrintCmdTextField);

  return;
}

/******************************************************************************/
/*                                                                            */
/* free_Filetypedata                                                          */
/*                                                                            */
/* INPUT:  FiletypeData * - pointer to FiletypeData structure                 */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void free_Filetypedata(pFtD)
     FiletypeData *pFtD;
#else /* _NO_PROTO */
void free_Filetypedata(FiletypeData *pFtD)
#endif /* _NO_PROTO */
{
  char buffer[MAXFILENAME];

#ifdef DEBUG
  printf("Free FiletypeData structure\n"); /* debug */
#endif

  if (pFtD->pszName) XtFree(pFtD->pszName);
#if DEBUG
  printf("Freed Name\n");
#endif

  if (pFtD->pszIcon) XtFree(pFtD->pszIcon);
  if (pFtD->pszMedPmIcon) {
#if DEBUG
     printf("free_Filetypedata: unlink '%s'\n", pFtD->pszMedPmIcon);  /* debug */
#endif
     unlink(pFtD->pszMedPmIcon);
     XtFree(pFtD->pszMedPmIcon);
  }
  if (pFtD->pszMedBmIcon) {
#if DEBUG
     printf("free_Filetypedata: unlink '%s'\n", pFtD->pszMedBmIcon);  /* debug */
#endif
     unlink(pFtD->pszMedBmIcon);
     /**** remove the mask if it exists ****/
     sprintf(buffer, "%s_m", pFtD->pszMedBmIcon);
     unlink(buffer);
     XtFree(pFtD->pszMedBmIcon);
  }
  if (pFtD->pszTinyPmIcon) {
#if DEBUG
     printf("free_Filetypedata: unlink '%s'\n", pFtD->pszTinyPmIcon);  /* debug */
#endif
     unlink(pFtD->pszTinyPmIcon);
     XtFree(pFtD->pszTinyPmIcon);
  }
  if (pFtD->pszTinyBmIcon) {
#if DEBUG
     printf("free_Filetypedata: unlink '%s'\n", pFtD->pszTinyBmIcon);  /* debug */
#endif
     unlink(pFtD->pszTinyBmIcon);
     /**** remove the mask if it exists ****/
     sprintf(buffer, "%s_m", pFtD->pszTinyBmIcon);
     unlink(buffer);
     XtFree(pFtD->pszTinyBmIcon);
  }

  if (pFtD->pszHelp) XtFree(pFtD->pszHelp);
  if (pFtD->pszOpenCmd) XtFree(pFtD->pszOpenCmd);
  if (pFtD->pszPrintCmd) XtFree(pFtD->pszPrintCmd);
  if (pFtD->pszPattern) XtFree(pFtD->pszPattern);
  if (pFtD->pszPermissions) XtFree(pFtD->pszPermissions);
  if (pFtD->pszContents) XtFree(pFtD->pszContents);
  XtFree((char *) pFtD);
  return;
}

/******************************************************************************/
/*                                                                            */
/* AddFiletypeToList                                                          */
/*                                                                            */
/* INPUT:  none                                                               */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void AddFiletypeToList()
#else /* _NO_PROTO */
void AddFiletypeToList(void)
#endif /* _NO_PROTO */
{
  if (!XmListItemExists(CA_FiletypesList, XmStringCreateSimple(XmTextFieldGetString(AF_FileTypeNameTextField)))) {
    XmListAddItem(CA_FiletypesList, XmStringCreateSimple(XmTextFieldGetString(AF_FileTypeNameTextField)), 0);
  }
}

/******************************************************************************/
/*                                                                            */
/* UpdateFiletypeDataArray                                                    */
/*                                                                            */
/* INPUT:  none                                                               */
/* OUTPUT: none                                                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
void UpdateFiletypeDataArray(pFtD)
     FiletypeData *pFtD;
#else /* _NO_PROTO */
void UpdateFiletypeDataArray(FiletypeData *pFtD)
#endif /* _NO_PROTO */
{
  FiletypeData  **papArray;
  int           i;

  /**************************************************************************/
  /* Add this structure to the filetype array in the ActionData structure.  */
  /**************************************************************************/
  papArray = (FiletypeData **) XtMalloc(sizeof(FiletypeData *) * (AD.cFiletypes + 1));
  for (i=0; i < AD.cFiletypes; i++) {
     papArray[i] = AD.papFiletypes[i];
  }
  papArray[AD.cFiletypes] = pFtD;
  if (AD.papFiletypes) {
    XtFree((char *) AD.papFiletypes);
  }
  AD.papFiletypes = papArray;
  AD.cFiletypes++;
}

/******************************************************************************/
/*                                                                            */
/* AddFiletypeCheckFields                                                     */
/*                                                                            */
/* INPUT:  none                                                               */
/* OUTPUT: FALSE  - no errors found                                           */
/*         TRUE   - found errors                                              */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
Boolean AddFiletypeCheckFields()
#else /* _NO_PROTO */
Boolean AddFiletypeCheckFields(void)
#endif /* _NO_PROTO */
{

  char    *ptr = (char *)NULL;
  Boolean bError = FALSE;
  char    buffer[MAXBUFSIZE];

  /**************************************************************************/
  /* Check if filetype name is present.                                     */
  /**************************************************************************/
  if (!bError) {
     ptr = (char *)NULL;
     GetWidgetTextString(AF_FileTypeNameTextField, &ptr);
#ifdef DEBUG
     printf("Filetype Family Name = '%s'\n", ptr);
#endif
     if (!ptr) {
        strcpy(buffer, GETMESSAGE(8, 10, "The Datatype Family Name is missing.\nEnter a name in the 'Name of Datatype Family' field."));
        display_error_message(AddFiletype, buffer);
        XmProcessTraversal(AF_FileTypeNameTextField, XmTRAVERSE_CURRENT);
        bError = TRUE;
     } else {
        XtFree(ptr);
     }
  }

  /**************************************************************************/
  /* Check if identifying characteristics are present.                      */
  /**************************************************************************/
  if (!bError) {
     ptr = (char *)NULL;
     GetWidgetTextString(AF_IdCharacteristicsText, &ptr);
#ifdef DEBUG
     printf("Identifying Chars = '%s'\n", ptr);
#endif
     if (!ptr) {
        strcpy(buffer, GETMESSAGE(8, 15, "The Identifying Characteristics are missing.\nYou must specify the characteristics before the datatype\ncan be created. Select the Edit button next to the\nIdentifying Characteristics list to specify the characteristics."));
        display_error_message(AddFiletype, buffer);
        XmProcessTraversal(AF_IdCharacteristicsEdit, XmTRAVERSE_CURRENT);
        bError = TRUE;
     } else {
        XtFree(ptr);
     }
  }

  return(bError);
}

