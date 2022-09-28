/* $XConsortium: Main.c /main/cde1_maint/2 1995/10/09 00:00:37 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Main.c
 **
 **   Project:     Cache Creek (Rivers) Project
 **
 **   Description: This is the main.c file for the helpview program.
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/




/* System Include Files  */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <limits.h>
#include <unistd.h>  /* R_OK */
#ifdef __osf__
/* Suppress unaligned access message */
#include <sys/types.h>
#include <sys/sysinfo.h>
#endif /* __osf__ */

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>

#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>

#include <Dt/HelpDialog.h>


#include <Dt/DtNlUtils.h>
#include <Dt/EnvControl.h>

/* Local Includes */

#include <DtI/HelposI.h>
#include <DtI/HelpP.h>
#include <DtI/FileUtilsI.h>
#include "Main.h"
#include "UtilI.h"
#include "ManPageI.h"

/*  Application resource list definition  */

static XrmOptionDescRec option_list[] =
{
   {  "-helpVolume",     "helpVolume",     XrmoptionSepArg,  NULL  },
   {  "-locationId",     "locationId",     XrmoptionSepArg,  NULL  },
   {  "-file",           "file",           XrmoptionSepArg,  NULL  },
   {  "-manPage",        "manPage",        XrmoptionSepArg,  NULL  },
   {  "-man",            "man",            XrmoptionNoArg,  "True" },

};


/*  Structure, resource definitions, for View's optional parameters.  */


typedef struct
{
   char * helpVolume;
   char * locationId;
   char * file;
   char * manPage;
   char * man;

} ApplicationArgs, *ApplicationArgsPtr;

static ApplicationArgs application_args;

static XtResource resources[] =
{

   {
     "helpVolume", "HelpVolume", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, helpVolume),XmRImmediate,(caddr_t) NULL,
   },

   {
     "locationId", "LocationId", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, locationId), XmRImmediate, (caddr_t) NULL,
   },

   {
     "file", "File", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, file), XmRImmediate, (caddr_t) NULL,
   },
   {
     "manPage", "ManPage", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, manPage), XmRImmediate, (caddr_t) NULL,
   },
   {
     "man", "Man", XmRString, sizeof (char *),
     XtOffset (ApplicationArgsPtr, man), XmRImmediate, (caddr_t) NULL,
   },

};






/********    Static Function Declarations    ********/

#ifdef _NO_PROTO
static void Usage();
static int ExpandVolume();
#else
static void Usage(
    char ** argv);
static int ExpandVolume(
    char    **helpVolume);

#endif /* _NO_PROTO */



/* Global Variables */
#define MAX_ARGS 20
#define charset       XmFONTLIST_DEFAULT_TAG






/****************************************************************************
 * Function:	    static void GlobalInit();
 *
 * Parameters:
 *
 * Return Value:    Void.
 *
 * Purpose: 	    Initializes our global variables to valid starting values.
 *
 ****************************************************************************/
#ifdef _NO_PROTO
static void GlobalInit()
#else
static void GlobalInit()
#endif /* _NO_PROTO */

{

  pCacheListHead   = NULL;
  pCacheListTale   = NULL;
  totalCacheNodes  = NULL;

  helpClass = XtNewString("Dthelpview");

  viewWidget = NULL;
  manWidget  = NULL;
  manBtn = NULL;
  manText = NULL;
  manForm = NULL;
  closeBtn = NULL;

}

/***************************************************************************
 * Main Line Program:
 *
 ***************************************************************************/
#ifdef _NO_PROTO
void main(argc, argv)
    int argc;
    char **argv;
#else
void main(
    int argc,
    char **argv)
