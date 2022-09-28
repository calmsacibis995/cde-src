/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)sdtaudio.c	1.51 97/06/04 SMI"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread.h>
#include <assert.h>
#include <netdb.h>
#include <limits.h>
#include <strings.h>
#include <X11/Intrinsic.h>
#include <X11/Core.h>
#include <nl_types.h>
#include <Xm/XmAll.h>
#include <Dt/Dt.h>
#include <Dt/Dnd.h>
#include <Dt/Dts.h>
#include <Dt/EnvControl.h>
#include <Dt/HelpDialog.h>
#include <Dt/HourGlass.h>
#include <Tt/tttk.h>

#include "sdtaudio.h"
#include "thrdefs.h"
#include "callbacks.h"
#include "cb_utils.h"
#include "utils.h"
#include "audiofile.h"
#include "file_utils.h"
#include "safile.h"
#include "mutex.h"
#include "audtt.h"

XtAppContext    appContext;
XFontStruct    *ww_font = NULL;
int             ww_font_height = 0;
nl_catd         msgCatalog;
Widget          appShell;
WindowData     *windowList = NULL;
Atom            WM_DELETE_WINDOW;
Atom            WM_SAVE_YOURSELF;
char           *appnameString = NULL;
static char    *separatorString = NULL;
static char    *untitledString = NULL;
char           *argv0 = NULL;
char           *programName = NULL;
char           *temp_buffer_name = NULL;

static XrmOptionDescRec optionTable[] = {
	{"-server", ".serverMode", XrmoptionIsArg, NULL},
};

typedef struct {
	String          serverMode;
}               appResourceRec;

static XtResource appResources[] = {
	{"serverMode", "ServerMode", XtRString, sizeof(String),
	XtOffsetOf(appResourceRec, serverMode), XtRString, NULL},
};

String  fallbacks[] = {

	/* Fonts fallbacks. */
	"Sdtaudio*progressStartLabel.fontList: -dt-interface system-*-*-*-*-*-120-*-*-*-*-*-*",
	"Sdtaudio*progressEndLabel.fontList: -dt-interface system-*-*-*-*-*-120-*-*-*-*-*-*",
	"Sdtaudio*waveStartLabel.fontList: -dt-interface system-*-*-*-*-*-120-*-*-*-*-*-*",
	"Sdtaudio*waveEndLabel.fontList: -dt-interface system-*-*-*-*-*-120-*-*-*-*-*-*",
/*
	"Sdtaudio*fileNameLabel.fontList: -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*",
	"Sdtaudio*fileSizeLabel.fontList: -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*",
	"Sdtaudio*fileLengthLabel.fontList: -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*",
	"Sdtaudio*fileChannelsLabel.fontList: -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*",
	"Sdtaudio*fileSampleRateLabel.fontList: -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*",
	"Sdtaudio*fileEncodingLabel.fontList: -dt-interface user-medium-r-normal-s*-*-*-*-*-*-*-*-*",
*/

	/* Accelerators text fallbacks. */
	"Sdtaudio*NewItem.acceleratorText: Ctrl+N",
	"Sdtaudio*OpenItem.acceleratorText: Ctrl+O",
	"Sdtaudio*SaveAsItem.acceleratorText: Ctrl+A",
	"Sdtaudio*ExitItem.acceleratorText: Alt+F4",

	/* Accelerators fallbacks. */
	"Sdtaudio*NewItem.accelerator: Ctrl<Key>N",	
	"Sdtaudio*OpenItem.accelerator: Ctrl<Key>O",	
	"Sdtaudio*SaveAsItem.accelerator: Ctrl<Key>A",	
	"Sdtaudio*ExitItem.accelerator: Alt<Key>F4",

	/* Mnemonic fallbacks. */
	"Sdtaudio*MenuBar.FileCascade.mnemonic:		F",
	"Sdtaudio*MenuBar.OptionsCascade.mnemonic:	O",
	"Sdtaudio*MenuBar.HelpCascade.mnemonic:		H",
	"Sdtaudio*FilePulldown.NewItem.mnemonic:	N",
	"Sdtaudio*FilePulldown.OpenItem.mnemonic:	O",
	"Sdtaudio*FilePulldown.SaveAsItem.mnemonic:	A",
	"Sdtaudio*FilePulldown.InfoItem.mnemonic:	I",
	"Sdtaudio*FilePulldown.ExitItem.mnemonic:	x",
	"Sdtaudio*OptionsPulldown.StartupItem.mnemonic:	S",
	"Sdtaudio*OptionsPulldown.PlayItem.mnemonic:	P",
	"Sdtaudio*OptionsPulldown.RecordItem.mnemonic:	R",
	"Sdtaudio*OptionsPulldown.DevicesItem.mnemonic:	D",
	"Sdtaudio*HelpPulldown.OverviewItem.mnemonic:	O",
	"Sdtaudio*HelpPulldown.TasksItem.mnemonic:	T",
	"Sdtaudio*HelpPulldown.ReferenceItem.mnemonic:	R",
	"Sdtaudio*HelpPulldown.OnItemItem.mnemonic:	I",
	"Sdtaudio*HelpPulldown.UsingHelpItem.mnemonic:	U",
	"Sdtaudio*HelpPulldown.AboutAudioItem.mnemonic:	A",
	NULL
};

