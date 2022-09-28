/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)sdtaudio.h	1.34 97/05/16 SMI"

#ifndef _SDTAUDIO_H_

#include "safile.h"
#include "sa.h"

/* Portability for catopen */
#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE 0
#endif

#define ApplicationClass "Sdtaudio"
#define PROPS_FILE      "/.dt/sessions/Sdtaudio"
#define MessageCatalog  "sdtaudio"
#define IconPixmap      "SDtAudapp.l"


/* Memory constant used to increase, for realloc() calls. */
#define MallocInc 10

/* Layout Constants */
#define WIDGET_OFFSET         10
#define TOP_WIDGET_OFFSET     WIDGET_OFFSET
#define LEFT_WIDGET_OFFSET    WIDGET_OFFSET
#define RIGHT_WIDGET_OFFSET   WIDGET_OFFSET
#define BOTTOM_WIDGET_OFFSET  WIDGET_OFFSET
#define CONTROL_BUTTON_WIDTH  72
#define CONTROL_BUTTON_HEIGHT (CONTROL_BUTTON_WIDTH/2) - 1
#define RECORD_BLINKER_WIDTH  6
#define RECORD_BLINKER_OFFSET 2
#define MUTE_BUTTON_WIDTH     CONTROL_BUTTON_HEIGHT
#define GAUGE_WIDTH           22
#define OPTIONS_SCALE_WIDTH   215
#define PROGRESS_BAR_TOP_GAP  5
#define VOL_WIDGET_OFFSET     8
#define LEVEL_LIGHT_BORDER    1
#define MAX_SEGMENTS          20
#define WAVE_WINDOW_HEIGHT    140
#define WAVE_CURSOR_BASE      10
#define WAVE_CURSOR_HEIGHT    10

enum {
	SDTA_VOICE_QUALITY,
	SDTA_CD_QUALITY,
	SDTA_DAT_QUALITY,
	SDTA_WAVE_VIEW,
	SDTA_STANDARD_VIEW
};


typedef enum {
	STATE_STOPPED,
	STATE_PLAY,
	STATE_RECORD
} PlayState;


typedef enum {
	PLAY_MSG,
	STOP_MSG,
	RECORD_MSG,
	END_MSG,
	ERROR_MSG,
	STATUS_MSG,
	LEVEL_MSG,
	INDEX_MSG,
	WAIT_MSG,
	SEEK_ERR_MSG,
	PLAY_ERR_MSG,
	RECORD_ERR_MSG,
	BUFFER_ERR_MSG
} MsgTag;


typedef enum {
	LOAD_EMPTY,
	LOAD_FILE,
	LOAD_BUFFER
} LoadType;


typedef struct {
	MsgTag              tag;

        union {
	        PlayState   prev_state;
                double      meter_gain;
                int         index;
	} u;

	SAError		    error;
} MsgData;


typedef struct {
	struct _WindowData *wd;
	int                 audio_to_uiFd[2];
	int                 ui_to_audioFd[2];
	XtInputId           inputId;
} XaThreadData;


typedef struct _AudioCmdLine {
	Boolean     tt_enabled;
	Boolean     play_enabled;
	Boolean     record_enabled;
	Boolean     wave_win_open;
	char       *filename;
} AudioCmdLine;


typedef struct _AudioProps {

	/* These options are for playing. */
	Boolean         showWave;             /* Start with wave view. */
	Boolean         autoPlayOnOpen;       
	Boolean         autoPlayOnSelect;
	int             playVolume;
	int             playBalance;

	/* These options are for recording. */
	int		recordVolume;
	int		recordBalance;
	int             recordingQuality;
	SAFileType	recordingFormat;
	Boolean         useDefaultRecValues;

	/* Device flags - determine what devices to use. */
	int		deviceFlag;
} AudioProps;


typedef struct _MenuData {
	Widget		SaveAsItem;
	Widget		InfoItem;
        Widget          AuItem;
        Widget          WavItem;
        Widget          AiffItem;
	Widget		StartupItem;
	Widget		PlayItem;
	Widget		RecordItem;
	Widget		DevicesItem;
} MenuData;


