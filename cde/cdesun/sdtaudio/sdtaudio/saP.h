/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */


#pragma ident "@(#)saP.h	1.12 97/06/04 SMI"

/*
 * SA private file types
 */

#ifndef SAP_H
#define SAP_H

#include <sys/audioio.h>

/**** SA Private funcs ****/

SABoolean _SASetError(SAError * errorReturn, SAErrorCode code,
		      const char * msg);
SABoolean _SADeviceUpdateState(SADevice * device);
float 	  _SADeviceCalculateDelay(SADevice * device);
SABoolean _SADevicePlayChunk(SADevice * device, double time);
int 	  _SADeviceConfigureRecord(SADevice * device);
int 	  _SADeviceConfigurePlay(SADevice * device);
SABoolean _SADeviceConfigure(SAStream *stream, int state);
 
/**** AU SPECIFIC TYPES ****/

/* Define the au magic number */
 
#define AU_SUN_MAGIC       0x2e736e64              /* Really '.snd' */
#define AU_SUN_INV_MAGIC   0x646e732e              /* '.snd' upside-down */
#define AU_DEC_MAGIC       0x2e736400              /* Really '\0ds.' (for DEC) */
#define AU_DEC_INV_MAGIC   0x0064732e              /* '\0ds.' upside-down */

/* Define the encoding fields */
#define AUDIO_FILE_ENCODING_MULAW_8     (1)     /* 8-bit ISDN u-law */
#define AUDIO_FILE_ENCODING_LINEAR_8    (2)     /* 8-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_16   (3)     /* 16-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_24   (4)     /* 24-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_32   (5)     /* 32-bit linear PCM */
#define AUDIO_FILE_ENCODING_FLOAT       (6)     /* 32-bit IEEE floating point */
#define AUDIO_FILE_ENCODING_DOUBLE      (7)     /* 64-bit IEEE floating point */
#define AUDIO_FILE_ENCODING_ADPCM_G721  (23)    /* 4-bit CCITT g.721 ADPCM */
#define AUDIO_FILE_ENCODING_ADPCM_G722  (24)    /* CCITT g.722 ADPCM */
#define AUDIO_FILE_ENCODING_ADPCM_G723_3 (25)   /* CCITT g.723 3-bit ADPCM */
#define AUDIO_FILE_ENCODING_ADPCM_G723_5 (26)   /* CCITT g.723 5-bit ADPCM */
#define AUDIO_FILE_ENCODING_ALAW_8      (27)    /* 8-bit ISDN A-law */

/*
 * Define an on-disk audio file header.
 *
 * This structure should not be arbitrarily imposed over a stream of bytes,
 * since the byte orders could be wrong.
 *
 * Note that there is an 'info' field that immediately follows this
 * structure in the file.
 *
 * The hdr_size field is problematic in the general case because the
 * field is really "data location", which does not ensure that all
 * the bytes between the header and the data are really 'info'.
 * Further, there are no absolute guarantees that the 'info' is ASCII text,
 * (non-ASCII info may eventually be far more useful anyway).
 *
 * When audio files are passed through pipes, the 'data_size' field may
 * not be known in advance.  In such cases, the 'data_size' should be
 * set to AUDIO_UNKNOWN_SIZE.
 */
typedef struct {
        unsigned        magic;          /* magic number */
        unsigned        hdr_size;       /* size of this header */
        unsigned        data_size;      /* length of data (optional) */
        unsigned        encoding;       /* data encoding format */
        unsigned        sample_rate;    /* samples per second */
        unsigned        channels;       /* number of interleaved channels */
} AuFileHeader;

/*
 * The following is the definition of the state structure
 * used by the G.721/G.723 encoder and decoder to preserve their internal
 * state between successive calls.  The meanings of the majority
 * of the state structure fields are explained in detail in the
 * CCITT Recommendation G.721.  The field names are essentially indentical
 * to variable names in the bit level description of the coding algorithm
 * included in this Recommendation.
 */
