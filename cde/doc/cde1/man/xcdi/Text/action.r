.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Execution Management"
.nr Ej 1
.xR1 execmgmtchap
.H 2 "Introduction"
The \*(Zx execution management service is the infrastructure and API
that can send a message to, or invoke a process on, the appropriate
system with the necessary supporting environment (for example,
running inside a terminal emulator).
.H 3 Scope
The execution management service allows existing command-line or
message-based applications and utilities to be encapsulated in an
object-oriented manner such that they can be accessed from the
desktop or other software.
.H 3 Components
The major components of the \*(Zx execution management service are:
.VL 11 5
.LI "\f3action database\fP"
.br
The database that defines actions that encapsulate applications
and utilities.
This database can be distributed across multiple systems and
customised on a network, system or personal scope.
.LI "\f3action invocation library\fP"
.br
The client-side library that provides an API for loading the
database, querying the database and invoking actions.
.LI "\f3dtexec client\fP"
.br
The utility that controls processes spawned as the result of
invoking an action and reports changes in their status.
The interface to the
.Cm dtexec
client is implementation-specific.
.LE
.H 3 "Action Database Entries"
Entries in the action database provide the following information:
.VL 11 5
.LI \f3icon\fP
The icons that represent the action.
.LI \f3arguments\fP
.br
The number and kind of arguments that the action accepts.
.LI \f3description\fP
.br
A textual description of the action.
.LI \f3type\fP
Whether the action encapsulates a command line or a message.
.LE
.P
Message-based actions provide additional information that
specifies the type of message to be sent and the details of
the message.
.H 3 "Command-Line Actions"
Command-line actions provide some or all of the following
additional information:
.VL 11 5
.LI \f3window\ type\fP
.br
The type of window support (for example, none or a terminal
emulator) required by the command.
.LI \f3directory\fP
.br
The current working directory where the command must execute.
.LI \f3execution\ host\fP
.br
The host (or list of possible hosts) where the command must execute.
.LI \f3command\fP
.br
The command line that is to be executed.
.LE
.H 2 "Functions"
.xR2 actionfuncs
This section defines the functions, macros and
external variables that provide \*(Zx execution management services
to support application portability at the C-language source level.
.so ../fx/DtActionCallbackProc.mm
.so ../fx/DtActionDescription.mm
.so ../fx/DtActionExists.mm
.so ../fx/DtActionIcon.mm
.so ../fx/DtActionInvoke.mm
.so ../fx/DtActionLabel.mm
.so ../fx/DtDbLoad.mm
.so ../fx/DtDbReloadNotify.mm
.ds XC Headers
.SK
.H 2 "Headers"
This section describes the contents of headers used
by the \*(Zx execution management functions, macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in 
.cX actionfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/DtAction.mm
.ds XC Command-Line Interfaces
.SK
.H 2 "Command-Line Interfaces"
This section defines the utilities that provide
\*(Zx execution management services.
.so ../fx/dtaction.mm
.ds XC Data Formats
.SK
.H 2 "Data Formats"
.xR2 dtactionfile
.so ../fx/dtactionfile.mm
.ds XC Rationale
.SK
.H 2 "Rationale and Open Issues"
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
CLIs are required for application portability.
Formats are required for application portability and interoperability.
Protocols are required for interoperability;
the subprocess control daemon and its protocol are
not planned for standardisation because they are likely to change in an
early revision and no suitable base document is available.
.P
.mc 5
In Draft 5, two functions were removed:
.Fn DtActionQuit
and
.Fn DtActionQuitType .
These were originally designed for use by the \*(Zx mail services,
but the mail design changed and no other component uses them.
Therefore, they were removed from the sample implementation
and would be inappropriate for standardisation.
.br
.mc
.eF e
