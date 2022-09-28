#ifndef lint
static char sccsid[] = "@(#)image.c 1.19 96/05/29";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include "imagetool.h"
#include "image.h"
#include "display.h"
#include "ui_imagetool.h"
#include "state.h"

extern int     	 max_x;
extern int     	 max_y;
extern int     	 min_x;
extern int     	 min_y;

static XilLookup	cc_496 = NULL;
static XilLookup	cc_242 = NULL;
static XilLookup	lut_1to24 = NULL;
static XilLookup	lut_8to1 = NULL;
static XilLookup	lut_8to24 = NULL;
static XilKernel	distribution = NULL;
static XilLookup	lut_216 = NULL;
static XilLookup	lut_RGB = NULL;
static XilDitherMask	dmask_443 = NULL;


ImageInfo 	*current_image;

TypeInfo         all_types [] = {
{RASTER, "Sun-raster", "Sun Raster", "Sun_Raster", rast_load, rast_save, NULL},
{TIF, "TIFF", "TIFF", "TIFF", tiff_load, tiff_save, NULL},
{GIF, "GIF", "GIF", "GIF", gif_load, gif_save, NULL},
{POSTSCRIPT, "POSTSCRIPT", "PostScript", "PostScript", ps_load, ps_save,		NULL},
{EPSF, "epsf-file", "EPSF", "EPS", ps_load, epsf_save, NULL},
{PICT, "pict-file", "PICT", NULL, ppm_load, NULL, "/usr/openwin/bin/picttoppm"},
{XPM, "PM", "XPM", "XPM", xpm_load, xpm_save, NULL},
{ILBM, "ilbm-file", "IFF ILBM",	NULL, ppm_load, NULL,					"/usr/openwin/bin/ilbmtoppm"},
{PI1, "pi1-file", "Atari Degas 1", NULL, ppm_load, NULL,				"/usr/openwin/bin/pi1toppm"},
{PCX, "pcx-file", "PC Paintbrush", NULL, ppm_load, NULL,				"/usr/openwin/bin/pcxtoppm"},
{PPM, "ppm-file", "PPM", NULL, ppm_load, NULL, NULL},
{PGM, "pgm-file", "PGM", NULL, ppm_load, NULL, NULL},
{PBM, "pbm-file", "PBM", NULL, ppm_load, NULL, NULL},
{JFIF, "JPEG", "JFIF (JPEG)", "JPEG", jfif_load, jfif_save, NULL},
{PCD16, "photocd-file/16", "Kodak Photo CD (Base/16)", NULL, photocd_load, NULL, NULL},
{PCD4, "photocd-file/4", "Kodak Photo CD (Base/4)", NULL, photocd_load, NULL, NULL},
{PCD, "photocd-file", "Kodak Photo CD (Base)", "PhotoCD", photocd_load, NULL, NULL},
{PCDX4, "4photocd-file", "Kodak Photo CD (4Base)", NULL, photocd_load, NULL, NULL},
{TGA, "tga-file", "TrueVision Targa", NULL, ppm_load, NULL,				"/usr/openwin/bin/tgatoppm"},
{PJT, "pj-file", "HP PaintJet", NULL, ppm_load, NULL,					"/usr/openwin/bin/pjtoppm"},
{XIMF, "xim-file", "Xim", NULL, ppm_load, NULL,	"/usr/openwin/bin/ximtoppm"},
{MTV, "mtv-file", "MTV/PRT Ray-Tracer", NULL, ppm_load, NULL,				"/usr/openwin/bin/mtvtoppm"},
{QRT, "qrt-file", "QRT Ray-Tracer",  NULL, ppm_load, NULL,				"/usr/openwin/bin/qrttoppm"},
{IMGW, "imgw-file", "Img-Whatnot", NULL, ppm_load, NULL,				"/usr/openwin/bin/imgtoppm"},
{SLD, "sld-file", "AutoCAD Slide", NULL, ppm_load, NULL,				"/usr/openwin/bin/sldtoppm"},
{SPC, "spc-file", "Atari Compressed Spectrum", NULL, ppm_load, NULL,			"/usr/openwin/bin/spctoppm"},
{SPU, "spu-file", "Atari Uncompressed Spectrum", NULL, ppm_load, NULL,			"/usr/openwin/bin/sputoppm"},
{GOULD, "gould-file", "Gould Scanner", NULL, ppm_load, NULL,				"/usr/openwin/bin/gouldtoppm"},
{FS1, "fs-file", "Usenix FaceSaver", NULL, ppm_load, NULL,				"/usr/openwin/bin/fstopgm"},
{FITS, "fits-file", "FITS", NULL, ppm_load, NULL, "/usr/openwin/bin/fitstopgm"},
{LISPM, "lispm-file", "Lisp Machine Bit-Array", NULL, ppm_load, NULL,			"/usr/openwin/bin/lispmtopgm"},
{HIPS, "hips-file", "HIPS", NULL, ppm_load, NULL, "/usr/openwin/bin/hipstopgm"},
{MAC, "mac-file", "MacPaint", NULL, ppm_load, NULL,					"/usr/openwin/bin/macptopbm"},
{ATK, "atk-file", "Andrew Toolkit", NULL, ppm_load, NULL,				"/usr/openwin/bin/atktopbm"},
{BRUSH, "brush-file", "Xerox Doodle Brush", NULL, ppm_load, NULL,			"/usr/openwin/bin/brushtopbm"},
{CMU, "cmu-file", "CMU Window Manager", NULL, ppm_load, NULL,				"/usr/openwin/bin/cmuwmtopbm"},
{MGR, "mgr-file", "MGR", NULL, ppm_load, NULL,"/usr/openwin/bin/mgrtopbm"},
{G3F, "g3-file", "Group 3 FAX", NULL, ppm_load, NULL,					"/usr/openwin/bin/g3topbm"},
{IMG, "img-file", "GEM", NULL, ppm_load, NULL,"/usr/openwin/bin/gemtopbm"},
{SICON, "sun-icon-image", "Sun Icon", NULL, ppm_load, NULL,					"/usr/openwin/bin/icontopbm"},
{PI3, "pi3-file", "Atari Degas 3", NULL, ppm_load, NULL,				"/usr/openwin/bin/pi3topbm"},
{XBM, "BM", "X Bitmap", "XBM", xbm_load, NULL, 	NULL},
{YBM, "ybm-file", "Benet Yee Face File", NULL, ppm_load, NULL,				"/usr/openwin/bin/ybmtopbm"},
{XWD, "XWD", "X11 Window Dump", NULL, ppm_load, NULL, 				"/usr/openwin/bin/xwdtopnm"},
};

