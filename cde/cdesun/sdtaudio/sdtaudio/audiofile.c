/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)audiofile.c	1.49 97/06/04 SMI"

/*
 * Contains all the C wrapper functions for the C++ Xaudio methods...
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <poll.h>
#include <thread.h>
#include <assert.h>
#include <nl_types.h>
#include <Xm/XmAll.h>
#include <sys/audioio.h>
#include <sys/errno.h>

#include "extern.h"
#include "sdtaudio.h"
#include "audiofile.h"
#include "callbacks.h"
#include "cb_utils.h"
#include "thrdefs.h"
#include "utils.h"
#include "safile.h"
#include "rec_init.h"
#include "mutex.h"

#define TEMPDIR     "/tmp"
#define REC_PREFIX  "audrx"

static int      xa_checkForStop(int fd);
static void    *xa_play_thread(void *);
static void    *xa_record_thread(void *);
static void	xa_free_thread_data(XaThreadData *thread_data);

char  *temp_rec_file = (char *) NULL;    /* Temporary file name for record. */

/**********************************************************************/

/*
 * xa_play_thread
 * 
 * Thread that is created when the play button is pressed...
 */
static void    *
xa_play_thread(void *data)
{
	XaThreadData   *thread_data = (XaThreadData *) data;
	WindowData     *wd = thread_data->wd;
	SAFile         *safile = wd->safile;
	MsgData         msg;
	int             num_samples = 0;
	int             start_pos = 0;
	SAError         error;
	struct pollfd   fds;
	SABoolean       done = SAFalse;
	int             frame_size = SAFormatFrameSize(&safile->format);
	char           *buffer = NULL;
	int             buffer_size = 0, save_buf_size = 0;
	int             buffer_start = 0;
	int             bytes_left = 0;
	int             bytes_written = 0;
	char           *outbuf = NULL;
	int             outbufsize = 0, save_out_size = 0;
	char            leftover_bytes[16];
	int             leftover_count = 0;
	SABoolean       start_playing = SAFalse;
	SABoolean       playing = SAFalse;
	SAStream       *stream = wd->sa_stream;
	int             stop = False;
	SABoolean       still_underflowed = SAFalse;
	int             bytes = 0;
	SAEvent         event;
	Boolean		msg_sent = False;

	buffer_size = wd->safile->format.sample_rate * frame_size;

#ifdef DEBUG
        fprintf(stderr, "TEMP BUFFER SIZE : %d\n", buffer_size);
#endif

	/*
	 * If clip_pos is greater than or equal to 100, i.e., end of
	 * sound file, reset it to 0, and start playing from there.
	 * Behavior is patterned after OpenWindows audiotool.
	 */
	if (wd->clip_pos >= 100.0)
		wd->clip_pos = 0.0; 

	/* Calculate the sampling number. */
	num_samples = SAFileGetDataSize(safile) / safile->format.sample_size / 
		      safile->format.channels;

	/* Calculate the start position, in case user moved slider. */
	start_pos = (int) ((float) num_samples * 
			   (float) (wd->clip_pos / 100.0));

	if (SAFileSeek(safile, start_pos, &error) != SATrue) {

		/* Send a seek error message. */
		msg.tag = SEEK_ERR_MSG;
		write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));
		return NULL;
	}

	if (wd->sa_stream != NULL) {

		buffer = (char *) malloc(buffer_size);
		if (buffer == NULL) {
			msg.tag = BUFFER_ERR_MSG;
			write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));
			return NULL;
		}

		if (wd->sa_real_format != NULL) {

			/* Allocate a buffer to do the conversion. */
			outbufsize = 2 * (buffer_size * frame_size /
					SAFormatFrameSize(wd->sa_real_format));
			outbuf = (char *) malloc(outbufsize);
			if (outbuf == NULL) {
				free(buffer);
				msg.tag = BUFFER_ERR_MSG;
				write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));
				return NULL;
			}

		} else {

			outbuf = buffer;
			outbufsize = buffer_size;
		}

		save_out_size = outbufsize;
		save_buf_size = buffer_size;

		/* Play audio file until done. */
		done = SAFalse;
		while (done == SAFalse) {

			outbufsize = save_out_size;
			buffer_size = save_buf_size;

			if (start_playing == SATrue) {

				while (SAStreamPlay(stream, &error) != SATrue) {

					if (error.code == EBUSY && !msg_sent) {

						/* Inform user to wait, since
						 * audio device is busy.
						 */
						msg.tag = WAIT_MSG;
						write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));
						msg_sent = True;

					} else if (error.code != EBUSY) {

						/* Send a play error message 
						 * to GUI pipe.
						 */
						msg.tag = PLAY_ERR_MSG;
						msg.error = error;
						write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));
						return NULL;
					}

				} /* End SAStreamPlay */

				playing = SATrue;
				start_playing = SAFalse;
				msg.tag = PLAY_MSG;
				write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));

			} /* start_playing */

			if (playing && still_underflowed == SAFalse) {

				fds.fd = SAGetPipe();
				fds.events = POLLIN;
				fds.revents = NULL;

				while (1) {

					if (poll(&fds, 1, -1) > 0) {

						if (SANextEvent(fds.fd, &event, &error) == SAFalse) {
							fprintf(stderr, "SAError: %s\n",
							 error.description);
							continue;
						}

						/* Update the level meter */
						msg.tag = LEVEL_MSG;
						SAStreamLock(wd->sa_stream);
						msg.u.meter_gain = wd->sa_stream->meter_gain;
						SAStreamUnlock(wd->sa_stream);

						XA_LOCK(wd, "xa_play_thread");
						write(thread_data->audio_to_uiFd[1],
						      &msg, sizeof(msg));
						XA_UNLOCK(wd, "xa_play_thread");

						switch (event.type) {

						case SAEvent_Underflow:
						case SAEvent_LowWater:
							break;
						default:
							fprintf(stderr, "Unkown SAEvent: %d\n", event.type);
							continue;
						}

						/* Fall out and refill buffer */
						break;
					} /* poll */
				} /* while */
			} /* playing */

			XA_LOCK(wd, "xa_play_thread");
			stop = xa_checkForStop(thread_data->audio_to_uiFd[1]);
			XA_UNLOCK(wd, "xa_play_thread");

			if (stop == True) {
				done = SATrue;
				continue;
			}

			/*
			 * This is roughly right though this number is always
			 * relative to the the start of the stream
			 */
			msg.tag = INDEX_MSG;
			SAStreamLock(wd->sa_stream);
			msg.u.index = wd->sa_stream->device->samples_played;
			SAStreamUnlock(wd->sa_stream);

			/*
			 * add in the the offset into the file that we
			 * started on
			 */
			msg.u.index += start_pos;
			XA_LOCK(wd, "xa_play_thread");
			write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));
			XA_UNLOCK(wd, "xa_play_thread");

			if (bytes_left == 0) {

				if (leftover_count != 0) {
					memcpy(buffer, leftover_bytes, leftover_count);
					bytes_left = leftover_count;
					leftover_count = 0;
				}

				buffer_start = 0;
				if (SAFileRead(safile, buffer + bytes_left,
					       buffer_size - bytes_left,
					       &bytes, &error) == SAFalse) {
					fprintf(stderr, "SAFileRead: %s\n", error.description);
					done = SATrue;

				} else {

					bytes_left += bytes;
					if (bytes_left != 0) {

						leftover_count = bytes_left % frame_size;
						if (leftover_count != 0) {

							/*
							 * we got a non
							 * multiple of the
							 * framesize back so
							 * hang onto them
							 * until we can do
							 * something with
							 * them.
							 */
							memcpy(buffer + bytes_left - leftover_count,
							       leftover_bytes, leftover_count);
							bytes_left -= leftover_count;
						}
					}

					if (bytes_left == 0) {

						if (playing) {
							SAStreamDrain(stream, &error);
							done = SATrue;
						} else {
							start_playing = SATrue;
						}
						continue;
					}
				}

				if (wd->sa_real_format != NULL) {
					SAConvertData(&safile->format, 
					              wd->sa_real_format,
						      buffer, bytes_left, 
						      outbuf, &outbufsize, 
						      &error);
					bytes_left = outbufsize;
				}
			}

			if (SAStreamWrite(stream, outbuf + buffer_start,
					  bytes_left, &bytes_written,
					  &error) == SAFalse) {

				if (error.code == SAError_BUFFER_FULL) {
					start_playing = SATrue;
					continue;
				} else {
					done = SATrue;
					continue;
				}
			}

			SAStreamLock(stream);
			if (stream->buffer.data_length < stream->buffer.water_line)
				still_underflowed = SATrue;
			else
				still_underflowed = SAFalse;
			SAStreamUnlock(stream);
			bytes_left -= bytes_written;
			buffer_start += bytes_written;
		}

		SAStreamStop(wd->sa_stream, &error);

		/* Clean up */
		if (buffer)
			free(buffer);
		if (outbuf)
			free(outbuf);
		buffer = outbuf = NULL;
	}

	/* Set the correct state. */
	AT_MUTEX_LOCK(wd, "xa_play_thread");
	wd->play_state = STATE_STOPPED;
	AT_MUTEX_UNLOCK(wd, "xa_play_thread");

	msg.tag = END_MSG;
	msg.u.prev_state = STATE_PLAY;
	write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));

	return NULL;
}

