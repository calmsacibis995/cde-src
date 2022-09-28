/**---------------------------------------------------------------------
***	
***    file:           efsjfif.c
***
***    description:    Image Library (IL) EFS support for JFIF 
***
***	
***    (c)Copyright 1992 Hewlett-Packard Co.
***    
***                             RESTRICTED RIGHTS LEGEND
***    Use, duplication, or disclosure by the U.S. Government is subject to
***    restrictions as set forth in sub-paragraph (c)(1)(ii) of the Rights in
***    Technical Data and Computer Software clause in DFARS 252.227-7013.
***                             Hewlett-Packard Company
***                             3000 Hanover Street
***                             Palo Alto, CA 94304 U.S.A.
***    Rights for non-DOD U.S. Government Departments and Agencies are as set
***    forth in FAR 52.227-19(c)(1,2).
***
***-------------------------------------------------------------------*/

    /*  Code for reading and writing JPEG File Interchange Format (JFIF)
        files, aka .jpg or .jpeg files.  See JFIF and JPEG specs for details.
    */

#include "il.h"
#include "ilefs.h"
#include "ilpipelem.h"
#include "ilerrors.h"
#include <string.h>
#include <stdio.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

    /*  Current majorversion (in APP0 marker) of JFIF supported */
#define SUPPORTED_JFIF_VERSION  1

        /*  JPEG marker codes recognized by this code */
#define JPEGM_FIRST_BYTE    0xFF        /* first byte of all JPEG markers */

#define JPEGM_TEM           0x01        /* TEMporary */
#define JPEGM_SOF0          0xC0        /* Start Of Image, 0 = baseline JPEG */
#define JPEGM_SOI           0xD8        /* Start Of Image */
#define JPEGM_RST0          0xD0        /* ReSeT markers 0..7 */
#define JPEGM_RST1          0xD1
#define JPEGM_RST2          0xD2
#define JPEGM_RST3          0xD3
#define JPEGM_RST4          0xD4
#define JPEGM_RST5          0xD5
#define JPEGM_RST6          0xD6
#define JPEGM_RST7          0xD7
#define JPEGM_EOI           0xD9        /* End Of Image */
#define JPEGM_SOS           0xDA        /* Start Of Scan */
#define JPEGM_APP0          0xE0        /* APPlication marker "0" */



    /*  Private data for each JFIF EFS file */
typedef struct {
    FILE                   *stream;         /* stdio stream file */
    unsigned long           length;         /* # of bytes in file */
    ilEFSPageInfo           info;           /* info on current/only page */
    } jfifPrivRec, *jfifPrivPtr;

    /*  Private data for read/write pipe element functions */
typedef struct {
    FILE                   *stream;         /* stdio stream file */
    unsigned long           length;         /* # of bytes in file */
    unsigned long           height;         /* height of image */
    unsigned long           width;          /* write: width of image */
    unsigned long           xRes, yRes;     /* write: res of image */
    ilBool                  writeDone;      /* a write has been done */
    } jfifRWPrivRec, *jfifRWPrivPtr;


    /*  Get a byte from the given stream; return FALSE if EOF. */
#define GET_BYTE(_stream, _byte) (((_byte) = getc (_stream)) != EOF)



    /*  ------------------------- jfifGetShort ------------------------- */
    /*  Get a short (2 bytes) from the stream, return FALSE if EOF.
    */
static ilBool jfifGetShort (
    register FILE  *stream,
    int            *pValue
    )
{
register unsigned int i, j;

    if (!GET_BYTE (stream, i)
     || !GET_BYTE (stream, j))
        return FALSE;
    *pValue = (i << 8) | (j & 0xff);
    return TRUE;
}


    /*  ------------------------- jfifGetMarker ------------------------- */
    /*  Find the next marker in the file, return it, leave file pointer
        pointing past the marker.
    */
static ilError jfifGetMarker (
    register FILE  *stream,
    int            *pMarker
    )
{
    register int    marker;

        /*  Find a two-byte sequence: 0xff, <marker>, where <marker> is a
           <byte other than 0xff or 0> 
        */
    do {
        do {
            if (!GET_BYTE (stream, marker))
                return IL_ERROR_EFS_FORMAT_ERROR;
            } while (marker != JPEGM_FIRST_BYTE);
        do {
            if (!GET_BYTE (stream, marker))
                return IL_ERROR_EFS_FORMAT_ERROR;
            } while (marker == JPEGM_FIRST_BYTE);
        } while (!marker);          /* skip "0" after ff */

    *pMarker = marker;
    return IL_OK;
}


    /*  ------------------------- jfifSkipBytes --------------------- */
    /*  "Skip" nBytes in the given stream.
    */
