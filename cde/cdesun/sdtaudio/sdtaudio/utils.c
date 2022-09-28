/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */


#pragma ident "@(#)sdtaudio.c	1.9 96/06/20 SMI"

/* File contains utility functions used by sdtaudio. */

#include <stdlib.h>
#include <sys/types.h>
#include <nl_types.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/varargs.h>
#include <X11/Intrinsic.h>
#include <X11/Core.h>
#include <Xm/XmAll.h>
#include <Dt/HelpDialog.h>
#include <Dt/HelpQuickD.h>
#include "utils.h"
#include "extern.h"

/*
 * Directory where the binary for this process whate loaded from
 */
static char                             *dtb_exe_dir = (char *)NULL;


/*
 * Variable for storing command used to invoke application
 */
static char                             *dtb_save_command_str = (char *)NULL;

#ifndef min
#define min(a,b) ((a) < (b)? (a):(b))
#endif

/*
 * Sets the XmNlabel from the image file (either xbitmap or xpixmap format).
 *
 * returns negative on error.
 */
int
dtb_set_label_from_image_file(
                Widget  widget,
                String  fileName
)
{
    int         rc = 0;         /* return code */
    Pixmap      labelPixmap = NULL;
    Pixmap      insensitivePixmap = NULL;
    int         depth;

    rc = dtb_cvt_image_file_to_pixmap(widget, fileName, &labelPixmap);
    if (rc < 0)
    {
        return rc;
    }

    insensitivePixmap = dtb_create_greyed_pixmap(widget,labelPixmap);
    rc = dtb_set_label_pixmaps(widget, labelPixmap, insensitivePixmap);
    if (rc < 0)
    {
        return rc;
    }

    return 0;
}

/**************************************************************************/
/*
 * dtb_cvt_image_file_to_pixmap
 */
int
dtb_cvt_image_file_to_pixmap(
                Widget  widget,
                String  fileName,
                Pixmap  *pixmap
)
{
  int         rc = 0;         /* return code */
  Pixmap      tmpPixmap = NULL;
  int         depth;
  
  if (dtb_file_has_extension(fileName, "pm") ||
      dtb_file_has_extension(fileName, "xpm") ||
      dtb_file_has_extension(fileName, "bm") ||
      dtb_file_has_extension(fileName, "xbm"))
    {
      /* If explicit filename requested, use it directly */
      rc = dtb_cvt_file_to_pixmap(fileName, widget, &tmpPixmap);
    }
  else /* Append extensions to locate best graphic match */
    {
      XtVaGetValues(XtIsSubclass(widget, xmGadgetClass)? XtParent(widget) : widget,
		    XmNdepth, &depth, NULL);
      
      if (depth > 1) /* Look for Color Graphics First */
        {
	  rc = dtb_cvt_filebase_to_pixmap(widget, fileName, ".pm", &tmpPixmap);
	  if (rc < 0)
	    rc = dtb_cvt_filebase_to_pixmap(widget, fileName, ".xpm", &tmpPixmap);
	  if (rc < 0)
	    rc = dtb_cvt_filebase_to_pixmap(widget, fileName, ".bm", &tmpPixmap);
	  if (rc < 0)
	    rc = dtb_cvt_filebase_to_pixmap(widget, fileName, ".xbm", &tmpPixmap);
        }
      else /* Look for Monochrome First */
        {
	  rc = dtb_cvt_filebase_to_pixmap(widget, fileName, ".bm", &tmpPixmap);
	  if (rc < 0)
	    rc = dtb_cvt_filebase_to_pixmap(widget, fileName, ".xbm", &tmpPixmap);
	  if (rc < 0)
	    rc = dtb_cvt_filebase_to_pixmap(widget, fileName, ".pm", &tmpPixmap);
	  if (rc < 0)
	    rc = dtb_cvt_filebase_to_pixmap(widget, fileName, ".xpm", &tmpPixmap);
        }
    }
  
  if (rc < 0)
    {
      *pixmap = NULL;
      return rc;
    }
  
  *pixmap = tmpPixmap;
  return 0;
}
/**********************************************************************/
/*
 * Sets the label and insensitive label pixmaps of the widget.
 *
 * If either (or both) pixmap is NULL, it is ignored.
 */