/* The sdtaudio data structure. */
WindowData *wd = (WindowData *) NULL;

/*******************************************************************/
void
showAudioUsage(void)
{
	fprintf(stdout, 
		catgets(msgCatalog, 1, 19, "Usage: %s [-p] [-r] [-w] [-h | -?]\n\t-p\tplay specified audiofile on start\n\t-r\tstart in record mode\n\t-w\tstart with wave window displayed\n\t-h or\tdisplay usage\n\t-?\n"), 
		programName);

	exit(0);
}

/*******************************************************************/
void
checkCmdlineArgs(AudioCmdLine *largs, int argc, char **argv)
{
	int	i = 0;

	/* Start at 1, since argv[0] is the command itself.
	 * REMEMBER - OPTIONS ARE CASE SENSITIVE!  
	 */
	for (i = 1; i < argc; i++) {

		if (strcmp(argv[i], "-tooltalk") == 0) {

			/* Set tooltalk to true. */
			largs->tt_enabled = True;

		} else if (strcmp(argv[i], "-r") == 0) {

			/* Bring up sdtaudio in record mode, but make sure
			 * that play mode is not set.  This could happen
			 * if user specified both arguments, the -p before
			 * -r, in the command line.
			 */
			if (!(largs->play_enabled))
				largs->record_enabled = True;

		} else if (strcmp(argv[i], "-p") == 0) {

			/* Bring up sdtaudio in play mode, but make sure
			 * that record mode is not set.  This could happen
			 * if user specified both arguments, the -r before
			 * -p, in the command line.
			 */
			if (!(largs->record_enabled))
				largs->play_enabled = True;

		} else if (strcmp(argv[i], "-w") == 0) {

			/* Bring up sdtaudio with the wave window displayed. */
			largs->wave_win_open = True;

		} else if (strcmp(argv[i], "-h") == 0 || 
			   strcmp(argv[i], "-?") == 0) {

			/* Show usage */
			showAudioUsage();
		} else {

			/* Is the user giving a bogus -whatever 
			 * argument?
			 */
			if (*argv[i] == '-')
				showAudioUsage();

			/* Assume the argument is a file name. */
			if (largs->filename == NULL)
				largs->filename = strdup(argv[i]);
		}
	}
}

