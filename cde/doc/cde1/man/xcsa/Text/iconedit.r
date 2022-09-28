.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Icon Editing Services"
.xR1 iconeditsvcs
.H 2 "Introduction"
.mc 4
The \*(Zx icon editing services allows users to create and
modify icons that are used in the \*(ZX.
This service provides a graphical interface
with a drawing area and tools and colours to use for
creating icons and modifying previously created icons.
.br
.mc
.H 2 "Actions" S
This section defines the actions that provide \*(Zx icon editing services
to support application portability at the C-language source
or shell script levels.
.so ../hx/dticonaction.mm
.ds XC Messages
.SK
.H 2 "Messages"
.mc 5
The \*(Zx icon editing services implement the
.I Edit
request for media types XPM and XBM.
See the \*(ZT.
These services also respond to the
.I Quit
desktop message.
See the \*(Zv.
.br
.mc
.H 2 "Capabilities" S
A conforming implementation of the \*(Zx icon editing
services supports at least the following capabilities:
.AL
.LI
.mc 5
Conforms to the Required items in the Application Style Checklist in
.cX checklist ,
.mc 6
with the following exception that may exist on some
implementations:
.AL a
.LI
There are certain operations that need not comply with
checklist item 7-10.
The option button in file selection
dialogs allowing users to specify the format when saving a
file need not be available.
.LE
.LI
.mc
Has been internationalised using the standard interfaces in
the \*(ZK, the \*(Zl, and the \*(ZM, and supports any
locale available in the underlying X Window System Xlib
implementation.
.LI
Creates new icon files or modifies existing icon files.
.LI
Reads and writes XPM and XBM icon file formats;
see
.cX iconconv .
.LI
Provides a drawing area where the icon is created or
modified.
.LI
Allows the drawing area to be a drop destination for icon
files.
.LI
Provides geometric drawing operations, including line,
polyline, rectangle, polygon, circle and ellipse, for
creating or modifying the icon.
.LI
Supports the following operations within the drawing area:
.AL a
.LI
Copy, cut and paste the selected area
.LI
Scale the selected area
.LI
Rotate (left/right) the selected area
.LI
Flip (vertical/horizontal) the selected area
.LE
.LI
Allows selection of the colour to use for drawing
operations, including \*(Zx dynamically defined colours;
see
.cX iconconv .
.LI
Supports resizing of the icon width and height.
.LI
Allows specification of the hot spot within the icon.
.LI
Supports magnification of the icon within the drawing area.
.LI
Allows as input to the drawing area any portion of the display screen
by providing an active bounding rectangle to the user for selecting
the screen area to be used.
.LE
.H 2 "Rationale and Open Issues" S
.I
The following is text copied from the
.R
CDE X/Open Specification Framework Proposal, Draft 2.
.I
It is not intended to remain in this specification when it is published
by X/Open.
.R
.P
Actions are required for application portability.
ToolTalk message
protocols are required for application portability.
Capabilities are
required to ensure that conforming implementations provide the minimum
expected services, without overly constraining tool design.
The
.Cm dticon
CLI is not planned for standardisation because Actions and messaging
are superior interfaces for this service.
.eF e
