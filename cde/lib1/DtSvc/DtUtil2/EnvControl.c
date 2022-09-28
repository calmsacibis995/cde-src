/****************************<+>*************************************
 ********************************************************************
 **
 **   File:     EnvControl.c
 **
 **   $XConsortium: EnvControl.c /main/cde1_maint/3 1995/09/06 02:16:25 lehors $
 **
 **   Project:  DtSvc Runtime Library
 **
 **   Description: Get/Set the client's shell environment
 **
 **(c) Copyright 1992,1993,1994 by Hewlett-Packard Company
 **(c) Copyright 1993,1994 International Business Machines Corp.
 **(c) Copyright 1993,1994 Sun Microsystems, Inc.
 **(c) Copyright 1993,1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
 **
 ********************************************************************
 ****************************<+>*************************************/
#include <stdlib.h>
#include <stdio.h>
#ifdef SVR4
#include <string.h>
#else
#include <strings.h>
#endif
#include <X11/Intrinsic.h>
#include <Dt/DtosP.h>
#include <Dt/DtNlUtils.h>
#include "EnvControl.h"
#include "EnvControlP.h"

/*
 *  Remove our dependency on the Xt toolkit library, so that clients
 *  can use us without having to link Xt.
 *
 */
#define XtMalloc(p)        malloc(p)
#define XtRealloc(p, t)    realloc(p, t)
#define XtFree(p)          (p ? free(p): (void) 0)

extern char	**environ;

 /* 
  *  We assume that the following fields of these static structures
  *  are initialized at runtime to NULL pointers.
  */
static struct environStruct _postDtEnvironment;
static struct environStruct _preDtEnvironment;

/* envBitVector is used to determine whether an environment
 * variable has been put to the environment by the application.
 * The flag values are in EnvControlP.h.
 */
static unsigned int envBitVector = 0;

static int environSetup = 0;

static void _DtEnvAdd (char *, char *, unsigned int);

static bin_mode = DT_ENV_NO_OP;

/****************************************************************
* NOTES on an application's environ(5):
* 
* The environment variable is an array of NULL-terminated strings.
* char **environ is defined in crt.o and is publically extern-ed to
* all applications.  There is one such variable per UN*X application.
* 
* To deference an environment string, as in the debugger, use:
* 
*        environ[0] -- environ[n-1], where n is the number of
*        name=value pairs.
* 
* E.g, in xdb:
* 
*        >p environ[0]
* 
* You must manually malloc() space to keep a specific environment string
* around.
* 
* putenv() is useless at NULLing out an environment variable.  It will
* create a new value and will replace an existing value; however,
* in terms of getting rid of an environment variable, putenv() will only
* replace it with a NULL string ("") rather than freeing the space 
* for it and NULLing the pointer.  Afterwards, getenv() will return
* the NULL string rather than a NULL pointer, and hence give "incorrect"
* information to an application querying for a non-set environment
* variable.  
*
* E.g., there's a big difference between the following pairs 
* to XmGetPixmap():
* 
*         XBMLANGPATH=""
* 
*         --and--
* 
*         XBMLANGPATH=<null pointer>
* 
* The first will cause XmGetPixmap() to look nowhere for an icon image;
* the second will cause a lookup in all the default locations, which
* is what you want.
* 
* Bottom line is that you have to NULL out an unneeded environ manually.
* 
* The environment array is able to be jerked any which way you'd like,
* using direct string/pointer manipulations.  However, for safety:
* make a local copy of **environ.
* 
********************************************************************/



/*
 * Common suffixes for icon pixmap and icon bitmap paths.  Each array must end
 * with a null pointer.  Note: the unusual string concatenation is necessary
 * in order to prevent expansion of SCCS id keywords.
 */

static const char *iconPmSuffixes[] = {
    "%B" "%M.pm",
    "%B" "%M.bm",
    "%B",
    NULL
};

static const char *iconBmSuffixes[] = {
    "%B" "%M.bm",
    "%B" "%M.pm",
    "%B",
    NULL
};


/*
 * makeDefaultIconPath
 * 
 * Construct the icon pixmap or bitmap search path by constructing the values
 * from the home directory, and then appending the system default path.  The
 * output is of the form "var=value", suitable for passing to putenv().
 *
 * Output:
 *
 * outbuf	A pointer to the memory to receive the output value.  This
 *		must be large enough to accommodate the largest anticipated
 *		environment variable; no bounds checking is done!
 * 
 * Inputs:
 * 
 * varname	The environment variable name.
 * basedefault	The default value to be appended to the end.
 * suffixes	An array of suffixes used in constructing the path elements in
 *		the user's home directory.  Must have a trailing NULL pointer.
 */