static ilError jfifSkipBytes (
    register FILE  *stream,
    register long   nBytes
    )
{
    while (nBytes-- > 0) {
        if (getc (stream) == EOF)
            return IL_ERROR_EFS_FORMAT_ERROR;
        }
    return IL_OK;
}


    /*  ------------------------- jfifSkipMarkerData --------------------- */
    /*  "Skip" the rest of the data in this marker.  stream must
        be pointing at a "length" (2 byte) byte count, which follows the
        marker - on exit, stream is positioned to the next marker, or error.
    */
static ilError jfifSkipMarkerData (
    register FILE  *stream
    )
{
    int             length;

        /*  Get the length (which includes 2 bytes for itself), skip */
    if (!jfifGetShort (stream, &length))
        return IL_ERROR_EFS_FORMAT_ERROR;
    return jfifSkipBytes (stream, length - 2);
}


    /*  ------------------------- jfifFindMarker ------------------------- */
    /*  Find marker "findMarker", or return premature EOF error.
        The given stream is left positioned after the marker.
    */
static ilError jfifFindMarker (
    FILE           *stream,
    int             findMarker
    )
{
    int             marker;
    ilError        error;

    while (TRUE) {
        if (error = jfifGetMarker (stream, &marker))
            return error;                       /* e.g. EOF */
        if (marker == findMarker)               /* found, return ok */
            return IL_OK;
            
            /*  Not desired marker, skip it - determine if length-based */
        switch (marker) {
          case JPEGM_RST0: case JPEGM_RST1: case JPEGM_RST2: case JPEGM_RST3:  
          case JPEGM_RST4: case JPEGM_RST5: case JPEGM_RST6: case JPEGM_RST7:
          case JPEGM_SOI:
          case JPEGM_EOI:
          case JPEGM_TEM:
            break;              /* "stand-alone" markers - no data to skip */

          default:              /* markers with lengths - skip their data */
            if (error = jfifSkipMarkerData (stream))
                return error;
            break;
            }
        }
}


    /*  -------------------- jfifParseSOF0 -------------------------- */
    /*  Parse a SOF0 marker (there or error; also some values are restricted
        by JFIF, and even further by the IL).
            Return the IL descriptor, format, width, height of the image.
    */
