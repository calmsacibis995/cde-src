/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)rec_init.c	1.3 97/03/21 SMI"

#include <nl_types.h>
#include <Xm/XmAll.h>

#include "sdtaudio.h"
#include "sa.h"

/* Sample rate used in recording. */
#define	RATE_VOICE	8000
#define RATE_CD	41000
#define	RATE_DAT	48000
#define	SB_RATE_VOICE	0
#define	SB_RATE_CD	11025
#define	SB_RATE_DAT	22050

typedef enum { 
	AMD, 	
	DBRI,
	SBPRO,
	SB16
} AudioHardware;

/**********************************************************************/

/* Function	: InitSAFile()
 * Objective	: Initialize the safile record of sdtaudio data structure.
 * Input	: WindowData	*	- the sdtaudio data structure
 * Return Value	: None.
 */

void
InitSAFile(WindowData *wd)
{
	AudioHardware	hw;

	/* Determine the audio hardware. */
	if (strcmp(wd->sa_stream->device->hardware_name, "SUNW,sbpro") == 0)
		hw = SBPRO;
	else if (strcmp(wd->sa_stream->device->hardware_name, "SUNW,sb16") == 0)
		hw = SB16;
	else if (strcmp(wd->sa_stream->device->hardware_name, "SUNW,am79c30") 
		 == 0)
		hw = AMD;
	else 
		hw = DBRI;

	/* Assign the file type - AU, WAV, AIFF - based on the
	 * recording format specified in options default.
	 */
	wd->safile->filetype = wd->options->recordingFormat;
	wd->safile->format.encoding = SAEncoding_LINEAR;

	/* Set the format flags. */
	wd->safile->format.flags = SAFormatFlag_Signed;

	/* No matter what hardware, the samples_per_unit flag is 
	 * always 1.
	 */
	wd->safile->format.samples_per_unit = 1;


	/* Set the sample rate based on the recording quality. */
	switch (wd->options->recordingQuality) {

		case SDTA_CD_QUALITY:
			wd->safile->format.sample_rate = RATE_CD;
			wd->safile->format.channels = 2;
			wd->safile->format.sample_size = 2;
			break;

		case SDTA_DAT_QUALITY:
			wd->safile->format.sample_rate = RATE_DAT;
			wd->safile->format.channels = 2;
			wd->safile->format.sample_size = 2;
			break;

		default:
			wd->safile->format.sample_rate = RATE_VOICE;
			if (wd->safile->filetype == SASunType)
				wd->safile->format.encoding = SAEncoding_ULAW;
			wd->safile->format.channels = 1;
			wd->safile->format.sample_size = 1;
			break;
	}
}