/*******************************************************************/
int
main(int argc, char **argv)
{
	int             i = 0;
	appResourceRec  argvals;
	AudioCmdLine	lineArgs;

	ww_font = NULL;

	argv0 = argv[0];
	dtb_save_command(argv[0]);
	programName = strrchr(argv[0], '/');
	if (programName == NULL)
		programName = argv[0];
	else
		++programName;

	/* Initialize the command line arguments. */
	lineArgs.tt_enabled = False;
	lineArgs.play_enabled = False;
	lineArgs.record_enabled = False;
	lineArgs.wave_win_open = False;
	lineArgs.filename = NULL;

	/* I18N support. */
	XtSetLanguageProc(NULL, NULL, NULL);

	/* Initialize DT environment, so that right language
	 * paths are picked up.
	 */
	_DtEnvControl(DT_ENV_SET);

	/* Creat the topmost shell. */
	appShell = XtAppInitialize(&appContext, ApplicationClass,
				   optionTable, XtNumber(optionTable),
				   &argc, argv, fallbacks, NULL, 0);
	XtGetApplicationResources(appShell, &argvals,
				  appResources, XtNumber(appResources),
				  NULL, 0);
	DtAppInitialize(appContext, XtDisplay(appShell), appShell, 
			argv0, ApplicationClass);
	_DtGetHourGlassCursor(XtDisplay(appShell));

	/* Get the command line parameters. */
	checkCmdlineArgs(&lineArgs, argc, argv);

	/* Initialize message catalog. */
	msgCatalog = catopen(MessageCatalog, NL_CAT_LOCALE);

	/* Initialize the default tool options. */
	SDtAuGetOptions();

	WM_DELETE_WINDOW = XmInternAtom(XtDisplay(appShell), 
					"WM_DELETE_WINDOW", False);
	WM_SAVE_YOURSELF = XmInternAtom(XtDisplay(appShell),
					 "WM_SAVE_YOURSELF", False);

	appnameString = catgets(msgCatalog, 1, 1, "Audio");
	separatorString = catgets(msgCatalog, 1, 5, " - ");
	untitledString = catgets(msgCatalog, 1, 6, "(untitled)");

	/* Initialize Data Typing. */
	DtDtsLoadDataTypes();

	/* Initialize the window. */
	if (!SDtAuNewWindow(LOAD_EMPTY, NULL, 0))
		/* Could not create an sdtaudio window and data
		 * structure.  Most likely, we're out of memory.
		 */
		Fatal(catgets(msgCatalog, 1, 16, "could not create audio structure - out of memory"));

	/* Initialize tooltalk, if required. */
	if (lineArgs.tt_enabled == TRUE) {

		if (!audtt_init(appnameString, wd))
			Fatal(catgets(msgCatalog, 1, 7, 
				      "could not initialize tooltalk."));
	}

	/* Did user request that the wave window be displayed? */
	if (lineArgs.wave_win_open) {
		SDtAuShowWaveWin(wd);
		wd->show_wave = True;
	}

	/* Did user request that sdtaudio be started in record mode? */
	if (lineArgs.record_enabled) {

		/* If there is a file name given, record and
		 * save the sample using the file name.
		 */
		if (lineArgs.filename) {

			if (CanWrite(wd->toplevel, lineArgs.filename))

				/* Use the given filename to save.  Other-
				 * wise, use a temporary file name.
				 */
				wd->name = XtNewString(lineArgs.filename);
		}

		/* Start recording. */
		if (wd->options->useDefaultRecValues)
			xa_record_audio(wd);
		else if (SDtAuGetRecordValues(wd))
			/* Only record if user chose record settings. */
			xa_record_audio(wd);		
	}
		
	/* If a file was given, load it.  */
	if (lineArgs.filename && !(lineArgs.record_enabled)) {

		LoadFile(wd, lineArgs.filename);
		if (!wd->options->autoPlayOnOpen && lineArgs.play_enabled)
			xa_play_audio(wd);
	}

	/*
	 * Start the GUI.  Note that we explicitly do not realize the
	 * appShell widget, since it is the unmapped parent of all of the
	 * top-level shells we pop up.
	 */
	XtAppMainLoop(appContext);

	if (lineArgs.filename)
		free(lineArgs.filename);

	tt_close();
	return(0);
}

/* Function	: Fatal()
 * Objective	: Print out an error message to stderr, and exit out of
 *		  sdtaudio.
 * Arguments	: char *		- message to display to user.
 * Return Value : None.
 */
void 
Fatal(char *msg)
{
	fprintf(stderr, "%s: %s\n", programName, msg);
	fprintf(stderr, catgets(msgCatalog, 1, 15, "%s: exiting...\n"), 
		programName);
	exit(1);
}

/* Function	: SetTitle()
 * Objective	: Set the widget's title to the program name followed by 
 *		  a separator followed by the trailing pathname component 
 *		  of the filename.  The widget must be a shell.  If name is 
 *		  NULL, uses "(untitled)" instead.
 * Arguments	: Widget		- the sdtaudio window
 *		  char *		- title to give to sdtaudio window.
 * Return Value : None.
 */