static void
makeDefaultIconPath(
	char		*outbuf,
	const char	*varname,
	const char	*basedefault,
	const char	**suffixes)
{
	char		*homedir;
	char		temp[MAX_ENV_STRING];

	(void)sprintf(outbuf, "%s=", varname);
	homedir = getenv("HOME");
	if (homedir != NULL) {
		for ( ; *suffixes ; ++suffixes) {
			(void)sprintf(temp, "%s/.dt/icons/%s:",
			    homedir, *suffixes);
			(void)strcat(outbuf, temp);
		}
	}
	(void)strcat(outbuf, basedefault);
}


/*
 * Construct a default value for XMICONSEARCHPATH into outbuf.
 */
static void
makeDefaultIconPmPath(char *outbuf)
{
	makeDefaultIconPath(outbuf, PM_PATH_ENVIRON, DTPMSYSDEFAULT,
			    iconPmSuffixes);
}


/*
 * Construct a default value for XMICONBMSEARCHPATH into outbuf.
 */
static void
makeDefaultIconBmPath(char *outbuf)
{
	makeDefaultIconPath(outbuf, BM_PATH_ENVIRON, DTBMSYSDEFAULT,
			    iconBmSuffixes);
}


/*****************************<->*************************************
 *

 *  _DtEnvControl ( int mode )
 *
 *
 *  Description:
 *  -----------
 *  Gets/Sets the application's environment 
 *
 *  Inputs:
 *  ------
 *  mode        - the command to the function; e.g., to set the
 *                DT environment, to restore the originial env., etc.
 *
 *  Returns:
 *  --------
 *  Returns a success code--usually just the value of the original
 *  mode parameter.  If an error occurs, returns DT_ENV_NO_OP.
 * 
 *****************************<->***********************************/
int
#ifdef _NO_PROTO
_DtEnvControl( mode )
        int mode ;
#else
_DtEnvControl(
        int mode )
