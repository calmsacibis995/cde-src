.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Style Management Services"
.xR1 stylesvcs
.H 2 "Introduction"
.mc 4
The \*(Zx style management services allow users to
customise the visual elements and system behaviour of the \*(ZX.
These services provide a graphical interface for
customising the colours, fonts, backdrops and screen savers
of the desktop and the system-wide behaviour of the keyboard and mouse.
They also allow users to set the screen time-out
interval and the session startup behaviour of \*(Zx
and to customise the window behaviour on the desktop.
.br
.mc
.H 2 "Actions" S
This section defines the actions that provide \*(Zx style management services
to support application portability at the C-language source
or shell script levels.
.so ../hx/dtstyleaction.mm
.ds XC Capabilities
.SK
.H 2 "Capabilities"
.xR2 stylecaplist
A conforming implementation of the \*(Zx style management
services supports at least the following capabilities:
.AL
.LI
.mc 5
Conforms to the Required items in the Application Style Checklist in
.cX checklist .
.LI
.mc
Has been internationalised using the standard interfaces in
the \*(ZK, the \*(Zl, and the \*(ZM, and supports any
locale available in the underlying X Window System Xlib
implementation.
.LI
.mc 5
Allows the user to select a colour
palette from a list of available colour palettes.
Users can specify the number of colours used in
the selected palette to be 8, 4 or 2 depending upon the
variety of colours they prefer.
Colour entries in the selected palette are used to colour
the various components of the desktop.
Desktop colours can
be changed by selecting a different palette or modifying
individual colours in the selected palette.
Users can also create entirely new palettes.
Selection of the colour
palette and customisation of individual colours are
immediately reflected on the desktop.
.mc
.LI
Allows the user to specify the desktop default font size.
.mc 5
The \*(Zx style management
services use the \*(Zx interface
font aliases to provide visual integrity across the desktop.
See
.cX DtStdInterfaceFontNames .
.LI
.mc
Allows the user to select a workspace's backdrop (root window)
from a list of available backdrops.
.LI
Allows the user to customise the auto-repeat capability of the
keyboard.
.LI
Allows the user to specify the mouse to be right-handed or
left-handed and to specify the double-click interval,
pointer acceleration, pointer movement threshold
.mc 6
and meaning of button 2 (BTransfer or BSelect).
.LI
.mc
Allows the user to customise the beeper volume and duration.
.LI
Allows the user to enable or disable the screen saver, to select
and preview from a list of available screen savers the
screen saver to be run at screen time-out, and to set the
screen saver time-out interval.
.LI
Allows the user to customise the window focus (focus follows
mouse, click in window for focus), window behaviour (raise
window with focus, allow primary windows on top, opaque
move), and iconification (use icon box, place on workspace).
.LI
Allows the user to customise the session startup (resume
current session, return to home session, prompt at logout
time) and to enable/disable the logout confirmation dialog.
.LI
Allows users to replace their home session with the current session.
.LE
.H 2 "Rationale and Open Issues" S
.I
The following is text copied from the
.R
CDE X/Open Specification Framework Proposal, Draft 2.
.I
It is not intended to remain in this specification when it
is published by X/Open.
.R
.P
Actions are required for application portability.
No CLI
for the
.Cm dtstyle
utility is planned for standardisation because Actions is a
superior interface for this service and the utility may be
replaced by a different interface in a future version.
Capabilities are required to ensure that conforming
implementations provide the minimum expected services,
without overly constraining tool design.
.eF e
