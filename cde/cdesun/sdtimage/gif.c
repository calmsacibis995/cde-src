#ifndef lint
static char sccsid[] = "@(#)gif.c 1.7 97/03/04";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

/*  Copyright (c) 1987, 1988, Sun Microsystems, Inc.  All Rights Reserved.
 *  Sun considers its source code as an unpublished, proprietary
 *  trade secret, and it is available only under strict license
 *  provisions.  This copyright notice is placed here only to protect
 *  Sun in the event the source is deemed a published work.  Dissassembly,
 *  decompilation, or other means of reducing the object code to human
 *  readable form is prohibited by the license agreement under which
 *  this code is provided to the user or company in possession of this
 *  copy.
 *
 *  RESTRICTED RIGHTS LEGEND: Use, duplication, or disclosure by the
 *  Government is subject to restrictions as set forth in subparagraph
 *  (c)(1)(ii) of the Rights in Technical Data and Computer Software
 *  clause at DFARS 52.227-7013 and in similar clauses in the FAR and
 *  NASA FAR Supplement.
 *
 * ------------------------------------------------------------------------
 *  These routines are based on routines found in xloadimage. They are
 *  adapted from code by Kirk Johnson
 *
 *  Copyright 1989, 1990 Kirk L. Johnson
 *  Copyright 1989, 1990 Jim Frost and Steve Losen.
 *
 *  Permission to use, copy, modify, distribute, and sell this
 *  software and its documentation for any purpose is hereby granted
 *  without fee, provided that the above copyright notice appear in
 *  all copies and that both that copyright notice and this
 *  permission notice appear in supporting documentation. The
 *  author makes no representations about the suitability of this
 *  software for any purpose. It is provided "as is" without express
 *  or implied warranty.
 *
 *  THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 *  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT
 *  OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 *  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 *  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 *  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "display.h"
#include "image.h"
#include "imagetool.h"

typedef unsigned char BYTE ;

#define  GIFIN_SUCCESS       0         /* Success. */
#define  GIFIN_DONE          1         /* No more images. */

#define  GIFIN_ERR_BAD_SD   -1         /* Bad screen descriptor. */
#define  GIFIN_ERR_BAD_SEP  -2         /* Bad image separator. */
#define  GIFIN_ERR_BAD_SIG  -3         /* Bad signature. */
#define  GIFIN_ERR_EOD      -4         /* Unexpected end of raster data. */
#define  GIFIN_ERR_EOF      -5         /* Unexpected end of input stream. */
#define  GIFIN_ERR_FAO      -6         /* File already open. */
#define  GIFIN_ERR_IAO      -7         /* Image already open. */
#define  GIFIN_ERR_NFO      -8         /* No file open. */
#define  GIFIN_ERR_NIO      -9         /* No image open. */
#define  GIFIN_ERR_PIXEL    -10	       /* Error in Push Pixel */

#define  GIF_RED            0          /* Colormap indices. */
#define  GIF_GRN            1
#define  GIF_BLU            2

#define  GIF_SIG            "GIF87a"
#define  GIF_SIG2           "GIF89a"
#define  GIF_SIG_LEN        6          /* GIF signature length. */
#define  GIF_SD_SIZE        7          /* GIF screen descriptor size. */
#define  GIF_ID_SIZE        9          /* GIF image descriptor size. */
 
#define  GIF_SEPARATOR      ','        /* GIF image separator. */
#define  GIF_EXTENSION      '!'        /* GIF extension block marker. */
#define  GIF_TERMINATOR     ';'        /* GIF terminator. */
 
#define  STAB_SIZE          4096       /* String table size. */
#define  PSTK_SIZE          1024       /* Pixel stack size. */ 
 
#define  NULL_CODE          -1         /* String table null code. */

#define  PUSH_PIXEL(p)                                      \
{                                                           \
  if (pstk_idx == PSTK_SIZE)                                \
  gifin_fatal( catgets (prog->catd, 1, 52,                  \
			  "pixel stack overflow in PUSH_PIXEL()"));   \
  else                                                      \
    pstk[pstk_idx++] = (p) ;                                \
}

