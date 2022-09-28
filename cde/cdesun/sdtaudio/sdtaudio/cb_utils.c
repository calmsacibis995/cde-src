/*
 * (c) Copyright 1996, 1997 Sun Microsystems, Inc.
 */

#pragma ident "@(#)cb_utils.c	1.14 97/06/04 SMI"

/* This file contains functions that are invoked by sdtaudio callback
 * routines.
 */

#include <stdlib.h>
#include <limits.h>
#include <nl_types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>
#include <X11/Core.h>
#include <Xm/XmAll.h>
#include <Dt/Dnd.h>

#include "sdtaudio.h"
#include "audiofile.h"
#include "extern.h"
#include "sa.h"
#include "cb_utils.h"
#include "default_options.h"
#include "utils.h"
#include "a_encode.h"

/* The resource database for the default tool options. */
XrmDatabase	sdta_options_rdb;

extern WindowData *wd;

/*----------------------- Functions internal to this module ------------------*/

/* Function	: SDtAuGetAvailableOutputDevices()
 * Objective	: Retrieve the output devices that are available on the system.
 * Arguments	: SAStream *		- sun audio stream data structure
 *					  used by sdtaudio
 *		  SAOutputFlags *	- output device structure
 * Return Value : True if operation succeeded, false otherwise.
 */
Boolean
SDtAuGetAvailableOutputDevices(SAStream *stream, SAOutputFlags *output)
{
	SAError  error;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, &error);
		if (!stream)
			return(False);
	}

	/* Now try and get the available output devices. */
	if (SADeviceGetAvailOutputPorts(stream->device, output, &error))
		return(True);
	else
		return(False);
}

/* Function	: SDtAuGetAvailableInputDevices()
 * Objective	: Retrieve the input devices that are currently being
 *		  used by the system.
 * Arguments	: SAStream *		- sun audio stream data structure
 *					  used by sdtaudio
 *		  SAInputFlags *	- input device structure
 * Return Value : True if operation succeeded, false otherwise.
 */
Boolean
SDtAuGetAvailableInputDevices(SAStream *stream, SAInputFlags *input)
{
	SAError  error;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, &error);
		if (!stream)
			return(False);
	}

	/* Now try and get the available input devices. */
	if (SADeviceGetAvailInputPorts(stream->device, input, &error))
		return(True);
	else
		return(False);
}

/* Function	: SDtAuGetCurrentOutputDevices()
 * Objective	: Retrieve the output devices that are currently being
 *		  used by the system.
 * Arguments	: SAStream *		- sun audio stream data structure
 *					  used by sdtaudio
 *		  SAOutputFlags *	- output device structure
 * Return Value : True if operation succeeded, false otherwise.
 */
Boolean
SDtAuGetCurrentOutputDevices(SAStream *stream, SAOutputFlags *output)
{
	SAError  error;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, &error);
		if (!stream)
			return(False);
	}

	/* Now try and get the output devices in use. */
	if (SADeviceGetOutputPorts(stream->device, output, &error))
		return(True);
	else
		return(False);
}

/* Function	: SDtAuGetCurrentInputDevices()
 * Objective	: Retrieve the input devices that are currently being
 *		  used by the system.
 * Arguments	: SAStream *		- sun audio stream data structure
 *					  used by sdtaudio
 *		  SAInputFlags *	- input device structure
 * Return Value : True if operation succeeded, false otherwise.
 */
Boolean
SDtAuGetCurrentInputDevices(SAStream *stream, SAInputFlags *input)
{
	SAError  error;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, &error);
		if (!stream)
			return(False);
	}

	/* Now try and get the input device in use. */
	if (SADeviceGetInputPorts(stream->device, input, &error))
		return(True);
	else
		return(False);
}

/* Function	: SDtAuGetAvailableDevices()
 * Objective	: Get the devices available on the current system.
 * Arguments	: WindowData *		- sdtaudio data structure
 * Return Value : None.
 */
void
SDtAuGetAvailableDevices(WindowData * wd)
{
	SAOutputFlags   outputs;
	SAInputFlags    inputs;

	/* Sensitize the available output ports */
	if (SDtAuGetAvailableOutputDevices(wd->sa_stream, &outputs)) {

		/* Speaker */
		if (outputs & SADevice_Speaker)
			XtSetSensitive(wd->devices_speaker, True);
		else
			XtSetSensitive(wd->devices_speaker, False);

		/* Headphone */
		if (outputs & SADevice_Headphone)
			XtSetSensitive(wd->devices_headphone, True);
		else
			XtSetSensitive(wd->devices_headphone, False);

		/* Line Out */
		if (outputs & SADevice_LineOut)
			XtSetSensitive(wd->devices_line_out, True);
		else
			XtSetSensitive(wd->devices_line_out, False);
	}

	/* Sensitize the available input ports */
	if (SDtAuGetAvailableInputDevices(wd->sa_stream, &inputs)) {

		/* Microphone */
		if (inputs & SADevice_Microphone)
			XtSetSensitive(wd->devices_mic, True);
		else
			XtSetSensitive(wd->devices_mic, False);

		/* Line In */
		if (inputs & SADevice_LineIn)
			XtSetSensitive(wd->devices_line_in, True);
		else
			XtSetSensitive(wd->devices_line_in, False);

		/* CD */
		if (inputs & SADevice_CD)
			XtSetSensitive(wd->devices_cd, True);
		else
			XtSetSensitive(wd->devices_cd, False);
	}
}

/* Function	: SDtAuGetCurrentlyUsedDevices()
 * Objective	: Get the devices that are in use by current system.
 * Arguments	: WindowData *		- sdtaudio data structure
 * Return Value : None.
 */
void
SDtAuGetCurrentlyUsedDevices(WindowData * wd)
{
	SAOutputFlags   outputs;
	SAInputFlags    inputs;

	/* Set the output ports */
	if (SDtAuGetCurrentOutputDevices(wd->sa_stream, &outputs)) {

		/* Speaker */
		if ((outputs & SADevice_Speaker) && 
		    (wd->options->deviceFlag & OUTPUT_SPEAKER))
			XtVaSetValues(wd->devices_speaker,
				      XmNset, True,
				      NULL);
		else
			XtVaSetValues(wd->devices_speaker,
				      XmNset, False,
				      NULL);

		/* Headphone */
		if ((outputs & SADevice_Headphone) &&
		    (wd->options->deviceFlag & OUTPUT_HEADPHONE))
			XtVaSetValues(wd->devices_headphone,
				      XmNset, True,
				      NULL);
		else
			XtVaSetValues(wd->devices_headphone,
				      XmNset, False,
				      NULL);

		/* Line Out */
		if ((outputs & SADevice_LineOut) &&
		    (wd->options->deviceFlag & OUTPUT_LINE_OUT))
			XtVaSetValues(wd->devices_line_out,
				      XmNset, True,
				      NULL);
		else
			XtVaSetValues(wd->devices_line_out,
				      XmNset, False,
				      NULL);
	}

	/* Set the input ports - remember, only one input port can
	 * be used to record data!
	 */
	if (SDtAuGetCurrentInputDevices(wd->sa_stream, &inputs)) {

		if (inputs & SADevice_Microphone) {

			XtVaSetValues(wd->devices_mic,
				      XmNset, True,
			 	      NULL);
			XtVaSetValues(wd->devices_line_in,
				      XmNset, False,
				      NULL);
			XtVaSetValues(wd->devices_cd,
				      XmNset, False,
				      NULL);

		} else if (inputs & SADevice_LineIn) {

			XtVaSetValues(wd->devices_line_in,
				      XmNset, True,
				      NULL);
			XtVaSetValues(wd->devices_mic,
				      XmNset, False,
				      NULL);
			XtVaSetValues(wd->devices_cd,
				      XmNset, False,
				      NULL);

		} else if (inputs & SADevice_CD) {

			XtVaSetValues(wd->devices_cd,
				      XmNset, True,
				      NULL);
			XtVaSetValues(wd->devices_mic,
				      XmNset, False,
				      NULL);
			XtVaSetValues(wd->devices_line_in,
				      XmNset, False,
				      NULL);
		} else {	

			/* No inputs selected */
			XtVaSetValues(wd->devices_mic,
				      XmNset, False,
				      NULL);
			XtVaSetValues(wd->devices_line_in,
				      XmNset, False,
				      NULL);
			XtVaSetValues(wd->devices_cd,
				      XmNset, False,
				      NULL);
		}
	}
}

#ifdef SELECTION
/* Function	: SDtAuDrawSel()
 * Objective	: Draw the highlight around the selected area of the wave
 *		  window, to let the user know that they selected a part
 *		  of the audio file.
 * Arguments	: WindowData *		- the sdtaudio data structure 
 * Return Value : None.
 */
void 
SDtAuDrawSel(WindowData * wd)
{
	Dimension       w_width, w_height;

	/* Get the wave window's dimensions. */
	XtVaGetValues(wd->wave_window,
		      XtNheight, &w_height,
		      XtNwidth, &w_width,
		      NULL);

	/* Now draw the selection. */
	XFillRectangle(XtDisplay(wd->wave_window), XtWindow(wd->wave_window),
		       wd->waveSelGC, wd->sel_start_x, WAVE_CURSOR_HEIGHT,
		       (unsigned int) abs(wd->sel_start_x - wd->sel_end_x), 
		       w_height - ww_font_height - WAVE_CURSOR_HEIGHT);
}
#endif

/* Function	: SDtAuCreateWavePixmap
 * Objective	: Create the pixmap used by the wave window.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if the pixmap was successfully created, 
 *		  False otherwise.
 */
Boolean 
SDtAuCreateWavePixmap(WindowData * wd)
{
	Dimension       width, height;
	int             depth;

	/* Destroy old pixmap, if it still exists. */
	if (wd->wave_pixmap != NULL) {
		XFreePixmap(XtDisplay(wd->wave_window),
			    wd->wave_pixmap);
		wd->wave_pixmap = NULL;
	}

	/* Create a pixmap based on the width, height and depth of
	 * the wave window drawing area.
	 */
	XtVaGetValues(wd->wave_window,
		      XmNwidth, &width,
		      XmNheight, &height,
		      XmNdepth, &depth,
		      NULL);
	wd->wave_pixmap = XCreatePixmap(XtDisplay(wd->wave_window),
					XtWindow(wd->wave_window),
					width, height, depth);

	if (wd->wave_pixmap == NULL)
		return(False);
	else
		return(True);
}

/* Function     : CalcWavByte()
 * Objective    : Calculate the y coordinates of the wave form graph
 *                for a *.wav sound file.
 * Arguments	: char *		- buffer of audio data retrieved
 *		  int			- index to the buffer
 *		  SAFile		- audio file info structure
 * Return Value : Y coordinate (in char)
 */
