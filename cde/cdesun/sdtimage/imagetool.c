#ifndef lint
static char sccsid[] = "@(#)imagetool.c 1.37 97/04/02";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*
 * imagetool.c - Main program for ImageTool
 */

#include <netdb.h>
#include <euc.h>
#include <locale.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <Dt/Dts.h>

#include "display.h"
#include "image.h"
#include "imagetool.h"
#include "session.h"
#include "tt.h"
#include "ui_imagetool.h"
#include <Dt/EnvControl.h>
#include "help.h"
#include "sdtimage_snap.h"
#include "snap_dialog.h"

#ifdef HAVE_EDITRES
#include <X11/Xmu/Editres.h>
#endif


#if defined(sun) && (_XOPEN_VERSION == 3)
#undef NL_CAT_LOCALE
#define NL_CAT_LOCALE 0
#endif

#define MAX_BUF_LENGTH   2048
#define offset(field)     XtOffsetOf(AppResourceInfo, field)

static XrmOptionDescRec options[] = {
        { "-session", "__session", XrmoptionSepArg, NULL }
};

static XtResource app_resources[] = {
    {XmNname, "Name", XmRString, sizeof (String),
      offset (name), XmRString, "sdtimage"},
    {XmNtitle, XmCTitle, XmRString, sizeof (String),
      offset (title), XmRString, (caddr_t) NULL},
    {XmNgeometry, XmCGeometry, XmRString, sizeof (String),
      offset (geometry), XmRString, (caddr_t) NULL},
    {XmNiconName, XmCIconName, XmRString, sizeof (String),
      offset (icon_name), XmRString, (caddr_t) NULL},
    {XmNvisual, XmCVisual, XmRString, sizeof (String),
      offset (visual), XmRString, (caddr_t) NULL},
/* Session file from -session option */
    {"__session", "__Session", XtRString, sizeof(XmString),
      offset (session_file), XtRString, NULL},
};

#undef offset

/*
 * Global object definitions.
 */
BaseObjects	        *base = NULL;
AppResourceInfo         *appresources = NULL;
Widget                   open_dialog = NULL;
OpenAsObjects           *openas = NULL;
Widget                   save_dialog = NULL;
SaveAsObjects           *saveas = NULL;
PrintObjects            *print = NULL;
PrintPrevObjects        *print_prev = NULL;
PgControlsObjects       *pg_controls = NULL;
PgOverviewObjects       *pg_overview = NULL;
PaletteObjects          *palette = NULL;
InfoObjects             *info = NULL;
PropertiesObjects       *properties = NULL;
CursorObjects      	*cursor = NULL;
ProgInfo		*prog = NULL;
XilProcs                *xil = NULL;

String fallbacks[ ] = {
/*
 * Mnemonics for MenuItems
 */
"Sdtimage*MenuBar.FileButton.mnemonic:                    F",
"Sdtimage*MenuBar.EditButton.mnemonic:                    E",
"Sdtimage*MenuBar.ViewButton.mnemonic:                    V",
"Sdtimage*MenuBar.HelpButton.mnemonic:                    H",
"Sdtimage*FileMenu.OpenItem.mnemonic:                     O",
"Sdtimage*FileMenu.OpenAsItem.mnemonic:                   p",
"Sdtimage*FileMenu.SaveItem.mnemonic:                     S",
"Sdtimage*FileMenu.SaveAsItem.mnemonic:                   A",
"Sdtimage*FileMenu.SaveSelItem.mnemonic:                  v",
"Sdtimage*FileMenu.SavePageItem.mnemonic:                 I",
"Sdtimage*FileMenu.PrintOneItem.mnemonic:                 r",
"Sdtimage*FileMenu.PrintPrevItem.mnemonic:                n",
"Sdtimage*FileMenu.PrintItem.mnemonic:                    P",
"Sdtimage*FileMenu.OptionsItem.mnemonic:                  t",
"Sdtimage*FileMenu.ExitItem.mnemonic:                     x",
"Sdtimage*EditMenu.UndoItem.mnemonic:                     U",
"Sdtimage*EditMenu.PaletteItem.mnemonic:                  P",
"Sdtimage*ViewMenu.ImageInfoItem.mnemonic:                I",
"Sdtimage*ViewMenu.OverviewItem.mnemonic:                 P",
"Sdtimage*ViewMenu.ControlsItem.mnemonic:                 V",
"Sdtimage*HelpMenu.OverviewItem.mnemonic:                 O",
"Sdtimage*HelpMenu.TasksItem.mnemonic:                    T",
"Sdtimage*HelpMenu.ReferenceItem.mnemonic:                R",
"Sdtimage*HelpMenu.OnItem.mnemonic:                       I",
"Sdtimage*HelpMenu.UsingHelpItem.mnemonic:                U",
"Sdtimage*HelpMenu.AboutItem.mnemonic:                    A",
"Sdtimage*OpenItem.accelerator:                  Ctrl<Key>O",
"Sdtimage*SaveItem.accelerator:                  Ctrl<Key>S",
"Sdtimage*SaveAsItem.accelerator:                Ctrl<Key>A",
"Sdtimage*PrintItem.accelerator:                 Ctrl<Key>P",
"Sdtimage*ExitItem.accelerator:                  Alt<Key>F4",
"Sdtimage*OptionsItem.accelerator:               Ctrl<Key>I",
"Sdtimage*OpenItem.acceleratorText:                  Ctrl+O",
"Sdtimage*SaveItem.acceleratorText:                  Ctrl+S",
"Sdtimage*SaveAsItem.acceleratorText:                Ctrl+A",
"Sdtimage*PrintItem.acceleratorText:                 Ctrl+P",
"Sdtimage*ExitItem.acceleratorText:                  Alt+F4",
"Sdtimage*OptionsItem.acceleratorText:               Ctrl+I",
NULL
};

