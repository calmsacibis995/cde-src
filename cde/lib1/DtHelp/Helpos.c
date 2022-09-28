/* $XConsortium: Helpos.c /main/cde1_maint/2 1995/10/08 17:20:10 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Dtos.c
 **
 **   Project:     Rivers Project
 **
 **   Description: All system/os dependent calls are placed in here. 
 ** 
 **
 ** WARNING:  Do NOT put any Xt or Xm dependencies in this code.
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 ****************************************************************************
 ************************************<+>*************************************/


#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#ifndef _SUN_OS /* don't need the nl_types.h file */
#include <nl_types.h>
#endif /* ! _SUN_OS */

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>

/* Dt Includes */
#include <Dt/Help.h>
/* #include <Dt/DtNlUtils.h> */

#include "HelpP.h"
#include "HelpI.h"

#ifndef NL_CAT_LOCALE
static const int NL_CAT_LOCALE = 0;
#endif

#ifdef _NO_PROTO
#else
#endif /* _NO_PROTO */


/* Global Message Catalog file names */
static char *CatFileName=NULL;



/*****************************************************************************
 * Function:	   Boolean _DtHelpOSGetHomeDirName(
 *
 *
 * Parameters:     
 *
 * Return Value:    String.
 *
 *
 * Description: 
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void _DtHelpOSGetHomeDirName(outptr) 
    String outptr;
#else
void _DtHelpOSGetHomeDirName(
    String outptr)
#endif

/* outptr is allocated outside this function, just filled here. */
/* this solution leads to less change in the current (mwm) code */
{
    int uid;
    struct passwd *pw;
    static char *ptr = NULL;

    if (ptr == NULL) {
	if((ptr = (char *)getenv("HOME")) == NULL) {
	    if((ptr = (char *)getenv("USER")) != NULL) 
		pw = getpwnam(ptr);
	    else {
		uid = getuid();
		pw = getpwuid(uid);
	    }
	    if (pw) 
		ptr = pw->pw_dir;
	    else 
		ptr = NULL;
	}
    }

    if (ptr) strncpy(outptr, ptr, MAXPATHLEN - 1);
    else outptr[0] = '\0' ;
}






/*****************************************************************************
 * Function:	   _DtHelpGetUserSearchPath(
 *
 *
 * Parameters:     
 *
 * Return Value:    String, owned by caller.
 *
 *
 * Description: 
 *                  Gets the user search path for use
 *                  when searching for a volume.
 *                  Takes path from the environment, 
 *                  or uses the default path. 
 *
 *****************************************************************************/
#ifdef _NO_PROTO
String _DtHelpGetUserSearchPath()
#else
String _DtHelpGetUserSearchPath(void)
#endif
{
  String path;
  char homedir[MAXPATHLEN + 2];
  String localPath;
  extern char * _DtCliSrvGetDtUserSession();  /* in libCliSrv */

   localPath = (char *)getenv (DtUSER_FILE_SEARCH_ENV);
   if (localPath  == NULL) 
   {
       char * session;

       /* Use our default path */
       _DtHelpOSGetHomeDirName(homedir);
       session = _DtCliSrvGetDtUserSession();

       path = calloc(1, 3*strlen(session) + 6*strlen(homedir) +
                        strlen(DtDEFAULT_USER_PATH_FORMAT));
       sprintf(path, DtDEFAULT_USER_PATH_FORMAT, 
                     homedir, session, homedir, session, homedir, session,
                     homedir, homedir, homedir);
       free(session);
       /* homedir is a local array */

       /* Return our expanded default path */
       return(path);
   }
   else
   {
       /* Make a local copy for us */
       localPath = strdup(localPath);

       /* Just Use our local path */
       return (localPath);
   }

}



/*****************************************************************************
 * Function:	   _DtHelpGetSystemSearchPath(
 *
 *
 * Parameters:     
 *
 * Return Value:    String, owned by caller.
 *
 *
 * Description: 
 *                  Gets the system search path for use
 *                  when searching for a volume.
 *                  Takes path from the environment, 
 *                  or uses the default path. 
 *
 *****************************************************************************/