char CalcWavByte(char *buffer, int idx, SAFile *safile)
{
        int          sample_size = safile->format.sample_size;
	char	     ret_byte = 0;
	char         left_byte, right_byte;
	INT16        int_16, left_int_16, right_int_16;
	int          temp;

	if (safile->format.channels == 1) {

		/* Mono sound - *.WAV file are only encoded using
		 * PCM.  Check if we have an 8-bit or 16-bit *.wav
		 * file.
		 */
	       	if (sample_size == 1)
	       		/* 8-bit WAV file. */
	       		ret_byte = (char) ((unsigned) buffer[idx] - 
	       				  (unsigned) 128);
	       	else {
	       		/* 16-bit WAV file. */
	       		/* First byte is low order. */
	       		int_16 = buffer[idx];

	       		/* Second byte is high order. */
	       		int_16 |= (buffer[idx+1] << 8);

       			/* Normalize to char value. */
       			temp = int_16 * 128 / 32767;
       			ret_byte = (char) temp;
       		}
	} else {
		/* Stereo sound - *.WAV file are only encoded using
		 * PCM.  Check if we have an 8-bit or 16-bit *.wav
		 * file.
		 */
	       	if (sample_size == 1) {
	       		/* 8-bit WAV file. */
	       		left_byte = 
	       			(char) ((unsigned) buffer[idx] -
	       				 (unsigned) 128);
	       		right_byte =
	       			(char) ((unsigned) buffer[idx + 1] -
	       				 (unsigned) 128);

	       		/* Get average of left and right
	       		 * channels, and normalize to char.
	       		 */
	       		temp = (int) ((left_byte + right_byte) / 2);
	       		ret_byte = (char) temp;

       		} else {
       			/* 16-bit WAV file. */	
       			/* First byte is left channel, low order byte. */
	       		left_int_16 = buffer[idx];

	       		/* Second byte is left channel, high order byte. */
	       		left_int_16 |= (buffer[idx+1] << 8);

	       		/* Third byte is right channel, low order byte. */
		       	right_int_16 = buffer[idx+2];

		       	/* Fourth byte is right channel, high order byte. */
		       	right_int_16 |= (buffer[idx+3] << 8);

		       	/* Get average of left and right channels, and 
			 * normalize to char.
	       		 */
	       		int_16 = ((left_int_16 + right_int_16) / 2);
	       		temp = int_16 * 128 / 32767;
	       		ret_byte = (char) temp;
		}
	}
	return(ret_byte);
}

/* Function	: CalcSunByte()
 * Objective	: Calculate the y coordinate of the wave form graph.
 *		  This function is used by both *.au and *.aiff files.
 * Arguments	: char *		- buffer of audio data retrieved
 *		  int			- index to the buffer
 *		  SAFile		- audio file info structure
 * Return Value : Number (in char) of the y coordinate.
 */
char CalcSunByte(char *buffer, int idx, SAFile *safile)
{
        int          sample_size = safile->format.sample_size;
	char	     ret_byte = 0;
	char	     left_byte, right_byte;
	INT16        int_val, left_int_val, right_int_val;
	float	    *flt;
	double      *dbl;
	long         temp, big_int, left_big, right_big;
	static int   bit_count = 0;

	if (safile->format.channels == 1) {

		switch (safile->format.encoding) {

		case SAEncoding_ULAW:
			ret_byte = audio_u2c(buffer[idx]);
			break;
	
		case SAEncoding_ALAW:
			ret_byte = audio_a2c(buffer[idx]);
			break;
	
		case SAEncoding_LINEAR:
			if (sample_size == 1)
				ret_byte = (char) (buffer[idx]);
		        else if (sample_size == 2) {

				/* 16-bit audio file. */
				/* First byte is high order. */
				int_val = (buffer[idx] << 8);

				/* Second byte is low order. */
				int_val |= buffer[idx + 1];
				temp = int_val * 128 / 32767;
				ret_byte = (char) temp;

  			} else if (sample_size = 3) {

				/* 24-bit audio file. */
				/* Data stored in 3 bytes. */
				big_int = (long) (buffer[idx] << 16) | 
					  (long) (buffer[idx+1] << 8) |
					  (long) (buffer[idx+2]);
				
				/* Normalize data to size char */
				ret_byte = (char) (big_int  * 128 / 0xFFFFFF);
			} else {

				/* 32-bit audio file. */
				/* Data stored in 4 bytes. */
				big_int = (long) (buffer[idx] << 24) |
					  (long) (buffer[idx+1] << 16) |
					  (long) (buffer[idx+2] << 8) |
					  (long) (buffer[idx+3]);

				/* Normalize data to size char */
				ret_byte = (char) (big_int * 128 / 0xFFFFFFFF);
			}
			break;

		case SAEncoding_FLOAT:
			/* EXTREMELY RARE FORMAT.  NO SUPPORT AT THIS TIME. */
			if (sample_size == 4) {

				/* Data store in 4 bytes. */
				/* flt = (short *) buffer; */
			} else {

				/* Data stored in 8 bytes. */
				/* dbl = (short *) buffer; */
			}
			break;

		case SAEncoding_G721: 
			/* Old sparc hardware.  Data stored every 4 bits. */
			if (bit_count == 0)
				ret_byte = (char) ((buffer[idx] >> 4) * 128
						   / 0xF);
			else
				ret_byte = (char) ((buffer[idx] & 0x0F) * 128
						   / 0xF);
			/* Toggle bit_count between 0 and 1. */
			bit_count++;
			bit_count %= 2;
			break;

		case SAEncoding_G723:  
			/* Old sparc hardware. */
			if (sample_size == 3) {

				/* First, check if idx is divisible by 3.
				 * If not, increment it till it is.
				 */
				while ((idx % 3 != 0) && (idx < sizeof(buffer)))
					idx++;

				/* Data stored every 3 bits. */
				ret_byte = (char) ((buffer[idx] >> 5) * 
						   128 / 0x07);

			} else {

				/* First, check if idx is divisible by 5.
				 * If not, increment it till it is.
				 */
				while ((idx % 5 != 0) && (idx < sizeof(buffer)))
					idx++;

				/* Data stored every 5 bits. */
				ret_byte = (char) ((buffer[idx] >> 3) 
						   * 128 / 0x1F);
			}
			break;

		default:
			ret_byte = (char) NULL;
		}

	} else {

		/* Channels is two - sound files with more than two
		 * channels are extremely rare/non-existent, at least,
		 * for the types that we support.
		 */

		switch (safile->format.encoding) {

		case SAEncoding_ULAW:
			ret_byte = (char) ((audio_u2c(buffer[idx]) + 
				           audio_u2c(buffer[idx + 1])) / 2);
			break;
	
		case SAEncoding_ALAW:
			ret_byte = (char) ((audio_a2c(buffer[idx]) + 
					   audio_a2c(buffer[idx + 1])) / 2);
			break;
	
		case SAEncoding_LINEAR:
			if (sample_size == 1) {

				ret_byte = (char) ((buffer[idx] + 
					           buffer[idx + 1]) / 2);

			} else if (sample_size == 2) {

				/* Normalize data to size char */
				left_int_val = (buffer[idx] << 8) |
					       buffer[idx+1];
				right_int_val = (buffer[idx+2] << 8) | 
						 buffer[idx+3];
				int_val = (unsigned) (left_int_val + 
						      right_int_val / 2);
				temp = int_val * 128 / 65535;
				ret_byte = (char) temp;

  			} else if (sample_size = 3) {

				/* Data stored in 3 bytes. */
				left_big = (long) (buffer[idx] << 16) | 
				           (long) (buffer[idx+1] << 8) |
					   (long) (buffer[idx+2]);
				right_big = (long) (buffer[idx+3] << 16) | 
				            (long) (buffer[idx+4] << 8) |
					    (long) (buffer[idx+5]);
				big_int = left_big + right_big / 2;

				/* Normalize data to size char */
				temp = big_int * 128 / 0xFFFF;
				ret_byte = (char) temp;

			} else {

				/* Data stored in 4 bytes. */
				left_big = (long) (buffer[idx] << 24) |
					   (long) (buffer[idx+1] << 16) |
					   (long) (buffer[idx+2] << 8) |
					   (long) (buffer[idx+3]);
				right_big = (long) (buffer[idx+4] << 24) |
					    (long) (buffer[idx+5] << 16) |
					    (long) (buffer[idx+6] << 8) |
					    (long) (buffer[idx+7]);
				big_int = left_big + right_big / 2;

				/* Normalize data to size char */
				temp = big_int * 128 / 0xFFFFFFFF;
				ret_byte = (char) temp;
			}
			break;

		default:
			ret_byte = (char) NULL;
		}
	}
	return(ret_byte);
}

/* Function	: SDtAuDrawWaveCursor()
 * Objective	: Draw the play point cursor associated with the wave window.
 * Arguments	: WindowData *		- the sdtaudio data structure.
 *		  short			- the new x coordinate of the
 *					  play point cursor
 * Return Value	: None.
 */
void 
SDtAuDrawWaveCursor(WindowData *wd, short new_x)
{
	Dimension	w_width, w_height;

	if (wd->wave_pixmap == NULL)
		return;

	/* Get the wave window width and height. */
	XtVaGetValues(wd->wave_window,
		      XtNwidth, &w_width,
		      XtNheight, &w_height,
		      NULL);

	/* Erase the old cursor. */
	wd->old_cursor[0] = wd->new_cursor[0];
	XFillPolygon(XtDisplay(wd->wave_window),
		     wd->wave_pixmap, wd->waveReverseGC,
		     wd->old_cursor, 3, Convex, CoordModePrevious);

	/* Now draw the new cursor. */
	wd->new_cursor[0].x = new_x;
	XFillPolygon(XtDisplay(wd->wave_window),
		     wd->wave_pixmap, wd->waveForeGrndGC,
		     wd->new_cursor, 3, Convex, CoordModePrevious);
	XCopyArea(XtDisplay(wd->wave_window), wd->wave_pixmap,
		  XtWindow(wd->wave_window), wd->wavePMGC,
		  0, 0, w_width, 10, 0, 0);
}

/* Function	: SDtAuDrawWave()
 * Objective	: Draw the audio wave form in the wave window.
 * Arguments	: WindowData *		- the sdtaudio data structure.
 * Return Value	: None.
 */
