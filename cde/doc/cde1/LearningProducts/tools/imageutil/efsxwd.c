/**
***    FILE:   EFSXWD.c
***    Image Library (IL) EFS support for XWD
***    Copyright (C) 1992 Hewlett-Packard Co.
***-----------------------------------------------------------------------
***                             RESTRICTED RIGHTS LEGEND
***    Use, duplication, or disclosure by the U.S. Government is subject to
***    restrictions as set forth in sub-paragraph (c)(1)(ii) of the Rights in
***    Technical Data and Computer Software clause in DFARS 252.227-7013.
***                             Hewlett-Packard Company
***                             3000 Hanover Street
***                             Palo Alto, CA 94304 U.S.A.
***    Rights for non-DOD U.S. Government Departments and Agencies are as set
***    forth in FAR 52.227-19(c)(1,2).
***-----------------------------------------------------------------------
***    Description:    
***
***        Contains the EFS functions to support reading XWD files into
***        and (IL) Image Pipeline.  
***
***-----------------------------------------------------------------------
***    History:  
***          Date                      Programmer        Description 
***          03/04/92                  larsson           Created.
***
***---------------------------------------------------------------------- */

    /*  efsxwd.c  */

#include "il.h"
#include "ilefs.h"
#include "ilpipelem.h"
#include "ilerrors.h"
#include <stdio.h>
#include "ilX.h"
#include <X11/XWDFile.h>


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif



/*  Private data for read/write pipe element functions */
typedef struct {
    FILE                   *stream;                   /* stdio stream file */
    char                   *fname;                    /* filename from open call */
    long int               data_offset;               /* offset from beg. of file to data */
    XWDFileHeader          header;                    /* XWD header structure */
    long                   bytes_per_aligned_row;     /* no. of bytes to make up 32 bit aligned row */
    int                    linecount;                 /* current linecount value  */
    int                    stripheight;               /* no. of lines per strip */ 
    ilBool                 writeDone;                 /* a write has been done */
    unsigned short         *pPalette;                 /* ptr to color table values */
    } xwdRWPrivRec, *xwdRWPrivPtr;


/*  Private data for each XWD EFS file */
typedef struct {
    ilEFSPageInfo          info;            /* contains width & height */
    xwdRWPrivRec           xwddata;         /* xwd file attributes */
    } xwdPrivRec, *xwdPrivPtr;




/*
 * The following utilities are borrowed from the MIT xwd client.  They are
 * used to account for machine byte-order differences.  xwd files are assumed
 * to be in big endian format.
 */
 
_swapshort (bp, n)
    register char *bp;
    register unsigned n;
{
    register char c;
    register char *ep = bp + n;

    while (bp < ep) {
	c = *bp;
	*bp = *(bp + 1);
	bp++;
	*bp++ = c;
    }
}

_swaplong (bp, n)
    register char *bp;
    register unsigned n;
{
    register char c;
    register char *ep = bp + n;
    register char *sp;

    while (bp < ep) {
	sp = bp + 3;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	sp = bp + 1;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	bp += 2;
    }
}



/* ------------------------  xwdreadinit  ------------------ */
/*  Sets up the given xwd file for decoding by positioning the
    file at the point after the header data.  Also inits the hex 
    conversion table from XReadBitmapFile.
*/

static ilError xwdreadinit(
      xwdRWPrivPtr   pPrivate,
      ilImageInfo    *pSrcImage,
      ilImageInfo    *pDstImage
      )
{


    /* reset to beginning of file */
    if (fseek (pPrivate->stream, 0, 0)) /* seek to start; != 0 return => error */
        return IL_ERROR_FILE_IO;
    
    /* Now skip over header */
    if (fseek (pPrivate->stream, pPrivate->data_offset, 0))
        return IL_ERROR_FILE_IO;

    pPrivate->linecount = 0;
    return IL_OK;
}


/*  ------------------------ xwdDestroy -------------------------------- */
/*  Destroy() function for XWD file support. Frees the palette space if present.
*/
static ilError xwdDestroy (
    xwdRWPrivPtr       pPriv
    )
{
    if (pPriv->pPalette)
        free (pPriv->pPalette);

    return IL_OK;
}