static ilError jfifParseSOF0 (
    register FILE  *stream,
    ilImageDes     *pDes,
    ilImageFormat  *pFormat,
    long           *pWidth,
    long           *pHeight
    )
{
    int             marker, precision, nSamples, sample, i, factor;
    int             horiz[IL_MAX_SAMPLES], vert[IL_MAX_SAMPLES];
    int             maxHoriz, maxVert;
    int             length;
    ilError         error;

        /*  Skip to SOF0 marker; error will be returned if not found */
    if (error = jfifFindMarker (stream, JPEGM_SOF0))
        return error;

        /*  Get length; must skip length - # bytes read, starting here. */
     if (!jfifGetShort (stream, &length))
        return IL_ERROR_EFS_FORMAT_ERROR;

        /*  Get the precision (bits per pixel); only 8 supported currently */
    if (!GET_BYTE (stream, precision))
        return IL_ERROR_EFS_FORMAT_ERROR;
    if (precision != 8)
        return IL_ERROR_EFS_UNSUPPORTED;

        /*  Get # of lines; "0" => DNL marker - unsupported; get width */
     if (!jfifGetShort (stream, &i))
        return IL_ERROR_EFS_FORMAT_ERROR;
     if (i <= 0)
        return IL_ERROR_EFS_UNSUPPORTED;
     *pHeight = i;
     if (!jfifGetShort (stream, &i))
        return IL_ERROR_EFS_FORMAT_ERROR;
     *pWidth = i;

        /*  Get # of samples (components); == 1 => gray, == 3 => YCbCr.
            Fill in pDes, pFormat, except for data set from marker data.
        */
    bzero ((char *)pDes, sizeof (*pDes));       /* zero: init "reserved" */
    bzero ((char *)pFormat, sizeof (*pFormat));

    if (!GET_BYTE (stream, nSamples))
        return IL_ERROR_EFS_FORMAT_ERROR;
    if (nSamples == 1)
        pDes->type = IL_GRAY;
    else if (nSamples == 3)
        pDes->type = IL_YCBCR;
    else return IL_ERROR_EFS_NOT_MINE;

    pDes->compression = IL_JPEG;
    pDes->blackIsZero = TRUE;           /* 0 = black for gray images */
    pDes->nSamplesPerPixel = nSamples;

        /*  Fill in the YCbCr color space info, based on JFIF spec. */
    pDes->typeInfo.YCbCr.sample[0].refBlack = 0;
    pDes->typeInfo.YCbCr.sample[0].refWhite = 255;
    pDes->typeInfo.YCbCr.sample[1].refBlack = 128;      /* Cr */
    pDes->typeInfo.YCbCr.sample[1].refWhite = 255;
    pDes->typeInfo.YCbCr.sample[2].refBlack = 128;      /* Cb */
    pDes->typeInfo.YCbCr.sample[2].refWhite = 255;
    pDes->typeInfo.YCbCr.lumaRed = 2990;
    pDes->typeInfo.YCbCr.lumaGreen = 5870;
    pDes->typeInfo.YCbCr.lumaBlue = 1140;
    pDes->typeInfo.YCbCr.positioning = 0;

    pFormat->sampleOrder = IL_SAMPLE_PIXELS;    /* JFIF is JPEG interleaved */
    pFormat->byteOrder = IL_MSB_FIRST;  /* arbitrary when precision <= 8 */
    pFormat->rowBitAlign = 8;           /* arbitrary for compressed images */

        /*  Read "nSamples" component-spec pars; set des subsampling.
            Store horizontal/vertical sample factors, find max; see below.
        */
    maxHoriz = maxVert = 0;
    for (sample = 0; sample < nSamples; sample++) {
        pFormat->nBitsPerSample[sample] = precision;
        pDes->nLevelsPerSample[sample] = 1 << precision;   /* e.g. 256 for 8 */

        if (!GET_BYTE (stream, i))      /* ignore component id */
            return IL_ERROR_EFS_FORMAT_ERROR;
        if (!GET_BYTE (stream, i))      /* two nibbles: Hi and Vi */
            return IL_ERROR_EFS_FORMAT_ERROR;
        factor = i >> 4;                /* horizontal in upper nibble */
        if (factor > maxHoriz)
            maxHoriz = factor;          /* save maximum */
        horiz[sample] = factor;
        factor = i & 0xF;               /* vertical in lower nibble */
        if (factor > maxVert)
            maxVert = factor;           /* save maximum */
        vert[sample] = factor;

        if (!GET_BYTE (stream, i))      /* ignore quantitization table # */
            return IL_ERROR_EFS_FORMAT_ERROR;
        }

        /*  Change JFIF sample factors to IL subsample.  JFIF factors specify
            "the number of horizontal/vertical data units" of each component
            in each MCU.  IL specify the amount of subsampling - they are
            roughly reciprocals.
        */
    for (sample = 0; sample < nSamples; sample++) {
        pDes->typeInfo.YCbCr.sample[sample].subsampleHoriz = 
            maxHoriz / horiz [sample];
        pDes->typeInfo.YCbCr.sample[sample].subsampleVert = 
            maxVert / vert [sample];
        }

        /*  Skip rest of data; "length" - bytes read after length */
    return jfifSkipBytes (stream, length - (7 + nSamples * 3));
}


        /*  ----------------------- jfifReadHeader ------------------------ */
        /*  Read the front of the given stream; return error if not a JFIF
            file stream, else return x/yRes from header, in dpi * 2.
            The given stream is left positioned after JFIF APP0 marker.
        */