void 
SDtAuDrawWave(WindowData * wd, int w_height, int w_width)
{
	char           *abuffer = NULL;
	char           *conv_buff = NULL;
	int             bufferLength = 0;
	Boolean         done = False;
	long            total_bytes = 0;
	int             x = 0;
	int             y_new = 0;
	int             y_last = w_height / 2 - ww_font_height;
	int             conv_size = 0, i = 0, tmp = 0;
	float           percent_of_buff = 0;
	int             window_slice = 0;
	float           bytes_per_pixel = 0;
	char            my_byte;
	int             return_nbytes = 0;
	SAFile          safile, converted;
	int		samples_per_unit = 0;
	SAError         error;
	int		buffer_size = 0, frame_size = 0;
        off_t		tmp_data_size = 0;

	/* If audio is recording, just return, since there is
	 * no wave to draw!
	 */
	if (wd->play_state == STATE_RECORD)
		return;

	/* Prabhat : Check for validity of safile */
	if (wd->name == NULL)
		return;

	/* Open up the file, so that we can read data points. */
	SAFileOpen(wd->name, &safile, &error);

	/* Prabhat :: Prepare for G721/G723 if reqd */
	samples_per_unit = safile.format.samples_per_unit;
	frame_size = SAFormatFrameSize(&safile.format);
	buffer_size = safile.format.sample_rate * frame_size;

	/* Make buffer allocations */
	abuffer = (char *) malloc(buffer_size);
	if (abuffer == NULL) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE,
			    catgets(msgCatalog, 3, 59, "Audio - Error"),
			    DIALOG_TEXT,
			    catgets(msgCatalog, 3, 46, "Could not allocate memory for operation."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	} 

	return_nbytes = buffer_size;

	/* Some audio files may not have the data size field set. */
	tmp_data_size = SAFileGetDataSize(&safile);

	/* Need to check - some formats are not byte based, but bit
	 * based!
	 */
	if (safile.format.encoding == SAEncoding_G721 ||
	    safile.format.encoding == SAEncoding_G723) {

		/* Keep the file type as *.AU. */
		converted.filetype = SASunType;
		SAConvertFormat(safile.format.encoding, 
				safile.format.sample_size,
				safile.format.channels,
				safile.format.samples_per_unit,
				converted.filetype, &converted.format,
				&error);
		converted.format.sample_rate = safile.format.sample_rate;
		converted.format.samples_per_unit = 
			safile.format.samples_per_unit;
		converted.data_size = safile.data_size;
		converted.header_size = safile.header_size;

		/* Create a conversion buffer - need to convert the
		 * ADPCM encoding to linear, so that values can be
		 * interpreted correctly.
		 */
		bufferLength = 2 * (buffer_size * frame_size /
				SAFormatFrameSize(&converted.format));
		conv_buff = (char *) malloc(bufferLength);
		if (conv_buff == NULL) {
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE,
				    catgets(msgCatalog, 3, 59, "Audio - Error"),
				    DIALOG_TEXT,
				    catgets(msgCatalog, 3, 46, "Could not allocate memory for operation."),
				    BUTTON_IDENT, ANSWER_ACTION_1,
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			free(abuffer);
			return;
		}
		conv_size = bufferLength;

	} else {
		/* Allocate a buffer to read in audio data. The size
		 * should be about 25% of data size - this percentage
		 * is arbitrarily chosen - the important thing is that
		 * data must fit within the window width.
		 */
		 bufferLength =  (int) (0.25 * tmp_data_size);
	}

       	/* Calc the x scale as a percentage of buffer length. */
       	percent_of_buff = (float) bufferLength / (float) tmp_data_size;
       	if (percent_of_buff < 1)
		/* the data is smaller than one buffer */
		window_slice = (int) ((float) w_width * 
				(float) percent_of_buff / 
				(float) samples_per_unit);
       	else
		window_slice = w_width;

	/* Read the audio file data. */
	while (!done) {

		SAFileRead(&safile, abuffer, buffer_size,
			   &return_nbytes, &error);
		total_bytes += (return_nbytes);

		/* Convert the data to linear instead of adaptive. */
		if (safile.format.encoding == SAEncoding_G721 ||
		    safile.format.encoding == SAEncoding_G723) {

			SAConvertData(&safile.format, &converted.format, 
				      abuffer, return_nbytes, conv_buff, 
				      &conv_size, &error);

			/* Recalculate the x scale */
			percent_of_buff = (float) bufferLength /
				(float) conv_size;
			if (percent_of_buff < 1)
				/* the data is smaller than one buffer */
				window_slice = (int) ((float) w_width * 
						      (float) percent_of_buff / 
						      (float) samples_per_unit);
			else
				window_slice = w_width;
			
			bytes_per_pixel = (float) conv_size /
				(float) samples_per_unit /
				(float) window_slice;
		} else
			bytes_per_pixel = (float) return_nbytes / 
				(float) samples_per_unit / 
				(float) window_slice;

		/* Did we get all data? */
		if (return_nbytes == 0)
			done = True;

		/* loop through returned buffer */
		for (i = 0; i < window_slice; ++i) {

			tmp = (int) ((float) (bytes_per_pixel) * (float) i);

			if (safile.filetype == SAWavType) 
				my_byte = CalcWavByte(abuffer, tmp, 
						      &safile);
			else if (safile.format.encoding == SAEncoding_G721 ||
				safile.format.encoding == SAEncoding_G723) {

				my_byte = CalcSunByte(conv_buff, tmp,
						      &converted);
			} else
				my_byte = CalcSunByte(abuffer, tmp, 
						      &safile);

			y_new = (int) ((float) w_height * 
				       (float) (my_byte + 128) / 
				       (float) 256);
			XDrawLine(XtDisplay(wd->wave_window),
				  wd->wave_pixmap, wd->waveForeGrndGC,
				  x, y_last, x, y_new);
			y_last = y_new;
			x++;
		}
	}
	SAFileClose(&safile, &error);
	free(abuffer);
	if (safile.format.encoding == SAEncoding_G721 ||
	    safile.format.encoding == SAEncoding_G723) {

		free(conv_buff);
	}
}

/* Function	: SDtAuClearInfo()
 * Objective	: Clear the information displayed in the info dialog.
 * Arguments	: WindowData *		- the sdtaudio data structure.
 * Return Value : None.
 */
void
SDtAuClearInfo(WindowData *wd)
{
    XmString xmstring;

    xmstring = XmStringCreateLocalized(" ");
    XtVaSetValues(wd->info_name,
		  XmNlabelString, xmstring,
		  NULL);
    XtVaSetValues(wd->info_size,
		  XmNlabelString, xmstring,
		  NULL);
    XtVaSetValues(wd->info_length,
		  XmNlabelString, xmstring,
		  NULL);
    XtVaSetValues(wd->info_channels,
		  XmNlabelString, xmstring,
		  NULL);
    XtVaSetValues(wd->info_sample_rate,
		  XmNlabelString, xmstring,
		  NULL);
    XtVaSetValues(wd->info_encoding,
		  XmNlabelString, xmstring,
		  NULL);
    XmStringFree(xmstring);
}

/*--------------------------------- PUBLIC -----------------------------------*/

/* Function	: SDtAuSetDeviceState()
 * Objective	: Set the system to either use or not use the speaker as
 *		  the output device, based on the device flag passed.
 * Arguments	: WindowData *		- the sdtaudio structure.
 * Return Value : Boolean True if the device could be set to the specified
 *		  state, False otherwise.
 */
Boolean
SDtAuSetDeviceState(WindowData *wd)
{
	SABoolean	flag;
	SAError		error;

	/* Check for valid sun audio stream. */
	if (!wd->sa_stream) {

		/* Open up a sun audio stream if it is not defined. */
		wd->sa_stream = SAStreamOpen(NULL, &error);
		if (!wd->sa_stream)
			return(False);
	}

	/* Set the speaker device. */
	if (wd->options->deviceFlag & OUTPUT_SPEAKER)
		flag = SATrue;
	else
		flag = SAFalse;
	if (!SADeviceSetSpeakerPort(wd->sa_stream->device, flag, &error))
		return(False);

	/* Set the headphone device. */
	if (wd->options->deviceFlag & OUTPUT_HEADPHONE)
		flag = SATrue;
	else
		flag = SAFalse;
	if (!SADeviceSetHeadphonePort(wd->sa_stream->device, flag, &error))
		return(False);

	/* Set the line out device. */
	if (wd->options->deviceFlag & OUTPUT_LINE_OUT)
		flag = SATrue;
	else
		flag = SAFalse;
	if (!SADeviceSetLineOutPort(wd->sa_stream->device, flag, &error))
		return(False);

	/* Set the microphone device */
	if (wd->options->deviceFlag & INPUT_MIC)
		flag = SATrue;
	else
		flag = SAFalse;
	if (!SADeviceSetMicrophonePort(wd->sa_stream->device, flag, &error))
		return(False);

	/* Set the line in device */
	if (wd->options->deviceFlag & INPUT_LINE_IN)
		flag = SATrue;
	else
		flag = SAFalse;
	if (!SADeviceSetLineInPort(wd->sa_stream->device, flag, &error))
		return(False);

	/* Set the cd device */
	if (wd->options->deviceFlag & INPUT_CD)
		flag = SATrue;
	else
		flag = SAFalse;
	if (!SADeviceSetCDPort(wd->sa_stream->device, flag, &error))
		return(False);

	return(True);
}

/* Function	: SDtAuGetRecordValues()
 * Objective	: Inquire from the user the recording settings to use
 *		  when performing a record operation.  This function
 *		  is invoked when two conditions are met: the user has
 *		  set "Ask me when I start recording" in their options,
 *		  and the user has pressed the record button.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : Boolean True if recording settings were retrieved from
 *		  the user, False otherwise.  False does not necessarily
 *		  indicate a failure.  The user may have decided to cancel
 *		  the recording operation.
 */
Boolean
SDtAuGetRecordValues(WindowData *wd)
{
	XtAppContext   cntxt;

	/* Get the application context. */
	cntxt = XtDisplayToApplicationContext(XtDisplay(wd->mainWindow));

	/* Bring up the dialog. */
	if (!wd->recval_dlog)
		SDtAuCreateRecordValuesDlog(wd);
	XtManageChild(wd->recval_dlog);
	
	/* Wait until the user has set the recording values, before
	 * continuing with the record.
	 */
	wd->wait_for_dialog = True;
	while(wd->wait_for_dialog)
		XtAppProcessEvent(cntxt, XtIMAll);

	if (wd->recval_set) {

		/* User set the values.  Reset the variable for
		 * next time, and return True to start recording.
		 */
		wd->recval_set = False;
		return(True);
	} else
		/* User cancelled operation. Return false and
		 * don't record.
		 */
		return(False);
}

/* Function	: SDtAudioSetPlayVol()
 * Objective	: Set the play volume to the desired linear level.
 * Input	: SAStream *	- pointer to sun audio stream variable
 *		: int		- desired volume level, ranging from VOL_MIN
 *				  through VOL_MAX
 *		  SAError *	- sun audio error variable, in case operation
 *				  failed
 * Return values: True if volume was successfully set to the desired level,
 *		  false otherwise. 
 */
Boolean
SDtAudioSetPlayVol(SAStream *stream, int level, SAError *error)
{
	SAOutputFlags     port;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, error);
		if (!stream)
			return(False);
	}

	/* Check for valid output port device. */
	if (stream->device->play_ports == 0) {
		if (SADeviceGetOutputPorts(stream->device, &port, error))
			stream->device->play_ports = port;
		else
			return(False);
	}

	/* Set the device volume. */
	return(SADeviceSetLinearPlayVolume(stream->device, (double) level, 
					   (double) VOL_MIN, (double) VOL_MAX, 
					    error));
}

/* Function	: SDtAudioGetPlayVol()
 * Objective	: Get the play volume to the desired linear level.
 * Input	: SAStream *	- pointer to sun audio stream variable
 *		: double *	- variable that will hold the volume
 *		  SAError *	- sun audio error variable, in case operation
 *				  failed
 * Return values: True if volume was successfully retrieved, false otherwise. 
 */
Boolean
SDtAudioGetPlayVol(SAStream *stream, double *level, SAError *error)
{
	SAOutputFlags     port;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, error);
		if (!stream)
			return(False);
	}

	/* Check for valid output port device. */
	if (stream->device->play_ports == 0) {
		if (SADeviceGetOutputPorts(stream->device, &port, error))
			stream->device->play_ports = port;
		else
			return(False);
	}

	/* Get the device volume. */
	return(SADeviceGetLinearPlayVolume(stream->device, level, 
					   (double) VOL_MIN, (double) VOL_MAX, 
					   error));
}

/* Function	: SDtAudioSetRecVol()
 * Objective	: Set the record volume to the desired linear level.
 * Input	: SAStream *	- pointer to sun audio stream variable
 *		: int		- desired volume level, ranging from VOL_MIN
 *				  through VOL_MAX
 *		  SAError *	- sun audio error variable, in case operation
 *				  failed
 * Return values: True if volume was successfully set to the desired level,
 *		  false otherwise. 
 */
