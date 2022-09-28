/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)callbacks.c	1.47 97/06/04 SMI"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <thread.h>
#include <X11/Intrinsic.h>
#include <X11/Core.h>
#include <nl_types.h>
#include <Xm/XmAll.h>
#include <Dt/Dnd.h>
#include <Dt/Dts.h>
#include <Dt/HelpDialog.h>
#include <Dt/HourGlass.h>
#include <Tt/tttk.h>

#include "sdtaudio.h"
#include "help.h"
#include "extern.h"
#include "utils.h"
#include "audiofile.h"
#include "callbacks.h"
#include "cb_utils.h"
#include "thrdefs.h"
#include "file_utils.h"
#include "mutex.h"

extern char         *temp_rec_file;
extern char         *temp_buffer_name;
extern char         *argv0;
extern WindowData   *windowList;

/* Function	: AboutCb()
 * Objective	: Callback invoked when the Help->About Audio menu item
 *		  is selected.  The callback creates the dialog, if 
 *		  necessary.
 */
void
AboutCb(Widget w, XtPointer client_data, XtPointer cbd)
{
	WindowData     *wd = client_data;

	/* Check our data structure first. */
	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 31, "Cannot bring up \"About Audio\" dialog."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Create the about dialog, if necessary. */
	_DtTurnOnHourGlass(wd->toplevel);
	if (!wd->about_dlog)
		wd->about_dlog = SDtAuCreateAboutDlog(wd);
	if (!wd->about_dlog) {
		_DtTurnOffHourGlass(wd->toplevel);
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 31, "Cannot bring up \"About Audio\" dialog."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Manage and pop-up the about dialog. */
	XtManageChild(wd->about_dlog);
	XtPopup(XtParent(wd->about_dlog), XtGrabNone);
	_DtTurnOffHourGlass(wd->toplevel);
}

/* OnItemCb
 *
 * On-item help callback.
 */
void
OnItemCb(Widget w, XtPointer client_data, XtPointer cbd)
{
	Widget          widget, top = client_data;

	switch (DtHelpReturnSelectedWidgetId(top, NULL, &widget)) {

	case DtHELP_SELECT_VALID:
		_DtTurnOnHourGlass(top);
		while (widget != NULL) {
			if ((XtHasCallbacks(widget, XmNhelpCallback) == XtCallbackHasSome)) {
				XtCallCallbacks(widget, XmNhelpCallback, NULL);
				break;
			} else
				widget = XtParent(widget);
		}
		_DtTurnOffHourGlass(top);
		break;

	case DtHELP_SELECT_INVALID:
		AlertPrompt(top, DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, 
			    catgets(msgCatalog, 1, 4, "Audio - Help"),
			    DIALOG_TEXT, 
			    catgets(msgCatalog, 1, 37, 
				    "You must select an item within Audio."),
			    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 7, "Help"),
			    NULL);
		break;

	case DtHELP_SELECT_ERROR:
		AlertPrompt(top, DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, 
			    catgets(msgCatalog, 1, 4, "Audio - Help"),
			    DIALOG_TEXT, 
			    catgets(msgCatalog, 1, 11, 
				    "Help selection error, cannot continue.\n"),
			    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 7, "Help"),
			    NULL);
		break;

	case DtHELP_SELECT_ABORT:
		AlertPrompt(top, DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, 
			    catgets(msgCatalog, 1, 4, "Audio - Help"),
			    DIALOG_TEXT, 
			    catgets(msgCatalog, 1, 12, 
				    "Help selection aborted by user.\n"),
			    DIALOG_STYLE, XmDIALOG_FULL_APPLICATION_MODAL,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 7, "Help"),
			    NULL);
		break;
	}
}

/* Function	: DndTransferCB()
 * Objective	: Callback invoked when a dragged object is dropped onto
 *		  the sdtaudio window.
 */
void 
DndTransferCB(Widget w, XtPointer cd, XtPointer cb)
{
	WindowData     *wd = (WindowData *) cd;

	DtDndTransferCallbackStruct  *cbs =
		(DtDndTransferCallbackStruct *) cb;

	/* If there is no callback structure, then return. */
	if (cbs == NULL) {
		cbs->status = DtDND_FAILURE;
		return;
	}

	switch(cbs->dropData->protocol) {

	case DtDND_FILENAME_TRANSFER:
		SDtAuDndFilename(wd, cbs);
		break;

	case DtDND_BUFFER_TRANSFER:
		SDtAuDndBuffer(wd, cbs);
		break;

	default:
		cbs->status = DtDND_FAILURE;
		break;
	}
}

/*
 * Function	:	ShowWaveWindowCb()
 * Objective	:	Callback invoked when the wave window
 *			button is pressed.  The callback expands the
 *			sdtaudio window in order to display the wave
 *			window, or shrinks sdtaudio to hide the wave 
 *			window, depending on the toggle state.
 */
void
ShowWaveWindowCb(Widget widget, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 3, "Cannot toggle wave window display."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	_DtTurnOnHourGlass(wd->toplevel);
	if (wd->show_wave) {	

		/* Hide the wave window */
		SDtAuHideWaveWin(wd);
		SDtAuSetMessage(wd->footer_msg, 
			        catgets(msgCatalog, 3, 30, "Show Wave Window"));
		wd->show_wave = False;

	} else {

		/* Show the wave window */
		SDtAuShowWaveWin(wd);
		SDtAuSetMessage(wd->footer_msg, 
			        catgets(msgCatalog, 3, 55, "Hide Wave Window"));
		wd->show_wave = True;
	}
	_DtTurnOffHourGlass(wd->toplevel);
}

/* Function	: MuteButtonCb()
 * Objective	: Callback invoked when the mute button is pressed.
 *		  This callback either mutes the volume, or unmutes
 *		  the volume depending on the state of the mute toggle.
 */
void
MuteButtonCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;
	SAError         error;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 5, "Cannot perform mute/unmute operation."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	if (wd->is_muted == SATrue) {

		/* Turn sound on. */
		if (SDtAudioSetMuteState(wd->sa_stream, SAFalse, &error)) {

			/* Toggle mute status. */
			XtVaSetValues(wd->mute_button,
				      XmNshadowType, XmSHADOW_ETCHED_OUT,
				      NULL);
			SDtAuSetMessage(wd->footer_msg,
				catgets(msgCatalog, 3, 53, "Turn Sound Off"));
			XmUpdateDisplay(wd->toplevel);
			wd->is_muted = SAFalse;

		} else {

			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 4, 9, "Could not turn off audio muting."),
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
		}

	} else {

		/* Turn sound off. */
		if (SDtAudioSetMuteState(wd->sa_stream, SATrue, &error)) {

			/* Toggle mute status. */
			XtVaSetValues(wd->mute_button,
				      XmNshadowType, XmSHADOW_ETCHED_IN,
				      NULL);
			SDtAuSetMessage(wd->footer_msg,
				catgets(msgCatalog, 3, 23, "Turn Sound On"));
			XmUpdateDisplay(wd->toplevel);
			wd->is_muted = SATrue;

		} else {

			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 4, 10, "Could not turn on audio muting."),
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
		}
	}
}

/* Function	: ProgScaleChangedCb()
 * Objective	: Callback invoked when the play point scale in the minimized
 *		  window is reset.  The function sets the clip position
 *		  to the proper value, so that audio plays at a particular
 *		  point.
 */
void 
ProgScaleChangedCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;
	XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) callData;
	Dimension	w_width;
	short		wave_pos;

	if (wd == NULL)
		return;

	/* Stop the audio, if it is playing. */
	SDtAuStopAndWait(wd);

	/* Set the audio clip position, so if user presses play, it
	 * starts from the specified play point.
	 */
	wd->clip_pos = cbs->value;

	/* Update the play point of the wave window as well. */
	if (wd->wave_window != NULL) {

		XtVaGetValues(wd->wave_window,
			      XtNwidth, &w_width,
			      NULL);
		wave_pos = (short) (wd->clip_pos * w_width / 100);
		SDtAuDrawWaveCursor(wd, wave_pos);
	}
}