void
checknextarg (next_arg, cmd_option)
char 	*next_arg;
char 	*cmd_option;
{
    if (next_arg != (char *) NULL)
       if (next_arg[0] != '-')
	  return;

    fprintf (stderr, catgets (prog->catd, 1, 96,
			      "%s: Need value for command line option %s\n"),
			      prog->name, cmd_option);
    exit (1);

}

/*
 * Parse the command line args before being
 * passed to xv_init().
 */
void
check_for_args (argc, argv)
    int		argc;
    char       *argv [];
{
  int		i;

  for (i = 0 ; i < argc; i++) {
    /*
     * Check if we were started by tooltalk
     */
    if (strcmp (argv[i], "-tooltalk") == 0) {
      prog->tt_started = TRUE;
    }

    if(strcmp (argv[i], "-snapshot") == 0) {
      prog->snapshot_on_start = TRUE;
    }

  }
}
#ifdef NEVER
/* This routine extracts the session argment from the argc, argv
   list.  It's in a separate routine as the argument doesn't conform
   to getopt conventions, and may change.  This really should be
   parsed out by a toolkit routine. */

static void
parse_sessionarg(int *argc,
		 char**argv)

{
     int i, j;
     char *sessionfile;

     for (i = 0; i < *argc; i++)
       {
	 if (!strcmp(argv[i], "-session"))
	   {
	     /* make sure there is another parameter to remove */

	     if (i < (*argc - 1))
	       {
		 sessionfile = strdup(argv[i + 1]);
		 for (j = i + 2; j < *argc; j++)
		   argv[j - 2] = argv[j];
		 *argc -= 2;
		 argv[*argc] = NULL;

		 GetSessionInfo (sessionfile);

	       }
	     else
	       printf(catgets (prog->catd, 1, 97,
			      "The -session argument requires a parameter\n"));

	   }
       }
}
#endif

void
usage ()
{
    fprintf (stderr, catgets (prog->catd, 1, 394, "Usage: %s %s"), "sdtimage",
	"[-usage] [-v] [-verbose] [-snapshot] [-timeout] [imagefile]\n");
}

