.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Help Services"
.xR1 helpsvcs
.H 2 "Introduction"
.mc 4
The \*(Zx help services are a system for
developing online help for any \*(Zx-based application.
It allows authors to write, and developers to integrate, online help
that includes graphics and text formatting, embedded hyperlinks,
and two-way communication with the application.
.P
The services include three main components:
a markup language, an
application program interface, and an action interface.
The
.I HelpTag
markup language is used to author help topics.
It complies with the Standard Generalised Markup Language (SGML),
ISO 8879:\|1986.
Various function calls allow applications to use the help services
directly.
Help actions provide access to help from other components such as
the front panel.
.br
.mc
.H 2 "Widgets" S
.xR2 helpwidgets
This section defines the widget classes
that provide \*(Zx help services
to support application portability at the C-language source level.
.so ../fx/DtHelpDialog.mm
.so ../fx/DtHelpQuickDialog.mm
.ds XC Functions
.SK
.H 2 "Functions" S
.xR2 helpfuncs
This section defines the functions, macros and
external variables that provide \*(Zx help services
to support application portability at the C-language source level.
.so ../fx/DtCreateHelpDialog.mm
.so ../fx/DtCreateHelpQuickDialog.mm
.so ../fx/DtHelpQuickDialogGetChild.mm
.so ../fx/DtHelpReturnSelectedWidgetId.mm
.so ../fx/DtHelpSetCatalogName.mm
.ds XC Headers
.SK
.H 2 "Headers" S
This section describes the contents of headers used
by the \*(Zx help service functions,
macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in
.cX helpfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/DtHelp.mm
.so ../hx/DtHelpDialog.mm
.so ../hx/DtHelpQuickD.mm
.ds XC Actions
.SK
.H 2 "Actions" S
.xR2 helpactions
This section defines the actions that provide \*(Zx help services
to support application portability at the C-language source
or shell script levels.
.so ../hx/dtmanaction.mm
.so ../hx/dthelpaction.mm
.ds XC Formats
.SK
.H 2 "Formats"
.xR2 dthelptagdtd
.so ../fx/dthelptagdtd.mm
.ds XC Capabilities
.SK
.H 2 "Capabilities"
A conforming implementation of the \*(Zx help services
supports at least the following capabilities:
.AL
.LI
Provides both General Help and Quick Help windows with the capabilities
described in the following subsections.
The General Help window provides access to full navigation capabilities.
The Quick Help window provides limited navigation and presentation.
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
.LE
.H 3 "Presentation in the Quick Help Window"
The Quick Help window has the capability to present a help topic.
Help topics can be authored in HelpTag (specified in
.cX dthelptagdtd )
or any of the other help types specified in
.I DtHelpQuickDialog
(see
.cX helpwidgets ).
.H 3 "Navigation in the Quick Help Window"
The user controls navigation of help information.
The Quick Help window allows the user to:
.AL
.LI
Use scroll bars to see portions of the current topic that
do not fit into the current window.
.LI
Traverse hypertext links.
.LI
Return to previously viewed topics.
.LI
Obtain help on using the window.
.LI
Copy text from the current help topic to another window.
.LI
Print text from the current help topic.
.LE
.H 3 "Presentation for the General Help Window"
.xR3 helppres
The General Help window has the capability to present the following:
.AL
.LI
A help topic.
Help topics can be authored in HelpTag (specified in
.cX dthelptagdtd )
or any of the other help types specified in
.I DtHelpQuickDialog
(see
.cX helpwidgets ).
.LI
A list of the HelpTag topics of the current help volume
that can be browsed.
.LI
A list of the topics previously visited.
.LI
A list of the index entries for HelpTag topics of the current volume.
.LE
.H 3 "Navigation for the General Help Window"
The user controls navigation of help information.
The General Help window allows users to:
.AL
.LI
Perform the navigation tasks as specified for the Quick Help window.
.LI
Go to the beginning of the current help volume.
.LI
Select an index entry and go to the topic to which the index refers.
.LI
Access the presentation capabilities listed in
.cX helppres .
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
APIs are required for application portability.
.mc 4
The HelpTag format is required for portability of application help data.
HelpTag can be considered the ``source'' format for
help data; the SDL format used internally in this version
of CDE is an implementation detail that can be considered
the ``object'' format.
Actions are required for application portability.
.mc
Capabilities are required to
ensure that conforming implementations provide the minimum expected
services, without overly constraining tool design.
.P
.ne 5
The following CLIs are not included:
.VL 15
.LI \f2dthelpgen\fP
An administrative utility out of scope.
.LI \f2dthelpprint\fP
This interface is subject to change.
.LI \f2dthelpview\fP
The Action interface is a superior interface for application-portable
access to help services.
.LE
.eF e