Boolean
SDtAudioSetRecVol(SAStream *stream, int level, SAError *error)
{
	SAOutputFlags     port;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, error);
		if (!stream)
			return(False);
	}

	/* Check for valid output port device. */
	if (stream->device->play_ports == 0) {
		if (SADeviceGetOutputPorts(stream->device, &port, error))
			stream->device->play_ports = port;
		else
			return(False);
	}

	/* Set the device volume. */
	return(SADeviceSetLinearRecordVolume(stream->device, (double) level, 
					     (double) VOL_MIN, 
					     (double) VOL_MAX, error));
}

/* Function	: SDtAudioGetRecVol()
 * Objective	: Get the play volume to the desired linear level.
 * Input	: SAStream *	- pointer to sun audio stream variable
 *		: double *	- variable that will hold the volume
 *		  SAError *	- sun audio error variable, in case operation
 *				  failed
 * Return values: True if volume was successfully retrieved, false otherwise. 
 */
Boolean
SDtAudioGetRecVol(SAStream *stream, double *level, SAError *error)
{
	SAOutputFlags     port;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, error);
		if (!stream)
			return(False);
	}

	/* Check for valid output port device. */
	if (stream->device->play_ports == 0) {
		if (SADeviceGetOutputPorts(stream->device, &port, error))
			stream->device->play_ports = port;
		else
			return(False);
	}

	/* Get the device volume. */
	return(SADeviceGetLinearRecordVolume(stream->device, level, 
					     (double) VOL_MIN, 
					     (double) VOL_MAX, error));
}

/* Function	: SDtAudioSetMuteState()
 * Objective	: Set the mute state of the device.
 * Input	: SAStream *	- pointer to sun audio stream variable
 *		: SABoolean	- desired mute state
 *		  SAError *	- sun audio error variable, in case operation
 *				  failed
 * Return values: True if mute state was successfully set, false otherwise. 
 */
Boolean
SDtAudioSetMuteState(SAStream *stream, SABoolean mute, SAError *error)
{
	SAOutputFlags     port;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, error);
		if (!stream)
			return(False);
	}

	/* Check for valid output port device. */
	if (stream->device->play_ports == 0) {
		if (SADeviceGetOutputPorts(stream->device, &port, error))
			stream->device->play_ports = port;
		else
			return(False);
	}

	/* Set the device volume. */
	if (mute == SATrue)
		return(SADeviceSetMuteOn(stream->device, error));
	else
		return(SADeviceSetMuteOff(stream->device, error));
}

/* Function	: SDtAudioGetMuteState()
 * Objective	: Get the mute state of the audio device.
 * Input	: SAStream *	- pointer to sun audio stream variable
 *		  SABoolean *	- variable that will hold the mute state
 *		  SAError *	- sun audio error variable, in case operation
 *				  failed
 * Return values: True if mute state was successfully retrieved, false 
 *		  otherwise. 
 */
Boolean
SDtAudioGetMuteState(SAStream *stream, SABoolean *mute, SAError *error)
{
	SAOutputFlags     port;

	/* Check for valid sun audio stream. */
	if (!stream) {

		/* Open up a sun audio stream if it is not defined. */
		stream = SAStreamOpen(NULL, error);
		if (!stream)
			return(False);
	}

	/* Check for valid output port device. */
	if (stream->device->play_ports == 0) {
		if (SADeviceGetOutputPorts(stream->device, &port, error))
			stream->device->play_ports = port;
		else
			return(False);
	}

	/* Get the device mute state. */
	return(SADeviceGetMute(stream->device, mute, error));
}

/* Function	: SDtAuUpdateDevices()
 * Objective	: Update what items are selectable by the user, and what
 *		  items should be currently set in the Devices pane,
 *		  so that the pane reflects the true state of the
 *		  system's audio hardware.
 * Arguments	: WindowData *		- sdtaudio data structure
 * Return Value : None.
 */
void
SDtAuUpdateDevices(WindowData * wd)
{
	SDtAuGetAvailableDevices(wd);
	SDtAuGetCurrentlyUsedDevices(wd);
}

/* Function	: SDtAuSaveOptions()
 * Objective	: Save the user's preferences into the resource file.
 * Arguments	: WindowData *		- sdtaudio data structure
 * Return value : None.
 */