/**********************************************************************/

/*
 * xa_record_thread
 * 
 * Thread that is created when the record button is pressed...
 */
static void    *
xa_record_thread(void *data)
{
	XaThreadData   *thread_data = (XaThreadData *) data;
	WindowData     *wd = thread_data->wd;
	MsgData         msg;
	struct pollfd   fds;
	int             stop = False;
	SAError         error;
	SABoolean       done = SAFalse;
	int             frame_size = SAFormatFrameSize(&(wd->safile->format)); 
	char           *buffer = NULL;
	int             buffer_size = 0;
	int             buffer_start = 0;
	int             bytes_left = 0;
	int             bytes_written = 0;
	char           *outbuf = NULL;
	int             outbufsize = 0;
	char            leftover_bytes[16];
	int             leftover_count = 0;
	SABoolean       start_playing = SAFalse;
	SABoolean       still_overflowed = SAFalse;
	SAFormat        alternate;

        buffer_size = wd->safile->format.sample_rate * frame_size;

#ifdef DEBUG
        fprintf(stderr, "TEMP BUFFER SIZE : %d\n", buffer_size);
#endif

	if (wd->sa_stream != NULL) {

		buffer = (char *) malloc(buffer_size);
		if (wd->sa_real_format != NULL) {

			/* We need to allocate a buffer to do the conversion
			 * into
			 */
			outbuf = (char *) malloc(2 * (buffer_size * frame_size /
				 SAFormatFrameSize(wd->sa_real_format)));
		} else
			outbuf = buffer;

		if (buffer == NULL || outbuf == NULL) {

			/* Clean up and send a buffer error message to GUI */
			if (buffer) 
				free(buffer);
			if (outbuf)
				free(outbuf);

			msg.tag = BUFFER_ERR_MSG;
			write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));
			return NULL;
		}

		if (SAStreamRecord(wd->sa_stream, &error) != SATrue) {

			/* Send an end message to GUI pipe. */
			msg.tag = RECORD_ERR_MSG;
			msg.error = error;
			write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));
			return NULL;
		}

		/* Inform the pipe that we are recording. */
		msg.tag = RECORD_MSG;
		write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));

		while (done == SAFalse) {

			fds.fd = SAGetPipe();
			fds.events = POLLIN;
			fds.revents = NULL;

			if (still_overflowed == SAFalse)

				while (1)
					if (poll(&fds, 1, -1) > 0) {
						SAEvent         event;
						if (SANextEvent(fds.fd, &event, &error) == SAFalse) {
							fprintf(stderr, "SAError: %s\n",
							 error.description);
							continue;
						}

						switch (event.type) {
						case SAEvent_Overflow:
						case SAEvent_HighWater:
						case SAEvent_LowWater:
							break;

						default:
							fprintf(stderr, "Unknown SAEvent: %s\n", SAEventToString(event.type));
							continue;
						}

						/* fall out and refill the
						 * buffer
						 */
						break;
			} /* still_overflowed */

			XA_LOCK(wd, "xa_record_thread");
			stop = xa_checkForStop(thread_data->audio_to_uiFd[1]);
			XA_UNLOCK(wd, "xa_record_thread");

			if (stop == True) {
				done = SATrue;
				continue;
			}

			if (bytes_left == 0) {
				int bytes = 0;

				if (leftover_count != 0) {

					memcpy(buffer, leftover_bytes, 
					       leftover_count);
					bytes_left = leftover_count;
					leftover_count = 0;
				}
				buffer_start = 0;
				error.code = SASuccess;
				if (SAStreamRead(wd->sa_stream, 
					         buffer + bytes_left,
						 buffer_size - bytes_left, 
						 &bytes, &error) == SAFalse) {
					
					bytes_left = 0;
					if (error.code == SAError_BUFFER_EMPTY)
						still_overflowed = SAFalse;
					else
						done = SATrue;

				} else {

					SAStreamLock(wd->sa_stream);
					if (wd->sa_stream->buffer.data_length > 
					    wd->sa_stream->buffer.water_line)
						still_overflowed = SATrue;
					else
						still_overflowed = SAFalse;
					SAStreamUnlock(wd->sa_stream);

					bytes_left += bytes;
					if (bytes_left != 0) {
						leftover_count = bytes_left %
							frame_size;

						if (leftover_count != 0) {

							/* We got a non-
							 * multiple of the
							 * framesize back  so
							 * hang onto them
							 * until we can do
							 * something with
							 * them
							 */
							memcpy(buffer + 
							       bytes_left - 
							       leftover_count,
							       leftover_bytes, 
							       leftover_count);
							bytes_left -= 
								leftover_count;
						}
					}
				}

				if ((wd->sa_real_format != NULL) && (bytes_left >0)) {

					SAConvertData(&(wd->safile->format), 
						      wd->sa_real_format,
						      buffer, bytes_left,
					              outbuf, &outbufsize,
						      &error);
					bytes_left = outbufsize;
				}
			}
		
			bytes_written = 0;
			if (bytes_left > 0) {

				if (SAFileWrite(wd->safile, outbuf + buffer_start,
					bytes_left, &bytes_written,
					&error) == SAFalse) {
					if (error.code != SAError_BUFFER_FULL)
						done = SATrue;
					else {
						start_playing = SATrue;
						continue;
					}
				}
			}
			bytes_left -= bytes_written;
			buffer_start += bytes_written;
		}

		SAFileClose(wd->safile, &error);
		SAStreamClose(wd->sa_stream, &error);
		XA_LOCK(wd, "xa_record_thread");
		free(wd->safile);
		wd->safile = (SAFile *) NULL;
		wd->sa_stream = NULL;
		XA_UNLOCK(wd, "xa_record_thread");
	}

	/* Clean up */
	if (buffer)
		free(buffer);
	if (outbuf)
		free(outbuf);
	buffer = outbuf = NULL;

	/* Set the correct state. */
	AT_MUTEX_LOCK(wd, "xa_record_thread");
	wd->play_state = STATE_STOPPED;
	AT_MUTEX_UNLOCK(wd, "xa_record_thread");

	msg.tag = END_MSG;
	msg.u.prev_state = STATE_RECORD;
	write(thread_data->audio_to_uiFd[1], &msg, sizeof(msg));

	/* Now that we've finished recording, need to create a stream
	 * for playback.
	 */
	XA_LOCK(wd, "xa_record_thread");
	wd->safile = (SAFile *) calloc(1, sizeof(SAFile));
	SAFileOpen(temp_rec_file, wd->safile, &error);

	if (wd->sa_stream != (SAStream *)NULL)
		SAStreamClose(wd->sa_stream, &error);
	if (wd->sa_real_format != NULL)
		free(wd->sa_real_format);
	wd->sa_real_format = NULL;

	wd->sa_stream = SAStreamOpen(NULL, &error);
	SAStreamSetFormat(wd->sa_stream, &(wd->safile->format),
			  &alternate, &error);
	if (alternate.encoding != SAEncoding_NONE) {
		wd->sa_real_format = 
			(SAFormat *) calloc(sizeof(SAFormat), 1);
		*(wd->sa_real_format) = alternate;
	}
	if (wd->name) {

		/* User specified a file name on the command line, so
		 * recording was automatically saved.  Update the title
		 * and make the Get Info sensitive.
		 */
		SetTitle(wd->toplevel, wd->name);
		XtSetSensitive(wd->menu_items->InfoItem, True);
	} else {
		wd->audio_modified = True;
		wd->name = XtNewString(temp_rec_file);
	}
	XA_UNLOCK(wd, "xa_record_thread");
	SDtAuUpdateWaveWin(wd);
	wd->clip_pos = 0;
	return NULL;
}