#endif /* _NO_PROTO */
{
    char *tempString;  
    char *ptr; /* used for indexing into the $PATH */
    int returnValue = DT_ENV_NO_OP;
    char postDtEnvironmentString[MAX_ENV_STRING]; 
	/* for creating the name=value string */

    char preDtEnvironmentString[MAX_ENV_STRING];
	/* for creating the name=value string for the old environment */


    switch (mode) {

       	case DT_ENV_SET:
	    if (!environSetup)  /* first time through */
	    {
		/*
		 * Set up DT environment in the application 
		 * environment, while stashing the old environment
		 * in the _preDtEnvironment structure 
		 *
		 * Note: this code will not check for duplicate
		 * environment values--it will append or prepend
		 * the DT environment values regardless of what's
		 * in the current environment.
		 *
		 *  Of the form:
		 *
		 *  PATH = /opt/dt/bin :  originalPath
		 *  %s  %s    %s        %s    %s
		 *
		 *
		 *  XBMLANGPATH = originalPath : dtPath
		 *        %s    %s    %s       %s    %s
		 *
		 *  Note: Of all the environmental vars, ONLY $PATH
		 *  is jammed with the DT value first.
		 *
		 *  Check for NULL environment strings before the sprintf
		 *
		 */

		/*
		 * Get the application's original environment 
		 * to save it in the _preDtEnvironment structure 
		 *
		 * We save in the _preDtEnvironment structure
		 * only existing (non-NULL getenv()) values.
		 *
		 */

		/* 
		 *  Time to prepend "/opt/dt/bin" to the
		 *  $PATH component.
		 */
		tempString = getenv(BIN_PATH_ENVIRON);

		/*
		 *  First, 'tho', ensure that /opt/dt/bin isn't already in
		 *  the $PATH.
		 */
		if (!tempString || !(strstr(tempString, BIN_PATH_STRING)))
		{
		    if (!tempString)
		    {
			/* 
			 * The user has no existing value for this
			 * PATH environment variable.  Don't save it.
			 * Just create the new DT environment.
			 */

			sprintf(postDtEnvironmentString,"%s%s%s",
				BIN_PATH_ENVIRON,
				"=",
				BIN_PATH_STRING
				);
		    }
		    else
		    {
		    /* save the path in the preDtEnvironmentString */
			sprintf(preDtEnvironmentString,"%s%s%s",
			    BIN_PATH_ENVIRON,
			    "=",
			    tempString
			    );

			_preDtEnvironment.binPath = (char *)
			    (XtMalloc(strlen(preDtEnvironmentString) + 1));

			strcpy(_preDtEnvironment.binPath, preDtEnvironmentString);

#ifdef sun
			if (ptr = strstr(tempString, "/usr/openwin/bin"))
#else
			if (ptr = strstr(tempString, "/usr/bin/X11"))
#endif
			{
			     /* 
			      * Shorten the string in tempString
			      * to the initial segment, up to the 
			      * initial slash in "/usr/bin/X11"
			      */
			     if (ptr != tempString)
			     {
			        /* 
				 * then put our dt string just ahead of 
				 * "/usr/bin/X11" in the new
				 * PATH 
				 */

			         char * newS = XtMalloc(strlen(tempString) +
					strlen(BIN_PATH_STRING) + 4);

				 *(ptr - 1) = NULL; 

				 strcpy(newS, tempString);

				 strcat(newS, ":");

				 strcat(newS, BIN_PATH_STRING);

				 strcat(newS, ":");

				 strcat(newS, ptr);

				 sprintf(postDtEnvironmentString,
				   "%s%s%s",
				    BIN_PATH_ENVIRON,
				    "=",
				    newS
				    );
				 XtFree(newS);
			     }
			     else
			     {
				/*
				 *  Turns out that "/usr/bin/X11"
				 *  is at the front of the PATH, so...
				 */
				sprintf(postDtEnvironmentString,
				    "%s%s%s%s%s",

				    BIN_PATH_ENVIRON,
				    "=",
				    BIN_PATH_STRING,
				    ":",
				    tempString
				    );
			    }

			}
			else if (ptr = strstr(tempString, "/usr/bin"))
			{
			     /* 
			      * Shorten the string in tempString
			      * to the initial segment, up to the 
			      * initial slash in "/usr/bin"
			      */
			     if (ptr != tempString)
			     {
			        /* 
				 * then put our dt string just ahead of 
				 * "/usr/bin" in the new
				 * PATH 
				 */

			         char * newS = XtMalloc(strlen(tempString) +
					strlen(BIN_PATH_STRING) + 4);

				 *(ptr - 1) = NULL; 

				 strcpy(newS, tempString);

				 strcat(newS, ":");

				 strcat(newS, BIN_PATH_STRING);

				 strcat(newS, ":");

				 strcat(newS, ptr);

				 sprintf(postDtEnvironmentString,
				   "%s%s%s",
				    BIN_PATH_ENVIRON,
				    "=",
				    newS
				    );
				 XtFree(newS);
			     }
			     else
			     {
				/*
				 *  Turns out that "/usr/bin"
				 *  is at the front of the PATH, so...
				 */
				sprintf(postDtEnvironmentString,
				    "%s%s%s%s%s",

				    BIN_PATH_ENVIRON,
				    "=",
				    BIN_PATH_STRING,
				    ":",
				    tempString
				    );
			    }
			}
			else
			{
			    /* 
			     * Put our dt string on the
			     * front of the PATH 
			     */

			    sprintf(postDtEnvironmentString,"%s%s%s%s%s",
				    BIN_PATH_ENVIRON,
				    "=",
				    BIN_PATH_STRING,
				    ":",
				    tempString
				    );
			}
		    }
		    _postDtEnvironment.binPath = (char *)
			    (XtMalloc(strlen(postDtEnvironmentString) + 1));

		    strcpy(_postDtEnvironment.binPath, postDtEnvironmentString);

		    _DtEnvAdd (BIN_PATH_ENVIRON, 
			       _postDtEnvironment.binPath, 
			       BV_BINPATH);
		}
		tempString = getenv(PM_PATH_ENVIRON);
		if (tempString)
		{
		    sprintf(preDtEnvironmentString,"%s%s",
				PM_PATH_ENVIRON,
				"=");

		    _preDtEnvironment.pmPath = (char *)
			    (XtMalloc(strlen(preDtEnvironmentString) + 
				      strlen(tempString) + 1));

		    sprintf(_preDtEnvironment.pmPath,"%s%s",
				preDtEnvironmentString,
				tempString);
		} else {
		    /* it doesn't exist, so generate a default value */
		    makeDefaultIconPmPath(postDtEnvironmentString);
		    _postDtEnvironment.pmPath =
			strdup(postDtEnvironmentString);
		   _DtEnvAdd(PM_PATH_ENVIRON, _postDtEnvironment.pmPath,
			BV_PMPATH);
		}
		tempString = getenv(BM_PATH_ENVIRON);
		if (tempString)
		{
		    sprintf(preDtEnvironmentString,"%s%s",
				BM_PATH_ENVIRON,
				"=");

		    _preDtEnvironment.bmPath = (char *)
			    (XtMalloc(strlen(preDtEnvironmentString) + 
				      strlen(tempString) + 1));

		    sprintf(_preDtEnvironment.bmPath,"%s%s",
				preDtEnvironmentString,
				tempString);
		} else {
		    /* it doesn't exist, so generate a default value */
		    makeDefaultIconBmPath(postDtEnvironmentString);
		    _postDtEnvironment.bmPath =
			strdup(postDtEnvironmentString);
		    _DtEnvAdd(BM_PATH_ENVIRON, _postDtEnvironment.bmPath,
			BV_BMPATH);
		}

		/* Do the admin for the NLSPATH env variable */

		tempString = getenv(NLS_PATH_ENVIRON);

		if (!tempString)
		{

		  /* If it doesn't exist, set it to the CDE default */

			sprintf(postDtEnvironmentString,"%s%s%s",
				NLS_PATH_ENVIRON,
				"=",
				NLS_PATH_STRING
				);
		}
		else
		{

		  /* If it does exist, store it away so it can be
		   * restored afterwards.....
		   */
			sprintf(preDtEnvironmentString,"%s%s%s",
				NLS_PATH_ENVIRON,
				"=",
				tempString
				);

			_preDtEnvironment.nlsPath = (char *)
				(XtMalloc(strlen(preDtEnvironmentString) + 1));

			strcpy(_preDtEnvironment.nlsPath, 
			       preDtEnvironmentString);

		  /* ... then append the CDE default to the existing
		   * value
		   */
			sprintf(postDtEnvironmentString,"%s%s%s%s%s",
				NLS_PATH_ENVIRON,
				"=",
				tempString,
				":",
				NLS_PATH_STRING
				);
		}
		_postDtEnvironment.nlsPath = (char *)
			(XtMalloc(strlen(postDtEnvironmentString) + 1));

		strcpy(_postDtEnvironment.nlsPath, postDtEnvironmentString);

		_DtEnvAdd (NLS_PATH_ENVIRON, 
			   _postDtEnvironment.nlsPath,
			   BV_NLSPATH);

		tempString = getenv(SYSTEM_APPL_PATH_ENVIRON);

		if (!tempString)
		{
			sprintf(postDtEnvironmentString,"%s%s%s",
				SYSTEM_APPL_PATH_ENVIRON,
				"=",
				SYSTEM_APPL_PATH_STRING
				);
		}
		else
		{
			sprintf(preDtEnvironmentString,"%s%s%s",
				SYSTEM_APPL_PATH_ENVIRON,
				"=",
				tempString
				);

			_preDtEnvironment.sysApplPath = (char *)
				(XtMalloc(strlen(preDtEnvironmentString) + 1));

			strcpy(_preDtEnvironment.sysApplPath, preDtEnvironmentString);

			sprintf(postDtEnvironmentString,"%s%s%s%s%s",
				SYSTEM_APPL_PATH_ENVIRON,
				"=",
				tempString,
				":",
				SYSTEM_APPL_PATH_STRING
				);
		}
		_postDtEnvironment.sysApplPath = (char *)
			(XtMalloc(strlen(postDtEnvironmentString) + 1));

		strcpy(_postDtEnvironment.sysApplPath, postDtEnvironmentString);

		_DtEnvAdd (SYSTEM_APPL_PATH_ENVIRON,
			   _postDtEnvironment.sysApplPath,
			   BV_SYSAPPLPATH);

		environSetup = 1;

	        returnValue = DT_ENV_SET;

	    }
	    else /* we've already been here -- do nothing */
	    {
		returnValue = DT_ENV_NO_OP;
	    }
	    break;

	case  DT_ENV_RESTORE_PRE_DT:
       	    if (environSetup) 
            {
		if (_preDtEnvironment.nlsPath) {
		    _DtEnvAdd (NLS_PATH_ENVIRON, 
			       _preDtEnvironment.nlsPath,
			       BV_NLSPATH);
		}
		else {
		    _DtEnvRemove(NLS_PATH_ENVIRON, 0);
		    envBitVector &= ~BV_NLSPATH;
		}

		if (_preDtEnvironment.sysApplPath) {
		    _DtEnvAdd (SYSTEM_APPL_PATH_ENVIRON, 
			       _preDtEnvironment.sysApplPath,
			       BV_SYSAPPLPATH);
		}
		else {
		    _DtEnvRemove(SYSTEM_APPL_PATH_ENVIRON, 0);
		    envBitVector &= ~BV_SYSAPPLPATH;
		}

		if (_preDtEnvironment.pmPath) {
		    _DtEnvAdd (PM_PATH_ENVIRON,
			       _preDtEnvironment.pmPath,
			       BV_PMPATH);
		}
		else {
		    _DtEnvRemove(PM_PATH_ENVIRON, 0);
		    envBitVector &= ~BV_PMPATH;
		}

		if (_preDtEnvironment.bmPath) {
		    _DtEnvAdd (BM_PATH_ENVIRON,
			       _preDtEnvironment.bmPath,
			       BV_BMPATH);
		}
		else {
		    _DtEnvRemove(BM_PATH_ENVIRON, 0);
		    envBitVector &= ~BV_BMPATH;
		}

	        returnValue = DT_ENV_RESTORE_PRE_DT;
            }
            else
	    {
		returnValue = DT_ENV_NO_OP;
	    }
	    break;

	case DT_ENV_RESTORE_POST_DT:
       	    if (environSetup) 
            {

		if (_postDtEnvironment.nlsPath) {
		    _DtEnvAdd (NLS_PATH_ENVIRON, 
			       _postDtEnvironment.nlsPath,
			       BV_NLSPATH);
		}

		if (_postDtEnvironment.pmPath) {
		    _DtEnvAdd (PM_PATH_ENVIRON,
			       _postDtEnvironment.pmPath,
			       BV_PMPATH);
		}

		if (_postDtEnvironment.bmPath) {
		    _DtEnvAdd (BM_PATH_ENVIRON,
			       _postDtEnvironment.bmPath,
			       BV_BMPATH);
		}

		if (_postDtEnvironment.sysApplPath) {
		    _DtEnvAdd (SYSTEM_APPL_PATH_ENVIRON, 
			       _postDtEnvironment.sysApplPath,
			       BV_SYSAPPLPATH);
		}

	        returnValue = DT_ENV_RESTORE_POST_DT;
	    }
            else
	    {
		returnValue = DT_ENV_NO_OP;
	    }
	    break;

	default:
	    /* do nothing */
	    break;
    }
#ifdef DEBUG
	switch (mode) {
	case DT_ENV_SET:
		printf("DT environment set:\n");
		printf("-------------------------------\n");
		break;

	case DT_ENV_RESTORE_PRE_DT:
		printf("Pre-DT environment restored:\n");
		printf("-------------------------------\n");
		break;

	case DT_ENV_RESTORE_POST_DT:
		printf("Post-DT environment restored:\n");
		printf("-------------------------------\n");
		break;

	case DT_ENV_NO_OP:
		printf("No change from last invocation:\n");
		printf("-------------------------------\n");
		break;
	}

	tempString = getenv(BIN_PATH_ENVIRON);
	printf("%s=|%s|\n", BIN_PATH_ENVIRON, tempString);
	tempString = getenv(SYSTEM_APPL_PATH_ENVIRON);
	printf("%s=|%s|\n", SYSTEM_APPL_PATH_ENVIRON, tempString);
	tempString = getenv(NLS_PATH_ENVIRON);
	printf("%s=|%s|\n", NLS_PATH_ENVIRON, tempString);
	tempString = getenv(PM_PATH_ENVIRON);
	printf("%s=|%s|\n", PM_PATH_ENVIRON, tempString);
	tempString = getenv(BM_PATH_ENVIRON); 
	printf("%s=|%s|\n", BM_PATH_ENVIRON, tempString);

#endif /* DEBUG */
	return (returnValue);

} /* END OF FUNCTION _DtEnvControl */