void 
SDtAuSaveOptions(WindowData *wd)
{
	char            opts_path[PATH_MAX];
	char           *home_path;
	char            int_str[16];	/* Integer, in a string form. */
	Boolean         bool_val;	/* Boolean value. */
	int             int_val;	/* Integer value. */
	Widget          menu_val;	/* Menu value. */

	/* Start with wave view or no? */
	XtVaGetValues(wd->wave_view_rb,
		      XmNset, &bool_val,
		      NULL);
	wd->options->showWave = bool_val;
	if (bool_val == True)
		XrmPutStringResource(&sdta_options_rdb, AUD_SHOW_WAVE, "True");
	else
		XrmPutStringResource(&sdta_options_rdb, AUD_SHOW_WAVE, 
				     "False");

	/* Should tool play file on load? */
	XtVaGetValues(wd->auto_play_open,
		      XmNset, &bool_val,
		      NULL);
	wd->options->autoPlayOnOpen = bool_val;
	if (bool_val == True)
		XrmPutStringResource(&sdta_options_rdb, AUD_AUTO_PLAY_OPEN,
				     "True");
	else
		XrmPutStringResource(&sdta_options_rdb, AUD_AUTO_PLAY_OPEN,
				     "False");

#ifdef SELECTION
	/* Should tool play selection? */
	XtVaGetValues(wd->auto_play_select,
		      XmNset, &bool_val,
		      NULL);
	wd->options->autoPlayOnSelect = bool_val;
	if (bool_val == True)
		XrmPutStringResource(&sdta_options_rdb, AUD_AUTO_PLAY_SELECT,
				     "True");
	else
		XrmPutStringResource(&sdta_options_rdb, AUD_AUTO_PLAY_SELECT,
				     "False");
#endif

	/* Default play volume. */
	XtVaGetValues(wd->play_vol_scale,
		      XmNvalue, &int_val,
		      NULL);
	wd->options->playVolume = int_val;
	sprintf(int_str, "%d", int_val);
	XrmPutStringResource(&sdta_options_rdb, AUD_PLAY_VOLUME, int_str);

	/* Default play balance. */
	XtVaGetValues(wd->play_bal_scale,
		      XmNvalue, &int_val,
		      NULL);
	wd->options->playBalance = int_val;
	sprintf(int_str, "%d", int_val);
	XrmPutStringResource(&sdta_options_rdb, AUD_PLAY_BALANCE, int_str);

	/* Default recording volume. */
	XtVaGetValues(wd->record_vol_scale,
		      XmNvalue, &int_val,
		      NULL);
	wd->options->recordVolume = int_val;
	sprintf(int_str, "%d", int_val);
	XrmPutStringResource(&sdta_options_rdb, AUD_REC_VOLUME, int_str);

	/* Default record balance. */
	XtVaGetValues(wd->record_bal_scale,
		      XmNvalue, &int_val,
		      NULL);
	wd->options->recordBalance = int_val;
	sprintf(int_str, "%d", int_val);
	XrmPutStringResource(&sdta_options_rdb, AUD_REC_BALANCE, int_str);

	/* Recording quality. */
	XtVaGetValues(wd->recqual_option,
		      XmNmenuHistory, &menu_val,
		      NULL);
	if (menu_val == wd->cd_menu_item) {
		wd->options->recordingQuality = SDTA_CD_QUALITY;
		XrmPutStringResource(&sdta_options_rdb, AUD_REC_QUALITY,
				     AUD_RSRC_CD);
	}
	else if(menu_val == wd->dat_menu_item) {
		wd->options->recordingQuality = SDTA_DAT_QUALITY;
		XrmPutStringResource(&sdta_options_rdb, AUD_REC_QUALITY,
				     AUD_RSRC_DAT);
	}
	else {
		wd->options->recordingQuality = SDTA_VOICE_QUALITY;
		XrmPutStringResource(&sdta_options_rdb, AUD_REC_QUALITY,
				     AUD_RSRC_VOICE);
	}

	/* Recording format. */
	XtVaGetValues(wd->audformat_option,
		      XmNmenuHistory, &menu_val,
		      NULL);
	if (menu_val == wd->wav_menu_item) {
		wd->options->recordingFormat = SAWavType;
		XrmPutStringResource(&sdta_options_rdb, AUD_REC_FILE,
				     AUD_RSRC_WAV);
	}
	else if (menu_val == wd->aiff_menu_item) {
		wd->options->recordingFormat = SAAiffType;
		XrmPutStringResource(&sdta_options_rdb, AUD_REC_FILE,
				     AUD_RSRC_AIFF);
	}
	else {
		wd->options->recordingFormat = SASunType;
		XrmPutStringResource(&sdta_options_rdb, AUD_REC_FILE,
				     AUD_RSRC_AU);
	}

	/* When recording, should tool use defaults, or should it
	 * inquire values from the user?
	 */
	XtVaGetValues(wd->selected_rb,
		      XmNset, &bool_val,
		      NULL);
	wd->options->useDefaultRecValues = bool_val;
	if (bool_val == True)
		XrmPutStringResource(&sdta_options_rdb, AUD_USE_REC_VALUE,
				     "True");
	else
		XrmPutStringResource(&sdta_options_rdb, AUD_USE_REC_VALUE,
				     "False");
	
	/* Default device flag - combination of input device and output
	 * device.
	 */
	int_val = 0;
	/* Check if speaker was selected for output. */
	XtVaGetValues(wd->devices_speaker,
		      XmNset, &bool_val,
		      NULL);
	if (bool_val)
		int_val |= OUTPUT_SPEAKER;

	/* Check if headphone was selected for output. */
	XtVaGetValues(wd->devices_headphone,
		      XmNset, &bool_val,
		      NULL);
	if (bool_val)
		int_val |= OUTPUT_HEADPHONE;

	/* Check if line out was selected for output. */
	XtVaGetValues(wd->devices_line_out,
		      XmNset, &bool_val,
		      NULL);
	if (bool_val)
		int_val |= OUTPUT_LINE_OUT;

	/* Check which input - mic, line in, or CD; unlike output devices,
	 * can only have one input device.
	 */
	XtVaGetValues(wd->devices_cd,
		      XmNset, &bool_val,
		      NULL);
	if (bool_val)
		int_val |= INPUT_CD;
	else {

		XtVaGetValues(wd->devices_line_in,
			      XmNset, &bool_val,
			      NULL);
		if (bool_val)
			int_val |= INPUT_LINE_IN;
		else
			int_val |= INPUT_MIC;
	}

	/* Set the resource. */
	wd->options->deviceFlag = int_val;
	sprintf(int_str, "%d", int_val);
	XrmPutStringResource(&sdta_options_rdb, AUD_DEVICE_FLAGS, int_str);

	/* Save the resources to disk.  The resource file is
	 * located in the directory
	 *	$HOME/.dt/sessions
	 * in the file
	 *	Sdtaudio
	 */
	home_path = getenv("HOME");
	if (home_path == NULL) {

		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_WARNING,
			    DIALOG_TITLE, catgets(msgCatalog, 3, 59, 
						  "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 1, 
						 "Could not save options."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		return;
	} 

	strcpy(opts_path, home_path);
	strcat(opts_path, PROPS_FILE);
	XrmPutFileDatabase(sdta_options_rdb, opts_path);

	/* Did user change devices to use? */
	SDtAuSetDeviceState(wd);
}

/* Function	: SDtAuInitOptions()
 * Objective	: Initialize the tools default options.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value	: None.
 */
void
SDtAuInitOptions(WindowData * wd)
{
	char           *str_type[20];
	XrmValue        value;
	char            buffer[32];
	long            tmp_int;

	/* Allocate space for the options that we need to track. */
	wd->options = (AudioProps *) malloc(sizeof(AudioProps));
	if (wd->options == NULL) {
		fprintf(stderr, "%s\n", 
			catgets(msgCatalog, 3, 46, "Could not allocate memory for operation."));
		exit(1);
	}

	/* Get the view type.  showWave defaults to False. */
	wd->options->showWave = False;
	if (XrmGetResource(sdta_options_rdb, AUD_SHOW_WAVE, AUD_SHOW_WAVE,
			   str_type, &value) == TRUE) {

		if (strncmp(value.addr, "True", (int) value.size) == 0)
			wd->options->showWave = True;
	}

	/* Get auto play on open resource, default is True. */
	wd->options->autoPlayOnOpen = True;
	if (XrmGetResource(sdta_options_rdb, AUD_AUTO_PLAY_OPEN, 
			   AUD_AUTO_PLAY_OPEN, str_type, &value) == TRUE) {

		if (strncmp(value.addr, "False", (int) value.size) == 0)
			wd->options->autoPlayOnOpen = False;
	}

#ifdef SELECTION
	/* Get auto play on select resource, default is False. */
	wd->options->autoPlayOnSelect = False;
	if (XrmGetResource(sdta_options_rdb, AUD_AUTO_PLAY_SELECT,
		           AUD_AUTO_PLAY_SELECT, str_type, &value) == TRUE) {

		if (strncmp(value.addr, "True", (int) value.size) == 0)
			wd->options->autoPlayOnSelect = True;
	}
#endif

	/* Get the play volume level resource, default is 50. */
	wd->options->playVolume = DEFAULT_VOLUME;
	if (XrmGetResource(sdta_options_rdb, AUD_PLAY_VOLUME, AUD_PLAY_VOLUME,
			   str_type, &value) == TRUE) {

		(void) strncpy(buffer, value.addr, (int) value.size);
		tmp_int = atol(buffer);
		if (tmp_int > VOL_MAX)
			tmp_int = VOL_MAX;

		if (tmp_int < VOL_MIN)
			tmp_int = VOL_MIN;

		wd->options->playVolume = (int) tmp_int;
	}

	/* Set the play volume level based on the value in options. */
	wd->play_vol = wd->options->playVolume;

	/* Get the play balance, default is 0, i.e., middle */
	wd->options->playBalance = DEFAULT_BALANCE;
	if (XrmGetResource(sdta_options_rdb, AUD_PLAY_BALANCE, AUD_PLAY_BALANCE,
			   str_type, &value) == TRUE) {

		(void) strncpy(buffer, value.addr, (int) value.size);
		tmp_int = atol(buffer);
		if (tmp_int > 50)
			tmp_int = 50;

		if (tmp_int < -50)
			tmp_int = -50;

		wd->options->playBalance = (int) tmp_int;
	}

	/* Set the application balance level based on the value in options. */
	wd->play_bal = wd->options->playBalance;

	/* Get the record volume level resource, default is 50. */
	wd->options->recordVolume = DEFAULT_VOLUME;
	if (XrmGetResource(sdta_options_rdb, AUD_REC_VOLUME, AUD_REC_VOLUME,
			   str_type, &value) == TRUE) {

		(void) strncpy(buffer, value.addr, (int) value.size);
		tmp_int = atol(buffer);
		if (tmp_int > 100)
			tmp_int = 100;

		if (tmp_int < 0)
			tmp_int = 0;

		wd->options->recordVolume = (int) tmp_int;
	}

	/* Set the recording volume level based on the value in options. */
	wd->rec_vol = wd->options->recordVolume;

	/* Get the recording balance, default is 0, i.e., middle */
	wd->options->recordBalance = 0;
	if (XrmGetResource(sdta_options_rdb, AUD_REC_BALANCE, AUD_REC_BALANCE,
			   str_type, &value) == TRUE) {

		(void) strncpy(buffer, value.addr, (int) value.size);
		tmp_int = atol(buffer);
		if (tmp_int > 50)
			tmp_int = 50;

		if (tmp_int < -50)
			tmp_int = -50;

		wd->options->recordBalance = (int) tmp_int;
	}

	/* Set the recording balance level based on the value in options. */
	wd->rec_bal = wd->options->recordBalance;

	/* Get the recording quality, default is voice. */
	wd->options->recordingQuality = SDTA_VOICE_QUALITY;
	if (XrmGetResource(sdta_options_rdb, AUD_REC_QUALITY, AUD_REC_QUALITY,
			   str_type, &value) == TRUE) {

		if (strncmp(value.addr, AUD_RSRC_CD, (int) value.size) == 0)
			wd->options->recordingQuality = SDTA_CD_QUALITY;
		else if (strncmp(value.addr, AUD_RSRC_DAT, (int) value.size) == 0)
			wd->options->recordingQuality = SDTA_DAT_QUALITY;
	}

	/* Get the recording format, default is AU. */
	wd->options->recordingFormat = SASunType;
	if (XrmGetResource(sdta_options_rdb, AUD_REC_FILE, AUD_REC_FILE,
			   str_type, &value) == TRUE) {

		if (strncmp(value.addr, AUD_RSRC_WAV, (int) value.size) == 0)
			wd->options->recordingFormat = SAWavType;
		else if (strncmp(value.addr, AUD_RSRC_AIFF, (int) value.size) == 0)
			wd->options->recordingFormat = SAAiffType;
	}

	/* Get the use default recording options resource, default is TRUE. */
	wd->options->useDefaultRecValues = True;
	if (XrmGetResource(sdta_options_rdb, AUD_USE_REC_VALUE, AUD_USE_REC_VALUE,
			   str_type, &value) == TRUE) {

		if (strncmp(value.addr, "False", (int) value.size) == 0)
			wd->options->useDefaultRecValues = False;
	}

	/* Get the device flags, which will determine what our input
	 * and output devices are.  Default devices are microphone and
	 * speaker for input and output respectively.
	 */
	wd->options->deviceFlag = DEFAULT_DEVICE_FLAG;
	if (XrmGetResource(sdta_options_rdb, AUD_DEVICE_FLAGS, AUD_DEVICE_FLAGS,
			   str_type, &value) == TRUE) {

		(void) strncpy(buffer, value.addr, (int) value.size);
		tmp_int = atol(buffer);

		wd->options->deviceFlag = (int) tmp_int;
	}

	/* Prabhat : Look into freeing of str_type */
}

/*
 * Function	:	SDtAuGetOptions()
 * Objective	:	Get the user's preferences for the tool from the
 *			specified resource file.  If resource file does
 *			not exist, then use the default resources defined
 *			in the global variable default_options.
 * Arguments	:	None.
 */
void
SDtAuGetOptions(void)
{
	char          *options_path;
	char          *home_path;

	/* Open options Resource Database */
	options_path = (char *) malloc(PATH_MAX);
	if (options_path == NULL) {
		fprintf(stderr, "%s\n",
			catgets(msgCatalog, 3, 46, "Could not allocate memory for operation."));
		exit(1);
	}

	home_path = getenv("HOME");
	if (home_path == NULL) {

		/* Great, can't find the user's home directory.
		 | Go ahead and create a resource database based
		 | on the default_options global variable.
		 */
		sdta_options_rdb = XrmGetStringDatabase(default_options);
	} else {

		/* The audio resource file is $HOME/.dt/sessions/<PROPS_FILE>,
		 * i.e., $HOME/.dt/sessions/Sdtaudio.  Get the resource
		 * from that file.
		 */
		strcpy(options_path, home_path);
		strcat(options_path, PROPS_FILE);
		sdta_options_rdb = XrmGetFileDatabase(options_path);
		if (sdta_options_rdb == (XrmDatabase) NULL) {

			/* The audio resource file does not exist. 
			 | Go ahead and create a resource database based
			 | on the default_options global variable.
			 */
			sdta_options_rdb = 
				XrmGetStringDatabase(default_options);
		}
	}

	free(options_path);
}

/* Function	: SDtAuHideWaveWin()
 * Objective	: Display the "short" version of sdtaudio by hiding the
 * 		  wave window and level meter, and just displaying the
 *		  progress indicator.
 * Arguments	: WindowData *		- the sdtaudio structure.
 * Return Value : None.
 */
void 
SDtAuHideWaveWin(WindowData * wd)
{
	/* Reset the wave window button's look so that it looks pushed out. */
	XtVaSetValues(wd->wave_button,
		      XmNshadowType, XmSHADOW_ETCHED_OUT,
		      NULL);

	/* Hide the level meter  and wave pane. */
	XtUnmanageChild(wd->wave_pane);
	XtUnmanageChild(XtParent(wd->level_meter));

	/* Now manage the progress indicator, and its associated labels. */
	XtManageChild(wd->prog_scale);
	XtManageChild(wd->prog_start);
	XtManageChild(wd->prog_end);
	XmUpdateDisplay(wd->toplevel);
}

/* Function	: SDtAuShowWaveWin()
 * Objective	: Display the "long" version of sdtaudio by displaying
 * 		  the wave window and level meter, and hiding the
 *		  progress indicator.
 * Arguments	: WindowData *		- the sdtaudio structure.
 * Return Value : None.
 */
void 
SDtAuShowWaveWin(WindowData * wd)
{
	XtVaSetValues(wd->wave_button,
		      XmNshadowType, XmSHADOW_ETCHED_IN,
		      NULL);

	/* Hide the progress indicator, and its associated labels. */
	XtUnmanageChild(wd->prog_scale);
	XtUnmanageChild(wd->prog_start);
	XtUnmanageChild(wd->prog_end);

	/* Now manage the level meter. */
	XtManageChild(XtParent(wd->level_meter));

	/* Now manage the wave pane. */
	XtManageChild(wd->wave_pane);
	wd->show_wave = True;
	SDtAuUpdateWaveWin(wd);
	XmUpdateDisplay(wd->toplevel);
}

/* Function	: SDtAuSetMessage()
 * Objective	: Display a message in the specified label widget.  
 *		  Widget is either the footer or status area of the
 *		  sdtaudio window.
 * Arguments	: Widget	- the label widget where message will be
 *				  displayed
 * 		  char *	- the message to display, in localized
 *				  format
 * Return Value : None.
 */
void 
SDtAuSetMessage(Widget wid, char *status_str)
{
	XmString        xmStr;

	/* Convert the message to an XmString, and then
	 * display it.
	 */
	xmStr = XmStringCreateLocalized(status_str);
	XtVaSetValues(wid, XmNlabelString, xmStr, NULL);
	XmUpdateDisplay(wid);
	XmStringFree(xmStr);
}

/* Function	: SDtAuSetSameDimension()
 * Objective	: This function queries each and every widget in the
 *		  array and gets the Xm dimension trait (e.g., width,
 *		  height, etc.).  The common trait is the largest value
 *		  among all the widgets.
 * Arguments	: Widget *	- array of widgets to query and set
 *		  int           - number of widgets in the array
 * 		  String	- the Xm widget characteristic to query
 * Return Value : None.
 */
void 
SDtAuSetSameDimension(Widget *wids, int num_wids, String xm_trait)
{
	register int   n;
        Dimension      common_val = 0;
	Dimension      wid_val = 0;

	/* Get the dimension of each and every widget, in order to
	 * determine the widget with the biggest value. 
	 */
	for (n = 0; n < num_wids; n++) {

		XtVaGetValues(wids[n], xm_trait, &wid_val, NULL);
		if (wid_val > common_val)
			common_val = wid_val;
	}

	/* Biggest value is now in common_val.  Set all the widgets
	 * to use this value.
	 */
	for (n = 0; n < num_wids; n++) 
		XtVaSetValues(wids[n], xm_trait, common_val, NULL);
}

/* Function	: SDtAuGetCurrentDeviceSettings()
 * Objective	: Get the current device values, e.g., volume, mute, etc.
 * Arguments	: WindowData *		- sdtaudio data structure
 * Return Value : None.
 */
void
SDtAuGetCurrentDeviceSettings(WindowData *wd)
{
	SAError        error;
	int	       val;		/* Audio values. */
        double         sa_val;
	SABoolean      sa_bool;

	/* Did play volume change? */
	if (SDtAudioGetPlayVol(wd->sa_stream, &sa_val, &error)) {

		val = (int) sa_val;
		if ((val != wd->play_vol) && (val >= VOL_MIN) && 
		    (val <= VOL_MAX)) {
			if (SDtAudioSetPlayVol(wd->sa_stream, val, &error)) {

				wd->play_vol = val;
				XmScaleSetValue(wd->vol_gauge, val);
			}
		}
	}

	/* Was the mute button pressed? */
	if (SDtAudioGetMuteState(wd->sa_stream, &sa_bool, &error)) {

		if (sa_bool != wd->is_muted) {

			if (SDtAudioSetMuteState(wd->sa_stream, sa_bool,
						 &error)) {

				wd->is_muted = sa_bool;
				if (wd->is_muted == SATrue)
					XtVaSetValues(wd->mute_button,
						      XmNshadowType, 
						      XmSHADOW_ETCHED_IN,
						      NULL);
				else
					XtVaSetValues(wd->mute_button,
						      XmNshadowType, 
						      XmSHADOW_ETCHED_OUT,
						      NULL);
			}
		}
	}
}

/* Function	: SDtAuUpdateView()
 * Objective	: Update all the possible GUI items that can be affected
 *		  whenever there is a change in the audio device, e.g.,
 *		  user pressed the mute button on the speaker itself.
 * Arguments	: None.
 * Return Value : None.
 * Global used  : WindowData *wd	- the sdtaudio data structure.
 */
void
SDtAuUpdateView()
{
	/* Only update if the options dlog has been created. */
	if (wd->options_dlog) {
		SDtAuGetAvailableDevices(wd);
		SDtAuGetCurrentlyUsedDevices(wd);
	}
	SDtAuGetCurrentDeviceSettings(wd);
	XmUpdateDisplay(wd->toplevel);
}

/* Function	: SDtAuSetSigpoll()
 * Objective	: This function sets the sigpool handler so that 
 *		  the audio devices can be updated on the fly whenever
 *		  there is a change, e.g., hardware mute, addition of
 *		  headphones, etc.
 * Arguments	: WindowData *		- the sdtaudio data structure.
 * Return Value : Boolean True if the signal handler is installed, False
 *		  otherwise.
 */
Boolean
SDtAuSetSigpoll(WindowData *wd)
{
	struct sigaction     action;
	SAError              error;
	SAOutputFlags        port;

	/* Check for valid sun audio stream. */
	if (!(wd->sa_stream)) {

		/* Open up a sun audio stream if it is not defined. */
		wd->sa_stream = SAStreamOpen(NULL, &error);
		if (!(wd->sa_stream))
			return(False);
	}

	/* Check for valid output port device. */
	if (wd->sa_stream->device->play_ports == 0) {
		if (SADeviceGetOutputPorts(wd->sa_stream->device, &port, 
					   &error))
			wd->sa_stream->device->play_ports = port;
		else
			return(False);
	}

	/* Set it up so that the audio library does polling. */
	if (!SADeviceSigPollUpdateOn(wd->sa_stream->device, &error))
		return(False);

	/* Set up the sigpoll handler. */
	action.sa_handler = SDtAuUpdateView;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	/* Now initialize the sigpoll handler. */
	if (sigaction(SIGPOLL, &action, NULL) < 0)
		return(False);
	else
		return(True);
}

/* Function	: SDtAuInitDrawables()
 * Objective	: Initializes all the drawing items, e.g., graphics contexts,
 *		  for the wave window.
 * Arguments	: WindowData *		- the sdtaudio data structure.
 * Return Value : None.
 */
void 
SDtAuInitDrawables(WindowData * wd)
{
	XGCValues       gc_values;
	XColor          bar_color;
	Pixel           def_background;
	Pixel           def_foreground;

	/* waveForeGrndGC is the main wave window graphic context.  This
	 * gc will be used to draw the wave, as well as the horizontal
	 * axis of the wave window.
	 */
	gc_values.foreground = 
		BlackPixel(XtDisplay(wd->wave_window),
			   DefaultScreen(XtDisplay(wd->wave_window)));
	gc_values.font = ww_font->fid;
	wd->waveForeGrndGC = XCreateGC(XtDisplay(wd->wave_window),
				       XtWindow(wd->wave_window),
				       GCForeground | GCFont,
				       &gc_values);

	/* wavePntrGC is the graphics context associated with the 
	 * line indicating to the user a particular location in
	 * the audio file.  This line indicator can be used to
	 * set the selection's start and end points within the audio
	 * file.
	 */
	gc_values.foreground = 
		BlackPixel(XtDisplay(wd->wave_window),
			   DefaultScreen(XtDisplay(wd->wave_window)));
	gc_values.function = GXxor;
	gc_values.line_width = 0;
	wd->wavePntrGC = XCreateGC(XtDisplay(wd->mainWindow),
				   XtWindow(wd->mainWindow),
				   (GCForeground | GCFunction | GCLineWidth),
				   &gc_values);

	/* The waveSelGC is the graphic context used to generate a selection
	 * within the wave window.
	 */
	XtVaGetValues(wd->wave_window,
		      XmNforeground, &def_foreground,
		      XmNbackground, &def_background,
		      NULL);
	gc_values.foreground = def_foreground;
	gc_values.background = def_background;
	gc_values.function = GXinvert;
	wd->waveSelGC = XCreateGC(XtDisplay(wd->mainWindow),
				  XtWindow(wd->mainWindow),
				  (GCForeground | GCBackground | GCFunction),
				  &gc_values);

	/* All wave window graphics operations are first performed on
	 * the pixmap.  This graphics context is associated with the pixmap,
	 * so that once graphics operations are done, the pixmap can be
	 * copied onto the wave window, thus "drawing" the wave window's
	 * contents.
	 */
	gc_values.foreground = def_foreground;
	gc_values.background = def_background;
	gc_values.function = GXcopy;
	wd->wavePMGC = XCreateGC(XtDisplay(wd->mainWindow),
				 XtWindow(wd->mainWindow),
				 (GCForeground | GCBackground | GCFunction),
				 &gc_values);


	/* The following graphics context is reversed so that we can erase
	 * whatever we drew on the screen.
	 */
	gc_values.foreground = def_background;
	gc_values.background = def_foreground;
	gc_values.font = ww_font->fid;
	gc_values.function = GXcopy;
	wd->waveReverseGC = XCreateGC(XtDisplay(wd->mainWindow),
				    XtWindow(wd->mainWindow),
				    (GCForeground | GCBackground |
				     GCFont | GCFunction),
				    &gc_values);

	/* Create a graphics context for the level meter squares, i.e.,
	 * the green squares that show the wave's level as it plays.  Try
	 * to allocate a green color - if it can't default to the white
	 * pixel.
	 */
	bar_color.red = 0;
	bar_color.green = 65535;
	bar_color.blue = 0;
	if (!XAllocColor(XtDisplay(wd->mainWindow),
			 DefaultColormap(XtDisplay(wd->wave_window),
			          DefaultScreen(XtDisplay(wd->wave_window))),
			 &bar_color)) {

		/* Don't display an alert prompt - just use the WhitePixel
		 * macro.
		 */
		gc_values.foreground = WhitePixel(XtDisplay(wd->wave_window),
				 DefaultScreen(XtDisplay(wd->wave_window)));

	} else {

		/* Green color allocated - use it. */
		gc_values.foreground = bar_color.pixel;
	}
	gc_values.background = BlackPixel(XtDisplay(wd->wave_window),
				 DefaultScreen(XtDisplay(wd->wave_window)));
	gc_values.function = GXcopy;
	wd->meterGC = XCreateGC(XtDisplay(wd->mainWindow),
				XtWindow(wd->mainWindow),
				(GCForeground | GCBackground | GCFunction),
				&gc_values);

	/* Initialization done - make sure we only do this once.  */
	wd->drawEnvInitialized = True;
}

/* Function	: SDtAuUpdateWaveCurso()
 * Objective	: Update the play point cursor in the wave window to its
 *		  new position, based on the value of clip_pos.
 * Arguments	: WindowData *		- the sdtaudio structure.
 * Return Value : None.
 */
void
SDtAuUpdateWaveCursor(WindowData *wd)
{
	Dimension	w_width;
	short		new_x;

	/* Get the wave window's width. */
	XtVaGetValues(wd->wave_window,
		      XtNwidth, &w_width,
		      NULL);

	/* The clip_pos variable is a percentage, so use this percentage
	 * to calculate the new x coordinate of the wave window play point
	 * cursor.
	 */
	new_x = (short) ((wd->clip_pos / 100.0) * w_width);
	SDtAuDrawWaveCursor(wd, new_x);
}

/* Function	: SDtAuUpdateWaveWin()
 * Objective	: Update the drawables, e.g., wave form, in the wave window.
 * Arguments	: WindowData *		- the sdtaudio structure.
 * Return Value : None.
 */
void 
SDtAuUpdateWaveWin(WindowData *wd)
{
	Dimension       w_width, w_height;
	GC              gc;

	/* Triangle cursor. */

	if ((wd == NULL) || (wd->wave_window == NULL)) {

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

	/* The wave window is not showing, so do nothing. */
	if (wd->show_wave == False)
		return;

	if (wd->drawEnvInitialized == False)
		SDtAuInitDrawables(wd);

	/* Get the dimensions of the wave window. */
	XtVaGetValues(wd->wave_window,
		      XtNheight, &w_height,
		      XtNwidth, &w_width,
		      NULL);

	if (wd->wave_pixmap == NULL) {

		if (!SDtAuCreateWavePixmap(wd)) {

			char buffer[2048];

			sprintf(buffer, "%s\n%s", 
				catgets(msgCatalog, 3, 46, "Could not allocate memory for operation."),
				catgets(msgCatalog, 3, 44, "Cannot draw audio wave form."));
			AlertPrompt(wd->toplevel,
				    DIALOG_TYPE, XmDIALOG_WARNING,
				    DIALOG_TITLE,
				    catgets(msgCatalog, 3, 59, "Audio - Error"),
				    DIALOG_TEXT, buffer,
				    BUTTON_IDENT, ANSWER_ACTION_1,
					catgets(msgCatalog, 2, 62, "Continue"),
				    NULL);
			return;

		}
	}

	/* Set the background of the wave window. */
	XFillRectangle(XtDisplay(wd->wave_window),
		       wd->wave_pixmap,
		       wd->waveReverseGC,
		       0, 0,
		       w_width,
		       w_height);

	/* Draw the horizontal axis. */
	XDrawLine(XtDisplay(wd->wave_window),
		  wd->wave_pixmap,
		  wd->waveForeGrndGC,
		  0, w_height / 2,
		  w_width, w_height / 2);

	/* Draw the wave form of the audio file. */
	if (wd->safile != NULL) {
		SDtAuDrawWave(wd, w_height, w_width);
		SDtAuDrawWaveCursor(wd, 
			(short) (wd->clip_pos * w_width / 100));
	}

	/* Now display the wave window by copying the contents 
	 * of the pixmap that we just painted to the main
	 * wave window.
	 */
	XCopyArea(XtDisplay(wd->wave_window), wd->wave_pixmap,
		  XtWindow(wd->wave_window), wd->wavePMGC,
		  0, 0, w_width, w_height, 0, 0);

#ifdef SELECTION
	/* Draw the selection, if necessary. */
	SDtAuDrawSel(wd);
#endif
}

/* Function	: SDtAuUpdateTimeLabels
 * Objective	: Update the time labels on the progress pane and the 
 *		  wave window.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value	: None.
 */
void
SDtAuUpdateTimeLabels(WindowData * wd)
{
	XmString        label_str = NULL;
	char            buf[1000];
	char           *p;
	float           disp_seconds, seconds;
	int             disp_minutes;
	SAFile         *safile = wd->safile;
        off_t		tmp_data_size;

	/* If no file, show no time, i.e., -:-- */
	if (safile == NULL) {

		/* no file data */
		label_str = XmStringCreateLocalized(catgets(msgCatalog, 2, 52,
							    "-:--"));
		XtVaSetValues(wd->prog_end,
			      XmNlabelString, label_str,
			      NULL);
		XtVaSetValues(wd->wave_end,
			      XmNlabelString, label_str,
			      NULL);
		XmStringFree(label_str);
		return;
	}

	/* Some audio files may not have the data size field set. */
	tmp_data_size = SAFileGetDataSize(safile);

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 40, "Cannot update time information."));
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

	/* Calculate the total time for the audio file. */
	seconds = (float) (tmp_data_size) /
		((float) (safile->format.channels *
			  safile->format.sample_size *
			  safile->format.sample_rate) /
		 (float) (safile->format.samples_per_unit));
	disp_minutes = (int) seconds / 60;
	disp_seconds = seconds - (float) (disp_minutes * 60);

	sprintf(buf, "%d:%02.1f", disp_minutes, disp_seconds);
	label_str = XmStringCreateLocalized(buf);
	XtVaSetValues(wd->prog_end,
		      XmNlabelString, label_str,
		      NULL);
	XtVaSetValues(wd->wave_end,
		      XmNlabelString, label_str,
		      NULL);
	XmStringFree(label_str);
}

