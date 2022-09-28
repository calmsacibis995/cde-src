/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>    
#include <Xm/MwmUtil.h>
    
    int main()
{
    Window windowId;
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
    Atom	XaWmHints;
    MotifWmHints       wmHintsData;
    XEvent	event;
    XFontStruct	*dtFont;
    GC	drawGC, undrawGC;
    XGCValues	dtValues;
    XTextItem	dtText;
    char	dtString[20][20];
    int		i, j, x, y, widths[5], index;
    int		saveX, saveY;
    
    if ((disptype = XOpenDisplay(NULL)) == NULL)
    {
	fprintf (stderr,"cannot create a window\n");
	return (0);
    }
    parent = RootWindow(disptype,DefaultScreen(disptype));
    width = WidthOfScreen(ScreenOfDisplay(disptype, DefaultScreen(disptype)));
    height = HeightOfScreen(ScreenOfDisplay(disptype, DefaultScreen(disptype)));
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

    XaWmHints = XInternAtom(disptype, _XA_MOTIF_WM_HINTS, False);

    wmHintsData.flags = MWM_HINTS_DECORATIONS;
    wmHintsData.decorations = 0;

    XChangeProperty(disptype, windowId, XaWmHints, XaWmHints, 32,
		    PropModeReplace, (unsigned char *) &wmHintsData,
		    PROP_MWM_HINTS_ELEMENTS);
    
    XMapWindow(disptype, windowId);

    XSelectInput(disptype, windowId, VisibilityChangeMask);

    dtFont = XLoadQueryFont(disptype,
			     "-agfa-cg times-bold-i-normal-92505-0-2160-0-0-p-0-hp-roman8");
    widths[0] = 72;
    widths[1] = 80;
    widths[2] = 86;
    widths[3] = 85;
    widths[4] = 69;

    dtValues.font = dtFont->fid;
    dtValues.foreground = WhitePixel(disptype, 0);
    dtValues.background = BlackPixel(disptype, 0);

    drawGC = XCreateGC(disptype, windowId,
		       GCFont | GCForeground | GCBackground,
		       &dtValues);
    undrawGC = XCreateGC(disptype, windowId,
			 GCFont,
			 &dtValues);

    strcpy(dtString[0], "H");
    strcpy(dtString[1], "P");
    strcpy(dtString[2], "V");
    strcpy(dtString[3], "U");
    strcpy(dtString[4], "E");
    strcpy(dtString[5], "HP DT");

    dtText.delta = 0;
    dtText.font = dtFont->fid;

    
    saveX = 100;

    
    while(1)
    {
	x = 100;
	y = 200;
	for(i = 0;i < 6;i++)
	{
	    dtText.chars = dtString[0];
	    dtText.nchars = strlen(dtString[0]);
	    XDrawText(disptype, windowId, drawGC, x, y, &dtText, 1);
	    XSync(disptype,0);

	    for(j = 0;j < 4;j++)
	    {
		sleep(1);
		XDrawText(disptype, windowId, undrawGC, x,y, &dtText, 1);
		XSync(disptype,0);
		index = widths[j + 1];
		x = x + dtFont->per_char[index].width;

		if(j == 1)
		{
		    /*
		     * if this is between P and V draw the space
		     */
		    x = x + (dtFont->per_char[32].width);
		}
		
		dtText.chars = dtString[j + 1];
		dtText.nchars = strlen(dtString[j + 1]);
		XDrawText(disptype, windowId, drawGC, x, y, &dtText, 1);
		XSync(disptype,0);
	    }

	    /*
	     * Undraw the E and draw the whole thing
	     */
	    sleep(1);
	    XDrawText(disptype, windowId, undrawGC, x, y, dtText, 1);
	    XSync(disptype,0);
	    x = saveX;
	    dtText.chars = dtString[5];
	    dtText.nchars = strlen(dtString[5]);
	    XDrawText(disptype, windowId, drawGC, x, y, &dtText, 1);
	    XSync(disptype,0);
	    
	    sleep(1);
	    /*
	     * Undraw the whole thing and move the y
	     */
	    XDrawText(disptype, windowId, undrawGC, x,y, &dtText, 1);
	    XSync(disptype,0);
	    y += 150;
	}
	
	if(XPending(disptype))
	{
	    XNextEvent(disptype, &event);
	    if(event.type == VisibilityNotify)
	    {
		XRaiseWindow(disptype, windowId);
	    }
	}
	
    }
    
}