static int gifin_open_file () ;
static int gifin_open_image () ;
static int gifin_get_pixel (int *);
static int gifin_close_file () ;
static int gifin_load_cmap (BYTE [][], int) ;
static int gifin_skip_extension () ;
static int gifin_read_data_block () ;
static int gifin_push_string (int) ;

static void gifin_add_string (int, int) ;
static void gifin_fatal (char *) ;

static int interlace_start[4] = {  /* Start line for interlacing. */
  0, 4, 2, 1
} ;

static int interlace_rate[4]  = {  /* Vertical acceleration rate. */
  8, 8, 4, 2
} ;

static BYTE file_open  = 0 ;       /* Status flags. */
static BYTE image_open = 0 ;

static int root_size ;             /* Root code size. */
static int clr_code ;              /* Clear code. */
static int eoi_code ;              /* End of information code. */
static int code_size ;             /* Current code size. */
static int code_mask ;             /* Current code mask. */
static int prev_code ;             /* Previous code. */

/* NOTE: a long is assumed to be at least 32 bits wide.  */

static long work_data ;            /* Working bit buffer */
static int  work_bits ;            /* Working bit count */

static BYTE buf[256] ;             /* Byte buffer */
static int  buf_cnt ;              /* Byte count */
static int  buf_idx ;              /* Buffer index */

static int table_size ;            /* String table size */
static int prefix[STAB_SIZE] ;     /* String table : prefixes */
static int extnsn[STAB_SIZE] ;     /* String table : extensions */

static BYTE pstk[PSTK_SIZE] ;      /* Pixel stack */
static int  pstk_idx ;             /* Pixel stack pointer */

static char gif_err_str[120] ;     /* To construct error messages. */
static int  gif_fatal_error ;      /* Set for a fatal GIF read error. */

static int  gifin_rast_width ;       /* Raster width. */
static int  gifin_rast_height ;      /* Raster height. */
static BYTE gifin_g_cmap_flag ;      /* Global colormap flag. */
static int  gifin_g_pixel_bits ;     /* Bits per pixel, global colormap. */
static int  gifin_g_ncolors ;        /* Number of colors, global colormap. */
static BYTE gifin_g_cmap[3][256] ;   /* Global colormap. */
static int  gifin_bg_color ;         /* Background color index. */
static int  gifin_color_bits ;       /* Bits of color resolution. */

static int  gifin_img_left ;         /* Image position on raster. */
static int  gifin_img_top ;          /* Image position on raster. */
static int  gifin_img_width ;        /* Image width. */
static int  gifin_img_height ;       /* Image height. */
static BYTE gifin_l_cmap_flag ;      /* Local colormap flag. */
static int  gifin_l_pixel_bits ;     /* Bits per pixel, local colormap. */
static int  gifin_l_ncolors ;        /* Number of colors, local colormap. */
static BYTE gifin_l_cmap[3][256] ;   /* Local colormap. */
static BYTE gifin_interlace_flag ;   /* Interlace image format flag. */

FILE *save_file;

XilImage 	gif_save_image;
ImageInfo      *gif_info;
unsigned char  *gif_data;
static int 
push_pixel (p)
BYTE p;
{  
  if (pstk_idx == PSTK_SIZE)  {
    gifin_fatal(catgets (prog->catd, 1, 53,
			 "pixel stack overflow in PUSH_PIXEL()")) ; 
    return (-1);
    }
  else 
    pstk[pstk_idx++] = (p) ; 

  return (0);
}

/* Open a GIF file, using s as the input stream. */