/* ------------------------  xwdreadheader  ------------------ */
/*  Read the header lines of the xwd file and return the error 
    IL_ERROR_EFS_NOT_MINE if the file can not be parsed.  
    The width & height are returned as parameters.
*/

static ilError xwdreadheader (
      FILE                  *stream,
      XWDFileHeader         *header,
      long int              *data_offset
      )
{

 int   win_name_size, hsize;
 char  *win_name;
 unsigned long swaptest = 1;
            

     if ( fread( (char *) header, sizeof(*header), 1, stream) != 1)
         return IL_ERROR_EFS_NOT_MINE;

     /*
      * The following check and fix for machines using little endian byte
      * ordering is borrowed from the MIT xwd client.
      */
     if (*(char *) &swaptest) {
         _swaplong((char *) header, sizeof(*header));
     }

     if (header->file_version != XWD_FILE_VERSION) 
         return IL_ERROR_EFS_NOT_MINE;

     hsize = sizeof(*header);
 
     if (hsize > header->header_size)
         return IL_ERROR_EFS_NOT_MINE;

     /* After header, get the size of the window name that follows */
     win_name_size = (header->header_size - sizeof(*header));
     if((win_name =  (char *) malloc((unsigned) win_name_size)) == NULL)
         return IL_ERROR_MALLOC;
          
     /* read in window name */
     if(fread(win_name, sizeof(char), win_name_size, stream) != win_name_size)
         return IL_ERROR_EFS_NOT_MINE;

     *data_offset = ftell(stream);

     if(! *data_offset) 
         return IL_ERROR_EFS_EOF;

     free(win_name);
     return ( 0 );
} 
  



/*  ------------------------ xwdOpen ------------------------- */
/*  Function for ilEFSFileTypeInfo.Open().  Open the given file, return
    the number of pages in it and ptr to private, or null if error.
*/

