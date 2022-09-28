/**---------------------------------------------------------------------
***	
***    file:           efsstarbm.c
***
***    description:    Image Library (IL) EFS support for starbase bitmap file
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

#define FAILURE -1              
#define PIXEL_MAJOR -1
#define PLANE_MAJOR -2                
#define NORMAL       0
#define MONOTONIC    1
#define CMAP_FULL    4
#define BMFREV 1	/* current rev of the bitmap file definition */
#define HEADER sizeof(bf_header) /* currently one block */
static char bfname[16] = "Bitmapfile";                                     
/********************************************************************************


                   Starbase bitmap file format  
                ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

                                    File Byte address
                   _______________________
                  |                       | <-- 0
                  |    HP header data     |
                  |                       |
       Header   { |_______________________| 
    (256 bytes)   |                       | <-- 192     
                  |    User header data   |
                  |                       |
                  |_______________________| 
                  |                       | <-- 256
                  |      Color Map        | ( color map storage 
                  |  (RGB Float Values)   |   is optional )
                  |                       |
                  |_______________________| 
                  |                       | <-- <bm_loc> ( points to    
                  |      Bitmap Data      |      first byte in bitmap data)
                  |                       |
                  |-----------------------|
                  |                       |
                  |   Additional HP Data  |
                  |                       | 
                  |_______________________| <-- <eod_loc> point to last    
                  |                       |     byte of HP data )
                  |  User Data if needed  |
                  |                       |
                   -----------------------
                            
*********************************************************************************/

typedef struct { 
    char file_id[16];         /* "Bitmapfile" */
    int  rev;                 /* revision of file format */
    char device_id[16];       /* device driver name */
    int  bm_loc;              /* offset in bytes to start of bitmap information 
                                 measured from the start of the file */
    int  eod_loc;             /* byte offset of last byte of HP data in the file */
    int  xstart, ystart;      /* upper left corner of source in pixels. This is pixel
                                 (0,0) in the file */
    int  xlen, ylen;          /* x,y dimensions of bitmap */
    int  bm_mode;             /* format of bitmap data:
                                 -1  --> pixel major full depth
                                 -2  --> plane major full depth
                                 >=0 --> plane major, single plane ( number indicates plane #
                                 with plane 0 = least significant) */
    int  depth;               /* number of bits stored per pixel.
                                 This indicates how many many planes were stored */
    int  pixel_align;         /* alignment of pixel data in file. 
                                 Allowable values are 1 and 8.
                                 1 --> every bit is a pixel
                                 8 --> every byte is a pixel */
    int  num_banks;           /* number of banks. Only relevant for pixel-major format */
    int  disp_en;             /* display enable mask of source */
    int  cmap_mode;           /* color map mode:
                                 0 = normal color mode
                                 1 = monotonic color mode
                                 4 = full color mode */
    int  csize;               /* Number of color map entries */
    int  back_index;          /* Index value of background color entry in color map */
    char hp_extend[100];      /* Reserved for HP extensions */
    char user_extend[64];     /* Reserved for user extensions */

    }bf_header;



    /*  Private data for each starbase bitmap EFS file */                                       
typedef struct {
    FILE                   *stream;         /* stdio stream file */
    unsigned long           length;         /* # of bytes in file */
    ilEFSPageInfo           info;           /* info on current/only page */
    char                   *filename;       /* name of bitmap file */
    bf_header               bfHeader;       /* Copy of bitmap file header info */
    } starbfPrivRec, *starbfPrivPtr;

    /*  Private data for read pipe element functions */
typedef struct {
    FILE                   *stream;         /* stdio stream file */
    unsigned long           length;         /* # of bytes in file */
    unsigned long           height;         /* height of image */
    char                   *filename;       /* name of bitmap file */
    bf_header               bfHeader;       /* Copy of bitmap file header info */
    unsigned short         *pPalette;       /* pointer to color map */
    } starbfRPrivRec, *starbfRPrivPtr;

       

    /* code lifted from starbase */

/*  --------------------- getcolormap ----------------------------- *
 *  Function to load the default starbase color map(all 256 entries)*
 *  Code was leveraged from starbase                                *
 *  NOTE: This routine is called only if the bitmap file contains   *
 *  no color map.                                                   *
 *------------------------------------------------------------------*/


