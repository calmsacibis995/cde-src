/**---------------------------------------------------------------------
***	
***    file:           efsgif.c
***
***    description:    Image Library (IL) EFS support for GIF 
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

    /*  Private data for each GIF EFS file */
typedef struct {
    FILE                   *stream;         /* stdio stream file */
    unsigned long           height;         /* height of image */
    unsigned long           width;          /* width of image */
    ilBool                  interlaced;     /* is interlaced */
    unsigned long           imageOffset;    /* offset to start of image data */
    unsigned long           imageLength;    /* # of bytes of image data */
    int                     bitMask;        /* mask of used bits */
    unsigned short          palette[3*256]; /* file's palette, in IL form */
    } gifPrivRec, *gifPrivPtr;


    /*  Private data for read/write pipe element functions */
typedef struct {
    FILE                   *stream;         /* stdio stream file */
    unsigned long           height;         /* height of image */
    unsigned long           width;          /* width of image */
    ilBool                  writeDone;      /* write: write has been done */

        /*  Rest of fields only for reading */
    ilBool                  interlaced;     /* is interlaced */
    unsigned long           imageOffset;    /* offset to start of image data */
    unsigned long           imageLength;    /* # of bytes of image data */
    int                     bitMask;        /* mask of used bits */
    } gifRWPrivRec, *gifRWPrivPtr;



/* ========================= GIF READ CODE =============================== */

typedef struct {
        /*  Values inited by Execute() */
    ilPtr                   pSrcData;       /* ptr to input (LZW compressed) data */
    ilPtr                   pDstData;       /* ptr to output (decompressed) data */
    ilBool                  interlaced;     /* is interlaced */
    int                     width, height;  /* size of image in pixels (bytes) */
    int                     mask;           /* bits to "and", based on # bits / code */

        /*  Values used locally by decompress code */
    long                    bitPosition;    /* position of current bit in code table */
    int                     nBitsInCode;    /* # of bits in a code (<= 12) */
    int                     codeMask;       /* mask for code, = (1<<nBitsInCode) - 1 */
    ilPtr                   pInterlace;     /* rest of vars used by interlace code */
    int                     xPos, yPos, prevYPos, interlaceIndex;
    } gifDecodePrivRec, *gifDecodePrivPtr;


    /*  ------------------------- GetString -------------------------- */
static int GetString (
    register gifDecodePrivPtr pG
    )
{
register int                  i, offset;
  
    offset = pG->bitPosition >> 3;
    i = pG->pSrcData[offset++];
    i += (pG->pSrcData[offset++] << 8);
    if (pG->nBitsInCode >= 8)
        i += (pG->pSrcData[offset] << 16);
    i >>= (pG->bitPosition % 8);
    pG->bitPosition += pG->nBitsInCode;
    i &= pG->codeMask;
    return i;
}


    /*  ------------------------- MixScanLines -------------------------- */
    /*  Output a byte for an interlaced image.  Switch output lines if
        at end of output line.
    */
static void MixScanLines (
    register gifDecodePrivPtr pG, 
    ilByte                    code
    )
{
  
    if (pG->prevYPos != pG->yPos) {
        pG->prevYPos = pG->yPos;
        pG->pInterlace = pG->pDstData + pG->yPos * pG->width;
        }
  
    if (pG->yPos < pG->height)
        *pG->pInterlace++ = code;
    pG->xPos++;

    if (pG->xPos == pG->width) {
        pG->xPos = 0;
        switch (pG->interlaceIndex) {
          case 3:
            pG->yPos += 2;  
            break;
          case 2:
              pG->yPos += 4;
              if (pG->yPos >= pG->height) { 
                  pG->yPos = 1; 
                  pG->interlaceIndex++; 
                  }
               break;
          case 1:
            pG->yPos += 8;
            if (pG->yPos >= pG->height) { 
                pG->yPos = 2; 
                pG->interlaceIndex++; 
                }
            break;
          case 0:
              pG->yPos += 8;
              if (pG->yPos >= pG->height) { 
                  pG->yPos = 4; 
                  pG->interlaceIndex++; 
                  }
              break;
              }
        }
}


    /*  ------------------------- gifDecode -------------------------- */
    /*  Decode (decompress) the GIF stream and write to output.
    */