static ilError jfifReadHeader (
    FILE                   *stream,
    long                   *pXRes,
    long                   *pYRes
    )
{
int                         i, resUnits, length;
int                         xRes, yRes;


        /*  Read first two markers; must be <SOI>, <APP0> */
    if (!GET_BYTE (stream, i)
     || (i != JPEGM_FIRST_BYTE)
     || !GET_BYTE (stream, i)
     || (i != JPEGM_SOI)
     || !GET_BYTE (stream, i)
     || (i != JPEGM_FIRST_BYTE)
     || !GET_BYTE (stream, i)
     || (i != JPEGM_APP0))
        return IL_ERROR_EFS_NOT_MINE;

        /*  Get length of the marker; "length" starts counting here. */
     if (!jfifGetShort (stream, &length))
        return IL_ERROR_EFS_NOT_MINE;

        /*  Must now be "JFIF<0>" */
    if (!GET_BYTE (stream, i)
     || (i != 'J')
     || !GET_BYTE (stream, i)
     || (i != 'F')
     || !GET_BYTE (stream, i)
     || (i != 'I')
     || !GET_BYTE (stream, i)
     || (i != 'F')
     || !GET_BYTE (stream, i)
     || (i != 0))
        return IL_ERROR_EFS_NOT_MINE;

        /*  Check major version #, for future version changes */
    if (!GET_BYTE (stream, i))
        return IL_ERROR_EFS_NOT_MINE;
    if (i != SUPPORTED_JFIF_VERSION)
        return IL_ERROR_EFS_UNSUPPORTED;
    if (!GET_BYTE (stream, i))              /* skip minor version # */
        return IL_ERROR_EFS_NOT_MINE;

        /*  Get res units and return resolution */
    if (!GET_BYTE (stream, resUnits)
     || !jfifGetShort (stream, &xRes)
     || !jfifGetShort (stream, &yRes))
        return IL_ERROR_EFS_NOT_MINE;

    switch (resUnits) {
      default:
      case 0:               /* no resolution, x/y are just aspect ratio */
        xRes = yRes = 100;  /* fake it - assume 100 dpi */
      case 1:               /* x/y are already in dpi */
        break;
      case 2:               /* x/y are in dots/cm; convert to dpi */
        xRes = ((double)xRes * 2.54 + 0.5);
        yRes = ((double)yRes * 2.54 + 0.5);
        break;
        }
    *pXRes = xRes * 2;      /* return IL std: dpi * 2 */
    *pYRes = yRes * 2;

        /*  Skip rest of data; "length" - bytes read after length */
    return jfifSkipBytes (stream, length - 14);
}


        /*  ------------------------ jfifOpen ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Open().  Open the given file, return
            the number of pages in it and ptr to private, or null if error.
        */
static ilPtr jfifOpen (
    ilEFSFileType           fileType,
    char                   *fileName,
    unsigned int            openMode,
    unsigned long          *pNPages
    )
{
register ilError            error;
register jfifPrivPtr        pPriv;
FILE                       *stream;
ilEFSPageInfo               info;
unsigned long               length;

        /*  Open the file, write mode if requested.  Error if open fails. */
    if (openMode == IL_EFS_WRITE) {
        stream = fopen (fileName, "w");
        if (!stream) {
            fileType->context->error = IL_ERROR_EFS_OPEN;
            return (ilPtr)NULL;
            }
        }
    else {
            /*  Opening in read mode: connect to file, return "not mine" error
                if not a JFIF file.  Do a quick check of front of file first:
                APP0, JFIF marker.  Then find and SOF0 and SOF markers, and
                extract the necessary image info.
                Get length of file by positioning to end.
            */
        stream = fopen (fileName, "r");
        if (!stream) {
            fileType->context->error = IL_ERROR_EFS_OPEN;
            return (ilPtr)NULL;
            }

        if ((error = jfifReadHeader (stream, &info.xRes, &info.yRes))
         || (error = jfifParseSOF0 (stream, &info.des, &info.format, 
                                    &info.width, &info.height)))
            goto openError;

        if (fseek (stream, 0, 2)) { /* seek to eof; nonzero return is error */
            error = IL_ERROR_FILE_IO;
            goto openError;
            }
        length = ftell (stream);    /* length is now current position */
        *pNPages = 1;
        }   /* END a read */


        /*  Allocate private, save info in it and return a ptr to it. */
    pPriv = (jfifPrivPtr)malloc (sizeof (jfifPrivRec));
    if (!pPriv) {
        error = IL_ERROR_MALLOC;
        goto openError;
        }
    pPriv->stream = stream;
    pPriv->length = length;
    info.page = 0;                  /* single page file; always page 0 */
    info.attributes = 0;
    pPriv->info = info;

    fileType->context->error = IL_OK;
    return (ilPtr)pPriv;

    /*  goto point on error after file open: post "error", close file. */
openError:
    fileType->context->error = error;
    fclose (stream);
    return (ilPtr)NULL;
}


        /*  ------------------------ jfifClose ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Close().  Close the given file.
        */