char *
parseargs(argc, argv)
    int         argc;
    char       *argv[];
{
    int          i, nread;
    char        *s;
    unsigned char *ptr;
    char	*pathname = NULL;
    FILE 	*fp;

    for (i = 1; i < argc; i++) {
	s = argv[i];
	if (s[0] == '-') {
	   int n = strlen (s);
	   if (!strncmp ("-", s, n))
	      prog->standardin++;
	   else if (!strcmp ("-verbose", s)) {
	      prog->verbose++;
	      }
	   else if (!strcmp ("-usage", s)) {
	      usage ();
	      exit (0);
	      }
	   else if (!strcmp ("-tooltalk", s)) {
	      prog->tt_started = TRUE;
	      }
	   else if (!strcmp ("-snapshot", s)) {
	      prog->snapshot_on_start = TRUE;
	      }
	   else if (!strcmp ("-timeout", s)) {
	      prog->timeout = atoi(argv[i++]);
	      }
	   else if (!strcmp ("-v", s)) {
	      fprintf (stderr, catgets (prog->catd, 1, 98,
					"%s version %s running on %s \n"),
					prog->name, prog->rel, XmVERSION_STRING);
	      exit (0);
	      }
	   else {
	      fprintf (stderr, catgets (prog->catd, 1, 99,
					"%s: Illegal switch `%s'\n"),
		       prog->name, s);
	      usage ();
	      exit (1);
	      }
	   }
	else {
	   if (prog->standardin)
	     fprintf (stderr, catgets (prog->catd, 1, 435,
				"%s: Ignoring `%s'\n"), prog->name, s);
	   else {
	     pathname = malloc (strlen (s) + 1);
	     strcpy (pathname, s);
	   }
	}
     }

    if (prog->standardin) {
      prog->standardinfile = (char *) mktemp ("/tmp/image.XXXXXX");

      if ((fp = fopen (prog->standardinfile, "w")) == NULL) {
        fprintf (stderr, catgets (prog->catd, 1, 436,
			 "%s: Can't create temp file `%s'\n"),
                 	 prog->name, prog->standardinfile);
        exit (1);
      }
      ptr = (unsigned char *)malloc (MAX_BUF_LENGTH * sizeof (unsigned char *));
      while (!feof(stdin)) {
	nread = fread (ptr, sizeof (char), MAX_BUF_LENGTH, stdin);
        fwrite (ptr, sizeof (char), nread, fp);
      }
      free (ptr);

      fclose(fp);
      fp = fopen (prog->standardinfile, "r");
      pathname = malloc (strlen (prog->standardinfile) + 1);
      strcpy (pathname, prog->standardinfile);
      return (pathname);
    }
    else if (pathname != NULL) {
       if (pathname[0] != '/') {
	  char *tmp_file = malloc (strlen (pathname) + 1);
	  strcpy (tmp_file, pathname);
	  pathname = malloc (strlen (tmp_file) + strlen (prog->directory) + 2);
	  sprintf (pathname,"%s/%s", prog->directory, tmp_file);
	  free (tmp_file);
	  }
       else {
         char   *tmp_dir = strip_filename (pathname);
	 struct  stat file_info;

	 if (stat (tmp_dir, &file_info) == 0)
	   if (S_ISDIR (file_info.st_mode))
   	     prog->directory = tmp_dir;
       }
       return (pathname);
       }

    return ((char *) NULL);

}

void
complete_init_prog ()
{
    char  *appname;
    char  *s;
    int    mb_locale = multibyte; /* from euc.h */

/*
 * Open the i18n message catalog file
 */
    prog->catd = catopen (IMAGETOOL_CAT, NL_CAT_LOCALE);
    appname = catgets (prog->catd, 1, 100, "Image Viewer");
    prog->name = malloc (strlen (appname) + 1);

    strcpy (prog->name, appname);

    prog->directory = getcwd (NULL, MAXPATHLEN);
    if (prog->directory == NULL ) {
       fprintf (stderr, catgets (prog->catd, 1, 101,
				 "%s: Can't get current directory!\n"),
		prog->name);
       exit(1);
       }
/*
 * Default timeout set in pre_init_prog.
 * Force timeout longer if multi-byte locale.
 * Needed to do this after XtSetLanguageProc
 */
    if (mb_locale)
      prog->timeout = 180;

#ifdef LATER
    prog->sb_right = TRUE;
    s = (char *) defaults_get_string (DGET ("openwindows.scrollbarplacement"),
				      DGET ("OpenWindows.ScrollbarPlacement"),
				      NULL);
    if (s) {
      if (strcasecmp (s, "left") == 0)
	prog->sb_right = FALSE;
    }
#endif

}

ProgInfo *
pre_init_prog ()
{
    ProgInfo	*tmp;
    char	*tmpdir;
    char	 tmp_filename [MAXPATHLEN];

    tmp = (ProgInfo *) calloc (1, sizeof (ProgInfo));

    tmp->rel = malloc (strlen (IMAGETOOL_VERSION) + 1);
    strcpy (tmp->rel, IMAGETOOL_VERSION);

    tmp->hostname = malloc (MAXHOSTNAMELEN);
    gethostname(tmp->hostname, MAXHOSTNAMELEN);

    tmp->uid = geteuid();
    tmp->gid = getegid();

    tmp->news_opened = FALSE;
    tmp->xil_opened = FALSE;
    tmp->frame_mapped = FALSE;
    tmp->ce_okay = -1;   /* CE not initalized yet */
    tmp->def_ps_zoom = 100;
    tmp->standardin = False;

/*
 * Make temporary file names.
 */

    tmpdir = getenv ("TMPDIR");
    if (tmpdir == (char *) NULL)
       tmpdir = "/tmp";

    sprintf (tmp_filename, "%s/sdtimage%d", tmpdir, getpid ());
    tmp->file_template = (char *) malloc (strlen (tmp_filename) + 1);
    strcpy (tmp->file_template, tmp_filename);

    tmp->def_printer = NULL;

    /* default to timeout after 60 seconds */
    tmp->timeout = 60;

    tmp->verbose = 0;
    tmp->tt_started = False;
    tmp->snapshot_on_start = False;
    tmp->check_for_color = True;
    tmp->footer_priority = 0;

    return (tmp);
}