struct g72x_state {
	long yl;	/* Locked or steady state step size multiplier. */
	short yu;	/* Unlocked or non-steady state step size multiplier. */
	short dms;	/* Short term energy estimate. */
	short dml;	/* Long term energy estimate. */
	short ap;	/* Linear weighting coefficient of 'yl' and 'yu'. */

	short a[2];	/* Coefficients of pole portion of prediction filter. */
	short b[6];	/* Coefficients of zero portion of prediction filter. */
	short pk[2];	/*
			 * Signs of previous two samples of a partially
			 * reconstructed signal.
			 */
	short dq[6];	/*
			 * Previous 6 samples of the quantized difference
			 * signal represented in an internal floating point
			 * format.
			 */
	short sr[2];	/*
			 * Previous 2 samples of the quantized difference
			 * signal represented in an internal floating point
			 * format.
			 */
	char td;	/* delayed tone detect, new in 1988 version */
};

/* Prabhat - To fix 4052334
 * This structure is needed to maintain the global state
 * of variables during a conversion of G72X state
 */
typedef struct {
	unsigned int	_in_buffer;
	int		_in_bits;
	int		_dec_bits;
} g72x_globals;

/**** WAV SPECIFIC TYPES ****/

/* purloined from public Microsoft RIFF docs */
 
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_PCM                 (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define IBM_FORMAT_MULAW                (0x0101)
#define IBM_FORMAT_ALAW                 (0x0102)
#define IBM_FORMAT_ADPCM                (0x0103)

/**** MISC UTILS ****/

#ifdef _BIG_ENDIAN

/* Some wacky processors don't have arithmetic down shift, so do divs */
#define LEFT(datum, bits)       (datum << bits)
/* Most compilers will turn this into a shift if they can, don't worry */
/* #define RIGHT(datum, bits)   (datum / (1L << bits)) */ /* use maybe? */
#define RIGHT(datum, bits)      (datum >> bits)

#endif /*big-endian*/
 
#ifdef _LITTLE_ENDIAN
/* x86 & 68k PC's have arith shift ops and dumb compilers */
#define LEFT(datum, bits)       (datum << bits)
#define RIGHT(datum, bits)      (datum >> bits)

#endif /*little-endian*/

#define FloatToUnsigned(f)      ((unsigned long)(((long)(f - 2147483648.0)) + 2147483647L) + 1)
#define UnsignedToFloat(u)   (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)


/***********************************************************************
 *
 * Debug logging support
 *
 ***********************************************************************/

typedef struct {
    unsigned int locking:1;
    unsigned int buffers:1;
    unsigned int events:1;
    unsigned int timing:1;
    unsigned int errors:1;
    unsigned int saplay:1;
    unsigned int device:1;
} _SADebugStruct;

void _SALog(const char * type, const char * format, ...);
extern _SADebugStruct _SADebugFlags;

#ifdef NDEBUG

#define DEBUGMSG(type, format)
#define DEBUGMSG1(type, format, arg1)
#define DEBUGMSG2(type, format, arg1, arg2)
#define DEBUGMSG3(type, format, arg1, arg2, arg3)
#define DEBUGMSG4(type, format, arg1, arg2, arg3, arg4)

#else

#define DEBUGMSG(type, format)  \
                 _SADebugFlags.type ? _SALog(#type, format) : ((void)0)

#define DEBUGMSG1(type, format, arg1) \
		 _SADebugFlags.type ? _SALog(#type, format, arg1) : ((void)0)

#define DEBUGMSG2(type, format, arg1, arg2) \
		 _SADebugFlags.type ? _SALog(#type, format, arg1, arg2) : ((void)0)

#define DEBUGMSG3(type, format, arg1, arg2, arg3) \
		 _SADebugFlags.type ? _SALog(#type, format, arg1, arg2, arg3) : ((void)0)

#define DEBUGMSG4(type, format, arg1, arg2, arg3, arg4) \
		 _SADebugFlags.type ? _SALog(#type, format, arg1, arg2, arg3, arg4) : ((void)0)

#endif


#endif 
