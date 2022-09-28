.\" Copyright 1993, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.Ho Preface \n(Pf
.\"	Copyright 1991, X/Open Company Ltd.
.sp 1.5
\f3X/Open\fP
.P
X/Open is an independent, worldwide, open systems organisation
supported by most of the world's largest information systems
suppliers, user organisations and software companies.
Its mission is to bring to users greater value from computing, through 
the practical implementation of open systems.  
.P
X/Open's strategy for achieving this goal is to combine 
existing and emerging standards into a comprehensive, 
integrated, high-value and usable system environment, called the 
Common Applications Environment (CAE).
This environment covers the standards, above the 
hardware level, that are needed to support open systems.  
It provides for portability and interoperability of applications, and 
allows users to move between systems with a minimum of retraining.
.P
The components of the Common Applications Environment are defined in 
X/Open CAE Specifications.
These contain, among other things, an evolving portfolio of 
practical application programming interfaces (APIs), which
significantly enhance portability of application programs at 
the source code level,
and definitions of, and references to, protocols and protocol profiles, which
significantly enhance the interoperability of applications.  
.P
The X/Open CAE Specifications are supported by an extensive set of 
conformance tests and a distinct X/Open trademark\*(EMthe XPG brand\*(EMthat
is licensed by X/Open and may be carried only on products that 
comply with the X/Open CAE Specifications.
.P
The XPG brand, when associated with a vendor's product, 
communicates clearly and unambiguously to a procurer that the software 
bearing the brand correctly implements the corresponding 
X/Open CAE Specifications.
Users specifying XPG-conformance in their procurements are therefore 
certain that the branded products they buy conform to the CAE
Specifications.
.P
X/Open is primarily concerned with the selection and adoption of
standards.
The policy is to use formal approved \f2de jure\fP
standards, where they exist, and to adopt widely supported 
\f2de facto\fP standards in other cases.
.P
Where formal standards do not exist, it is X/Open policy to work 
closely with standards development organisations to assist in the 
creation of formal standards covering the needed functions, and to 
make its own work freely available to such organisations.  
Additionally, X/Open has a commitment to align its definitions 
with formal approved standards.
.bp
\f3X/Open Specifications\fP
.P
There are two types of X/Open specification:
.BL
.LI
\f2CAE Specifications\fP
.P     
CAE (Common Applications Environment) Specifications are the 
long-life specifications that form the basis for conformant and
branded X/Open systems.
They are intended to be used widely 
within the industry for product development and procurement purposes.
.P
Developers who base their products on a current CAE Specification can 
be sure that either the current specification or an upwards-compatible 
version of it will be referenced by a future XPG brand (if not 
referenced already), and that a variety of compatible, XPG-branded 
systems capable of hosting their products will be available, either 
immediately or in the near future.
.P
CAE Specifications are not published to coincide with the launch 
of a particular XPG brand, but are published as soon as they are 
developed.
By providing access to its specifications in
this way, X/Open makes it possible for products that conform
to the CAE (and hence are eligible for a future XPG brand) to be 
developed as soon as practicable, enhancing the value of the XPG 
brand as a procurement aid to users.
.LI
\f2Preliminary Specifications\fP
.P
These are specifications, usually addressing an emerging area of 
technology, and consequently not yet supported by a base of 
conformant product implementations, that are released in a 
controlled manner for the purpose of validation through practical 
implementation or prototyping.
A Preliminary Specification is not a ``draft'' specification.
Indeed, it is as stable as X/Open can 
make it, and on publication has gone through the same rigorous 
X/Open development and review procedures as a CAE Specification.  
.P
Preliminary Specifications are analogous with the ``trial-use'' 
standards issued by formal standards organisations, and product 
development teams are intended to develop products on the basis of them.
However, because of the nature of the technology that
a Preliminary Specification is addressing, it is untried in 
practice and may therefore change before being published as a CAE 
Specification.
In such a case the CAE Specification will be made 
as upwards-compatible as possible with the corresponding Preliminary 
Specification, but complete upwards-compatibility in all cases is 
not guaranteed.
.LE
.P
In addition, X/Open periodically publishes:
.BL
.LI
\f2Snapshots\fP
.P
Snapshots are ``draft'' documents, which provide a mechanism for 
X/Open to disseminate information on its current direction and 
thinking to an interested audience, in advance of formal 
publication, with a view to soliciting feedback and comment. 
.P
A Snapshot represents the interim results of an X/Open technical 
activity.
Although at the time of publication X/Open intends to 
progress the activity towards publication of an X/Open Preliminary 
or CAE Specification, X/Open is a consensus organisation, and 
makes no commitment regarding publication.
.P
Similarly, a Snapshot does not represent any commitment by any 
X/Open member to make any specific products available.
.LE
.sp 1.5
.ne 4
\f3X/Open Guides\fP
.P
X/Open Guides provide information that X/Open believes is useful
in the evaluation, procurement, development or management of open
systems, particularly those that are X/Open-compliant.
.P
X/Open Guides are not normative, and should not be referenced for
purposes of specifying or claiming X/Open-conformance.
.HU "This Document"
This specification is one of a set of X/Open CAE Specifications (see
above) defining the \*(ZX (\*(Zx)\*F requirements:
.FS
These names are tentative.
X/Open and the CDE sponsors will select a unique name for this specification
that can be trademark-protected for branding purposes.
.FE
.BL
.LI
X/Open Common Desktop Environment \(em Definitions and Infrastructure
.if @\*(Zf@@ (this document)
.LI
X/Open Common Desktop Environment \(em Services and Applications
.if !@\*(Zf@@ (this document)
.LE
.P
.mc 2
The \*(ZD document provides common definitions for the \*(ZD and the
\*(ZS; therefore, readers should be familiar with the \*(ZD
before using the \*(ZS.
(Readers are also expected to be familiar with the
.mc
\*(Zb,
which contains a number of applicable definitions.)
This document also describes interfaces available to
both the application program and the \*(ZX Services and Applications.
.so struct.r
.P
Comprehensive references are available in the index.
.HU "Typographical Conventions"
The following typographical conventions are used throughout this document:
.BL
.LI
.B Bold
font is used in text for options to commands, filenames, keywords and
type names.  It is also used to identify brackets that surround optional 
items in syntax, \f3[\|]\fP.
.LI
.I Italic
strings are used for emphasis or 
to identify the first instance of a word requiring
definition.  Italics in text also denote:
.DL
.LI
variable names, for example, substitutable argument prototypes
.LI
environment variables, which are also shown in capitals
.LI
commands or utilities
.LI
external variables, such as
.I errno 
.LI
X Window System widgets
.LI
functions; these are shown as follows:
.Fn name ;
names without parentheses are either external variables or
function family names
.LE
.LI
Normal font is used for the names of constants and literals.
.LI
The notation
.B <file.h>
indicates a header file.
.LI
Names surrounded by braces, for example, {ARG_MAX},
represent symbolic limits or configuration values 
which may be declared in appropriate headers by means of the C
.B #define 
construct.
.LI
Ellipses, .\|.\|., are used to show that additional  
arguments are optional.
.LI
Syntax and code examples are shown in \f5fixed width font\fP.  
Brackets shown in this font, \f5[\|]\fP, are part of the syntax and
do 
.I not
indicate optional items.
.LI
Variables within syntax statements are shown in \f6italic fixed width
font\fP. 
.LI
.mc 6
The names of virtual keys, such as
.I <Help>
or
.I <Insert>
are used as described by the model keyboard section of the \*(ZY.
.LE
.mc
.eF