/* Function	: ProgScaleDragCb()
 * Objective	: Callback invoked when the progress bar in the minimized
 *		  window is dragged.  The function stops the current
 *		  audio file being played.
 */
void 
ProgScaleDragCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;

	/* Stop the audio file currently being played. */
	SDtAuStopAndWait(wd);
}

/* Function	: ChangeVolCb()
 * Objective	: Callback invoked when the user clicks on either the up
 *		  or down volume buttons.  The function increments/decrements
 *		  the volume everytime the button is pressed.
 */
void 
ChangeVolCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;
	int		level = wd->play_vol;
	SAError		error;
	char            buffer[1024];

	if (wd == NULL) {

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 6, "Cannot change volume."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Which widget was pressed - increase or decrease volume? */
	if (w == wd->up_vol) {

		/* If volume level is at maximum, just return. */
		if (level == VOL_MAX)
			return;
		else {
			/* Increment the volume, and if it goes over VOL_MAX,
			 * then reset it back to VOL_MAX.
			 */
			level += VOL_INCREMENT;
			if (level > VOL_MAX)
				level = VOL_MAX;
		}

	} else {

		/* If volume level is at minimum, just return. */
		if (level == VOL_MIN)
			return;
		else {
			/* Decrement the volume, and if it goes below VOL_MIN,
			 * then reset it back to VOL_MIN.
			 */
			level -= VOL_INCREMENT;
			if (level < VOL_MIN)
				level = VOL_MIN;
		}
	}

	/* Set the device volume. */
	XA_LOCK(wd, "ChangeVolCb");
	if (SDtAudioSetPlayVol(wd->sa_stream, level, &error)) {
		wd->play_vol = level;
		XtVaSetValues(wd->vol_gauge, XtNvalue, wd->play_vol, NULL);
		XmUpdateDisplay(wd->toplevel);
		XA_UNLOCK(wd, "ChangeVolCb");

	} else {
		/* 
		   Prabhat : Moved this from being invoked after
		   AlertPrompt() as positiion affects timing, locking
		*/
		XA_UNLOCK(wd, "ChangeVolCb");
		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 6, "Cannot change volume."),
			error);
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}
}

/* Function	: PlayCb()
 * Objective	: Callback invoked when the play button is pressed.
 *		  Function starts playing the audio file.
 */
void 
PlayCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 7, "Cannot play audio file."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	if (wd->play_state == STATE_RECORD)
		return;

	XA_LOCK(wd, "PlayCb");
	if (wd->safile == NULL) {

		XA_UNLOCK(wd, "PlayCb()\n");
		return;
	}
	XA_UNLOCK(wd, "PlayCb()\n");

	AT_MUTEX_LOCK(wd, "PlayCb");
	if (wd->play_state == STATE_PLAY) {

		AT_MUTEX_UNLOCK(wd, "PlayCb");
		xa_stop_audio(wd);

	} else {

		AT_MUTEX_UNLOCK(wd, "PlayCb");
		xa_play_audio(wd);
	}
}

/* Function	: RecordCb()
 * Objective	: Callback invoked when the record button is pressed.
 */
void 
RecordCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;

	if (wd == NULL) {

		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	if (wd->play_state == STATE_PLAY)
		return;

	AT_MUTEX_LOCK(wd, "RecordCb");
	if (wd->play_state == STATE_RECORD) {

		AT_MUTEX_UNLOCK(wd, "RecordCb");
		xa_stop_audio(wd);

		/* Turn on Save As menu item */
		XtSetSensitive(wd->menu_items->SaveAsItem, True);

	} else {

		AT_MUTEX_UNLOCK(wd, "RecordCb");
		if (!wd->options->useDefaultRecValues) {
		
			/* User set the component's behavior to
			 * query before recording.  If user cancels
			 * query, drop out of function and don't
			 * record.
			 */
			if (!SDtAuGetRecordValues(wd))
				return;
		}
		xa_record_audio(wd);
	}
}

/* Function	: BlinkLightCb()
 * Objective	: Callback timer function that toggles the record light
 *		  while user is recording audio.
 */
void 
BlinkLightCb(XtPointer clientData, XtIntervalId * id)
{
	WindowData     *wd = (WindowData *) clientData;
	if (wd->blink_off == True) {

		wd->blink_off = False;
		dtb_set_label_pixmaps(wd->record_light,
				wd->rec_blink_pixmap, wd->ins_blink_pixmap);

	} else {

		wd->blink_off = True;
		XtVaSetValues(wd->record_light,
			      XmNlabelPixmap, wd->rec_blink_off_pixmap,
			      NULL);
	}

	wd->record_time_out = 
		XtAppAddTimeOut(appContext, 500,
				(XtTimerCallbackProc) BlinkLightCb, wd);
}

/* Function	: SaveAsCb()
 * Objective	: Callback invoked when the user selects the File->Save As
 * 		  menu item.  The callback creates, if necessary, the 
 *		  Save As dialog window, and displays it.
 */
void 
SaveAsCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;

	/* Check our data structure first. */
	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 8, "Cannot bring up \"Save As\" dialog."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Create the save as dialog, if required. */
	_DtTurnOnHourGlass(wd->toplevel);
	if (wd->saveas_dlog == NULL) {

		if (!SDtAuCreateSaveAsDlog(wd)) {
			_DtTurnOffHourGlass(wd->toplevel);
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 3, 8, "Cannot bring up \"Save As\" dialog."),
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			return;
		}
	}

	/* Set the format menu to the default recording format. */
	if (wd->options->recordingFormat == SAWavType)
		XtVaSetValues(wd->format_menu, 
			      XmNmenuHistory, wd->menu_items->WavItem,
			      NULL);

	else if (wd->options->recordingFormat == SAAiffType)
		XtVaSetValues(wd->format_menu, 
			      XmNmenuHistory, wd->menu_items->AiffItem,
			      NULL);
	else
		XtVaSetValues(wd->format_menu, 
			      XmNmenuHistory, wd->menu_items->AuItem,
			      NULL);

	/* Manage the dialog. */
	XtManageChild(wd->saveas_dlog);
	_DtTurnOffHourGlass(wd->toplevel);
}

/* Function	: OptionsCb()
 * Objective	: Callback invoked when any of the Options menu items (in
 *		  the menu bar is selected.
 */
void
OptionsCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;
	Widget		menu_item;
	XmString        label_str;
	char	       *label;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 9, "Cannot bring up \"Options\" dialog."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	_DtTurnOnHourGlass(wd->toplevel);
	if (wd->options_dlog == NULL)

		if (!SDtAuCreateOptionsDlog(wd)) {
			_DtTurnOffHourGlass(wd->toplevel);
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 3, 9, "Cannot bring up \"Options\" dialog."),
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			return;
	}

	/* Set the category options menu appropriately, and
	 * invoke the callback so that the proper pane is displayed.
	 */
	XtVaGetValues(w, XmNlabelString, &label_str, NULL);
	XmStringGetLtoR(label_str, XmFONTLIST_DEFAULT_TAG, &label);
	if (strcmp(label, catgets(msgCatalog, 1, 17, "Play...")) == 0)
		menu_item = wd->menu_items->PlayItem;
	else if (strcmp(label, catgets(msgCatalog, 1, 18, "Record...")) == 0)
		menu_item = wd->menu_items->RecordItem;
	else if (strcmp(label, catgets(msgCatalog, 1, 73, "Devices...")) == 0)
		menu_item = wd->menu_items->DevicesItem;
	else
		menu_item = wd->menu_items->StartupItem;
	XtVaSetValues(wd->options_category,
		      XmNmenuHistory, menu_item,
		      NULL);
	_DtTurnOffHourGlass(wd->toplevel);
	XtCallCallbacks(menu_item, XmNactivateCallback, NULL);
	XmStringFree(label_str);
	XtFree(label);
	XtManageChild(wd->options_dlog);
}