/* Function	: SDtAuUpdateInfoDlog()
 * Objective	: Update the contents of the info dialog with information
 *		  about the audio file that is loaded in sdtaudio.
 * Arguments	: WindowData *		- the sdtaudio data structure
 * Return Value : None.
 */
void
SDtAuUpdateInfoDlog(WindowData * wd)
{
	XmString        label_str = NULL;
	char            buf[1000];
	char           *p;
	SAFile         *safile = wd->safile;
	off_t		tmp_data_size;

	if (wd == NULL) {

		char	buffer[1024];

		sprintf(buffer, "%s\n%s", 
			catgets(msgCatalog, 3, 2, "Internal error: sdtaudio structure is empty."),
			catgets(msgCatalog, 3, 41, "Cannot update \"File Information\" window."));
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

	if (safile == NULL) {

		/* No file data */
		SDtAuClearInfo(wd);
		return;
	}

	/* Some audio files don't have data size set! */
	tmp_data_size = SAFileGetDataSize(safile);

	if (wd->name != NULL) {
		p = strrchr(wd->name, '/');
		if (p == NULL)
			p = wd->name;
		else
			p++;

		label_str = XmStringCreateLocalized(p);
		XtVaSetValues(wd->info_name,
			      XmNlabelString, label_str,
			      NULL);

		XmStringFree(label_str);
	}
	sprintf(buf, "%3.2f Kb",
		(float) (tmp_data_size + safile->header_size) /
		(float) 1024);
	label_str = XmStringCreateLocalized(buf);
	XtVaSetValues(wd->info_size,
		      XmNlabelString, label_str,
		      NULL);
	XmStringFree(label_str);

	sprintf(buf, "%3.2f seconds",
		(float) (tmp_data_size) /
		((float) (safile->format.channels *
			  safile->format.sample_size *
			  safile->format.sample_rate) /
		 (float) (safile->format.samples_per_unit)));
	label_str = XmStringCreateLocalized(buf);
	XtVaSetValues(wd->info_length,
		      XmNlabelString, label_str,
		      NULL);
	XmStringFree(label_str);

	switch (safile->format.channels) {
	case 0:
		label_str =
			XmStringCreateLocalized(catgets(msgCatalog, 3, 36,
							"Zero?"));
		break;

	case 1:
		label_str =
			XmStringCreateLocalized(catgets(msgCatalog, 3, 37,
							"Mono"));
		break;

	case 2:
		label_str =
			XmStringCreateLocalized(catgets(msgCatalog, 3, 38,
							"Stereo"));
		break;

	default:
		label_str =
			XmStringCreateLocalized(catgets(msgCatalog, 3, 39,
							"Unknown"));
		break;
	}
	XtVaSetValues(wd->info_channels,
		      XmNlabelString, label_str,
		      NULL);
	XmStringFree(label_str);

	sprintf(buf, "%3.2f kHz",
		(float) safile->format.sample_rate /
		(float) 1000);
	label_str = XmStringCreateLocalized(buf);
	XtVaSetValues(wd->info_sample_rate,
		      XmNlabelString, label_str,
		      NULL);
	XmStringFree(label_str);

	sprintf(buf, "%d-bit, ", safile->format.sample_size * 8);
	switch (safile->format.encoding) {
	case SAEncoding_NONE:
		strcat(buf, catgets(msgCatalog, 1, 23,
				    "No encoding specified."));
		break;

	case SAEncoding_ULAW:
		strcat(buf, catgets(msgCatalog, 1, 24, "ISDN u-law"));
		break;

	case SAEncoding_ALAW:
		strcat(buf, catgets(msgCatalog, 1, 25, "ISDN A-law"));
		break;

	case SAEncoding_LINEAR:
		strcat(buf, catgets(msgCatalog, 1, 26, "PCM 2's-complement"));
		break;

	case SAEncoding_FLOAT:
		strcat(buf, catgets(msgCatalog, 1, 27, "IEEE float"));
		break;

	case SAEncoding_G721:
		strcat(buf, catgets(msgCatalog, 1, 28, "CCITT g.721 ADPCM"));
		break;

	case SAEncoding_G722:
		strcat(buf, catgets(msgCatalog, 1, 29, "CCITT g.722 ADPCM"));
		break;

	case SAEncoding_G723:
		strcat(buf, catgets(msgCatalog, 1, 30, "CCITT g.723 ADPCM"));
		break;

	case SAEncoding_DVI:
		strcat(buf, catgets(msgCatalog, 1, 31, "DVI ADPCM"));
		break;

	default:
		strcat(buf, catgets(msgCatalog, 1, 32, "Unknown."));
		break;
	}
	label_str = XmStringCreateLocalized(buf);
	XtVaSetValues(wd->info_encoding,
		      XmNlabelString, label_str,
		      NULL);
	XmStringFree(label_str);
}


/* Function	: SDtAuUpdateLevelMeter
 * Objective	: Redraws the level meter corresponding to the current 
 *		  audio level.
 * Arguments	: WindowData *		- the sdtaudio data structure.
 * Return Value	: None.
 */
void 
SDtAuUpdateLevelMeter(WindowData * wd)
{
	Dimension       w_width, w_height;
	int             element_width;
	int             num_segments = MAX_SEGMENTS * wd->meter_gain;
	int             i = 0;
	XRectangle      rects[MAX_SEGMENTS];

	if (!wd->show_wave)
		return;

	if (wd->drawEnvInitialized == False)
		SDtAuInitDrawables(wd);

	XtVaGetValues(wd->level_meter,
		      XmNheight, &w_height,
		      XmNwidth, &w_width,
		      NULL);

	XFillRectangle(XtDisplay(wd->level_meter),
		       XtWindow(wd->level_meter),
		       wd->waveForeGrndGC,
		       0, 0,
		       w_width,
		       w_height);

	element_width = w_width / MAX_SEGMENTS;
	while (i < num_segments) {
		rects[i].x = (short) ((i * element_width) + LEVEL_LIGHT_BORDER);
		rects[i].y = (short) LEVEL_LIGHT_BORDER;
		rects[i].width = (short) (element_width - (LEVEL_LIGHT_BORDER * 2));
		rects[i].height = (short) (w_height - (LEVEL_LIGHT_BORDER * 2));
		i++;
	}

	if (num_segments > 0)
		XFillRectangles(XtDisplay(wd->level_meter),
				XtWindow(wd->level_meter),
				wd->meterGC,
				rects,
				num_segments);
}

/* Function	: SDtAuDndFilename
 * Objective	: Process the file name that was passed in via a drag
 *		  and drop opertion.
 * Arguments	: WindowData *				- the sdtaudio
 *							  data structure.
 *		  DtDndTransferCallbackStruct *		- the drag and
 *							  drop structure.
 * Return Value	: None.
 */
void 
SDtAuDndFilename(WindowData * wd, DtDndTransferCallbackStruct *cbs)
{
	char           *path;
	int             status;

	/* Check for multiple filenames. */
	if (cbs->dropData->numItems != 1) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE,
				catgets(msgCatalog, 3, 59, "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 62,
				"Please drag only one file onto Audio."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		cbs->status = DtDND_FAILURE;
		return;
	}
			    
	path = cbs->dropData->data.files[0];
	if (path)
		if (!LoadFile(wd, path))
			cbs->status = DtDND_FAILURE;
}


/* Function	: SDtAuDndBuffer
 * Objective	: Process the buffer that was passed in via a drag
 *		  and drop opertion.
 * Arguments	: WindowData *				- the sdtaudio
 *							  data structure.
 *		  DtDndTransferCallbackStruct *		- the drag and
 *							  drop structure.
 * Return Value	: None.
 */
void 
SDtAuDndBuffer(WindowData * wd, DtDndTransferCallbackStruct *cbs)
{
	DtDndBuffer       *buffers;
	char              *tmp_name = (char *) NULL;

	/* Get the buffer from the callback structure. */
	if (cbs->dropData == NULL) {
		cbs->status = DtDND_FAILURE;
		return;
	} else
		buffers = cbs->dropData->data.buffers;

	/* Check for multiple buffers. */
	if (cbs->dropData->numItems != 1) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE,
				catgets(msgCatalog, 3, 59, "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 62,
				"Please drag only one file onto Audio."),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		cbs->status = DtDND_FAILURE;
		return;
	}

	/* Check for empty buffer. */
	if (buffers[0].size < 1) {
		AlertPrompt(wd->toplevel,
			    DIALOG_TYPE, XmDIALOG_ERROR,
			    DIALOG_TITLE,
				catgets(msgCatalog, 3, 59, "Audio - Error"),
			    DIALOG_TEXT, catgets(msgCatalog, 3, 63,
				"File is empty!"),
			    BUTTON_IDENT, ANSWER_ACTION_1,
				catgets(msgCatalog, 2, 62, "Continue"),
			    NULL);
		cbs->status = DtDND_FAILURE;
		return;
	}

	/* Is there a name associated with the buffer? */
	if (buffers[0].name) {

		/* Strip out directory path, so that we are left only
		 * with a file name.
		 */
		tmp_name = strrchr(buffers[0].name, '/');
		if (tmp_name)
			tmp_name++;
		else
			tmp_name = buffers[0].name;
	}

	/* Now load the buffer. */
	if (!LoadBuffer(wd, buffers[0].bp, buffers[0].size, tmp_name))
		cbs->status = DtDND_FAILURE;
}


