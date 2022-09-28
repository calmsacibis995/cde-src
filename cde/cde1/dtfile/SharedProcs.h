/* $XConsortium: SharedProcs.h /main/cde1_maint/1 1995/07/17 20:50:22 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           SharedProcs.h
 *
 *   COMPONENT_NAME: Desktop File Manager
 *
 *   DESCRIPTION:    Public include file for some shared functions.
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _SharedProcs_h
#define _SharedProcs_h

#include <Dt/Icon.h>
#include <Dt/Action.h>
#include <Dt/Dnd.h>

#include "Common.h"
#include "FileMgr.h"

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

/* Path functions */
extern char * _DtPName() ;
extern char * _DtFollowLink();

/* String functions */
extern Boolean _DtStringsAreEquivalent();

/* Encapsulator functions */
extern void _DtDuplicateDialogNameList() ;
extern void _DtLoadSubdialogArray() ;
extern void _DtSaveSubdialogArray() ;
extern void _DtDestroySubdialogArray() ;
extern void _DtDestroySubdialog() ;
extern void _DtHideOneSubdialog() ;
extern void _DtMappedCB() ;
extern void _DtGenericMapWindow() ;
extern void _DtGenericDestroy() ;
extern void _DtGenericUpdateWindowPosition() ;

/* Pixmap functions */
extern void _DtBuildPath() ;
extern void _DtPath() ;
extern PixmapData * _DtRetrievePixmapData() ;
extern void _DtCheckAndFreePixmapData() ;

/* Datatype attribute functions */
extern Boolean _DtCheckForDataTypeProperty() ;
extern char ** _DtCompileActionVector() ;
extern char * _DtRetrieveDefaultAction() ;

/* Misc functions */
extern char * _DtBuildFMTitle() ;
extern char * _DtGetSelectedFilePath() ;

/* Action args functions */
extern void _DtBuildActionArgsWithSelectedFiles() ;
extern void _DtBuildActionArgsWithDroppedFiles() ;
extern void _DtBuildActionArgsWithDroppedBuffers() ;
extern void _DtFreeActionArgs() ;

/* Drop buffer functions */
extern void _DtSetDroppedBufferInfo();
extern void _DtFreeDroppedBufferInfo();
extern Boolean _DtIsBufferExecutable();

/* Drop file functions */
extern void _DtSetDroppedFileInfo();
extern void _DtCopyDroppedFileInfo();
extern void _DtFreeDroppedFileInfo();

/* User input functions */
extern void _DtPathFromInput();
extern char * _DtChangeTildeToHome();
extern Boolean _DtSpacesInFileNames();

/* Appmanager functions */
extern char * _DtResolveAppManPath();

#else 

extern char * _DtPName(
                        register char *name) ;
extern char * _DtFollowLink(
                        char * path) ;
extern Boolean _DtStringsAreEquivalent(
                        register char *in_str,
                        register char *test_str) ;
extern void _DtDuplicateDialogNameList (
                        char ** nameList,
                        char ** newNameList,
                        int newNameListSize,
                        int * nameCount) ;
extern void _DtLoadSubdialogArray (
                        char ** nameList,
                        int    nameCount,
                        DialogData *** dialogArray,
                        int    dialogCount,
                        int    dialogId,
                        XrmDatabase dataBase,
                        int    firstId );
extern void _DtSaveSubdialogArray (
                        char ** nameList,
                        int    nameCount,
                        DialogData ** dialogArray,
                        int    dialogCount,
                        int    fd,
                        int    firstId );
extern void _DtDestroySubdialogArray (
                        DialogData ** dialogArray,
                        int    dialogCount) ;
extern void _DtDestroySubdialog (
                        DialogData * dialogData) ;
extern void _DtHideOneSubdialog (
                        DialogData * dialogData,
                        DialogData *** dialogArray,
                        int         * dialogCountPtr) ;
extern void _DtAddOneSubdialog (
                        DialogData * dialogData,
                        DialogData *** dialogArray,
                        int         * dialogCountPtr) ;
