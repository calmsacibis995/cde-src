.\" **
.\" **
.\" **  (c) Copyright 1989, 1990 by Open Software Foundation, Inc.
.\" **      All Rights Reserved.
.\" **
.\" **  (c) Copyright 1987, 1988, 1989, by Hewlett-Packard Company
.\" **
.\" **  (c) Copyright 1987, 1988 by Digital Equipment Corporation,
.\" **      Maynard, MA.  All Rights Reserved.
.\" **
.\" **
.TH XmPushButton 3X "" "" "" ""
.ds )H Hewlett-Packard Company
.ds ]W Motif Release 1.1: August 1990
.ds )H Hewlett-Packard Company
.ds ]W Motif Release 1.1: August 1990
.SH NAME
\fBXmPushButton \- the PushButton widget class.\fP
.iX "XmPushButton"
.iX "widget class" "PushButton"
.sp .5
.SH SYNOPSIS
\fB#include <Xm/PushB.h>\fP
.sp .5
.SH DESCRIPTION
PushButton issues commands within an application.
It consists of a text label or pixmap surrounded
by a border shadow. 
When a PushButton is selected, the shadow changes to give the appearance
that it has been pressed in.  When a PushButton is 
unselected, the shadow changes to give the appearance that it is out.
.PP
The default behavior associated with a PushButton in a menu depends on
the type of menu system in which it resides.
By default, \fBBSelect\fP controls the behavior of the PushButton.
In addition, \fBBMenu\fP controls the behavior of the PushButton if
it resides in a PopupMenu system.
The actual mouse button used is determined by its RowColumn parent.
.PP
Thickness for a second shadow, used when the PushButton is the default
button,  may be specified by using the 
\fBXmNshowAsDefault\fP resource.  If it has a non-zero value, the Label's
resources \fBXmNmarginLeft\fP, \fBXmNmarginRight\fP, \fBXmNmarginTop\fP, and
\fBXmNmarginBottom\fP may be modified to accommodate the second shadow.
.sp .5
.SS "Classes"
PushButton inherits behavior and resources from \fBCore\fP, \fBXmPrimitive\fP, 
and \fBXmLabel\fP Classes.
.PP
The class pointer is \fBxmPushButtonWidgetClass\fP. 
.PP
The class name is \fBXmPushButton\fP.
.sp .5
.SS "New Resources"
The following table defines a set of widget resources used by the programmer
to specify data.  The programmer can also set the resource values for the 
inherited classes to set attributes for this widget.  To reference a
resource by name or by class in a .Xdefaults file, remove the \fBXmN\fP or
\fBXmC\fP prefix and use the remaining letters.  To specify one of the defined 
values for a resource in a .Xdefaults file, remove the \fBXm\fP prefix and use
the remaining letters (in either lowercase or uppercase, but include any
underscores between words).
The codes in the access column indicate if the given resource can be
set at creation time (\fBC\fP),
set by using \fBXtSetValues\fP (\fBS\fP), 
retrieved by using \fBXtGetValues\fP (\fBG\fP), or is not applicable (\fBN/A\fP).
.sp .5
.sp .5
.in 0
.KS
.sp 1
.in 0
.TS
center;
cBp7 ssss
lBp6 lBp6 lBp6 lBp6 lBp6
lp6 lp6 lp6 lp6 lp6.
XmPushButton Resource Set
Name	Class	Type	Default	Access
_
XmNactivateCallback	XmCCallback	XtCallbackList	NULL	C
XmNarmCallback	XmCCallback	XtCallbackList	NULL	C
XmNarmColor	XmCArmColor	Pixel	dynamic	CSG
XmNarmPixmap	XmCArmPixmap	Pixmap	XmUNSPECIFIED_PIXMAP	CSG
XmNdefaultButtonShadowThickness	XmCDefaultButtonShadowThickness	Dimension	0	CSG
XmNdisarmCallback	XmCCallback	XtCallbackList	NULL	C
XmNfillOnArm	XmCFillOnArm	Boolean	True	CSG
XmNmultiClick	XmCMultiClick	unsigned char	dynamic	CSG
XmNshowAsDefault	XmCShowAsDefault	Dimension	0	CSG
.TE
.in
.sp 1
.KE
.in
.sp .5
.sp .5
.IP "\fBXmNactivateCallback\fP"
Specifies the list of callbacks that is called when PushButton is activated.
PushButton is activated when the user presses
and releases the active mouse button 
while the pointer is inside that widget.  Activating the PushButton
also disarms it.
For this callback the reason is \fBXmCR_ACTIVATE\fP.
.IP "\fBXmNarmCallback\fP"
Specifies the list of callbacks that is called when PushButton is armed.
PushButton is armed when the user presses
the active mouse button while the pointer is inside that
widget. 
For this callback the reason is \fBXmCR_ARM\fP.
.IP "\fBXmNarmColor\fP"
Specifies the color with which to fill the armed button.
\fBXmNfillOnArm\fP must be set to True for this resource to have an effect.
The default for a color display is a color between the background and the
bottom shadow color.  For a monochrome display, the default is set to the
foreground color, and any text in the label appears in the background
color when the button is armed.
.IP "\fBXmNarmPixmap\fP"
Specifies the pixmap to be used as the button face if \fBXmNlabelType\fP is
\fBXmPIXMAP\fP and PushButton is armed.  This resource is disabled when the  
PushButton is in a menu.
.IP "\fBXmNdefaultButtonShadowThickness\fP"
This resource specifies the width of the default button indicator shadow.
If this resource is zero, the width of the shadow comes from the value of
the \fBXmNshowAsDefault\fP resource.  If this resource is greater than
zero, the \fBXmNshowAsDefault\fP resource is only used to specify whether
this button is the default.
.IP "\fBXmNdisarmCallback\fP"
Specifies the list of callbacks
that is called when PushButton is disarmed.  PushButton is disarmed when
the user presses and releases the active mouse button
while the pointer is inside that widget.  For this callback, the reason is
\fBXmCR_DISARM\fP.
.IP "\fBXmNfillOnArm\fP"
Forces the PushButton to fill the background of the button with the
color specified by \fBXmNarmColor\fP when the button is armed and when this
resource is set to True.  If False, only the top and bottom
shadow colors are switched.  
When the PushButton is in a menu, this resource is ignored and assumed to be
False.
.IP "\fBXmNmultiClick\fP"
If a button click is followed by another button click within the time
span specified by the display's multi-click time, and this resource is
set to \fBXmMULTICLICK_DISCARD\fP, do not process the second click.
If this resource is set to \fBXmMULTICLICK_KEEP\fP, process the event
and increment \fIclick_count\fP in the callback structure.
When the button is not in a menu, the default value is
\fBXmMULTICLICK_KEEP\fP.
.IP "\fBXmNshowAsDefault\fP"
If \fBXmNdefaultButtonShadowThickness\fP is greater than zero, a value
greater than zero in this resource
specifies to mark this button as the default button.
If \fBXmNdefaultButtonShadowThickness\fP is zero, a value greater than
zero in this resource specifies to mark this button as the default
button with the shadow thickness specified by this resource.
The space between
the shadow and the default shadow is equal to the sum of both shadows. 
The default value is zero.  When this value is not zero, the Label
resources \fBXmNmarginLeft\fP, \fBXmNmarginRight\fP, \fBXmNmarginTop\fP, and
\fBXmNmarginBottom\fP may be modified to accommodate the second shadow.
This resource is disabled when the PushButton is in a menu.
.sp .5
.SS "Inherited Resources"
PushButton inherits behavior and resources from the following   
superclasses.  For a complete description of each resource, refer to the
man page for that superclass.
.sp .5
.sp .5
.in 0
.KS
.sp 1
.in 0
.TS
center;
cBp7 ssss
lBp6 lBp6 lBp6 lBp6 lBp6
lp6 lp6 lp6 lp6 lp6.
XmLabel Resource Set
Name	Class	Type	Default	Access
_
XmNaccelerator	XmCAccelerator	String	NULL	CSG
XmNacceleratorText	XmCAcceleratorText	XmString	NULL	CSG
XmNalignment	XmCAlignment	unsigned char	XmALIGNMENT_CENTER	CSG
XmNfontList	XmCFontList	XmFontList	dynamic	CSG
XmNlabelInsensitivePixmap	XmCLabelInsensitivePixmap	Pixmap	XmUNSPECIFIED_PIXMAP	CSG
XmNlabelPixmap	XmCLabelPixmap	Pixmap	XmUNSPECIFIED_PIXMAP	CSG
XmNlabelString	XmCXmString	XmString	dynamic	CSG
XmNlabelType	XmCLabelType	unsigned char	XmSTRING	CSG
XmNmarginBottom	XmCMarginBottom	Dimension	dynamic	CSG
XmNmarginHeight	XmCMarginHeight	Dimension	2	CSG
XmNmarginLeft	XmCMarginLeft	Dimension	dynamic	CSG
XmNmarginRight	XmCMarginRight	Dimension	dynamic	CSG
XmNmarginTop	XmCMarginTop	Dimension	dynamic	CSG
XmNmarginWidth	XmCMarginWidth	Dimension	2	CSG
XmNmnemonic	XmCMnemonic	KeySym	NULL	CSG
XmNmnemonicCharSet	XmCMnemonicCharSet	String	dynamic	CSG
XmNrecomputeSize	XmCRecomputeSize	Boolean	True	CSG
XmNstringDirection	XmCStringDirection	XmStringDirection	dynamic	CSG
.TE
.in
.sp 1
.KE
.in
.sp .5
.sp .5
.sp .5
.in 0
.KS
.sp 1
.in 0
.TS
center;
cBp7 ssss
lBp6 lBp6 lBp6 lBp6 lBp6
lp6 lp6 lp6 lp6 lp6.
XmPrimitive Resource Set
Name	Class	Type	Default	Access
_
XmNbottomShadowColor	XmCBottomShadowColor	Pixel	dynamic	CSG
XmNbottomShadowPixmap	XmCBottomShadowPixmap	Pixmap	XmUNSPECIFIED_PIXMAP	CSG
XmNforeground	XmCForeground	Pixel	dynamic	CSG
XmNhelpCallback	XmCCallback	XtCallbackList	NULL	C
XmNhighlightColor	XmCHighlightColor	Pixel	dynamic	CSG
XmNhighlightOnEnter	XmCHighlightOnEnter	Boolean	False	CSG
XmNhighlightPixmap	XmCHighlightPixmap	Pixmap	dynamic	CSG
XmNhighlightThickness	XmCHighlightThickness	Dimension	2	CSG
XmNnavigationType	XmCNavigationType	XmNavigationType	XmNONE	G
XmNshadowThickness	XmCShadowThickness	Dimension	2	CSG
XmNtopShadowColor	XmCTopShadowColor	Pixel	dynamic	CSG
XmNtopShadowPixmap	XmCTopShadowPixmap	Pixmap	dynamic	CSG
XmNtraversalOn	XmCTraversalOn	Boolean	True	CSG
XmNunitType	XmCUnitType	unsigned char	dynamic	CSG
XmNuserData	XmCUserData	Pointer	NULL	CSG
.TE
.in
.sp 1
.KE
.in
.sp .5
.sp .5
.sp .5
.in 0
.KS
.sp 1
.in 0
.TS
center;
cBp7 ssss
lBp6 lBp6 lBp6 lBp6 lBp6
lp6 lp6 lp6 lp6 lp6.
Core Resource Set 
Name	Class	Type	Default	Access
_
XmNaccelerators	XmCAccelerators	XtAccelerators	NULL	CSG
XmNancestorSensitive	XmCSensitive	Boolean	dynamic	G
XmNbackground	XmCBackground	Pixel	dynamic	CSG
XmNbackgroundPixmap	XmCPixmap	Pixmap	XmUNSPECIFIED_PIXMAP	CSG
XmNborderColor	XmCBorderColor	Pixel	XtDefaultForeground	CSG
XmNborderPixmap	XmCPixmap	Pixmap	XmUNSPECIFIED_PIXMAP	CSG
XmNborderWidth	XmCBorderWidth	Dimension	0	CSG
XmNcolormap	XmCColormap	Colormap	dynamic	CG
XmNdepth	XmCDepth	int	dynamic	CG
XmNdestroyCallback	XmCCallback	XtCallbackList	NULL	C
XmNheight	XmCHeight	Dimension	dynamic	CSG
XmNinitialResourcesPersistent	XmCInitialResourcesPersistent	Boolean	True	CG
XmNmappedWhenManaged	XmCMappedWhenManaged	Boolean	True	CSG
XmNscreen	XmCScreen	Screen *	dynamic	CG
XmNsensitive	XmCSensitive	Boolean	True	CSG
XmNtranslations	XmCTranslations	XtTranslations	NULL	CSG
XmNwidth	XmCWidth	Dimension	dynamic	CSG
XmNx	XmCPosition	Position	0	CSG
XmNy	XmCPosition	Position	0	CSG
.TE
.in
.sp 1
.KE
.in
.sp .5
.sp .5
.SS "Callback Information"
A pointer to the following structure is passed to each callback:
.sp .5
.nf
.ta .25i 1.1i
\fBtypedef struct\fP
{
	\fBint\fP	\fIreason\fP;
	\fBXEvent\fP	\fI* event\fP;
	\fBint\fP	\fIclick_count\fP;
} \fBXmPushButtonCallbackStruct\fP;
.fi
.fi
.sp .5
.IP "\fIreason\fP" .75i
Indicates why the callback was invoked.
.IP "\fIevent\fP" .75i
Points to the \fBXEvent\fP that triggered the callback.  
.IP "\fIclick_count\fP" .75i
This value is valid only when the reason is \fBXmCR_ACTIVATE\fP.
It contains the number of clicks in the last multiclick sequence
if the \fBXmNmultiClick\fP resource is set to \fBXmMULTICLICK_KEEP\fP,
otherwise it contains \fB1\fP.
The activate callback is invoked for each click if \fBXmNmultiClick\fP
is set to \fBXmMULTICLICK_KEEP\fP.
.sp .5 \"Adds space before the SS
.SS "Translations"
XmPushButton includes translations from Primitive.
.P
Note that altering translations in \fB#override\fP
or \fB#augment\fP mode is undefined.
.P
The XmPushButton translations for XmPushButtons not in a
menu system are listed below.
These translations may not directly correspond to a
translation table.
\fB
.nf
.ta 1.5i
.sp .5
BSelect Press:	Arm()
BSelect Click:	Activate()
	Disarm()