/*****************************<->*************************************
 *
 *  _DtEnvAdd(char * str)
 *
 *
 *  Description:
 *  -----------
 *  Adds the given string to the application's environment
 *  If the existing environment variable string was allocated
 *  by this application, the space is freed.
 *
 *  Inputs:
 *  ------
 *  envVar          The environment variable. E.g. NLSPATH
 *  envVarSetting   The environment variable setting. 
 *                  E.g. NLSPATH=/usr/lib/nls
 *  bv_flag         The constant denoting which environment variable
 *                  is being set.
 * 
 *****************************<->***********************************/
static void _DtEnvAdd
  (
   char * envVar,
   char * envVarSetting,
   unsigned int bv_flag
  )
{
  extern char **environ;

  if (envBitVector & bv_flag) {
      register int i;
      int envVarLen = strlen(envVar);
      char *envPtr = NULL;

    /* if we have previously put this environment variable out to the
     * environment, we can retrieve it and reuse it rather than letting
     * it disappear into the ether
     */

    for ( i = 0; environ[i] ; i++ )
    {
        if ( environ[i][0] == envVar[0]
             && ( strlen(environ[i]) >= envVarLen )
             && ( environ[i][envVarLen] == '=' ) 
             && !strncmp(envVar,environ[i],envVarLen))
        {
		envPtr = environ[i];
		break;
        }
    }
    
    if ( envPtr )
    {
            XtFree(envPtr);
            envPtr = XtMalloc(strlen(envVarSetting) + 1);
	    strcpy(envPtr,envVarSetting);
	    environ[i] = envPtr;
    }
    else
    {
            /* This should never happen */
	    putenv(strdup(envVarSetting));
    }
  }
  else
    putenv(strdup(envVarSetting));

  envBitVector |= bv_flag;
}

