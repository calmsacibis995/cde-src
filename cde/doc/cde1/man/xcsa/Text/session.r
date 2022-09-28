.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Session Management Services"
.xR1 sessmgmtsvcs
.H 2 "Introduction"
.mc 4
The \*(Zx session management services provides ICCCM
Version 1.1 session management during a user's session,
from login to logout.
These services allow for saving a session, restoring a session,
locking a session and launching screen savers.
.P
A session is the collection of applications, settings and
resources that are present on the user's desktop.
Session management is a set of conventions and protocols that allow
a session manager to save and restore a user's session.
.br
.mc
.H 2 "Functions" S
.xR2 sessfuncs
This section defines the functions, macros and
external variables that provide \*(Zx session management services
to support application portability at the C-language source level.
.so ../fx/DtSaverGetWindows.mm
.so ../fx/DtSessionRestorePath.mm
.so ../fx/DtSessionSavePath.mm
.ds XC Headers
.SK
.H 2 "Headers" S
This section describes the contents of headers used
by the \*(Zx session management service functions,
macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in
.cX sessfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/DtSaver.mm
.so ../hx/DtSession.mm
.ds XC Actions
.SK
.H 2 "Actions"
.xR2 sessionmgractions
This section defines the actions that provide \*(Zx session management
services to support application portability at the C-language source
or shell script levels.
.so ../hx/dtsessionaction.mm
.ds XC Capabilities
.SK
.H 2 "Capabilities"
A conforming implementation of the \*(Zx session management services
supports at least the following capabilities:
.AL
.LI
.mc 5
Conforms to the Required items in the Application Style Checklist in
.cX checklist .
.LI
.mc
Has been internationalised using the standard interfaces
in the \*(ZK, the \*(Zl, and the \*(ZM,
and supports any locale available in the underlying X Window System
Xlib implementation.
.LI
Provides ICCCM 1.1 compliant session management (see the \*(Zi).
.LI
Restores an initial
.I home
or
.I current
session at session startup.
.mc 5
See the Style Management Services Capabilities in
.cX stylecaplist
for configuration of session startup.
.LI
.mc 5
Provides session locking on user request.
Some systems may provide a user option to perform
session locking on X server timeout.
.LI
Invokes a session screen saver on user request.
Some systems may provide a user option to perform
screen saving on X server timeout.
.LI
.mc
Saves the
.I home
session at user request or
.I current
session at session exit.
.LI
Displays a confirmation dialog or session selection dialog
at session exit.
.mc 5
See the Style Management Services Capabilities in
.cX stylecaplist
for configuration of session exit.
.LE
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
APIs are required for application portability.
.mc 1
Actions are required for application portability.
The
.Cm dtsession
CLI is not planned for standardisation because Actions
are superior interfaces for this service.
Capabilities are required to
.mc
ensure that conforming implementations provide the minimum expected
services, without overly constraining tool design.
.P
.mc 2
.Fn DtSaverGetWindows
is named as it is to distance itself from
any other component subsystem within \*(Zx.
.mc 1
There are a few future designs that
would make this capability not part of the session manager and changing the
name of this function to
.Fn DtSessionGetWindows
would prohibit those designs.
.br
.mc
.eF e