#endif
{
  char		  *appName;
  Arg	 	  args[2];
  int             n;
  int             newArgc=0;
  char            **newArgv;
  int             counter=0;
  char           *hvx  = NULL;
  char           *sdlx = NULL;
  char           *orig_hv = NULL;
  size_t          len = 0;

#ifdef __osf__
/* Code to suppress unaligned access message. */
   unsigned long        op;
   int                  buffer[2];
   unsigned long        nbytes = 1;
   char                *arg = 0;

   unsigned long        flag = 0;

   int                  ssi_status;

   op = SSI_NVPAIRS;

   buffer[0] = SSIN_UACPROC;
   buffer[1] =  0x00000001;
#ifdef DEBUG_UAC
   buffer[1] |= 0x00000004;
#endif

   ssi_status = setsysinfo ( op, (caddr_t) buffer, nbytes, arg, flag );
#endif

  int expandable = 0;	/* Status if help volume file passed in is expandable */

  XtSetLanguageProc(NULL, NULL, NULL);

  appName = strrchr(argv[0], '/');
  if (appName != NULL)
    appName++;
  else
    appName = argv[0];


  /* Copy our argv values into a new array for use in DisplayTopic... */
   newArgc = argc;
   newArgv = (char **) XtMalloc (sizeof(char *) * (argc +1));

   for (counter=0;counter < argc; counter++)
     {
       newArgv[counter] = (char *) XtMalloc (strlen(argv[counter]) +1);
       (void) strcpy (newArgv[counter], argv[counter]);
      }

  /* Setup our Help message catalog file name */
  DtHelpSetCatalogName("DtHelp.cat");

  _DtEnvControl(DT_ENV_SET);
   {
     /*  char * foo = ((char *)GETMESSAGE(7, 1, ""));  ??? */
   }


  /* Initialize toolkit and open the display */
  topLevel = XtInitialize(appName, "Dthelpview", option_list,
                          XtNumber(option_list), &argc, argv);
  appDisplay = XtDisplay(topLevel);

  if (!appDisplay)
    {
      XtWarning ("Dialogs: Can't open display, exiting...");
      exit (0);
    }

  /* Get the application resources. */
  XtGetApplicationResources(topLevel, &application_args,
                            resources, XtNumber(resources), NULL, 0);

  /*  If all of the command line parameters were not processed  */
  /*  out, print out a usage message set and exit.              */

  if (argc != 1)
    Usage (argv);

  /* Call our global init routine */
  GlobalInit();


  /* Setup or environment to handle multi-byte stuff */
#ifdef NLS16
  Dt_nlInit();
#endif


  /* Give our shell a default size greater than zero */
  n = 0;
  XtSetArg (args[n], XmNwidth, 10);           n++;
  XtSetArg (args[n], XmNheight, 10);          n++;
  XtSetValues(topLevel, args, n);

  /* Setup up our top level shell */
   /* XtSetMappedWhenManaged(topLevel, FALSE);
    * XtRealizeWidget(topLevel);
    */


  if (application_args.file != NULL)
      DisplayFile(topLevel, application_args.file);
  else if (application_args.manPage != NULL)
      DisplayMan(topLevel, application_args.manPage, EXIT_ON_CLOSE);
  else if (application_args.man != NULL)
      PostManDialog(topLevel, newArgc, newArgv);
  else if (application_args.helpVolume != NULL)
    {
      orig_hv = (char*) malloc(strlen(application_args.helpVolume) + 1);
      strcpy(orig_hv, application_args.helpVolume);

      /* ---------------------------------------------------------------------- */
      /*  Append an .sdl suffix to the end of the helpVolume if no .hv or .sdl  */
      /*  suffixes specified. Allow for the help volume to contain the suffixes */
      /*  anywhere else in the middle.                                          */
      /*  Note that the memory allocated is reclaimed when the program exits. ??*/
      /* ---------------------------------------------------------------------- */

      hvx  = strstr(application_args.helpVolume, DtHelpCCDF_VOL_SUFFIX);
      sdlx = strstr(application_args.helpVolume, DtHelpSDL_VOL_SUFFIX);

      if ((sdlx != NULL && strcmp(sdlx, DtHelpSDL_VOL_SUFFIX) == 0) ||
	  (hvx != NULL && strcmp(hvx, DtHelpCCDF_VOL_SUFFIX) == 0)   )
	{
	  /* exact suffix match at end, leave it as is. */
	  sdlx = application_args.helpVolume;
	}
      else
	{
	  len = strlen(application_args.helpVolume);
	  sdlx = (char*) malloc(len + 4 + 1); /* for .sdl and null char */
	  strcpy(sdlx, application_args.helpVolume);
	  sdlx = strcat(sdlx, DtHelpSDL_VOL_SUFFIX);
	}

      application_args.helpVolume = sdlx;

      /* See if you can expand the helpVolume value */
      expandable = ExpandVolume(&(application_args.helpVolume));

      /* ----------------------------------------------- */
      /*  Allow for the old convention continue to work  */
      /* ----------------------------------------------- */

      /* If the file is not expandable to a full path, then simply pass
      ** the argument value because this may be a registered helpvolume.
      */
      if (!expandable )	/* use original argument value */
      {
	application_args.helpVolume = orig_hv;
      }

      /* This will invoke DtCreateHelpDialog eventually. The help
      ** volume will be displayed if the value of 
      ** application_args.helpvolume is either an complete file to
      ** the help file or a registered help volume.
      */
      DisplayTopic(topLevel, application_args.helpVolume,
		   application_args.locationId, newArgc, newArgv);
    }
  else
    Usage (argv);

  XtMainLoop();
}