static ilPtr xwdOpen (
    ilEFSFileType           fileType,
    char                   *fileName,
    unsigned int            openMode,
    unsigned long          *pNPages
    )
{
register xwdPrivPtr        pPriv;
FILE                       *stream;
ilEFSPageInfo              info;
ilError                    error;
ilBool                     doWrite, i;
long int                   bytes_per_width, size;
ilImageDes                 Ides;
ilImageFormat              Iformat;
XWDFileHeader              header;
long int                   data_offset;



    *pNPages = 1;
    doWrite = (openMode == IL_EFS_WRITE);

    /*  Open the file, write mode if requested.  Error if open fails. */
    stream = fopen (fileName, (doWrite) ? "w" : "r");
    if (!stream) {
        fileType->context->error = IL_ERROR_EFS_OPEN;
        return (ilPtr)NULL;
    }

    if (!doWrite) {

       /*  Opening in read mode: read header; return "not mine" error
           if not a XWD file, or "empty file" if no images.
       */

       error = xwdreadheader(stream, &header, &data_offset);
                          
       if(error) {
           fileType->context->error = error;
           fclose (stream);
           return (ilPtr)NULL;
       }
    }


    pPriv = (xwdPrivPtr)malloc (sizeof (xwdPrivRec));
    if (!pPriv) {
        fileType->context->error = IL_ERROR_MALLOC;
        fclose (stream);
        return (ilPtr)NULL;
    }


    pPriv->xwddata.stream = stream;
    pPriv->xwddata.fname =  fileName;
    pPriv->xwddata.data_offset = data_offset;
    pPriv->xwddata.header = header;
    pPriv->xwddata.writeDone = FALSE;
    pPriv->xwddata.bytes_per_aligned_row = ((header.pixmap_width + 31)/32) << 2;
    pPriv->xwddata.pPalette = (unsigned short *) NULL;    

    info.width = header.pixmap_width;
    info.height = header.pixmap_height;
    info.page = 0;                  /* single page file; always page 0 */
    info.attributes = 0;
    info.xRes = 0;
    info.yRes = 0;


    /*  set up image descriptor structure */
    IL_INIT_IMAGE_DES(&Ides);

    /* Z pixmap format is supported with pixmap depths = 1, 8, 24  */
    if(header.pixmap_format != 2) {
         fileType->context->error = IL_ERROR_EFS_UNSUPPORTED;
         fclose (stream);
         return (ilPtr)NULL;
    }

    if ((header.pixmap_depth == 1) && (header.bits_per_pixel == 1))
          Ides.type   = IL_BITONAL;
    else if ((header.pixmap_depth > 8) && (header.bits_per_pixel > 8)) {
          Ides.type   = IL_RGB;

          if ( (header.red_mask != 0xFF0000)||(header.green_mask != 0xFF00)||
               (header.blue_mask != 0xFF) ) {
               fileType->context->error = IL_ERROR_EFS_UNSUPPORTED;
               fclose (stream);
               return (ilPtr)NULL;
          }
    }
    else if(((header.pixmap_depth == 8) && (header.bits_per_pixel == 8)) ||
            ((header.pixmap_depth == 4) && (header.bits_per_pixel == 4)))
          Ides.type   = IL_PALETTE;
    else {
         fileType->context->error = IL_ERROR_EFS_UNSUPPORTED;
         fclose (stream);
         return (ilPtr)NULL;
    }

    Ides.compression       =   IL_UNCOMPRESSED;
    Ides.blackIsZero       =   FALSE;  
    Ides.nSamplesPerPixel  =   header.pixmap_depth/header.bits_per_rgb;
    for (i = 0; i < Ides.nSamplesPerPixel; i++)
          Ides.nLevelsPerSample [i]  =  header.ncolors;


    /*  set up image format structure */
    IL_INIT_IMAGE_FORMAT(&Iformat)    
    Iformat.sampleOrder  =  IL_SAMPLE_PIXELS;
    Iformat.byteOrder    =  1 - header.byte_order;
    if (Ides.type == IL_BITONAL) 
       Iformat.rowBitAlign  =  32;
    else
       Iformat.rowBitAlign  =  8;

    for (i = 0; i < Ides.nSamplesPerPixel; i++)
          Iformat.nBitsPerSample [i]  = header.bits_per_rgb;

    if(header.ncolors == 0) {
         fileType->context->error = IL_ERROR_EFS_UNSUPPORTED;
         fclose (stream);
         return (ilPtr)NULL;
    }

    info.des = Ides;
    info.format = Iformat;
    pPriv->info = info;
   
    fileType->context->error = IL_OK;
    return (ilPtr)pPriv;

}



/*  ------------------------ xwdClose ------------------------- */
/*  Function for ilEFSFileTypeInfo.Close().  Close the given file.
*/

static ilBool xwdClose (
    ilEFSFile               file
    )
{
register xwdPrivPtr        pPriv;

    pPriv = (xwdPrivPtr)file->pPrivate;
    fclose (pPriv->xwddata.stream);
    return TRUE;
}



/*  ------------------------ xwdGetPageInfo ---------------------- */
/*  Function for ilEFSFileTypeInfo.GetPageInfo().  Return information
    on the current page.  Called only if file opened in read mode.
*/

static ilBool xwdGetPageInfo (
    ilEFSFile               file,
    register ilEFSPageInfo *pInfo
    )
{
xwdPrivPtr                 pPriv;

    /*  Return info on the current (only) image in the file. */

    *pInfo = ((xwdPrivPtr)file->pPrivate)->info;
    return TRUE;
}




/*  --------------------- xwdbitReadImageExecute --------------------- */
/*  Execute() function for reading XWD bitonal images.  Read the XWD image out to the 
    dst image in strips and signal last strip.
*/