/* Save a sound file  */
Boolean
SaveAsFile(WindowData * wd, char *fileName)
{
	char            ibuf[BUFSIZ], obuf[BUFSIZ * 2];
	char            format_str[512];
	SAError         error;
	SAFile          infile, outfile;
	SAFile         *safile = wd->safile;
	int             nbytes_read = 1, nbytes_written;
	int             ibufsize = 0, obufsize = 0, total_written = 0;
	XmString        xmstring;

	/* Make sure file specified is not a directory and that we have
	 * permission to write to this directory/file.
	 */
	if (!CanWrite(wd->toplevel, fileName))
		return False;

	if (SAFileOpen(wd->name, &infile, &error) == SAFalse) {

		char            buffer[1024];

		sprintf(buffer, "%s\n%s",
			catgets(msgCatalog, 4, 15, "Error saving file."),
			error.description);
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, 
			    catgets(msgCatalog, 1, 3, "Audio - Save As"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1, 
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return False;
	}

	/*
	 * Get the format to save out to from the Format menu.
	 */
	_DtTurnOnHourGlass(wd->saveas_dlog);
	_DtTurnOnHourGlass(wd->toplevel);
	XtVaGetValues(XmOptionButtonGadget(wd->format_menu),
		      XmNlabelString, &xmstring, NULL);
	ConvertCompoundToChar(xmstring, format_str);
	if (strcmp(format_str, catgets(msgCatalog, 2, 64, "AU")) == 0)
		outfile.filetype = SASunType;
	else if (strcmp(format_str, catgets(msgCatalog, 2, 65, "WAV")) == 0)
		outfile.filetype = SAWavType;
	else
		outfile.filetype = SAAiffType;
	XmStringFree(xmstring);

	/* Based on the source file and file format to save out to, generate
	 * characteristics of the file to write to.
	 */
	if ((SAConvertFormat(infile.format.encoding, 
			     infile.format.sample_size,
			     infile.format.channels, 
			     infile.format.samples_per_unit,
			     outfile.filetype, 
			     &outfile.format, &error)) == SAFalse) {

		_DtTurnOffHourGlass(wd->saveas_dlog);
		_DtTurnOffHourGlass(wd->toplevel);
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, 
			    catgets(msgCatalog, 3, 59, "Audio - Error"),
			    DIALOG_TEXT, 
			    catgets(msgCatalog, 3, 61, 
				    "Format conversion failed."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return False;
	}

	/* Copy the rest of the file attributes. */
	outfile.format.sample_rate = infile.format.sample_rate;
	outfile.format.samples_per_unit = infile.format.samples_per_unit;
	outfile.data_size = infile.data_size;
	outfile.header_size = infile.header_size;

	/* Create the new file and write out the header portion. */
	if (SAFileCreate(fileName, &outfile, &error) == SAFalse) {

		_DtTurnOffHourGlass(wd->saveas_dlog);
		_DtTurnOffHourGlass(wd->toplevel);
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, 
			    catgets(msgCatalog, 1, 3, "Audio - Save As"),
			    DIALOG_TEXT, 
			    catgets(msgCatalog, 4, 15, 
				    "Error saving file."),
			    BUTTON_IDENT, ANSWER_ACTION_1, 
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return False;
	}

	/* Convert & write out the data. */
	ibufsize = sizeof(ibuf);
	while (nbytes_read > 0) {

		SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
		if (nbytes_read > 0) {

			SAConvertData(&infile.format, &outfile.format, 
				      ibuf, nbytes_read, obuf, 
				      &obufsize, &error);
			SAFileWrite(&outfile, obuf, obufsize, 
				    &nbytes_written, &error);
			if (nbytes_written != obufsize) {

				_DtTurnOffHourGlass(wd->saveas_dlog);
				_DtTurnOffHourGlass(wd->toplevel);
				AlertPrompt(wd->toplevel,
					    DIALOG_TYPE, XmDIALOG_WARNING,
					    DIALOG_TITLE, 
					    catgets(msgCatalog, 1, 3,
						    "Audio - Save As"),
					    DIALOG_TEXT, 
					    catgets(msgCatalog, 4, 15,
						    "Error saving file."),
					    BUTTON_IDENT, ANSWER_ACTION_1,
						catgets(msgCatalog, 2, 62, 
						    "Continue"),
					    NULL);
				return False;
			} else if (nbytes_written == 0)
				break;
			else
				total_written += nbytes_written;
		}
	}

	(void) SAFileClose(&infile, &error);
	(void) SAFileClose(&outfile, &error);

	/* Remove the temporary file, if this is the one that we are 
	 * saving.
	 */
	if (wd->audio_modified && temp_rec_file) {
		unlink(temp_rec_file);
		free(temp_rec_file);
		temp_rec_file = (char *) NULL;
		wd->audio_modified = False;
	}

	/* Remove the temp_buffer file, in case we loaded from buffer. */
	if (temp_buffer_name) {
		unlink(temp_buffer_name);
		free(temp_buffer_name);
		temp_buffer_name = (char *) NULL;
	}
	wd->wait_for_dialog = False;
	_DtTurnOffHourGlass(wd->saveas_dlog);
	_DtTurnOffHourGlass(wd->toplevel);
	return True;
}

/* Function	: SaveAsOkCb()
 * Objective	: Callback invoked when the user has selected
 *		  the OK button from a Save As file selection box.
 */
void 
SaveAsOkCb(Widget w, XtPointer clientData, XtPointer callData)
{
	char            fileName[MAXPATHLEN];
	SAError		error;
	WindowData     *wd = (WindowData *) clientData;
	XmFileSelectionBoxCallbackStruct *cbs = callData;

	if (wd == NULL || wd->saveas_dlog == NULL) {

		AlertPrompt(XtParent(w),
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 2,
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 11, "Internal error: sdtaudio structure is empty."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	ConvertCompoundToChar(cbs->value, fileName);
	if (SaveAsFile(wd, fileName)) {
		/* Unmanage the SaveAs dialog. */
		XtUnmanageChild(wd->saveas_dlog);

		/* Discard the old safile, since new safile may be
		 * of different type.
		 */
		(void) SAFileClose(wd->safile, &error);
		free(wd->safile);
		wd->safile = (SAFile *) calloc(1, sizeof(SAFile));
		SAFileOpen(fileName, wd->safile, &error);

		/* Update other widgets with new info. */
		SetTitle(wd->toplevel, fileName);
		if (wd->name)
			XtFree(wd->name);
		wd->name = XtNewString(fileName);
		XtSetSensitive(wd->menu_items->InfoItem, True);
		if (wd->info_dlog && XtIsManaged(wd->info_dlog))
			SDtAuUpdateInfoDlog(wd);
		SDtAuUpdateTimeLabels(wd);
		XmScaleSetValue(wd->prog_scale, 0);
		wd->clip_pos = 0;

		/* Free the current wave pixmap. */
		if (wd->wave_pixmap != NULL) {
			XFreePixmap(XtDisplay(wd->wave_window), 
				    wd->wave_pixmap);
			wd->wave_pixmap = NULL;
		}
		if (wd->show_wave == TRUE) {
			SDtAuUpdateWaveWin(wd);
		}

		XmUpdateDisplay(wd->toplevel);
	}
}

/* Function	: SaveAsCancelCb()
 * Objective	: Callback invoked when the cancel button in a Save As
 *		  file selection box is pressed.  This function dismisses
 *		  the save as dialog, and resets the wait_for_dialog flag.
 */
void 
SaveAsCancelCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;

	if (wd == NULL || wd->saveas_dlog == NULL) {

		AlertPrompt(XtParent(w),
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 11, "Internal error: Cannot dismiss Save As dialog."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}
	wd->wait_for_dialog = False;
	XtUnmanageChild(wd->saveas_dlog);
}

/* Function	: GenericCancelCb
 * Objective	: Generic callback that dismisses a window.
 */
void
GenericCancelCb(Widget w, XtPointer clientData, XtPointer callData)
{
	Widget     window = (Widget) clientData;

	if (window == NULL) {

		AlertPrompt(XtParent(w),
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 9, "Internal error: cannot determine ID of window to be dismissed."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}
	XtUnmanageChild(window);
}

/* Function	: OptionsSetOkCb()
 * Objective	: Callback invoked when the user selects the OK or Apply
 * 		  buttons from the Options dialog.
 */
void
OptionsSetCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 11, "Cannot save audio options."));
		AlertPrompt(XtParent(w),
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Save the options. */
	SDtAuSaveOptions(wd);
	wd->options_changed = False;

	/* If the OK button was pressed, then dismiss the options dialog. */
	if (w == wd->options_ok)
		XtUnmanageChild(wd->options_dlog);
}

/* Function	: OptionsChangedCb()
 * Objective	: Callback invoked whenever something in the options dialog
 *		  was changed by the user.  The callback sets the modified
 *		  flag to true.
 */
void 
OptionsChangedCb(Widget w, XtPointer clientData, XtPointer callData)
{
	Boolean *flag = (Boolean *) clientData;

	/* Set the flag to True. */
	*flag = True;
}

/* Function	: OptionsCancelCb()
 * Objective	: Callback invoked when cancel is pressed in the options
 *		  dialog window.  The options dialog window is dismissed.
 */
void
OptionsCancelCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData   *wd = (WindowData *) clientData;

	if (wd == NULL || wd->options_dlog == NULL) {

		AlertPrompt(XtParent(w),
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 10, 
					         "Internal error: cannot determine ID of window to be dismissed."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* If the user changed options, and is now canceling, ask the
	 * user if they want to save their options, just in case.
	 */
	if (wd->options_changed) {

		int            answer;
		char           buffer[1024];

		sprintf(buffer, "%s%s",
			catgets(msgCatalog, 4, 13, "You have unsaved changes.\n\n"),
			catgets(msgCatalog, 4, 14, "Do you want to save changes before closing?"));
		answer = AlertPrompt(wd->toplevel,
				     DIALOG_TYPE, XmDIALOG_QUESTION,
				     DIALOG_TITLE, 
				     catgets(msgCatalog, 3, 56, "Audio - Options"),
				     DIALOG_STYLE, 
				     XmDIALOG_FULL_APPLICATION_MODAL,
				     DIALOG_TEXT, 
				     buffer,
				     BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 3, 15,"Yes"),
				     BUTTON_IDENT, ANSWER_ACTION_2, 
					catgets(msgCatalog, 3, 16,"No"),
				     BUTTON_IDENT, ANSWER_CANCEL, 
					catgets(msgCatalog, 2, 40, "Cancel"),
				     NULL);
		/* 
		 * If anser is:
		 *	Yes	- save changes and dismiss the options window
		 *	No	- don't save, reset and dismiss the options 
		 * 		  window.
		 *	Cancel	- don't save, but leave options window up.
		 */
		switch(answer) {

			case ANSWER_ACTION_1:		/* Yes. */
				SDtAuSaveOptions(wd);
				break;

			case ANSWER_ACTION_2:		/* No. */
				SDtAuResetOptions(wd);
				break;

			default:
				return;			/* Cancel. */

		}
	} 

	/* Dismiss the options window. */
	wd->options_changed = False;
	XtUnmanageChild(wd->options_dlog);
}

/* Function	: OptionsResetCb()
 * Objective	: Callback invoked when reset is pressed in the options
 *		  dialog window.  All the option window's panes will be 
 *		  set to the original initial state.
 */
void
OptionsResetCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData   *wd = (WindowData *) clientData;

	if (wd == NULL || wd->options_dlog == NULL) {

		AlertPrompt(XtParent(w),
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 42, 
					         "Cannot reset values in \"Options\" window."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* If the user didn't make any changes, just return. */
	if (!wd->options_changed)
		return;

	/* Need to do a reset. */
	SDtAuResetOptions(wd);
	wd->options_changed = False;
}

/* Function	: InfoCb
 * Objective	: Callback invoked when the File->Get Info menu item
 *		  is selected.  This function brings up the Info dialog,
 *		  which contains data about the audio file loaded.
 */
void 
InfoCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;

	/* Check our data structure first. */
	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 32, "Cannot bring up \"File Information\" dialog."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Create the Info dialog, if necessary. */
	_DtTurnOnHourGlass(wd->toplevel);
	if (wd->info_dlog == NULL)

		if (!SDtAuCreateInfoDlog(wd)) {
			_DtTurnOffHourGlass(wd->toplevel);
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 3, 32, "Cannot bring up \"File Information\" dialog."),
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			return;
		}
	SDtAuUpdateInfoDlog(wd);

	/* Manage the window. */
	XtManageChild(wd->info_dlog);
	_DtTurnOffHourGlass(wd->toplevel);
}

/* Function	: LevelMeterRefreshCb()
 * Objective	: Callback that redraws the level meter amplitude bar
 *		  graph, when the wave window is shown.
 */
void 
LevelMeterRefreshCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;
	int             val;

	XmDrawingAreaCallbackStruct *cbs = 
			(XmDrawingAreaCallbackStruct *) callData;

	if (wd == NULL || cbs == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 34, "Cannot update level meter."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}
	if (wd->drawEnvInitialized == False)
		SDtAuInitDrawables(wd);

	SDtAuUpdateLevelMeter(wd);
}

/*
 * Function	: WaveWindowRefreshCb() 
 * Objective	: Callback invoked whenever the wave window gets an 
 *		  expose event.
 */
void 
WaveWindowRefreshCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;
	int             val;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 60, "Cannot update wave window."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* If audio is recording, just return - can't set playpoint. */
	if (wd->play_state == STATE_RECORD)
		return;

	/* Wave window is not showing. */
	if (wd->show_wave == False)
		return;

	if (wd->drawEnvInitialized == False)
		SDtAuInitDrawables(wd);

	SDtAuUpdateWaveWin(wd);
}