int 		 ntypes = sizeof all_types / sizeof all_types [0];

TypeInfo *
str_to_ftype( str )
char    *str;
{
    int  i;

    for (i = 0; i < ntypes; i++)
        if (strcmp (str, all_types [i].type_name) == 0)
           return (&all_types[i]);

    return ( (TypeInfo *) NULL);
}

ImageInfo *
init_image (file, realfile, file_size, type, data)
char            *file;
char		*realfile;
int              file_size;
TypeInfo        *type;
char		*data;
{
    int		 i;
    ImageInfo   *image = (ImageInfo *) calloc (1, sizeof (ImageInfo));
    int 	 len;
    extern	 void image_rotate_func ();
    extern	 void image_zoom_func ();
    extern	 void image_hflip_func ();
    extern	 void image_vflip_func ();
    extern	 void image_turnover_func ();
    extern	 void image_revert_func ();
    extern	 void ps_rotate_func ();
    extern	 void ps_zoom_func ();
    extern	 void ps_hflip_func ();
    extern	 void ps_vflip_func ();
    extern	 void ps_turnover_func ();
    extern	 void ps_revert_func ();
 
    image->type_info = type;
    if (file != (char *) NULL) {
       len = strlen (file);
       if ((image->file = (char*)malloc((len+1) * sizeof(char))) == NULL) {
	  if (prog->verbose)
            printf (catgets (prog->catd, 1, 71, "cannot malloc memory\n"));
          image->file = file;
          }
       else 
          strcpy(image->file, file);
       }
    else {
       len = strlen ("Untitled");
       if ((image->file = (char*)malloc((len+1) * sizeof(char))) == NULL) {
	  if (prog->verbose)
            printf (catgets (prog->catd, 1, 72, "cannot malloc memory\n"));
          image->file = file;
          }
       else 
          strcpy(image->file, "Untitled");
       }

    if ((realfile == (char *) NULL) ||
        (strcmp (image->file, realfile) == 0))
       image->realfile = image->file;
    else
       image->realfile = realfile;

    image->file_size = file_size;
    image->data = data;
 
    image->free_data = FALSE;
    image->width = 0;
    image->height = 0;
    image->nbands = 0;
    image->datatype = XIL_BYTE;
    image->depth = 0;
    image->bytes_per_line = 0;
    image->saving = 0;
    image->ditheredto216 = FALSE;
    image->dithered24to8 = FALSE;
    image->colortogray = FALSE;
    image->rgborder = 1;
    image->compression = NO_COMPRESS;
    image->red = (unsigned char *) NULL;
    image->green = (unsigned char *) NULL;
    image->blue = (unsigned char *) NULL;
    image->old_red = (unsigned char *) NULL;
    image->old_green = (unsigned char *) NULL;
    image->old_blue = (unsigned char *) NULL;
    image->colors_compressed = FALSE;
    image->cis = (XilCis) NULL;
    image->orig_image = (XilImage) NULL;
    image->old_image = (XilImage) NULL;
    image->dest_image = (XilImage) NULL;
    image->revert_image = (XilImage) NULL;
    image->view_image = (XilImage) NULL;
    image->cmap = (Colormap) NULL;
    image->colors = 0;
    image->old_colors = 0;
    image->offset = 0;
    image->pages = 0;
    image->orient = ORIENT_NONE; /* default to portrait */
    if ((type->type == POSTSCRIPT) || (type->type == EPSF)) {
       image->rotate_func = ps_rotate_func;
       image->zoom_func = ps_zoom_func;
       image->hflip_func = ps_hflip_func;
       image->vflip_func = ps_vflip_func;
       image->turnover_func = ps_turnover_func;
       image->revert_func = ps_revert_func;
       }
    else {
	image->rotate_func = image_rotate_func;
	image->zoom_func = image_zoom_func;
	image->hflip_func = image_hflip_func;
	image->vflip_func = image_vflip_func;
	image->turnover_func = image_turnover_func;
	image->revert_func = image_revert_func;
       }
  
    return (image);         
}

