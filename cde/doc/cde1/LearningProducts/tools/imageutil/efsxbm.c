/**
***    FILE:   EFSXBM.c
***    Image Library (IL) EFS support for XBM
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
***        Contains the EFS functions to support reading XBM files into
***        and (IL) Image Pipeline.  Add Includes support for generating
***        XBM files.
***
***-----------------------------------------------------------------------
***    History:  
***          Date                      Programmer        Description 
***          02/19/92                  larsson           Created.
***          02/21/92                  larsson           Add Write support.
***
***---------------------------------------------------------------------- */

    /*  efsxbm.c  */

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


#define MAX_SIZE  255
static ilBool initialized = FALSE;
static short  hexTable[256];   /* Hex conversion value */

/*  Private data for read/write pipe element functions */
typedef struct {
    FILE                   *stream;                   /* stdio stream file */
    char                   *fname;                    /* filename from open call */
    long                   width;                     /* width  */
    long                   height;                    /* height */
    unsigned long          size;                      /* total size of image in bytes */
    long                   bytes_per_width;           /* no. of bytes per width of line */
    long                   bytes_per_aligned_row;     /* no. of bytes to make up 32 bit aligned row */
    int                    linecount;                 /* current linecount value  */
    int                    stripheight;               /* no. of lines per strip */ 
    long                   x_hot;                     /* x hotspot default -1 */
    long                   y_hot;                     /* y hotspot default -1 */
    ilBool                 version10p;                /* short (version 10p) format specifier - not supported */
    ilBool                 padding;                   /* pad with used with short format only */     
    ilBool                 writeDone;                 /* a write has been done */
    } xbmRWPrivRec, *xbmRWPrivPtr;


/*  Private data for each XBM EFS file */
typedef struct {
    ilEFSPageInfo          info;            /* contains width & height */
    xbmRWPrivRec           xbmdata;         /* xbm file attributes */
    } xbmPrivRec, *xbmPrivPtr;






/* ------------------------  xbmreadinit  ------------------ */
/*  Sets up the given xbm file for decoding by positioning the
    file at the point after the header data.  Also inits the hex 
    conversion table from XReadBitmapFile.
*/

static ilError xbmreadinit(
      xbmRWPrivPtr   pPrivate,
      ilImageInfo    *pSrcImage,
      ilImageInfo    *pDstImage
      )
{

  char    line[MAX_SIZE];              /* input line from file */
  char    name_and_type[MAX_SIZE];     /* name string for width and height */
  ilBool  header_read;                 /* header flag */


    /* reset to beginning of file */
    if (fseek (pPrivate->stream, 0, 0)) /* seek to start; != 0 return => error */
        return IL_ERROR_FILE_IO;
    
    /* skip over header */
    header_read = FALSE;
    while(!header_read)  {
       if ( !fgets(line, MAX_SIZE,  pPrivate->stream) ) 
           return IL_ERROR_EFS_EOF;

       if( ( sscanf(line, "static char %s = {",  name_and_type ) == 1)           ||
           ( sscanf(line, "static unsigned char %s = {",  name_and_type ) == 1)  ||
           ( sscanf(line, "static short %s = {",  name_and_type ) == 1)  )
            header_read = TRUE;
    }


    if (!initialized) {    

       /* We build the table at run time for several reasons:
        *     1.  portable to non-ASCII machines.
        *     2.  still reentrant since we set the init flag after setting table.
        *     3.  easier to extend.
        *     4.  less prone to bugs.
        */
       bzero( hexTable, 512 );
       hexTable['0'] = 0;	hexTable['1'] = 1;
       hexTable['2'] = 2;	hexTable['3'] = 3;
       hexTable['4'] = 4;	hexTable['5'] = 5;
       hexTable['6'] = 6;	hexTable['7'] = 7;
       hexTable['8'] = 8;	hexTable['9'] = 9;
       hexTable['A'] = 10;	hexTable['B'] = 11;
       hexTable['C'] = 12;	hexTable['D'] = 13;
       hexTable['E'] = 14;	hexTable['F'] = 15;
       hexTable['a'] = 10;	hexTable['b'] = 11;
       hexTable['c'] = 12;	hexTable['d'] = 13;
       hexTable['e'] = 14;	hexTable['f'] = 15;

       /* delimiters of significance are flagged w/ negative value */
       hexTable[' '] = -1;	hexTable[','] = -1;
       hexTable['}'] = -1;	hexTable['\n'] = -1;
       hexTable['\t'] = -1;
       initialized = TRUE;
    }

    pPrivate->linecount = 0;
    return IL_OK;
}



