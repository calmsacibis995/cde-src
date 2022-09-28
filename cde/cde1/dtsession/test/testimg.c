/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#include <fcntl.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define ball_width 32
#define ball_height 32
static char ball_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x1c, 0x70, 0x00,
   0x00, 0x82, 0xc8, 0x00, 0x80, 0x09, 0x01, 0x03, 0x40, 0x00, 0x50, 0x04,
   0x40, 0x11, 0x02, 0x04, 0xa0, 0x24, 0x00, 0x0c, 0x30, 0x00, 0x20, 0x11,
   0x30, 0x03, 0x02, 0x19, 0x10, 0x54, 0x80, 0x18, 0x88, 0x01, 0x01, 0x30,
   0x28, 0x00, 0x10, 0x34, 0x38, 0x91, 0x04, 0x21, 0xc8, 0x00, 0x40, 0x34,
   0x68, 0x46, 0x01, 0x32, 0xd8, 0x0d, 0x88, 0x3b, 0xb8, 0x15, 0x42, 0x3e,
   0xf0, 0xbf, 0x50, 0x15, 0x70, 0xdd, 0x9d, 0x1b, 0xf0, 0x77, 0x77, 0x1e,
   0xe0, 0xfe, 0xd9, 0x0f, 0xc0, 0x57, 0xfe, 0x07, 0xc0, 0xfd, 0xbb, 0x07,
   0x80, 0xdf, 0xef, 0x03, 0x00, 0x7e, 0xff, 0x00, 0x00, 0xfc, 0x7f, 0x00,
   0x00, 0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

main()
{
	Window windowId;
	char *s;
	char *w_name;
	int i;
	static char *display_name = "hpcvxkd:0.0";
	int open_display = 1;
	Window parent = -1;
	int x_origin = 0;
	int y_origin = 0;
	int width = -1;
	int height = -1;
	int border_width = 0;
	Pixmap border = -1;
	Pixmap background = -1;
	int    depth = CopyFromParent;
	Visual *visual;
	XImage   *blkimg;
	unsigned long    valuemask = 0;
	XSetWindowAttributes  attributes;
	unsigned int   class = CopyFromParent;
	Display  *disptype;
	FILE     *fopen(), *info;
	int      n;
	GC       fungc, fun2gc;
	unsigned long funmask;
	XGCValues     funvals;
	Pixmap        funpix;
	XEvent		  event;

        if ((disptype = XOpenDisplay(NULL)) == NULL)
	{
	    fprintf (stderr,"cannot create a window on %s\n", display_name);
	    return (0);
	}
	parent = RootWindow(disptype,DefaultScreen(disptype));
	width = 200;
	height = 200;
        attributes.border_pixel = WhitePixel(disptype,DefaultScreen(disptype));
	valuemask |= CWBorderPixel;
        attributes.background_pixel = BlackPixel(disptype,DefaultScreen(disptype));
	valuemask |= CWBackPixel;

        /* Create a window and put it on the display */
	/* Set attributes before creating            */
	visual = DefaultVisual(disptype, DefaultScreen(disptype));
        windowId = XCreateWindow (disptype,parent, x_origin, y_origin,
 	      width, height, border_width, depth, class, visual,
	      valuemask, &attributes);
	if (!windowId)
	{
	    fprintf (stderr,"error during Window Create\n");
	    return (0);
	}
	XMapWindow(disptype, windowId);
	
	XSync(disptype,1);
	sleep(5);
	funvals.foreground = 1;
	funvals.background = 0;
	funmask = GCForeground | GCBackground; 
	fungc = XCreateGC(disptype, windowId, funmask, &funvals);
	funpix = XCreateBitmapFromData(disptype,
				       XRootWindow(disptype,
						   DefaultScreen(disptype)),
				       ball_bits,
				       ball_width,
				       ball_height);
	blkimg = XGetImage(disptype, funpix, 0,0, ball_width, ball_height,
			   AllPlanes, ZPixmap);
/*	blkimg = XCreateImage(disptype,  
			      DefaultVisual(disptype, DefaultScreen(disptype)),
			      DefaultDepth(disptype, DefaultScreen(disptype)),
			      ZPixmap, 0, ball_bits, ball_width,
			      ball_height, 8, 0);*/
	XSync(disptype, 0);
	XPutImage(disptype, windowId, fungc, blkimg, 0, 0, 50, 50,
		  ball_width, ball_height); 
	XSync(disptype, 0);
	printf("Put the image out there\n");
	sleep(10);
}