static int
gifin_open_file()
{

/* Make sure there isn't already a file open. */

  if (file_open) return(GIFIN_ERR_FAO) ;

  file_open = 1 ;            /* Remember that we've got this file open. */

/* Check GIF signature. */

  if (im_fread((char *) buf, 1, GIF_SIG_LEN) != GIF_SIG_LEN)
    return(GIFIN_ERR_EOF) ;

  buf[GIF_SIG_LEN] = '\0' ;
  if (strcmp((char *) buf, GIF_SIG) != 0) 
     if (strcmp((char *) buf, GIF_SIG2) != 0) 
        return(GIFIN_ERR_BAD_SIG) ;

/* Read screen descriptor. */

  if (im_fread((char *) buf, 1, GIF_SD_SIZE) != GIF_SD_SIZE)
    return(GIFIN_ERR_EOF) ;

/* Decode screen descriptor. */

  gifin_rast_width   = (buf[1] << 8) + buf[0] ;
  gifin_rast_height  = (buf[3] << 8) + buf[2] ;
  gifin_g_cmap_flag  = (buf[4] & 0x80) ? 1 : 0 ;
  gifin_color_bits   = ((((int) buf[4]) & 0x70) >> 4) + 1 ;
  gifin_g_pixel_bits = (buf[4] & 0x07) + 1 ;
  gifin_bg_color     = buf[5] ;

  if (buf[6] != 0) return(GIFIN_ERR_BAD_SD) ;

/* Load global colormap. */

  if (gifin_g_cmap_flag)
    {
      gifin_g_ncolors = (1 << gifin_g_pixel_bits) ;

      if (gifin_load_cmap(gifin_g_cmap, gifin_g_ncolors) != GIFIN_SUCCESS)
        return(GIFIN_ERR_EOF) ;
    }
  else gifin_g_ncolors = 0 ;

  return(GIFIN_SUCCESS) ;
}


/*  Open next GIF image in the input stream ; returns GIFIN_SUCCESS if
 *  successful. if there are no more images, returns GIFIN_DONE. (might
 *  also return various GIFIN_ERR codes.)
 */

static int
gifin_open_image()
{
  int i ;
  int separator ;

/* Make sure there's a file open. */

  if (!file_open) return(GIFIN_ERR_NFO) ;

/* Make sure there isn't already an image open. */

  if (image_open) return(GIFIN_ERR_IAO) ;

/* Remember that we've got this image open. */

  image_open = 1 ;

/* Skip over any extension blocks. */

  do
    {
      separator = im_fgetc () ;
      if (separator == GIF_EXTENSION)
        {
          if (gifin_skip_extension() != GIFIN_SUCCESS)
            return(GIFIN_ERR_EOF) ;
        }
    }
  while (separator == GIF_EXTENSION) ;

/* Check for end of file marker. */

  if (separator == GIF_TERMINATOR) return(GIFIN_DONE) ;

/* Make sure we've got an image separator. */

  if (separator != GIF_SEPARATOR) return(GIFIN_ERR_BAD_SEP) ;

/* Read image descriptor. */

  if (im_fread((char *) buf, 1, GIF_ID_SIZE) != GIF_ID_SIZE)
    return(GIFIN_ERR_EOF) ;

/* Decode image descriptor. */

  gifin_img_left       = (buf[1] << 8) + buf[0] ;
  gifin_img_top        = (buf[3] << 8) + buf[2] ;
  gifin_img_width      = (buf[5] << 8) + buf[4] ;
  gifin_img_height     = (buf[7] << 8) + buf[6] ;
  gifin_l_cmap_flag    = (buf[8] & 0x80) ? 1 : 0 ;
  gifin_interlace_flag = (buf[8] & 0x40) ? 1 : 0 ;
  gifin_l_pixel_bits   = (buf[8] & 0x07) + 1 ;

/* Load local colormap. */

  if (gifin_l_cmap_flag)
    {
      gifin_l_ncolors = (1 << gifin_l_pixel_bits) ;

      if (gifin_load_cmap(gifin_l_cmap, gifin_l_ncolors) != GIFIN_SUCCESS)
        return(GIFIN_ERR_EOF) ;
    }
  else gifin_l_ncolors = 0 ;

/* Initialize raster data stream decoder. */

  root_size = im_fgetc () ;
  clr_code  = 1 << root_size ;
  eoi_code  = clr_code + 1 ;
  code_size = root_size + 1 ;
  code_mask = (1 << code_size) - 1 ;
  work_bits = 0 ;
  work_data = 0 ;
  buf_cnt   = 0 ;
  buf_idx   = 0 ;

  for (i = 0; i < STAB_SIZE; i++)     /* Initialize string table. */
    {
      prefix[i] = NULL_CODE ;
      extnsn[i] = i ;
    }

  pstk_idx = 0 ;                      /* Initialize pixel stack. */

  return(GIFIN_SUCCESS) ;
}