void 
SetTitle(Widget w, char *name)
{
	char            buf[1000];
	char           *p;

	if (name == NULL) {
		p = untitledString;
	} else {
		p = strrchr(name, '/');
		if (p == NULL)
			p = name;
		else
			p++;
	}
	sprintf(buf, "%s%s%s", appnameString, separatorString, p);
	XtVaSetValues(w, XtNtitle, buf, NULL);
}

/* Function	: NewData()
 * Objective	: Create a new WindowData structure.
 * Arguments	: None.
 * Return Value : WindowData *		- the sdtaudio data structure.
 */
WindowData *
NewData(void)
{
	WindowData     *wd = (WindowData *) NULL;

	/* Allocate memory. */
	wd = XtNew(WindowData);
	if (wd == (WindowData *) NULL)
		return((WindowData *) NULL);

	/* Initialize the mutex locks. */
	if ((mutex_init(&(wd->xa_mtx), USYNC_THREAD, 0) != 0) ||
	    (mutex_init(&(wd->state_mtx), USYNC_THREAD, 0) != 0)) {

		Fatal(catgets(msgCatalog, 1, 14,
		      "Could not initialize sdtaudio locks."));
	}

	AT_MUTEX_LOCK(wd, "NewData");
	wd->play_state = STATE_STOPPED;
	AT_MUTEX_UNLOCK(wd, "NewData");

	wd->menu_items = XtNew(MenuData);
	if (wd->menu_items == NULL)
		return((WindowData *) NULL);
	wd->menu_items->SaveAsItem = NULL;
	wd->menu_items->InfoItem = NULL;
	wd->menu_items->AuItem = NULL;
	wd->menu_items->WavItem = NULL;
	wd->menu_items->AiffItem = NULL;
	wd->menu_items->StartupItem = NULL;
	wd->menu_items->PlayItem = NULL;
	wd->menu_items->RecordItem = NULL;
	wd->menu_items->DevicesItem = NULL;

	wd->options = NULL;

	wd->toplevel = NULL;
	wd->mainWindow = NULL;
	wd->base_form = NULL;
	wd->footer_msg = NULL;
	wd->status_msg = NULL;
	wd->play_button = NULL;
	wd->bwd_button = NULL;
	wd->fwd_button = NULL;
	wd->record_button = NULL;
	wd->record_light = NULL;
	wd->mute_button = NULL;
	wd->wave_button = NULL;
	wd->vol_gauge = NULL;
	wd->up_vol = NULL;
	wd->down_vol = NULL;
	wd->prog_scale = NULL;
	wd->level_meter = NULL;
	wd->prog_start = NULL;
	wd->prog_end = NULL;
	wd->wave_pane = NULL;
	wd->wave_window = NULL;
	wd->wave_start = NULL;
	wd->wave_end = NULL;
	wd->saveas_dlog = NULL;
	wd->format_menu = NULL;
	wd->about_dlog = NULL;
	wd->info_dlog = NULL;
	wd->info_name = NULL;
	wd->info_size = NULL;
	wd->info_length = NULL;
	wd->info_channels = NULL;
	wd->info_sample_rate = NULL;
	wd->info_encoding = NULL;
	wd->options_dlog = NULL;
	wd->options_form = NULL;
	wd->options_cmd_pane = NULL;
	wd->options_category = NULL;
	wd->opt_startup_pane = NULL;
	wd->standard_view_rb = NULL;
	wd->wave_view_rb = NULL;
	wd->opt_play_pane = NULL;
	wd->auto_play_open = NULL;
#ifdef SELECTION
	wd->auto_play_select = NULL;
#endif
	wd->play_vol_scale = NULL;
	wd->play_bal_scale = NULL;
	wd->opt_record_pane = NULL;
	wd->voice_menu_item = NULL;
	wd->cd_menu_item = NULL;
	wd->dat_menu_item = NULL;
	wd->recqual_option = NULL;
	wd->au_menu_item = NULL;
	wd->wav_menu_item = NULL;
	wd->aiff_menu_item = NULL;
	wd->audformat_option = NULL;
	wd->selected_rb = NULL;
	wd->ask_rb = NULL;
	wd->record_vol_scale = NULL;
	wd->record_bal_scale = NULL;
	wd->opt_devices_pane = NULL;
	wd->devices_speaker = NULL;
	wd->devices_headphone = NULL;
	wd->devices_line_out = NULL;
	wd->devices_mic = NULL;
	wd->devices_line_in = NULL;
	wd->devices_cd = NULL;
	wd->recval_dlog = NULL;
	wd->recval_quality = NULL;
	wd->recval_format = NULL;
	wd->open_dlog = NULL;
	wd->save_as_dlog = NULL;

	wd->safile = NULL;
	wd->sa_stream = NULL;
	wd->sa_real_format = NULL;

	wd->play_vol = 0;
	wd->play_bal = 0;
	wd->rec_vol = 0;
	wd->rec_bal = 0;
	wd->clip_pos = 0;
	wd->meter_gain = 0;

	wd->is_muted = SAFalse;
	wd->show_wave = False;
	wd->audio_modified = False;
	wd->blink_off = True;
	wd->recval_set = False;

	wd->wait_for_dialog = False;
	wd->options_changed = False;

	wd->wave_pixmap = NULL;

	wd->stop_pixmap = NULL;
	wd->ins_stop_pixmap = NULL;
	wd->play_pixmap = NULL;
	wd->ins_play_pixmap = NULL;
	wd->record_pixmap = NULL;
	wd->ins_record_pixmap = NULL;
	wd->rec_blink_pixmap = NULL;
	wd->ins_blink_pixmap = NULL;
	wd->rec_blink_off_pixmap = NULL;
	wd->ins_blink_off_pixmap = NULL;

	strcpy(wd->old_pos_str, "");
	strcpy(wd->new_pos_str, "");

	wd->sel_start_x = 0;
	wd->sel_end_x = 0;
	wd->record_time_out = 0;
	wd->select_start = FALSE;
	wd->selection_made = FALSE;
	wd->name = NULL;

	/* Cursor seen in wave window.  Initialize both old
	 * and new cursors to the same location.  We're defining
	 * an inverted triangle as the cursor.  The triangles base
	 * is 10 pixels wide, and it's height is 10 pixels.  The
	 * first point is the tip, and the succeeding point's coordinates
	 * are relative to the tip's coordinates.
	 */
	wd->new_cursor[0].x =  0; wd->new_cursor[0].y =  WAVE_CURSOR_HEIGHT;
	wd->new_cursor[1].x = -5; wd->new_cursor[1].y = -10;
	wd->new_cursor[2].x = WAVE_CURSOR_BASE; wd->new_cursor[2].y =   0;
	wd->old_cursor[0] = wd->new_cursor[0];
	wd->old_cursor[1] = wd->new_cursor[1];
	wd->old_cursor[2] = wd->new_cursor[2];

	wd->trackWaveMotion = False;
	wd->drawEnvInitialized = False;
	wd->waveForeGrndGC = NULL;
	wd->wavePntrGC = NULL;
	wd->waveSelGC = NULL;
	wd->waveReverseGC = NULL;
	wd->wavePMGC = NULL;
	wd->meterGC = NULL;

	wd->voice_menu_item = NULL;
	wd->cd_menu_item = NULL;

	/* Push it onto the front of the global list and return. */
	wd->next = windowList;
	windowList = wd;
	return wd;
}