typedef struct _WindowData {

#ifndef NO_THREADS			/* Mutex locks used in program. */
	mutex_t         xa_mtx;
	mutex_t         state_mtx;
#endif

	PlayState       play_state;

	AudioProps     *options;        /* the audio options structure */
	MenuData       *menu_items;	/* list of menu items we care about */

	Widget          toplevel;
	Widget          mainWindow;

	/* Main sdtaudio dialog components. */
	Widget          base_form;	
	Widget          footer_msg;
	Widget          status_msg;

	Widget		control_pane;

	Widget          play_button;	
	Widget          bwd_button;	/* rewind button */
	Widget          fwd_button;	/* forward button */
	Widget          record_button;
	Widget          record_light;

	Widget          mute_button;
	Widget          wave_button;	/* toggle to hide/show wave window */
	Widget          vol_gauge;	/* volume indicator */
	Widget          up_vol;	        /* increase volume button */
	Widget          down_vol;	/* decrease volume button */

	Widget          prog_scale;
	Widget          level_meter;
	Widget          prog_start;
	Widget          prog_end;

	Widget          wave_pane;
	Widget          wave_window;
	Widget          wave_start;
	Widget          wave_end;

	/* Save As dialog. */
	Widget          saveas_dlog;
        Widget          format_menu;

	/* About dialog. */
	Widget          about_dlog;

	/* Get Info dialog. */
	Widget          info_dlog;
	Widget          info_name;
	Widget          info_size;
	Widget          info_length;
	Widget          info_channels;
	Widget          info_sample_rate;
	Widget          info_encoding;

	/* Options dialog. */
	Widget          options_dlog;
	Widget		options_form;
	Widget		options_cmd_pane; /* Command buttons OK, Cancel, etc. */
	Widget		options_category;
	Widget		options_ok;	  /* Track the OK and cancel button. */
	Widget		options_cancel;

	/* Options dialog - Start-up. */
	Widget          opt_startup_pane;
	Widget          standard_view_rb;
	Widget          wave_view_rb;

	/* Options dialog - Play. */
	Widget		opt_play_pane;
	Widget          auto_play_open;	  /* auto play on open */
#ifdef SELECTION
	Widget          auto_play_select; /* auto play selection */
#endif
	Widget          play_vol_scale;
	Widget          play_bal_scale;

	/* Options dialog - Record. */
	Widget          opt_record_pane;
	Widget          voice_menu_item;
	Widget          cd_menu_item;
	Widget		dat_menu_item;
	Widget		recqual_option;
	Widget		au_menu_item;
	Widget		wav_menu_item;
	Widget		aiff_menu_item;
	Widget		audformat_option;
	Widget          selected_rb;	/* use selected default radio box */
	Widget          ask_rb;	        /* ask me when I start recording  */
	Widget		record_vol_scale;
	Widget		record_bal_scale;

	/* Options dialog - Devices. */
	Widget		opt_devices_pane;
	Widget		devices_speaker;
	Widget		devices_headphone;
	Widget		devices_line_out;
	Widget		devices_mic;		/* Microphone. */
	Widget		devices_line_in;
	Widget		devices_cd;		/* Compact Disc */


	/* Recording values dialog - brought up if user sets the
	 * option to inquire every time record is pressed.
	 */
 	Widget          recval_dlog;
	Widget          recval_quality;
	Widget          recval_format;

	/* File selection boxes. */
	Widget          open_dlog;
	Widget          save_as_dlog;

	/* Sun audio variables. */
	SAFile	       *safile;       
        SAStream       *sa_stream;
        SAFormat       *sa_real_format;

	/* Audio state variables - keep track of audio state. */
	int		play_vol;       /* Current volume and balance. */
	int		play_bal;
	int		rec_vol;
	int		rec_bal;
	int             clip_pos;       /* Play point position. */
        double          meter_gain;     /* Current level to display. */

	SABoolean       is_muted;	/* Mute button. */
	Boolean         show_wave;	/* Wave button. */
	Boolean		audio_modified; /* Did user record? */
	Boolean         blink_off;
	Boolean		recval_set;	/* Were record values specified? */

	/* Flag to determine if GUI waits for user response from a dialog. */
 	Boolean		wait_for_dialog;

	/* Flag to determine if user changed something in the options
	 * window, so that in case Cancel is pressed, dialog comes
	 * up asking if user wants to save changes.
	 */
	Boolean         options_changed;

	/* Wave window graphic. */
	Pixmap          wave_pixmap;

	/* Pixmap for GUI buttons and components. */
	Pixmap          stop_pixmap;
	Pixmap          ins_stop_pixmap;
	Pixmap          play_pixmap;
	Pixmap          ins_play_pixmap;
	Pixmap          record_pixmap;
	Pixmap          ins_record_pixmap;
	Pixmap          rec_blink_pixmap;
	Pixmap          ins_blink_pixmap;
	Pixmap          rec_blink_off_pixmap;
	Pixmap          ins_blink_off_pixmap;

	XtIntervalId    record_time_out;

	int             old_x;
	int             old_y;
	int             sel_start_x;
	int             sel_end_x;
	Boolean         select_start;
	Boolean         selection_made;
	Boolean         trackWaveMotion;
	Boolean         drawEnvInitialized;
	GC              waveForeGrndGC;
	GC              wavePntrGC;
	GC              waveSelGC;
	GC              waveReverseGC;
	GC              wavePMGC;
	GC              meterGC;

	char            old_pos_str[16];
	char            new_pos_str[16];

	XPoint		new_cursor[3];
	XPoint		old_cursor[3];
	char           *name;	/* NULL if no file */

	struct _WindowData *next;

	XaThreadData   *play_thread_data;
} WindowData;


/* FUNCTION PROTOTYPES */
Boolean         SDtAuNewWindow(LoadType, char *, int);
Boolean         SDtAuCreateSaveAsDlog(WindowData *);
Widget          SDtAuCreateAboutDlog(WindowData *);
Boolean         SDtAuCreateInfoDlog(WindowData *);
Boolean		SDtAuCreateOptionsDlog(WindowData *);
Boolean		SDtAuCreateRecordValuesDlog(WindowData *);


void            update_info_dlog(WindowData *);
void            draw_selection(WindowData *);
void            Fatal(char *);
void            ReallyExit(int);
void            SetTitle(Widget, char *);
void            InitWidgets(Widget, WindowData *);


WindowData     *NewData(void);
void            AssocData(WindowData *, Widget);
WindowData     *FindData(Widget);
void            DestroyData(WindowData *);
Boolean         LoadFile(WindowData *, char *);
Boolean         LoadBuffer(WindowData *, void *, int, char *);
void            FreeData(WindowData *);
/*
void            AddPoint(WindowData *, int, int);
*/
void            DrawPoint(Widget, int, int);
char           *AppendString(char *, char *);
void            SetMessage(Widget, char *);

#endif				/* _SDTAUDIO_H_ */