/* ------------------------  xbmreadheader  ------------------ */
/*  Read the header lines of the xbm file and return the error 
    IL_ERROR_EFS_NOT_MINE if the file can not be parsed.  
    The width & height are returned as parameters.
*/

static ilError xbmreadheader (
      FILE                  *stream,
      long                  *width,
      long                  *height,
      long                  *bytes_per_width,
      long                  *size,
      long                  *x_hot,
      long                  *y_hot,
      ilBool                *version10p,
      ilBool                *padding
      )
{
#define MAX_SIZE  255


  char    line[MAX_SIZE];              /* input line from file */
  char    name_and_type[MAX_SIZE];     /* name string for width and height */
  char    *type;                       /* field holder for parsing  */
  int     value;                       /* value specified in file */
  ilBool  header_read;                 /* header flag */
  ilBool  in_comment;                  /* processing comment over multiple lines */
  ilBool  removing_comments;           /* processing a single line to remove
                                            multiple comments */
  char    comment_start[] = { '/', '*', '\0' };
  char    comment_end[]   = { '*', '/', '\0' };
  char    cfline[MAX_SIZE];             /* input line stripped of comments */
  char	  *cs, *ce;
  int		  i;

  /* initialization */
  header_read = FALSE;
  in_comment  = FALSE;
  removing_comments = FALSE;
  *x_hot  = -1;
  *y_hot  = -1;
  *width  =  0;
  *height =  0;
 
  while(!header_read)  {
             
     if ( !removing_comments ) {
        if ( !fgets(line, MAX_SIZE, stream) ) 
           return IL_ERROR_EFS_NOT_MINE;
     }
     else
        removing_comments = FALSE;

     if (strlen(line) == MAX_SIZE-1) 
        return IL_ERROR_EFS_NOT_MINE;

     if ( in_comment ) {  /* if comment ends on this line, process it */
        if (strstr (line, comment_end))   in_comment = FALSE;
        continue;         /* but always ignore this line */
     }

     if (cs = strstr (line, comment_start)) {       /* if there is a comment */
        if (cs != line) {                            /* not at the beginning of the line */
          strncpy (cfline, line, (int) (cs - line));/* save the beginning */
	  cfline[cs - line] = '\0';
        }
        else                                        /* o/w */
          cfline[0] = '\0';                         /* skip */
                                                    /* copy after comment ends */
        if (ce = strstr (cs, comment_end)) {
          strcat (cfline, ce+2);
          strcpy (line, cfline);                    /* replace original line */
          removing_comments = TRUE;
          continue;
        }
        else {
          in_comment = TRUE;
          continue;
        }
     }




     if ( sscanf(line, "#define %s %d", name_and_type, &value) == 2) {

	    if ( ! (type = (char *) strrchr(name_and_type, '_')))
	      type = name_and_type;
	    else
	      type++;

	    if (!strcmp("width", type))
	       *width = (unsigned int) value;

	    else if (!strcmp("height", type))
	       *height = (unsigned int) value;

	    else if (!strcmp("hot", type)) {

        		if (type-- == name_and_type || type-- == name_and_type)
		           continue;
        		if (!strcmp("x_hot", type))
		           *x_hot = value;
         		else if (!strcmp("y_hot", type))
		           *y_hot = value;
	    }

     }        
     else if ( sscanf(line, "static char %s = {",  name_and_type ) == 1)  {
        *version10p = 0;
        header_read = TRUE;
     }
     else if ( sscanf(line, "static unsigned char %s = {",  name_and_type ) == 1)  {
        *version10p = 0;
        header_read = TRUE;
     }
     else if ( sscanf(line, "static short %s = {",  name_and_type ) == 1)  {
        *version10p = 1;
        header_read = TRUE;
     }
     else {      /* check for and skip blank lines */
        for (i = 0; i < strlen(line); i++) {
            if (isspace(line[i])) continue;
            else                              /* found something other than white space */
                return IL_ERROR_EFS_NOT_MINE;
        }
        continue;
     }

     if (header_read)  {
           	if (!(type = (char *) strrchr(name_and_type, '_')))
            	  type = name_and_type;
           	else
            	  type++;

	        if (strcmp("bits[]", type))  {
                 if ( !(*width) || !(*height) ) 
                     return IL_ERROR_EFS_NOT_MINE;
            }
     }   


  }  /* end while */


  if ( ((*width) % 16)  && ( ((*width) % 16)  < 9) && (*version10p) )
	  *padding = 1;
  else
	  *padding = 0;


  /* compute bytes per line and total size */
  *bytes_per_width  = ( (*width)+7)/8 + *padding;
  *size  = *bytes_per_width * (*height);

  if ((*size == 0) || (*bytes_per_width == 0))
     return IL_ERROR_EFS_NOT_MINE;


  return ( 0 );
} 
  



