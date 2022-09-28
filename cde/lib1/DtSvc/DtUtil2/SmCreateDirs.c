/* $XConsortium: SmCreateDirs.c /main/cde1_maint/1 1995/07/17 18:15:14 drk $ */
/*                                                                            *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                           *
 * (c) Copyright 1993, 1994 International Business Machines Corp.             *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                            *
 * (c) Copyright 1993, 1994 Novell, Inc.                                      *
 */
/******************************************************************************
 *
 * File Name: SmCreateDirs.c
 *
 * Contains the _DtCreateDtDirs function - which creates the directories
 * needed to store session info - if they need to be created
 *
 *****************************************************************************/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Intrinsic.h>
#include <Dt/DtNlUtils.h>
#include <Dt/DtPStrings.h>

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO
static int SmGetShortHostname();

#else
static int SmGetShortHostname(char*, unsigned int);

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

/*************************************<->*************************************
 *
 *  _DtCreateDtDirs (display)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

char * 
#ifdef _NO_PROTO
_DtCreateDtDirs( display )
        Display *display ;
#else
_DtCreateDtDirs(
        Display *display )
#endif /* _NO_PROTO */
{
    char *tmpPath, *tmpPath2;
    char hostName[101], *pch, *tmpNumber = NULL;
    char displayName[101];
    char buildPath[MAXPATHLEN];
    Boolean oldStyleDir = False;
    struct stat buf;
    int status;
    char *home;


    if ((home =getenv("HOME")) == NULL)
      home = "";
    
    /*
     * Creates the dt directories if they don't exist.  Returns the
     * total path of where information should be saved if the directories
     * exist or are created.  Otherwise, it returns null.
     */
    
    /*
     * This function returns a fully qualified path.  Use XtFree() to free the
     * memory
     */
    tmpPath = (char *) XtMalloc((MAXPATHLEN + 1) * sizeof(char));
    if(tmpPath == NULL)
    {
	return(NULL);
    }

    /*
     *  Check to see if the $HOME/.dt directory exists.  If not, we need
     *  to create it
     */
    strcpy(tmpPath, home);
    strcat(tmpPath, "/" DtPERSONAL_CONFIG_DIRECTORY);
    
    status = stat(tmpPath, &buf);
    if(status == -1)
    {
	
	/*
	 * dt directory doesn't exist - create it
	 */
	status = mkdir(tmpPath, 0000);
	if(status == -1)
	{
	    XtFree(tmpPath);
	    return(NULL);
	}
	(void)chmod(tmpPath, 0755);
    }
    else if (display != NULL) {
	/*
	 *  If a .dt directory exists - check for a display dependent
	 *  directory.  If one exists, that's what we use.  This is done
	 *  for backward compatibility - THE DISPLAY DEPENDENT DIR TAKES
	 *  PRECEDENT.
	 */
    
	/*
	 * Create the display name and append it to the current path.
	 */
	(void)strcpy(hostName, display->display_name);
	(void)strcpy(displayName, display->display_name);
    
	/*
	 * If this is run to unix or local get the host name - otherwise
	 * just use what we have
	 */
    
	/*
	 * Strip host name to nothing but the unqualified (short) host name
	 */
	if (pch = DtStrchr(hostName, ':'))
	    *pch = '\0';

	if (pch = DtStrchr(hostName, '.'))
	    *pch = '\0';

	if((!strcmp(hostName, "unix")) || (!strcmp(hostName, "local"))
	   || (!strcmp(hostName, "")))
	{
	    /*
	     * host name is local - get the real name
	     */
	    (void) SmGetShortHostname(hostName, 25);
	}
    
	/*
	 * Strip screen off of display name
	 */
	if (tmpNumber = DtStrchr(displayName, ':'))
	    if (pch = DtStrchr(tmpNumber, '.'))
		*pch = '\0';

	/*
	 * Strip it down to 14 chars (actually, 14 bytes or less)
	 */
	if((strlen(tmpNumber) + strlen(hostName)) > 14)
	{
	    int tnLen;
	    int lastChLen;
	    char * lastCh;

	    /* Pare display number to at most 12 bytes */
	    while ((tnLen = strlen(tmpNumber)) > 12)
	    {
		/* Remove the last character, an try again */
		DtLastChar(tmpNumber, &lastCh, &lastChLen);
		*lastCh = '\0';
	    }

	    /* Pare down host name, if necessary */
	    while ((strlen(hostName) + tnLen) > 14)
	    {
		/* Remove the last character, and try again */
		DtLastChar(hostName, &lastCh, &lastChLen);
		*lastCh = '\0';
	    }
	}
    
	(void)strcat(hostName, tmpNumber);
	strcpy(buildPath, tmpPath);
	strcat(buildPath, "/");
	strcat(buildPath, hostName);

	status = stat(buildPath, &buf);
	if(status != -1)
	{
	    strcpy(tmpPath, buildPath);
	    oldStyleDir = True;
	}
    }

    if(oldStyleDir == False)
    {
	/*
	 *  If we don't have an old style directory - we check for a sessions
	 *  directory, and create it if it doesn't exist
	 */
	strcat(tmpPath, "/");
	strcat(tmpPath, DtSM_SESSION_DIRECTORY);

	status = stat(tmpPath, &buf);
	if(status == -1)
	{
	    status = mkdir(tmpPath, 0000);
	    if(status == -1)
	    {
		XtFree(tmpPath);
		return(NULL);
	    }
	    (void)chmod(tmpPath, 0755);
	}
    }
	    

    /*
     * Create the personal DB directory and TOOLS directory if they
     * don't exist.  If a failure occurs while building these, keep
     * going.
     */
    strcpy(buildPath, home);
    strcat(buildPath, "/" DtPERSONAL_DB_DIRECTORY);
    
    if ((status = stat (buildPath, &buf)) == -1)
    {
	    if ((status = mkdir (buildPath, 0000)) != -1)
		    (void) chmod (buildPath, 0755);
    }


    /*
     * If the user's personal tmp dir doesn't exist, create it.
     */
    strcpy(buildPath, home);
    strcat(buildPath, "/" DtPERSONAL_TMP_DIRECTORY);

    if ((status = stat (buildPath, &buf)) == -1)
    {
	if ((status = mkdir (buildPath, 0000)) != -1) {
	    (void) chmod (buildPath, 0755);
	}
    }

    return(tmpPath);

} /* END OF FUNCTION _DtCreateDtDirs */

static int
#ifdef _NO_PROTO
SmGetShortHostname( buffer, bufsize )
        char *buffer ;
        unsigned int bufsize ;
#else
SmGetShortHostname(
        char *buffer,
        unsigned int bufsize )
#endif /* _NO_PROTO */
/*------------------------------------------------------------------------+*/
{
   char * ptr;
   int status;

   if (status = gethostname(buffer, bufsize))
      return status; /* failed gethostname */
   if (ptr = strstr(buffer, (char *)"."))
      *ptr = NULL;  /* delete domain name if there is one */
   return 0;
}