/* Try to read next pixel from the raster, return result in *pel. */

static int
gifin_get_pixel(pel)
int *pel ;
{
  int code ;
  int first ;
  int place ;

/* Decode until there are some pixels on the pixel stack. */

  while (pstk_idx == 0)
    {

      if (gif_fatal_error == 1)
          return(GIFIN_ERR_EOF) ;               

/* Load bytes until we have enough bits for another code. */

      while (work_bits < code_size)
        {
          if (buf_idx == buf_cnt)
            {

/* Read a new data block. */

              if (gifin_read_data_block() != GIFIN_SUCCESS)
                return(GIFIN_ERR_EOF) ;

              if (buf_cnt == 0) return(GIFIN_ERR_EOD) ;
            }

          work_data |= ((long) buf[buf_idx++]) << work_bits ;
          work_bits += 8 ;
        }

      code        = work_data & code_mask ;     /* Get the next code. */
      work_data >>= code_size ;
      work_bits  -= code_size ;

      if (code == clr_code)                     /* Interpret the code. */
        {
          code_size  = root_size + 1 ;          /* reset decoder stream. */
          code_mask  = (1 << code_size) - 1 ;
          prev_code  = NULL_CODE ;
          table_size = eoi_code + 1 ;
        }
      else if (code == eoi_code)
        {
          return(GIFIN_ERR_EOF) ;               /* Oops! no more pixels. */
        }
      else if (prev_code == NULL_CODE)
        {
          gifin_push_string(code);
	  if (gif_fatal_error == 1)
	     return (GIFIN_ERR_PIXEL);
          prev_code = code ;
        }
      else
        {
          if (code < table_size)
            {
              first = gifin_push_string(code) ;
	      if (gif_fatal_error == 1)
		 return (GIFIN_ERR_PIXEL);
            }
          else
            {
              place = pstk_idx ;
/*
              PUSH_PIXEL(NULL_CODE) ;
*/
              if (push_pixel(NULL_CODE)  == -1)
	         return (GIFIN_ERR_PIXEL);
              first = gifin_push_string(prev_code) ;
	      if (gif_fatal_error == 1)
	 	 return (GIFIN_ERR_PIXEL);
              pstk[place] = first ;
            }

          gifin_add_string(prev_code, first) ;
          prev_code = code ;
        }
    }

  *pel = (int) pstk[--pstk_idx] ;    /* Pop a pixel off the pixel stack. */

  return(GIFIN_SUCCESS) ;
}


static int
gifin_close_file()      /* Close an open GIF file. */
{

/* Make sure there's a file open. */

  if (!file_open) return(GIFIN_ERR_NFO) ;

/* Mark file (and image) as closed. */

  file_open  = 0 ;
  image_open = 0 ;

  return(GIFIN_SUCCESS) ;
}


/* Load a colormap from the input stream. */

static int
gifin_load_cmap(cmap, ncolors)
BYTE cmap[3][256] ;
int ncolors ;
{
  int i ;

  for (i = 0; i < ncolors; i++)
    {
      if (im_fread((char *) buf, 1, 3) != 3) return(GIFIN_ERR_EOF) ;
    
      cmap[GIF_RED][i] = buf[GIF_RED] ;
      cmap[GIF_GRN][i] = buf[GIF_GRN] ;
      cmap[GIF_BLU][i] = buf[GIF_BLU] ;
    }

  return(GIFIN_SUCCESS) ;
}
 

/* Skip an extension block in the input stream. */

static int
gifin_skip_extension()
{
  int tmp;

  (void) im_fgetc () ;         /* Get the extension function byte. */

/* Skip any remaining raster data. */

  do
    {
      if (gifin_read_data_block() != GIFIN_SUCCESS)
        return(GIFIN_ERR_EOF) ;
    }
  while (buf_cnt > 0) ;

  return(GIFIN_SUCCESS) ;
}


