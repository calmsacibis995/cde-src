/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */


#pragma ident "@(#)safile.c	1.33 97/06/04 SMI"

/*
 * SAFile (Sun Audio File) Implementation
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <nl_types.h>

#include "sa.h"
#include "saP.h"
#include "safile.h"
#include "a_encode.h"

/* Some private functions for SAFile. */
static SABoolean     auRead(int fd, SAFile *safile, SAError *errorReturn);
static SABoolean     wavRead(int fd, SAFile *safile, SAError *errorReturn);
static SABoolean     aiffRead(int fd, SAFile *safile, SAError *errorReturn);
static SABoolean     auWrite(int fd, SAFile *safile, SAError *errorReturn);
static SABoolean     wavWrite(int fd, SAFile *safile, SAError *errorReturn);
static SABoolean     aiffWrite(int fd, SAFile *safile, SAError *errorReturn);
static SAFileType    findFileTypeByContent(char *buf, int bufsize);

static SAEncoding   _SAAudioEncode2SAEncode(int encoding);
static int          _SAEncode2AudioEncode(SAEncoding saencoding);
static int          _SAEncode2AudioFileEncode(SAEncoding saencoding, 
					      int sample_size);
static void          g72x_init_state(struct g72x_state *state_ptr);
static void          g72x_init_globals(g72x_globals *state_ptr);

/* Message catalog */
extern nl_catd msgCatalog;

/* Global state used to calc G721 ADPCM */
/* Prabhat :: need another to maintain UI */
static struct g72x_state gstate_cnv; 
static struct g72x_state gstate_ui;

/* More globals for G.721 */
static g72x_globals gglobals_cnv;
static g72x_globals gglobals_ui;

/* Some tables used for G.721 ADPCM */
static short power2[15] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 
			   512, 1024, 2048, 4096, 8192, 16384};
static int power2_count = sizeof(power2)/sizeof(power2[0]);

/* Maps G.721 code word to reconstructed scale factor normalized log
 * magnitude values.
 */
static short    _dqlntab[16] = {-2048, 4, 135, 213, 273, 323, 373, 425,
                                425, 373, 323, 273, 213, 135, 4, -2048};

/* Maps G.721 code word to log of scale factor multiplier. */
static short	_witab[16] = {-12, 18, 41, 64, 112, 198, 355, 1122,
			      1122, 355, 198, 112, 64, 41, 18, -12};
/*
 * Maps G.721 code words to a set of values whose long and short
 * term averages are computed and then compared to give an indication
 * how stationary (steady state) the signal is.
 */
static short	_fitab[16] = {0, 0, 0, 0x200, 0x200, 0x200, 0x600, 0xE00,
			      0xE00, 0x600, 0x200, 0x200, 0x200, 0, 0, 0};

/* Info field in .au generated files (with trailing spaces to make 
 * the string a multiple of 4).
 */
#define AU_INFO_FIELD   "Au File Format  "


/********************************************************
 * SAFileCreate() should be used for non-existing files and
 * opens the file for write-only.
 * - Determine the format type, if not specified in SAFile->filetype.
 * - Create a new file named by pathname.
 * - Write out header information for this format base
 *   on info from SAFile
 * - Leaves the file pointer at the beginning of the data
 * - Note: if pathname already exists, it will overwrite 
 *         the file.
 * - Returns SAFalse failure, SATrue success.
 *
 */
