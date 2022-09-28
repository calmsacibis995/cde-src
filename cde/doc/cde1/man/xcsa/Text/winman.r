.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.fC
.H 1 "Window Management Services"
.xR1 winmgmtsvcs
.nr Ej 1
.H 2 "Introduction"
.mc 3
This chapter describes the \*(Zx window management services.
The \*(Zx window manager is a superset of the X/Open Motif
.Cm mwm
window manager, which includes the
following major enhancements:
.BL
.LI
Support for historical OPEN LOOK applications.
(OPEN LOOK hints are mapped to the nearest Motif behaviour.)
.LI
Support for multiple workspaces.
See
.cX wkspmgmtsvcs .
.LI
Support for the front panel.
See
.cX frontpsvcs .
.P
.mc 6
.Bl
.LE
.mc 3
.P
Many features of the window manager can be configured via X resources and
configuration files, including:
.BL
.LI
Root window pop-up menu.
.LI
Window menu for client windows.
.LI
Key and button bindings for raising and lowering client windows.
.LE
.P
.mc 6
Commonly customised window manager features, such as the focus policy,
can also be configured through dialogs in the style manager.
.P
.mc 3
The window manager provides API support for the following tasks:
.BL
.LI
Standard ICCCM and Motif capabilities (for example, iconify a window).
These functions are described in the following standards:
.DL
.LI
\*(Zi
.LI
\*(Zl
.LI
.mc 3
\*(ZM
.LE
.LI
Selecting icons placed on the root window but controlled by other
applications
.mc 6
.LE
.P
There is one additional functional difference between the \*(Zx
window manager and
.Cm mwm :
.BL
.LI
By default, the only keyboard accelerator
included in the window menu for
the \*(ZX window manager is Close (Alt/F4).
.LE
.mc 2
.H 2 "Data Formats"
The resource description file
.B $HOME/.dtwmrc
is equivalent in function to the
.B $HOME/.mwmrc
described in the \*(ZM, which provides customisation
abilities for users of the Motif Window Manager.
The
.B $HOME/.dtwmrc
file allows customisation for users of the \*(Zx Window Manager.
It has identical formatting to
.BR $HOME/.mwmrc ,
except that the resource class name
.B Mwm
is replaced by the name
.BR Dtwm .
.br
.mc
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
.mc 2
The data format of the
.B $HOME/.dtwmrc
file is required to allow users to transport their window management
customisations from one conforming system to another.
.P
The CLI to
.Cm dtwm
is not
standardised because it concerns aspects of system administration outside
the scope of the current \*(Zx standardisation effort in X/Open.
.mc
.P
.mc 6
The \*(Zx window manager differs from
.Cm mwm
in its default support of only the Close accelerator because
reducing the use of accelerators avoids conflicts with keyboard accelerators
used by applications.
The user can configure additional accelerators,
but conforming applications cannot rely on them
being present on all desktops.
.br
.mc
.eF e