static ilError xwdbitReadImageExecute (
    ilExecuteData          *pData,
    long                   dstLine,
    long                   *pNLines
    )
{
register xwdRWPrivPtr      pPriv;
ilPtr                      pDst;
ilImagePlaneInfo           *pPlane;
register int               y, bperline, brow;
register int               bytes;
int                        lastcount;
int                        nlines;
register unsigned char     *ptr;
int                        readvalue;


    pPriv = (xwdRWPrivPtr)pData->pPrivate;
    if ((pPriv->header.pixmap_height - pPriv->linecount) <= 0) 
        return IL_OK;

    /* compute no. of lines in strip */
    nlines = ( (pPriv->linecount + pPriv->stripheight) <= pPriv->header.pixmap_height )  
                ?  pPriv->stripheight  :  pPriv->header.pixmap_height - pPriv->linecount;

    pPlane = pData->pDstImage->plane;

    pDst = pPlane->pPixels + dstLine * pPlane->nBytesPerRow;

    bperline = (pPlane->nBytesPerRow < pPriv->header.bytes_per_line) ? pPlane->nBytesPerRow : pPriv->header.bytes_per_line;
    brow = (pPlane->nBytesPerRow < pPriv->bytes_per_aligned_row) ? pPlane->nBytesPerRow : pPriv->bytes_per_aligned_row;

    lastcount = pPriv->linecount;
    /* Begin decode */


        for (y = lastcount,  ptr = pDst;   y < (lastcount + nlines);  y++, pPriv->linecount++)  {

             readvalue =  fread( ptr,  sizeof(char),  bperline,  pPriv->stream );
             if(readvalue != pPriv->header.bytes_per_line) 
                    return (IL_ERROR_EFS_EOF);
             ptr +=  pPriv->header.bytes_per_line;

             /* Add padding if necessary */
             for (bytes = pPriv->header.bytes_per_line;  bytes  <  brow;  bytes++)
                  *ptr++ = 0;
        }



    *pNLines = pPriv->linecount - lastcount;

    if ( pPriv->linecount >= pPriv->header.pixmap_height ) 
       return  IL_ERROR_LAST_STRIP;   
    else
       return IL_OK;     

}







/*  --------------------- xwdReadImageExecute --------------------- */
/*  Execute() for reading palette XWD images .  Read the XWD image out to the 
    dst image in strips and signal last strip.
*/

static ilError xwdReadImageExecute (
    ilExecuteData          *pData,
    long                   dstLine,
    long                   *pNLines
    )
{
register xwdRWPrivPtr      pPriv;
ilPtr                      pDst;
ilImagePlaneInfo           *pPlane;
register int               bytes;
register int               y;
int                        lastcount;
int                        nlines;
register unsigned char     *ptr, *Pskip;
int                        readvalue;
unsigned char              skip = 0;


    pPriv = (xwdRWPrivPtr)pData->pPrivate;
    if ((pPriv->header.pixmap_height - pPriv->linecount) <= 0) 
        return IL_OK;

    /* compute no. of lines in strip */
    nlines = ( (pPriv->linecount + pPriv->stripheight ) <= pPriv->header.pixmap_height )  
                ?  pPriv->stripheight   :  pPriv->header.pixmap_height - pPriv->linecount;

    pPlane = pData->pDstImage->plane;

    pDst = pPlane->pPixels + dstLine * pPlane->nBytesPerRow;

    Pskip = &skip;

    lastcount = pPriv->linecount;
    /* Begin decode */


        for (y = lastcount,  ptr = pDst;   y < (lastcount + nlines);  y++, pPriv->linecount++)  {

             readvalue =  fread( ptr,  sizeof(char),  pPriv->header.pixmap_width,  pPriv->stream );
             if(readvalue != pPriv->header.pixmap_width) 
                    return (IL_ERROR_EFS_EOF);
             ptr += pPlane->nBytesPerRow;

             /* skip padding if necessary */
             for (bytes = pPriv->header.pixmap_width;  bytes  <  pPriv->header.bytes_per_line;  bytes++)
                readvalue = fread(Pskip,  sizeof(char),  1,  pPriv->stream);
        }



    *pNLines = pPriv->linecount - lastcount;

    if ( pPriv->linecount >= pPriv->header.pixmap_height ) 
       return  IL_ERROR_LAST_STRIP;   
    else
       return IL_OK;     

}


/*  --------------------- xwdReadImage24Execute --------------------- */
/*  Execute() for reading 24 bit XWD images .  Read the XWD image out to the 
    dst image in strips and signal last strip.
*/

