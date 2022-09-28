/* $XConsortium: Encaps.h /main/cde1_maint/2 1995/08/29 19:33:07 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           Encaps.h
 *
 *   COMPONENT_NAME: Desktop File Manager
 *
 *   DESCRIPTION:    Public include file for the dialog encapsulation functions.
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Encapsualte_h
#define _Encapsulate_h


/* Timer globals */
extern int initialTimeoutLength;
extern int activeTimeoutLength;
extern int idleTimeoutLength;

/* Dialog positioning offset value */
extern int topPositionOffset;



/*
 *  Common dialog instance data.
 */

typedef struct
{
   Boolean   displayed;
   Position  x;
   Position  y;
   Dimension width;
   Dimension height;
} DialogInstanceData, * DialogInstanceDataPtr;


/*
 *  DialogData contains the dialog type specific data shared between
 *  the dialog and the application.
 */

typedef struct
{
   int     type;
   XtPointer data;
} DialogData;


/*
 *  The following typedefs are the function definitions used by the
 *  Dialog Class structure.
 */

typedef void (*DialogCreateProc)(
#ifdef _NO_PROTO
/*  Display * display;  */
/*  Widget    parent;   */
/*  Widget  * shell;    */
/*  XtPointer * dialog;   */
#else
    Display * display,
    Widget    parent,
    Widget  * shell,
    XtPointer * dialog
#endif
);

typedef void (*DialogInstallChangeProc)(
#ifdef _NO_PROTO
/*  XtPointer        dialog;       */
/*  XtCallbackProc callback;     */
/*  XtPointer        client_data;  */
#else
    XtPointer        dialog,
    XtCallbackProc callback,
    XtPointer        client_data
#endif
);

typedef void (*DialogInstallCloseProc)(
#ifdef _NO_PROTO
/*  XtPointer        dialog;       */
/*  XtCallbackProc callback;     */
/*  XtPointer        client_data;  */
#else
    XtPointer        dialog,
    XtCallbackProc callback,
    XtPointer        client_data
#endif
);

typedef void (*DialogDestroyProc)(
#ifdef _NO_PROTO
/*  XtPointer dialog;  */
#else
    XtPointer dialog
#endif
);

typedef XtPointer (*DialogGetValuesProc)(
#ifdef _NO_PROTO
/*  XtPointer dialog;  */
#else
    XtPointer dialog
#endif
);

typedef XtPointer (*DialogGetDefaultValuesProc)(
#ifdef _NO_PROTO
/*  takes no arguments   */
#else
    void
#endif
);


typedef XtPointer (*DialogGetResourceValuesProc)(
#ifdef _NO_PROTO
/*  XrmDatabase    data_base;  */
/*  char        ** name_list;  */
#else
    XrmDatabase    data_base,
    char        ** name_list
#endif
);

typedef void (*DialogSetValuesProc)(
#ifdef _NO_PROTO
/*  XtPointer      dialog;  */
/*  DialogData * values;  */
#else
    XtPointer      dialog,
    DialogData * values
#endif
);
typedef void (*DialogWriteResourceValuesProc)(
#ifdef _NO_PROTO
/*  DialogData *  values;     */
/*  int           fd;         */
/*  char       ** name_list;  */
#else
    DialogData *  values,
    int           fd,
    char       ** name_list
#endif
);

typedef void (*DialogFreeValuesProc)(
#ifdef _NO_PROTO
/*  DialogData * values;  */
#else
    DialogData * values
#endif
);

typedef void (*DialogSetFocusProc)(
#ifdef _NO_PROTO
/*  XtPointer      dialog;  */
/*  DialogData * values;  */
#else
    XtPointer      dialog,
    DialogData * values
#endif
);


typedef void (*DialogMapWindowProc)(
#ifdef _NO_PROTO
/*  Widget    parent;   */
/*  XtPointer   dialog;   */
#else
    Widget    parent,
    XtPointer  dialog
#endif
);


/*
 *  Resource write proc and structure used for dialog resources. 
 */

typedef void (*WriteResourceProc)();
/*  XtPointer resource_val;   */

typedef struct 
{
   String  name;
   String  type;
   int     size;
   int     offset;
   XtPointer default_value;
   WriteResourceProc write_resource;   
} DialogResource;


/*
 *  The Dialog Class structure.
 */

typedef struct
{
   DialogResource              * resources;
   int                           resource_count;
   DialogCreateProc              create;
   DialogInstallChangeProc       install_change_callback;
   DialogInstallCloseProc        install_close_callback;
   DialogDestroyProc             destroy;
   DialogGetValuesProc           get_values;
   DialogGetDefaultValuesProc    get_default_values;
   DialogGetResourceValuesProc   get_resource_values;
   DialogSetValuesProc           set_values;
   DialogWriteResourceValuesProc write_resource_values;
   DialogFreeValuesProc          free_values;
   DialogMapWindowProc           map;
   DialogSetFocusProc            set_focus;
} DialogClass;



/*
 *  Procedure types used by the application for functions to be passed
 *  to the encapsulation mechanism.
 */

typedef void (*DialogChangedProc)(
#ifdef _NO_PROTO
/*  XtPointer      change_data;  */
/*  DialogData * old_data;     */
/*  DialogData * new_data;     */
/*  XtPointer      call_data;    */
#else
    XtPointer      change_data,
    DialogData * old_data,
    DialogData * new_data,
    XtPointer      call_data
#endif
);