/************************************************************************
 *
 *  Usage
 *	When incorrect parameters have been specified on the command
 *	line, print out a set of messages detailing the correct use
 *	and exit.
 *
 ************************************************************************/
#ifdef _NO_PROTO
static void Usage (argv)
char ** argv;
#else
static void Usage(
    char ** argv)
#endif /* _NO_PROTO */
{
   (void) fprintf (stderr, ((char *)_DTGETMESSAGE(7, 1,
                            "Usage: %s...\n")), argv[0]);
   (void) fprintf (stderr, "\n");
   (void) fprintf (stderr, ((char *)_DTGETMESSAGE(7, 2,
                            "\t-helpVolume <Help Volume File>\n")), argv[0]);
   (void) fprintf (stderr, ((char *) _DTGETMESSAGE(7, 3,
                     "\t-locationId  <ID>\n")), argv[0]);
   (void) fprintf (stderr, ((char *) _DTGETMESSAGE(7, 5,
                     "\t-file  <ASCII Text File>\n")), argv[0]);
   (void) fprintf (stderr, ((char *) _DTGETMESSAGE(7, 6,
                     "\t-man\n")), argv[0]);
   (void) fprintf (stderr, ((char *) _DTGETMESSAGE(7, 7,
                     "\t-manPage  <Unix Man Page>\n\n\n")), argv[0]);
   exit (0);
}



/****************************************************************************
 * Function:         ExpandVolume()
 *
 * Parameters:
 *
 * Return Value:    return 1 if the expanded helpvolume is present else 0.
 *
 * Purpose: 	    Takes a helpVolume file name and adds the cwd path to the
 *                  front of it, and possibly adds a suffix.
 *                  It then stats the file; if the file is present, the
 *                  function returns a malloc'd full path in helpVolume,
 *                  otherwise it returns a malloc'd copy of the original value.
 *
 ***************************************************************************/
#ifdef _NO_PROTO
static int ExpandVolume(helpVolume)
    char    **helpVolume;
#else
static int ExpandVolume(
    char    **helpVolume)
#endif /* _NO_PROTO */
{

  char * workingPath=NULL;

   /* try to locate file and its entry, if present */
   /* True: search relative to current directory as well */
   workingPath = _DtHelpFileLocate("volumes", *helpVolume,
                                  _DtHelpFileSuffixList,True,R_OK);
   if (workingPath != NULL)
   {
     *helpVolume = workingPath;
     return(1);
   }
   else
   {
     *helpVolume = strdup(*helpVolume);
     return(0);
   }
}