static ilError getcolormap(
    char        *cmap,
    int          mode
    )
{
    static float def_map[18][3] = { 0.0,0.0,0.0, 1.0,1.0,1.0,
                                    1.0,0.0,0.0, 1.0,1.0,0.0,
                                    0.0,1.0,0.0, 0.0,1.0,1.0,
                                    0.0,0.0,1.0, 1.0,0.0,1.0,
                                    0.1,0.1,0.1, 0.2,0.2,0.2,
                                    0.3,0.3,0.3, 0.4,0.4,0.4,
                                    0.5,0.5,0.5, 0.6,0.6,0.6,
                                    0.7,0.7,0.7, 0.8,0.8,0.8,
                                    0.9,0.9,0.9, 1.0,1.0,1.0};


    float    *map;
    int       loop_break;
    int       i,r,g,b;

	/* set background color */
	map = (float *)cmap;
	*map = 0.0; map++;	/* set index 0 red */
	*map = 0.0; map++;	/* set index 0 green */
	*map = 0.0; map++;	/* set index 0 blue */
    loop_break = 18;
	/* set up the rest of the color map based on mode */
	switch (mode) {
        case NORMAL:	

			/* first put out the 17 color default map */
			for (i=1; i<18; i++){
                *map = def_map[i][0]; map++;    /* red */
                *map = def_map[i][1]; map++;    /* green */
                *map = def_map[i][2]; map++;    /* blue */
                }

            /* do rest of entries with function beginning at 1.0 */
            for (b=4; b>=0; b-- ) 
                for (r=6; r>=0; r--)
                    for (g=6; g>=0; g--) 
                        if (((g!=0)&(g!=6))|((r!=0)&(r!=6))|((b!=0)&(b!=4))) {
							if (loop_break++ > 256) 
								return IL_OK;
							*map = ((float)r)/6; map++;	/* red */
							*map = ((float)g)/6; map++;	/* green */
							*map = ((float)b)/4; map++;	/* blue */
                            }
            break;

        case MONOTONIC:	

            for (i=1; i<256; i++){
                *map = ((float)i)/255; map++;    /* red bank */
                *map = ((float)i)/255; map++;    /* green bank */
                *map = ((float)i)/255; map++;    /* blue bank */
                }

            break;

        default:    break;
    }    /*end switch*/
    return IL_OK;
}



/*  --------------------- starbfDestroy ---------------------------- *
 *  Function to free palette space if allocated                      *
 *------------------------------------------------------------------ */
        
static ilError starbfDestroy (
    starbfRPrivPtr      pPriv
    )
{
    if (pPriv->pPalette)
        free (pPriv->pPalette);
    return IL_OK;
}



/*  --------------------- starbfReadImageExecute ------------------- *
 *  Execute() for reading starbase bitmap. Read the whole bitmap out *
 *  to the dst image                                                 *
 *------------------------------------------------------------------ */

