#ifndef lint
static char sccsid[] = "@(#)photocd.c 1.4 96/05/29";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <xil/xil.h>

#include "image.h"
#include "imagetool.h"
#include "display.h"

int
photocd_load (image)
    ImageInfo	*image;
{
    XilImage            src, tmp;
    int			status;
    XilPhotoCDRotate	rotation;
    float		angle;
    unsigned int	w, h;
 
    src = (*xil->create_from_device) (image_display->state, "ioSUNWPhotoCD", NULL);
 
    status = (*xil->set_device_attribute) (src, "FILEPATH", image->realfile);
    if (status != XIL_SUCCESS) {
	(*xil->destroy) (src);
	return (-1);
    }
    
    switch (image->type_info->type) {
      case PCD16:
        status = (*xil->set_device_attribute)
	           (src, "RESOLUTION", (void *) XIL_PHOTOCD_16TH_BASE);
 	break;
      case PCD4:
        status = (*xil->set_device_attribute)
	           (src, "RESOLUTION", (void *) XIL_PHOTOCD_4TH_BASE);
 	break;
      case PCD:
        status = (*xil->set_device_attribute) 
	           (src, "RESOLUTION", (void *) XIL_PHOTOCD_BASE);
 	break;
      case PCDX4:
        status = (*xil->set_device_attribute) 
	           (src, "RESOLUTION", (void *) XIL_PHOTOCD_4X_BASE);
 	break;
      default:
        break;
    }

    status = (*xil->get_device_attribute) (src, "ROTATION", (void **)&rotation);
    if (status != XIL_SUCCESS) {
	(*xil->destroy) (src);
	return (-1);
    }

    w = (*xil->get_width) (src);
    h = (*xil->get_height) (src);

    switch (rotation) {
	case XIL_PHOTOCD_CCW0:
	    image->orig_image = (*xil->create) (image_display->state, 
		w, h, 3, XIL_BYTE);
	    angle = 0.0;
	    tmp = src;
	    src = NULL;
	break;
	case XIL_PHOTOCD_CCW90:
	    image->orig_image = (*xil->create) (image_display->state, 
		h, w, 3, XIL_BYTE);
	    tmp = (*xil->create) (image_display->state, 
		h, w, 3, XIL_BYTE);
	    (*xil->set_origin) (src, (float)w/2.0, (float)h/2.0);
	    (*xil->set_origin) (tmp, (float)h/2.0, (float)w/2.0);
	    angle = 1.5708;
	    (*xil->rotate) (src, tmp, "nearest", angle);
	    (*xil->destroy) (src);
	    src = NULL;
	break;
	case XIL_PHOTOCD_CCW180:
	    image->orig_image = (*xil->create) (image_display->state, 
		w, h, 3, XIL_BYTE);
	    tmp = (*xil->create) (image_display->state, 
		w, h, 3, XIL_BYTE);
	    (*xil->set_origin) (src, (float)w/2.0, (float)h/2.0);
	    (*xil->set_origin) (tmp, (float)w/2.0, (float)h/2.0);
	    angle = 3.14159;
	    (*xil->rotate) (src, tmp, "nearest", angle);
	    (*xil->destroy) (src);
	    src = NULL;
	break;
	case XIL_PHOTOCD_CCW270:
	    image->orig_image = (*xil->create) (image_display->state, 
		h, w, 3, XIL_BYTE);
	    tmp = (*xil->create) (image_display->state, 
		h, w, 3, XIL_BYTE);
	    (*xil->set_origin) (src, (float)w/2.0, (float)h/2.0);
	    (*xil->set_origin) (tmp, (float)h/2.0, (float)w/2.0);
	    angle = 4.71239;
	    (*xil->rotate) (src, tmp, "nearest", angle);
	    (*xil->destroy) (src);
	    src = NULL;
	break;
    }

    (*xil->set_origin) (tmp, 0.0, 0.0);

    (*xil->set_colorspace) (tmp, (*xil->colorspace_get_by_name) 
	(image_display->state, "photoycc"));
    (*xil->set_colorspace) (image->orig_image, (*xil->colorspace_get_by_name) 
	(image_display->state, "rgb709"));

    (*xil->color_convert) (tmp, image->orig_image);

    image->depth = 24;	
    image->nbands = 3;
    image->datatype = XIL_BYTE;
    image->width = (*xil->get_width) (image->orig_image);
    image->height = (*xil->get_height) (image->orig_image);

    /* the data is in BGR order */
    image->rgborder = 0;

    (*xil->destroy) (tmp);

    return(0);

}