static ilError xwdReadImage24Execute (
    ilExecuteData          *pData,
    long                   dstLine,
    long                   *pNLines
    )
{
register xwdRWPrivPtr      pPriv;
ilPtr                      pDst;
ilImagePlaneInfo           *pPlane;
register int               bytes;
register int               y, x;
int                        lastcount, imagewidth;
int                        nlines;
register unsigned char     *ptr, *Pskip;
int                        readvalue;
unsigned char              skip = 0;


    pPriv = (xwdRWPrivPtr)pData->pPrivate;
    if ((pPriv->header.pixmap_height - pPriv->linecount) <= 0) 
        return IL_OK;

    /* compute no. of lines in strip */
    nlines = ( (pPriv->linecount + pPriv->stripheight ) <= pPriv->header.pixmap_height )  
                ?  pPriv->stripheight   :  pPriv->header.pixmap_height - pPriv->linecount;

    pPlane = pData->pDstImage->plane;

    pDst = pPlane->pPixels + dstLine * pPlane->nBytesPerRow;
    imagewidth = pPriv->header.pixmap_width * 3;

    Pskip = &skip;
    
    lastcount = pPriv->linecount;
    /* Begin decode */


        for (y = lastcount,  ptr = pDst;   y < (lastcount + nlines);  y++, pPriv->linecount++)  {

               for (x = 0;  x < imagewidth;  x+=3) {
                     /* handle 32 bit format with leading pad byte  */
                     readvalue =  fread(Pskip, sizeof(char),  1,  pPriv->stream);   /* pad byte */
                     readvalue =  fread( ptr,  sizeof(char),  3,  pPriv->stream);   /* r,g,b,   */
                     if(readvalue != 3) 
                            return (IL_ERROR_EFS_EOF);
                     ptr += 3;
               } 
               pDst += pPlane->nBytesPerRow;
               ptr = pDst;
        }


    *pNLines = pPriv->linecount - lastcount;

    if ( pPriv->linecount >= pPriv->header.pixmap_height ) 
       return  IL_ERROR_LAST_STRIP;   
    else
       return IL_OK;     

}





/*  ------------------------ xwdReadImage ---------------------- */
/*  Function for ilEFSFileTypeInfo.ReadImage().  Add a producer element
    to read the current page into the given pipe.
*/