#ifdef _NO_PROTO
String _DtHelpGetSystemSearchPath()
#else
String _DtHelpGetSystemSearchPath(void)
#endif
{
   char * path;

   /* try to retrieve env var value */
   path = getenv(DtSYS_FILE_SEARCH_ENV);

   /* if fail, use default */
   if (NULL == path)
      path = DtDEFAULT_SYSTEM_PATH;

   return strdup(path);
}




/*****************************************************************************
 * Function:	   void DtHelpSetCatalogName(char *catFile);
 *
 *
 * Parameters:     catFile   Specifies the name of the message catalog file
 *                           to use.  
 *
 * Return Value:   void
 *
 *
 * Description:    This function will set the name of the message catalog file
 *                 within the Dt help system environment. 
 *      
 *                 The new name must be of the format <name>.cat and the file
 *                 must be installed such that the NLS search path can find it.
 *           
 *                 If this function is not called then the default value of 
 *                 Dt.cat will be used.
 *
 *
 *****************************************************************************/
#ifdef _NO_PROTO
void DtHelpSetCatalogName(catFile)
        char* catFile;
#else
void DtHelpSetCatalogName(
        char* catFile)
#endif /* _NO_PROTO */
{
  int len;

  /* Setup our Message Catalog file names */
  if (catFile == NULL)
    {
      /* Setup the short and long versions */
#ifdef __ultrix
      CatFileName = strdup("DtHelp.cat");  
#else
      CatFileName = strdup("DtHelp");
#endif
    }
  else
    { 
#ifdef __ultrix

      /* If we have a full path, just use it */
      if (*catFile == '/')
        CatFileName = strdup(catFile);
      else
        {
          /* We don't have a full path, and the ultirx os needs the 
           * ".cat" extention so let's make sure its there.
           */
          if (strcmp(&catFile[strlen(catFile) -4], ".cat") == 0)
            CatFileName = strdup(catFile);
          else
            {
              /* Create our CatFileName with the extention */
              CatFileName = malloc (strlen(catFile) + 5);    
                                    /* +5, 1 for NULL, 4 for ".cat" */
              strcpy(CatFileName, catFile);
              strcat(CatFileName, ".cat");
	    }
         }

        
#else

      /* If we have a full path, just use it */
      if (*catFile == '/')
        CatFileName = strdup(catFile);
      else
        {
          /* hp-ux os does not work with the ".cat" extention, so
           * if one exists, remove it.
           */
          
           if (strcmp(&catFile[strlen(catFile) -4], ".cat") != 0)
            CatFileName = strdup(catFile);
          else
            {
              /* Create our CatFileName with out the extention */
              len = strlen(catFile) -4;
              CatFileName = malloc(len +1);
              strncpy (CatFileName, catFile, len);
              CatFileName[len]= '\0';
            }
        }
#endif
    }
}



#ifdef MESSAGE_CAT
/*****************************************************************************
 * Function:	   Boolean _DtHelpGetMessage(
 *
 *
 * Parameters:     
 *
 * Return Value:   char *
 *
 *
 * Description:    This function will retreive the requested message from the
 *                 cache proper cache creek message catalog file.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
char *_DtHelpGetMessage(set,n,s)
        int set;
        int n;
        char *s;
#else
char *_DtHelpGetMessage(
        int set,
        int n,
        char *s)
#endif /* _NO_PROTO */
{
   char *msg;
   char *lang;
   nl_catd catopen();
   char *catgets();
   static int first = 1;
   static nl_catd nlmsg_fd;

   if ( first ) 
   {

     /* Setup our default message catalog names if none have been set! */
     if (CatFileName  == NULL)
       {
         /* Setup the short and long versions */
#ifdef __ultrix
         CatFileName = strdup("DtHelp.cat"); 
#else 
         CatFileName = strdup("DtHelp");
#endif
       }
		first = 0;

		lang = (char *) getenv ("LANG");

		if (!lang || !(strcmp (lang, "C"))) 
			/*
			 * If LANG is not set or if LANG=C, then there
			 * is no need to open the message catalog - just
			 * return the built-in string "s".
			 */
			nlmsg_fd = (nl_catd) -1;
		else
			nlmsg_fd = catopen(CatFileName, NL_CAT_LOCALE);
	}
	msg=catgets(nlmsg_fd,set,n,s);
	return (msg);

}
#endif