typedef void (*DialogClosedProc)(
#ifdef _NO_PROTO
/*  XtPointer      close_data;  */
/*  DialogData * old_data;    */
/*  DialogData * new_data;     */
#else
    XtPointer      close_data,
    DialogData * old_data,
    DialogData * new_data
#endif
);



/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void _DtInitializeEncapsulation() ;
extern int _DtInstallDialog() ;
extern DialogData * _DtGetDefaultDialogData() ;
extern DialogData * _DtGetResourceDialogData() ;
extern void _DtShowDialog() ;
extern void _DtHideDialog() ;
extern Widget _DtGetDialogShell() ;
extern XtPointer _DtGetDialogInstance() ;
extern DialogData * _DtGetInstanceData() ;
extern Boolean _DtIsDialogShowing() ;
extern void _DtWriteDialogData() ;
extern void _DtFreeDialogData() ;
extern void _DtDialogGetResources() ;
extern void _DtDialogPutResources() ;
extern void _DtIntToString() ;
extern void _DtPositionToString() ;
extern void _DtDimensionToString() ;
extern void _DtBooleanToString() ;
extern void _DtXmStringToString() ;
extern void _DtXmStringTableToString() ;
extern void _DtStringToString() ;
extern void _DtmapCB();
extern void _DtEncapSetWorkSpaceHints() ;
extern void _DtBuildDialog( ) ;
extern void _DtShowBuiltDialog( ) ;
extern void _DtFreeDialog( );

#else

extern void _DtInitializeEncapsulation( 
                        Display *display,
                        char *name,
                        char *class) ;
extern int _DtInstallDialog( 
                        DialogClass *dialog_class,
                        Boolean cache,
                        Boolean destroyPopups) ;
extern DialogData * _DtGetDefaultDialogData( 
                        int dialog_type) ;
extern DialogData * _DtGetResourceDialogData( 
                        int dialog_type,
                        XrmDatabase data_base,
                        char **name_list) ;
extern void _DtShowDialog( 
                        Widget parent,
                        Widget map_parent,
                        XtPointer top_rec,
                        DialogData *dialog_data,
                        DialogChangedProc change_proc,
                        XtPointer change_data,
                        DialogClosedProc close_proc,
                        XtPointer close_data,
                        char *workspaces,
                        Boolean iconify_state,
                        Boolean ignoreCache,
                        char * title,
                        XClassHint * classHints,
                        Boolean UseIconX, int IconX,
                        Boolean UseIconY, int IconY);
extern void _DtHideDialog( 
                        DialogData *dialog_data,
                        Boolean call_callbacks) ;
extern Widget _DtGetDialogShell( 
                        DialogData *dialog_data) ;
extern XtPointer _DtGetDialogInstance( 
                        DialogData *dialog_data) ;
extern DialogData * _DtGetInstanceData( 
                        XtPointer instance) ;
extern Boolean _DtIsDialogShowing( 
                        DialogData *dialog_data) ;
extern void _DtWriteDialogData( 
                        DialogData *dialog_data,
                        int fd,
                        char **name_list) ;
extern void _DtFreeDialogData( 
                        DialogData *dialog_data) ;
extern void _DtDialogGetResources( 
                        XrmDatabase database,
                        char **name_list,
                        char *dialog_name,
                        char *base,
                        DialogResource *resources,
                        int resource_count) ;
extern void _DtDialogPutResources( 
                        int fd,
                        char **name_list,
                        char *dialog_name,
                        char *base,
                        DialogResource *resources,
                        int resource_count) ;
extern void _DtIntToString( 
                        int fd,
                        int *value,
                        char *out_buf) ;
extern void _DtPositionToString( 
                        int fd,
                        Position *value,
                        char *out_buf) ;
extern void _DtDimensionToString( 
                        int fd,
                        Dimension *value,
                        char *out_buf) ;
extern void _DtBooleanToString( 
                        int fd,
                        Boolean *value,
                        char *out_buf) ;
extern void _DtXmStringToString( 
                        int fd,
                        XmString *value,
                        char *out_buf) ;
extern void _DtXmStringTableToString( 
                        int fd,
                        XmStringTable *value,
                        char *out_buf) ;
extern void _DtStringToString( 
                        int fd,
                        char **value,
                        char *out_buf) ;
extern void _DtmapCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
extern void _DtEncapSetWorkSpaceHints( 
                        Widget shell,
                        char *workspaces) ;
extern void _DtBuildDialog(
                        Widget parent,
                        Widget map_parent,
                        XtPointer top_rec,
                        DialogData *dialog_data,
                        DialogChangedProc change_proc,
                        XtPointer change_data,
                        DialogClosedProc close_proc,
                        XtPointer close_data,
                        char *workspaces,
                        Boolean iconify_state,
                        Boolean ignoreCache,
                        char * title,
                        XClassHint * classHints ) ;
extern void _DtShowBuiltDialog(
                        Widget parent,
                        Widget map_parent,
                        DialogData *dialog_data,
                        char *workspaces,
                        Boolean iconify_state,
                        XClassHint * classHints,
                        Boolean UseIconX, int iconX,
                        Boolean UseIconY, int IconY);

extern void _DtFreeDialog(
                        DialogData *dialog_data);
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#endif /* _Encapsulate_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