/* Read a new data block from the input stream. */

static int
gifin_read_data_block()
{
  buf_cnt = im_fgetc () ;        /* Read the data block header. */

/* Read the data block body. */

  if (im_fread((char *) buf, 1, buf_cnt) != buf_cnt)
    return(GIFIN_ERR_EOF) ;

  buf_idx = 0 ;

  return(GIFIN_SUCCESS) ;
}


/*  Push a string (denoted by a code) onto the pixel stack
 *  (returns the code of the first pixel in the string).
 */

static int
gifin_push_string(code)
int code ;
{
  int rslt ;

  while (prefix[code] != NULL_CODE)
    {
      if (push_pixel(extnsn[code]) == -1)
	  return (0);
      code = prefix[code] ;
    }

  if (push_pixel(extnsn[code])  == -1)
     return (0);
  rslt = extnsn[code] ;

  return rslt ;
}


static void
gifin_add_string(p, e)      /* Add a new string to the string table. */
int p, e ;
{
  prefix[table_size] = p ;
  extnsn[table_size] = e ;

  if ((table_size == code_mask) && (code_size < 12))
    {
      code_size += 1 ;
      code_mask  = (1 << code_size) - 1 ;
    }

  table_size += 1 ;
}


static void
gifin_fatal(msg)        /* Semi-graceful fatal error mechanism. */
char *msg ;
{
  strcpy (gif_err_str, catgets (prog->catd, 1, 54,
				"Error reading GIF file: "));
  strcat (gif_err_str, msg);
  gif_fatal_error = 1 ;
}