int
assign_display_func(image, depth)
    ImageInfo *image;
    int depth;
{
    XilDataType   datatype;
    unsigned int  width, height, nbands;
    unsigned int  pixel_stride, scanline_stride;
    unsigned char *image_data;
    XilImage image_1to24();
    XilImage image_1to8();
    XilImage image_1to1();
    XilImage image_8to24();
    XilImage image_8to8();
    XilImage image_copy8to8();
    XilImage image_8to4();
    XilImage image_8to1();
    XilImage image_24to24();
    XilImage image_24to8();
    XilImage image_24to4();
    void image_geo_op();

    if (image->saving && image->colortogray) {
	/* if we are saving an image which has been convert to gray scale
	 * we need to retrieve the original image which has been kept in
	 * the old_image field
	 */
	if (image->orig_image) {
	    (*xil->destroy) (image->orig_image);
	    image->orig_image = image->old_image;
	    image->old_image = NULL;
	}
	image_geo_op(image);
    }

    (*xil->get_info) (image->orig_image, &width, &height, &nbands, &datatype);

    switch (depth) {
    case 24:
	if (!image->saving) {
	    set_canvas_values();
	}
	if ((nbands == 1) && (datatype == XIL_BIT)) {

	    if (!lut_1to24) {
		Xil_unsigned8 data[6];

		data[0] = data[1] = data[2] = 255;
		data[3] = data[4] = data[5] = 0;

		lut_1to24 = (*xil->lookup_create) (image_display->state, XIL_BYTE,
			XIL_BYTE, 3, 2, 0, data);
	    }

	    image->display_func = image_1to24;

	} else if ((nbands == 1) && (datatype == XIL_BYTE)) {
	    int            i, j;
	    Xil_unsigned8  data[256*3];
	    float	   c;

	    if (image->saving) {
		if (image->ditheredto216) {
		    restore_colormap(image);
		    image->ditheredto216 = 0;
		} else {
		    c = (float)image->offset;
		    if (c) {
			(*xil->subtract_const) (image->dest_image, &c,
			    image->dest_image);
		    }
		}
		image->rgborder = 0;
	    }

	    for (j = 0, i = 0; i < image->colors; i++, j += 3) {
		data[j]   = (image->blue[i] << 8) + image->blue[i];
		data[j+1] = (image->green[i] << 8) + image->green[i];
		data[j+2] = (image->red[i] << 8) + image->red[i];
	    }

	    lut_RGB = (*xil->lookup_create) (image_display->state, XIL_BYTE,
		XIL_BYTE, 3, image->colors, 0, data);

	    image->offset = 0;

	    image->display_func = image_8to24;
	}
	else if ((nbands == 3) && (datatype == XIL_BYTE)) {
	    if (!image->saving) {
		unsigned int  pixel_stride, scanline_stride;
		unsigned char *image_data;

		/* make sure the image data is in bgr order
		 * so we can copy it to display
		 */
		if (image->rgborder) {
		    image_data = retrieve_data(image->orig_image,
			&pixel_stride, &scanline_stride);
		    swap_red_blue(image, image_data, pixel_stride,
			scanline_stride);
                    reimport_image(image->orig_image, TRUE);
		}
	    }
	    image->display_func = image_24to24;
	}
    break;

    case 8:
	if ((nbands == 1) && (datatype == XIL_BIT)) {

	    image->colors = 2;
	    image->offset = 0;
	    image->red = (unsigned char *) malloc (sizeof (unsigned char) * 
						 image->colors);
	    image->green = (unsigned char *) malloc (sizeof (unsigned char) * 
						   image->colors);
	    image->blue = (unsigned char *) malloc (sizeof (unsigned char) * 
						  image->colors);
	    image->red[0] = image->green[0] = image->blue[0] = 255; /* White. */
	    image->red[1] = image->green[1] = image->blue[1] = 0;   /* Black. */

	    image_display->white[0] = 0.0;
	    image_display->black = 1;

	    image->display_func = image_1to8;

	} else if ((nbands == 1) && (datatype == XIL_BYTE)) {
	    XilImage      tmp_image;
	    float         mult[1], offset[1];
	    unsigned int  width, height, nbands;
	    XilDataType   datatype;
	    int           save_width, save_height;
	    float	  c;

	    if (image->saving) {
		if (image->ditheredto216) {
		    restore_colormap(image);
		    image->ditheredto216 = 0;
		} else {
		    c = (float)image->offset;
		    if (c) {
			(*xil->subtract_const) (image->dest_image, &c,
			    image->dest_image);
		    }
		}
		image->display_func = image_copy8to8;
	    } else {
		/*
		 * The default is to compress 8-bit image to no more than 
		 * 216 colors unless resources says not to.
		 * Change later check props resource.
		 */
		save_colormap(image);
		  compress_colors(image);

		if (image->colors > 216 && !image->saving) {
		    unsigned int          width, height;
		    XilImage              tmp;
		    XilLookup             cmap;
		    XilLookup		src_lut, dst_lut;
		    XilMemoryStorageByte	storage;
		    Xil_unsigned8	        cmap_data[256*3];
		    int i, j = 0;
		    short start;
		    unsigned int num_entries;

		    save_colormap (image);
		    image->ditheredto216 = TRUE;
		    width = (*xil->get_width) (image->orig_image);
		    height = (*xil->get_height) (image->orig_image);

		    /*
		    * Copy current image colors into cmap_data.
		    */
		    for (i = 0; i < image->colors; i++) {
			cmap_data[j] = image->red[i];
			cmap_data[j+1] = image->green[i];
			cmap_data[j+2] = image->blue[i];
			j += 3;
		    }
		    tmp = (*xil->create) (image_display->state, image->colors,
			    1, 3, XIL_BYTE);
		    (*xil->export_ptr) (tmp);
		    (*xil->get_memory_storage) (tmp, (XilMemoryStorage *)&storage);
		    storage.data = cmap_data;
		    (*xil->set_memory_storage) (tmp, (XilMemoryStorage *)&storage);
		    (*xil->import_ptr) (tmp, 1);

		    src_lut = (*xil->lookup_create) (image_display->state, XIL_BYTE,
			XIL_BYTE, 3, image->colors, 0, cmap_data);
		    dst_lut = (*xil->choose_colormap) (tmp, 216);
		    lut_216 = (*xil->lookup_convert) (src_lut, dst_lut); /* 0-216 */ 

		    (*xil->lookup_destroy) (src_lut);
		    (*xil->destroy) (tmp);

		    image->colors = 216;
		    image->offset = 40;

		    /* use dst_lut for display */
		    set_cmap8to216 (current_display->xdisplay,
			current_display->win, dst_lut, image, 8, 0);
		    (*xil->lookup_destroy) (dst_lut);

		    start = (*xil->lookup_get_offset) (lut_216);
		    num_entries = (*xil->lookup_get_num_entries) (lut_216);

		    (*xil->lookup_get_values) (lut_216, start, num_entries,
			    cmap_data);

		    for (i = 0; i < num_entries; i++) {
			cmap_data[i] += 40;
		    }

		    (*xil->lookup_set_values) (lut_216, start, num_entries, cmap_data);
		} else {
		    c = (float)image->offset;
		    (*xil->add_const) (image->orig_image, &c, image->orig_image);
		    if (current_display->depth == 8 && !image->saving) {
			set_cmap8 (current_display->xdisplay,
			    image_display->win, image, 8);
		    }
		}

		image->display_func = image_8to8;
	    }
	} else if ((nbands == 3) && (datatype == XIL_BYTE)) {
	    void set_cmap24to8();

	    /* We need to rescale the image by offset if we are saving it */
	    if (image->saving) {
		image->dithered24to8 = 1;
	    }

	    if (dmask_443 == NULL) {
		dmask_443 = (*xil->dithermask_get_by_name) (image_display->state,
		    "dm443");
	    }

	    if (cc_496 == NULL) {
		cc_496 = (*xil->lookup_get_by_name) (image_display->state,
		    "cc496");
	    }

	    /* set_cmap24to8 will install the colormap differently
	     * based on the channel order of the image
	     * if the current display is not 8 bit, this will just
	     * copy the lookup table data into image's red, green, blue.
	     */
	    set_cmap24to8(current_display->xdisplay, current_display->win,
		cc_496, image, 8);

	    image->display_func = image_24to8;
	} 
    break;

    case 4:
	if ((nbands == 1) && (datatype == XIL_BIT)) {
	    image->display_func = image_1to8;
	} else if ((nbands == 1) && (datatype == XIL_BYTE)) {
	    int		    i, j;
	    char	    data[256 * 3];
	    int             multipliers[3];
	    unsigned int    dimensions[3];

	    if (dmask_443 == NULL) {
		dmask_443 = (*xil->dithermask_get_by_name) (image_display->state,
		    "dm443");
	    }
	    if (cc_242 == NULL) {
		dimensions[0] = 2;  dimensions[1] = 4; dimensions[2] = 2;
		multipliers[0] = 1; multipliers[1] = 2; multipliers[2] = 8;
		cc_242 = (*xil->colorcube_create) (image_display->state,
		    datatype, datatype, 3, 0, multipliers, dimensions);
	    }
	    if (lut_8to24 == NULL) {
		for (j = 0, i = 0; i < image->colors; i++, j += 3) {
		    data[j]   = image->blue[i];
		    data[j+1] = image->green[i];
		    data[j+2] = image->red[i];
		}
		lut_8to24 = (*xil->lookup_create) (image_display->state, XIL_BYTE,
			XIL_BYTE, 3, image->colors, 0, data);
	    }
	    /* after the image goes through lut_8to24, it's in bgr order */
	    image->rgborder = 0;

	    if (current_display->depth == 4) {
		set_cmap4 (current_display->xdisplay, current_display->win,
		    cc_242, image);
	    }
	    image->display_func = image_8to4;
	} else if ((nbands == 3) && (datatype == XIL_BYTE)) {
	    int              multipliers[3];
	    unsigned int     dimensions[3];

	    if (dmask_443 == NULL) {
		dmask_443 = (*xil->dithermask_get_by_name) (image_display->state,
		    "dm443");
	    }
	    if (cc_242 == NULL) {
		dimensions[0] = 2;  dimensions[1] = 4; dimensions[2] = 2;
		multipliers[0] = 1; multipliers[1] = 2; multipliers[2] = 8;
		cc_242 = (*xil->colorcube_create) (image_display->state,
		    datatype, datatype, 3, 0, multipliers, dimensions);
	    }

	    if (current_display->depth == 4) {
		set_cmap4 (current_display->xdisplay, current_display->win,
		    cc_242, image);
	    }
	    image->display_func = image_24to4;
	}
    break;

    case 1:
    if ((nbands == 1) && (datatype == XIL_BIT)) {  
	image->colors = 0;
	image->offset = 0;
	image->display_func = image_1to1;
    }
    else if ((nbands == 1) && (datatype == XIL_BYTE)) {
	if (!distribution) {
	    distribution = (*xil->kernel_get_by_name) (image_display->state,
		"floyd-steinberg");
	}
	if (!lut_8to1) {
	    int			multipliers[1] = {-1};
	    unsigned int	dimensions[1] = {2};

	    lut_8to1 = (*xil->colorcube_create)(image_display->state, XIL_BIT,
		XIL_BYTE, 1, 0, multipliers, dimensions);
	}

	color_to_gray(image);

	/* applied affine matrix if saving to a file */
	if (image->saving) {
	    image_geo_op(image);
	}
	image->display_func = image_8to1;
    }
    else if ((nbands == 3) && (datatype == XIL_BYTE)) {
	if (!distribution) {
	    distribution = (*xil->kernel_get_by_name)(image_display->state,
		"floyd-steinberg");
	}
	if (!lut_8to1) {
	    int			multipliers[1] = {-1};
	    unsigned int	dimensions[1] = {2};

	    lut_8to1 = (*xil->colorcube_create) (image_display->state, XIL_BIT,
		XIL_BYTE, 1, 0, multipliers, dimensions);
	}

	color_to_gray(image);

	/* applied affine matrix if saving to a file */
	if (image->saving) {
	    image_geo_op(image);
	}
	image->display_func = image_8to1;
    }
    break;

    default:
    if (prog->verbose)
      fprintf (stderr, catgets (prog->catd, 1, 73, 
	       "%s: %d depth is not supported.\n"), prog->name,
	       image_display->depth);
    break;
    } 
    if (image->display_func == NULL)
	return -1;
    else
        return 0;
}

