/* $XConsortium: LabelObj.h /main/cde1_maint/2 1995/10/23 10:14:57 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef LABELOBJ_H
#define LABELOBJ_H

#include "MotifUI.h"

class LabelObj : public MotifUI {

 private:
  
   Widget _label;
   LabelType _style;

 public:

   LabelObj(MotifUI * parent,
	    char *name,
	    LabelType style = LEFT_JUSTIFIED,
	    boolean has_border = false);

   void LabelStyle(LabelType style);
   LabelType LabelStyle()             { return _style; }
   const Widget InnerWidget()         { return _label; }
   const UI_Class UIClass()           { return LABEL; }
   const char *const UIClassName()    { return "LabelObj"; }

};

#endif /* LABELOBJ_H */