/**********************************************************************/

/* Function	: xt_thread_shut_down()
 * Objective	: When playing/recording audio halts (due to an
 *		  explicit stop, end of data, or error), reset
 *		  audio GUI to some halt state and close 
 *		  down pipes from play and record threads to
 *		  GUI.
 * Arguments	: ThreadData *		- thread data structure
 *		  WindowData *		- audio data structure
 * Return Value	: None.
 * 
 */
void
xt_thread_shut_down(XaThreadData *thread_data, WindowData *wd)
{
	xa_free_thread_data(thread_data);

	/* Reset state of play, record, record light and message
	 * footer GUI components.
	 */
	wd->play_state = STATE_STOPPED;
	dtb_set_label_pixmaps(wd->play_button, wd->play_pixmap,
			      wd->ins_play_pixmap);
	dtb_set_label_pixmaps(wd->record_button, wd->record_pixmap,
			      wd->ins_record_pixmap);
	dtb_set_label_pixmaps(wd->record_light, wd->rec_blink_off_pixmap,
			      wd->ins_blink_off_pixmap);
	SDtAuSetMessage(wd->footer_msg, "");
}

/*
 * xt_thread_handler
 * 
 * called when a message from the play thread is passed down the pipe
 */
void 
xt_thread_handler(XaThreadData *thread_data, int source, XtInputId *id)
{
	WindowData     *wd = thread_data->wd;
	MsgData         msg;
	SABoolean       done = SAFalse;
	SAError         error;
	off_t		tmp_data_size;
	char            err_buf[2048];

	tmp_data_size = SAFileGetDataSize(wd->safile);
	while (done == SAFalse) {

		struct pollfd fds;

		fds.fd = thread_data->audio_to_uiFd[0];
		fds.events = POLLIN;
		fds.revents = NULL;
	
		/* read the pipe */
		if ((poll(&fds, 1, 0) > 0) ||
		    (fds.revents & (POLLERR | POLLHUP | POLLNVAL))) {
			read(thread_data->audio_to_uiFd[0],
			     (char *) &msg, sizeof(MsgData));
		} else {
			done = SATrue;
			continue;
		}
	
		switch (msg.tag) {

		case PLAY_MSG:
			dtb_set_label_pixmaps(wd->play_button,
					      wd->stop_pixmap,
					      wd->ins_stop_pixmap);
			break;

		case BUFFER_ERR_MSG:
			xt_thread_shut_down(thread_data, wd);

			/* Alert the user that buffer allocation failed.  */
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
			  	    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 3, 46, 
							 "Could not allocate memory for operation."),
			            BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			XmUpdateDisplay(wd->toplevel);
			done = SATrue;
			break;

		case PLAY_ERR_MSG:
			xt_thread_shut_down(thread_data, wd);

			/* Alert the user that play failed.  */
			sprintf(err_buf,  catgets(msgCatalog, 4, 21, 
						  "Play failed : %s."), 
						  msg.error.description);
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
							  "Audio - Error"),
				    DIALOG_TEXT, err_buf,
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			XmUpdateDisplay(wd->toplevel);
			done = SATrue;
			break;

		case SEEK_ERR_MSG:
			xt_thread_shut_down(thread_data, wd);
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 4, 20, 
							 "Could not read audio data from specified location in file."),
				    BUTTON_IDENT, ANSWER_ACTION_1,
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			XmUpdateDisplay(wd->toplevel);
			done = SATrue;

		case END_MSG:
			xt_thread_shut_down(thread_data, wd);

			/* INDEXUPDATE */
			/* Reset the position indicator to the maximum,
			 * i.e., the end of the audio file; we are following
			 * OW audiotool behavior by leaving the play point
			 * at the end of the file.
			 */
			wd->clip_pos = 100.0;
			XmScaleSetValue(wd->prog_scale, wd->clip_pos);
			SDtAuUpdateWaveCursor(wd);

			done = SATrue;
			wd->meter_gain = 0;
			SDtAuUpdateLevelMeter(wd);
			XmUpdateDisplay(wd->toplevel);
			continue;
	    
		case RECORD_MSG:
			dtb_set_label_pixmaps(wd->record_button,
					      wd->stop_pixmap,
					      wd->ins_stop_pixmap);
			XtSetSensitive(wd->play_button, False);
			XtSetSensitive(wd->fwd_button, False);
			XtSetSensitive(wd->bwd_button, False);
			break;
	    
		case RECORD_ERR_MSG:
			xt_thread_shut_down(thread_data, wd);

			/* Alert the user that play failed.  */
			sprintf(err_buf,  catgets(msgCatalog, 4, 22,
						  "Record failed: %s"), 
						  msg.error.description);
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
							  "Audio - Error"),
				    DIALOG_TEXT, err_buf,
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			XmUpdateDisplay(wd->toplevel);
			done = SATrue;
			break;

		case STOP_MSG:
			xt_thread_shut_down(thread_data, wd);
			if (wd->record_time_out != 0)
				XtRemoveTimeOut(wd->record_time_out);
	    
			wd->record_time_out = 0;
	    	    
			XtSetSensitive(wd->play_button, True);
			XtSetSensitive(wd->fwd_button, True);
			XtSetSensitive(wd->bwd_button, True);
			XtSetSensitive(wd->prog_scale, True);
			XtSetSensitive(wd->record_button, False);
			done = SATrue;
			wd->meter_gain = 0;
			SDtAuUpdateLevelMeter(wd);
			if (wd->info_dlog && XtIsManaged(wd->info_dlog))
				SDtAuUpdateInfoDlog(wd);
			SDtAuUpdateTimeLabels(wd);
			XmUpdateDisplay(wd->toplevel);
			break;
	    
		case INDEX_MSG:
			/* Update the play point cursor's location. */
			if (tmp_data_size == 0)
				wd->clip_pos = 0;
			else
				wd->clip_pos = msg.u.index * 100.0 /
					(tmp_data_size /
			                wd->safile->format.sample_size /
			                wd->safile->format.channels);
			if (wd->clip_pos > 100)
				wd->clip_pos = 100;
			else if (wd->clip_pos < 0)
				wd->clip_pos = 0;

			/* If wave window is not shown, update the
			 * slider, otherwise, update the play point
			 * cursor.
			 */
			if (!wd->show_wave)
				XmScaleSetValue(wd->prog_scale, wd->clip_pos);
			else
				SDtAuUpdateWaveCursor(wd);
			XmUpdateDisplay(wd->toplevel);
		
			break;
	    
		case LEVEL_MSG:
			wd->meter_gain = msg.u.meter_gain;
			SDtAuUpdateLevelMeter(wd);
			break;
	    
		case WAIT_MSG:
			SDtAuSetMessage(wd->footer_msg, 
					catgets(msgCatalog, 2, 42, 
						"Audio device is busy - waiting..."));
			XmUpdateDisplay(wd->toplevel);
			break;
			
		default:
			wd->play_state = STATE_STOPPED;
			fprintf(stderr, catgets(msgCatalog, 3, 45, 
						"unknown pipe message...\n"));
			break;
	    
		}
	}
	XmUpdateDisplay(wd->toplevel);
}