void
make_newsserver ()
{
    char		*xdisplay;
    char		*host;
    char		 tmp_server [256];
    unsigned int 	 port;
    struct hostent	*hp;


    xdisplay = DisplayString (ps_display->xdisplay);
    if (xdisplay[0] == ':') {
       host = malloc (strlen (prog->hostname) + 1);
       strcpy (host, prog->hostname);
       sscanf (xdisplay, ":%u", &port);
       }
    else {
       host = malloc (strlen (xdisplay) + 1);
       sscanf (xdisplay, "%[^:]:%u", host, &port);
       if ((strcmp (host, "unix") == 0) ||
           (strcmp (host, "localhost") == 0)) {
	   free (host);
           host = malloc (strlen (prog->hostname) + 1);
           strcpy (host, prog->hostname);
	   }
       }

    port += 2000;
    hp = gethostbyname (host);

/*
 * Check if we got the info..
 */

    if (hp == (struct hostent *) NULL) {
       char *x;
       char  addr[4];
       int   i;

       x = strtok (host, ".");
       addr [0] = (char) atoi (x);
       for (i = 1; i < 4; i++) {
           x = strtok (NULL, ".");
           addr [i] = (char) atoi (x);
           }

       hp = gethostbyaddr (addr, 4, AF_INET);
       if (hp == (struct hostent *) NULL)  {
          fprintf(stderr, catgets (prog->catd, 1, 102,
			      "%s: Can't get host information.. aborting.\n"),
		  prog->name);
          exit(1);
          }

       free (host);
       host = malloc (strlen (hp->h_name) + 1);
       strcpy (host, hp->h_name);
       }

    sprintf(tmp_server, "%lu.%u;%s\n",
            ntohl(*(u_long *) hp->h_addr), port, host);

    prog->newsserver = malloc (strlen (tmp_server) + 1);
    strcpy (prog->newsserver, tmp_server);

/*
 * We can now also determine if we were started remotely.
 */

    if (strcmp (host, prog->hostname) != 0)
       prog->remote = TRUE;

    endhostent();
    free (host);
}

void
ShutdownImagetool()
{
/*
    if (prog->xil_opened)
      xil_close (image_display->state);
*/
/*     if (base->app_context != NULL) { */
/*       XtDestroyApplicationContext (base->app_context); */
/*       base->app_context = NULL; */
/*     } */
/*     XtCloseDisplay (image_display->xdisplay); */

    close_ps (TRUE, TRUE);
    catclose (prog->catd);

    if (prog->datafile != (char *) NULL)
       unlink (prog->datafile);
    if (prog->compfile != (char *) NULL)
       unlink (prog->compfile);
    if (prog->uncompfile != (char *) NULL)
       unlink (prog->uncompfile);
    if (prog->printfile != (char *) NULL)
       unlink (prog->printfile);
    if (prog->rashfile != (char *) NULL)
       unlink (prog->rashfile);

}

