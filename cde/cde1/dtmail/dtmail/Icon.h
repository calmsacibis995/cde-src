/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef ICON_H
#define ICON_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Icon.h	1.19 09/06/94"
#endif

#include "UIComponent.h"
#include "Attachment.h"

#include <DtMail/DtMail.hh>

#include <Xm/Xm.h>

#include <Dt/Dts.h>
#include <Dt/IconFile.h>
extern "C" {
#include <Dt/Icon.h>
}

class Icon : public UIComponent {
    private:
	Boolean	 _is_selected;
	Boolean	 _is_armed;
	Pixel	_cur_fg, _cur_bg;
	void  invert(void);
	void  arm(void);
	void  disarm(void);
        int 	_indx;	/* user data */
	static void	dragMotionHandler(
				Widget,
				XtPointer,
				XEvent *
			);

	static void	iconCallback( 
				Widget, 
				XtPointer, 
				XtPointer
			);
    protected:
	Attachment     *_parent;	// associated attachemnt

    public:
	// Constructor and destructor
	Icon ( Attachment *, char *, unsigned char *label, unsigned short, Widget, int);
	virtual		~Icon();

	// AV callback methods
	void select(void);
	void primitiveSelect(void);
	void defaultAction(void);
	void unselect();

	// associated attachment
	Attachment* 	parent() 		{ return ( _parent ); }
	virtual const char *const className() 	{ return ( "Icon" ); }
};
#endif