/**********************************************************************/

/*
 * xa_stop_audio
 *
 * stop the current audio file
 */
void 
xa_stop_audio(WindowData *wd)
{
	MsgData         msg;

	msg.tag = STOP_MSG;

	XA_LOCK(wd, "xa_stop_audio");
	write(wd->play_thread_data->audio_to_uiFd[0], &msg, sizeof(msg));
	XA_UNLOCK(wd, "xa_stop_audio");
}

/**********************************************************************/

/*
 * xa_play_audio
 * 
 * play the current audio file...
 */
void 
xa_play_audio(WindowData * wd)
{
	XaThreadData   *thread_data = NULL;

	/* Set the correct state. */
	XA_LOCK(wd, "xa_play_audio");
	AT_MUTEX_LOCK(wd, "xa_play_audio");
	if (wd->play_state == STATE_PLAY) {	/* set the play state */

		/* don't forget to unlock the mutex */
		AT_MUTEX_UNLOCK(wd, "xa_play_audio");
		XA_UNLOCK(wd, "xa_play_audio");
		return;
	} else {
		wd->play_state = STATE_PLAY;
		SDtAuSetMessage(wd->footer_msg,
		   catgets(msgCatalog, 3, 24, "Stop Playing"));
	}
	AT_MUTEX_UNLOCK(wd, "xa_play_audio");

	/* Set up the data structure to pass to the thread program. */
	thread_data = (XaThreadData *) malloc(sizeof(XaThreadData));
	if (thread_data == NULL) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
		  	    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 46, "Could not allocate memory for operation."),
		            BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		wd->play_state = STATE_STOPPED;
		XA_UNLOCK(wd, "xa_play_audio");
		return;
	}

	/* Open a pipe from audio thread to UI */
	if (pipe(thread_data->audio_to_uiFd) < 0) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
		  	    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 47, "Could not establish pipe between sdtaudio and audio device."),
		            BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		wd->play_state = STATE_STOPPED;
		XA_UNLOCK(wd, "xa_play_audio");
		return;
	}
	thread_data->inputId = 
		XtAppAddInput(appContext,
			      thread_data->audio_to_uiFd[0],
			      (XtPointer) XtInputReadMask,
			      (XtInputCallbackProc) xt_thread_handler,
			      (XtPointer) thread_data);
	/* clean up 
	if (wd->play_thread_data)
		xa_free_thread_data(wd->play_thread_data);
*/

	thread_data->wd = wd;
	wd->play_thread_data = thread_data;
	at_thread_create(xa_play_thread, (void *) thread_data);

	/* Unlock the mutex */
	XA_UNLOCK(wd, "xa_play_audio");
}