static ilError starbfReadImageExecute(
    ilExecuteData          *pData,
    long                    dstLine,
    long                   *pNLines
    )
{
register starbfRPrivPtr     pPriv;
long                        nBytes;
ilPtr                       pDst;
FILE                       *redbank,*greenbank,*bluebank;
bf_header                   header;
unsigned char               red,green,blue; 
unsigned long               greenstart, bluestart, banksize;
int                         i;

    pPriv = (starbfRPrivPtr)pData->pPrivate;
    nBytes = pPriv->length;

    pDst = pData->pDstImage->plane[0].pPixels + dstLine * pData->pDstImage->plane[0].nBytesPerRow;
    if ( fseek (pPriv->stream, 0, 0) )
        return IL_ERROR_FILE_IO;     
    if ( fread( &header, sizeof(bf_header), 1, pPriv->stream ) !=1 )
        return IL_ERROR_FILE_IO;

    if ( header.bm_mode == PIXEL_MAJOR ){
        if ( header.num_banks == 1 ) {
                                       /* one bank. Every byte is a pixel */
                                       /* load the pixel bytes */
            if (fseek (pPriv->stream, (long)header.bm_loc, 0)) /* seek to start of bitmap; != 0 return => error */
                return IL_ERROR_FILE_IO;
            if (fread (pDst, nBytes, 1, pPriv->stream) != 1)
                return IL_ERROR_FILE_IO;     /* failed to read 1 record; error */

            }  /*** end num_banks=1 ***/
        else if ( header.num_banks == 3 ) {
            redbank = fopen(pPriv->filename, "r");
            bluebank = fopen(pPriv->filename, "r");
            greenbank = fopen(pPriv->filename, "r");
            if (fseek ( redbank, (long)header.bm_loc, 0)) /* seek to start of red bank; != 0 return => error */
                return IL_ERROR_FILE_IO;
            banksize = header.xlen * header.ylen;
            greenstart = header.bm_loc + banksize; 
            if (fseek ( greenbank, greenstart , 0 )) /* seek to start of green bank; != 0 return => error */
                return IL_ERROR_FILE_IO;
            bluestart = greenstart + banksize;
            if (fseek ( bluebank, bluestart , 0 )) /* seek to start of blue bank; != 0 return => error */
                return IL_ERROR_FILE_IO;
            if ( header.cmap_mode == CMAP_FULL){
                for (i=0; i<banksize; i++){
                    red = getc(redbank);
                    green = getc(greenbank);
                    blue = getc(bluebank);
                    *pDst++ = blue;
                    *pDst++ = green;
                    *pDst++ = red;
                    }
                }  /***end cmap_mode=CMAP_FULL***/
            else return IL_ERROR_EFS_UNSUPPORTED;
            fclose(redbank);
            fclose(greenbank);         
            fclose(bluebank);
            }  /*** end num_banks > 2 ***/
        else return IL_ERROR_EFS_UNSUPPORTED;

        } /* end if PIXEL_MAJOR */
    else if ( header.bm_mode >= 0 ) {  /* plane major, single plane        */
                                       /* single plane is in packed format */
                                       /* every bit is a pixel             */      
        if (fseek (pPriv->stream, (long)header.bm_loc, 0)) /* seek to start; != 0 return => error */
            return IL_ERROR_FILE_IO;
        if (fread (pDst, nBytes, 1, pPriv->stream) != 1)
            return IL_ERROR_FILE_IO;     /* failed to read; error */

        } /* end if bm_mode >=0 */
    else 
        return IL_ERROR_EFS_UNSUPPORTED;   /* return ERROR */

    *pNLines = pPriv->height;
    return IL_ERROR_LAST_STRIP;      /* the single strip was read */
}




/*  ------------------------ starbfReadImage ---------------------------- *
*  Function for ilEFSFileTypeInfo.ReadImage().  Add a producer element    *
*            to read the current page into the given pipe.                *
*                                                                         *
*-------------------------------------------------------------------------*/

static ilBool starbfReadImage (
    ilPipe                  pipe,
    ilEFSFile               file,
    unsigned int            readMode        /* ignored - no mask images */
    )

{
register starbfPrivPtr        pPriv;
register starbfRPrivPtr       pRPriv;
ilDstElementData              dstData;
unsigned short               *pPalette;
float                        *cmaptr;
float                         r,g,b;
int                           i,csize;
char                         *cmap;
size_t                        cmap_size;

        /*  Add a pipe producer to read the entire bitmap file into the pipe.
            The des, format and size are in private, from starbfOpen().
        */
     pPriv = (starbfPrivPtr)file->pPrivate;
       
        /* if pseudo color bitmap, allocate memory for the color map */
    if ( pPriv->info.des.type == IL_PALETTE ) {

        if ( pPriv->bfHeader.csize == 0 )
            /* no color map in the file but we will use the default color map */
            csize = 256;
        else 
            csize = (pPriv->bfHeader.csize > 256 ) ? 256 :pPriv->bfHeader.csize; 

        pPalette = (unsigned short *)malloc(sizeof (unsigned short) * 3 * 256);
        if (!pPalette){
            file->context->error = IL_ERROR_MALLOC;
            return FALSE;
            }
        /* load the palette */
        /* Starbase stores the color map as an array of floats in the range 0 to 1.0
           We convert them to integers in the range 0 to 255 
         */                                   
        cmap_size = csize * 3 * sizeof(float);
        cmap = (char *)calloc( 1, cmap_size );
        if (!cmap){
            free(pPalette);
            file->context->error = IL_ERROR_MALLOC;
            return FALSE;
            }
             
        if ( pPriv->bfHeader.csize ){  /* if color map is stored */
            if (fseek(pPriv->stream, (long)sizeof(bf_header), 0))
               return IL_ERROR_FILE_IO ;
            if (fread( cmap, cmap_size, 1, pPriv->stream ) != 1) 
               return IL_ERROR_FILE_IO ;
            }
        else                          /* No color map. Get default */
            getcolormap( cmap, pPriv->bfHeader.cmap_mode  );
            
        cmaptr = (float *)cmap;          

        for (i = 0; i < csize; i++) {   /* NOTE: csize may be < 256 */
            r = *cmaptr++;
            g = *cmaptr++;
            b = *cmaptr++;
            pPalette[i] = ((unsigned short)(r * 65535.0));
            pPalette [i + 256] = ((unsigned short)(g * 65535.0));
            pPalette [i + 512] = ((unsigned short)(b * 65535.0));
            }
         free(cmap);
        }

    else pPalette = (unsigned short *)NULL;

    dstData.producerObject = (ilObject)NULL;
    dstData.pDes = &pPriv->info.des;
    dstData.pFormat = &pPriv->info.format;
    dstData.width = pPriv->info.width;
    dstData.height = pPriv->info.height;

#ifdef NOTDEF
    /* Get recommended strip height from the utility function */
    dstData.stripHeight = ilRecommendedStripHeight( &pPriv->info.des, &pPriv->info.format,
                                               pPriv->info.width, pPriv->info.height );
#else
    dstData.stripHeight = pPriv->info.height;
#endif
    dstData.constantStrip = TRUE;
    dstData.pCompData = (ilPtr)NULL;
    dstData.pPalette = pPalette;  

    pRPriv = (starbfRPrivPtr)ilAddPipeElement (pipe, IL_PRODUCER, 
        sizeof (starbfRPrivRec), 0, (ilSrcElementData *)NULL, &dstData,
        IL_NPF, IL_NPF, starbfDestroy, starbfReadImageExecute, 0);
    if (!pRPriv){
        if (pPalette) free(pPalette);
        return FALSE;
        }

        /*  Element added; copy relevant data into pipe element private */
    pRPriv->stream = pPriv->stream;
    pRPriv->length = pPriv->length;
    pRPriv->height = pPriv->info.height;
    pRPriv->filename = pPriv->filename;
    pRPriv->pPalette = pPalette;
    return TRUE;
}
                                        