int
dtb_set_label_pixmaps(
    Widget      widget,
    Pixmap      labelPixmap,
    Pixmap      labelInsensitivePixmap
)
{
    if (   (widget == NULL)
        || ((labelPixmap == NULL) && (labelInsensitivePixmap == NULL)) )
    {
        return -1;
    }

    /*
     * Set the approriate resources.
     */
    XtVaSetValues(widget, XmNlabelType, XmPIXMAP, NULL);
    if (labelPixmap != NULL)
    {
        XtVaSetValues(widget, XmNlabelPixmap, labelPixmap, NULL);
    }
    if (labelInsensitivePixmap != NULL)
    {
        XtVaSetValues(widget, XmNlabelInsensitivePixmap,
                                labelInsensitivePixmap, NULL);
    }

    return 0;
}
/**************************************************************************/
/*
 * For a given pixmap, create a 50% greyed version.  Most likely this will
 * be used where the source pixmap is the labelPixmap for a widget and an
 * insensitivePixmap is needed so the widget will look right when it is
 * "not sensitive" ("greyed out" or "inactive").
 *
 * NOTE: This routine creates a Pixmap, which is an X server resource.  The
 *       created pixmap must be freed by the caller when it is no longer
 *       needed.
 */
Pixmap
dtb_create_greyed_pixmap(
    Widget      widget,
    Pixmap      pixmap
)
{
    Display      *dpy;
    Window       root;
    Pixmap       insensitive_pixmap;
    Pixel        background;
    unsigned int width, height, depth, bw;
    int          x,y;
    XGCValues    gcv;
    XtGCMask     gcm;
    GC           gc;


    dpy = XtDisplayOfObject(widget);

    if(pixmap == XmUNSPECIFIED_PIXMAP || pixmap == (Pixmap)NULL) {
        return((Pixmap)NULL);
    }

    XtVaGetValues(widget,
        XmNbackground, &background,
        NULL);

    /* Get width/height of source pixmap */
    if (!XGetGeometry(dpy,pixmap,&root,&x,&y,&width,&height,&bw,&depth)) {
            return((Pixmap)NULL);
    }
    gcv.foreground = background;
    gcv.fill_style = FillStippled;
    gcv.stipple = XmGetPixmapByDepth(XtScreenOfObject(widget),
                        "50_foreground", 1, 0, 1);
    gcm = GCForeground | GCFillStyle | GCStipple;
    gc = XtGetGC(widget, gcm, &gcv);

    /* Create insensitive pixmap */
    insensitive_pixmap = XCreatePixmap(dpy, pixmap, width, height, depth);
    XCopyArea(dpy, pixmap, insensitive_pixmap, gc, 0, 0, width, height, 0, 0);
    XFillRectangle(dpy, insensitive_pixmap, gc, 0, 0, width, height);

    XtReleaseGC(widget, gc);
    return(insensitive_pixmap);
}
/***************************************************************/
/*
 * Returns True if the fileName has the extension
 */
Boolean
dtb_file_has_extension(
    String      fileName,
    String      extension
)
{
    Boolean        hasExt = False;

    if (extension == NULL)
    {
        hasExt = ( (fileName == NULL) || (strlen(fileName) == 0) );
    }
    else
    {
        if (fileName == NULL)
            hasExt = False;
        else
        {
            char *dotPtr= strrchr(fileName, '.');
            if (dotPtr == NULL)
                hasExt= False;
            else if (strcmp(dotPtr+1, extension) == 0)
                hasExt = True;
        }
    }
    return hasExt;
}
/*****************************************************************/
/*
 * Appends the extension to fileBase and attempts to load in
 * the Pixmap
 */
int
dtb_cvt_filebase_to_pixmap(
    Widget      widget,
    String      fileBase,
    String      extension,
    Pixmap      *pixmap_ptr
)
{
    char        fileName[512];
    int         rc = 0;

    strcpy(fileName, fileBase);
    strcat(fileName, extension);
    rc = dtb_cvt_file_to_pixmap(fileName, widget, pixmap_ptr);
    return rc;
}
/*******************************************************************/
/*
 * Create/load a Pixmap given an XPM or Bitmap files
 * NOTE: this allocates a server Pixmap;  it is the responsibility
 * of the caller to free the Pixmap
 */