/*****************************<->*************************************
 *
 *  _DtEnvRemove(
 *      char *str,
 *      int length )
 *
 *
 *  Description:
 *  -----------
 *  Removes the given string from the application's environment
 *  Uses the char **extern environment string array.
 *
 *  Inputs:
 *  ------
 *  str         - the name of the variable to remove (e.g., "NLSPATH")
 *  length      - the length of the variable--we compute it dynamically.
 *
 *  Returns:
 *  --------
 *  Returns a success code--"0" if the variable is found and removed.
 *                          "1" if the variable isn't found.
 * 
 *****************************<->***********************************/
int
#ifdef _NO_PROTO
_DtEnvRemove( str, length )
        char *str ;
        int length ;
#else
_DtEnvRemove(
        char *str,
        int length )
#endif /* _NO_PROTO */
{
	char **pEnviron, **pEnviron2 = (char **)environ;
	char *p, *freeMe;
	int temp;
			 
	int count  = 0;  /* count is the number of items in the */
			 /* environ                             */
	int index;   /* index will range from 0 to count - 1 */
        int len;

	if (!(len = strlen(str)))
		return(1);

	pEnviron = pEnviron2;

	p = *pEnviron;

	while (p)
	{
		pEnviron++;
		count++;
		p = *pEnviron;
	}

	pEnviron = pEnviron2;
	p = *pEnviron;

	for  (index = 0; index < count; index++)
	{
		if (  p[0] == str[0] 
                      && ( strlen(p) >= len )
                      && ( p[len] == '=' )
                      && !strncmp(p, str, len))
		{
		    freeMe = pEnviron2[index];

		    /* just move the last one into the gap - any
		     * putenv destroyed the initial lexical ordering
		     * anyway
		     */

		    pEnviron2[index] = pEnviron2[count - 1];
		    pEnviron2[count - 1] = NULL;

		    XtFree (freeMe);
		    return(0);
		}
		pEnviron++;
		p = *pEnviron;
	}
	return(1);
}

