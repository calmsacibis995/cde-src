/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: xmbind.c /main/cde1_maint/2 1995/10/18 22:50:55 pascale $"
#endif
#endif
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/VirtKeysP.h>

main(argc, argv)
    int argc;
    char **argv;
{
    XtAppContext  app_context;
    Display *display;
    String bindings = NULL;

    XtToolkitInitialize();
    app_context = XtCreateApplicationContext();
    display = XtOpenDisplay(app_context, NULL, argv[0], "Xmbind",
			NULL, 0, &argc, argv);
    
    if (display == NULL) {
	fprintf(stderr, "%s:  Can't open display\n", argv[0]);
	exit(1);
    }

    if (argc == 2) {
	if (_XmVirtKeysLoadFileBindings (argv[1], &bindings) == True) {
	    XDeleteProperty (display, RootWindow (display, 0),
		XInternAtom (display, "_MOTIF_DEFAULT_BINDINGS", False));
	    XChangeProperty (display, RootWindow(display, 0),
		XInternAtom (display, "_MOTIF_BINDINGS", False),
		XA_STRING, 8, PropModeReplace,
		(unsigned char *)bindings, strlen(bindings));
	    XFlush (display);
	    XtFree (bindings);
	    exit(0);
	}
	else {
	    fprintf(stderr, "%s:  Can't open %s\n", argv[0], argv[1]);
	    exit(1);
	}
    }

    XDeleteProperty (display, RootWindow (display, 0),
		XInternAtom (display, "_MOTIF_BINDINGS", False));
    XDeleteProperty (display, RootWindow (display, 0),
		XInternAtom (display, "_MOTIF_DEFAULT_BINDINGS", False));

    _XmVirtKeysLoadFallbackBindings (display, &bindings);

    XFlush (display);
    XtFree (bindings);
   
    exit(0);
}

