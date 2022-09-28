/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)help.h	1.1 97/03/18 SMI"

#ifndef _SDTAUDIO_HELP_H
#define _SDTAUDIO_HELP_H

/* Help volume associated with sdtaudio. */
#define	SDTAUDIO_HELP_VOLUME	"Sdtaudio"

/* Help IDs for the Help menu */
#define HELP_OVERVIEW			"_HOMETOPIC"
#define HELP_TASKS			"TASKS"
#define HELP_REF			"REFERENCE"
#define HELP_USING_HELP			"_HOMETOPIC"
#define HELP_AUDIO_INFO			"AudioFileInfoDialog"
#define HELP_AUDIO_START_OPTIONS	"ToSetStartOptions"
#define HELP_AUDIO_PLAY_OPTIONS		"ToSetPlayOptions"
#define HELP_AUDIO_RECORD_OPTIONS	"ToSetRecordOptions"
#define HELP_AUDIO_DEVICES_OPTIONS	"ToSetDeviceOptions"
#define HELP_AUDIO_SAVE_NEW		"ToSaveANewFile"
#define HELP_AUDIO_SAVE_AS		"ToSaveAsAFile"
#define HELP_AUDIO_OPEN			"ToOpenFile"
#define HELP_AUDIO_MAIN_WIN		"AudioWindow"
#define HELP_AUDIO_FILE_MENU		"AudioFileMenu"
#define HELP_AUDIO_OPTIONS_MENU		"AudioOptionsMenu"
#define HELP_AUDIO_HELP_MENU		"AudioHelpMenu"
#define HELP_AUDIO_WAVE_WIN		"AudioWindowWave"

/* Help event numbers. */
typedef enum {
	HELP_OVERVIEW_EVENT = 7001,
	HELP_TASKS_EVENT = 7002,
	HELP_REF_EVENT = 7003,
	HELP_ONITEM_EVENT = 7004,
	HELP_USING_HELP_EVENT = 7005,
	HELP_AUDIO_INFO_EVENT = 7006,
	HELP_AUDIO_OPTIONS_EVENT = 7007,
	HELP_AUDIO_SAVE_AS_EVENT = 7008,
	HELP_AUDIO_OPEN_EVENT = 7009,
	HELP_AUDIO_SET_RECORD_VALUES_EVENT = 7010,
	HELP_AUDIO_MAIN_WIN_EVENT = 7011,
	HELP_AUDIO_FILE_MENU_EVENT = 7012,
	HELP_AUDIO_OPTIONS_MENU_EVENT = 7013,
	HELP_AUDIO_HELP_MENU_EVENT = 7014,
	HELP_AUDIO_WAVE_WIN_EVENT = 7015
} HELP_EVENT;

/*
 * Audio help item consists of the parent window of the help dialog, and
 * the location id to display in the help dialog.  The window data
 * variable is also provided in case we need some info from it.
 */
typedef struct _SDtAudioHelpItem {
	Widget		help_parent_win;
	HELP_EVENT	help_event;
	WindowData     *wd;
} SDtAudioHelpR, *SDtAudioHelpP;


#endif		/* _SDTAUDIO_HELP_H */