static ilBool xwdReadImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned int            readMode        /* ignored - no mask images */
    )
{
register xwdPrivPtr        pPriv;
register xwdRWPrivPtr      pRWPriv;
ilDstElementData           dstData;
unsigned short             *pPalette;
int                        ncolors, i;
XColor                     *colors, color;
unsigned long              swaptest = 1;


        /*  Add a pipe producer to read the entire XBM file into the pipe.
            The des, format and size are in private, from xwdOpen().
        */
    pPriv = (xwdPrivPtr)file->pPrivate;

    /* reset to beginning of file */
    if (fseek (pPriv->xwddata.stream, 0, 0)) /* seek to start; != 0 return => error */
        return IL_ERROR_FILE_IO;
    
    /* Now skip over header */
    if (fseek( pPriv->xwddata.stream, pPriv->xwddata.data_offset, 0))
        return IL_ERROR_FILE_IO;

    /* Read in the color map data */
    ncolors = pPriv->xwddata.header.ncolors;
    colors = (XColor *) malloc ( (unsigned ) ncolors * sizeof(XColor));
    if ( !colors )
           return IL_ERROR_MALLOC;
    if ( fread( (char *) colors, sizeof(XColor),  ncolors, pPriv->xwddata.stream )  !=  ncolors )
           return IL_ERROR_FILE_IO;

    if (*(char *) &swaptest) {
	for (i = 0; i < ncolors; i++) {
	    _swaplong((char *) &colors[i].pixel, sizeof(long));
	    _swapshort((char *) &colors[i].red, 3 * sizeof(short));
	}
    }

    if ( pPriv->info.des.type == IL_PALETTE ) {
       pPalette = (unsigned short *) malloc( sizeof(unsigned short) * 3 * 256);
       if ( !pPalette)
           return IL_ERROR_MALLOC;
    
       /* Now setup palette color map */
       for ( i = 0;  i < ncolors;  i++ ) {
              pPalette[i]      =  colors[i].red;
              pPalette[i+256]  =  colors[i].green;
              pPalette[i+512]  =  colors[i].blue;
       }

    }
    else pPalette = (unsigned short *) NULL;


    pPriv->xwddata.data_offset = ftell (pPriv->xwddata.stream);
    
    dstData.producerObject = (ilObject)NULL;
    dstData.pDes = &pPriv->info.des;
    dstData.pFormat = &pPriv->info.format;
    dstData.width = pPriv->info.width;
    dstData.height = pPriv->info.height;

    /* Get recommended strip size */
    dstData.stripHeight = ilRecommendedStripHeight( &pPriv->info.des, &pPriv->info.format,
                                                    pPriv->info.width, pPriv->info.height );

    dstData.constantStrip = TRUE;
    dstData.pPalette =  pPalette;
    pPriv->xwddata.pPalette = pPalette;
    dstData.pCompData = (ilPtr)NULL;


    if (pPriv->info.des.type == IL_BITONAL) 
        pRWPriv = (xwdRWPrivPtr) ilAddPipeElement ( pipe, IL_PRODUCER, sizeof(xwdRWPrivRec), 0, (ilSrcElementData *)NULL, 
                                                    &dstData, xwdreadinit, IL_NPF, xwdDestroy, xwdbitReadImageExecute, 0);

    else if (pPriv->info.des.type == IL_RGB) 
        pRWPriv = (xwdRWPrivPtr) ilAddPipeElement ( pipe, IL_PRODUCER, sizeof(xwdRWPrivRec), 0, (ilSrcElementData *)NULL, 
                                                    &dstData, xwdreadinit, IL_NPF, xwdDestroy, xwdReadImage24Execute, 0);
        
    else 
        pRWPriv = (xwdRWPrivPtr) ilAddPipeElement ( pipe, IL_PRODUCER, sizeof(xwdRWPrivRec), 0, (ilSrcElementData *)NULL, 
                                                    &dstData, xwdreadinit, IL_NPF, xwdDestroy, xwdReadImageExecute, 0);
        
    if (!pRWPriv)
         return FALSE;

 
       /*  Element added; copy relevant data into pipe element private */

    *pRWPriv = pPriv->xwddata;
    pRWPriv->stripheight = dstData.stripHeight;

    return TRUE;
}


#ifdef NOTDEF


/*  --------------------- xwdWriteImageExecute --------------------- */
/*  Execute() for writing XBM files. 
*/

static ilError xwdWriteImageExecute (
    ilExecuteData          *pData,
    long                   dstLine,
    long                   *pNLines
    )
{

register xwdRWPrivPtr      pPriv;
ilPtr                      pBytes;
unsigned char              *ptr;


}


/*  ------------------------ xwdWriteImage ---------------------- */
/*  Function for ilEFSFileTypeInfo.WriteImage().  Add a consumer
    to write the pipe image to the given file.
*/

static ilBool xwdWriteImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned long           xRes,
    unsigned long           yRes,
    ilClientImage           maskImage
    )
{
register xwdPrivPtr        pPriv;
register xwdRWPrivPtr      pRWPriv;
ilPipeInfo                 info;
ilImageDes                 des;
ilImageFormat              format;
ilSrcElementData           srcdata;

}

#endif



/*  ------------------------ _ilefsInitXWD ------------------------- */
/*  Add XWD EFS file type handler to the given context; 
    return false if error.
*/

ilBool _ilefsInitXWD (
    ilContext               context
    )
{
ilEFSFileTypeInfo           info;

    bzero (&info, sizeof (ilEFSFileTypeInfo));

    strcpy (info.name, "XWD");
    strcpy (info.displayName, "XWD");
    info.checkOrder = 50;

    info.nExtensions = 3;
    strcpy (info.extensions[0], "xwd");
    strcpy (info.extensions[1], "XWD");
    strcpy (info.extensions[2], "Xwd");

    info.openModes = (1<<IL_EFS_READ) | (1<<IL_EFS_READ_SEQUENTIAL);
    info.attributes = 0;

    info.Open = xwdOpen;
    info.GetPageInfo = xwdGetPageInfo;
    info.ReadImage = xwdReadImage;
    info.Close = xwdClose;

    return (ilEFSAddFileType (context, &info, NULL) != NULL);
}