/*  ------------------------ xbmOpen ------------------------- */
/*  Function for ilEFSFileTypeInfo.Open().  Open the given file, return
    the number of pages in it and ptr to private, or null if error.
*/

static ilPtr xbmOpen (
    ilEFSFileType           fileType,
    char                   *fileName,
    unsigned int            openMode,
    unsigned long          *pNPages
    )
{
register xbmPrivPtr        pPriv;
FILE                       *stream;
ilEFSPageInfo              info;
ilError                    error;
ilBool                     doWrite;
long                       x_hot, y_hot;
ilBool                     padding,  version10p;
long                       bytes_per_width, size;
ilImageDes                 *pdes;
ilImageFormat              *pformat;



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
           if not a XBM file, or "empty file" if no images & not error.
       */

       error = xbmreadheader(stream, &info.width, &info.height, &bytes_per_width, &size, 
                          &x_hot, &y_hot, &version10p, &padding);
       if(error) {
           fileType->context->error = error;
           fclose (stream);
           return (ilPtr)NULL;
       }
    }


    pPriv = (xbmPrivPtr)malloc (sizeof (xbmPrivRec));
    if (!pPriv) {
        fileType->context->error = IL_ERROR_MALLOC;
        return (ilPtr)NULL;
    }


    pPriv->xbmdata.stream = stream;

    pPriv->xbmdata.fname = (char *) malloc( strlen(fileName) + 1);
    if (pPriv->xbmdata.fname != NULL)
           strcpy(pPriv->xbmdata.fname , fileName );

    pPriv->xbmdata.width = info.width;
    pPriv->xbmdata.height = info.height;
    pPriv->xbmdata.size = size;
    pPriv->xbmdata.x_hot = x_hot;
    pPriv->xbmdata.y_hot = y_hot;
    pPriv->xbmdata.version10p = version10p;
    pPriv->xbmdata.padding = padding;
    pPriv->xbmdata.bytes_per_width = bytes_per_width;
    pPriv->xbmdata.size = size;
    pPriv->xbmdata.writeDone = FALSE;

    pPriv->xbmdata.bytes_per_aligned_row = ((info.width + 31)/32) << 2;

    info.page = 0;                         /* single page file; always page 0 */
    info.attributes = 0;
    info.xRes = 0;
    info.yRes = 0;
    pdes = IL_DES_BITONAL;
    pformat = IL_FORMAT_BIT; 
    info.des = *pdes;
    info.format = *pformat;
    pPriv->info = info;
   
    fileType->context->error = IL_OK;
    return (ilPtr)pPriv;

}



/*  ------------------------ xbmClose ------------------------- */
/*  Function for ilEFSFileTypeInfo.Close().  Close the given file.
*/

static ilBool xbmClose (
    ilEFSFile               file
    )
{
register xbmPrivPtr        pPriv;

    pPriv = (xbmPrivPtr)file->pPrivate;
    fclose (pPriv->xbmdata.stream);
    free( pPriv->xbmdata.fname );
    return TRUE;
}



