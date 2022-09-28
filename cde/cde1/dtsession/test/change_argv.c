/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <sys/types.h>

char *progname;

/* change_argv:  Find X clients with a given XA_WM_NAME and change their
   XA_WM_COMMAND to the desired value.  The purpose is to "tame"
   uncooperative clients to work with Dt.  Flow:

     First, examine all existing windows for the desired XA_WM_NAME
     property.  For each one found matching, change its XA_WM_COMMAND
     property to the argv list given starting with argv[2].

     Then, hang around (as a daemon) and examine all windows newly
     created or having their XA_WM_NAME property changed.  If a match is
     found to the requested XA_WM_NAME, change the XA_WM_COMMAND as
     above.

   This program terminates when killed or it loses contact with the
   server.

   There are no options: display is always obtained from $DISPLAY.
*/

int nargc;
char **nargv;
char *wm_name_value;
int wm_name_len;

usage()
{
  fprintf(stderr,
    "Usage: %s <WM_NAME_value> <argv0> [<argv1> [<argv2> ... ] ]\n", progname);
  exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
  Display *display;
  int screen, i, nprop;
  XEvent event;
  Atom *property_list;
  char *foo, *atomname();
  Window window;
  pid_t pid;

  progname = argv[0];

  if (argc < 3) usage();

  nargc = argc - 2;
  nargv = argv + 2;
  wm_name_value = argv[1];
  wm_name_len = strlen(wm_name_value) + 1;

  display = XOpenDisplay(NULL);
  if (display == NULL)
  {
    fprintf(stderr, "%s: Cannot open display\n", progname);
    exit(1);
  }
  screen = DefaultScreen(display);

  pid = fork();

  if (pid == -1)
  {
    fprintf(stderr, "%s: fork() failed\n", progname);
    exit(1);
  }

  /* Daemonize */

  if (pid) exit(0);
  setpgrp(getpid());

  /* Create a window to attach properties to... this window will never
     be mapped. */
  window = XCreateWindow(display, RootWindow(display, screen), 0, 0, 1, 1,
			 0, CopyFromParent, CopyFromParent, CopyFromParent,
			 0, NULL);

  /* Set WM_NAME and WM_COMMAND properties for the window manager's benefit */
  XStoreName(display, window, argv[0]);
  XSetCommand(display, window, argv, argc);

  XSelectInput(display, RootWindow(display, screen), SubstructureNotifyMask);

  /* Now search all existing windows for properties */
  check_existing_windows_and_properties(display, RootWindow(display, screen));

  /* Await new windows and report on their properties */
  for (;;)
  {
    XNextEvent(display, &event);
    switch (event.type)
    {
      case CreateNotify:
	property_list = XListProperties(display, event.xcreatewindow.window,
					&nprop);
	if (property_list)
	{
	  for (i = 0; i < nprop; i++)
	    check_argv(display, event.xcreatewindow.window, property_list[i]);
	  XFree(property_list);
	}
	XSelectInput(display, event.xcreatewindow.window,
		     SubstructureNotifyMask | PropertyChangeMask);
	break;
      case PropertyNotify:
	check_argv(display, event.xproperty.window, event.xproperty.atom);
	break;
    }
  }
}

check_existing_windows_and_properties(display, window)
Display *display;
Window window;
{
  Window root, parent, *children;
  Atom *property_list;
  int i, nchildren, nprop;
  char *foo;

  property_list = XListProperties(display, window, &nprop);

  if (property_list)
  {
    for (i = 0; i < nprop; i++)
      check_argv(display, window, property_list[i]);
    XFree(property_list);
  }

  XQueryTree(display, window, &root, &parent, &children, &nchildren);
  if (children)
  {
    for (i = 0; i < nchildren; i++)
      check_existing_windows_and_properties(display, children[i]);
    XFree(children);
  }
}

check_argv(display, window, property)
Display *display;
Window window;
Atom property;
{
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytesleft;
  char *prop_return;

  if (property != XA_WM_NAME) return;

  if (XGetWindowProperty(display, window, property, 0L,
		         (long)((wm_name_len + 3) >> 2),
		         False, AnyPropertyType, &actual_type,
		         &actual_format, &nitems, &bytesleft,
		         &prop_return) == Success)
  {
    if (!bytesleft && !strcmp(prop_return, wm_name_value))
      XSetCommand(display, window, nargv, nargc);
    XFree(prop_return);
  }
}