void
copy_image_data (new_image, old_image)
ImageInfo	*new_image;
ImageInfo	*old_image;
{
    int	   	  i;
    unsigned int  width, height, nbands;
    XilDataType   datatype;
    int           size;
    float         zoom;

/*
 * If an 8-bit image was dithered to 4 or 1, restore the colormap.
 * because we may want to save to 8-bit and colors have been reduced.
 * Else, just copy what's in current image.
 */
    if ((old_image->depth == 8) &&
        (current_display->depth == 4) || (current_display->depth == 1)) {
	restore_colormap (old_image);
    } else if (old_image->ditheredto216) {
	restore_colormap (old_image);
    }

    new_image->colors = old_image->colors;
    new_image->red = (unsigned char *) malloc (sizeof (unsigned char) *
						new_image->colors);
    new_image->green = (unsigned char *) malloc (sizeof (unsigned char) *
						  new_image->colors);
    new_image->blue = (unsigned char *) malloc (sizeof (unsigned char) *
						 new_image->colors);

    memcpy(new_image->red, old_image->red, new_image->colors);
    memcpy(new_image->green, old_image->green, new_image->colors);
    memcpy(new_image->blue, old_image->blue, new_image->colors);
    new_image->offset = old_image->offset;
     
    if (current_display == image_display)
      (*xil->get_info) (current_image->orig_image, &width, &height,
                    &nbands, &datatype);
    else {
       zoom = (float) prog->def_ps_zoom / 100.0;
       height = (int) (ps_display->pageheight * ps_display->res_y * 
			      zoom);
       width = (int) (ps_display->pagewidth * ps_display->res_x * zoom);
    }
    new_image->width = width;
    new_image->height = height;
    new_image->nbands = old_image->nbands;
/*
    new_image->cmap = old_image->cmap;
*/
    new_image->rgborder = old_image->rgborder;
    new_image->colors_compressed = old_image->colors_compressed;
    new_image->bytes_per_line = old_image->bytes_per_line;

}

void
destroy_image (image)
ImageInfo       *image;
{
     
    if (image->red != (unsigned char *) NULL) {
       free (image->red);
       image->red = NULL;
    }
 
    if (image->green != (unsigned char *) NULL) {
       free (image->green);
       image->green = NULL;
    }
 
    if (image->blue != (unsigned char *) NULL) {
       free (image->blue);
       image->blue = NULL;
    }
     
    if (image->old_red != (unsigned char *) NULL) {
       free (image->old_red);
       image->old_red = NULL;
    }
 
    if (image->old_green != (unsigned char *) NULL) {
       free (image->old_green);
       image->old_green = NULL;
    }
 
    if (image->old_blue != (unsigned char *) NULL) {
       free (image->old_blue);
       image->old_blue = NULL;
    }
 
    if (image->orig_image != (XilImage) NULL) {
      (*xil->destroy) (image->orig_image);
      image->orig_image = NULL;
    }
 
    if (image->dest_image != (XilImage) NULL) {
      (*xil->destroy) (image->dest_image);
      image->dest_image = NULL;
    }
 
    if (image->revert_image != (XilImage) NULL) {
      (*xil->destroy) (image->revert_image);
      image->revert_image = NULL;
    }
 
    if (image->view_image != (XilImage) NULL) {
      (*xil->destroy) (image->view_image);
      image->view_image = NULL;
    }

    if ((image->data != (char *) NULL) && (image->free_data == TRUE)) {
       free (image->data);
       image->data = NULL;
    }

    if ((image->type_info->type != POSTSCRIPT) && 
	(image->type_info->type != EPSF)) 
       if (image->cmap ) {
          XFreeColormap (current_display->xdisplay, image->cmap);
	  image->cmap = NULL;
       }
 
    if (image->cis) {
       (*xil->cis_destroy) (image->cis);
       image->cis = NULL;
    }

    if (image->file) {
       free(image->file);
       image->file = NULL;
    }

    free (image);
    image = NULL;
}