/* Function	: SetRecordValuesCb().
 * Objective	: Callback invoked when user dismisses the Set recording
 *		  values dialog.  If the OK button was pressed, this
 * 		  callback sets the audio format and quality that will
 *		  be used when recording.  The wait_for_dilaog variable
 *		  is also re-set to False, so that sdtaudio continues.
 */
void
SetRecordValuesCb(Widget w, XtPointer client_data, XtPointer cbd)
{
	WindowData     *wd = (WindowData *) client_data;
	Widget		menu;
	XmString	label_str;
	char	       *buffer;
	XmAnyCallbackStruct   *cbs = (XmAnyCallbackStruct *) cbd;

	/* Set to false, so program continues and doesn't wait for
	 * user input.
	 */
	wd->wait_for_dialog = False;

	/* Did user select cancel button? */
	if (cbs->reason == XmCR_CANCEL) {
		XtUnmanageChild(wd->recval_dlog);
		return;
	}

	/* Set-up quality. */
	XtVaGetValues(wd->recval_quality, XmNmenuHistory, &menu, NULL);
	XtVaGetValues(menu, XmNlabelString, &label_str, NULL);
	buffer = (char *) malloc(XmStringLength(label_str) + 5);
	ConvertCompoundToChar(label_str, buffer);
	if (strcmp(buffer, catgets(msgCatalog, 2, 33, "CD")) == 0)
		wd->options->recordingQuality = SDTA_CD_QUALITY;
	else if (strcmp(buffer, catgets(msgCatalog, 2, 68, "DAT")) == 0)
		wd->options->recordingQuality = SDTA_DAT_QUALITY;
	else
		wd->options->recordingQuality = SDTA_VOICE_QUALITY;
	free(buffer);

	/* Set-up format. */
	XtVaGetValues(wd->recval_format, XmNmenuHistory, &menu, NULL);
	XtVaGetValues(menu, XmNlabelString, &label_str, NULL);
	buffer = (char *) malloc(XmStringLength(label_str) + 5);
	ConvertCompoundToChar(label_str, buffer);
	if (strcmp(buffer, catgets(msgCatalog, 2, 65, "WAV")) == 0)
		wd->options->recordingFormat = SAWavType;
	else if (strcmp(buffer, catgets(msgCatalog, 2, 66, "AIFF")) == 0)
		wd->options->recordingFormat = SAAiffType;
	else
		wd->options->recordingFormat = SASunType;
	XmStringFree(label_str);
	free(buffer);

	wd->recval_set = True;
	XtUnmanageChild(wd->recval_dlog);
}