BSelect Release:	Activate()
	Disarm()
BSelect Press 2+:	MultiArm()
BSelect Release 2+:	MultiActivate()
	Disarm()
KActivate:	ArmAndActivate()
KSelect:	ArmAndActivate()
KHelp:	Help()
.fi
\fP
.P
XmPushButton inherits menu traversal translations
from XmLabel.
Additional XmPushButton translations for PushButtons in a
menu system are listed below.
In a Popup menu system, \fBBMenu\fP also performs the \fBBSelect\fP
actions.
These translations may not directly correspond to a
translation table.
\fB
.nf
.ta 1.5i
.sp .5
BSelect Press:	BtnDown()
BSelect Release:	BtnUp()
KActivate:	ArmAndActivate()
KSelect:	ArmAndActivate()
MAny KCancel:	MenuShellPopdownOne()
.fi
\fP
.sp .5 \"Adds space before the SS
.SS "Action Routines"
The XmPushButton action routines are described below:
.IP "\fBActivate()\fP:"
This action draws the shadow in the unarmed 
state.
If the button is not in a menu and if \fBXmNfillOnArm\fP is set to True,
the background color reverts to the unarmed color.
If \fBXmNlabelType\fP is \fBXmPIXMAP\fP, the \fBXmNlabelPixmap\fP is
used for the button face.
If the pointer is still within the button,
this action calls the callbacks for \fBXmNactivateCallback\fP.
.IP "\fBArm()\fP:"
This action arms the PushButton.
It draws the shadow in the armed state.
If the button is not in a menu and if \fBXmNfillOnArm\fP is set
to True, it fills the button with the color specified by
\fBXmNarmColor\fP.
If \fBXmNlabelType\fP is \fBXmPIXMAP\fP, the \fBXmNarmPixmap\fP is
used for the button face.
It calls the \fBXmNarmCallback\fP callbacks.
.IP "\fBArmAndActivate()\fP:"
In a menu, unposts all menus in the menu hierarchy, calls the
\fBXmNarmCallback\fP callbacks, and calls the \fBXmNactivateCallback\fP
callbacks.
.IP
Otherwise, draws the shadow in the armed state and, if
\fBXmNfillOnArm\fP is set to True, fills the button with the color
specified by \fBXmNarmColor\fP.
If \fBXmNlabelType\fP is \fBXmPIXMAP\fP, the \fBXmNarmPixmap\fP is
used for the button face.
Calls the \fBXmNarmCallback\fP callbacks.
Draws the shadow in the unarmed state and, if \fBXmNfillOnArm\fP is set
to True, causes the background color to revert to the unarmed color.
If \fBXmNlabelType\fP is \fBXmPIXMAP\fP, the \fBXmNlabelPixmap\fP is
used for the button face.
Calls the \fBXmNactivateCallback\fP callbacks.
.IP "\fBBtnDown()\fP:"
This action unposts any menus posted by the PushButton's parent menu,
disables keyboard traversal for the menu, and enables mouse traversal
for the menu.
It draws the shadow in the armed state
and calls the \fBXmNarmCallback\fP callbacks.
.IP "\fBBtnUp()\fP:"
This action unposts all menus in the menu hierarchy and activates the
PushButton.
It calls the \fBXmNactivateCallback\fP callbacks and then the
\fBXmNdisarmCallback\fP callbacks.
.IP "\fBDisarm()\fP:"
Calls the callbacks for \fBXmNdisarmCallback\fP.
.IP "\fBHelp()\fP:"
In a Pulldown or Popup MenuPane, unposts all menus in the menu hierarchy
and restores keyboard focus to the tab group that had the focus before
the menu system was entered.
Calls the callbacks for \fBXmNhelpCallback\fP if any exist.
If there are no help
callbacks for this widget, this action calls the help callbacks
for the nearest ancestor that has them.
.IP "\fBMenuShellPopdownOne()\fP:"
In a toplevel Pulldown MenuPane from a MenuBar, unposts the menu,
disarms the MenuBar CascadeButton and the MenuBar, and restores keyboard
focus to the tab group that had the focus before the MenuBar was
entered.
In other Pulldown MenuPanes, unposts the menu.
.IP
In a Popup MenuPane, unposts the menu and restores keyboard focus to the
widget from which the menu was posted.
.IP "\fBMultiActivate()\fP:"
If \fBXmNmultiClick\fP is \fBXmMULTICLICK_DISCARD\fP, this action does
nothing.
.IP
If \fBXmNmultiClick\fP is \fBXmMULTICLICK_KEEP\fP, this action does the
following:
Increments \fIclick_count\fP in the callback structure.
Draws the shadow in the unarmed state.
If the button is not in a menu and if \fBXmNfillOnArm\fP is set to True,
the background color reverts to the unarmed color.
If \fBXmNlabelType\fP is \fBXmPIXMAP\fP, the \fBXmNlabelPixmap\fP is
used for the button face.
If the pointer is within the PushButton, calls the
callbacks for \fBXmNactivateCallback\fP.
Calls the callbacks for \fBXmNdisarmCallback\fP.
.IP "\fBMultiArm()\fP:"
If \fBXmNmultiClick\fP is \fBXmMULTICLICK_DISCARD\fP, this action does
nothing.
.IP
If \fBXmNmultiClick\fP is \fBXmMULTICLICK_KEEP\fP, this action does the
following:
Draws the shadow in the armed state.
If the button is not in a menu and if \fBXmNfillOnArm\fP is set
to True, fills the button with the color specified by
\fBXmNarmColor\fP.
If \fBXmNlabelType\fP is \fBXmPIXMAP\fP, the \fBXmNarmPixmap\fP is
used for the button face.
Calls the \fBXmNarmCallback\fP callbacks.
.sp .5 \"Adds space before the SS
.SS "Additional Behavior"
This widget has the additional behavior described below:
.IP "\fB<EnterWindow>\fP:"
In a menu, if keyboard traversal is enabled, this action does nothing.
Otherwise, it draws the shadow in the armed state and calls the
\fBXmNarmCallback\fP callbacks.
.IP
If the PushButton is not in a menu and the cursor leaves and then
reenters the PushButton's window while the button is pressed, this
action draws the shadow in the armed state.
If \fBXmNfillOnArm\fP is set to True, it also fills the button with the
color specified by \fBXmNarmColor\fP.
If \fBXmNlabelType\fP is \fBXmPIXMAP\fP, the \fBXmNarmPixmap\fP is
used for the button face.
.IP "\fB<LeaveWindow>\fP:"
In a menu, if keyboard traversal is enabled, this action does nothing.
Otherwise, it draws the shadow in the unarmed state and calls the
\fBXmNdisarmCallback\fP callbacks.
.IP
If the PushButton is not in a menu and the cursor leaves the
PushButton's window while the button is pressed, this action draws the
shadow in the unarmed state.
If \fBXmNfillOnArm\fP is set to True, the background color reverts to
the unarmed color.
If \fBXmNlabelType\fP is \fBXmPIXMAP\fP, the \fBXmNlabelPixmap\fP is
used for the button face.
.sp .5 \"Adds space before the SS
.SS "Virtual Bindings"
The bindings for virtual keys are vendor specific.
For information about bindings for virtual buttons and keys, see \fBVirtualBindings(3X)\fP.
.SH RELATED INFORMATION
\fBCore(3X)\fP,
\fBXmCreatePushButton(3X)\fP, \fBXmLabel(3X)\fP, 
\fBXmPrimitive(3X)\fP, and \fBXmRowColumn(3X)\fP.