void
check_canvas (width, height, dsp)
     int                 width;
     int                 height;
     DisplayInfo	*dsp;
{
    Dimension   canvas_height = 0, canvas_width = 0;
    Dimension   disp_height, disp_width; 
    Dimension   sw_height, sw_width; 
    int         max_width, max_height;

/*
    XtVaGetValues (base->scrolledw, XmNheight, &sw_height,
		                    XmNwidth, &sw_width, NULL);
    disp_height = sw_height - base->sb_width;
    disp_width = sw_width - base->sb_width;
*/

    XtVaGetValues (current_display->new_canvas, XmNwidth, &canvas_width,
		                                XmNheight, &canvas_height, NULL);
    disp_width = canvas_width;
    disp_height = canvas_height;

    if ((int) (width + current_state->currentx) < (int) disp_width) 
       current_state->width_size = SMALLER;
    else if ((int) (width + current_state->currentx) > (int) disp_width)
       current_state->width_size = LARGER;
    else
       current_state->width_size = MIDDLE;

    if ((int)(height + current_state->currenty) < (int) disp_height) 
       current_state->height_size = SMALLER;
    else if ((int) (height + current_state->currenty) > (int) disp_height)
       current_state->height_size = LARGER;
    else 
       current_state->height_size = MIDDLE;
    
    if (current_state->width_size == LARGER)  {
       max_x = 0;
       min_x = disp_width - width;
      }
    else if (current_state->width_size == MIDDLE) {
       max_x = current_state->currentx;
       min_x = 0;
       }
    else {
       min_x = 0;
       max_x = canvas_width - width;
       }

    if (current_state->height_size == LARGER) {
      max_y = 0;
      min_y = disp_height - height;
    }
    else if (current_state->height_size == MIDDLE) {
      max_y = current_state->currenty;
      min_y = 0;
    }
    else { 
      min_y = 0;
      max_y = canvas_height - height;
    }

    if (current_image->width > canvas_width)
      max_width = current_image->width;
    else if (canvas_width < disp_width)
      max_width = disp_width;
    else
      max_width = canvas_width;

    if (current_image->height > canvas_height)
      max_height = current_image->height;
    else if (canvas_height < disp_height)
      max_height = disp_height;
    else
      max_height = canvas_height;

    XtVaSetValues (base->hscroll, XmNmaximum, max_width,
		                  XmNsliderSize, (int) disp_width,
		                  XmNvalue, 0, NULL);
    XtVaSetValues (base->vscroll, XmNmaximum, max_height,
		                  XmNsliderSize, (int) disp_height,
		                  XmNvalue, 0, NULL);
    dsp->width = (int) canvas_width;
    dsp->height = (int) canvas_height;

}

void
ScrollBarCallback (w, client_data, cbs)
     Widget                     w;
     XtPointer                  client_data;
     XmScrollBarCallbackStruct *cbs;
{
     unsigned char orientation = (unsigned char) client_data;

     if (current_state == NULL)
       return;

     if (orientation == XmVERTICAL) 
       current_state->currenty = -(cbs->value);
     else 
       current_state->currentx = -(cbs->value);

/*
 * Set this flag so that the background will not be repainted.
 */
     current_state->scrollbar_update = True;
     update_image ();
}


void
adjust_scrollbars ()
{
     int        value, max, size;

     if (current_state->width_size == LARGER) 
       XtVaSetValues (base->hscroll, XmNvalue, -current_state->currentx, NULL);

     if (current_state->height_size == LARGER) 
       XtVaSetValues (base->vscroll, XmNvalue, -current_state->currenty, NULL);
}
 
static void
ShowScrollbars ()
{
    Dimension         width, height;
    Dimension         disp_height, disp_width;
    Boolean	      hsb_attached = False;
    Boolean	      vsb_attached = False;

    DetachScrollbar (base->vscroll);
    DetachScrollbar (base->hscroll);
      /*
       * Determine the height/width of the viewing area:
       */
    if (current_display && current_display->new_canvas) {
      XtVaGetValues (current_display->new_canvas, 
		     XmNheight, &disp_height,
		     XmNwidth, &disp_width, 
		     NULL);
    }
    else 
      return;
      
      /*
       * Determine if scrollbars are needed. 
       */
      if (disp_height >= current_image->height) 
	DetachScrollbar (base->vscroll);
      else {
	AttachScrollbar (base->vscroll);
	disp_width -= base->sb_width;
      }

      if (disp_width >= current_image->width) {
	DetachScrollbar (base->hscroll);
      }
      else {
	AttachScrollbar (base->hscroll);
	disp_height -= base->sb_width;
      }
      /*
       * Set top left back to 0
       */
      XtVaSetValues (base->vscroll, 
		     XmNvalue, 0, 
		     XmNmaximum, (disp_height > current_image->height) ?
		                  disp_height : current_image->height,
		     XmNsliderSize, disp_height, 
		     NULL);

      XtVaSetValues (base->hscroll, 
		     XmNvalue, 0, 
		     XmNmaximum, (disp_width > current_image->width) ?
		                  disp_width : current_image->width,
		     XmNsliderSize, disp_width,
		     NULL);

}