/*  ------------------------ xbmGetPageInfo ---------------------- */
/*  Function for ilEFSFileTypeInfo.GetPageInfo().  Return information
    on the current page.  Called only if file opened in read mode.
*/

static ilBool xbmGetPageInfo (
    ilEFSFile               file,
    register ilEFSPageInfo *pInfo
    )
{
xbmPrivPtr                 pPriv;

    /*  Return info on the current (only) image in the file. */

    *pInfo = ((xbmPrivPtr)file->pPrivate)->info;
    return TRUE;
}





/* --------------------- NextByte -------------------------------- */
/*  Read the next hex value in the input stream; return -1 if EOF .
*/
                                                                   
static ilError NextByte ( 
      FILE           *fstream,
      unsigned char  *value
      )
{
  unsigned char  ch;
  int            got_one, error;
  ilBool         done;

  /* loop, accumulate hex value until we hit delimiter.
     skip any initial delimiters.
  */

  error = 0;
  *value = 0;
  got_one = 0;
  done = FALSE;
  while ((got_one < 2) && (!done)) {

         ch = getc(fstream);
         if ( ch != EOF ) {
             if (isascii(ch) && isxdigit(ch)) {
                   *value = (*value << 4) + hexTable[ch];
                   got_one++;
             }  
             else  if ((hexTable[ch]) < 0  && got_one)
                      done++; 
             else got_one = 0; 
         } 

         else {
            error = -1;
            done++;
         }

  }
  *value = ilBitReverseTable[ *value ];
  return(error);
}

 


/*  --------------------- xbmReadImageExecute --------------------- */
/*  Execute() for reading XBM.  Read the whole XBM image out to the 
    dst image and signal last strip.
*/

static ilError xbmReadImageExecute (
    ilExecuteData          *pData,
    long                   dstLine,
    long                   *pNLines
    )
{
register xbmRWPrivPtr      pPriv;
ilPtr                      pDst;
ilImagePlaneInfo           *pPlane;
register int               bytes;
register int               y;
int                        lastcount;
int                        nlines;


    pPriv = (xbmRWPrivPtr)pData->pPrivate;
    if ((pPriv->height - pPriv->linecount) <= 0) 
        return IL_OK;

    /* compute no. of lines in strip */
    nlines = ( (pPriv->linecount + pPriv->stripheight ) <= pPriv->height )  
                ?  pPriv->stripheight  :  pPriv->height - pPriv->linecount;

    pPlane = pData->pDstImage->plane;

    pDst = pPlane->pPixels + dstLine * pPlane->nBytesPerRow;

    lastcount = pPriv->linecount;
    /* Begin decode */

	if (!pPriv->version10p) {
	    unsigned char *ptr,  value;

        for (y = lastcount,  ptr = pDst;   y < (lastcount + nlines);  y++, pPriv->linecount++)  {

              for ( bytes = 1;  bytes < pPriv->bytes_per_width;  bytes++) {
            	  if ((NextByte(pPriv->stream, &value)) < 0) 
               		   return (IL_ERROR_EFS_EOF);
                  *ptr++ = value;
              }
        	  if ((NextByte(pPriv->stream, &value)) < 0) 
            		return (IL_ERROR_EFS_EOF);
              *ptr++  = value;

              /* add pad if necessary */
              while ( bytes < pPriv->bytes_per_aligned_row ) {
                    *ptr++ = 0;
                    bytes++; 
              }
        }

	} 
    else      /* version10p not supported right now */
        return (IL_ERROR_EFS_UNSUPPORTED); 


    *pNLines = pPriv->linecount - lastcount;

    if ( pPriv->linecount >= pPriv->height ) 
       return  IL_ERROR_LAST_STRIP;   
    else
       return IL_OK;     

}






/*  ------------------------ xbmReadImage ---------------------- */
/*  Function for ilEFSFileTypeInfo.ReadImage().  Add a producer element
    to read the current page into the given pipe.
*/

