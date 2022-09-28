/* $XConsortium: OpenFile.c /main/cde1_maint/2 1995/10/02 18:00:17 lehors $ */
/*******************************************************************************
        OpenFile.c

       Associated Header file: OpenFile.h
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include <Xm/FileSB.h>
#include <Dt/Icon.h>
#include "UxXt.h"
#include "dtcreate.h"
#include "cmnrtns.h"
#include "ca_aux.h"
#include "fileio.h"

#define CONTEXT_MACRO_ACCESS 1
#include "OpenFile.h"
#undef CONTEXT_MACRO_ACCESS

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

#ifdef _NO_PROTO
static  void    cancelCB_OpenFile( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    cancelCB_OpenFile( Widget UxWidget,
                                  XtPointer UxClientData,
                                  XtPointer UxCallbackArg )
#endif /* _NO_PROTO */

{
        _UxCOpenFile            *UxSaveCtx, *UxContext;

        UxSaveCtx = UxOpenFileContext;
        UxOpenFileContext = UxContext =
                        (_UxCOpenFile *) UxGetContext( UxWidget );
        {

        UxPopdownInterface(UxWidget);
        }
        UxOpenFileContext = UxSaveCtx;
}

#ifdef _NO_PROTO
static  void    okCallback_OpenFile( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    okCallback_OpenFile( Widget UxWidget,
                                    XtPointer UxClientData,
                                    XtPointer UxCallbackArg )
#endif /* _NO_PROTO */

{
        _UxCOpenFile                     *UxSaveCtx, *UxContext;
        XmFileSelectionBoxCallbackStruct *cbs;
        /*char        *text;*/
        int         rc;
        ActionData  ADTmp;

        UxSaveCtx = UxOpenFileContext;
        UxOpenFileContext = UxContext =
                        (_UxCOpenFile *) UxGetContext( UxWidget );
        {

        /********************************************************************/
        /* Call file parsing code here!                                     */
        /********************************************************************/
        cbs = (XmFileSelectionBoxCallbackStruct *)UxCallbackArg;
        if (pszFileToEdit) {
           XtFree(pszFileToEdit);
        }
        if (XmStringGetLtoR(cbs->value, XmFONTLIST_DEFAULT_TAG, &pszFileToEdit)) {
           memset(&ADTmp, 0, sizeof(ActionData));
           rc = OpenDefinitionFile(pszFileToEdit, &ADTmp);
           /*XtFree(text);*/
           /*****************************************************************/
           /* If everything ok, then clear out data from action structure   */
           /* and store new data.                                           */
           /*****************************************************************/
           if (!rc) {
              FreeAndClearAD(&AD);
              memcpy(&AD, &ADTmp, sizeof(ActionData));
           }
        }
        UxPopdownInterface (UxWidget);

        /********************************************************************/
        /* If successfully opened and parsed file, then init the fields of  */
        /* the main window.                                                 */
        /********************************************************************/
        if (!rc) {
           clear_CreateActionAppShell_fields();
           writeCAToGUI(&AD);
        }
        }
        UxOpenFileContext = UxSaveCtx;
}

#ifdef _NO_PROTO
static  void    helpCallback_OpenFile( UxWidget, UxClientData, UxCallbackArg )
        Widget          UxWidget;
        XtPointer       UxClientData, UxCallbackArg;
#else /* _NO_PROTO */
static  void    helpCallback_OpenFile( Widget UxWidget,
                                    XtPointer UxClientData,
                                    XtPointer UxCallbackArg )
#endif /* _NO_PROTO */

{
        _UxCOpenFile            *UxSaveCtx, *UxContext;

        UxSaveCtx = UxOpenFileContext;
        UxOpenFileContext = UxContext =
                        (_UxCOpenFile *) UxGetContext( UxWidget );
        {
        DisplayHelpDialog(UxWidget, HELP_OPENFILE, UxCallbackArg);
        }
        UxOpenFileContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

#ifdef _NO_PROTO
static Widget   _Uxbuild_OpenFile()
#else /* _NO_PROTO */
static Widget   _Uxbuild_OpenFile(void)
#endif /* _NO_PROTO */
{
        Widget          _UxParent;

        /* Creation of OpenFile */
        _UxParent = XtVaCreatePopupShell( "OpenFile_shell",
                        xmDialogShellWidgetClass, UxTopLevel,
                        XmNx, 200,
                        XmNy, 290,
                        XmNwidth, 398,
                        XmNheight, 500,
                        XmNallowShellResize, TRUE,
                        XmNshellUnitType, XmPIXELS,
                        XmNtitle, GETMESSAGE(12, 25, "Create Action - Open"),
                        NULL );

        OpenFile = XtVaCreateWidget( "OpenFile",
                        xmFileSelectionBoxWidgetClass,
                        _UxParent,
                        XmNresizePolicy, XmRESIZE_GROW,
                        XmNunitType, XmPIXELS,
#ifdef NOT_ILS
                        XmNwidth, 398,
#endif
                        XmNheight, 500,
                        RES_CONVERT( XmNdialogTitle, GETMESSAGE(12, 25, "Create Action - Open")),
                        RES_CONVERT( XmNdirectory, "" ),
                        RES_CONVERT( XmNtextString, "" ),
                        RES_CONVERT( XmNdirSpec, "" ),
                        XmNdialogType, XmDIALOG_FILE_SELECTION,
                        XmNtextColumns, 20,
                        XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
                        XmNallowOverlap, FALSE,
                        RES_CONVERT( XmNchildPlacement, "place_below_selection" ),
                        XmNdefaultPosition, FALSE,
                        RES_CONVERT( XmNdirMask, "" ),
                        RES_CONVERT( XmNpattern, "" ),
                        NULL );
        XtAddCallback( OpenFile, XmNcancelCallback,
                (XtCallbackProc) cancelCB_OpenFile,
                (XtPointer) UxOpenFileContext );
        XtAddCallback( OpenFile, XmNokCallback,
                (XtCallbackProc) okCallback_OpenFile,
                (XtPointer) UxOpenFileContext );
        XtAddCallback( OpenFile, XmNhelpCallback,
                (XtCallbackProc) helpCallback_OpenFile,
                (XtPointer) UxOpenFileContext );
        XtVaSetValues(OpenFile, XmNuserData, OpenFile, NULL);

        UxPutContext( OpenFile, (char *) UxOpenFileContext );

        XtAddCallback( OpenFile, XmNdestroyCallback,
                (XtCallbackProc) UxDestroyContextCB,
                (XtPointer) UxOpenFileContext);

        return ( OpenFile );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

#ifdef _NO_PROTO
Widget  create_OpenFile()
#else /* _NO_PROTO */
Widget  create_OpenFile(void)
#endif /* _NO_PROTO */
{
        Widget                  rtrn;
        _UxCOpenFile            *UxContext;

        UxOpenFileContext = UxContext =
                (_UxCOpenFile *) UxNewContext( sizeof(_UxCOpenFile), False );


        rtrn = _Uxbuild_OpenFile();

        return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