/**********************************************************************/

/*
 * xa_record_audio
 * 
 * record an audio file...
 */
void 
xa_record_audio(WindowData *wd)
{
	char           *abuffer = NULL;
	XaThreadData   *thread_data = NULL;
	SAError         error;
	SAFormat        alternate;

	XA_LOCK(wd, "xa_record_audio");
	if (wd->play_state == STATE_RECORD) { 
		SDtAuSetMessage(wd->footer_msg,
			catgets(msgCatalog, 3, 54, "Stop Recording"));
		wd->play_state = STATE_STOPPED;
		XA_UNLOCK(wd, "xa_record_audio");
		return;
	}
	else {
		wd->play_state = STATE_RECORD;
		SDtAuSetMessage(wd->footer_msg,
			catgets(msgCatalog, 3, 54, "Recording"));
	}
	XA_UNLOCK(wd, "xa_record_audio");

	/* If the user wants the recording to be saved to a file,
	 * as in something specified in the command line, then
	 * use that file name.  Otherewise, generate a temporary 
	 * file name.
	 */
	if (wd->name)
		temp_rec_file = wd->name;
	else {
		temp_rec_file = tempnam(TEMPDIR, REC_PREFIX);
		if (temp_rec_file == (char *) NULL) {
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_ERROR,
				    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
							  "Audio - Error"),
				    DIALOG_TEXT, catgets(msgCatalog, 3, 48, "Could not generate temporary file for recording."),
				    BUTTON_IDENT, ANSWER_ACTION_1, 
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			wd->play_state = STATE_STOPPED;
			return;
		}
	}

	XA_LOCK(wd, "xa_record_audio");

	/* Clean up */
	if (wd->sa_stream != (SAStream *) NULL)
		SAStreamClose(wd->sa_stream, &error);

	/* Set up the stream to record the data */
	wd->sa_stream = SAStreamOpen(NULL, &error);
	if (wd->sa_stream == NULL) {
		wd->play_state = STATE_STOPPED;
		XA_UNLOCK(wd, "xa_record_audio");
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
		  	    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 58, "Could not open stream to audio device."),
		            BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);

		return;
	}

	/* Allocate memory for the sound file, and then initialize
	 * it with values used for recording.
	 */
	if (wd->safile != (SAFile *) NULL) {
		/* Discard the old safile and create new one. */
		(void) SAFileClose(wd->safile, &error);
		free(wd->safile); wd->safile = NULL;
	}

	wd->safile = (SAFile *) calloc(1, sizeof (SAFile));
	if (wd->safile == (SAFile *) NULL) {
		wd->play_state = STATE_STOPPED;
		XA_UNLOCK(wd, "xa_record_audio");
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			      DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 46, "Could not allocate memory for operation."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);

		return;
	}

	InitSAFile(wd);

	/* Create the sound file to be used for recording. */
	if (SAFileCreate(temp_rec_file, wd->safile, &error) == SAFalse) {
		wd->play_state = STATE_STOPPED;
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
		  	    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 48, "Could not generate temporary file for recording."),
		            BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		if (wd->name) {
			free(wd->name);
			wd->name = NULL;
		} else
			free(temp_rec_file);
		XA_UNLOCK(wd, "xa_record_audio");
		return;
	}

	/* Set the format on the audio stream. */
	if (SAStreamSetFormat(wd->sa_stream, &(wd->safile->format), 
			      &alternate, &error) == SAFalse) {
		wd->play_state = STATE_STOPPED;
		XA_UNLOCK(wd, "xa_record_audio");
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
		  	    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 33, "Could not set audio format."),
		            BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Configure audio device with my format */
	if (_SADeviceConfigure(wd->sa_stream, SATrue) == SAFalse) {
	   wd->play_state = STATE_STOPPED;
	   XA_UNLOCK(wd, "xa_record_audio");
	   return;
	}

	/* Clean up
	if (wd->play_thread_data)
		xa_free_thread_data(wd->play_thread_data);
	*/

	/* Set up the data structure to pass to the thread program */
	thread_data = (XaThreadData *) malloc(sizeof(XaThreadData));
	if (thread_data == NULL) {
		wd->play_state = STATE_STOPPED;
		XA_UNLOCK(wd, "xa_record_audio");
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
		  	    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
					  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 46, 
					"Could not allocate memory for operation."),
		            BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	/* Open a pipe from audio thread to UI. */
	if (pipe(thread_data->audio_to_uiFd) < 0) {
		wd->play_state = STATE_STOPPED;
		XA_UNLOCK(wd, "xa_record_audio");
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
		  	    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 47, "Could not establish pipe between sdtaudio and audio device."),
		            BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	}

	thread_data->inputId = 
		XtAppAddInput(appContext,
			      thread_data->audio_to_uiFd[0],
			      (XtPointer) XtInputReadMask,
			      (XtInputCallbackProc) xt_thread_handler,
			      (XtPointer) thread_data);
	wd->record_time_out =
		XtAppAddTimeOut(appContext, 500,
				(XtTimerCallbackProc) BlinkLightCb,
				wd);
	thread_data->wd = wd;
	wd->play_thread_data = thread_data;
	XA_UNLOCK(wd, "xa_record_audio");

	at_thread_create(xa_record_thread, (void *) thread_data);
}