void
display_new_image ()
{
    float *xform;
    unsigned int w, h, nbands;
    XilDataType datatype;
    int dx1, dx2, dy1, dy2;
    float background[3];
    float tmp;
    XmDrawingAreaCallbackStruct cbs;
    XEvent event;
    float x, y;

    xform = current_state->xform;

    if (current_display == ps_display) {
      check_canvas (current_image->width, current_image->height, current_display);
    } else {
	w = (*xil->get_width) (current_image->orig_image);
	h = (*xil->get_height) (current_image->orig_image);
	dx1 = abs(w*xform[0]-h*xform[2]);
	dy1 = abs(w*xform[1]-h*xform[3]);
	dx2 = abs(w*xform[0]+h*xform[2]);
	dy2 = abs(w*xform[1]+h*xform[3]);

	current_image->width = (dx1 > dx2 ? dx1 : dx2);
	current_image->height = (dy1 > dy2 ? dy1 : dy2);

	resize_canvas();

	/* create the dest image */
	if (current_image->dest_image) {
	    (*xil->destroy) (current_image->dest_image);
	}
	current_image->dest_image = (*xil->create) (image_display->state,
		current_image->width, current_image->height,
		(*xil->get_nbands) (current_image->orig_image),
		(*xil->get_datatype) (current_image->orig_image));
	if (!current_image->dest_image) {
	    if (prog->verbose)
		fprintf (stderr, catgets (prog->catd, 1, 74, 
				 "%s: xil_create() failed.\n"), prog->name);
	    return;
	}

	check_canvas (current_image->width, current_image->height, current_display);

	tmp = xform[1] * xform[2];
	/* if tmp != 0.0 or 1.0 or -1.0 then there is a
	 * rotation component in the xform matrix
	 */
	if (tmp != 0.0 && tmp != 1.0 && tmp != -1.0) {
	    if ((*xil->get_nbands) (current_image->dest_image) == 1) {
		background[0] = 1.0;
	    } else {
		background[0] = background[1] = background[2] = 0.0;
	    }
	    (*xil->set_value) (current_image->dest_image, background);
	}

	x = xil->get_width (current_image->orig_image) / 2.0;
	y = xil->get_height (current_image->orig_image) / 2.0;
	(*xil->set_origin) (current_image->orig_image, x, y);

	x = xil->get_width (current_image->dest_image) / 2.0;
	y = xil->get_height (current_image->dest_image) / 2.0;
	(*xil->set_origin) (current_image->dest_image, x, y);

	/* perform the affine operation */
	(*xil->affine) (current_image->orig_image, current_image->dest_image,
		"nearest", xform);

	(*xil->set_origin) (current_image->orig_image, 0.0, 0.0);
	(*xil->set_origin) (current_image->dest_image, 0.0, 0.0);

	if (current_image->view_image) {
	    (*xil->destroy) (current_image->view_image);
	}
	current_image->view_image =
		(current_image->display_func)(current_image->dest_image);
    }

    ShowScrollbars ();
    event.type = Expose;
    event.xexpose.x = 0;
    event.xexpose.y = 0;
    event.xexpose.width = current_display->width;
    event.xexpose.height = current_display->height;
    cbs.event = &event;
    cbs.reason = XmCR_EXPOSE;

    CanvasRepaintCallback (current_display->new_canvas, NULL, &cbs);
    if (print_prev && print_prev->showing == True) 
	position_image();
    
}

XilImage
image_1to8(src)
  XilImage src;
{
    unsigned int  width, height, nbands;
    XilDataType   datatype;
    XilImage      tmp_image;

    (*xil->get_info) (src, &width, &height, &nbands, &datatype);
    /*
     * First convert 1-band XIL_BIT to 1-band XIL_BYTE.
     */
    tmp_image = (*xil->create) (image_display->state, width, height, 1, XIL_BYTE);
    (*xil->cast) (src, tmp_image);

    return (tmp_image);
}

XilImage
image_1to24(src)
  XilImage src;
{
    unsigned int  width, height, nbands;
    XilDataType   datatype;
    XilImage      tmp_image;
    XilImage      tmp_image2;

    (*xil->get_info) (src, &width, &height, &nbands, &datatype);
    /*
     * First convert 1-band XIL_BIT to 1-band XIL_BYTE.
     */
    tmp_image = (*xil->create) (image_display->state, width, height, 1, XIL_BYTE);
    (*xil->cast) (src, tmp_image);

    tmp_image2 = (*xil->create) (image_display->state, width, height, 3, XIL_BYTE);
    (*xil->lookup) (tmp_image, tmp_image2, lut_1to24);
    (*xil->destroy) (tmp_image);

    return (tmp_image2);
}

XilImage
image_8to24 (src)
  XilImage src;
{
    XilImage        tmp_image;

    tmp_image = (*xil->create) (image_display->state,
	(*xil->get_width) (src), (*xil->get_height) (src), 3, XIL_BYTE);
    (*xil->lookup) (src, tmp_image, lut_RGB);
    return (tmp_image);
}

XilImage
image_24to24 (src)
  XilImage src;
{
    XilImage        tmp_image;

    tmp_image = (*xil->create_copy) (src, 0, 0, (*xil->get_width) (src),
	(*xil->get_height) (src), 0, (*xil->get_nbands) (src));
    return(tmp_image);
}

XilImage
image_copy8to8 (src)
  XilImage 	src;
{
    XilImage      tmp_image;

    tmp_image = (*xil->create_copy) (src,
	0, 0, (*xil->get_width)(src), (*xil->get_height) (src),
	0, (*xil->get_nbands) (src));

    return (tmp_image);
}

XilImage
image_8to8 (src)
  XilImage 	src;
{
    XilImage      tmp_image, dst;
    float         mult[1], offset[1];
    unsigned int  width, height;

    width = (*xil->get_width) (src);
    height = (*xil->get_height) (src);

    if (current_image->ditheredto216) {
	tmp_image = (*xil->create) (image_display->state, width, height,
	    1, XIL_BYTE);
	(*xil->lookup) (src, tmp_image, lut_216);
    } else {
	tmp_image = (*xil->create_copy) (src, 0, 0,
	    width, height, 0, (*xil->get_nbands) (src));
    }

    return (tmp_image);
}

XilImage
image_24to8 (src)
  XilImage src;
{
    unsigned int    width, height, nbands;
    XilDataType     datatype;
    XilImage        tmp_image;

    if (cc_496 == NULL)
        cc_496 = (*xil->lookup_get_by_name) (image_display->state, "cc496");

    tmp_image = (*xil->create) (image_display->state,
	(*xil->get_width) (src), (*xil->get_height) (src), 1, XIL_BYTE);
    (*xil->ordered_dither) (src, tmp_image, cc_496, dmask_443);
    return(tmp_image);
}

XilImage
image_1to1 (src)
  XilImage src;
{
    XilImage        tmp_image;

    tmp_image = (*xil->create_copy) (src, 0, 0, (*xil->get_width) (src),
	(*xil->get_height) (src), 0, (*xil->get_nbands) (src));
    return(tmp_image);
}