int
#ifdef _NO_PROTO
_DtEnvPrint()
#else
_DtEnvPrint( void )
#endif /* _NO_PROTO */
{
	char **p, *q;

	p = (char **)(environ);

	q = *p;

	while (q)
	{
		printf("%s\n", q);
		p++;
		q = *p;
	}
	return 0;
}


/*************************************<->*************************************
 *
 *  _DtAddToPath (sPath, sDir)
 *
 *
 *  Description:
 *  -----------
 *  Append a directory to a directory path. 
 *
 *  Inputs:
 *  ------
 *  sPath	- the old path (must be a pointer to malloc'ed memory)
 *  sDir	- the directory to add to the path
 *
 *
 *  Returns:
 *  --------
 *  A pointer to the new path if successful.
 *  NULL on memory allocation failures.
 *
 *  Comment:
 *  --------
 *  This functions adds sDir to the path of directories already in
 *  sPath. Each directory is separated by a colon (':'). The function
 *  is designed to work off the heap. sPath should be a pointer into
 *  the heap on entry, or NULL. If sPath is NULL, then new memory is
 *  allocated and returned. If sPath is not NULL, the storage for sPath
 *  is reallocated to include space for the new string. In this case,
 *  the pointer returned may point to a different value than sPath
 *  that was passed in. If the pointer returned is different than sPath,
 *  then sPath is invalid and should be reassigned.
 *
 *************************************<->***********************************/
static char *
#ifdef _NO_PROTO
_DtAddToPath (sPath, sDir)
        char * sPath;
        char * sDir;
#else
_DtAddToPath (
	char * sPath,
	char * sDir)
#endif /* _NO_PROTO */
{
    char * sNew;

    if (sPath != NULL)
    {
	sNew = XtRealloc (sPath, 1+strlen(sPath)+1+strlen(sDir));
    }
    else
    {
	sNew = XtMalloc (1+strlen(sPath)+1+strlen(sDir));
    }

    strcat (sNew, ":");
    strcat (sNew, sDir);

    return (sNew);
}