/**********************************************************************/

/* Reads from a file and writes to an audio thing. */
static int 
xa_checkForStop(int fd)
{
	struct pollfd   fds;

	/* Test for stop msg! */
	fds.fd = fd;
	fds.events = POLLIN;
	fds.revents = NULL;

	/* Poll only checks for success, must also handle failure. */
	if (poll(&fds, 1, 0) > 0) {

		if (fds.revents && POLLIN) {
		        int result;
			MsgData msg;

			result = read(fd, (char *) &msg, sizeof(MsgData));
			if (result != 0)

				if (result != sizeof(MsgData)) {

					/* An error has occured! */
					fprintf(stderr, "%s\n", catgets(msgCatalog, 3, 49, "Could not read data from pipe."));
					exit(1);

				} else {

					if (msg.tag == STOP_MSG) {
						/* Send the stop msg to the UI.  */
						write(fd, &msg, sizeof(msg));
						return True;
					}
				}
		}
	}
	return False;
}

/**********************************************************************/

/*
 * xa_free_thread_data()
 *
 * Clean play or record thread data
 */
static void
xa_free_thread_data(XaThreadData *thread_data)
{
    if (thread_data != NULL) {
	XtRemoveInput(thread_data->inputId);
        thread_data->ui_to_audioFd[1] = -1;
        thread_data->ui_to_audioFd[0] = -1;
        close(thread_data->audio_to_uiFd[0]);
        close(thread_data->audio_to_uiFd[1]);
        thread_data->audio_to_uiFd[1] = -1;
        thread_data->audio_to_uiFd[0] = -1;

        free(thread_data);
        thread_data = NULL;
    }
}