int
dtb_cvt_file_to_pixmap(
    String      fileName,
    Widget      widget,
    Pixmap      *pixmapReturnPtr
)
{
#define pixmapReturn (*pixmapReturnPtr)
    Pixmap      pixmap = NULL;
    Screen      *screen = NULL;
    Pixel       fgPixel = 0;
    Pixel       bgPixel = 0;
    char        image_path[MAXPATHLEN+1];
    Boolean     pixmap_found = False;

    /*
     * Get default values
     */
    screen = XtScreenOfObject(widget);
    fgPixel = WhitePixelOfScreen(screen);
    bgPixel = BlackPixelOfScreen(screen);

    /*
     * Get proper colors for widget
     */
    XtVaGetValues(widget,
        XmNforeground, &fgPixel,
        XmNbackground, &bgPixel,
        NULL);

    /*
     * In CDE, XmGetPixmap handles .xpm files, as well.
     */
    if (!pixmap_found)
    {
        pixmap = XmGetPixmap(screen, fileName, fgPixel, bgPixel);
    }
    pixmap_found = ((pixmap != NULL) && (pixmap != XmUNSPECIFIED_PIXMAP));

    if (!pixmap_found)
    {
        sprintf(image_path, "%s/%s", dtb_get_exe_dir(), fileName);
        pixmap = XmGetPixmap(screen, image_path, fgPixel, bgPixel);
    }
    pixmap_found = ((pixmap != NULL) && (pixmap != XmUNSPECIFIED_PIXMAP));

    if (!pixmap_found)
    {
        sprintf(image_path, "%s/bitmaps/%s", dtb_get_exe_dir(), fileName);
        pixmap = XmGetPixmap(screen, image_path, fgPixel, bgPixel);
    }
    pixmap_found = ((pixmap != NULL) && (pixmap != XmUNSPECIFIED_PIXMAP));

    if (!pixmap_found)
    {
        return -1;
    }

    pixmapReturn = pixmap;
    pixmapReturn = pixmap;
    return 0;
#undef pixmapReturn
}
/*******************************************************************/
/*
 * Returns the directory that the executable for this process was loaded
 * from.
 */
String dtb_get_exe_dir(void)
{
    return dtb_exe_dir;
}
/*******************************************************************/
/*
 ** Routines to save and access the command used to invoke the application.
 */
void
dtb_save_command(
    char        *argv0
)
{
    char        exe_dir[MAXPATHLEN+1];
    dtb_save_command_str = argv0;

    /*
     * Save the path to the executable
     */
    if (determine_exe_dir(argv0, exe_dir, MAXPATHLEN+1) >= 0)
    {
        dtb_exe_dir = (char *)malloc(strlen(exe_dir)+1);
        if (dtb_exe_dir != NULL)
        {
            strcpy(dtb_exe_dir, exe_dir);
        }
    }
}
/*******************************************************************/
/*
 * Determines the directory the executable for this process was loaded from.
 */
static int
determine_exe_dir(
    char        *argv0,
    char        *buf,
    int         bufSize
)
{
    Boolean     foundDir= False;

    if ((buf == NULL) || (bufSize < 1))
    {
        return -1;
    }

    if (determine_exe_dir_from_argv(argv0, buf, bufSize) >= 0)
    {
        foundDir = True;
    }

    if (!foundDir)
    {
        if (determine_exe_dir_from_path(argv0, buf, bufSize) >= 0)
        {
            foundDir = True;
        }
    }

    /*
     * Convert relative path to absolute, so that directory changes will
     * not affect us.
     */
    if (foundDir && (buf[0] != '/'))
    {
        char    cwd[MAXPATHLEN+1];
        char    *env_pwd = NULL;
        char    *path_prefix = NULL;
        char    abs_exe_dir[MAXPATHLEN+1];

        if (getcwd(cwd, MAXPATHLEN+1) != NULL)
        {
            path_prefix = cwd;
        }
        else
        {
            env_pwd = getenv("PWD");
            if (env_pwd != NULL)
            {
                path_prefix = env_pwd;
            }
        }

        if (path_prefix != NULL)
        {
            strcpy(abs_exe_dir, path_prefix);
            if (strcmp(buf, ".") != 0)
            {
                strcat(abs_exe_dir, "/");
                strcat(abs_exe_dir, buf);
            }
            strcpy(buf, abs_exe_dir);
        }
    }

    return foundDir? 0:-1;
}
/******************************************************************/
/*
 *  Looks for absolute path in arv[0].
 */
static int
determine_exe_dir_from_argv(
    char        *argv0,
    char        *buf,
    int         bufSize
)
{
    int         i= 0;
    Boolean     foundit= False;

    for (i= strlen(argv0)-1; (i >= 0) && (argv0[i] != '/'); --i)
    {
    }

    if (i >= 0)
    {
        int     maxStringSize= min(i, bufSize);
        strncpy(buf, argv0, maxStringSize);
        buf[min(maxStringSize, bufSize-1)]= 0;
        foundit = True;
    }

    return foundit? 0:-1;
}
/*********************************************************************/
/*
 * Assumes: bufSize > 0
 */