static ilError gifDecode (
    register gifDecodePrivPtr pG,
    unsigned long           nBytesOfData
    )
{
int                     before[4097];
int                     doEOF, doFree, freeStart;
int                     outputArray[1026];
register ilPtr          pSrc, pDst;
int                     nBitsInCodeInit, maxValue, value, reset, doEnd;
register ilByte         byte;
int                     after[4097];
long                    temp, maxDstPixels, nDstPixels, i;
int                     outputIndex, this, prev, next;


    pG->pInterlace = (ilPtr)NULL;
    pG->interlaceIndex = 0;
    nDstPixels = outputIndex = 0;
    pSrc = pG->pSrcData;
    pG->bitPosition = 0;
    pG->xPos = 0;
    pG->yPos = 0;
    pG->prevYPos = -1;
    maxDstPixels = pG->width * pG->height;

    pG->nBitsInCode = *pSrc++;
    reset = (1 << pG->nBitsInCode);
    doEOF = reset + 1;
    doFree = freeStart = reset + 2;
    pG->nBitsInCode++;
    nBitsInCodeInit = pG->nBitsInCode;
    maxValue = (1 << pG->nBitsInCode);
    pG->codeMask = maxValue - 1;

        /*  The GIF data is a sequence of bytes: a count followed by "count" bytes */
    pDst = pG->pSrcData;
    temp = nBytesOfData;                /* downcount, error if < 0 */
    while (TRUE) {
      temp--;                           /* -1 for count byte */
      if (temp < 0) break;              /* terminate if no more data */
      byte = *pSrc++;
      if (!byte) break;                 /* zero-length = end of data */
      temp -= byte;                     /* subtract # of bytes to copy */
      if (temp < 0)                     /* would "read" off end of data */
          byte += temp;                 /* so subtract excess to read what's left */
      while (byte--) 
          *pDst++ = *pSrc++;
      }

    pDst = pG->pDstData;                       /* point to dst buffer */
  
    value = GetString(pG);
    while (value != doEOF) {
        if (value == reset) {
            doFree = freeStart;
            pG->nBitsInCode = nBitsInCodeInit;
            maxValue = (1 << pG->nBitsInCode);
            pG->codeMask = maxValue - 1;
            value = GetString(pG);
            this = prev = value;
            doEnd = this & pG->mask;
            if (pG->interlaced) 
                MixScanLines (pG, doEnd);
            else *pDst++ = doEnd;
            nDstPixels++;
            }
        else {          /* not a reset */
            if (doFree >= 4096)
                goto terminateDecode;         /* EXIT loop */
            this = next = value;
            if (this >= doFree) {
                this = prev;
                if (outputIndex > 1024)
                    goto terminateDecode;       /* EXIT loop */
                outputArray[outputIndex++] = doEnd;
                }
            while (this > pG->mask) {
              if (outputIndex > 1024) 
                  goto terminateDecode;     /* EXIT loop */
              outputArray[outputIndex++] = after[this];
              this = before[this];
              }
            doEnd = this & pG->mask;
            if (outputIndex > 1024) 
                goto terminateDecode;       /* EXIT loop */
            outputArray[outputIndex++] = doEnd;
            if ((nDstPixels + outputIndex) > maxDstPixels)
                outputIndex = maxDstPixels - nDstPixels;
            nDstPixels += outputIndex;
            i = outputIndex - 1;
            outputIndex = 0;

            if (pG->interlaced) {
                  do {
                      MixScanLines (pG, outputArray[i]);
                      } while (--i >= 0);
              }
            else {
                  do {
                      *pDst++ = outputArray[i];
                      } while (--i >= 0);
              }

        after[doFree] = doEnd;
        before[doFree] = prev;
        doFree++;
        prev = next;
        if (doFree >= maxValue) {
            if (pG->nBitsInCode < 12) {
                maxValue *= 2;
                pG->nBitsInCode++;
                pG->codeMask = (1 << pG->nBitsInCode) - 1;
                }
            }
        }   /* END not a reset */

        if (nDstPixels > maxDstPixels)
            goto terminateDecode;         /* EXIT loop */
        value = GetString(pG);
        }   /* END while */

terminateDecode:  
    return IL_OK;
}

        /*  --------------------- gifReadImageExecute --------------------- */
        /*  Execute() for reading GIF.  Read the whole JIF image out to the 
            dst image and signal last strip.
        */
