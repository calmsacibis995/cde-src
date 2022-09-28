/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/XHPlib.h>
main(argc, argv)
int argc;
char *argv[];
{
    Atom XaProperty;
	Window windowId;
	char *s;
	int i;
	char *display_name = NULL;
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
	unsigned long    valuemask = 0;
	XSetWindowAttributes  attributes;
	unsigned int   class = CopyFromParent;
	Display  *disptype;
	XWindowAttributes winatt;
	Status  winstat;
	unsigned int pixwidth,  pixheight;
	Pixmap  newpix;
	XFontStruct *load_font;
	XFontStruct *assoc_load;
	
    /* Open the display specified by display_name */
    disptype = XOpenDisplay(NULL); 
    XaProperty = XInternAtom(disptype, argv[1], False);
    XChangeProperty(disptype, DefaultRootWindow(disptype), XaProperty,
		    XA_STRING, 8, PropModeReplace, argv[2], strlen(argv[2]));
	
    XSync(disptype,0);
}