static int
determine_exe_dir_from_path (
    char        *argv0,
    char        *buf,
    int         bufSize
)
{
    Boolean     foundDir= False;
    Boolean     moreDirs= True;
    char        *szExeName= argv0;
    int         iExeNameLen= strlen(szExeName);
    char        *szTemp= NULL;
    char        szPathVar[MAXPATHLEN+1];
    int         iPathVarLen= 0;
    char        szCurrentPath[MAXPATHLEN+1];
    int         iCurrentPathLen= 0;
    int         iCurrentPathStart= 0;
    int         i = 0;
    uid_t       euid= geteuid();
    uid_t       egid= getegid();

    szTemp= getenv("PATH");
    if (szTemp == NULL)
    {
        moreDirs= False;
    }
    else
    {
        strncpy(szPathVar, szTemp, MAXPATHLEN);
        szPathVar[MAXPATHLEN]= 0;
        iPathVarLen= strlen(szPathVar);
    }

    while ((!foundDir) && (moreDirs))
    {
        /* find the current directory name */
        for (i= iCurrentPathStart; (i < iPathVarLen) && (szPathVar[i] != ':');
            )
        {
            ++i;
        }
        iCurrentPathLen= i - iCurrentPathStart;
        if ((iCurrentPathLen + iExeNameLen + 2) > MAXPATHLEN)
        {
            iCurrentPathLen= MAXPATHLEN - (iExeNameLen + 2);
        }

        /* create a possible path to the executable */
        strncpy(szCurrentPath, &szPathVar[iCurrentPathStart], iCurrentPathLen);
        szCurrentPath[iCurrentPathLen]= 0;
        strcat(szCurrentPath, "/");
        strcat(szCurrentPath, szExeName);

        /* see if the executable exists (and we can execute it) */
        if (path_is_executable(szCurrentPath, euid, egid))
        {
            foundDir= True;

        }

        /* skip past the current directory name */
        if (!foundDir)
        {
            iCurrentPathStart+= iCurrentPathLen;
            while (   (iCurrentPathStart < iPathVarLen)
                   && (szPathVar[iCurrentPathStart] != ':') )
            {
                ++iCurrentPathStart;    /* find : */
            }
            if (iCurrentPathStart < iPathVarLen)
            {
                ++iCurrentPathStart;    /* skip : */
            }
            if (iCurrentPathStart >= iPathVarLen)
            {
                moreDirs= False;
            }
        }
    } /* while !foundDir */

    if (foundDir)
    {
        szCurrentPath[iCurrentPathLen]= 0;
        strncpy(buf, szCurrentPath, bufSize);
        buf[bufSize-1]= 0;
    }
    return foundDir? 0:-1;
}
/*********************************************************************/
/*
 * returns False is path does not exist or is not executable
 */