/* Function	: DestroyData()
 * Objective	: Destroy a WindowData structure that was allocated by
 *		  NewData().
 * Arguments 	: WindowData *		- the sdtaudio data structure
 * Return Value	: None.
 */
void
DestroyData(WindowData * wd)
{
	WindowData    **p;
	Pixmap		tmppix;

	FreeData(wd);

	/* Prabhat :: free the pixmaps */
        tmppix = wd->stop_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XmDestroyPixmap(XtScreenOfObject(wd->play_button), tmppix);

        tmppix = wd->ins_stop_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XFreePixmap(XtDisplayOfObject(wd->play_button), tmppix);

        tmppix = wd->play_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XmDestroyPixmap(XtScreenOfObject(wd->play_button), tmppix);

        tmppix = wd->ins_play_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XFreePixmap(XtDisplayOfObject(wd->play_button), tmppix);

        tmppix = wd->record_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XmDestroyPixmap(XtScreenOfObject(wd->record_button), tmppix);

        tmppix = wd->ins_record_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XFreePixmap(XtDisplayOfObject(wd->record_button), tmppix);

        tmppix = wd->rec_blink_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XmDestroyPixmap(XtScreenOfObject(wd->record_light), tmppix);

        tmppix = wd->rec_blink_off_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XmDestroyPixmap(XtScreenOfObject(wd->record_light), tmppix);

        tmppix = wd->ins_blink_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XFreePixmap(XtDisplayOfObject(wd->record_light), tmppix);

        tmppix = wd->ins_blink_off_pixmap;
        if (tmppix == XmUNSPECIFIED_PIXMAP || tmppix == (Pixmap)NULL);
        else
          XFreePixmap(XtDisplayOfObject(wd->record_light), tmppix);

	if (wd->toplevel != NULL)
		XtDestroyWidget(wd->toplevel);
	if (wd->open_dlog != NULL)
		XtDestroyWidget(wd->open_dlog);
	if (wd->save_as_dlog != NULL)
		XtDestroyWidget(wd->save_as_dlog);

	/* remove from the global list */
	p = &windowList;
	while (*p != NULL) {
		if (*p == wd) {
			*p = wd->next;
			break;
		}
		p = &((*p)->next);
	}

	if (wd->menu_items != NULL)
		XtFree((char *) wd->menu_items);

	XtFree((char *) wd);
}

