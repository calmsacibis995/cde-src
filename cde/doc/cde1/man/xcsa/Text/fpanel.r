.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Front Panel Services"
.nr Ej 1
.xR1 frontpsvcs
.H 2 "Introduction"
.mc 5
The \*(Zx front panel services provide
a key aspect of the \*(Zx user interface.
The front panel
appears in every workspace and provides access to the most commonly used
facilities in the desktop.
Considerable customisation is available to meet specific needs.
.br
.mc
.H 2 "Formats" S
.so ../fx/dtfpfile.mm
.H 2 "Capabilities"
A conforming implementation of the \*(Zx front panel services
supports at least the following capabilities:
.AL
.LI
Provides front panel services
as described in the following subsections.
.LI
Conforms to the Required items in the Application Style Checklist in
.cX checklist .
.LI
Has been internationalised using the standard interfaces
in the \*(ZK, the \*(Zl, and the \*(ZM,
and supports any locale available in the underlying X Window System
Xlib implementation.
.LE
.H 3 "General Layout"
The default configuration of the front panel is as follows:
.AL
.LI
The front panel appears in every workspace.
.LI
The front panel includes controls for the most commonly used actions,
as listed in the next section.
Selecting a control invokes the action associated with the control.
.LI
Controls in the front panel may have indicators associated
with them whose selection causes a subpanel to appear.
.LI
Subpanels contain controls that provide access to additional actions.
.LI
Subpanels may have a special control that adds an icon to the subpanel.
The user can drop an icon on the special control,
which causes that icon to become a control in the subpanel.
.LI
The user can move and iconify the front panel.
.LE
.H 3 "Special Controls"
The front panel services provide the following special controls by default:
.AL
.LI
The file manager control depicts a file folder.
When it is selected,
it invokes the
.B DtfileHome
action (see
.cX filemgractions ),
which opens a view of the user's home directory.
.LI
The lock control depicts a lock.
When it is selected, it
invokes the
.B LockDisplay
action (see
.cX sessionmgractions ),
which
prevents access to the system until the user's password is entered.
.LI
The exit control, when it is selected,
invokes the
.B ExitSession
action (see
.cX sessionmgractions ),
which logs the user out of the current desktop session.
.LI
The printer control prints files that are dropped on it,
by invoking the
.mc 6
.B Print
action (see
.cX printjobactions ).
At least text files are supported.
.mc
Its icon shows a printer.
.LI
The application manager control,
when it is selected,
invokes the
.B Dtappmgr   
action (see
.cX appmgractions ),
which shows applications
installed on the system.
.LI
The help control, when it is selected,
invokes the
.B Dthelpview   
action (see
.cX helpactions ),
which displays the top-level help topic for the desktop.
.LI
The workspace controls move users among available workspaces.
There is one control for each workspace.
Selecting a workspace control moves the user into that workspace.
.LI
The launch light indicates that an action is in the process
of starting.
.LI
The trash can control removes files that are dropped on it
by invoking the
.B Dttrash
action (see
.cX filemgractions ).
.LE
.H 3 "Other Capabilities"
The following additional capabilities are provided:
.AL
.LI
Animation for drag and drop actions.
.LI
The user can add and delete workspaces.
.LI
The user can provide custom names for each workspace.
.LI
The user can add and remove subpanels.
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
The format of the FPLS is required for users to customise their front
panels.
The
.Cm dtfplist
CLI is not planned for standardisation because it is an administrative
utility out of scope.
.eF e