extern void _DtMappedCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void _DtGenericMapWindow (
                        Widget parent,
                        XtPointer recordPtr) ;
extern void _DtGenericDestroy(
                        XtPointer recordPtr) ;
extern void _DtGenericUpdateWindowPosition(
                        DialogData * dataPtr) ;
extern void _DtBuildPath(
                        char         *path,
                        char         *directory,
                        char         *fileName) ;
extern char * _DtPath(
                        char         *directory,
                        char         *fileName) ;
extern PixmapData * _DtRetrievePixmapData(
                        char         *dataType,
                        char         *fileName,
                        char         *directory,
                        Widget        shell,
                        int           size);
extern void _DtCheckAndFreePixmapData(
                        char         *dataType,
                        Widget        shell,
                        DtIconGadget  iconGadget,
                        PixmapData   *pixmapData) ;
extern Boolean _DtCheckForDataTypeProperty(
                        char *dataType,
                        char *property) ;
extern char ** _DtCompileActionVector(
                        char *dataType) ;
extern char * _DtRetrieveDefaultAction(
                        char *dataType) ;
extern char * _DtBuildFMTitle(
                        FileMgrData *file_mgr_data,
			Display *dpy) ;
extern char * _DtGetSelectedFilePath(
                        FileViewData *selected_file) ;
extern void _DtBuildActionArgsWithSelectedFiles(
                        FileViewData **selection_list,
                        int selected_file_count,
                        DtActionArg **action_args,
                        int *args_count) ;
extern void _DtBuildActionArgsWithDroppedFiles(
                        FileViewData *dropped_on_obj,
                        DtDndDropCallbackStruct *drop_parameters,
                        DtActionArg **action_args,
                        int *args_count) ;
extern void _DtBuildActionArgsWithDroppedBuffers(
                        FileViewData *dropped_on_obj,
                        DtDndDropCallbackStruct *drop_parameters,
                        DtActionArg **action_args,
                        int *args_count) ;
extern void _DtFreeActionArgs(
                        DtActionArg *action_args,
                        int args_count) ;
extern void _DtSetDroppedBufferInfo(
                        char **file_set,
                        BufferInfo *buffer_set,
                        char **host_set,
                        DtDndDropCallbackStruct *drop_parameters);
extern void _DtFreeDroppedBufferInfo (
                        char **file_set,
                        BufferInfo *buffer_set,
                        char **host_set,
                        int num_of_buffers);
extern Boolean _DtIsBufferExecutable(
                        void *buffer,
                        int buffer_size) ;
extern void _DtSetDroppedFileInfo(
                        DtDndDropCallbackStruct *drop_parameters,
                        char ***file_set,
                        char ***host_set) ;
extern void _DtCopyDroppedFileInfo(
                        int num_files,
                        char **orig_file_set,
                        char **orig_host_set,
                        char ***new_file_set,
                        char ***new_host_set) ;
extern void _DtFreeDroppedFileInfo(
                        int num_files,
                        char **file_set,
                        char **host_set) ;
extern void _DtPathFromInput(
                        char *input_string,
                        char *current_dir,
                        char **host,
                        char **rel_path) ;
extern char * _DtChangeTildeToHome(
                        char *input_string) ;
extern Boolean _DtSpacesInFileNames(
                        char **fileNames,
                        int fileCount) ;
extern char * _DtResolveAppManPath(
                        char *path,
                        char *restricted_dir) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/* _DtMessage Dialog build defines */
#define ERROR_DIALOG  1
#define WARNING_DIALOG  2
#define QUESTION_DIALOG 3

/* Flag which can be used to prevent error dialogs from being posted */
extern Boolean messageDisplayEnabled;

/* Flag controlling whether dialogs are auto-positioned */
extern Boolean disableDialogAutoPlacement;

/* Generic overlay for all dialog 'Rec' structures */
typedef struct
{
   Widget shell;
} GenericRecord;

#endif /* _SharedProcs_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
