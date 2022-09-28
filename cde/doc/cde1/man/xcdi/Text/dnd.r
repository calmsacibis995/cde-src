.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Drag and Drop"
.xR1 dndchap
.nr Ej 1
.H 2 "Introduction"
.mc 3
The \*(Zx drag and drop services defined in this chapter
are an extension of the
Motif drag and drop services defined in the \*(ZM.
Convenience APIs are included that reduce programming complexity
for common operations, such as the dragging of files.
.br
.mc
.H 2 "Functions"
.xR2 dndfuncs
This section defines the functions, macros and
external variables that provide \*(Zx drag and drop services
to support application portability at the C-language source level.
.so ../fx/DtDndCreateSourceIcon.mm
.so ../fx/DtDndDragStart.mm
.so ../fx/DtDndDropRegister.mm
.ds XC Headers
.SK
.H 2 "Headers"
This section describes the contents of headers used
by the \*(Zx drag and drop functions, macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in 
.cX dndfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/DtDnd.mm
.ds XC Protocols
.SK
.H 2 "Protocols"
.so ../fx/DtDndProtocol.mm
.H 2 "Rationale and Open Issues" S
.I
The following is text copied from the
.R
CDE X/Open Specification Framework Proposal, Draft 2.
.I
It is not intended to remain in this specification
when it is published by X/Open.
.R
.P
APIs are required for application portability.
The protocol definition
is required because the ICCCM target space for using X selections to
transfer data is expanded in \*(Zx to include application-named data.
(A further extension to support transfer of non-file container objects is
being designed, but is not ready for standardisation.)
.P
.mc 3
A Policies section in Draft 2 that stated
``Text describing the policy that the preregister mechanism must be used
by portable applications instead of the dynamic drag and drop mechanism
will be provided \*(El'' was in error and has been omitted;
there is no such requirement.
.P
A protocol for the transfer of multiple heterogeneous data
may be introduced in a future draft.
.br
.mc
.eF e
