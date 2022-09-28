/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/**********************************************************************************
  * pushbutton.c: Test the XmPushButton widget.
  *         From:
  *                   The X Window System, 
  *            Programming and Applications with Xt
  *                   OSF/Motif Edition
  *         by
  *                Douglas Young
  *              Prentice Hall, 1990
  *
  *                 Example described on pages:     75-76
  *
  *
  *  Copyright 1989 by Prentice Hall
  *  All Rights Reserved
  *
  * This code is based on the OSF/Motif widget set and the X Window System
  *
  * Permission to use, copy, modify, and distribute this software for 
  * any purpose and without fee is hereby granted, provided that the above
  * copyright notice appear in all copies and that both the copyright notice
  * and this permission notice appear in supporting documentation.
  *
  * Prentice Hall and the author disclaim all warranties with regard to 
  * this software, including all implied warranties of merchantability and fitness.
  * In no event shall Prentice Hall or the author be liable for any special,
  * indirect or cosequential damages or any damages whatsoever resulting from 
  * loss of use, data or profits, whether in an action of contract, negligence 
  * or other tortious action, arising out of or in connection with the use 
  * or performance of this software.
  *
  * Open Software Foundation is a trademark of The Open Software Foundation, Inc.
  * OSF is a trademark of Open Software Foundation, Inc.
  * OSF/Motif is a trademark of Open Software Foundation, Inc.
  * Motif is a trademark of Open Software Foundation, Inc.
  * DEC is a registered trademark of Digital Equipment Corporation
  * HP is a registered trademark of the Hewlett Packard Company
  * DIGITAL is a registered trademark of Digital Equipment Corporation
  * X Window System is a trademark of the Massachusetts Institute of Technology
  **********************************************************************************/



#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Protocols.h>
#include <X11/Xatom.h>
#include <Xm/Xm.h> 
#include <Xm/PushB.h>
#include <Dt/Dt.h>

Widget toplevel, button;

extern void button_callback();

main(argc, argv) 
    int   argc;
    char *argv[];
{
  XEvent event;
  
  toplevel = XtInitialize(argv[0], "Pushbutton", 
                          NULL, 0, &argc, argv); 
  /* 
   * Create the pushbutton widget. 
   */ 
  button =  XtCreateManagedWidget("button",
                                   xmPushButtonWidgetClass,
                                   toplevel, NULL, 0);
  /*  
   * Add callbacks. 
   */ 
  XtAddCallback(button, XmNactivateCallback,
                button_callback, NULL);
  XtRealizeWidget(toplevel);

  while(1)
  {
      XtNextEvent(&event);
      XtDispatchEvent(&event);
  }

}

void button_callback(w, client_data, call_data) 
   Widget               w;  
   caddr_t              client_data; 
   XmAnyCallbackStruct *call_data;
{ 
	char *savePath;

	savePath = DtCreateSavePath(toplevel);
}