static ilError gifReadImageExecute (
    ilExecuteData          *pData,
    long                    dstLine,
    long                   *pNLines
    )
{
gifDecodePrivRec            data;
gifRWPrivPtr                pPriv;
ilError                     error;

        /*  Setup globals for decode.  malloc pSrcData: buffer for compressed
            data from file. Point to output buffer, set max # of output bytes.
        */
    pPriv = (gifRWPrivPtr)pData->pPrivate;
    data.pSrcData = (ilPtr)malloc (pPriv->imageLength + 256);
    if (!data.pSrcData)
        return IL_ERROR_MALLOC;

    data.pDstData = (ilPtr)(pData->pDstImage->plane[0].pPixels +
                         dstLine * pData->pDstImage->plane[0].nBytesPerRow);
    data.interlaced = pPriv->interlaced;
    data.width = pPriv->width;
    data.height = pPriv->height;
    data.mask = pPriv->bitMask;

        /*  Read all of the GIF data into memory, and pass to decomp code. */
    if (fseek (pPriv->stream, pPriv->imageOffset, 0)    /* 0 return=success */
     || (fread (data.pSrcData, pPriv->imageLength, 1, pPriv->stream) != 1))
        return IL_ERROR_EFS_IO;

    error = gifDecode (&data, pPriv->imageLength);

        /*  Return "last strip" error, or error from Decode() if one */
    free (data.pSrcData);
    *pNLines = pPriv->height;
    return (error) ? error : IL_ERROR_LAST_STRIP;
}


        /*  ------------------------ gifReadImage ---------------------- */
        /*  Function for ilEFSFileTypeInfo.ReadImage().  Add a producer element
            to read the current page into the given pipe.
        */
static ilBool gifReadImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned int            readMode        /* ignored - no mask images */
    )
{
register gifPrivPtr        pPriv;
register gifRWPrivPtr      pRWPriv;
ilDstElementData            dstData;

        /*  Add a pipe producer to read the entire GIF file into the pipe.
            The des, format and size are in private, from gifOpen().
        */
    pPriv = (gifPrivPtr)file->pPrivate;
    dstData.producerObject = (ilObject)NULL;
    dstData.pDes = IL_DES_PALETTE;
    dstData.pFormat = IL_FORMAT_BYTE;
    dstData.width = pPriv->width;
    dstData.height = pPriv->height;
    dstData.stripHeight = pPriv->height;    /* GIF is one big strip */
    dstData.constantStrip = TRUE;
    dstData.pPalette = pPriv->palette;
    dstData.pCompData = (ilPtr)NULL;

    pRWPriv = (gifRWPrivPtr)ilAddPipeElement (pipe, IL_PRODUCER, 
        sizeof (gifRWPrivRec), 0, (ilSrcElementData *)NULL, &dstData,
        IL_NPF, IL_NPF, IL_NPF, gifReadImageExecute, 0);
    if (!pRWPriv)
        return FALSE;

        /*  Element added; copy relevant data into pipe element private */
    pRWPriv->stream = pPriv->stream;
    pRWPriv->height = pPriv->height;
    pRWPriv->width = pPriv->width;
    pRWPriv->interlaced = pPriv->interlaced;
    pRWPriv->imageOffset = pPriv->imageOffset;
    pRWPriv->imageLength = pPriv->imageLength;
    pRWPriv->bitMask = pPriv->bitMask;
    return TRUE;
}


#ifdef NOTDEF

/* ========================= GIF WRITE CODE =============================== */

        /*  --------------------- gifWriteImageExecute -------------------- */
        /*  Pipe element Execute() for writing GIF files.
        */