/*************************************<->*************************************
 *
 *  _DtWsmSetBackdropSearchPath (screen, backdropPath);
 *
 *
 *  Description:
 *  -----------
 *  Append the specified directories to DTICONSEARCHPATH environment 
 *  variable.  This must be done before any icon lookup by XmGetPixmap.
 *  Each directory in backdropPath is modified so the backdrop file
 *  names can be matched (%B, %M, etc.).
 *
 *  _DtEnvControl( DT_ENV_RESTORE_PRE_DT ) will restore the original
 *  environment.
 *
 *  Inputs:
 *  ------
 *  screen		- screen of display 
 *  backdropPath	- a colon-separated list of directories
 *  useMultiColoricons 	- True if color backdrops are desired.
 *
 *  Returns:
 *  --------
 *  Returns a sucess code:    sucess: DT_ENV_SET
 *                            failure: DT_ENV_NO_OP
 *
 *************************************<->***********************************/
int
#ifdef _NO_PROTO
_DtWsmSetBackdropSearchPath (screen, backdropPath, useMultiColorIcons)
        Screen  *screen;
        char    *backdropPath;
        Boolean  useMultiColorIcons;
#else
_DtWsmSetBackdropSearchPath (
	Screen  *screen,
        char    *backdropPath,
        Boolean  useMultiColorIcons)

