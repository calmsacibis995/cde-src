.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Application Integration Services"
.nr Ej 1
.xR1 appintegsvcs
.H 2 "Introduction"
.mc 3
The \*(Zx application integration services allow application developers and
system administrators to integrate applications into the \*(ZX.
These services provide a utility for making an
application's actions and datatypes, icons and help volumes available for
use in the desktop.
.br
.mc
.H 2 "Command-Line Interfaces" S
This section defines the utility that provides
\*(Zx application integration services.
.so ../fx/dtappintegrate.mm
.ds XC Actions
.SK
.H 2 "Actions"
.xR2 appmgractions
This section defines the actions
that provide \*(Zx application integration services
to support application portability at the C-language source
or shell script levels.
.so ../hx/dtappaction.mm
.ds XC Rationale
.SK
.H 2 "Rationale and Open Issues"
.I
The following is text copied from the
.R
CDE X/Open Specification Framework Proposal, Draft 2.
.I
It is not intended to remain in this specification when it is published
by X/Open.
.R
.P
.mc 4
The CLI and Actions are required for application portability.
In earlier drafts, a Capabilities section was promised in error.
This was removed because the man page for the utility
and the action provide a full specification.
.br
.mc
.eF e