static ilError gifWriteImageExecute (
    register ilExecuteData *pData,
    long                    dstLine,    /* not used */
    long                   *pNLines
    )
{
register gifRWPrivPtr       pPriv;
register ilPtr              pByte;
register long               nBytes, nBytesToWrite, i;
register unsigned short    *pPal;

    /*  Buffer for header + palette + image descriptor + code size byte  */
#define WRITE_HEADER_SIZE   (13 + 3 * 256 + 10 + 1)
char                        header [WRITE_HEADER_SIZE];


    pPriv = (gifRWPrivPtr)pData->pPrivate;
    if (*pNLines != pPriv->height)      /* ? not complete strip - error */
        return IL_ERROR_EFS_IO;

        /*  Exit if a write has already been done, as per EFS spec. */
    if (pPriv->writeDone)
        return IL_OK;
    pPriv->writeDone = TRUE;

        /*  Form and write a header */
    pByte = (ilPtr)header;
    *pByte++ = 'G';                     /* write id: "GIF87a" */
    *pByte++ = 'I';
    *pByte++ = 'F';
    *pByte++ = '8';
    *pByte++ = '7';
    *pByte++ = 'a';

    *pByte++ = pPriv->width & 0xff;     /* write image size as screen size */
    *pByte++ = pPriv->width >> 8;
    *pByte++ = pPriv->height & 0xff;
    *pByte++ = pPriv->height >> 8;

        /*  Fill in packed field: global color table, color resolution = 8
            bits/pixel (assumed), sort flag = 0, not sorted (assumed), 
            8 bit (256 entry) global color table follows.
        */
    *pByte++ = 0xf7;            
    *pByte++ = 0;                       /* background color index (assumed) */
    *pByte++ = 0;                       /* some require aspect ratio = 0 */
    
        /*  Fill in global color table: 256 * 8 bit rgb */
    for (pPal = pData->pSrcImage->pPalette, i = 0; i < 256; i++, pPal++) {
        *pByte++ = pPal[0] >> 8;
        *pByte++ = pPal[256] >> 8;
        *pByte++ = pPal[512] >> 8;
        }

        /*  Fill in image descriptor */
    *pByte++ = 0x2c;                    /* image separator */
    *pByte++ = 0;                       /* image left, top position = (0,0) */
    *pByte++ = 0;
    *pByte++ = 0;
    *pByte++ = 0;
    *pByte++ = pPriv->width & 0xff;     /* image size */
    *pByte++ = pPriv->width >> 8;
    *pByte++ = pPriv->height & 0xff;
    *pByte++ = pPriv->height >> 8;
    *pByte++ = 0;                       /* no local color table or interlace */

        /*  Write GIF data: first a byte for the code size, = 8 */
    *pByte++ = 8;

        /*  Write "header" to the file */
    if (fwrite ((char *)header, WRITE_HEADER_SIZE, 1, pPriv->stream) != 1) 
        return IL_ERROR_FILE_IO;

        /*  Pack all the compressed data into "strings" and write them */
    nBytes = pData->compressed.nBytesToRead;
    pByte = pData->pSrcImage->plane[0].pPixels + pData->compressed.srcOffset;
    while (nBytes > 0) {
        nBytesToWrite = (nBytes >= 255) ? 255 : nBytes;
        nBytes -= nBytesToWrite;
        header [0] = nBytesToWrite;
        bcopy ((char *)pByte, (char *)&header[1], nBytesToWrite);
        pByte += nBytesToWrite;
        if (fwrite ((char *)header, nBytesToWrite + 1, 1, pPriv->stream) != 1) 
            return IL_ERROR_FILE_IO;
        }

        /*  Write trailing null byte for end of data */
    header[0] = 0;
    if (fwrite ((char *)header, 1, 1, pPriv->stream) != 1) 
        return IL_ERROR_FILE_IO;

    fflush (pPriv->stream);             /* flush output buffers */
    return IL_OK;
}

        /*  ------------------------ gifWriteImage ---------------------- */
        /*  Function for ilEFSFileTypeInfo.WriteImage().  Add a consumer
            to write the pipe image out to the file.
        */
static ilBool gifWriteImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned long           xRes,
    unsigned long           yRes,
    ilClientImage           maskImage       /* ignored - no mask images */
    )
{
register gifPrivPtr         pPriv;
register gifRWPrivPtr       pRWPriv;
ilPipeInfo                  info;

        /*  Query the pipe and decompress; error if not "forming". */
    if (ilGetPipeInfo (pipe, TRUE, &info, (ilImageDes *)NULL, 
                       (ilImageFormat *)NULL) != IL_PIPE_FORMING) {
        if (!pipe->context->error)
            ilDeclarePipeInvalid (pipe, IL_ERROR_PIPE_STATE);
        return FALSE;
        }

        /*  Convert to a LZW-compressed single-strip palette image. */
    if (!ilConvert (pipe, IL_DES_PALETTE, IL_FORMAT_BYTE, 0, NULL)
     || !ilCompress (pipe, IL_LZW, (ilPtr)NULL, info.height, 0))
            return FALSE;

        /*  Add a consumer element to write the resulting data. */
    pPriv = (gifPrivPtr)file->pPrivate;
    pRWPriv = (gifRWPrivPtr)ilAddPipeElement (pipe, IL_CONSUMER,
        sizeof (gifRWPrivRec), 0, (ilSrcElementData *)NULL, 
        (ilDstElementData *)NULL, IL_NPF, IL_NPF, IL_NPF, 
        gifWriteImageExecute, 0);
    if (!pRWPriv)
        return FALSE;

        /*  Element added; copy relevant data into pipe element private */
    pRWPriv->stream = pPriv->stream;
    pRWPriv->height = info.height;
    pRWPriv->width = info.width;
    pRWPriv->writeDone = FALSE;
    return TRUE;
}