static ilBool xbmReadImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned int            readMode        /* ignored - no mask images */
    )
{
register xbmPrivPtr        pPriv;
register xbmRWPrivPtr      pRWPriv;
ilDstElementData           dstData;

        /*  Add a pipe producer to read the entire XBM file into the pipe.
            The des, format and size are in private, from xbmOpen().
        */
    pPriv = (xbmPrivPtr)file->pPrivate;
    dstData.producerObject = (ilObject)NULL;
    dstData.pDes = &pPriv->info.des;
    dstData.pFormat = &pPriv->info.format;
    dstData.width = pPriv->info.width;
    dstData.height = pPriv->info.height;

    /* Get recommended strip size */
    dstData.stripHeight = ilRecommendedStripHeight( &pPriv->info.des, &pPriv->info.format,
                                                    pPriv->info.width, pPriv->info.height );

    dstData.constantStrip = TRUE;
    dstData.pPalette = (unsigned short *)NULL;
    dstData.pCompData = (ilPtr)NULL;

    pRWPriv = (xbmRWPrivPtr) ilAddPipeElement ( pipe, IL_PRODUCER, sizeof(xbmRWPrivRec), 0, (ilSrcElementData *)NULL, 
                                                &dstData, xbmreadinit, IL_NPF, IL_NPF, xbmReadImageExecute, 0);
        
    if (!pRWPriv)
         return FALSE;

 
       /*  Element added; copy relevant data into pipe element private */

    *pRWPriv = pPriv->xbmdata;
    pRWPriv->stripheight = dstData.stripHeight;

    return TRUE;
}








/*  --------------------- xbmWriteImageExecute --------------------- */
/*  Execute() for writing XBM files. 
*/

static ilError xbmWriteImageExecute (
    ilExecuteData          *pData,
    long                   dstLine,
    long                   *pNLines
    )
{
#define BYTES_PER_OUTPUT_LINE   12

register xbmRWPrivPtr      pPriv;
ilPtr                      pBytes;
unsigned char              *ptr;
int                        x, y, byte;


    pPriv = (xbmRWPrivPtr) pData->pPrivate;
    if (*pNLines != pPriv->height)      /* ? not complete strip - error */
         return IL_ERROR_EFS_IO;

    /*  Exit if a write has already been done, as per EFS spec. */
    if (pPriv->writeDone)
        return IL_OK;
 

    /* Write the standard xbm header */
    fprintf(pPriv->stream, "#define %s_width %d\n", pPriv->fname, pPriv->width);
    fprintf(pPriv->stream, "#define %s_height %d\n", pPriv->fname, pPriv->height);
    if(pPriv->x_hot != -1) {    
           fprintf(pPriv->stream, "#define %s_x_hot %d\n", pPriv->fname, pPriv->x_hot);
           fprintf(pPriv->stream, "#define %s_y_hot %d\n", pPriv->fname, pPriv->y_hot);
    }


    /* Set ptr to src image data */
     pBytes = pData->pSrcImage->plane[0].pPixels + 
                     pData->srcLine * pData->pSrcImage->plane[0].nBytesPerRow;


    /* Write out data */
    fprintf(pPriv->stream,  "static char %s_bits[] = {\n   ", pPriv->fname);
    fprintf(pPriv->stream, "0x%02x",  ilBitReverseTable[ *pBytes++ ] );
    ptr = pBytes;
    x = byte = 1;

    for ( y = 0;   y  < pPriv->height;   y++,  x = 0)  {
        for(;  x  <  pPriv->bytes_per_width;  byte++,  x++, ptr++) {

            if(byte  &&  (! (byte % BYTES_PER_OUTPUT_LINE)))
               fprintf(pPriv->stream,  ",\n   ");
            else
               fprintf(pPriv->stream,  ", ");
       
            fprintf(pPriv->stream, "0x%02x",  ilBitReverseTable[ *ptr ] );     
        }
        /*  Remove (skip over) padding  */
        while (x <  pPriv->bytes_per_aligned_row) {
             ptr++;
             x++;
        }
    }

    fprintf(pPriv->stream, "};\n");

    /* Cleanup */
    pPriv->writeDone = TRUE;
    fflush (pPriv->stream);             /* flush output buffers */
    return IL_OK;
}








/*  ------------------------ xbmWriteImage ---------------------- */
/*  Function for ilEFSFileTypeInfo.WriteImage().  Add a consumer
    to write the pipe image to the given file.
*/