static ilBool jfifClose (
    ilEFSFile               file
    )
{
    fclose (((jfifPrivPtr)file->pPrivate)->stream);
    return TRUE;
}


        /*  ------------------------ jfifGetPageInfo ---------------------- */
        /*  Function for ilEFSFileTypeInfo.GetPageInfo().  Return information
            on the current page.  Called only if file opened in read mode.
        */
static ilBool jfifGetPageInfo (
    ilEFSFile               file,
    ilEFSPageInfo          *pInfo
    )
{
        /*  Return info on the current (only) image in the file. */
    *pInfo = ((jfifPrivPtr)file->pPrivate)->info;
    return TRUE;
}


        /*  --------------------- jfifReadImageExecute --------------------- */
        /*  Execute() for reading JFIF.  Read the whole JIF image out to the 
            dst image and signal last strip.
        */
static ilError jfifReadImageExecute (
    ilExecuteData          *pData,
    long                    dstLine,
    long                   *pNLines
    )
{
register jfifRWPrivPtr      pPriv;
long                        dstOffset, nBytes;
ilPtr                       pDst;
ilImagePlaneInfo           *pPlane;

    pPriv = (jfifRWPrivPtr)pData->pPrivate;
    nBytes = pPriv->length;

        /*  Read "JIFLength" bytes from "JIFOffset" in the file to dst image.
            Compressed images are handled differently from regular images, in
            that one doesnt know how big a buffer to allocate for a compressed
            images, so none is pre-allocated.  Instead, each pipe element that
            outputs compressed data must determine if the output buffer is big
            enough, and it not it must realloc / malloc for the first time.
        */
    pPlane = pData->pDstImage->plane;
    dstOffset = *pData->compressed.pDstOffset;
    if ((dstOffset + nBytes) > pPlane->bufferSize) {
        pPlane->bufferSize = dstOffset + nBytes;
        if (!pPlane->pPixels) 
            pPlane->pPixels = (ilPtr)malloc (pPlane->bufferSize);
        else pPlane->pPixels = (ilPtr)realloc (pPlane->pPixels, 
                                               pPlane->bufferSize);
        if (!pPlane->pPixels) {
            pPlane->bufferSize = 0;
            return IL_ERROR_MALLOC;
            }
        }

        /*  Read "nBytes" into buffer at "dstOffset" */
    pDst = pPlane->pPixels + dstOffset;
    if (fseek (pPriv->stream, 0, 0)) /* seek to start; != 0 return => error */
        return IL_ERROR_FILE_IO;
    if (fread (pDst, nBytes, 1, pPriv->stream) != 1)
        return IL_ERROR_FILE_IO;     /* failed to read 1 record; error */

    *pNLines = pPriv->height;
    *pData->compressed.pNBytesWritten = nBytes;
    return IL_ERROR_LAST_STRIP;      /* the single strip was read */
}


        /*  ------------------------ jfifReadImage ---------------------- */
        /*  Function for ilEFSFileTypeInfo.ReadImage().  Add a producer element
            to read the current page into the given pipe.
        */
static ilBool jfifReadImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned int            readMode        /* ignored - no mask images */
    )
{
register jfifPrivPtr        pPriv;
register jfifRWPrivPtr      pRWPriv;
ilDstElementData            dstData;

        /*  Add a pipe producer to read the entire JFIF file into the pipe.
            The des, format and size are in private, from jfifOpen().
        */
    pPriv = (jfifPrivPtr)file->pPrivate;
    dstData.producerObject = (ilObject)NULL;
    dstData.pDes = &pPriv->info.des;
    dstData.pFormat = &pPriv->info.format;
    dstData.width = pPriv->info.width;
    dstData.height = pPriv->info.height;
    dstData.stripHeight = pPriv->info.height;   /* JFIF is one big strip */
    dstData.constantStrip = TRUE;
    dstData.pPalette = (unsigned short *)NULL;
    dstData.pCompData = (ilPtr)NULL;

    pRWPriv = (jfifRWPrivPtr)ilAddPipeElement (pipe, IL_PRODUCER, 
        sizeof (jfifRWPrivRec), 0, (ilSrcElementData *)NULL, &dstData,
        IL_NPF, IL_NPF, IL_NPF, jfifReadImageExecute, 0);
    if (!pRWPriv)
        return FALSE;

        /*  Element added; copy relevant data into pipe element private */
    pRWPriv->stream = pPriv->stream;
    pRWPriv->length = pPriv->length;
    pRWPriv->height = pPriv->info.height;
    return TRUE;
}


        /*  --------------------- jfifWriteImageExecute -------------------- */
        /*  Pipe element Execute() for writing JFIF files.
        */
