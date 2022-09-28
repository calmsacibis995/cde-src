.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Action Creation Services"
.nr Ej 1
.xR1 actionsvcs
.H 2 "Introduction"
.mc 3
The \*(Zx action creation services allow users to create and modify actions
and datatypes that are used in the \*(ZX.
Actions provide the ability to associate an
application with an icon on the desktop.
Datatypes provide the ability to associate a data file possessing specific
attributes with an icon on the desktop.
.P
This service has a graphical interface for gathering information about the
actions and datatypes the user is defining.
The interface allows the user to
define an application action and the datatypes for the data files associated
with that application.
.P
Users can choose to define actions and datatypes
for personal or system-wide use.
Actions and datatypes created for system-wide use should be installed
using the
.Cm dtappintegrate
utility (see
.cX appintegsvcs ).
.br
.mc
.H 2 "Actions" S
This section defines the actions that provide \*(Zx action creation
services to support application portability at the C-language source
or shell script levels.
.so ../hx/dtactionaction.mm
.ds XC Capabilities
.SK
.H 2 "Capabilities"
A conforming implementation of the \*(Zx action creation services
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
Creates action and datatype definitions files.
.LI
Allows modification of action and datatype definition files that were
previously created using the \*(Zx action creation services.
.LI
Allows specification in an action definition of the application, command,
shell script, etc., to be executed when the action is opened.
.LI
Allows specification in an action definition of the window type
(graphical, terminal emulator or no output) to be used for
displaying output when the action is opened.
.LI
Allows specification in an action definition of the datatypes, such as
an application's data files, that use the action.
.LI
Allows specification in an action definition of the icon to be used
for representing the action.
.LI
.mc 5
Allows specification in an action definition of the icon label to be used
for representing the action.
.LI
.mc
Allows specification in an action definition of help information to be
associated with the action icon.
.LI
Allows specification in a datatype definition of the datatype
characteristics based on name pattern, file permission pattern
and/or file contents.
.LI
Allows specification in a datatype definition of the command to be used
to print the datatype.
.LI
Allows specification in a datatype definition of the icon to be used
for representing the datatype.
.LI
Allows specification in a datatype definition of help information to be
associated with the datatype icon.
.LI
Allows for invocation of the
.B ReloadApps
action, so that newly created or
modified actions and datatypes become immediately available.
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
Capabilities are required to ensure that conforming implementations
provide the minimum expected services, without overly constraining tool
design.
.mc 5
CLIs are not planned for standardisation because Actions
are superior interfaces for these services.
.br
.mc
.eF e