/* Function	: LoadFile()
 * Objective	: Load a file into sdtaudio.
 * Arguments	: WindowData		- the sdtaudio data structure
 * 		  char *		- full path/file name of file
 * Return Value : Boolean True if file was loaded, false otherwise.
 */
Boolean
LoadFile(WindowData * wd, char *fileName)
{
	SAError         error;
	SAFormat        alternate;

	/*
	 * If file doesn't exist or if the user doesn't have read access,
	 * then return false.
	 */
	if (!CanRead(wd->toplevel, fileName)) {
		return(False);
	}

	/* Prabhat : Let user know if he chose a bad file type */
	if (findFileType((const char *) fileName) == SANoType) {

		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 4, 7,
						 "File type not supported."),
			    BUTTON_IDENT, ANSWER_ACTION_1, 
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);

		return (False);
	}

	_DtTurnOnHourGlass(wd->toplevel);
	/* In case the user loads a file while still playing another
	 * one...
	 */
	if (wd->play_thread_data && wd->play_state == STATE_PLAY) {

		xa_stop_audio(wd);

		/* Loop until the state has changed to stop. */
		while(wd->play_state != STATE_STOPPED);
	}

	XA_LOCK(wd, "LoadFile");

	/* Close the audio file. */
	if (wd->safile == (SAFile *) NULL) {

		/* Allocate memory for safile structure. */
		wd->safile = (SAFile *) calloc(1, sizeof(SAFile));
		if (wd->safile == NULL) {

			_DtTurnOffHourGlass(wd->toplevel);
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 1, 10, "Could not allocate memory for sound file."),
				    BUTTON_IDENT, ANSWER_ACTION_1,
					catgets(msgCatalog, 2, 62,
						"Continue"),
				    NULL);
			return(False);
		}

	} else if (wd->safile->format.encoding != SAEncoding_NONE) {

		if (!SAFileClose(wd->safile, &error)) {

			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 1, 21, "Failed to close current audio file."),
				    BUTTON_IDENT, ANSWER_ACTION_1,
					catgets(msgCatalog, 2, 62,
						"Continue"),
				    NULL);
		}
	}

	if (SAFileOpen(fileName, wd->safile, &error) == SAFalse) {

		char            error_msg[BUFSIZ];

		XA_UNLOCK(wd, "LoadFile");
		if (error.code == SAError_NO_FILE_TYPE)
			sprintf(error_msg, catgets(msgCatalog, 4, 7, "File type not supported."));
		else if (error.code == SAError_CANNOT_OPEN)
			sprintf(error_msg, catgets(msgCatalog, 4, 8, "Could not open file."));
		else if (error.code == SAError_CANNOT_DECODE_FILE)
			sprintf(error_msg, catgets(msgCatalog, 4, 13, "Could not parse audio file information."));
		else
			sprintf(error_msg, catgets(msgCatalog, 4, 12, 
						   "Invalid file."));

       		_DtTurnOffHourGlass(wd->toplevel);
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, error_msg,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		if (wd->info_dlog && XtIsManaged(wd->info_dlog))
			SDtAuUpdateInfoDlog(wd);
		SDtAuUpdateTimeLabels(wd);
		if (wd->toplevel != NULL)
			SetTitle(wd->toplevel, NULL);
		wd->name = NULL;
		return False;
	}

