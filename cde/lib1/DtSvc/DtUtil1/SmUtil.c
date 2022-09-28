/* $XConsortium: SmUtil.c /main/cde1_maint/1 1995/07/17 18:07:32 drk $ */
/*                                                                            *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                           *
 * (c) Copyright 1993, 1994 International Business Machines Corp.             *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                            *
 * (c) Copyright 1993, 1994 Novell, Inc.                                      *
 */
/******************************************************************************
 *
 * File Name: SmUtil.c
 *
 *  Contains the DT functions used by an application to communicate with
 *  dtsession.
 *
 *****************************************************************************/

#if 0
#include <sys/param.h>
#include <sys/types.h>
#endif

#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Dt/WsmP.h>
#include <Dt/DtP.h>
#include <Dt/Session.h>
#include <Dt/SmCreateDirs.h>

#ifndef CDE_INSTALLATION_TOP
#define CDE_INSTALLATION_TOP "/opt/dt"
#endif

/********    Static Function Declarations    ********/

#ifdef _NO_PROTO
static char *getSessionName();
static Boolean getSessionPath();
#else
static char *getSessionName(Display *, Atom);
static Boolean getSessionPath( Widget, char *, char **, char **);
#endif /* _NO_PROTO */

/********    End Static Function Declarations    ********/

/*************************************<->*************************************
 *
 *  getSessionName (display, prop )
 *
 *
 *  Description:
 *  -----------
 *  Returns the session name. 
 *
 *  Inputs:
 *  ------
 *  display - the display
 *  prop - the property name of the save or restore session
 *
 *  Outputs:
 *  -------
 *  
 *  Return:
 *  ------
 *  Returns the session name string or NULL if it could not be obtained. 
 *  This value should be freed with XFree().
 *  
 *
 *************************************<->***********************************/

static char *
#ifdef _NO_PROTO
getSessionName( display, prop )
        Display *display;
        Atom prop;
#else
getSessionName(
        Display *display,
        Atom prop)
#endif
{
  int                 propStatus;
  Atom                actualType;
  int                 actualFormat;
  unsigned long       nitems;
  unsigned long       leftover;
  char               *property = NULL;

  propStatus = XGetWindowProperty (display, RootWindow(display, 0), 
                                   prop, 0L,
                                   1000000L, False,
                                   AnyPropertyType, &actualType,
                                   &actualFormat, &nitems, &leftover, 
                                   (unsigned char **)&property);

 
  if(propStatus == Success &&
     actualType != None &&
     actualFormat == 8 &&
     nitems != 0)
  {
    return(property);
  }
 
  if (property)
  {
    XFree(property);
  }

  return(NULL);
}

/*************************************<->*************************************
 *
 *  getSessionPath (widget, propstring, savePath, saveFile)
 *
 *
 *  Description:
 *  -----------
 *  This function generates a full path name for an application's state
 *  file. If *saveFile is NULL, a new file name is generated, else
 *  *saveFile is used. It returns True if the path is returned, False
 *  otherwise.
 *
 *
 *  Inputs:
 *  ------
 *  widget - a widget to use to get the display
 *  propstring - session name property
 *  savePath - pointer to memory in which to place pointer to path
 *  saveFile - pointer to filename. If *saveFile is NULL, a new filename
 *             will be allocated and returned in *saveFile, else *saveFile
 *             will be used to generate path name
 *
 *  Outputs:
 *  -------
 *  True - path name returned
 *  False - path name not returned
 *
 *************************************<->***********************************/

static Boolean
#ifdef _NO_PROTO
getSessionPath( widget, propstring, savePath, saveFile )
        Widget widget ;
        char *propstring;
        char **savePath ;
        char **saveFile ;
#else
getSessionPath(
        Widget widget,
        char *propstring,
        char **savePath,
        char **saveFile )