static ilError jfifWriteImageExecute (
    register ilExecuteData *pData,
    long                    dstLine,    /* not used */
    long                   *pNLines
    )
{
register jfifRWPrivPtr      pPriv;
unsigned long               nBytesTotal, length;
ilPtr                       pBytes;
register ilPtr              pByte;
register unsigned long      nBytes, i;
#define WRITE_HEADER_SIZE   20
char                        header [WRITE_HEADER_SIZE];

    pPriv = (jfifRWPrivPtr)pData->pPrivate;
    if (*pNLines != pPriv->height)      /* ? not complete strip - error */
        return IL_ERROR_EFS_IO;

        /*  Exit if a write has already been done, as per EFS spec. */
    if (pPriv->writeDone)
        return IL_OK;
    pPriv->writeDone = TRUE;

        /*  Get ptr and length of src compressed data */
    nBytesTotal = pData->compressed.nBytesToRead;
    pBytes = pData->pSrcImage->plane[0].pPixels + pData->compressed.srcOffset;

        /*  Point pByte past SOI marker, and APP0/"JFIF" marker if present */
    pByte = pBytes;
    nBytes = nBytesTotal;
    if ((nBytes -= 2) <= 0)
        return IL_ERROR_EFS_FORMAT_ERROR;
    pByte += 2;                         /* skip SOI marker */
    if ((nBytes > 9)
     && (pByte[0] == JPEGM_FIRST_BYTE)
     && (pByte[1] == JPEGM_APP0)
     && (pByte[4] == 'J')
     && (pByte[5] == 'F')
     && (pByte[6] == 'I')
     && (pByte[7] == 'F')
     && (pByte[8] == 0)) {
        pByte += 2;                     /* position to length of APP0 marker */
        length = *pByte;
        length <<= 8;
        length |= pByte[1];
        pByte += length;                /* point pByte past APP0 data */
        }

        /*  Form and write header data: SOI, JFIF marker data */
    header[0] = JPEGM_FIRST_BYTE;
    header[1] = JPEGM_SOI;
    header[2] = JPEGM_FIRST_BYTE;
    header[3] = JPEGM_APP0;
    header[4] = 0;                      /* length = 16, MS byte first */
    header[5] = 16;
    header[6] = 'J';
    header[7] = 'F';
    header[8] = 'I';
    header[9] = 'F';
    header[10] = 0;
    header[11] = 1;                     /* major version # */
    header[12] = 1;                     /* minor version # */
    header[13] = 1;                     /* resUnits = 1 = dpi */
    i = pPriv->xRes / 2;
    header[14] = i >> 8;
    header[15] = i & 0xff;              /* res in dpi */
    i = pPriv->yRes / 2;
    header[16] = i >> 8;
    header[17] = i & 0xff;
    header[18] = 0;                     /* finish w/ 0's - dont know why */
    header[19] = 0;
    if (fwrite ((char *)header, WRITE_HEADER_SIZE, 1, pPriv->stream) != 1) 
        return IL_ERROR_FILE_IO;            /* EXIT */

        /*  # of bytes to write after SOI and APP0 markers is total # bytes,
            less bytes skipped above.  Error if no bytes, else write nBytes.
        */
    nBytes = nBytesTotal - (pByte - pBytes);
    if (nBytes <= 0)
        return IL_ERROR_EFS_FORMAT_ERROR;
    if (fwrite ((char *)pByte, nBytes, 1, pPriv->stream) != 1) 
        return IL_ERROR_FILE_IO;            /* EXIT */

    fflush (pPriv->stream);             /* flush output buffers */
    return IL_OK;
}

        /*  ------------------------ jfifWriteImage ---------------------- */
        /*  Function for ilEFSFileTypeInfo.WriteImage().  Add a consumer
            to write the pipe image out to the file.
        */