static ilBool xbmWriteImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned long           xRes,
    unsigned long           yRes,
    ilClientImage           maskImage
    )
{
register xbmPrivPtr        pPriv;
register xbmRWPrivPtr      pRWPriv;
ilPipeInfo                 info;
ilImageDes                 des;
ilImageFormat              format;
ilSrcElementData           srcdata;


    /*  Query the pipe -force decompress; error if not "forming". */
    if (ilGetPipeInfo (pipe, TRUE, &info, &des, &format) != IL_PIPE_FORMING) {
        if (!pipe->context->error)
            ilDeclarePipeInvalid (pipe, IL_ERROR_PIPE_STATE);
        return FALSE;
    }

    /* If the pipe image is not already std. bitonal format than convert */

    if( (des.compression != IL_UNCOMPRESSED) || (des.type != IL_BITONAL) ||
        (des.nSamplesPerPixel != 1) || (des.blackIsZero) ||
        (des.nLevelsPerSample[0] != 2) || (format.sampleOrder != IL_SAMPLE_PIXELS ) ||
        (format.byteOrder != IL_MSB_FIRST) || (format.rowBitAlign != 32) ||
        (format.nBitsPerSample[0] != 1) )  {

            if (!ilConvert (pipe,  IL_DES_BITONAL, IL_FORMAT_BIT, 0, NULL ))
                  return FALSE;

            if (ilGetPipeInfo (pipe, TRUE, &info, &des, &format) != IL_PIPE_FORMING) {
                if (!pipe->context->error)
                    ilDeclarePipeInvalid (pipe, IL_ERROR_PIPE_STATE);
                return FALSE;
            }

    }

    srcdata.consumerImage = (ilObject)NULL;
    srcdata.stripHeight = info.height;
    srcdata.constantStrip = TRUE;
    srcdata.minBufferHeight = 0;

    /*  Add a consumer element to write the resulting data. */
    pPriv = (xbmPrivPtr)file->pPrivate;
    pRWPriv =  (xbmRWPrivPtr) ilAddPipeElement( pipe, IL_CONSUMER, sizeof (xbmRWPrivRec), 0,
                                                &srcdata, (ilDstElementData *)NULL, IL_NPF, 
                                                IL_NPF, IL_NPF, xbmWriteImageExecute, 0);

    if (!pRWPriv)
        return FALSE;

    /*  Element added; copy relevant data into pipe element private */
    pRWPriv->stream                 =  pPriv->xbmdata.stream;
    pRWPriv->fname                  =  pPriv->xbmdata.fname;
    pRWPriv->width                  =  info.width;
    pRWPriv->height                 =  info.height;
    pRWPriv->bytes_per_width        =  (info.width + 7)/8;
    pRWPriv->bytes_per_aligned_row  =  ((info.width + 31)/32) << 2;
    pRWPriv->size                   =  pRWPriv->bytes_per_width * info.height;
    pRWPriv->x_hot                  =  -1;
    pRWPriv->y_hot                  =  -1;
    pRWPriv->writeDone              =  FALSE;

    return TRUE;

}





/*  ------------------------ _ilefsInitXBM ------------------------- */
/*  Add XBM EFS file type handler to the given context; 
    return false if error.
*/

ilBool _ilefsInitXBM (
    ilContext               context
    )
{
ilEFSFileTypeInfo           info;

    bzero (&info, sizeof (ilEFSFileTypeInfo));

    strcpy (info.name, "XBM");
    strcpy (info.displayName, "XBM");
    info.checkOrder = 50;

    info.nExtensions = 3;
    strcpy (info.extensions[0], "xbm");
    strcpy (info.extensions[1], "XBM");
    strcpy (info.extensions[2], "Xbm");

    info.openModes = (1<<IL_EFS_READ) | (1<<IL_EFS_READ_SEQUENTIAL) | (1<<IL_EFS_WRITE);
    info.attributes = 0;

    info.Open = xbmOpen;
    info.GetPageInfo = xbmGetPageInfo;
    info.ReadImage = xbmReadImage;
    info.WriteImage = xbmWriteImage;
    info.Close = xbmClose;

    return (ilEFSAddFileType (context, &info, NULL) != NULL);
}