SABoolean
SAFileCreate(const char *pathname, SAFile * safile, SAError * errorReturn)
{
	/* Type the file and return the filetype, if not defined already. */
	if (safile->filetype == SANoType)
		safile->filetype = findFileType(pathname);
	if (safile->filetype == SANoType)
		return _SASetError(errorReturn, SAError_NO_FILE_TYPE, NULL);

	/* Create the new file. */ 
	safile->fd = open(pathname,
			  O_CREAT | O_WRONLY | O_TRUNC | O_NONBLOCK, 0644);
	if (safile->fd < 0)
		return _SASetError(errorReturn, SAError_CANNOT_OPEN, NULL);
	safile->mode = SAOpenForWrite;

	/* Write out the header information based on this file type. */
	switch (safile->filetype) {

	case SASunType:
		if (auWrite(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	case SAWavType:
		if (wavWrite(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	case SAAiffType:
		if (aiffWrite(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	default:
		break;
	}

	/* Set data_size to 0. */
	safile->data_size = 0;
	return SATrue;
}


/*
 * SAFileAdjust() should be used for existing, opened files.
 * Needed because for some file formats, the data size cannot
 * be determined until after writing the data.
 * - Seek to the beginning of the file.
 * - Write out header information for this format base
 *   on info from SAFile
 * - Leaves the file pointer at the beginning of the data
 * - Returns SAFalse failure, SATrue success.
 *
 */
static          SABoolean
adjust_file_hdr(SAFile * safile, SAError * errorReturn)
{
	off_t           status;

	/* Reset pointer to beginning */
	status = lseek(safile->fd, (off_t) 0, SEEK_SET);
	if (status < 0)
		return _SASetError(errorReturn, SAError_CANNOT_SEEK, NULL);

	/* Write out the header information based on this format. */
	switch (safile->filetype) {

	case SASunType:
		if (auWrite(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	case SAWavType:
		if (wavWrite(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	case SAAiffType:
		if (aiffWrite(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	default:
		break;
	}

	return SATrue;
}

/*
 * SAFileOpen() should be used for existing files and opens
 * the file for read-only.
 * - Opens the existing file and stores the
 *   header information in SAFile.
 * - Leaves the file ptr at the beginning of the data.
 * - Success, returns SATrue
 * - Failure, returns SAFalse
 */
SABoolean
SAFileOpen(const char *pathname, SAFile * safile, SAError * errorReturn)
{
	/* Type the file and return the filetype. */
	safile->format.flags = 0;
	safile->swap = 0;
	safile->filetype = findFileType(pathname);
	if (safile->filetype == SANoType)
		return _SASetError(errorReturn, SAError_NO_FILE_TYPE, NULL);

	/* Now open the file and save the fd. */
	safile->fd = open(pathname, O_RDONLY | O_NONBLOCK);
	if (safile->fd < 0)
		return _SASetError(errorReturn, SAError_CANNOT_OPEN, NULL);
	safile->mode = SAOpenForRead;

	/* Based on filetype, parse the header. */
	switch (safile->filetype) {

	case SASunType:
		if (auRead(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	case SAWavType:
		if (wavRead(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	case SAAiffType:
		if (aiffRead(safile->fd, safile, errorReturn) == SAFalse)
			return SAFalse;
		break;

	default:
		break;
	}
	return SATrue;
}

/* Simply does a read from the file. */
SABoolean
SAFileRead(SAFile * safile, char *buf, int bufsize,
	   int *nbytes_read, SAError * errorReturn)
{
	int             nbytes;

	/* Set return value. */
	*nbytes_read = 0;

	/* Check if this file was opened for read. */
	if (safile->mode != SAOpenForRead)
		return _SASetError(errorReturn, SAError_CANNOT_READ, NULL);

	/* Do a read. */
	nbytes = read(safile->fd, buf, bufsize);
	if (nbytes < 0) {
		return _SASetError(errorReturn, SAError_CANNOT_READ, NULL);
	} else
		*nbytes_read = nbytes;

	return SATrue;
}


/* Simply does a write to the file. */
SABoolean
SAFileWrite(SAFile * safile, char *buf, int bufsize,
	    int *nbytes_written, SAError * errorReturn)
{
	int             nwritten;

	/* Set return value. */
	*nbytes_written = 0;

	/* Check if this file was opened for writing. */
	if (safile->mode != SAOpenForWrite)
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* Do a write. */
	nwritten = write(safile->fd, buf, bufsize);
	if (nwritten < 0)
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
	else
		*nbytes_written = nwritten;

	/* Update data_size field.  */
	safile->data_size += nwritten;
	return SATrue;
}

/* Closes the file. */
SABoolean
SAFileClose(SAFile * safile, SAError * errorReturn)
{
	/* Flush header once more, to update data_size field only if this
	 * file was opened for write.
	 */
	if (safile->mode == SAOpenForWrite) {

		if (adjust_file_hdr(safile, errorReturn) == SAFalse)
			return SAFalse;
	}

	/* Close file. */
	if (close(safile->fd) < 0)
		return _SASetError(errorReturn, SAError_CANNOT_CLOSE, NULL);
	return SATrue;
}

/* Set file pointer in file to current sample_number. */
SABoolean
SAFileSeek(SAFile * safile, int sample_number, SAError * errorReturn)
{
	off_t    offset, status;

	offset = safile->header_size + (sample_number *
		 safile->format.sample_size * safile->format.channels);

	if (offset > 0) {

		status = lseek(safile->fd, offset, SEEK_SET);
		if (status < 0)
			return _SASetError(errorReturn, SAError_CANNOT_SEEK, 
					   NULL);
	}
	return SATrue;
}

/* Set file pointer in file to beginning of the data. */
SABoolean
SAFileRewind(SAFile * safile, SAError * errorReturn)
{
	if (SAFileSeek(safile, 0, errorReturn) == SAFalse)
		return SAFalse;

	return SATrue;
}

static void
decode_long(char *from, long *to, SABoolean swap)
{
	if (swap == SATrue) {
		((char *) (to))[0] = ((char *) (from))[3];
		((char *) (to))[1] = ((char *) (from))[2];
		((char *) (to))[2] = ((char *) (from))[1];
		((char *) (to))[3] = ((char *) (from))[0];
	} else {
		*((long *) (to)) = *((long *) (from));
	}
}

/* Try to decode buffer containing a Sun audio file header 
 * into an SAFile structure.
 * Input	- buffer of sun audio data
 * Output	- SAFile structure
 * 		- size of info
 * Returns SATrue success, SAFalse failure
 */
static          SABoolean
decode_sun_filehdr(unsigned char *buf, SAFile * safile, off_t * isize)
{
	unsigned		magic, hdr_size, data_size, encoding, 
				sample_rate, channels;

	/* Decode the 32-bit integer header fields. */
	decode_long((char *) buf, (long *) &magic, safile->swap);
	buf += 4;

	/* Check the magic number. */
	if (magic == AU_SUN_MAGIC || magic == AU_DEC_MAGIC) {

#if defined(_BIG_ENDIAN)
		safile->format.flags = SAFormatFlag_BigEndian;
#elif defined(_LITTLE_ENDIAN)
		safile->format.flags &= ~SAFormatFlag_BigEndian;
#else
#error "Don't know the endianness if this machine"
#endif
		safile->format.flags |= SAFormatFlag_Signed;

	} else if (magic == AU_SUN_INV_MAGIC || magic == AU_DEC_INV_MAGIC) {

#if defined(_BIG_ENDIAN)
		safile->format.flags &= ~SAFormatFlag_BigEndian;
#elif defined(_LITTLE_ENDIAN)
		safile->format.flags = SAFormatFlag_BigEndian;
#else
#error "Don't know the endianness if this machine"
#endif
		safile->format.flags |= SAFormatFlag_Signed;
		safile->swap = SATrue;

	} else
		return SAFalse;

	decode_long((char *) buf, (long *) &hdr_size, safile->swap);
	buf += 4;
	decode_long((char *) buf, (long *) &data_size, safile->swap);
	buf += 4;
	decode_long((char *) buf, (long *) &encoding, safile->swap);
	buf += 4;
	decode_long((char *) buf, (long *) &sample_rate, safile->swap);
	buf += 4;
	decode_long((char *) buf, (long *) &channels, safile->swap);

	/* Set values in the SAFile struct. */
	safile->header_size = hdr_size;
	safile->data_size = data_size;
	safile->format.sample_rate = sample_rate;
	safile->format.channels = channels;
	safile->format.encoding = (SAEncoding) encoding;

	/* Set the info field size (ie, number of bytes left before data). */
	*isize = hdr_size - sizeof(AuFileHeader);

	/* Check the data encoding. */
	switch (encoding) {

	case AUDIO_FILE_ENCODING_MULAW_8:
		safile->format.encoding = SAEncoding_ULAW;
		safile->format.sample_size = 1;
		safile->format.samples_per_unit = 1;
		break;

	case AUDIO_FILE_ENCODING_ALAW_8:
		safile->format.encoding = SAEncoding_ALAW;
		safile->format.sample_size = 1;
		safile->format.samples_per_unit = 1;
		break;

	case AUDIO_FILE_ENCODING_LINEAR_8:
		safile->format.encoding = SAEncoding_LINEAR;
		safile->format.sample_size = 1;
		safile->format.samples_per_unit = 1;
		break;

	case AUDIO_FILE_ENCODING_LINEAR_16:
		safile->format.encoding = SAEncoding_LINEAR;
		safile->format.sample_size = 2;
		safile->format.samples_per_unit = 1;
		break;

	case AUDIO_FILE_ENCODING_LINEAR_24:
		safile->format.encoding = SAEncoding_LINEAR;
		safile->format.sample_size = 3;
		safile->format.samples_per_unit = 1;
		break;

	case AUDIO_FILE_ENCODING_LINEAR_32:
		safile->format.encoding = SAEncoding_LINEAR;
		safile->format.sample_size = 4;
		safile->format.samples_per_unit = 1;
		break;

	case AUDIO_FILE_ENCODING_FLOAT:
		safile->format.encoding = SAEncoding_FLOAT;
		safile->format.sample_size = 4;
		safile->format.samples_per_unit = 1;
		break;

	case AUDIO_FILE_ENCODING_DOUBLE:
		safile->format.encoding = SAEncoding_FLOAT;
		safile->format.sample_size = 8;
		safile->format.samples_per_unit = 1;
		break;

	case AUDIO_FILE_ENCODING_ADPCM_G721:
		safile->format.encoding = SAEncoding_G721;
		safile->format.sample_size = 1;
		safile->format.samples_per_unit = 2;

		/* Set some values for G.721 decoding. These are global to
		 * this type of file!
		 */
		g72x_init_state(&gstate_ui);
		g72x_init_globals(&gglobals_ui);
		g72x_init_state(&gstate_cnv);
		g72x_init_globals(&gglobals_cnv);
			
		break;

	case AUDIO_FILE_ENCODING_ADPCM_G723_3:
		safile->format.encoding = SAEncoding_G723;
		safile->format.sample_size = 3;
		safile->format.samples_per_unit = 8;
		break;

	case AUDIO_FILE_ENCODING_ADPCM_G723_5:
		safile->format.encoding = SAEncoding_G723;
		safile->format.sample_size = 5;
		safile->format.samples_per_unit = 8;
		break;

	default:
		return SAFalse;
	}
	return SATrue;
}

/* Read long, little-endian: little end first. VAX/386 style. */
unsigned long
rllong(int fd)
{
	unsigned char   uc, uc2, uc3, uc4;

	read(fd, &uc, 1);
	read(fd, &uc2, 1);
	read(fd, &uc3, 1);
	read(fd, &uc4, 1);
	return (((long) uc4 << 24) | ((long) uc3 << 16) | 
		((long) uc2 <<  8) |  (long) uc);
}
 
/* Read short, little-endian: little end first. VAX/386 style. */
unsigned short
rlshort(int fd)
{
	unsigned char   uc, uc2;

	read(fd, &uc, 1);
	read(fd, &uc2, 1);
	return (uc2 << 8) | uc;
}
 
/* Read long, big-endian: little end first. 68000/386 style. */
unsigned long
rblong (int fd)
{
        unsigned char uc, uc2, uc3, uc4;

        read (fd, &uc, 1);
        read (fd, &uc2, 1);
        read (fd, &uc3, 1);
        read (fd, &uc4, 1);
        return (((long)uc << 24) | ((long)uc2 << 16) | 
		((long)uc3 << 8) | (long)uc4);
}

/* Read short, big-endian: little end first. 68000/386 style. */
unsigned short
rbshort(int fd)
{
        unsigned char uc, uc2;

        read (fd, &uc2, 1);
        read (fd, &uc, 1);
        return (uc2 << 8) | uc;
}

/* Write short, little-endian: little end first. VAX/386 style. */
unsigned short
wlshort(int fd, unsigned short us)
{
	unsigned char   datum[2];

	datum[0] = (us) & 0xff;
	datum[1] = us >> 8;
	if (write(fd, &datum, 2) != 2)
		return 0;
	else
		return 1;
}

/* Write long, little-endian: little end first. VAX/386 style. */
unsigned long
wllong(int fd, unsigned long ul)
{
	unsigned char   datum[4];

	datum[0] = (ul) & 0xff;
	datum[1] = (ul >> 8) & 0xff;
	datum[2] = (ul >> 16) & 0xff;
	datum[3] = (ul >> 24) & 0xff;
	if (write(fd, &datum, 4) != 4)
		return 0;
	else
		return 1;
}

/* Write long, big-endian: big end first. 68000/SPARC style. */
unsigned long
wblong(int fd, unsigned long ul)
{
	unsigned char   datum[4];

	datum[0] = (ul >> 24) & 0xff;
	datum[1] = (ul >> 16) & 0xff;
	datum[2] = (ul >> 8) & 0xff;
	datum[3] = (ul) & 0xff;
	if (write(fd, &datum, 4) != 4)
		return 0;
	else
		return 1;
}

/* Write short, big-endian: big end first. 68000/SPARC style. */
unsigned short
wbshort(int fd, unsigned short us)
{
	unsigned char   datum[2];

	datum[0] = (us >> 8);
	datum[1] = us;
	if (write(fd, &datum, 2) != 2)
		return 0;
	else
		return 1;
}


/*
 * C O N V E R T   T O   I E E E   E X T E N D E D
 */

/* Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */
static void
ConvertToIeeeExtended(double num, char *bytes)
{
	int             sign;
	int             expon;
	double          fMant, fsMant;
	unsigned long   hiMant, loMant;

	if (num < 0) {
		sign = 0x8000;
		num *= -1;
	} else {
		sign = 0;
	}

	if (num == 0) {
		expon = 0;
		hiMant = 0;
		loMant = 0;
	} else {
		fMant = frexp(num, &expon);
		if ((expon > 16384) || !(fMant < 1)) {	/* Infinity or NaN */
			expon = sign | 0x7FFF;
			hiMant = 0;
			loMant = 0;	/* infinity */
		} else {	/* Finite */
			expon += 16382;
			if (expon < 0) {	/* denormalized */
				fMant = ldexp(fMant, expon);
				expon = 0;
			}
			expon |= sign;
			fMant = ldexp(fMant, 32);
			fsMant = floor(fMant);
			hiMant = FloatToUnsigned(fsMant);
			fMant = ldexp(fMant - fsMant, 32);
			fsMant = floor(fMant);
			loMant = FloatToUnsigned(fsMant);
		}
	}

	bytes[0] = expon >> 8;
	bytes[1] = expon;
	bytes[2] = hiMant >> 24;
	bytes[3] = hiMant >> 16;
	bytes[4] = hiMant >> 8;
	bytes[5] = hiMant;
	bytes[6] = loMant >> 24;
	bytes[7] = loMant >> 16;
	bytes[8] = loMant >> 8;
	bytes[9] = loMant;
}

static void
write_ieee_extended(int fd, double x)
{
	char            buf[10];
	ConvertToIeeeExtended(x, buf);
	(void) write(fd, buf, 10);
}

/*
 * - Writes the Aiff Header from the info in the safile struct.
 * - Always writes little-endian because all aiff files
 *   seems to always be little-endian.
 * - Returns SATrue success, SAFalse failure.
 */
static SABoolean
aiffWrite(int fd, SAFile *safile, SAError *errorReturn)
{
    int   hsize = 
		8 /*COMM hdr*/ + 18 /*COMM chunk*/ +
		8 /*SSND hdr*/ + 12 /*SSND chunk*/;
    int   bits = safile->format.sample_size * 8;
    long  nframes;  

    write (fd, "FORM", 4);  /* IFF header */

    nframes = 0x7f000000 / 
              (safile->format.sample_size * safile->format.channels);
    if (wblong (fd, safile->data_size + 82 + 8) == 0) /* file size */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    if (write (fd, "AIFF", 4) != 4)  /* file type */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

    /*
    // COMM chunk -- describes encoding (and #frames)
    */
    if (write (fd, "COMM", 4) != 4)
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    if (wblong (fd, (long)18) == 0)        /* COMM chunk size */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    if (wbshort (fd, safile->format.channels) == 0)       /* channels */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    if (wbshort (fd, nframes) == 0)        /* number of frames */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    if (wblong (fd, bits) == 0)            /* sample width, in bits */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    write_ieee_extended (fd, (double)safile->format.sample_rate);
    /*
    // SSND chunk -- describes data
    */
    if (write (fd, "SSND", 4) != 4)
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    if (wblong(fd, 8 + nframes * safile->format.channels * safile->format.sample_size) == 0) /* data size */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    if (wblong(fd, (long) 0) == 0) /* offset */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
    if (wblong(fd, (long) 0) == 0) /* block size */
      return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

    return SATrue;

}
 
/*
 * C O N V E R T   F R O M   I E E E   E X T E N D E D  
 */

/* 
 * Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */

/****************************************************************
 * Extended precision IEEE floating-point conversion routine.
 ****************************************************************/

static double 
ConvertFromIeeeExtended(unsigned char *bytes)
{
    double    f;
    int    expon;
    unsigned long hiMant, loMant;
    
    expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
    hiMant    =    ((unsigned long)(bytes[2] & 0xFF) << 24)
            |    ((unsigned long)(bytes[3] & 0xFF) << 16)
            |    ((unsigned long)(bytes[4] & 0xFF) << 8)
            |    ((unsigned long)(bytes[5] & 0xFF));
    loMant    =    ((unsigned long)(bytes[6] & 0xFF) << 24)
            |    ((unsigned long)(bytes[7] & 0xFF) << 16)
            |    ((unsigned long)(bytes[8] & 0xFF) << 8)
            |    ((unsigned long)(bytes[9] & 0xFF));

    if (expon == 0 && hiMant == 0 && loMant == 0) {
        f = 0;
    }
    else {
        if (expon == 0x7FFF) {    /* Infinity or NaN */
            f = HUGE;
        }
        else {
            expon -= 16383;
            f  = ldexp(UnsignedToFloat(hiMant), expon-=31);
            f += ldexp(UnsignedToFloat(loMant), expon-=32);
        }
    }

    if (bytes[0] & 0x80)
        return -f;
    else
        return f;
}

static double 
read_ieee_extended(int fd)
{
	char buf[10];
	if (read(fd, buf, 10) != 10)
	  return 0.0;
	return ConvertFromIeeeExtended((unsigned char *)buf);
}

/* Decodes a wav file header into an SAFile.
 * Returns SATrue success, SAFalse failure
 */
static          SABoolean
decode_wav_header(int fd, SAFile * safile)
{
	unsigned char   buf[sizeof(BUFSIZ)];
	long            length, samples;
	unsigned short  nbits;
	unsigned        nread = 0;
	unsigned        wav_type;
	int             nbytes;
	char            c;

	/* Read the magic number. */
	nbytes = read(fd, (char *) buf, 4);
	nread += 4;
	if (nbytes != 4 || strncmp("RIFF", (char *) buf, 4))
		return SAFalse;

	/* Read the next 4 bytes in little-endian. */
	safile->data_size = rllong(fd);
	nread += 4;

	/* Read another magic number. */
	nbytes = read(fd, (char *) buf, 4);
	nread += 4;
	if (nbytes != 4 || strncmp("WAVE", (char *) buf, 4))
		return SAFalse;

	/* Skip to the next "fmt " or end of file. */
	while (1) {

		nbytes = read(fd, buf, 4);
		nread += 4;
		if (nbytes == 0)
			return SAFalse;
		else if ((nbytes == 4) && !strncmp("fmt ", (char *) buf, 4))
			break;
		length = rllong(fd);
		nread += 4;
		for (length; length > 0; length--) {
			read(fd, &c, 1);
			nread += 1;
		}
	}

	/* Read the data size. */
	length = rllong(fd);
	nread += 4;

	/* Read the wave format type.  We can only do WAVE_FORMAT_PCM
	 * now, the rest are proprietary.
	 */
	wav_type = (unsigned) rlshort(fd);
	nread += 2;
	if (wav_type == WAVE_FORMAT_PCM)
		/* Assume PCM for now. */
		safile->format.encoding = SAEncoding_LINEAR;
	else if (wav_type == WAVE_FORMAT_ALAW)
		safile->format.encoding = SAEncoding_ALAW;
	else if (wav_type == WAVE_FORMAT_MULAW)
		safile->format.encoding = SAEncoding_ULAW;
	else
		return SAFalse;

	/* Assume 1 sample per unit for now. */
	safile->format.samples_per_unit = 1;

	/* Read the number of channels. */
	safile->format.channels = (unsigned) rlshort(fd);
	nread += 2;

	/* Read the sample_rate */
	safile->format.sample_rate = (unsigned) rllong(fd);
	nread += 4;
	rllong(fd);
	nread += 4;
	rlshort(fd);
	nread += 2;
	nbits = rlshort(fd);
	nread += 2;
	safile->format.sample_size = nbits / 8;

	/* Go to the end of this data. */
	length -= 16;
	while (--length >= 0) {
		read(fd, &c, 1);
		nread += 1;
	}

	/* Read the magic number for the start of the data. */
	nbytes = read(fd, (char *) buf, 4);
	nread += 4;

	/* Skip over INFO field if found. */
	if (strncmp("INFO", (char *) buf, 4) == 0 ||
	    strncmp("fact", (char *) buf, 4) == 0) {

		length = rllong(fd);
		nread += 4;
		while (--length >= 0) {
			read(fd, &c, 1);
			nread += 1;
		}
		read(fd, buf, 4);
		nread += 4;	/* read next 4 bytes */
	}
	if (strncmp("data", (char *) buf, 4))
		return SAFalse;

	/* Samples and data_size. */
	samples = rllong(fd);
	nread += 4;
	safile->data_size = samples;

	/* Set header size for easier reset later */
	safile->header_size = nread;

	/* Set endian */
	safile->format.flags &= ~SAFormatFlag_BigEndian;
	return SATrue;
}

/* - Reads/Parses the Sun Header and stuffs values into safile.
 * - Returns SATrue success, SAFalse failure.
 */ 
static SABoolean
auRead(int fd, SAFile * safile, SAError * errorReturn)
{
	unsigned char   buf[sizeof(AuFileHeader)];
	int             nbytes;
	off_t           isize, err;

	/* Read the header (but not the text info).  Decode the hdr into
	 * an Audio_hdr.  Leave the fd ptr at the beginning of the data.
	 */
	nbytes = read(fd, (char *) buf, sizeof(buf));
	if ((nbytes != sizeof(buf)) ||
	    (decode_sun_filehdr(buf, safile, &isize) == SAFalse))
		return _SASetError(errorReturn, SAError_CANNOT_DECODE_FILE, NULL);

	/* Skip over info field and set the fd at the beginning of the
	 * data.
	 */
	err = lseek(fd, isize, SEEK_CUR);
	if (err < 0)
		return _SASetError(errorReturn, SAError_CANNOT_SEEK, NULL);

	return SATrue;
}

/* - Writes the Sun Header from the info into the safile struct.
 * - Returns SATrue success, SAFalse failure.
 */
static          SABoolean
auWrite(int fd, SAFile * safile, SAError * errorReturn)
{
	AuFileHeader    hdr;
	int             nbytes;

	hdr.magic = AU_SUN_MAGIC;
	hdr.sample_rate = safile->format.sample_rate;
	hdr.channels = safile->format.channels;
	hdr.data_size = safile->data_size;
	hdr.encoding = _SAEncode2AudioFileEncode(safile->format.encoding,
						 safile->format.sample_size);
	hdr.hdr_size = sizeof(AuFileHeader) + strlen(AU_INFO_FIELD);

	nbytes = write(safile->fd, (void *) &hdr, sizeof(hdr));
	if (nbytes != sizeof(hdr))
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);
	nbytes = write(safile->fd, (char *) AU_INFO_FIELD, 
		       strlen(AU_INFO_FIELD));
	if (nbytes != strlen(AU_INFO_FIELD))
		return SAFalse;

	return SATrue;
}

/* - Reads/Parses the Wav Header and stuffs values into safile.
 * - Returns SATrue success, SAFalse failure.
 */ 
SABoolean
wavRead(int fd, SAFile * safile, SAError * errorReturn)
{
	/* Read the header into the safile struct. */
	if (decode_wav_header(fd, safile) == SAFalse)
		return _SASetError(errorReturn, SAError_CANNOT_DECODE_FILE, 
				   NULL);
	return SATrue;
}

/* - Writes the Wav Header from the info into the safile struct.
 * - Returns SATrue success, SAFalse failure.
 */
static SABoolean
wavWrite(int fd, SAFile * safile, SAError * errorReturn)
{
	int  samsize = (safile->format.sample_size * 8);

	if (write(fd, "RIFF", 4) != 4)
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* Wavform chunk size: FIXUP(4) */
	if (wllong(fd, safile->data_size + 8 + 16 + 12) == 0)	
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	if (write(fd, "WAVE", 4) != 4)
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	if (write(fd, "fmt ", 4) != 4)
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* fmt chunk size */
	if (wllong(fd, (long) 16) == 0)	
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* FormatTag: WAV_FORMAT_PCM */
	if (wlshort(fd, WAVE_FORMAT_PCM) == 0)	
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* channels */
	if (wlshort(fd, safile->format.channels) == 0)	
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* SamplesPerSec */
	if (wllong(fd, (long) safile->format.sample_rate) == 0)	
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* Avg Bytes/sec, nBlockAlign */
	if (wllong(fd, ((long) safile->format.sample_rate * 
			safile->format.channels * samsize + 7) / 8) == 0)
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* BlockAlign */
	if (wlshort(fd, (safile->format.sample_size * samsize + 7) / 8) == 0)	
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* Sample size */
	if (wlshort(fd, samsize) == 0)	
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	if (write(fd, "data", 4) != 4)
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	/* data chunk size: FIXUP(40) */
	if (wllong(fd, safile->data_size) == 0)	
		return _SASetError(errorReturn, SAError_CANNOT_WRITE, NULL);

	return SATrue;
}

static void
read_string(int fd, char *compname)
{
	char            c;
	int             length;

	read(fd, &c, 1);
	length = (int) c;
	if (length == 0) {
		compname[0] = NULL;
		return;
	}
	read(fd, compname, length);
}

/* Decodes an aiff file header into SAFile.
 * Returns SATrue success, SAFalse failure
 */
static          SABoolean
decode_aiff_header(int fd, SAFile * safile)
{
	unsigned char   buf[sizeof(BUFSIZ)];
	char            comptype[40], compname[128];
	unsigned long   totalsize;
	unsigned long   chunksize;
	unsigned long   blocksize = 0;
	unsigned long   offset;
	unsigned long   frames;
	unsigned long   version, dummy;
	int             bits;
	unsigned        nread = 0;
	int             nbytes;
	char            c;
	SABoolean       aifc = SAFalse;

	/* Make some assumptions about aiff file. */
	safile->format.flags = SAFormatFlag_BigEndian | SAFormatFlag_Signed;
	safile->format.samples_per_unit = 1;
	safile->format.encoding = SAEncoding_LINEAR;


	/* FORM chunk magic number */
	nbytes = read(fd, (char *) buf, 4);
	nread += 4;
	if (nbytes != 4 || strncmp("FORM", (char *) buf, 4))
		return SAFalse;

	/* Total size. */
	totalsize = rblong(fd);
	nread += 4;

	/* Read another magic number. */
	nbytes = read(fd, (char *) buf, 4);
	nread += 4;
	if (nbytes != 4 || (strncmp("AIFF", (char *) buf, 4) != 0 &&
			    strncmp("AIFC", (char *) buf, 4) != 0))
		return SAFalse;
	if (strncmp("AIFC", (char *) buf, 4) == 0)
		aifc = SATrue;

	/* Skip everything but the COMM chunk and the SSND chunk.  The
	 * SSND chunk must be the last in the file.
	 */
	while (1) {

		nbytes = read(fd, buf, 4);
		nread += 4;
		if (nbytes != 4)
			return SAFalse;

		if (strncmp((char *) buf, "FVER", 4) == 0) {

			/* Skip over FVER chunk, AIFC only */
			version = rblong(fd);
			nread += 4;
			dummy = rblong(fd);
			nread += 4;	/* ? */

		} else if (strncmp((char *) buf, "COMM", 4) == 0) {

			/* COMM chunk */
			chunksize = rblong(fd);
			nread += 4;
			if (chunksize != 18 && chunksize != 34)
				return SAFalse;
			safile->format.channels = rbshort(fd);
			nread += 2;
			frames = rblong(fd);
			nread += 4;
			bits = rbshort(fd);
			nread += 2;
			safile->format.sample_rate = 
				(unsigned) read_ieee_extended(fd);
			nread += 10;	/* ? */
			if (aifc == SATrue) {

				/* Compression type and name. */
				nbytes = read(fd, buf, 4);
				nread += 4;	
				strcpy(comptype, (char *) buf);
				read_string(fd, compname);	
				if (strncmp(comptype, "NONE", 4) != 0) {

					if (strncmp(comptype, "G722", 4) == 0)
						dummy = 0;
					else if (strncmp(comptype, "ULAW", 4) == 0)
						dummy = 0;
					else if (strncmp(comptype, "ALAW", 4) == 0)
						dummy = 0;
					else
						return SAFalse;
				}
			}

		} else if (strncmp((char *) buf, "SSND", 4) == 0) {

			/* SSND chunk */
			chunksize = rblong(fd);
			nread += 4;
			offset = rblong(fd);
			nread += 4;
			blocksize = rblong(fd);
			nread += 4;
			break;

		} else {
			chunksize = rblong(fd);
			nread += 4;
			while ((long) (--chunksize) >= 0) {
				nbytes = read(fd, &c, 1);
				nread += 1;
				if (nbytes != 1)
					return SAFalse;
			}
		}
	}

	/* SSND chunk just read */
	if (blocksize != 0)
		return SAFalse;
 	while ((long) (--offset) >= 0) {
		nbytes = read(fd, &c, 1);
		nread += 1;
		if (nbytes != 1)
			return SAFalse;
	}

	switch (bits) {

	case 8:
		safile->format.sample_size = 1;
		break;

	case 16:
		safile->format.sample_size = 2;
		break;

	default:
		return SAFalse;	/* unsupported sample size in Aiff header. */
	}

	safile->header_size = nread;
	safile->data_size = totalsize + 8 - nread;

	return SATrue;
}

/* - Reads/Parses the Aiff Header and stuffs values into safile.
 * - Returns SATrue success, SAFalse failure.
 */
static          SABoolean
aiffRead(int fd, SAFile * safile, SAError * errorReturn)
{
	/* Read the header into the safile struct. */
	if (decode_aiff_header(fd, safile) == SAFalse)
		return _SASetError(errorReturn, SAError_CANNOT_DECODE_FILE, 
				   NULL);
	return SATrue;
}

/* Takes a file path and attemps to type the file into an SAFileType 
 * (au, wav, aiff) file.
 * - First tries pattern (faster).
 * - Then tries content of the file (magic #).
 */
SAFileType
findFileType(const char *path)
{
	SAFileType      filetype = SANoType;
	int             len = 0;
	int             bufsize;
	char            buf[64];
	FILE           *f;

	if (path)
		len = strlen(path);
	if (len == 0)
		return SANoType;

	/* First try typing file by content read in first 64 bytes or so and
	 * type it.
	 */
	if ((f = fopen(path, "r")) != 0) {
		bufsize = fread(buf, 1, sizeof(buf), f);
		fclose(f);
		filetype = findFileTypeByContent(buf, bufsize);
	}

	/* Next try typing by extension */
	if (filetype == SANoType) {

		/* .wav extension */
		if (len >= 4 && path[len - 4] == '.' && 
		    path[len - 3] == 'w' && path[len - 2] == 'a' && 
		    path[len - 1] == 'v')
			filetype = SAWavType;

		/* .aiff extension */
		else if (len >= 5 && path[len - 5] == '.' && 
			 path[len - 4] == 'a' && path[len - 3] == 'i' && 
			 path[len - 2] == 'f' && path[len - 1] == 'f')
			filetype = SAAiffType;

		/* .aif extension */
		else if (len >= 4 && path[len - 4] == '.' && 
			 path[len - 3] == 'a' && path[len - 2] == 'i' && 
			 path[len - 1] == 'f')
			filetype = SAAiffType;

		/* .iff extension */
		else if (len >= 4 && path[len - 4] == '.' &&
			 path[len - 3] == 'i' && path[len - 2] == 'f' &&
			 path[len - 1] == 'f')
			filetype = SAAiffType;

		/* .au extension */
		else if (len >= 3 && path[len - 3] == '.' && 
			 path[len - 2] == 'a' && path[len - 1] == 'u')
			filetype = SASunType;

		/* .aud extension */
		else if (len >= 3 && path[len - 4] == '.' && 
			 path[len - 3] == 'a' && path[len - 2] == 'u' && 
			 path[len - 1] == 'd')
			filetype = SASunType;

#ifdef LATER
		/* WORK FOR FUTURE VERSIONS OF SDTAUDIO.
		 * Do this later because SAFileOpen needs to use the amended
		 * path with .Z.  We need to uncompress this file into /tmp
		 * before opening.  After uncompressing, type by content.
		 */
		else if (len >= 2 && path[len - 2] == '.' && 
		path[len - 1] == 'Z') {

			char            pbuf[MAXPATHLEN];
			char            tmpbuf[PIPE_BUF];
			char            buf[64];
			int             path_len = strlen(path);
			FILE           *pfd;

			sprintf(pbuf, "/usr/bin/zcat %s", path);
			pfd = popen(pbuf, "r");
			if (pfd == NULL)
				return SANoType;
			bufsize = fread(buf, 1, sizeof(buf), pfd);

			/* Drain the pipe before closing. */
			while (fgets(tmpbuf, sizeof(tmpbuf), pfd));
			pclose(pfd);

			filetype = findFileTypeByContent(buf, bufsize);
		}
#endif
	}
	return filetype;
}

/* Type file by content through the buffer passed in. */
static SAFileType
findFileTypeByContent(char *buf, int bufsize)
{
	unsigned long   au_magic;
	char           *bufPtr = buf;

	/* Check if .au file. */
	if (bufsize >= 4) {

		decode_long((char *) buf, (long *) &au_magic, SAFalse);
		if (au_magic == AU_SUN_MAGIC ||
		    au_magic == AU_SUN_INV_MAGIC ||
		    au_magic == AU_DEC_MAGIC ||
		    au_magic == AU_DEC_INV_MAGIC)
			return SASunType;
	}

	if (bufsize >= 12) {

		/* Check if .wav file. */
		if (strncmp("RIFF", bufPtr, 4) == 0) {

			bufPtr += 4;	/* data_size */
			bufPtr += 4;	/* another magic number here. */
			if (strncmp("WAVE", bufPtr, 4) == 0)
				return SAWavType;

		} else if (strncmp("FORM", bufPtr, 4) == 0) {

			/* Check if .aiff file. */
			bufPtr += 4;	/* total size */
			bufPtr += 4;	/* another magic number */
			if (strncmp("AIFF", bufPtr, 4) == 0)
				return SAAiffType;
		}
	}

	return SANoType;
}

static int
alaw2Int(char *ibuf, int ibufsize, long *obuf)
{
	register long   datum;
	int             done = 0;

	while (done < ibufsize) {
		datum = ibuf[done];
		datum = audio_a2l(ibuf[done]);
		obuf[done] = datum;
		done++;
	}

	return done;
}

static int
ulaw2Int(char *ibuf, int ibufsize, long *obuf)
{
	register long   datum;
	int             done = 0;

	while (done < ibufsize) {
		datum = ibuf[done];
		datum = audio_u2l(ibuf[done]);
		obuf[done] = datum;
		done++;
	}

	return done;
}

static unsigned short
swapw(unsigned short us)
{
	return ((us >> 8) | (us << 8)) & 0xffff;
}

static void
rshort(char *ibuf, unsigned short *us, SABoolean swap)
{
	unsigned char   uc, uc2;

	uc2 = ibuf[0];
	uc = ibuf[1];
	*us = (uc2 << 8) | uc;

	if (swap == SATrue)
		*us = swapw(*us);
}

static int
linear2Int(char *ibuf, int ibufsize, long *obuf, SABoolean sign)
{
	register long   datum;
	int             done = 0;

	while (done < ibufsize) {

		datum = (int) (unsigned char) ibuf[done];
		if (sign == SAFalse)
			datum ^= 128;

		/* Scale signed up to long's range */
		datum = LEFT(datum, 24);
		obuf[done] = datum;
		done++;
	}

	return done;
}
	  
static int
linear162Int(char *ibuf, int ibufsize, long *obuf, SABoolean swap)
{
	register long   datum;
	int             done = 0, i = 0;
	unsigned short  us;

	while (done < ibufsize) {

		rshort(&ibuf[done], &us, swap);
		datum = us;
		datum = LEFT(datum, 16);
		obuf[i] = datum;
		done = done + 2;
		i++;
	}

	return i;
}

/****************************
 * G.721 decode utils....
 ****************************/
static int
unpack_input(unsigned char in_byte, unsigned char *code, int tid, SABoolean * increment)
{
	g72x_globals *gglobals = NULL;

	*increment = SAFalse;

	if (tid == 1)
		gglobals = &gglobals_ui;
	else
		gglobals = &gglobals_cnv;

	if (gglobals->_in_bits < gglobals->_dec_bits) {
		gglobals->_in_buffer |= (in_byte << gglobals->_in_bits);
		gglobals->_in_bits += 8;
	} else
		*increment = SATrue;

	*code = gglobals->_in_buffer & ((1 << gglobals->_dec_bits) - 1);
	gglobals->_in_buffer >>= gglobals->_dec_bits;
	gglobals->_in_bits -= gglobals->_dec_bits;
	return (gglobals->_in_bits > 0);
}

/* quan()
 *
 * quantizes the input val against the table of size short integers.
 * It returns i if table[i - 1] <= val < table[i].
 *
 * Using linear search for simple coding.
 */
static int
quan(int val)
{
	int             i;

	for (i = 0; i < power2_count; i++)
		if (val < power2[i])
			break;
	return (i);
}

/* fmult()
 *
 * returns the integer product of the 14-bit integer "an" and
 * "floating point" representation (4-bit exponent, 6-bit mantissa) "srn".
 */
static int
fmult(int an, int srn)
{
	short           anmag, anexp, anmant;
	short           wanexp, wanmant;
	short           retval;

	anmag = (an > 0) ? an : ((-an) & 0x1FFF);
	anexp = quan(anmag) - 6;
	anmant = (anmag == 0) ? 32 :
		(anexp >= 0) ? anmag >> anexp : anmag << -anexp;
	wanexp = anexp + ((srn >> 6) & 0xF) - 13;

	wanmant = (anmant * (srn & 077) + 0x30) >> 4;
	retval = (wanexp >= 0) ? ((wanmant << wanexp) & 0x7FFF) :
		(wanmant >> -wanexp);

	return (((an ^ srn) < 0) ? -retval : retval);
}

/* predictor_pole()
 *
 * computes the estimated signal from 2-pole predictor.
 */
static int
predictor_pole(struct g72x_state * state_ptr)
{
	return (fmult(state_ptr->a[1] >> 2, state_ptr->sr[1]) +
		fmult(state_ptr->a[0] >> 2, state_ptr->sr[0]));
}

/* predictor_zero()
 *
 * computes the estimated signal from 6-zero predictor.
 */
static int
predictor_zero(struct g72x_state * state_ptr)
{
	int             i;
	int             sezi;

	sezi = fmult(state_ptr->b[0] >> 2, state_ptr->dq[0]);
	for (i = 1; i < 6; i++)	/* ACCUM */
		sezi += fmult(state_ptr->b[i] >> 2, state_ptr->dq[i]);
	return (sezi);
}

/* step_size()
 * 
 * computes the quantization step size of the adaptive quantizer.
 */
static int
step_size(struct g72x_state * state_ptr)
{
	int             y;
	int             dif;
	int             al;

	if (state_ptr->ap >= 256)
		return (state_ptr->yu);
	else {
		y = state_ptr->yl >> 6;
		dif = state_ptr->yu - y;
		al = state_ptr->ap >> 2;
		if (dif > 0)
			y += (dif * al) >> 6;
		else if (dif < 0)
			y += (dif * al + 0x3F) >> 6;
		return (y);
	}
}

/* reconstruct()
 *
 * Returns reconstructed difference signal 'dq' obtained from
 * codeword 'i' and quantization step size scale factor 'y'.
 * Multiplication is performed in log base 2 domain as addition.
 */
static int
reconstruct(
	int		sign,	/* 0 for non-negative value */
	int		dqln,	/* G.72x codeword */
	int		y	/* Step size multiplier */
)
{
	short           dql;	/* Log of 'dq' magnitude */
	short           dex;	/* Integer part of log */
	short           dqt;
	short           dq;	/* Reconstructed difference signal sample */

	dql = dqln + (y >> 2);	/* ADDA */

	if (dql < 0) {
		return ((sign) ? -0x8000 : 0);

	} else {		/* ANTILOG */

		dex = (dql >> 7) & 15;
		dqt = 128 + (dql & 127);
		dq = (dqt << 7) >> (14 - dex);
		return ((sign) ? (dq - 0x8000) : dq);
	}
}

/* update()
 *
 * updates the state variables for each output code
 */
void
update(
	int		   code_size,	/* distinguish 723_40 with others */
	int		   y,		/* quantizer step size */
	int		   wi,		/* scale factor multiplier */
	int		   fi,		/* for long/short term energies */
	int		   dq,		/* quantized prediction difference */
	int		   sr,		/* reconstructed signal */
	int		   dqsez,	/* difference from 2-pole predictor */
	struct g72x_state *state_ptr	/* coder state pointer */
	)
{
	int		cnt;
	short		mag, exp;	/* Adaptive predictor, FLOAT A */
	short		a2p;		/* LIMC */
	short		a1ul;		/* UPA1 */
	short		pks1;		/* UPA2 */
	short		fa1;
	char		tr;		/* tone/transition detector */
	short		ylint, thr2, dqthr;
	short  		ylfrac, thr1;
	short		pk0;

	pk0 = (dqsez < 0) ? 1 : 0;   /* needed in updating predictor poles */
	mag = dq & 0x7FFF;	     /* prediction difference magnitude */

	/* TRANS */
	ylint = state_ptr->yl >> 15;	        /* exponent part of yl */
	ylfrac = (state_ptr->yl >> 10) & 0x1F;	/* fractional part of yl */
	thr1 = (32 + ylfrac) << ylint;		/* threshold */
	thr2 = (ylint > 9) ? 31 << 10 : thr1;	/* limit thr2 to 31 << 10 */
	dqthr = (thr2 + (thr2 >> 1)) >> 1;	/* dqthr = 0.75 * thr2 */

	if (state_ptr->td == 0)		/* signal supposed voice */
		tr = 0;
	else if (mag <= dqthr)		/* supposed data, but small mag */
		tr = 0;			/* treated as voice */
	else				/* signal is data (modem) */
		tr = 1;

	/* Quantizer scale factor adaptation. */

	/* FUNCTW & FILTD & DELAY - update non-steady state step size
	 * multiplier.
	 */
	state_ptr->yu = y + ((wi - y) >> 5);

	/* LIMB */
	if (state_ptr->yu < 544)	/* 544 <= yu <= 5120 */
		state_ptr->yu = 544;
	else if (state_ptr->yu > 5120)
		state_ptr->yu = 5120;

	/* FILTE & DELAY - update steady state step size multiplier */
	state_ptr->yl += state_ptr->yu + ((-state_ptr->yl) >> 6);

	/* Adaptive predictor coefficients. */
	if (tr == 1) {			

		/* reset a's and b's for modem signal */
		state_ptr->a[0] = 0;
		state_ptr->a[1] = 0;
		state_ptr->b[0] = 0;
		state_ptr->b[1] = 0;
		state_ptr->b[2] = 0;
		state_ptr->b[3] = 0;
		state_ptr->b[4] = 0;
		state_ptr->b[5] = 0;

	} else {

		/* update a's and b's */
		pks1 = pk0 ^ state_ptr->pk[0];		/* UPA2 */

		/* update predictor pole a[1] */
		a2p = state_ptr->a[1] - (state_ptr->a[1] >> 7);
		if (dqsez != 0) {

			fa1 = (pks1) ? state_ptr->a[0] : -state_ptr->a[0];
			if (fa1 < -8191)	
				a2p -= 0x100;  /* a2p = function of fa1 */
			else if (fa1 > 8191)
				a2p += 0xFF;
			else
				a2p += fa1 >> 5;

			if (pk0 ^ state_ptr->pk[1])
				/* LIMC */
				if (a2p <= -12160)
					a2p = -12288;
				else if (a2p >= 12416)
					a2p = 12288;
				else
					a2p -= 0x80;
			else if (a2p <= -12416)
				a2p = -12288;
			else if (a2p >= 12160)
				a2p = 12288;
			else
				a2p += 0x80;
		}

		/* TRIGB & DELAY */
		state_ptr->a[1] = a2p;

		/* UPA1 - update predictor pole a[0] */
		state_ptr->a[0] -= state_ptr->a[0] >> 8;
		if (dqsez != 0)
			if (pks1 == 0)
				state_ptr->a[0] += 192;
			else
				state_ptr->a[0] -= 192;

		/* LIMD */
		a1ul = 15360 - a2p;
		if (state_ptr->a[0] < -a1ul)
			state_ptr->a[0] = -a1ul;
		else if (state_ptr->a[0] > a1ul)
			state_ptr->a[0] = a1ul;

		/* UPB : update predictor zeros b[6] */
		for (cnt = 0; cnt < 6; cnt++) {

			if (code_size == 5)	/* for 40Kbps G.723 */
				state_ptr->b[cnt] -= state_ptr->b[cnt] >> 9;

			else			/* for G.721 and 24Kbps G.723 */
				state_ptr->b[cnt] -= state_ptr->b[cnt] >> 8;

			if (dq & 0x7FFF) {			/* XOR */
				if ((dq ^ state_ptr->dq[cnt]) >= 0)
					state_ptr->b[cnt] += 128;
				else
					state_ptr->b[cnt] -= 128;
			}
		}
	}

	for (cnt = 5; cnt > 0; cnt--)
		state_ptr->dq[cnt] = state_ptr->dq[cnt-1];

	/* FLOAT A : convert dq[0] to 4-bit exp, 6-bit mantissa f.p. */
	if (mag == 0) {
		state_ptr->dq[0] = (dq >= 0) ? 0x20 : 0xFC20;
	} else {
		exp = quan(mag);
		state_ptr->dq[0] = (dq >= 0) ?
		    (exp << 6) + ((mag << 6) >> exp) :
		    (exp << 6) + ((mag << 6) >> exp) - 0x400;
	}

	state_ptr->sr[1] = state_ptr->sr[0];

	/* FLOAT B : convert sr to 4-bit exp., 6-bit mantissa f.p. */
	if (sr == 0) {
		state_ptr->sr[0] = 0x20;
	} else if (sr > 0) {
		exp = quan(sr);
		state_ptr->sr[0] = (exp << 6) + ((sr << 6) >> exp);
	} else if (sr > -32768) {
		mag = -sr;
		exp = quan(mag);
		state_ptr->sr[0] =  (exp << 6) + ((mag << 6) >> exp) - 0x400;
	} else
		state_ptr->sr[0] = 0xFC20;

	/* DELAY A */
	state_ptr->pk[1] = state_ptr->pk[0];
	state_ptr->pk[0] = pk0;

	/* TONE */
	if (tr == 1)		      /* this sample has been treated as data */
		state_ptr->td = 0;    /* next one will be treated as voice */
	else if (a2p < -11776)	      /* small sample-to-sample correlation */
		state_ptr->td = 1;    /* signal may be data */
	else			      /* signal is voice */
		state_ptr->td = 0;

	/* Adaptation speed control. */
	state_ptr->dms += (fi - state_ptr->dms) >> 5;		/* FILTA */
	state_ptr->dml += (((fi << 2) - state_ptr->dml) >> 7);	/* FILTB */

	if (tr == 1)
		state_ptr->ap = 256;
	else if (y < 1536)					/* SUBTC */
		state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
	else if (state_ptr->td == 1)
		state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
	else if (abs((state_ptr->dms << 2) - state_ptr->dml) >=
	    (state_ptr->dml >> 3))
		state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
	else
		state_ptr->ap += (-state_ptr->ap) >> 4;
}

/* g721_decoder()
 *
 * Description:
 *
 * Decodes a 4-bit code of G.721 encoded data of i and
 * returns the resulting linear PCM, A-law or u-law value.
 * return -1 for unknown out_coding value.
 */
int
g721_decoder(int i, struct g72x_state * state_ptr)
{
	short           sezi, sei, sez, se;	/* ACCUM */
	short           y;	                /* MIX */
	short           sr;	                /* ADDB */
	short           dq;
	short           dqsez;

	i &= 0x0f;		        /* mask to get proper bits */

	sezi = predictor_zero(state_ptr);
	sez = sezi >> 1;
	sei = sezi + predictor_pole(state_ptr);

	se = sei >> 1;		        /* se = estimated signal */
	y = step_size(state_ptr);	/* dynamic quantizer step size */

	dq = reconstruct(i & 0x08, _dqlntab[i], y);	/* quantized diff. */
	sr = (dq < 0) ? (se - (dq & 0x3FFF)) : se + dq;	/* reconst. signal */

	dqsez = sr - se + sez;	/* pole prediction diff. */

	update(4, y, _witab[i] << 5, _fitab[i], dq, sr, dqsez, state_ptr);

	return (sr << 2);	/* sr was 14-bit dynamic range */
}

static void
g72x_init_globals(g72x_globals *state_ptr)
{
   state_ptr->_in_buffer = 0;
   state_ptr->_in_bits = 0;
   state_ptr->_dec_bits = 4;
}

static void
g72x_init_state(struct g72x_state * state_ptr)
{
	int             cnta;

	state_ptr->yl = 34816;
	state_ptr->yu = 544;
	state_ptr->dms = 0;
	state_ptr->dml = 0;
	state_ptr->ap = 0;
	for (cnta = 0; cnta < 2; cnta++) {
		state_ptr->a[cnta] = 0;
		state_ptr->pk[cnta] = 0;
		state_ptr->sr[cnta] = 32;
	}
	for (cnta = 0; cnta < 6; cnta++) {
		state_ptr->b[cnta] = 0;
		state_ptr->dq[cnta] = 32;
	}
	state_ptr->td = 0;
}

static int
g7212Int(char *ibuf, int ibufsize, long *obuf, int tid)
{
	unsigned char   code;
	int             done = 0, i = 0;
	SABoolean       increment = SATrue;
	struct g72x_state       *gstate = NULL;

	if (tid == 1)
		gstate = &gstate_ui;
	else
		gstate = &gstate_cnv;

	while ((done < ibufsize) && (i < ibufsize * 2)) {
		unpack_input((unsigned char) ibuf[done], &code, tid, &increment);
		obuf[i] = g721_decoder(code, gstate);
		obuf[i] = LEFT(obuf[i], 16);
		if (increment == SATrue)
			done++;

		i++;
	}

#ifdef DEBUG
        fprintf(stderr, "G7212INT Returns -> %d:THR-1 %d, THR %d\n", i, thr_self(), tid);
#endif

	return i;
}
	  
static int
int2Ulaw(long *ibuf, int ibufsize, char *obuf)
{
	register long   datum;
	int             done = 0;

	while (done < ibufsize) {
		datum = audio_l2u(ibuf[done]);
		obuf[done] = (unsigned char) datum;
		done++;
	}

	return done;
}

static int
int2Alaw(long *ibuf, int ibufsize, char *obuf)
{
	register long   datum;
	int             done = 0;

	while (done < ibufsize) {
		datum = audio_l2a(ibuf[done]);
		obuf[done] = (unsigned char) datum;
		done++;
	}

	return done;
}

static int
int2Linear(long *ibuf, int ibufsize, char *obuf, SABoolean sign)
{
	register long   datum;
	int             done = 0;

	while (done < ibufsize) {
		datum = RIGHT(ibuf[done], 24);

		/* Convert to unsigned */
		if (sign == SAFalse)
			datum ^= 128;
		obuf[done] = (unsigned char) datum;
		done++;
	}

	return done;
}

static void
wshort(char *obuf, unsigned short us)
{
	obuf[0] = (us >> 8);
	obuf[1] = us;
}

static int
int2Linear16(long *ibuf, int ibufsize, char *obuf, SABoolean swap)
{
	register long   datum;
	int             done = 0, i = 0;
	unsigned short  us;

	while (done < ibufsize) {
		datum = RIGHT(ibuf[done], 16);

		/* Convert to unsigned */
		us = datum;
		if (swap)
			us = swapw(us);
		obuf[i] = (us >> 8);
		obuf[i + 1] = us;
		done++;
		i += 2;
	}

	return i;
}


/*
 * Takes the average and mixes into center channel (or mono). This can be
 * fixed to be more general, currently does 2 -> 1 channels.
 */
static int
channelMixdown(long *ibuf, int ibufsize, long *cbuf, int *cbufsize)
{
	int             done = 0, i = 0;

	while (done < ibufsize) {
		cbuf[i] = ibuf[done] / 2 + ibuf[done + 1] / 2;
		done = done + 2;
		i++;
	}

	*cbufsize = i;

	return done;
}

/* SAConvertData()
 * Used to determine which format we will save or convert the data to
 * based on an in-format and and out-format.
 * - outbuf must be allocated memory.
 * - outbuf can at most be 2 times the size of inbuf for the case
 *   that outbuf is converting from 4-bit G.721 to 16-bit linear.
 * - outbuf is the same size as inbuf in all other cases besides the
 *   one above.
 * - This expects infmt to have encoding, sample_size, samples_per_unit
 * 	channels, flags set! (yuk)
 * - This also expects outfmt to have encoding, channels, sample_size,
 * 	flags set!  The outfmt from SAConvertFormat can be used here.
 */
SABoolean
SAConvertData(SAFormat *infmt, SAFormat *outfmt, char *inbuf,
	      int inbufsize, char *outbuf, int *outbufsize,
	      SAError *errorReturn)
{
	int             done = 0, tmpbufsize = 0;
	long           *tmpbuf = NULL;

	if ((inbufsize == 0) || (*outbufsize == 0))
		return SAFalse;
	if ((inbufsize % SAFormatFrameSize(infmt)) != 0) {
		return _SASetError(errorReturn, SAError_NOT_FRAME, NULL);
	}

#ifdef DEBUG
	fprintf(stderr, "INBUFSIZE -> %d :: THREAD -> %d\n", inbufsize, thr_self());
#endif

	tmpbufsize = inbufsize;
	tmpbuf = (long *) malloc(tmpbufsize * sizeof(long));
	if (tmpbuf == NULL) {
		return _SASetError(errorReturn, SAError_CANNOT_ALLOC, NULL);
	}

#ifdef DEBUG
	fprintf(stderr, "MALLOC ->0x%x :: NO -> %d\n", tmpbuf, tmpbufsize);
#endif

	/* Input Data */

	/* 8-bit ulaw */
	if (infmt->encoding == SAEncoding_ULAW && infmt->sample_size == 1)
		tmpbufsize = ulaw2Int(inbuf, inbufsize, tmpbuf);

	/* 8-bit ulaw */
	else if (infmt->encoding == SAEncoding_ALAW &&
		 infmt->sample_size == 1) {

		tmpbufsize = alaw2Int(inbuf, inbufsize, tmpbuf);
	}

	/* 8-bit linear */
	else if (infmt->encoding == SAEncoding_LINEAR &&
		 infmt->sample_size == 1) {

		tmpbufsize = linear2Int(inbuf, inbufsize, tmpbuf,
			   infmt->flags & SAFormatFlag_Signed);

	} else if (infmt->encoding == SAEncoding_G721 &&
		   infmt->sample_size == 1 &&
		   infmt->samples_per_unit == 2) {

#ifdef DEBUG
	fprintf(stderr, "INBUFSIZE - 2 -> %d :: THREAD -> %d\n", inbufsize, thr_self());
#endif
		tmpbufsize *= 2;
#ifdef DEBUG
	fprintf(stderr, "MALLOC ->0x%x :: NO -> %d\n", tmpbuf, tmpbufsize);
#endif
		tmpbuf = (long *) realloc(tmpbuf, 
					  tmpbufsize * sizeof(long));
		if (tmpbuf == NULL) {
			return _SASetError(errorReturn, 
					   SAError_CANNOT_ALLOC, 
					   NULL);
		}

#ifdef DEBUG
	fprintf(stderr, "MALLOC ->0x%x :: NO -> %d\n", tmpbuf, tmpbufsize);
#endif

		tmpbufsize = g7212Int(inbuf, inbufsize, tmpbuf, thr_self());
	}

	/* 16-bit linear */
	else if (infmt->encoding == SAEncoding_LINEAR &&
		 infmt->sample_size == 2) {

		SABoolean swap = SAFalse;

		if (!(infmt->flags & SAFormatFlag_BigEndian))
			swap = SATrue;
		
		tmpbufsize = linear162Int(inbuf, inbufsize, tmpbuf, swap);
	}

	/* just leave the data as is */
	else {
		memcpy(outbuf, inbuf, inbufsize);
		*outbufsize = inbufsize;
		tmpbufsize = inbufsize;
		if (tmpbuf)
			free(tmpbuf);
		return SATrue;
	}

	/* See if we need to convert channel data. No channel expansion,
	 * just 2 channels to 1.
	 */
	if (infmt->channels == 2 && outfmt->channels == 1) {

		int             cbufsize;
		long           *chanbuf;

		/* Allocate temp buffer for channel mixdown */
		chanbuf = (long *) malloc(tmpbufsize * sizeof(long));
		if (chanbuf == NULL) {
			if (tmpbuf)
				free(tmpbuf);
			return _SASetError(errorReturn, 
					   SAError_CANNOT_ALLOC, 
				           NULL);
		}
		channelMixdown(tmpbuf, tmpbufsize, chanbuf, &cbufsize);

		/* Copy back into tmpbuf for out data processing */
		memcpy(tmpbuf, chanbuf, sizeof(long) * cbufsize);
		tmpbufsize = cbufsize;
		if (chanbuf)
			free(chanbuf);
	}

	/* Output Data */

	/* 8-bit Linear */
	if (outfmt->encoding == SAEncoding_LINEAR &&
	    outfmt->sample_size == 1) {

		*outbufsize = int2Linear(tmpbuf, tmpbufsize, outbuf,
			   outfmt->flags & SAFormatFlag_Signed);
	}

	/* 16-bit Linear */
	else if (outfmt->encoding == SAEncoding_LINEAR &&
		 outfmt->sample_size == 2) {

		SABoolean swap = SAFalse;

		if (!(outfmt->flags & SAFormatFlag_BigEndian))
			swap = SATrue;

		*outbufsize = int2Linear16(tmpbuf, tmpbufsize, outbuf, swap);
	}

	/* 8-bit Ulaw */
	else if (outfmt->encoding == SAEncoding_ULAW &&
		 outfmt->sample_size == 1) {

		*outbufsize = int2Ulaw(tmpbuf, tmpbufsize, outbuf);
	}

	/* 8-bit Alaw */
	else if (outfmt->encoding == SAEncoding_ALAW &&
		 outfmt->sample_size == 1) {

		*outbufsize = int2Alaw(tmpbuf, tmpbufsize, outbuf);
	}

	if (tmpbuf)
		free(tmpbuf);
	return SATrue;
}

/* SAConvertFormat()
 * Takes data attributes and an outfiletype (au, aiff, wav)
 * and determines the "best" output data format (to save out to, e.g.).  
 * This function fills in: outfmt->flags,
 *                         outfmt->channels,
 *                         outfmt->sample_size,
 *                         outfmt->encoding.
 * and this outfmt can be passed directly into SAConvertData.
 * No sample rate conversions.
 */
SABoolean
SAConvertFormat(SAEncoding encoding, int sample_size,
		int channels, int samples_per_unit,
		SAFileType outfiletype, SAFormat * outfmt,
		SAError * errorReturn)
{
	outfmt->flags = 0;
	outfmt->channels = channels;

	/* 8-bit ulaw -> 8-bit linear wav */
	if (encoding == SAEncoding_ULAW &&
	    sample_size == 1 &&
	    outfiletype == SAWavType) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 1;
		outfmt->channels = 1;
	}

	/* 8-bit alaw -> 8-bit linear wav */
	if (encoding == SAEncoding_ALAW &&
	    sample_size == 1 &&
	    outfiletype == SAWavType) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 1;
		outfmt->channels = 1;
	}

	/* 8-bit ulaw-> 8-bit linear aiff */
	else if (encoding == SAEncoding_ULAW &&
		 sample_size == 1 &&
		 outfiletype == SAAiffType) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 1;
		outfmt->flags = SAFormatFlag_Signed;
		outfmt->channels = 1;
	}

	/* 8-bit alaw-> 8-bit linear aiff */
	else if (encoding == SAEncoding_ALAW &&
		 sample_size == 1 &&
		 outfiletype == SAAiffType) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 1;
		outfmt->flags = SAFormatFlag_Signed;
		outfmt->channels = 1;
	}

	/* 4-bit G721 ADPCM -> 8-bit ulaw sun */
	else if (encoding == SAEncoding_G721 &&
		 sample_size == 1 &&
		 samples_per_unit == 2 &&
		 outfiletype == SASunType) {

		outfmt->encoding = SAEncoding_ULAW;
		outfmt->sample_size = 1;
		outfmt->channels = 1;
	}

	/* 4-bit G721 ADPCM -> 8-bit linear wav */
	else if (encoding == SAEncoding_G721 &&
		 sample_size == 1 &&
		 samples_per_unit == 2 &&
		 outfiletype == SAWavType) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 1;
		outfmt->channels = 1;
	}

	/* 4-bit G721 ADPCM -> 8-bit linear aiff */
	else if (encoding == SAEncoding_G721 &&
		 sample_size == 1 &&
		 samples_per_unit == 2 &&
		 outfiletype == SAAiffType) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 1;
		outfmt->flags = SAFormatFlag_Signed;
		outfmt->channels = 1;
	}

	/* 8-bit ulaw -> 8-bit ulaw */
	else if (encoding == SAEncoding_ULAW &&
		 sample_size == 1 &&
		 outfiletype == SASunType) {

		outfmt->encoding = SAEncoding_ULAW;
		outfmt->sample_size = 1;
		outfmt->channels = 1;
	}

	/* 8-bit alaw -> 8-bit ulaw */
	else if (encoding == SAEncoding_ALAW &&
		 sample_size == 1 &&
		 outfiletype == SASunType) {

		outfmt->encoding = SAEncoding_ULAW;
		outfmt->sample_size = 1;
		outfmt->channels = 1;
	}

	/* 16-bit linear -> 16-bit linear */
	else if (encoding == SAEncoding_LINEAR &&
		 sample_size == 2) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 2;
		outfmt->flags = SAFormatFlag_Signed;

		/* Sun can be either, depending on architecture */
		if (outfiletype == SASunType) {

#if defined(_BIG_ENDIAN)
			outfmt->flags |= SAFormatFlag_BigEndian;
#elif defined(_LITTLE_ENDIAN)
			outfmt->flags &= ~SAFormatFlag_BigEndian;
#else
#error "Don't know the endianness of this machine"
#endif
		}

		/* aiff is always big endian */
		else if (outfiletype == SAAiffType)
			outfmt->flags |= SAFormatFlag_BigEndian;

		/* wav is always little endian */
		else if (outfiletype == SAWavType)
			outfmt->flags &= ~SAFormatFlag_BigEndian;
	}

	/* 8-bit linear -> 8-bit ulaw au */
	else if (encoding == SAEncoding_LINEAR &&
		 sample_size == 1 &&
		 outfiletype == SASunType) {

		outfmt->encoding = SAEncoding_ULAW;
		outfmt->sample_size = 1;
		outfmt->channels = 1;
	}

	/* 8-bit linear -> 8-bit linear aiff */
	else if (encoding == SAEncoding_LINEAR &&
		 sample_size == 1 &&
		 outfiletype == SAAiffType) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 1;
		outfmt->flags = SAFormatFlag_Signed;
		outfmt->channels = 1;
	}

	/* 8-bit linear -> 8-bit linear wav */
	else if (encoding == SAEncoding_LINEAR &&
		 sample_size == 1 &&
		 outfiletype == SAWavType) {

		outfmt->encoding = SAEncoding_LINEAR;
		outfmt->sample_size = 1;
		outfmt->channels = 1;
	} else
		_SASetError(errorReturn, SAError_NOT_SUPPORTED, NULL);

	return SATrue;
}

/* Converts audioio.h AUDIO_ENCODING* value to SAEncoding. */
static          SAEncoding
_SAAudioEncode2SAEncode(int encoding)
{
	if (encoding == AUDIO_ENCODING_LINEAR)
		return SAEncoding_LINEAR;
	else if (encoding == AUDIO_ENCODING_ULAW)
		return SAEncoding_ULAW;
	else if (encoding == AUDIO_ENCODING_ALAW)
		return SAEncoding_ALAW;
	else if (encoding == AUDIO_ENCODING_G721)
		return SAEncoding_G721;
	else if (encoding == AUDIO_ENCODING_G722)
		return SAEncoding_G722;
	else if (encoding == AUDIO_ENCODING_G723)
		return SAEncoding_G723;
	else if (encoding == AUDIO_ENCODING_FLOAT)
		return SAEncoding_FLOAT;
	else if (encoding == AUDIO_ENCODING_DVI)
		return SAEncoding_DVI;
	else if (encoding == AUDIO_ENCODING_LINEAR8)
		return SAEncoding_LINEAR8;
	else {
		fprintf(stderr, catgets(msgCatalog, 5, 52, "Unknown encoding - defaulting to SAEncoding_NONE.\n"));
		return SAEncoding_NONE;
	}
}

/* Converts SAEncoding value to audioio.h AUDIO_ENCODING* value. */
static int
_SAEncode2AudioEncode(SAEncoding saencoding)
{
	if (saencoding == SAEncoding_LINEAR)
		return AUDIO_ENCODING_LINEAR;
	else if (saencoding == SAEncoding_ULAW)
		return AUDIO_ENCODING_ULAW;
	else if (saencoding == SAEncoding_ALAW)
		return AUDIO_ENCODING_ALAW;
	else if (saencoding == SAEncoding_G721)
		return AUDIO_ENCODING_G721;
	else if (saencoding == SAEncoding_G722)
		return AUDIO_ENCODING_G722;
	else if (saencoding == SAEncoding_G723)
		return AUDIO_ENCODING_G723;
	else if (saencoding == SAEncoding_LINEAR8)
		return AUDIO_ENCODING_LINEAR8;
	else if (saencoding == SAEncoding_DVI)
		return AUDIO_ENCODING_DVI;
	else if (saencoding == SAEncoding_FLOAT)
		return AUDIO_ENCODING_FLOAT;
	else {
		fprintf(stderr, catgets(msgCatalog, 5, 53, "Unknown encoding - defaulting to AUDIO_ENCODING_NONE.\n"));
		return AUDIO_ENCODING_NONE;
	}
}

/* Converts SAEncoding value to Sun AUDIO_FILE_ENCODING* value */
static int
_SAEncode2AudioFileEncode(SAEncoding saencoding, int sample_size)
{
	if (saencoding == SAEncoding_LINEAR) {
		if (sample_size == 1)
			return AUDIO_FILE_ENCODING_LINEAR_8;
		else if (sample_size == 2)
			return AUDIO_FILE_ENCODING_LINEAR_16;
	} else if (saencoding == SAEncoding_ULAW)
		return AUDIO_FILE_ENCODING_MULAW_8;
	else if (saencoding == SAEncoding_ALAW)
		return AUDIO_FILE_ENCODING_ALAW_8;
	else if (saencoding == SAEncoding_G721)
		return AUDIO_FILE_ENCODING_ADPCM_G721;
	else if (saencoding == SAEncoding_G722)
		return AUDIO_FILE_ENCODING_ADPCM_G722;
	else if (saencoding == SAEncoding_G723) {
		if (sample_size == 3)
			return AUDIO_FILE_ENCODING_ADPCM_G723_3;
		else if (sample_size == 5)
			return AUDIO_FILE_ENCODING_ADPCM_G723_5;
	} else if (saencoding == SAEncoding_LINEAR8)
		return AUDIO_FILE_ENCODING_LINEAR_8;
	else if (saencoding == SAEncoding_FLOAT) {
		if (sample_size == 4)
			return AUDIO_FILE_ENCODING_FLOAT;
		else if (sample_size == 8)
			return AUDIO_FILE_ENCODING_DOUBLE;
	} else {
		fprintf(stderr, catgets(msgCatalog, 5, 53, "Unknown encoding - defaulting to AUDIO_ENCODING_NONE.\n"));
		return AUDIO_ENCODING_NONE;
	}
}

/* Function	: SAFileGetDataSize
 * Objective	: Return the data size of the audio file.  Some *.au
 *		  files may have the field undefined, so we need to 
 * 		  calculate a data size temporarily.
 * Arguments	: SAFile *	- SAFile structure
 * Return Value : unsigned	- data size of audio file.
 */
off_t SAFileGetDataSize(SAFile *safile)
{
	struct stat     stat_buf;

	/* In case safile is nil */
	if (!safile)
		return((off_t) 0);

	/* Certain *.au files may have an uknown data size field.
	 * In this case, calculate the data size by subtracting
	 * the header size from the file size.
	 */
	if (safile->data_size == (off_t) AUDIO_UNKNOWN_SIZE) {
		fstat(safile->fd, &stat_buf);
		return(stat_buf.st_size - safile->header_size);
	} else
		return(safile->data_size);

}