static ilBool jfifWriteImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned long           xRes,
    unsigned long           yRes,
    ilClientImage           maskImage       /* ignored - no mask images */
    )
{
register jfifPrivPtr        pPriv;
register jfifRWPrivPtr      pRWPriv;
ilPipeInfo                  info;
ilImageDes                  des;
ilImageFormat               format;

        /*  Query the pipe (dont decompress!); error if not "forming". */
    if (ilGetPipeInfo (pipe, FALSE, &info, &des, &format) != IL_PIPE_FORMING) {
        if (!pipe->context->error)
            ilDeclarePipeInvalid (pipe, IL_ERROR_PIPE_STATE);
        return FALSE;
        }

        /*  Convert if necessary to a subsampled by 2 YCbCr image */
    if ((des.type != IL_YCBCR)
     || (des.nLevelsPerSample[0] != 256)
     || (des.nLevelsPerSample[1] != 256)
     || (des.nLevelsPerSample[2] != 256)
     || (des.typeInfo.YCbCr.sample[0].refBlack != 0)
     || (des.typeInfo.YCbCr.sample[0].refWhite != 255)
     || (des.typeInfo.YCbCr.sample[0].subsampleHoriz != 1)
     || (des.typeInfo.YCbCr.sample[0].subsampleVert != 1)
     || (des.typeInfo.YCbCr.sample[1].refBlack != 128)
     || (des.typeInfo.YCbCr.sample[1].refWhite != 255)
     || (des.typeInfo.YCbCr.sample[1].subsampleHoriz != 2)
     || (des.typeInfo.YCbCr.sample[1].subsampleVert != 2)
     || (des.typeInfo.YCbCr.sample[2].refBlack != 128)
     || (des.typeInfo.YCbCr.sample[2].refWhite != 255)
     || (des.typeInfo.YCbCr.sample[2].subsampleHoriz != 2)
     || (des.typeInfo.YCbCr.sample[2].subsampleVert != 2)
     || (des.typeInfo.YCbCr.lumaRed != 2990)
     || (des.typeInfo.YCbCr.lumaGreen != 5870)
     || (des.typeInfo.YCbCr.lumaBlue != 1140)
     || (des.typeInfo.YCbCr.positioning != 0)) {
        if (!ilConvert (pipe, IL_DES_YCBCR_2, (ilImageFormat *)NULL, 0, NULL))
            return FALSE;
        ilGetPipeInfo (pipe, FALSE, &info, &des, &format);
        }

        /*  Compress to JPEG Interchange Format (JIF), of which JFIF
            is a subset.  Will be a noop if already correctly compressed.
        */
    if (!ilCompress (pipe, IL_JPEG, (ilPtr)NULL, 0, 0))
        return FALSE;

        /*  Add a consumer element to write the resulting data. */
    pPriv = (jfifPrivPtr)file->pPrivate;
    pRWPriv = (jfifRWPrivPtr)ilAddPipeElement (pipe, IL_CONSUMER,
        sizeof (jfifRWPrivRec), 0, (ilSrcElementData *)NULL, 
        (ilDstElementData *)NULL, IL_NPF, IL_NPF, IL_NPF, 
        jfifWriteImageExecute, 0);
    if (!pRWPriv)
        return FALSE;

        /*  Element added; copy relevant data into pipe element private */
    pRWPriv->stream = pPriv->stream;
    pRWPriv->height = info.height;
    pRWPriv->width = info.width;
    pRWPriv->xRes = xRes;
    pRWPriv->yRes = yRes;
    pRWPriv->writeDone = FALSE;
    return TRUE;
}


        /*  ------------------------ _ilefsInitJFIF ------------------------- */
        /*  Add JFIF EFS file type handler to the given context; 
            return false if error.
        */
ilBool _ilefsInitJFIF (
    ilContext               context
    )
{
ilEFSFileTypeInfo           info;

    bzero (&info, sizeof (ilEFSFileTypeInfo));

    strcpy (info.name, "JFIF");
    strcpy (info.displayName, "JFIF");
    info.checkOrder = 80;                   /* somewhat arbitrary */

    info.nExtensions = 4;
    strcpy (info.extensions[0], "jpg");
    strcpy (info.extensions[1], "JPG");
    strcpy (info.extensions[2], "jpeg");
    strcpy (info.extensions[3], "JPEG");

    info.openModes = (1<<IL_EFS_READ) | (1<<IL_EFS_READ_SEQUENTIAL) |
                     (1<<IL_EFS_WRITE);
    info.attributes = 0;

    info.Open = jfifOpen;
    info.GetPageInfo = jfifGetPageInfo;
    info.ReadImage = jfifReadImage;
    info.WriteImage = jfifWriteImage;
    info.Close = jfifClose;

    return (ilEFSAddFileType (context, &info, NULL) != NULL);
}