#endif


/* ========================= GIF EFS CODE  =============================== */


        /*  ------------------------ gifOpen ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Open().  Open the given file, return
            the number of pages in it and ptr to private, or null if error.
        */
static ilPtr gifOpen (
    ilEFSFileType           fileType,
    char                   *fileName,
    unsigned int            openMode,
    unsigned long          *pNPages
    )
{
register ilError            error;
register gifPrivPtr         pPriv;
FILE                       *stream;
ilBool                      writeOpen;
int                         i, nColors, opByte, nBytes, red, green, blue;
ilByte                      id[6], header[7], desc[10];;
#define BYTE_FROM_FILE(_stream, _byte) (((_byte) = getc (_stream)) != EOF)



    pPriv = (gifPrivPtr)NULL;           /* in case of error */
    writeOpen = (openMode == IL_EFS_WRITE);

        /*  Open the file, write mode if requested.  Error if open fails. */
    stream = fopen (fileName, (writeOpen) ? "w" : "r");
    if (!stream) {
        fileType->context->error = IL_ERROR_EFS_OPEN;
        return (ilPtr)NULL;
        }

        /*  If reading, return "not mine" error if not a GIF file. */
    if (!writeOpen) {
        if ((fread (id, 6, 1, stream) != 1)
         || (id[0] != 'G') || (id[1] != 'I') || (id[2] != 'F')
         || (id[3] != '8') || (id[5] != 'a')) {
            error = IL_ERROR_EFS_NOT_MINE;
            goto openError;
            }
        if (id[4] != '7') {             /* gif89a not supported at this time */
            error = IL_ERROR_EFS_UNSUPPORTED;
            goto openError;
            }
        }

        /*  malloc private, init it; done if opening in write mode */
    pPriv = (gifPrivPtr)calloc (1, sizeof (gifPrivRec));    /* alloc w/ 0s */
    if (!pPriv) {
        error = IL_ERROR_MALLOC;
        goto openError;
        }
    pPriv->stream = stream;
    if (writeOpen) {
        fileType->context->error = IL_OK;
        return (ilPtr)pPriv;            /* writing, done: EXIT */
        }

        /*  Doing a read: read header info and palette, store into *pPriv.
            Next 7 bytes include colormap size; read if present, else default.
            5th byte  = # of colormap colors; bit 7 = 1 if a colormap present,
            in which case read and scale 0..255 (GIF) to 0..65535 (IL).
        */
    error = IL_ERROR_EFS_IO;            /* dflt to I/O error for now */
    if (fread (header, 7, 1, stream) != 1)
        goto openError;
    nColors = 1 << ((header[4] & 7) + 1);
    pPriv->bitMask = nColors - 1;

    if (header[4] & 0x80) {             /* bit 7 => colormap present */
        for (i = 0; i < nColors; i++) {
            if (!BYTE_FROM_FILE (stream, red)
             || !BYTE_FROM_FILE (stream, green)
             || !BYTE_FROM_FILE (stream, blue))
                goto openError;
            pPriv->palette[0  +i] = (red << 8) | red;
            pPriv->palette[256+i] = (green << 8) | green;
            pPriv->palette[512+i] = (blue << 8) | blue;
            }
        }
    else {
        static ilByte dfltPalette[16][3] = {
          {0,0,0}, {0,0,128}, {0,128,0}, {0,128,128},
          {128,0,0}, {128,0,128}, {128,128,0}, {200,200,200},
          {100,100,100}, {100,100,255}, {100,255,100}, {100,255,255},
          {255,100,100}, {255,100,255}, {255,255,100}, {255,255,255} };
        for (i = 0; i < 256; i++) {
            pPriv->palette[0  +i] = dfltPalette[i&0xf][0];
            pPriv->palette[256+i] = dfltPalette[i&0xf][1];
            pPriv->palette[512+i] = dfltPalette[i&0xf][2];
            }
        }

        /*  Get and parse Image Descriptor: 10 bytes long */
    if (fread (desc, 10, 1, stream) != 1)
        goto openError;
    if (desc[0] != 0x2c) {
        error = IL_ERROR_EFS_FORMAT_ERROR;
        goto openError;
        }
    pPriv->width = desc[5] + (desc[6] << 8);
    pPriv->height = desc[7] + (desc[8] << 8);
    pPriv->interlaced = (desc[9] & 0x40) ? TRUE : FALSE;

        /*  If present, read local colormap in over global colormap */
    if (desc[9] & 0x80) {
        nColors = 1 << ((desc[9] & 7) + 1);
        for (i = 0; i < nColors; i++) {
            if (!BYTE_FROM_FILE (stream, red)
             || !BYTE_FROM_FILE (stream, green)
             || !BYTE_FROM_FILE (stream, blue))
                goto openError;
            pPriv->palette[0  +i] = (red << 8) | red;
            pPriv->palette[256+i] = (green << 8) | green;
            pPriv->palette[512+i] = (blue << 8) | blue;
            }
        }

        /*  Save current position and # bytes left in file for read filter */
    pPriv->imageOffset = ftell (stream);
    if (fseek (stream, 0, 2))   /* seek to eof; nonzero return is error */
        goto openError;
    pPriv->imageLength = ftell (stream) - pPriv->imageOffset;

    fileType->context->error = IL_OK;
    *pNPages = 1;                   /* GIF handled as single-image format */
    return (ilPtr)pPriv;            /* success; EXIT */

    /*  goto point on error after file open: post "error", close file. */
openError:
    fileType->context->error = error;
    if (pPriv) 
        free (pPriv);
    fclose (stream);
    return (ilPtr)NULL;
}


        /*  ------------------------ gifClose ------------------------- */
        /*  Function for ilEFSFileTypeInfo.Close().  Close the given file.
        */