main(argc, argv)
int	argc;
char	**argv;
{
    char		*input_file;
    extern int	  	 xerror_handler ();
    XEvent               event;
    Boolean              status;
    register char       *s_ptr, *d_ptr;
    register int         i;

/*
 *  Initialize program info..
 */
    prog = pre_init_prog ();

/*
 * Init ps variables
 */
    init_ps_vars ();

/*
 * Need to check and see if we were started with several different
 * command line options before we do the Xview initialization (such
 * as -tooltalk, -depth, -geometry, etc.).
 */
    check_for_args (argc, argv);

/*
 * Save the argv list into a WM_COMMAND format string for
 * later use when the application is asked to save itself.
 * The format is a buffer with a series of null terminated
 * strings within it, one for each string in the argv list.
 * Thus the total length is that of the arguments plus a null
 * byte for each argument.
 */
    for (i = 0, prog->wm_cmdstrlen = 0; i < argc; i++) {
      /* Do not record the -session option as we add that
       * automatically when we save the session.
       */
      if (!strcmp("-session", argv[i]) || !strcmp ("-tooltalk", argv[i])) {
        i++;
        continue;
      }
      prog->wm_cmdstrlen += strlen(argv[i]) + 1;
    }

    d_ptr = prog->wm_cmdstr = (char *) malloc(prog->wm_cmdstrlen);

    for (i = 0; i < argc; i++) {
      char *s_ptr = argv[i];

      /* Do not record the -session option as we add that
       * automatically when we save the session.
       */
      if (!strcmp("-session", argv[i]) || !strcmp ("-tooltalk", argv[i])) {
        i++;
        continue;
      }

      while (*s_ptr)
        *d_ptr++ = *s_ptr++;
      *d_ptr++ = NULL;
    }
    d_ptr = NULL;

/*
 * Initialize Motif
 */
    XtSetLanguageProc (NULL, NULL, NULL);
    base = (BaseObjects *) calloc (1, sizeof (BaseObjects));

    _DtEnvControl(DT_ENV_SET); /* set up environment variables */
    base->top_level = XtVaAppInitialize (&base->app_context, "Sdtimage",
					 options, XtNumber (options),
					 &argc, argv, fallbacks,
					 XmNwidth, 500, XmNheight, 600,
					 XmNwidthInc, 1, XmNheightInc, 1,
					 XmNdeleteResponse, XmDO_NOTHING,
					 NULL);

    XSetErrorHandler (xerror_handler);
    appresources = (AppResourceInfo *) calloc (1, sizeof (AppResourceInfo));
    XtGetApplicationResources (base->top_level, (XtPointer) appresources,
			       app_resources,
			       XtNumber (app_resources), NULL, 0);

#ifdef HAVE_EDITRES
        XtAddEventHandler(base->top_level,
                          0,
                          True,
                          (XtEventHandler) _XEditResCheckMessages,
                          (XtPointer) NULL);
#endif


/* nothing relevant in sessionfile right now since -geometry is used .
    if (appresources->session_file)
      GetSessionInfo (appresources->session_file);
*/
/*
 * Initialize Deskttop Services library
 */
    status = DtAppInitialize (base->app_context, XtDisplay (base->top_level),
			      base->top_level, "sdtimage", "Sdtimage");
    if (status == False) {
      fprintf (stderr, catgets (prog->catd, 1, 103,
               "Sdtimage: Could not initialize Desktop Services Library.\n"));
    }
/*
 * Load actions and data types database
 */
    DtDbLoad();
    prog->ce_okay = TRUE;

/*
 * Finish initialization with prog/gettext.
 */
    complete_init_prog ();

/*
 * Create base frame and load pop up only...
 */
    base = BaseObjectsInitialize (base, base->top_level);

    input_file = parseargs (argc, argv);

/*
 * Initialize the resoure database
 * before we create the canvas.
 */
    read_props (True);

    set_labels (catgets (prog->catd, 1, 104, "(None)"));

/*
 * Set an Exit function to make sure we clean up.
 */
    (void) atexit (ShutdownImagetool);

/*
 * This needs to be here in order to create the xil_window() from
 * the canvas and also to XCreateGC() and XAllocColor() to work
 * called from BaseImageCanvasObjectsCreate.
 */
    XtSetMappedWhenManaged (base->top_level, False);
    XtRealizeWidget (base->top_level);

/*
 * Initialize ToolTalk only if started with -tooltalk command line option.
 */
    if (prog->tt_started == TRUE) {
      status = TTInitialize (XtDisplay (base->top_level));
      if (status == False) {
	fprintf (stderr, catgets (prog->catd, 1, 105,
			 "%s: Could not initialize ToolTalk.\n"), prog->name);
	exit (1);
      }
    }
    else if (input_file != (char *) NULL) {
      prog->frame_mapped = TRUE;
      if (OpenAtStart (input_file, -1) < 0) {
	if (ps_display->new_canvas == NULL)
	  current_display = image_display;
      }
    }
    else
      current_display = image_display;

    if (!XtIsRealized (base->top_level))
      XtRealizeWidget (base->top_level);

    if (prog->tt_started == False)
      XtMapWidget (base->top_level);

/*
 * Wait until now to initialize the snap dlog
 */
    dtb_sdtimage_snap_initialize(&argc, &argv, base->top_level);

    if (prog->snapshot_on_start == TRUE)
      {
        setbusy();
        DtbCreateSnapDialog(&dtb_snap_dlog_dialog, base->top_level);
        setactive();
      }
    /*
     * This is the equivalent of XtAppMainLoop()
     */

    while (True)
      {
        XtAppNextEvent (base->app_context, &event);
        if (!XDPSDispatchEvent (&event))
          (void) XtDispatchEvent (&event);
      }
}