#ifdef VERBOSITY
	/* Print out audio file attributes... */
	fprintf(stderr, "Channels: %u Sample Rate: %u\n",
		wd->safile->format.channels, wd->safile->format.sample_rate);
	fprintf(stderr, "Samples Per Unit: %u Bytes Per Sample %u\n",
		wd->safile->format.samples_per_unit, 
		wd->safile->format.sample_size);
	fprintf(stderr, "Encoding: %u\n", wd->safile->format.encoding);
	fprintf(stderr, "Data size: %u\n", wd->safile->data_size);

#endif	
	/* Prabhat :: Need to free stream here */
	if (wd->sa_stream != NULL) {
		SAStreamClose(wd->sa_stream, &error);
		wd->sa_stream = NULL;
	}

	/* Create the port for playing */
	wd->sa_stream = SAStreamOpen(NULL, &error);

	/* Prabhat :: Should check if a valid stream was opened before proceeding */
	if (wd->sa_stream == NULL) {

		XA_UNLOCK(wd, "LoadFile");
		 _DtTurnOffHourGlass(wd->toplevel);
		AlertPrompt(wd->toplevel,
			DIALOG_TYPE, XmDIALOG_ERROR,
			DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
				              "Audio - Error"),
			DIALOG_TEXT, catgets(msgCatalog, 3, 58, 
				"Could not open stream to audio device."),
			BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			NULL);
		return False;
	}

	/* Initialize real_format to NULL.  No data conversions. */
	if (wd->sa_real_format != NULL) {
		free(wd->sa_real_format);
		wd->sa_real_format = NULL;
	}

	if (SAStreamSetFormat(wd->sa_stream, &(wd->safile->format),
			      &alternate, &error) == SAFalse) {

		XA_UNLOCK(wd, "LoadFile");
       		_DtTurnOffHourGlass(wd->toplevel);
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 4, 14, "Audio format not supported on current hardware."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return False;
	}

	if (alternate.encoding != SAEncoding_NONE) {

		wd->sa_real_format = (SAFormat *) calloc(sizeof(SAFormat), 1);

		if (wd->sa_real_format == NULL) {
			XA_UNLOCK(wd, "LoadFile");
			_DtTurnOffHourGlass(wd->toplevel);
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 3, 46, "Could not allocate memory for operation."),
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			return(False);

		}
		else
			*wd->sa_real_format = alternate;
	}

	/* Set the GUI components to reflect that a new file was loaded. */
	if (wd->toplevel != NULL)
		SetTitle(wd->toplevel, fileName);
	wd->name = XtNewString(fileName);
	XtSetSensitive(wd->menu_items->InfoItem, True);
	if (wd->info_dlog && XtIsManaged(wd->info_dlog))
		SDtAuUpdateInfoDlog(wd);
	SDtAuUpdateTimeLabels(wd);
	XmScaleSetValue(wd->prog_scale, 0);
	wd->clip_pos = 0;

	/* Free the current wave pixmap. */
	if (wd->wave_pixmap != NULL) {

		XFreePixmap(XtDisplay(wd->wave_window), wd->wave_pixmap);
		wd->wave_pixmap = NULL;
	}
	if (wd->show_wave == TRUE) {
		SDtAuUpdateWaveWin(wd);
	}
	XA_UNLOCK(wd, "LoadFile");

	/*
	 * Set these buttons to sensitive, so that the user can play and and
	 * rewind.
	 */
	XtSetSensitive(wd->prog_scale, True);
	XtSetSensitive(wd->bwd_button, True);
	XtSetSensitive(wd->fwd_button, True);
	XtSetSensitive(wd->play_button, True);
	XmUpdateDisplay(wd->toplevel);

	/* Play the sound file per user preference. */
	if (wd->options->autoPlayOnOpen == True)
		xa_play_audio(wd);

	XtSetSensitive(wd->record_button, False);
	XtSetSensitive(wd->menu_items->SaveAsItem, True);
	_DtTurnOffHourGlass(wd->toplevel);

	/* Delete the old buffer, if loaded previously from buffer. */
	if (temp_buffer_name) {
		unlink(temp_buffer_name);
		free(temp_buffer_name);
		temp_buffer_name = (char *) NULL;
	}

	return True;
}