static ilBool gifClose (
    ilEFSFile               file
    )
{
    fclose (((gifPrivPtr)file->pPrivate)->stream);
    return TRUE;
}


        /*  ------------------------ gifGetPageInfo ---------------------- */
        /*  Function for ilEFSFileTypeInfo.GetPageInfo().  Return information
            on the current page.  Called only if file opened in read mode.
        */
static ilBool gifGetPageInfo (
    ilEFSFile               file,
    register ilEFSPageInfo *pInfo
    )
{
register gifPrivPtr        pPriv;

        /*  Return info on the current (only) image in the file. */
    pPriv = (gifPrivPtr)file->pPrivate;
    pInfo->attributes = 0;
    pInfo->page = 0;
    pInfo->width = pPriv->width;
    pInfo->height = pPriv->height;
    pInfo->xRes = pInfo->yRes = 0;
    pInfo->des = *IL_DES_PALETTE;
    pInfo->format = *IL_FORMAT_BYTE;

    return TRUE;
}



        /*  ------------------------ _ilefsInitGIF ------------------------- */
        /*  Add GIF EFS file type handler to the given context; 
            return false if error.
        */
ilBool _ilefsInitGIF (
    ilContext               context
    )
{
ilEFSFileTypeInfo           info;

    bzero (&info, sizeof (ilEFSFileTypeInfo));

    strcpy (info.name, "GIF");
    strcpy (info.displayName, "GIF");
    info.checkOrder = 85;                   /* somewhat arbitrary */

    info.nExtensions = 2;
    strcpy (info.extensions[0], "gif");
    strcpy (info.extensions[1], "GIF");

    info.openModes = (1<<IL_EFS_READ) | (1<<IL_EFS_READ_SEQUENTIAL);
    info.attributes = 0;

    info.Open = gifOpen;
    info.GetPageInfo = gifGetPageInfo;
    info.ReadImage = gifReadImage;
    info.Close = gifClose;

    return (ilEFSAddFileType (context, &info, NULL) != NULL);
}