int
gif_load(image)
ImageInfo	*image;
{
  int            pass, pixel, pixlen, scanlen, x, y ;
  unsigned char *pixptr, *pixline, *image_data;
  unsigned int   pixel_stride, scanline_stride;

  gif_fatal_error = 0 ;

  if (openfile(image->file, image->realfile, image->compression, image->data, 
	       image->file_size) != 0) 
      return(-1);

  if ((gifin_open_file()  != GIFIN_SUCCESS) ||   /* Read GIF header. */
      (gifin_open_image() != GIFIN_SUCCESS))     /* Read image header. */
    {
      gifin_close_file() ;
      closefile () ;
      return(-1);
    }

  
  image->width  = gifin_img_width ;
  image->height = gifin_img_height ;
  image->depth  = 8 ;

/*
 * Create the image and get the pointer to the data.
 */

  image_data = create_data (image, &pixel_stride, &scanline_stride);
  image->bytes_per_line = scanline_stride;

  pixlen = (image->depth / 8) + (image->depth % 8 ? 1 : 0) ;

  if (gifin_l_cmap_flag)
    {
      image->colors = gifin_l_ncolors;
      image->red = (unsigned char *) malloc (sizeof (unsigned char) * 
					     image->colors);
      image->green = (unsigned char *) malloc (sizeof (unsigned char) * 
					       image->colors);
      image->blue = (unsigned char *) malloc (sizeof (unsigned char) * 
					      image->colors);

      for (x = 0; x < image->colors; x++)
        {
          image->red [x]   = gifin_g_cmap[GIF_RED][x] ;
          image->green [x] = gifin_g_cmap[GIF_GRN][x] ;
          image->blue [x]  = gifin_g_cmap[GIF_BLU][x] ;
        }
    }

  else  {
      image->colors = gifin_g_ncolors ;
      image->red = (unsigned char *) malloc (sizeof (unsigned char) * 
					     image->colors);
      image->green = (unsigned char *) malloc (sizeof (unsigned char) * 
					       image->colors);
      image->blue = (unsigned char *) malloc (sizeof (unsigned char) * 
					      image->colors);

      for (x = 0; x < gifin_g_ncolors; x++)
        {
          image->red [x]   = gifin_g_cmap[GIF_RED][x] ;
          image->green [x] = gifin_g_cmap[GIF_GRN][x] ;
          image->blue [x]  = gifin_g_cmap[GIF_BLU][x] ;
        }
    }


/*  Interlaced image -- futz with the vertical trace. I wish I knew what
 *  kind of drugs the GIF people were on when they decided that they
 *  needed to support interlacing.
 */

  if (gifin_interlace_flag)
    {
      scanlen = image->height * pixlen ;

/*  Interlacing takes four passes to read, each starting at a different
 *  vertical point.
 */

      for (pass = 0; pass < 4; pass++)
        {
          y = interlace_start[pass] ;
          scanlen = image->bytes_per_line * pixlen * interlace_rate[pass] ;
          pixline = image_data + (y * image->bytes_per_line * pixlen) ;
          while (y < gifin_img_height)
            {
              pixptr = pixline ;
              for (x = 0; x < gifin_img_width; x++)
                {
                  if (gifin_get_pixel(&pixel) != GIFIN_SUCCESS)
                    {
                      if (gif_fatal_error)
                        {
      			  gifin_close_file() ;
      			  closefile () ;
                          return(-1);
                        }
                      else
                        {
                          sprintf(gif_err_str,
                                  catgets (prog->catd, 1, 55,
				    "%s: Short read within image data\n"),
                                  image->file) ;
                          x = gifin_img_width ;
                          y = gifin_img_height ;
                        }
                    }
                  *pixptr = pixel ;
                  pixptr++ ;
                }
              y += interlace_rate[pass] ;
              pixline += scanlen ;
            }
        }
    }

/* Not an interlaced image, just read in sequentially. */

  else
    {
      pixptr = image_data ;
      for (y = 0; y < gifin_img_height; y++)
        {
          for (x = 0; x < gifin_img_width; x++)
            {
              if (gifin_get_pixel(&pixel) != GIFIN_SUCCESS)
                {
                  if (gif_fatal_error)
                    {
      		      gifin_close_file() ;
      		      closefile () ;
                      return(-1);
                    }
                  else
                    { 
                      sprintf(gif_err_str,
                              catgets (prog->catd, 1, 56, "%s: Short read within image data\n"), 
			      image->file) ;
                      x = gifin_img_width ;
                      y = gifin_img_height ;
                    }
                }
              *pixptr = pixel ;
              pixptr++ ;
            }
          if (image->width != image->bytes_per_line)
            pixptr += (image->bytes_per_line - image->width) ;
        }
    }
  gifin_close_file() ;
  closefile () ;

    (*xil->import_ptr) (image->orig_image, TRUE);

  return(0) ;
}

int
gif_get_pixel (x, y)
int x,y;
{
    int r;
    unsigned char tmp;
    int isneg;
    
    tmp = gif_data [ (y * gif_info->bytes_per_line) + x];
    isneg =  ((tmp & 0x80) == 0x80);
    tmp &= 0x7f;
    
    r = (int) tmp;
    if (isneg == 1)
       r |= 0x0080;

    return (r);
}

int
gif_save (image)
ImageInfo	*image;
{
    int		 	 tmp_red [256];
    int		 	 tmp_green [256];
    int		 	 tmp_blue [256];
    int			 i;
    unsigned int         width, height, nbands;
    unsigned int         pixel_stride, scanline_stride;
    XilDataType		 datatype;
    
    gif_info = image;
    gif_data = retrieve_data (image->orig_image, &pixel_stride, 
			      &scanline_stride);

    gif_save_image = image->orig_image;

    for (i = 0; i < image->colors ; i++) {
	tmp_red [i] = (int) image->red [i];
	tmp_green [i] = (int) image->green [i];
	tmp_blue [i] = (int) image->blue [i];
	}

    for (i = image->colors ; i < 256; i++) {
	tmp_red [i] = 0;
	tmp_green [i] = 0;
	tmp_blue [i] = 0;
	}

    (*xil->get_info) (gif_save_image, &width, &height, &nbands, &datatype);

    GIFEncode (image->file, (int)width, (int)height,
		FALSE, 0, image->depth, tmp_red, tmp_green, tmp_blue,
		gif_get_pixel);

    reimport_image(image->orig_image, FALSE);

    return 0;
}