/* Function	: DrawLocationCb().
 * Objective	: Callback invoked when user has mouse pressed down
 * 		  while inside the wave window, and moves the mouse
 *		  in order to make a selection within the wave window.
 *		  The callback draws the selected area of the wave
 *		  wave window in reverse video, so that the user
 *		  can see what section was selected.
 */
void
DrawLocationCb(Widget w, XtPointer clientData, XEvent * ev,
	     Boolean * cont_to_disp)
{
	WindowData     *wd = (WindowData *) clientData;
	XMotionEvent   *event = (XMotionEvent *) ev;
	char            buf[100];
	XmString        label_str = NULL;
	float           tot_seconds, disp_seconds, scale_factor;
	int             disp_minutes;
	Dimension       w_width;
	SAFile         *safile = wd->safile;

	if (wd == NULL) {

		char            buffer[1024];

		sprintf(buffer, "%s\n%s",
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
		  catgets(msgCatalog, 3, 60, "Cannot update wave window."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Continue handling this event. */
	*cont_to_disp = False;

	/* If audio is recording, just return - can't set playpoint. */
	if (wd->play_state == STATE_RECORD)
		return;

	/* No file - just return. */
	if (safile == NULL)
		return;

	/* Don't create selection if we're not supposed to track the
	 * event; for example, the mouse is moved, but outside of
	 * the wave window.
	 */
	if (!wd->trackWaveMotion)
		return;

	/* Get the wave window's width. */
	XtVaGetValues(wd->wave_window,
		      XtNwidth, &w_width,
		      NULL);

	/* DANGER WILL ROBINSON!!!!   Calculate the coordinates,
	 * based on the time within the audio file.
	 */
	scale_factor = (float) (event->x - 3) / (float) (w_width - 6);
	tot_seconds = ((float) (SAFileGetDataSize(wd->safile)) * 
		       (float) (safile->format.samples_per_unit) *
			       scale_factor) /
		      ((float) safile->format.channels * 
			       safile->format.sample_size *
			       safile->format.sample_rate);
	disp_minutes = (int) tot_seconds / 60;
	disp_seconds = tot_seconds - (float) (disp_minutes * 60);

	/* Update the time information, so that it can be properly
	 * displayed in the wave window.
	 */
	sprintf(buf, "%d:%02.2f", disp_minutes, disp_seconds);
	strcpy(wd->old_pos_str, wd->new_pos_str);
	strcpy(wd->new_pos_str, buf);

	/* Erase the old wave location line and time text, since the user 
	 * moved the pointer to modify the selection.
	 */
	if (wd->old_x > 2 && wd->old_x <= (w_width - 3))
		SDtAuWaveLocLine(wd->old_x, wd);
	
	/* Prabhat : Fake an expose event to clean wave window */
        XtCallCallbacks(wd->wave_window, XmNexposeCallback, wd);
	SDtAuWaveLocTime(event->x, wd);

#ifdef SELECTION
	/* Update the selection, if appropriate, i.e., button still pressed,
	 * and mouse is moving.  Update the location line in the process.
	 */
	if (wd->select_start == True && (event->state & Button1Mask)) {

		SDtAuChangeSel(event->x, wd->sel_end_x, wd->sel_start_x, wd);
		wd->sel_end_x = event->x;
	}
#endif
	SDtAuWaveLocLine(event->x, wd);
	if (event->state & Button1Mask)
		SDtAuDrawWaveCursor(wd, event->x);

	/* Track the positions, to be used when erasing the location line
	 * in the next iteration.
	 */
	wd->old_x = event->x;
	wd->old_y = event->y;
}

/* Function	: EnterWaveWindowCb().
 * Objective	: Callback invoked when user drags mouse pointer inside
 *		  the wave window.  Assuming that an audio file is loaded,
 *		  when the user enters the wave window, a location pointer
 *		  (a thin line) tells the user where (in seconds) they
 *		  are in the audio file.  The user can move the mouse up
 *		  and down the wave window in order to set a play point
 *		  at a specific time, or set a specific play interval
 * 		  selection.
 */
void 
EnterWaveWindowCb(Widget w, XtPointer clientData, XEvent *ev,
		  Boolean *cont_to_disp)
{
	WindowData       *wd = (WindowData *) clientData;
	XCrossingEvent   *event = (XCrossingEvent *) ev;
	Dimension         w_width;

	if ((wd == NULL) || (event == NULL)) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 60, "Cannot update wave window."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Continue to handle this event. */
	*cont_to_disp = False;

	/* If audio is recording, just return - can't set playpoint. */
	if (wd->play_state == STATE_RECORD)
		return;

	/* No file loaded - no need to draw a location pointer. */
	if (wd->safile == NULL)
		return;

	/* Track the pointer as the user moves up and down the length
	 * of the audio file.  The old_x and old_y values will be used
	 * to erase the line if the user changes position, or if the user
	 * leaves the wave window.
	 */
	wd->trackWaveMotion = True;
	wd->old_x = event->x;
	wd->old_y = event->y;

	/* Don't track if very close to border, since location line may be
	 * obscured by border width.
	 */
	XtVaGetValues(wd->wave_window,
		      XtNwidth, &w_width,
		      NULL);
	if (event->x < 3 || event->x > (w_width - 3))
		return;

	/* Draw the line and the time value. */
	strcpy(wd->new_pos_str, "");
	strcpy(wd->old_pos_str, "");
	SDtAuWaveLocLine(event->x, wd);
	SDtAuWaveLocTime(event->x, wd);
}

/* Function	: LeaveWaveWindowCb().
 * Objective	: Callback invoked when user drags mouse pointer out of
 *		  the wave window.  The callback should remove the location
 *		  line and location time from the screen.  This callback
 *		  does not remove any selection that the user has made - the
 *		  selection should still be displayed in the wave window.
 */
void
LeaveWaveWindowCb(Widget w, XtPointer clientData, XEvent * ev,
		  Boolean * cont_to_disp)
{
	WindowData     *wd = (WindowData *) clientData;
	XCrossingEvent *event = (XCrossingEvent *) ev;
	Dimension       w_width, w_height;

	/* Check data structure. */
	if ((wd == NULL) || (event == NULL)) {

		char            buffer[1024];

		sprintf(buffer, "%s\n%s",
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
		  catgets(msgCatalog, 3, 60, "Cannot update wave window."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* No file loaded - no drawable to remove from the wave window. */
	if (wd->safile == NULL)
		return;

	/* Continue to handle this event. */
	*cont_to_disp = False;

	/* Get the dimensions of the wave window. */
	XtVaGetValues(wd->wave_window,
		      XtNwidth, &w_width,
		      XtNheight, &w_height,
		      NULL);

	/* Remove the location line. */
	if (wd->old_x > 2 && wd->old_x < (w_width - 3))
		SDtAuWaveLocLine(wd->old_x, wd);

	/* Redraw the wave window. */
	XFillRectangle(XtDisplay(wd->wave_window),
		       XtWindow(wd->wave_window),
		       wd->waveReverseGC,
		       0, w_height - ww_font_height,
		       w_width, ww_font_height);

	/* Stop tracking the location line, and reset old values. */
	wd->trackWaveMotion = False;
	wd->old_x = 0;
	wd->old_y = 0;
}

/* Function	: ButtonDownWWCb().
 * Objective	: Callback invoked when user first presses the mouse button
 *		  and leaves it pressed.  The callback should start the
 *		  selection process, i.e., selection of a specific part
 *		  of the audio file.
 */
void
ButtonDownWWCb(Widget w, XtPointer clientData, XEvent * ev,
	       Boolean * cont_to_disp)
{
	XButtonEvent   *event = (XButtonEvent *) ev;
	WindowData     *wd = (WindowData *) clientData;
	Dimension	w_width;

	/* Check data structures. */
	if (wd == NULL) {

		char            buffer[1024];

		sprintf(buffer, "%s\n%s",
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
		  catgets(msgCatalog, 3, 60, "Cannot update wave window."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Continue to handle this event. */
	*cont_to_disp = False;

	/* If audio is recording, just return - can't set playpoint. */
	if (wd->play_state == STATE_RECORD)
		return;

	/* No file loaded, so nothing to select. */
	if (wd->safile == NULL)	/* no current file */
		return;

	/* If the audio file is being played, stop it.  The user 
	 * clicked in the wave window, and reset the play point.
	 * The user will have to explicitly hit play in order to
	 * play the audio file from the selected play point.
	 */
	SDtAuStopAndWait(wd);

#ifdef SELECTION
	/* Set the selection flag to true, to signal that the user has
	 * started selection.
	 */
	if (event->button == Button1)
		wd->select_start = True;
	else
		return;

	/* Track the point selected, and update the wave window view. */
	wd->sel_start_x = wd->sel_end_x = event->x;
	SDtAuUpdateWaveWin(wd);
	SDtAuDrawWaveCursor(wd, event->x);

	/* Button down implies user wants new play point. */
	XtVaGetValues(wd->wave_window,
		      XtNwidth, &w_width,
		      NULL);
	wd->clip_pos = (int) (event->x * 100 / w_width);
#endif
}

void 
ConfigNotifyWWCb(Widget w, XtPointer clientData, XEvent * ev,
		 Boolean * cont_to_disp)
{
	XButtonEvent   *event = (XButtonEvent *) ev;
	WindowData     *wd = (WindowData *) clientData;

	if (wd->wave_pixmap != NULL) {
		XFreePixmap(XtDisplay(wd->wave_window), wd->wave_pixmap);
		wd->wave_pixmap = NULL;
	}
	SDtAuUpdateWaveWin(wd);
}

void 
ButtonUpWWCb(Widget w, XtPointer clientData, XEvent * ev,
	     Boolean * cont_to_disp)
{
	XButtonEvent   *event = (XButtonEvent *) ev;
	WindowData     *wd = (WindowData *) clientData;
	int             temp_x;
	Dimension	w_width;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 60, "Cannot update wave window."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59,
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	if (wd->safile == NULL)	/* no current file */
		return;

	/* We'll handle this event. */
	*cont_to_disp = False;

#ifdef SELECTION
	wd->sel_end_x = event->x;
	if (wd->sel_end_x == wd->sel_start_x) {

		/* just a click */
		wd->select_start = False;
		wd->selection_made = False;
		wd->sel_end_x = wd->sel_start_x = 0;

		/* Erase the selection line, since it is on the
		 * same point anyway.
		 */
		SDtAuWaveLocLine(event->x, wd);
		return;
	}
	if (wd->sel_end_x < wd->sel_start_x) {
		temp_x = wd->sel_end_x;
		wd->sel_end_x = wd->sel_start_x;
		wd->sel_start_x = temp_x;

		/* Move the play point cursor to the start of the
		 * selection.
		 */
		SDtAuDrawWaveCursor(wd, wd->sel_start_x);

		/* Get the width of the wave window. */
		XtVaGetValues(wd->wave_window,
			      XtNwidth, &w_width,
		 	      NULL);

		/* Reset play point. */
		wd->clip_pos = (int) (wd->sel_start_x * 100 / w_width);
	}
	wd->select_start = False;
	wd->selection_made = True;

#else
	/* Set the wave window play point. */
	SDtAuDrawWaveCursor(wd, event->x);

	/* Get the width of the wave window. */
	XtVaGetValues(wd->wave_window,
		      XtNwidth, &w_width,
	 	      NULL);

	/* Reset clip position/play point */
	wd->clip_pos = (int) (event->x * 100 / w_width);
	XmScaleSetValue(wd->prog_scale, wd->clip_pos);
#endif

	/* SDtAuDrawSel(wd); */
	SDtAuUpdateWaveWin(wd);
	return;
}

/* Function	:	LeaveButtonCb()
 * Objective	:	Remove the footer message when the cursor goes
 *			out of the button's area.
 */
void 
LeaveButtonCb(Widget w, XtPointer clientData, XEvent * event,
	      Boolean *cont_to_disp)
{
	WindowData     *wd = (WindowData *) clientData;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 19, "Cannot update footer with button information."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Set the flag to false, since we don't need to display
	 * what the button does anymore to the user.
	 */
	*cont_to_disp = False;
	SDtAuSetMessage(wd->footer_msg, "");
}

/* Function	: OptionsDlogPaneCb
 * Objective	: Callback invoked when the user selects a category from
 *		  the options window.  The category selection determines
 *		  what pane is displayed to the user.
 */
void
OptionsDlogPaneCb(Widget w, XtPointer clientData, XtPointer callData)
{
	WindowData     *wd = (WindowData *) clientData;
	Dimension       height, mheight;
	Widget          manage, unmanage[3];
	register int    n;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 18, "Cannot switch options pane."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	if (w == wd->menu_items->StartupItem) {	

		/* Manage the start-up pane, unmanage the rest. */
		manage = wd->opt_startup_pane;
		n = 0;
		unmanage[n] = wd->opt_play_pane; ++n;
		unmanage[n] = wd->opt_record_pane; ++n;
		unmanage[n] = wd->opt_devices_pane; ++n;

	} else if (w == wd->menu_items->PlayItem) {

		/* Manage the play pane, unmanage the rest. */
		manage = wd->opt_play_pane;
		n = 0;
		unmanage[n] = wd->opt_startup_pane; ++n;
		unmanage[n] = wd->opt_record_pane; ++n;
		unmanage[n] = wd->opt_devices_pane; ++n;

	} else if (w == wd->menu_items->RecordItem) {

		/* Manage the record pane, unmanage the rest. */
		manage = wd->opt_record_pane;
		n = 0;
		unmanage[n] = wd->opt_startup_pane; ++n;
		unmanage[n] = wd->opt_play_pane; ++n;
		unmanage[n] = wd->opt_devices_pane; ++n;

	} else if (w == wd->menu_items->DevicesItem) {

		/* Manage the devices pane, unmanage the rest. */
		manage = wd->opt_devices_pane;
		n = 0;
		unmanage[n] = wd->opt_startup_pane; ++n;
		unmanage[n] = wd->opt_play_pane; ++n;
		unmanage[n] = wd->opt_record_pane; ++n;
	}
	XtUnmanageChildren(unmanage, n);
	XtManageChild(manage);
	XtVaSetValues(wd->options_cmd_pane,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, manage,
		      XmNtopOffset, 15,
		      NULL);
	XtManageChild(wd->options_form);
	XmUpdateDisplay(wd->toplevel);
}

/*
 * Function	:	EnterButtonCb()
 *
 * Objective	:	Xt event handler function that displays a text 
 *			description of the button's function in the 
 *			sdtaudio footer whenever the pointer goes inside
 * 			the button.
 */
void 
EnterButtonCb(Widget w, XtPointer clientData, XEvent *event,
	      Boolean *cont_to_disp)
{
	WindowData     *wd = (WindowData *) clientData;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 19, "Cannot update footer with button information."));
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, buffer,
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* We'll handle this event */
	*cont_to_disp = False;

	if (w == wd->play_button) {
		if (wd->play_state == STATE_PLAY)
			SDtAuSetMessage(wd->footer_msg,
				   catgets(msgCatalog, 3, 24, "Stop Playing"));
		else
			SDtAuSetMessage(wd->footer_msg,
				   catgets(msgCatalog, 3, 22, "Play Audio"));
	} else if (w == wd->mute_button) {

		if (wd->is_muted)
			SDtAuSetMessage(wd->footer_msg,
				catgets(msgCatalog, 3, 23, "Turn Sound On"));
		else
			SDtAuSetMessage(wd->footer_msg,
				catgets(msgCatalog, 3, 53, "Turn Sound Off"));
	} else if (w == wd->bwd_button)
		SDtAuSetMessage(wd->footer_msg,
			   catgets(msgCatalog, 3, 25, "Go to Start"));
	else if (w == wd->fwd_button)
		SDtAuSetMessage(wd->footer_msg,
			   catgets(msgCatalog, 3, 26, "Go to End"));
	else if (w == wd->record_button) {
		if (wd->play_state == STATE_RECORD)
			SDtAuSetMessage(wd->footer_msg,
				   catgets(msgCatalog, 3, 54,
					   "Stop Recording"));
		else
			SDtAuSetMessage(wd->footer_msg,
				   catgets(msgCatalog, 3, 27, "Record Audio"));
	} else if (w == wd->up_vol)
		SDtAuSetMessage(wd->footer_msg,
			   catgets(msgCatalog, 3, 28, "Increase Volume"));
	else if (w == wd->down_vol)
		SDtAuSetMessage(wd->footer_msg,
			   catgets(msgCatalog, 3, 29, "Decrease Volume"));
	else if (w == wd->wave_button) {
		if (wd->show_wave)
			SDtAuSetMessage(wd->footer_msg,
				   catgets(msgCatalog, 3, 55, 
					  "Hide Wave Window"));
		else
			SDtAuSetMessage(wd->footer_msg,
				   catgets(msgCatalog, 3, 30,
					   "Show Wave Window"));
	}
}

/* Function	:	SetPositionCallback()
 * Objective	:	Set the progress scale to the beginning or
 * 			end of the audio file, depending on whether
 * 			the forward or rewind button was pressed.
 */
void
SetPositionCb(Widget w, XtPointer clientData, XtPointer calldata)
{
	WindowData     *wd = (WindowData *) clientData;
	int		scale_value;
	short		wave_win_value;
	Boolean		initial_play;
	Dimension	w_width;

	/* Is audio playing? */
	if (wd->play_state == STATE_PLAY)
		initial_play = True;

	/* If the audio file is being played, stop it temporarily
	 * before setting the location of the slider.
	 */
	SDtAuStopAndWait(wd);

	if (w == wd->bwd_button) {
		scale_value = 0;	/* Set to the beginning. */
		wave_win_value = (short) 0;
	} else {
		scale_value = 100;	/* Set to the end. */
		XtVaGetValues(wd->wave_window,
			      XtNwidth, &w_width,
	 		      NULL);
		wave_win_value = (short) w_width;
	}

	/* Set the slider to the specified value. */
	XmScaleSetValue(wd->prog_scale, scale_value);
	SDtAuDrawWaveCursor(wd, wave_win_value);
	wd->clip_pos = scale_value;
	XmUpdateDisplay(wd->toplevel);

	/* If the user was playing an audio file initially, and they
	 * pressed rewind in the middle of play, then continue playing
	 * the audio file.
	 */
	if (scale_value == 0 && initial_play)
		xa_play_audio(wd);
}

/* Function	: HelpCb()
 * Objective	: Create the help widget and display the appropriate help
 *		  volume.
 */
void 
HelpCb(Widget w, XtPointer client_data, XtPointer cb)
{
	static Widget   helpDialog = NULL;
	Widget		menu_item;
	register int    n = 0;
	SDtAudioHelpP   help_rec = (SDtAudioHelpP) client_data;
	Arg             args[10];
	char           *location_id, *help_volume;

	/* Turn on the busy cursor. */
	_DtTurnOnHourGlass(help_rec->help_parent_win);

	/* If the help widget is not created, then do so. */
	help_volume = SDTAUDIO_HELP_VOLUME;
	if (helpDialog == NULL) {
		char           *title;
		Arg             args[10];
		int             n;

		n = 0;
		helpDialog = 
			DtCreateHelpDialog(appShell, "helpDialog", args, n);
		title = catgets(msgCatalog, 1, 4, "Audio - Help");
		XtVaSetValues(XtParent(helpDialog), XmNtitle, title, NULL);
	}

	/* Determine which help volume to display. */
	switch (help_rec->help_event) {

	case HELP_TASKS_EVENT:
		location_id = HELP_TASKS;
		break;

	case HELP_REF_EVENT:
		location_id = HELP_REF;
		break;

	case HELP_USING_HELP_EVENT:
		location_id = HELP_USING_HELP;
		help_volume = "Help4Help";
		break;

	case HELP_AUDIO_INFO_EVENT:
		location_id = HELP_AUDIO_INFO;
		break;

	case HELP_AUDIO_OPTIONS_EVENT:
		XtVaGetValues(help_rec->wd->options_category,
			      XmNmenuHistory, &menu_item,
			      NULL);
		if (menu_item == help_rec->wd->menu_items->PlayItem)
			location_id = HELP_AUDIO_PLAY_OPTIONS;
		else if (menu_item == help_rec->wd->menu_items->RecordItem)
			location_id = HELP_AUDIO_RECORD_OPTIONS;
		else if (menu_item == help_rec->wd->menu_items->DevicesItem)
			location_id = HELP_AUDIO_DEVICES_OPTIONS;
		else
			location_id = HELP_AUDIO_START_OPTIONS;
		break;

	case HELP_AUDIO_SAVE_AS_EVENT:
		if (help_rec->wd->audio_modified)
			location_id = HELP_AUDIO_SAVE_NEW;
		else
			location_id = HELP_AUDIO_SAVE_AS;
		break;

	case HELP_AUDIO_OPEN_EVENT:
		location_id = HELP_AUDIO_OPEN;
		break;

	case HELP_AUDIO_SET_RECORD_VALUES_EVENT:
		location_id = HELP_AUDIO_RECORD_OPTIONS;
		break;

	case HELP_AUDIO_MAIN_WIN_EVENT:
		location_id = HELP_AUDIO_MAIN_WIN;
		break;

	case HELP_AUDIO_FILE_MENU_EVENT:
		location_id = HELP_AUDIO_FILE_MENU;
		break;

	case HELP_AUDIO_OPTIONS_MENU_EVENT:
		location_id = HELP_AUDIO_OPTIONS_MENU;
		break;

	case HELP_AUDIO_HELP_MENU_EVENT:
		location_id = HELP_AUDIO_HELP_MENU;
		break;

	case HELP_AUDIO_WAVE_WIN_EVENT:
		location_id = HELP_AUDIO_WAVE_WIN;
		break;

	default:
		location_id = HELP_OVERVIEW;
		break;
	}

	/* Set the help volume location id. */
	n = 0;
	XtSetArg(args[n], DtNhelpType, DtHELP_TYPE_TOPIC); n++;
	XtSetArg(args[n], DtNhelpVolume, help_volume); n++;
	XtSetArg(args[n], DtNlocationId, location_id); n++;
	XtSetValues(helpDialog, args, n);
	XtManageChild(helpDialog);

	/* Turn off the busy cursor. */
	_DtTurnOffHourGlass(help_rec->help_parent_win);
}

/* Function	: NewCb()
 * Bbjective	: Clear the sdtaudio window of its current contents in 
 *		  preparation for a recording, or for loading in a new
 *		  file.
 */
void
NewCb(Widget w, XtPointer cd, XtPointer cb)
{
	WindowData *wd = (WindowData *) cd;
	SAError     error;

	/* Before loading in the new audio file, stop the current one
	 * from playing.
	 */
	SDtAuStopAndWait(wd);

	/* Lock the data before checking. */
	XA_LOCK(wd, "NewCb");

	/* Return if New was selected previously, and then
	 * it was selected again.
	 */
	if (wd->safile == (SAFile *) NULL) {
		XA_UNLOCK(wd, "NewCb");
		return;
	}

	/* Remove the temp_buffer file, in case we loaded from buffer. */
	if (temp_buffer_name) {
		unlink(temp_buffer_name);
		free(temp_buffer_name);
		temp_buffer_name = (char *) NULL;
	}

	if (SaveNeeded(wd)) {
		XA_UNLOCK(wd, "NewCb");
		return;
	}

	/* Return - sdtaudio has no data. */
	if (wd->safile->format.encoding == SANoType) {

		/* Turn of SaveAs menu item, no data */
		XA_UNLOCK(wd, "NewCb");
		XtSetSensitive(wd->menu_items->SaveAsItem, False);
		return;
	}

	/* Close the audio file.  If close fails, still proceed? */
	if (!SAFileClose(wd->safile, &error)) {
		XA_UNLOCK(wd, "NewCb");
		AlertPrompt(wd->toplevel,
			DIALOG_TYPE, XmDIALOG_WARNING,
			DIALOG_TITLE, catgets (msgCatalog, 3, 59, "Audio - Error"),
			DIALOG_TEXT, catgets(msgCatalog,1, 21 ,"Failed to close current audio file."),
			BUTTON_IDENT, ANSWER_ACTION_1, 
				catgets(msgCatalog, 2, 62, "Continue"),
			NULL);
		return;
	}

	XA_UNLOCK(wd, "NewCb");
	FreeData(wd);

	/* Reset the title, footer, and status areas.  Reset then end
	 * label.  Reset the slider to the beginning.
	 */
	XA_LOCK(wd, "NewCb");
	SetTitle(wd->toplevel, NULL);
	SDtAuSetMessage(wd->footer_msg, "");
	SDtAuSetMessage(wd->status_msg, "");
	XtSetSensitive(wd->menu_items->SaveAsItem, False);
	XtSetSensitive(wd->menu_items->InfoItem, False);
	XtSetSensitive(wd->record_button, True);
	SDtAuUpdateTimeLabels(wd);
	XmScaleSetValue(wd->prog_scale, 0);
	wd->clip_pos = 0;

	/* Unamanage info dialog if it is displayed. */
	if (wd->info_dlog && XtIsManaged(wd->info_dlog))
		XtUnmanageChild(wd->info_dlog);

	/*
	 * These should be insensitive initially, until a file is loaded, or
	 * after the user starts recording some audio.
	 */
	XtSetSensitive(wd->prog_scale, False);
	XtSetSensitive(wd->bwd_button, False);
	XtSetSensitive(wd->fwd_button, False);
	XtSetSensitive(wd->play_button, False);
	XmUpdateDisplay(wd->toplevel);

#ifdef SELECTION
	/* Clear the selection. */
	SDtAuClearSelection(wd);
#endif
	/* Free the current wave pixmap. */
	if (wd->wave_pixmap != NULL) {
		XFreePixmap(XtDisplay(wd->wave_window), wd->wave_pixmap);
		wd->wave_pixmap = NULL;
	}

	/* After disassociating the audio file, update the wave window. */
	if (wd->show_wave) {
		SDtAuUpdateWaveWin(wd);
		SDtAuUpdateLevelMeter(wd);
	}

	if (wd->info_dlog && XtIsManaged(wd->info_dlog))
		SDtAuUpdateInfoDlog(wd);
	SDtAuUpdateTimeLabels(wd);
	XA_UNLOCK(wd, "NewCb");
}

/* Function	: OpenCb()
 * Objective	: On the first call, create a file selection dialog that
 *		  will be used to pick and open audio files; display this
 *		  file selection dialog.
 */
void 
OpenCb(Widget w, XtPointer cd, XtPointer cb)
{
	WindowData     *wd = (WindowData *) cd;
	static SDtAudioHelpR help_rec;

	/* Did user record and not save the sound file? */
	if (SaveNeeded(wd)) {
		return;
	}

	/* Create file selection dialog if necessary. */
	_DtTurnOnHourGlass(wd->toplevel);
	if (wd->open_dlog == NULL) {

		XmString        dialogTitle;
		Arg             args[20];
		int             n;

		dialogTitle = XmStringCreateLocalized(
			catgets(msgCatalog, 1, 2, "Audio - Open File"));

		n = 0;
		XtSetArg(args[n], XmNdialogStyle, 
			 XmDIALOG_PRIMARY_APPLICATION_MODAL);
		n++;
		XtSetArg(args[n], XmNautoUnmanage, False); n++;
		XtSetArg(args[n], XmNdialogTitle, dialogTitle); n++;
		XtSetArg(args[n], "pathMode", 1); n++;
		wd->open_dlog = 
			XmCreateFileSelectionDialog(wd->toplevel, "open_dlog",
						    args, n);
		XtAddCallback(wd->open_dlog, XmNokCallback, OpenOkCb, cd);
		XtAddCallback(wd->open_dlog, XmNcancelCallback, 
			      GenericCancelCb, (XtPointer) wd->open_dlog);
		help_rec.help_parent_win = wd->open_dlog;
		help_rec.help_event = HELP_AUDIO_OPEN_EVENT;
		help_rec.wd = wd;
		XtAddCallback(wd->open_dlog, XmNhelpCallback, HelpCb,
			      (XtPointer) &help_rec);

		XmStringFree(dialogTitle);
	}

	XtManageChild(wd->open_dlog);
	_DtTurnOffHourGlass(wd->toplevel);
}

/* Function	: OpenOkCb()
 * Objective	: Load the selected file into sdtaudio.
 */
void 
OpenOkCb(Widget w, XtPointer cd, XtPointer cb)
{
	char            fileName[MAXPATHLEN];
	WindowData     *wd = (WindowData *) cd;

	XmFileSelectionBoxCallbackStruct *fsbcs =
			(XmFileSelectionBoxCallbackStruct *) cb;

	ConvertCompoundToChar(fsbcs->value, fileName);
	if (LoadFile(wd, fileName))
		XtUnmanageChild(wd->open_dlog);
}

/* Function	: ExitCb()
 * Objective	: Delete the current window.  If there are no more windows 
 *		  in the linked list, exit the application.
 */
void 
ExitCb(Widget w, XtPointer cd, XtPointer cb)
{
	WindowData     *wd = (WindowData *) cd;

	/* Remove and unlink any temporary buffer. */
	if (temp_buffer_name) {
		unlink(temp_buffer_name);
		free(temp_buffer_name);
		temp_buffer_name = (char *) NULL;
	}

	/* Did user record and not save the sound file? */
	if (SaveNeeded(wd)) {
		XA_UNLOCK(wd, "ExitCb");
		return;
	}

	/* Destroy the instance sdtaudio and exit. */
	DestroyData(wd);
	if (windowList == NULL) {
		tt_close();
		exit(0);
	}
}

/* Function	: SaveSessionCb()
 * Objective	: Handle WM_SAVE_YOURSELF by updating the command line
 *		  with the files currently being edited.
 */
void 
SaveSessionCb(Widget w, XtPointer cd, XtPointer cb)
{
	char          **command;
	int             argcount = 1;	/* starts at 1 for command name */
	WindowData     *wd = windowList;
	int             i;
	Widget          first;

	/* count the number of windows bound to files */
	for (wd = windowList; wd != NULL; wd = wd->next) {
		if (wd->name != NULL)
			++argcount;
	}

	command = (char **) XtMalloc(argcount * sizeof(char *));
	command[0] = argv0;
	i = 1;
	for (wd = windowList; wd != NULL; wd = wd->next) {
		if (wd->name != NULL)
			command[i++] = wd->name;
	}

	first = windowList->toplevel;
	XSetCommand(XtDisplay(first), XtWindow(first), command, i);
	if (w != first)
		XChangeProperty(XtDisplay(w), XtWindow(w), XA_WM_COMMAND, 
			        XA_STRING, 8, PropModeReplace, NULL, 0);
	XtFree((char *) command);
}