XilImage
image_8to1 (src)
  XilImage src;
{
    XilImage      tmp_image;

    if (!distribution) {
      distribution = (*xil->kernel_get_by_name) (image_display->state,
                "floyd-steinberg");
    }
    if (!lut_8to1) {
      int                 multipliers[1] = {-1};
      unsigned int        dimensions[1] = {2};
 
      lut_8to1 = (*xil->colorcube_create)(image_display->state, XIL_BIT,
                XIL_BYTE, 1, 0, multipliers, dimensions);
    }

    tmp_image = (*xil->create) (image_display->state,
	(*xil->get_width) (src), (*xil->get_height) (src), 1, XIL_BIT);
    (*xil->error_diffusion) (src, tmp_image, lut_8to1, distribution);
    return (tmp_image);
}

XilImage
image_8to4 (src)
  XilImage src;
{
    unsigned int    width, height, nbands;
    XilDataType     datatype;
    XilImage        tmp_image, rgb_image;

    rgb_image = (*xil->create) (image_display->state,
	(*xil->get_width) (src), (*xil->get_height) (src), 3, XIL_BYTE);
    (*xil->lookup) (src, rgb_image, lut_8to24);
    tmp_image = (*xil->create) (image_display->state,
	(*xil->get_width) (src), (*xil->get_height) (src), 1, XIL_BYTE);
    (*xil->ordered_dither) (rgb_image, tmp_image, cc_242, dmask_443);
    (*xil->destroy) (rgb_image);
    return(tmp_image);
}

XilImage
image_24to4 (src)
  XilImage src;
{
    unsigned int    width, height, nbands;
    XilDataType     datatype;
    XilImage        tmp_image;

    tmp_image = (*xil->create) (image_display->state,
	(*xil->get_width) (src), (*xil->get_height) (src), 1, XIL_BYTE);
    (*xil->ordered_dither) (src, tmp_image, cc_242, dmask_443);
    return(tmp_image);
}

/*
 * linelen function... Determine linelen required for xil image.
 */

int
linelen (width, depth)
int width;
int depth;
{
    if (depth == 1) 
       return ((width / 8) + ((width % 8 == 0) ? 0 : 1));
    else if ((depth == 8) || (depth == 4))
       return (width);
    else /* depth > 8  */
       return (width * depth / 8);

}

/*
 * Create the image and return pointer to data.
 */
unsigned char *
create_data (image, pixel_stride, scanline_stride)
  ImageInfo     *image;
  unsigned int  *pixel_stride;
  unsigned int  *scanline_stride;
{
  XilMemoryStorage  storage;
  Xil_boolean       success;
  XilDataType       datatype;
  u_char           *ptr;

  datatype = image->depth == 1 ? XIL_BIT : XIL_BYTE;

  image->orig_image = (*xil->create) (image_display->state, image->width, 
			image->height, image->depth >= 24 ? 3 : 1, datatype);
  if (!image->orig_image) {
    if (prog->verbose)
      fprintf (stderr, catgets (prog->catd, 1, 75,
				"%s: xil_create() failed.\n"), prog->name);
    return (NULL);
  }
  
  (*xil->export_ptr) (image->orig_image);
  success = (*xil->get_memory_storage) (image->orig_image, &storage);
  if (success == FALSE) {
    if (prog->verbose)
      fprintf (stderr, 
	       catgets (prog->catd, 1, 76, 
			"%s.create_data: xil_get_memory_storage() failed.\n"), 
	       prog->name);
    return (NULL);
  }

  if ( datatype == XIL_BYTE ) {
    ptr = (unsigned char *) storage.byte.data;
    *pixel_stride = storage.byte.pixel_stride;
    *scanline_stride = storage.byte.scanline_stride; 
  }
  else {
    ptr =  (unsigned char *) storage.bit.data; 
    *pixel_stride = storage.bit.band_stride;
    *scanline_stride = storage.bit.scanline_stride; 
  }
 
  return ptr;

}

/*
 * Retrieve the data pointer from the XilImage
 */
unsigned char *
retrieve_data (image, pixel_stride, scanline_stride)
  XilImage       image;
  unsigned int  *pixel_stride;
  unsigned int  *scanline_stride;
{
  XilMemoryStorage  storage;
  Xil_boolean       success;
  XilDataType       datatype;
  unsigned int      width, height, nbands;
  u_char           *ptr;

  (*xil->get_info) (image, &width, &height, &nbands, &datatype);
  (*xil->export_ptr) (image);

  success = (*xil->get_memory_storage) (image, &storage);
  if (success == FALSE) {
    if (prog->verbose)
      fprintf (stderr, 
	       catgets (prog->catd, 1, 77, 
		      "%s.retrieve_data: xil_get_memory_storage() failed.\n"), 
	       prog->name);
    return (NULL);
  }

  if (datatype == XIL_BYTE) {
    ptr = (u_char *) storage.byte.data;
    *pixel_stride = storage.byte.pixel_stride;
    *scanline_stride = storage.byte.scanline_stride;
  }
  else if (datatype == XIL_BIT) {
    ptr = (u_char *) storage.bit.data;
    *pixel_stride = storage.bit.band_stride;
    *scanline_stride = storage.bit.scanline_stride; 
  }

  /*
   * jlf - 2/28/96
   *
   * THIS IS A *MAJOR* BUG.
   *
   * Routines use the pointer returned by retrieve_data.  The pointer is only
   * vaild as long as the image is exported.  As soon as the image is
   * imported, the pointer is no longer valid!  So, none of the operations
   * performed on ptr actually have an effect on the image data.
   *
   * (*xil->import_ptr) (image, FALSE);
   */

  return (ptr);

}

/*
 * Reimport the data after it's been aquired by retrieve data and
 *   then modified.
 */
void
reimport_image(image, data_modified)
  XilImage       image;
  int            data_modified;
{
    (*xil->import_ptr) (image, data_modified);
}


/*
 * Retrieve the depth
 */
int
retrieve_depth (nbands, datatype)
  int          nbands;
  XilDataType  datatype;
{
  int  depth;

  if ((nbands == 1) && (datatype == XIL_BYTE))
    depth = 8;
  else if ((nbands == 1) && (datatype == XIL_BIT))
    depth = 1;
  else if ((nbands == 3) && (datatype == XIL_BYTE))
    depth = 24;

  return (depth);
}

/*
 * This routine removes unused colormap entries,
 * no information is lost in this process.
 */