static Boolean
path_is_executable(
    char        *path,
    uid_t       euid,
    gid_t       egid
)
{
    Boolean     bExecutable= False;
    struct stat sStat;

    if (stat(path, &sStat) == 0)
    {
        Boolean bDetermined= False;

        if (!bDetermined)
        {
            if (!S_ISREG(sStat.st_mode))
            {
                /* not a regular file */
                bDetermined= True;
                bExecutable= False;
            }
        }

        if (!bDetermined)
        {
            if (   (euid == 0)
                && (   ((sStat.st_mode & S_IXOTH) != 0)
                    || ((sStat.st_mode & S_IXGRP) != 0)
                    || ((sStat.st_mode & S_IXUSR) != 0) )
               )
            {
                bDetermined= True;
                bExecutable= True;
            }
        }

        if (!bDetermined)
        {
            if (   (((sStat.st_mode & S_IXOTH) != 0)    )
                || (((sStat.st_mode & S_IXGRP) != 0) && (sStat.st_gid == egid))
                || (((sStat.st_mode & S_IXUSR) != 0) && (sStat.st_gid == euid))
               )
            {
                bDetermined= True;
                bExecutable= True;
            }
        }
    } /* if stat */

    return bExecutable;
}
/************************************************************************/
/* AlertPrompt code... */
int
AlertPrompt(
    Widget frame,
    ... )
{
    static int		answer;
    int			dialog_type;
    int			button_cnt = 0;
    int			msg_cnt = 0;
    int			button_id = 0;
    int			modality = XmDIALOG_MODELESS;
    char		*str, *ptr;
    char		*help_str = NULL;
    XmString		xmstring;
    Widget		pshell, dialog, button, label;
    va_list		ap;
    int			op;
    Atom		delete_window; 

    va_start( ap, frame );
    op = va_arg( ap, int );
    while ( op )
    {
        switch ( op )
        {
            case DIALOG_TYPE:
                dialog_type = va_arg( ap, int );
                dialog = XmCreateMessageDialog( frame, "AlertDialog", NULL, 0 );
                XtVaSetValues( dialog,
                    XmNdialogType, dialog_type,
                    XmNnoResize, True,
                    XmNverticalSpacing, 0,
                    NULL );
                pshell = XtParent( dialog );
                delete_window = XmInternAtom( XtDisplay(frame),
                    "WM_DELETE_WINDOW", False );
                XmAddWMProtocolCallback( pshell,
                    delete_window, MessageDialogQuitHandler, &answer );
                break;

            case DIALOG_TITLE:
                str = va_arg( ap, char * );
                if ( str && pshell )
                    XtVaSetValues( pshell,
                        XtNtitle, str,
                        NULL );
                break;

            case DIALOG_STYLE:
                modality = va_arg( ap, int );
                break;

            case DIALOG_TEXT:
                str = va_arg( ap, char * );
                if ( str && dialog )
                {
                    xmstring = XmStringCreateLocalized( str );
                    XtVaSetValues( dialog, XmNmessageString, xmstring, NULL );
                    XmStringFree( xmstring );
                }
                break;

            case HELP_IDENT:
                help_str = va_arg( ap, char * );
                break;

            case BUTTON_IDENT:
                button_id = va_arg( ap, int );
                str = va_arg( ap, char * );
                ++button_cnt;
                button = XtVaCreateManagedWidget( str,
                             xmPushButtonGadgetClass, dialog,
                             XmNuserData, button_id,
                             NULL);
                XtAddCallback( button,
                    XmNactivateCallback, MessageDialogResponse_CB, &answer );
                break;

            default:
                break;
        }
        op = va_arg( ap, int );
    }
    va_end( ap );

    /*
     * Unmanage buttons so we can display our own.
     */
    button = XmMessageBoxGetChild( dialog, XmDIALOG_OK_BUTTON );
    if ( button )
        XtUnmanageChild( button );
    button = XmMessageBoxGetChild( dialog, XmDIALOG_CANCEL_BUTTON );
    if ( button )
        XtUnmanageChild( button );
    button = XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON );
    if ( button )
    {
        if ( help_str != NULL )
        {
            XtAddCallback( button,
                XmNactivateCallback, AlertPromptHelp_CB, help_str );
        }
        else
            XtUnmanageChild( button );
    }

    /*
     * Set modality
     */
    XtVaSetValues( dialog,
        XmNdialogStyle, modality,
        NULL );

    XtManageChild( dialog );
    XBell( XtDisplay(frame), 0 );

    XtPopup( pshell, XtGrabNone );

    answer = -1;
    while( answer == -1 )
        XtAppProcessEvent( appContext, XtIMXEvent | XtIMAll );

    XtDestroyWidget( pshell );
    XSync( XtDisplay(frame), 0 );

    return( answer );
}

void
AlertPromptHelp_CB(
    Widget	w,
    XtPointer	clientData,
    XtPointer	cbs )
{
    static Widget quickHelpDialog = (Widget)NULL;
    Widget button;

    /* Create shared help dialog */
    if( quickHelpDialog == (Widget)NULL )
    {
        quickHelpDialog = DtCreateHelpQuickDialog( w,
            "QuickHelp", NULL, 0 );
        XtVaSetValues( XtParent( quickHelpDialog ),
            XmNtitle, "Properties Help",
            DtNhelpVolume, NULL,
            NULL );

        XtUnmanageChild( DtHelpQuickDialogGetChild( quickHelpDialog,
            DtHELP_QUICK_BACK_BUTTON ) );
    }

    XtVaSetValues( quickHelpDialog,
        DtNhelpType, DtHELP_TYPE_DYNAMIC_STRING,
        DtNstringData, (char *)clientData,
        NULL );

    /* Now display the help dialog */
    XtManageChild( quickHelpDialog );
}
/*
 * Close button was selected from the window manager, so treated it
 * as the equivalent of Cancel. 
 */

void
MessageDialogQuitHandler(
    Widget	w,
    XtPointer	clientData,
    XtPointer	callData )
{
    int *result = (int *)clientData;
    *result = ANSWER_CANCEL;
}


void
MessageDialogResponse_CB(
    Widget	w,
    XtPointer	clientData,
    XtPointer	cbs )
{
    XtVaGetValues( w,
        XmNuserData, (int *)clientData,
        NULL );
}
