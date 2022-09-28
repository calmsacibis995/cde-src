.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.mc 5
.H 1 "Print Job Services"
.xR1 printqsvcs
.nr Ej 1
.H 2 "Introduction"
The \*(Zx print job services provide information to users about printers
and print jobs.
These services have a graphical interface for viewing
the properties of printers and print jobs, and for deleting print jobs.
.br
.mc
.H 2 "Actions" S
.xR2 printjobactions
This section defines the actions
that provide \*(Zx print job services
to support application portability at the C-language source
or shell script levels.
.so ../hx/dtprintinfoaction.mm
.ds XC Capabilities
.SK
.H 2 "Capabilities"
A conforming implementation of the \*(Zx print job services
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
Displays the list of configured printers.
The user can select which printers will be displayed.
.LI
Allows viewing of the list of print jobs for each printer being displayed.
Users can view all jobs or only their own jobs.
.LI
Allows deletion of displayed print jobs, based on
appropriate permissions.
.LI
.mc 5
Allows the user
to display selectively
only one printer upon invocation of the \*(Zx print job services.
.mc
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
.mc 1
No CLI for this service
is planned for standardisation because Actions is a superior
interface for this service.
.br
.mc
.eF e