#endif
{
    Display 		*display;
    char 		*tmpPath = NULL;
    char        	*property = NULL;
    char                *fileName;
    struct stat  	buf;
    int 		status;
    
    display = XtDisplay(widget); 
    
    tmpPath = _DtCreateDtDirs(display);
    if (tmpPath == NULL) goto abort;
    
    property = getSessionName(display, 
                  XInternAtom(display, propstring, False));
    if (property == NULL) goto abort;

   /*
    * NOTE: it is assumed that _DtCreateDtDirs() returns a buffer of 
    *       size MAXPATHLEN+1. This allows us to avoid a extra alloc
    *       and copy -- at the expense of code maintainability.
    */
    if ((strlen(tmpPath) + 1 + strlen(property)) > MAXPATHLEN) goto abort;

   /* 
    * parse the property string and create directory if needed 
    */
    (void)strcat(tmpPath, "/");
    (void)strcat(tmpPath, property);
    status = stat(tmpPath, &buf);

   /*
    * directory does not exist.
    */
    if(status == -1)
    {
        status = mkdir(tmpPath, 0000);
        if(status == -1) goto abort;

        (void)chmod(tmpPath, 0755);
    }

    (void)strcat(tmpPath, "/");

    if (*saveFile == NULL)
    {
     /*
      * No saveFile name was provided, so generate a new one.
      */
      int len = strlen(tmpPath);

      (void)strcat(tmpPath, "dtXXXXXX");
      (void)mktemp(tmpPath);

      *saveFile = (char *) XtMalloc(15 * sizeof(char));
      if(*saveFile == NULL) goto abort;

      (void)strcpy(*saveFile, tmpPath+len);
    }
    else
    {
     /*
      * A saveFile name was provided, so use it.
      */
      (void)strcat(tmpPath, *saveFile);
    }

    *savePath = tmpPath;

    XFree ((char *)property);
    return(True);

  abort:
   /*
    * ObGoto: if it clarifies the logic and reduces code, 
    *         goto's are ok by me.
    */
    *savePath = NULL;
    if (tmpPath) XtFree ((char *)tmpPath);
    if (property) XFree ((char *)property);
    return(False);
}


/*************************************<->*************************************
 *
 *  DtSessionSavePath (widget, savePath, saveFile)
 *
 *
 *  Description:
 *  -----------
 *  This function returns (in it's parameters) the full path name for an
 *  application to save to, as well as the file name to save away for later
 *  restoration.  It returns True if the path is returned, False
 *  otherwise.
 *
 *
 *  Inputs:
 *  ------
 *  widget - a widget to use to get the display
 *
 *  Outputs:
 *  -------
 *  returns a status value
 *
 *************************************<->***********************************/

Boolean 
#ifdef _NO_PROTO
DtSessionSavePath( widget, savePath, saveFile )
        Widget widget ;
        char **savePath ;
        char **saveFile ;
#else
DtSessionSavePath(
        Widget widget,
        char **savePath,
        char **saveFile )
#endif /* _NO_PROTO */
{
    Boolean             result;

    *saveFile = NULL;
    result = getSessionPath(widget, _XA_DT_SAVE_MODE, savePath, saveFile);
    
    return(result);
} /* END OF FUNCTION DtSessionSavePath */


/*************************************<->*************************************
 *
 *  DtSessionRestorePath (widget, restorePath, restoreFile)
 *
 *
 *  Description:
 *  -----------
 *  This function returns (in its parameters), the path where the application
 *  is to restore its file from.
 *  It returns True if the path is returned, False
 *  otherwise.
 *
 *
 *  Inputs:
 *  ------
 *  widget - a widget to use to get the display
 *
 *  Outputs:
 *  -------
 *  returns a status value
 *
 *************************************<->***********************************/

Boolean 
#ifdef _NO_PROTO
DtSessionRestorePath( widget, savePath, saveFile )
        Widget widget ;
        char **savePath ;
        char *saveFile ;
#else
DtSessionRestorePath(
        Widget widget,
        char **savePath,
        char *saveFile )
#endif /* _NO_PROTO */
{
    Boolean             result;

    result = getSessionPath(widget, _XA_DT_RESTORE_MODE, savePath, &saveFile);
    
    return(result);
} /* END OF FUNCTION DtSessionRestorePath */