void
compress_colors (image)
ImageInfo *image;
{
    int length, i;
    unsigned char *iptr, *tmpptr;
    unsigned long color;
    unsigned long *pixel;
    unsigned int *used;
    unsigned int a, x, y;
    int               new_cmapused = 0;
    unsigned int       width, height, nbands;
    unsigned int       pixel_stride, scanline_stride;
    XilDataType        datatype;  
    XilMemoryStorage   storage;
    Xil_boolean        success;
    unsigned char     *image_data;
    float		offset;

    if (image->colors_compressed == TRUE)
	return;
    if ((image->red == NULL)||(image->green == NULL)||(image->blue == NULL)) 
	return;

    (*xil->get_info) (image->orig_image, &width, &height, &nbands, &datatype);
    (*xil->export_ptr) (image->orig_image);
    success = (*xil->get_memory_storage) (image->orig_image, &storage);
    if (success == FALSE) {
      if (prog->verbose)
        fprintf (stderr, 
		 catgets (prog->catd, 1, 78, 
		    "%s.compress_colors: xil_get_memory_storage failed.\n"),
		 prog->name);
      return;
    }
    if (datatype == XIL_BYTE) {
	image_data = (u_char *)storage.byte.data;
	pixel_stride = storage.byte.pixel_stride;
	scanline_stride = storage.byte.scanline_stride;
    } else {
	/* NOTE: should not get here */
	exit(1);
    }

    new_cmapused = 0;

    pixel = (unsigned long *) malloc(sizeof(unsigned long) * image->colors) ;
    used  = (unsigned int *)  malloc(sizeof(unsigned int)  * image->colors) ;
    iptr  = (unsigned char *) image_data ;

    for (x = 0; x < image->colors; x++)
	*(used + x) = NULL;

    if (image->colors > 0) {
      for (y = 0; y < image->height; y++) {
	tmpptr = iptr;
	for (x = 0; x < image->width; x++) {
	    color = *tmpptr;
	    if (*(used + color) == 0) {
		for (a = 0; a < new_cmapused; a++) {
		    if ((*(image->red  +a) == *(image->old_red  +color)) &&
			(*(image->green+a) == *(image->old_green+color)) &&
			(*(image->blue +a) == *(image->old_blue +color)))
			break ;
		    }
		*(pixel + color) = a ;
		*(used + color)  = 1 ;
		if (a == new_cmapused) {
		    *(image->red   + a) = *(image->old_red   + color) ;
		    *(image->green + a) = *(image->old_green + color) ;
		    *(image->blue  + a) = *(image->old_blue  + color) ;
		    new_cmapused++;
                }
            }
	    *tmpptr = (unsigned char)pixel[(unsigned int)color] ;
	    tmpptr++ ;
        }
	iptr += scanline_stride;
      }
    }

    image->colors = new_cmapused;
    image->offset = MAXCOLORS - image->colors;
    free(used);
    free(pixel);

    image->colors_compressed = TRUE;
    (*xil->import_ptr) (image->orig_image, TRUE);
}

#ifdef NEVER
/*
 * Copy revert_image to view_image.
 */
static void
copy_to_view (image)
  ImageInfo *image;
{
  XilDataType   datatype;
  unsigned int  width, height, nbands;

printf("calling copy_to_view\n");
return;

  if (image->revert_image == NULL)
    return;  

  (*xil->get_info) (image->revert_image, &width, &height, &nbands, &datatype);

  image->view_image = (*xil->create) (image_display->state, width, height, nbands, datatype);
  (*xil->copy) (image->revert_image, image->view_image);

}
#endif

void
image_geo_op(image)
    ImageInfo *image;
{
    float *xform;
    unsigned int w, h, nbands;
    XilDataType datatype;
    int dx1, dx2, dy1, dy2;
    float background[3];
    float tmp;

    xform = current_state->xform;

    w = (*xil->get_width) (image->orig_image);
    h = (*xil->get_height) (image->orig_image);
    dx1 = abs(w*xform[0]-h*xform[2]);
    dy1 = abs(w*xform[1]-h*xform[3]);
    dx2 = abs(w*xform[0]+h*xform[2]);
    dy2 = abs(w*xform[1]+h*xform[3]);

    image->width = (dx1 > dx2 ? dx1 : dx2);
    image->height = (dy1 > dy2 ? dy1 : dy2);

    if (image->dest_image) {
	(*xil->destroy) (image->dest_image);
    }
    image->dest_image = (*xil->create) (image_display->state,
	    image->width, image->height,
	    (*xil->get_nbands) (image->orig_image),
	    (*xil->get_datatype) (image->orig_image));
    if (!image->dest_image) {
	if (prog->verbose)
	    fprintf (stderr, catgets (prog->catd, 1, 79, 
				   "%s: xil_create() failed.\n"), prog->name);
	return;
    }

    tmp = xform[1] * xform[2];
    /* if tmp != 0.0 or 1.0 or -1.0 then there is a
     * rotation component in the xform matrix
     */
    if (tmp != 0.0 && tmp != 1.0 && tmp != -1.0) {
	if ((*xil->get_nbands) (image->dest_image) == 1) {
	    background[0] = 1.0;
	} else {
	    background[0] = background[1] = background[2] = 0.0;
	}
	(*xil->set_value) (image->dest_image, background);
    }

    (*xil->set_origin) (image->orig_image,
	(float)(*xil->get_width) (image->orig_image)/2.0,
	(float)(*xil->get_height) (image->orig_image)/2.0);
    (*xil->set_origin) (image->dest_image,
	(float)(*xil->get_width) (image->dest_image)/2.0,
	(float)(*xil->get_height) (image->dest_image)/2.0);

    /* perform the affine operation */
    (*xil->affine) (image->orig_image, image->dest_image, "nearest", xform);

    (*xil->set_origin) (image->orig_image, 0.0, 0.0);
    (*xil->set_origin) (image->dest_image, 0.0, 0.0);
}

#ifdef USING_XIL
void
copy_to_pixmap ()
{
    if (image_display->pixmap1 != (Pixmap) NULL)
       XFreePixmap (image_display->xdisplay, image_display->pixmap1);

    image_display->pixmap1 = create_pixmap (image_display, 
			     current_image->width, current_image->height, 
			     image_display->depth);
    check_canvas (current_image->width, current_image->height,
                  base_window->canvas, base_window->hscroll,
                  base_window->vscroll, NULL, NULL, image_display);
    XSetFunction (image_display->xdisplay, image_display->win_gc, GXcopy);
    XPutImage (image_display->xdisplay, image_display->pixmap1, 
	       image_display->win_gc, current_image->view_image, 0, 0, 0, 0,
               current_image->width, current_image->height);
}
#endif
