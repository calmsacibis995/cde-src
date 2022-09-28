.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.3 94/08/21
This document describes the \*(ZX, a common graphical
user interface environment supported on systems
.mc 2
supporting the X Windows System.
The \*(Zx specification addresses the following standardisation targets:
.mc
.VL 11 5
.LI "Application Portability"
.br
This target requires that all systems provide identical
base interfaces, documenting application program interfaces (APIs),
command-line interfaces (CLIs), and data formats.
Application portability is the primary goal of this document.
.P
The APIs are defined in terms of the source code interfaces
for the C programming language, which is defined in the \*(Z3.
It is possible that some implementations may make the interfaces
available to languages other than C,
but this document does not currently define the source code interfaces
for any other language.
.P
The CLIs are defined using the conventions defined in the \*(ZC,
making them accessible from shell scripts or from C source code
employing such functions as
.Fn system
or
.Fn execlp ,
defined in the \*(ZK.
.P
.mc 2
Some data formats are described in tabular form,
others in syntactical description languages such as BNF.
The method chosen depends on the complexity of the format.
.P
.mc
In some cases, applications can access services via an ``actions
interface,'' as described in
.cX execmgmtchap .
Both APIs and CLIs are provided to actions.
.P
This document allows an application to
be built using a set of services that are consistent across all
systems that conform to this specification (see
.cX conformance ).
Such systems are termed \*(Zx-conformant systems.
Applications written in C or as shell scripts
using only these interfaces
and avoiding implementation-dependent constructs
are portable to all \*(Zx-conformant systems.
.LI "User Portability"
.br
This target requires that all systems provide
identical base functionality with similar look and feel or driveability.
.P
This document attempts to provide adequate user portability
by balancing two competing priorities:
.BL
.LI
The desire for completely identical user
interfaces, down to the exact menu structure, button placement,
and pixel arrangement.
.LI
The desire for flexibility in the interfaces to allow
for innovation in future versions or competing implementations.
.LE
.P
This document achieves this balance by specifying or referring to
detailed style guidelines for system services (the same as those
recommended for applications) and providing detailed ``capability
lists'' that describe the features to be supported by an
interactive service.
The result is that a reasonably competent user would feel comfortable and
.ne 2
productive moving between competing \*(Zx-conformant implementations.
.LI "System Interoperability"
.br
This target requires that all systems intercommunicate using
common data formats and standardised protocols.
This version of this document concentrates on the former,
but includes protocols when they have achieved the level
of stability comparable to the APIs and suitable base
documentation is available.
Note that this situation is similar to that of XPG4,
which does not attempt to document all formats and protocols
for system interoperability.
Future versions of the \*(Zx specification should
provide additional system interoperability details.
.LI "Component Portability/Replaceability"
.br
This target requires that the
interfaces between services and service providers be standardised so
that an Independent Software vendor (ISV)
can market portable replacements for major end-user-visible components,
such as the mail tool or calendar manager.
This target can be considered a special case of Application Portability as
long as there are adequate interfaces to provide comparable end-user
services;
given this, the mail tool or calendar manager are themselves
applications.
.LE
.P
This document defines all of its
stated interfaces and their run-time behaviour
without imposing any particular restrictions on the way in
which the interfaces are implemented.
.P
The following areas are outside the scope of this document:
.VL 11 5
.LI "Binary Portability"
.br
This target was avoided because the binary formats of compiled
or linked applications are system-dependent.
.LI "System Administration"
.br
The details of the installation, maintenance, and performance
tuning of the \*(Zx itself are also system-dependent.
.LE