/*
 * Load a buffer
 */
Boolean LoadBuffer(WindowData *wd, void *buf, int len, char *tmp_name)
{
	char      *tmp_file = (char *) NULL;
	char      *tmp_dir = (char *) NULL;
	char      *tmp_filename = (char *) NULL;
	int	   dnd_fd;
	int	   bytes_written, bytes_remaining = len;
        Boolean    status;

	/* Always try and use $HOME/dt/tmp. */
	tmp_dir = getenv("HOME");
	if (!tmp_dir) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
				"Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 64,
				"Could not open temporary file!"),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return(False);
	}

	/* Allocate memory used up by path and filename. */
	if (tmp_name)
		tmp_file = (char *) malloc(strlen(tmp_dir) +
					   strlen("/.dt/tmp/") +
					   strlen(tmp_name) + 1);
	else {
		tmp_filename = tempnam(tmp_dir, "audrx");
		if (!tmp_filename) {
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
					"Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 3, 64,
					"Could not open temporary file!"),
				    BUTTON_IDENT, ANSWER_ACTION_1,
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			return(False);
		}

		tmp_file = (char *) malloc(strlen(tmp_dir) +
					   strlen("/.dt/tmp/") +
					   strlen(tmp_filename) + 1);
	}

	/* Was space allocated? */
	if (!tmp_file) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
				"Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 64,
				"Could not open temporary file!"),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		if (tmp_filename)
			free(tmp_filename);
		return(False);
	}

	/* Construct the file name. */
	if (tmp_name)
		sprintf(tmp_file, "%s/.dt/tmp/%s", tmp_dir, tmp_name);
	else {
		sprintf(tmp_file, "%s/.dt/tmp/%s", tmp_dir, tmp_filename);
		free(tmp_filename);
	}

	/* Open a file descriptor for the temporary file for writing. */
	dnd_fd = open(tmp_file, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (dnd_fd == -1) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
				"Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 64,
				"Could not open temporary file!"),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		if (tmp_filename)
			free(tmp_filename);
		return(False);
	}

	/* Write out the buffer. */
	while (bytes_remaining > 0) {

		bytes_written = write(dnd_fd, buf, bytes_remaining);
		if (bytes_written == -1) {

			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
					"Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 3, 65,
					"Could not write to temporary file!"),
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			free(tmp_file);
			tmp_file = (char *) NULL;
			close(dnd_fd);
			return(False);
		}
		bytes_remaining -= bytes_written;
	}

	/* Buffer written to the file, so now load it into sdtaudio. */
	close(dnd_fd);
	status = LoadFile(wd, tmp_file);
	if (status == True)
		/* Load file should have deleted the old temp_buffer_name,
		 * so just assign tmp_file to temp_buffer_name. */
		temp_buffer_name = tmp_file;

	return(status);
}


/* Function	: FreeData()
 * Objective	: Free some of the audio data structure members.
 * Arguments	: WindowData *		- the sdtaudio structure
 * Return Value	: None.
 */
void 
FreeData(WindowData * wd)
{
	XtFree(wd->name);
	wd->name = NULL;

	XA_LOCK(wd, "FreeData");
	if (wd->safile) {
		free(wd->safile);
		wd->safile = NULL;
	}
	XA_UNLOCK(wd, "FreeData");

	if (wd->toplevel != NULL)
		SetTitle(wd->toplevel, NULL);
}
