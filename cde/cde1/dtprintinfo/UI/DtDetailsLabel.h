/* $XConsortium: DtDetailsLabel.h /main/cde1_maint/2 1995/10/23 09:52:20 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef DTDETAILSLABEL_H
#define DTDETAILSLABEL_H

#include "MotifUI.h"

class DtDetailsLabel : public MotifUI {

   static void ClickCB(Widget, XtPointer, XtPointer);

   static boolean first_time;
   static XmString bottom_label[2];
   static Pixmap blank_pixmap;
   static Pixmap blank_mask;

 public:

   DtDetailsLabel(MotifUI *parent);
   void Update(boolean show_only_my_jobs);

   const UI_Class UIClass()           { return LABEL; }
   const char *const UIClassName()    { return "DtDetailsLabel"; }
};

#endif // DTDETAILSLABEL_H
