/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 *
 * default_options.h
 *
 * This file species the defaults for the sdtaudio options resource database.
 */


#pragma ident "@(#)default_options.h	1.4 97/02/20 SMI"

/* Strings in the resource file. */
#define AUD_SHOW_WAVE        "sdtaudio.showWave"
#define AUD_AUTO_PLAY_OPEN   "sdtaudio.autoPlayOnOpen"
#define AUD_PLAY_VOLUME      "sdtaudio.defaultPlayVolume"
#define AUD_PLAY_BALANCE     "sdtaudio.defaultPlayBalance"
#define AUD_REC_VOLUME       "sdtaudio.defaultRecordVolume"
#define AUD_REC_BALANCE      "sdtaudio.defaultRecordBalance"
#define AUD_REC_QUALITY      "sdtaudio.defaultRecordQuality"
#define AUD_REC_FILE         "sdtaudio.defaultRecordFileType"
#define AUD_USE_REC_VALUE    "sdtaudio.useDefaultRecordValues"
#define AUD_DEVICE_FLAGS     "sdtaudio.deviceFlag"

#ifdef SELECTION
#define AUD_AUTO_PLAY_SELECT "sdtaudio.autoPlayOnSelect"
#endif

/* Strings for the AUD_REC_QUALITY resource. */
#define AUD_RSRC_VOICE       "voice"
#define AUD_RSRC_CD          "cd"
#define AUD_RSRC_DAT         "dat"

/* Strings for the AUD_REC_FILE resource. */
#define AUD_RSRC_AU          "au"
#define AUD_RSRC_WAV         "wav"
#define AUD_RSRC_AIFF        "aiff"

/* Default play and recording volume levels and balances. */
#define DEFAULT_VOLUME   50
#define DEFAULT_BALANCE  50

/* Input and output device flags. */
#define INPUT_MIC		0x01
#define INPUT_LINE_IN		0x02
#define INPUT_CD		0x04
#define OUTPUT_SPEAKER  	0x08
#define OUTPUT_HEADPHONE	0x10
#define OUTPUT_LINE_OUT		0x20
#define DEFAULT_DEVICE_FLAG	INPUT_MIC | OUTPUT_SPEAKER

/* If user doesn't have defaults file yet, then the following
 * resources are to be used as the default.
 */
char *default_options = 
  "! This is the sdtaudio options file, please do not edit"
  "sdtaudio.showWave: False\n"
  "sdtaudio.autoPlayOnOpen: True\n"
  "sdtaudio.autoPlayOnSelect: False\n"
  "sdtaudio.defaultPlayVolume: 50\n"
  "sdtaudio.defaultPlayBalance: 0\n"
  "sdtaudio.defaultRecordVolume: 50\n"
  "sdtaudio.defaultRecordBalance: 0\n"
  "sdtaudio.defaultRecordQuality: voice\n"
  "sdtaudio.defaultRecordFileType: au\n"
  "sdtaudio.useDefaultRecordValues: True\n"
  "sdtaudio.deviceFlag: 9";