#endif /* _NO_PROTO */
{
    char *sBackDirs;
    char *sOldEnv;
    char *sNext;
    int returnValue = DT_ENV_NO_OP;

    char postDtEnvironmentString[MAX_ENV_STRING]; 
	/* for creating the name=value string */

    char preDtEnvironmentString[MAX_ENV_STRING];
	/* for creating the name=value string for the old environment */

    if (backdropPath == NULL) return (returnValue);

    sOldEnv = getenv(PM_PATH_ENVIRON);

    DtNlInitialize();
                                            
    if (!sOldEnv)
    {
	/* 
	 * The user has no existing value for this
	 * environment variable.  Don't save it.
	 * Just create the new DT environment.
	 */

	/* make a copy that we can modify locally */
	sBackDirs = XtMalloc (strlen(backdropPath)+1);
	strcpy (sBackDirs, backdropPath);

	/*
	 * Initialize path string
	 */
	if (useMultiColorIcons)
	    makeDefaultIconPmPath(postDtEnvironmentString);
	else
	    makeDefaultIconBmPath(postDtEnvironmentString);

	_postDtEnvironment.pmPath = 
		    XtMalloc (1+strlen(postDtEnvironmentString));
	strcpy (_postDtEnvironment.pmPath, postDtEnvironmentString);

	/* get first directory */
	sNext = DtStrtok (sBackDirs, ":");

	/* process each directory individually */
	while (sNext != NULL)
	{
	    if (useMultiColorIcons)
		sprintf(postDtEnvironmentString,
			"%s%s%s%s%s%s",  
			sNext, "/%B.pm:",
			sNext, "/%B.bm:",
			sNext, "/%B");
	    else
		sprintf(postDtEnvironmentString,
			"%s%s%s%s%s%s",  
			sNext, "/%B.bm:",
			sNext, "/%B.pm:",
			sNext, "/%B");

	    _postDtEnvironment.pmPath = 
		_DtAddToPath (_postDtEnvironment.pmPath,
				postDtEnvironmentString);

	    /* get next directory */
	    sNext = DtStrtok (NULL, ":");
	}

	XtFree (sBackDirs);
    }
    else
    {
	/* 
	 * Save the old path if not previously saved
	 */
	if (!_preDtEnvironment.pmPath)
	{
	    sprintf(preDtEnvironmentString,"%s%s",
			PM_PATH_ENVIRON,
			"=");

	    _preDtEnvironment.pmPath = (char *)
		    (XtMalloc(strlen(preDtEnvironmentString) + 
			      strlen(sOldEnv) + 1));

	    sprintf(_preDtEnvironment.pmPath,"%s%s",
			preDtEnvironmentString,
			sOldEnv);
	}

	/* make a copy that we can modify locally */
	sBackDirs = XtMalloc (strlen(backdropPath)+1);
	strcpy (sBackDirs, backdropPath);

	/*
	 * Initialize path environment string
	 */
	sprintf(postDtEnvironmentString, 
		    "%s%s%s",  
		    PM_PATH_ENVIRON, "=",
		    "%s");

	_postDtEnvironment.pmPath = 
	    XtMalloc (1+strlen(sOldEnv)+strlen(postDtEnvironmentString));
	sprintf (_postDtEnvironment.pmPath, postDtEnvironmentString, sOldEnv);

	/* get first directory */
	sNext = DtStrtok (sBackDirs, ":");

	/* process each directory individually */
	while (sNext != NULL)
	{
	    if (useMultiColorIcons)
		sprintf(postDtEnvironmentString, 
			"%s%s%s%s%s%s",  
			sNext, "/%B.pm:",
			sNext, "/%B.bm:",
			sNext, "/%B");
	    else
		sprintf(postDtEnvironmentString, 
			"%s%s%s%s%s%s",  
			sNext, "/%B.bm:",
			sNext, "/%B.pm:",
			sNext, "/%B");

	    _postDtEnvironment.pmPath = 
		_DtAddToPath (_postDtEnvironment.pmPath,
				postDtEnvironmentString);

	    /* get next directory */
	    sNext = DtStrtok (NULL, ":");
	}

	XtFree (sBackDirs);
    }

    putenv(strdup(_postDtEnvironment.pmPath));
    envBitVector |= BV_PMPATH;

    /*
     * Now do the same thing for the bitmap lookup path
     */
    sOldEnv = getenv(BM_PATH_ENVIRON);

    if (!sOldEnv)
    {
	/* 
	 * The user has no existing value for this
	 * environment variable.  Don't save it.
	 * Just create the new DT environment.
	 */

	/* make a copy that we can modify locally */
	sBackDirs = XtMalloc (strlen(backdropPath)+1);
	strcpy (sBackDirs, backdropPath);

	/*
	 * Initialize path string
	 */
	makeDefaultIconBmPath(postDtEnvironmentString);

	_postDtEnvironment.bmPath = 
		    XtMalloc (1+strlen(postDtEnvironmentString));
	strcpy (_postDtEnvironment.bmPath, postDtEnvironmentString);

	/* get first directory */
	sNext = DtStrtok (sBackDirs, ":");

	/* process each directory individually */
	while (sNext != NULL)
	{
	    sprintf(postDtEnvironmentString,
		    "%s%s%s%s%s%s",  
		    sNext, "/%B.bm:",
		    sNext, "/%B.pm:",
		    sNext, "/%B");

	    _postDtEnvironment.bmPath = 
		_DtAddToPath (_postDtEnvironment.bmPath,
				postDtEnvironmentString);

	    /* get next directory */
	    sNext = DtStrtok (NULL, ":");
	}

	XtFree (sBackDirs);
    }
    else
    {
	/* 
	 * Save the old path if not previously saved
	 */
	if (!_preDtEnvironment.bmPath)
	{
	    sprintf(preDtEnvironmentString,"%s%s",
			BM_PATH_ENVIRON,
			"=");

	    _preDtEnvironment.bmPath = (char *)
		    (XtMalloc(strlen(preDtEnvironmentString) + 
			      strlen(sOldEnv) + 1));

	    sprintf(_preDtEnvironment.bmPath,"%s%s",
			preDtEnvironmentString,
			sOldEnv);
	}

	/* make a copy that we can modify locally */
	sBackDirs = XtMalloc (strlen(backdropPath)+1);
	strcpy (sBackDirs, backdropPath);

	/*
	 * Initialize path environment string
	 */
	sprintf(postDtEnvironmentString, 
		    "%s%s%s",  
		    BM_PATH_ENVIRON, "=",
		    "%s");

	_postDtEnvironment.bmPath = 
	    XtMalloc (1+strlen(sOldEnv)+strlen(postDtEnvironmentString));
	sprintf (_postDtEnvironment.bmPath, postDtEnvironmentString, sOldEnv);

	/* get first directory */
	sNext = DtStrtok (sBackDirs, ":");

	/* process each directory individually */
	while (sNext != NULL)
	{
	    sprintf(postDtEnvironmentString, 
		    "%s%s%s%s%s%s",  
		    sNext, "/%B.bm:",
		    sNext, "/%B.pm:",
		    sNext, "/%B");

	    _postDtEnvironment.bmPath = 
		_DtAddToPath (_postDtEnvironment.bmPath,
				postDtEnvironmentString);

	    /* get next directory */
	    sNext = DtStrtok (NULL, ":");
	}

	XtFree (sBackDirs);
    }

    putenv(strdup(_postDtEnvironment.bmPath));
    envBitVector |= BV_BMPATH;

    returnValue = DT_ENV_SET;
    return (returnValue);

} /* END OF FUNCTION _DtWsmSetBackdropSearchPath */

/*********************         eof   *************************/