/* Function	: SDtAuWaveLocLine
 * Objective	: Draw a vertical line that will indicate to the user a
 *		  particular location within the audio file.  The location's 
 *		  point is referenced via time (which will be displayed via
 *		  the SDtAuDrawLocTime() function.
 * Arguments	: int		- the x location in the wave window where
 *				  the line will be drawn
 *		  WindowData	- the sdtaudio data structure
 * Return Value	: None.
 */
void
SDtAuWaveLocLine(int x, WindowData *wd)
{
	Dimension       w_width, w_height;

	XtVaGetValues(wd->wave_window,
		      XtNheight, &w_height,
		      XtNwidth, &w_width,
		      NULL);
	XDrawLine(XtDisplay(wd->wave_window),
		  XtWindow(wd->wave_window),
		  wd->wavePntrGC,
		  x, 0,
		  x, w_height - ww_font_height - 2);
}

/* Function	: SDtAuWaveLocTime
 * Objective	: Draw the time (from the beginning) associated with
 *		  the location line.
 * Arguments	: int		- the x location in the wave window where
 *				  the text will be written
 *		  WindowData	- the sdtaudio data structure
 * Return Value	: None.
 */
void
SDtAuWaveLocTime(int x, WindowData * wd)
{
	Dimension       w_width, w_height;
	int             real_x;
	int             txt_width;

	XtVaGetValues(wd->wave_window,
		      XtNheight, &w_height,
		      XtNwidth, &w_width,
		      NULL);

	/* Center the time relative to the location line. */
	txt_width = XTextWidth(ww_font, wd->old_pos_str, 
			       strlen(wd->old_pos_str));
	real_x = wd->old_x - (txt_width / 2);

	/* First - erase the old time text (because the draw line
	 * was moved).  Remember not to go off the left side of
	 * the wave window (assume: border width is 3, so if line
	 * is at left most, keep text at position 3.
	 */
	if (real_x < 3)
		real_x = 3;

	/* Do the same, but this time, for the right side of the wave win. */
	if ((wd->old_x + txt_width) > w_width)	
		real_x = w_width - txt_width;

	/* Erase old text. */
	XDrawString(XtDisplay(wd->wave_window),
		    XtWindow(wd->wave_window),
		    wd->waveReverseGC,
		    real_x, w_height - 3,
		    wd->old_pos_str,
		    strlen(wd->old_pos_str));

	/* Now write in new time text, with same conditions about the
	 * left and right end of the wave window.
	 */
	txt_width = XTextWidth(ww_font, wd->new_pos_str, 
			       strlen(wd->new_pos_str));
	real_x = x - (txt_width / 2);
	if (real_x < 3)
		real_x = 3;
	if ((x + (txt_width)) > w_width)
		real_x = w_width - txt_width;
	XDrawString(XtDisplay(wd->wave_window),
		    XtWindow(wd->wave_window),
		    wd->waveForeGrndGC,
		    real_x, w_height - 3,
		    wd->new_pos_str,
		    strlen(wd->new_pos_str));
}