/*  ------------------------ starbfOpen -------------------------------  *
 *  Function for ilEFSFileTypeInfo.Open().  Open the given file, return  *
 *  the number of pages in it and ptr to private, or null if error.      *
 *                                                                       *
 *-----------------------------------------------------------------------*/

 
static ilPtr starbfOpen (
    ilEFSFileType           fileType,
    char                   *fileName,
    unsigned int            openMode,
    unsigned long          *pNPages
    )
{
register ilError            error;
register starbfPrivPtr      pPriv;
FILE                       *stream;
ilEFSPageInfo               info;
unsigned long               length;
bf_header                   header;

        /* if write mode is requested return error. Starbase bitmap efs does
           not support write  */
    if (openMode == IL_EFS_WRITE) {
        fileType->context->error = IL_ERROR_EFS_OPEN_MODE;
        return (ilPtr)NULL;
        }
    else {
            /*  Opening in read mode: connect to file, return "not mine" error
                if not a *base bitmap file.  
            */
        stream = fopen (fileName, "r");
        if (!stream) {
            fileType->context->error = IL_ERROR_EFS_OPEN;
            return (ilPtr)NULL;
            }

        *pNPages = 1;
        }   /* END a read */


        /*  Allocate private, save info in it and return a ptr to it. */
    pPriv = (starbfPrivPtr)malloc (sizeof (starbfPrivRec));
    if (!pPriv) {
        error = IL_ERROR_MALLOC;
        goto openError;
        }

        /* read the header from the bitmap file into pPriv */

    if ( fseek (stream, 0, 0) ) {
            error = IL_ERROR_FILE_IO;
            goto openError;
            }

    if ( fread( &header, sizeof(bf_header), 1, stream ) != 1 ){
        error = IL_ERROR_EFS_NOT_MINE;
        goto openError;
        }

     pPriv->bfHeader = header;
     pPriv->filename = fileName;
        /* check the revision and the file id */
    if ((pPriv->bfHeader.rev != BMFREV) ||
        (strncmp(pPriv->bfHeader.file_id,bfname,strlen(bfname)) !=0)) {
        error = IL_ERROR_EFS_NOT_MINE; 
        goto openError;
        }
    
    if ( (pPriv->bfHeader.bm_mode == PLANE_MAJOR) || 
         ((pPriv->bfHeader.bm_mode >=0) && ( pPriv->bfHeader.depth !=1)) ) {
            error = IL_ERROR_EFS_UNSUPPORTED; 
            goto openError;
            }

    pPriv->stream = stream;
    if (pPriv->bfHeader.bm_mode >=0)
        pPriv->length = ((pPriv->bfHeader.xlen + 7)/8) * pPriv->bfHeader.ylen * pPriv->bfHeader.num_banks;
    else 
        pPriv->length = pPriv->bfHeader.xlen * pPriv->bfHeader.ylen * pPriv->bfHeader.num_banks;
    info.width = pPriv->bfHeader.xlen;
    info.height = pPriv->bfHeader.ylen;
    if ( (pPriv->bfHeader.cmap_mode == CMAP_FULL) && (pPriv->bfHeader.num_banks == 3) ) {
        info.des = *IL_DES_RGB;
        info.format = *IL_FORMAT_3BYTE_PIXEL;
#ifdef SUPPORT4BITS
/* 
    NOTE: FOLLOWING THE CORRECT THING TO DO - BUT THE IL WON'T HANDLE
    16 LEVELS/SAMPLE IN AN 8 BIT VALUE - IT CAN'T CONVERT IT!!!
    SO FOR NOW JUST RETURN "UNSUPPORTED" ERROR.
*/
            /*  RGB: each component is a byte, but only depth/#components pixels of
                the byte are used.  Set the des.nLevelsPerSample to 2**(depth/#components)
                e.g. to 16 if depth is 12.  This allows n (e.g. 12) bit RGB to work.
            */
        info.des.nLevelsPerSample[0] = 
            info.des.nLevelsPerSample[1] = 
            info.des.nLevelsPerSample[2] = 1 << (pPriv->bfHeader.depth / 3);
#else
        if (pPriv->bfHeader.depth != 24) {
            error = IL_ERROR_EFS_UNSUPPORTED;
            goto openError;
            }
#endif
        }
    else if ( pPriv->bfHeader.bm_mode >= 0 ){
        info.des = *IL_DES_BITONAL;
        info.format = *IL_FORMAT_BYTE;
        }
    else {
        info.des = *IL_DES_PALETTE;
        info.format = *IL_FORMAT_BYTE;
        }

    info.xRes = 0;
    info.yRes = 0;
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


/*  ------------------------ starbfClose ------------------------- *
 *  Function for ilEFSFileTypeInfo.Close().  Close the given file. *
 *                                                                 *
 *-----------------------------------------------------------------*/

static ilBool starbfClose (
    ilEFSFile               file
    )
{
    fclose (((starbfPrivPtr)file->pPrivate)->stream);
    return TRUE;
}


/*  ------------------------ starbfGetPageInfo ---------------------- *
 *  Function for ilEFSFileTypeInfo.GetPageInfo().  Return information *
 *   on the current page.  Called only if file opened in read mode.   *
 *                                                                    *
 *------------------------------------------------------------------- */

static ilBool starbfGetPageInfo (
    ilEFSFile               file,
    ilEFSPageInfo          *pInfo
    )
{
        /*  Return info on the current (only) image in the file. */
    *pInfo = ((starbfPrivPtr)file->pPrivate)->info;
    return TRUE;
}

    

/*  ------------------------ _ilefsInitSTARBF -------  ------------------ *
 *  Add STARBM (Starbase Bitmap)  EFS file type handler to the given   *
 *  context; return false if error.                                    *
 *                                                                     *
 *  -------------------------------------------------------------------*/

ilBool _ilefsInitSTARBF (
    ilContext               context
    )
{
ilEFSFileTypeInfo           info;

    bzero (&info, sizeof (ilEFSFileTypeInfo));

    strcpy (info.name, "BMF");
    strcpy (info.displayName, "BMF");
    info.checkOrder = 80;                   /* somewhat arbitrary */

    info.nExtensions = 6;
    strcpy (info.extensions[0], "bf");
    strcpy (info.extensions[1], "BF");
    strcpy (info.extensions[2], "bm");
    strcpy (info.extensions[3], "BM");
    strcpy (info.extensions[4], "bmf");
    strcpy (info.extensions[5], "BMF");

    info.openModes = (1<<IL_EFS_READ) | (1<<IL_EFS_READ_SEQUENTIAL);
    info.attributes = 0;

    info.Open = starbfOpen;
    info.GetPageInfo = starbfGetPageInfo;
    info.ReadImage = starbfReadImage;
    info.Close = starbfClose;

    return (ilEFSAddFileType (context, &info, NULL) != NULL);
}




