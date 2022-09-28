/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */


#pragma ident "@(#)safile.h	1.15 97/04/30 SMI"

#ifndef SAFILE_H
#define SAFILE_H

#include <sys/types.h>
#include "sa.h"

/*
 * SAFile I/O implementation.
 */
 
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SANoType,
    SASunType,
    SAWavType,
    SAAiffType
} SAFileType;

typedef enum {
    SANotOpen,
    SAOpenForRead,
    SAOpenForWrite
} SAFileMode;

/*
 * These encoding types are
 * here because these are not defined
 * in <sys/audioio.h>
 */
#define AUDIO_ENCODING_FLOAT    (100)   /* IEEE float (-1. <-> +1.) */
#define AUDIO_ENCODING_G721     (101)   /* CCITT g.721 ADPCM */
#define AUDIO_ENCODING_G722     (102)   /* CCITT g.722 ADPCM */
#define AUDIO_ENCODING_G723     (103)   /* CCITT g.723 ADPCM */

/*
 * SAFile defines the attributes of an audio file.
 * This may become a union struct if more attributes are
 * added that only apply to certain file types.
 */
typedef struct {
    int		 fd;
    SAFileType   filetype;
    SAFileMode   mode;
    SAFormat     format;
    off_t        data_size;
    off_t        header_size;  /* for easy lseeks */
    SABoolean    swap;
} SAFile;


/*
 * Prototypes to SAFile Interface.
 */
SABoolean SAFileCreate(const char *pathname, SAFile *safile, 
		       SAError *errorReturn);
SABoolean SAFileOpen(const char *pathname, SAFile *safile,
		     SAError *errorReturn);
SABoolean SAFileClose(SAFile *safile, SAError *errorReturn);
SABoolean SAFileRead(SAFile *safile, char* buf, int bufsize, 
		     int *nbytes_read, SAError *errorReturn);
SABoolean SAFileWrite(SAFile *safile, char* buf, int bufsize, 
		      int *nbytes_written, SAError *errorReturn);
SABoolean SAFileSeek(SAFile *safile, int sampleNumber, 
		     SAError *errorReturn);
SABoolean SAFileRewind(SAFile *safile, SAError *errorReturn);
SABoolean SAConvertData(SAFormat *infmt, SAFormat *outfmt, char *inbuf, 
			 int inbufsize, char *outbuf, int *outbufsize, 
			 SAError *errorReturn);
SABoolean SAConvertFormat(SAEncoding encoding, int sample_size, int channels, 
			  int samples_per_unit, SAFileType outfiletype, 
			  SAFormat *outfmt, SAError *errorReturn);
off_t     SAFileGetDataSize(SAFile *safile);

SAFileType findFileType(const char *path);

#ifdef  __cplusplus
}
#endif
 
#endif  /* SAFILE_H */