/* Function	: SDtAuResetOptions
 * Objective	: Reset the values found in the options dialog window.
 * Arguments	: WindowData *	- the audio data structure
 * Return Value : None.
 */
void
SDtAuResetOptions(WindowData *wd)
{
	/* Reset the options based on the original values. */
	XtVaSetValues(wd->wave_view_rb,
		      XmNset, wd->options->showWave,
		      NULL);
	XtVaSetValues(wd->standard_view_rb,
		      XmNset, !wd->options->showWave,
		      NULL);
	XtVaSetValues(wd->auto_play_open,
		      XmNset, wd->options->autoPlayOnOpen,
		      NULL);
	XtVaSetValues(wd->play_vol_scale,
		      XmNvalue, wd->options->playVolume,
		      NULL);
	XtVaSetValues(wd->play_bal_scale,
		      XmNvalue, wd->options->playBalance,
		      NULL);
	XtVaSetValues(wd->record_vol_scale,
		      XmNvalue, wd->options->recordVolume,
		      NULL);
	XtVaSetValues(wd->record_bal_scale,
		      XmNvalue, wd->options->recordBalance,
		      NULL);
	switch (wd->options->recordingQuality) {

		case SDTA_CD_QUALITY:
			XtVaSetValues(wd->recqual_option, 
				      XmNmenuHistory, wd->cd_menu_item,
				      NULL);
			break;

		case SDTA_DAT_QUALITY:
			XtVaSetValues(wd->recqual_option, 
				    XmNmenuHistory, wd->dat_menu_item,
				    NULL);
			break;

		default:
			XtVaSetValues(wd->recqual_option, 
				      XmNmenuHistory, wd->voice_menu_item,
				      NULL);
			break;
	}
	switch (wd->options->recordingFormat) {

		case SAWavType:
			XtVaSetValues(wd->audformat_option, 
				      XmNmenuHistory, wd->wav_menu_item,
				      NULL);
			break;

		case SAAiffType:
			XtVaSetValues(wd->audformat_option, 
				      XmNmenuHistory, wd->aiff_menu_item,
				      NULL);
			break;

		default:
			XtVaSetValues(wd->audformat_option, 
				      XmNmenuHistory, wd->au_menu_item,
				      NULL);
			break;
	}
	XtVaSetValues(wd->selected_rb,
		      XmNset, wd->options->useDefaultRecValues,
		      NULL);
	XtVaSetValues(wd->ask_rb,
		      XmNset, !wd->options->useDefaultRecValues,
		      NULL);
	SDtAuSetDeviceState(wd);
	SDtAuUpdateDevices(wd);
	XmUpdateDisplay(wd->options_dlog);
}

/* Function	: SDtAuStopAndWait
 * Objective	: Stop the current audio file being played and wait
 *		  until the buffers have been cleared.
 * Arguments	: WindowData *	- the audio data structure
 * Return Value : None.
 */
void
SDtAuStopAndWait(WindowData *wd)
{
	if (wd->play_thread_data && wd->play_state == STATE_PLAY) {

		xa_stop_audio(wd);

		/* Loop until the state has changed to stop. */
		while(wd->play_state != STATE_STOPPED);

		/* Wait until this value is 0, before we continue.
		 * This is a hack. This insures that nothing is
		 * being sent to the audio device anymore.
		 */
		while (wd->sa_stream->device->samples_played != 0);
	}
}

#ifdef SELECTION
/* Function	: SDtAuChangeSel
 * Objective	: Update the selection in the wave window, based on the
 *		  the mouse pointer's movement.  The selection area can
 *		  either decrease or increase, depending on where the
 *		  intial anchor was placed, and on the direction of the
 *		  mouse pointer's movement.
 * Arguments	: int		- the new x coordinate of pointer
 *		  int		- the old x coordinate of the pointer
 *		  int		- the initial anchor point of the selection
 *		  WindowData	- the sdtaudio data structure
 * Return Value	: None.
 */
void 
SDtAuChangeSel(int new_x, int old_x, int selection_anchor, WindowData *wd)
{
	Dimension       w_width, w_height;
	int             render_x, render_width;
	GC              render_gc = NULL;

	/* Get the wave window's dimensions. */
	XtVaGetValues(wd->wave_window,
		      XtNheight, &w_height,
		      XtNwidth, &w_width,
		      NULL);

	/* Calculate the new selection width. */
	render_width = abs(new_x - old_x);

	/* Was pointer moved to the right of the original anchor point? */
	if (new_x > selection_anchor) {

		if (new_x > old_x) {

			/* Expand selection. */
			render_x = old_x;
			render_gc = wd->waveSelGC;
			XFillRectangle(XtDisplay(wd->wave_window),
				       XtWindow(wd->wave_window),
				       render_gc,
				       render_x, WAVE_CURSOR_HEIGHT,
				       (unsigned int) render_width, 
				       w_height - ww_font_height - 
						WAVE_CURSOR_HEIGHT);
		} else {

			/* Contract selection. */
			render_x = new_x;
			render_gc = wd->waveReverseGC;
			XCopyArea(XtDisplay(wd->wave_window),
				  wd->wave_pixmap,
				  XtWindow(wd->wave_window),
				  render_gc,
				  /* 
				  wd->wavePMGC,
				  */
				  render_x, 0,
				  (unsigned int) render_width,
				  w_height - ww_font_height,
				  render_x, 0);
		}
	} else {
		/* Pointer is to the left of the original anchor point. */
		if (new_x < old_x) {

			/* Expand selection. */
			render_x = new_x;
			render_gc = wd->waveSelGC;
			XFillRectangle(XtDisplay(wd->wave_window),
				       XtWindow(wd->wave_window),
				       render_gc,
				       render_x, WAVE_CURSOR_HEIGHT,
				       (unsigned int) render_width, 
				       w_height - ww_font_height - 
						WAVE_CURSOR_HEIGHT);
		} else {

			/* Contract selection. */
			render_x = old_x;
			render_gc = wd->waveReverseGC;
			XCopyArea(XtDisplay(wd->wave_window),
				  wd->wave_pixmap,
				  XtWindow(wd->wave_window),
				  render_gc,
				  /*
				  wd->wavePMGC,
				  */
				  render_x, 0,
				  (unsigned int) render_width, 
				  w_height - ww_font_height,
				  render_x, 0);
		}
	}
}

/* Function	: SDtAuClearSelection
 * Objective	: Clear the selection from the wave window.
 * Arguments	: int		- the new x coordinate of pointer
 *		  int		- the old x coordinate of the pointer
 *		  int		- the initial anchor point of the selection
 *		  WindowData	- the sdtaudio data structure
 * Return Value	: None.
 */
void 
SDtAuClearSelection(WindowData *wd)
{
	Dimension       w_width, w_height;
	int             render_width;

	/* If the drawing variables were not initialized, then
	 * there is no selection to clear - just return.
	 */
	if (!wd->drawEnvInitialized)
		return;

	/* Get the wave window's dimensions. */
	XtVaGetValues(wd->wave_window,
		      XtNheight, &w_height,
		      XtNwidth, &w_width,
		      NULL);

	/* Calculate selection width. */
	render_width = abs(wd->sel_start_x - wd->sel_end_x);

	/* Clear the selected area. */
	XCopyArea(XtDisplay(wd->wave_window),
		  wd->wave_pixmap,
		  XtWindow(wd->wave_window),
		  wd->waveReverseGC,
		  wd->sel_start_x, 0,
		  (unsigned int) render_width, 
		  w_height - ww_font_height,
		  wd->sel_start_x, 0);

	/* Reset the start and end values. */
	wd->sel_start_x = 0;
	wd->sel_end_x = 0;
}
#endif
