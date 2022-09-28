.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.iX "format of entries" "" 1
.iX "manual pages" "format"
.H 3 "C-Language Functions and Headers"
.xR3 funcfmt
The entries for C-language functions and headers
are based on a common format.
.P
.VL 11 5
.LI \f3NAME\fP
.br
This section gives the name or names of the entry and briefly
states its purpose.
.LI \f3SYNOPSIS\fP
.br
This section summarises the use of the entry being described.
If it is necessary to include a header to use this interface,
the names of such headers are shown, for example:
.P
.yS
#include\ <stdio.h>
.yE
.LI "\f3DESCRIPTION\fP"
.br
This section describes the functionality of the interface or header.
.LI "\f3RETURN VALUE\fP"
.br
This section indicates the possible return values, if any.
.P
If the implementation can detect errors, ``successful completion''
means that no error has been detected during execution of the function.
If the implementation does detect an error, the error will be indicated.
.P
For functions where no errors are defined, ``successful completion'' means
that if the implementation checks for errors, no error has been detected.
If the implementation can detect errors, and an error is detected, the
indicated return value will be returned.
No function in this document affects the value of the
.I errno
variable described in the \*(ZK.
.LI "\f3APPLICATION USAGE\fP"
.br
This section gives warnings and advice to application writers
about the entry.
.LI "\f3EXAMPLES\fP"
.br
This section gives examples of usage, where appropriate.
.LI "\f3FUTURE DIRECTIONS\fP"
.br
This section provides comments which should be used as a 
guide to current thinking;
there is not necessarily a commitment to adopt
these future directions.
.LI "\f3SEE ALSO\fP"
.br
This section gives references to related information.
.LI "\f3CHANGE HISTORY\fP"
.br
This section shows the derivation of the entry and any significant 
changes that have been made to it.
.LE
.P
The only sections relating to conformance are the 
.B SYNOPSIS ,
.B DESCRIPTION
and
.B "RETURN VALUE"
sections.
.H 3 "Messages"
The ToolTalk desktop message set (see
.cX desktopmessages )
and media exchange message set (see
.cX mediamessages )
descriptions use a modified version of the
.B SYNOPSIS
notation used for C-language functions.
Despite the similarity of its appearance to a
C-language function prototype,
it represents a message created by the
.Fn tt_create_message
function (or one of the related message functions),
and its arguments are typically added using separate calls
to functions such as
.Fn tt_message_arg_ival_set .
Within the synopsis, the square brackets
.RB ( [\ ] )
surround optional arguments.
.H 3 "Service Interfaces"
In addition to C-language APIs, this document
describes services available to the user and
to applications.
Such services are often represented by a specific utility
program, but are sometimes described in more general terms.
Services are described using the following categories,
expressed in order of increasing specificity:
.VL 11 5
.LI "\f2Capabilities\fP"
.br
A service is required to provide all of the capabilities
within a bulleted list.
These general capabilities are expressed in a manner
that promotes user portability without sacrificing
the ability of implementors to innovate by providing
additional or improved interfaces.
.LI "\f2Actions\fP"
.br
An application can generally access a \*(Zx service
through an action interface; see
.cX execmgmtchap .
When available, the action interface is the preferred
means for a \*(Zx application to access a service.
Actions are described in a format identical to that
used for C-language functions; see
.cX funcfmt .
.LI "\f2Messages\fP"
.br
An application can generally access a \*(Zx service
by sending ToolTalk messages to it and receiving messages in response.
A list is presented of the
ToolTalk messages that are sent by, or can be received by, the service.
(These ToolTalk messages are described in
.cX desktopmessages
and
.cX mediamessages ).
.LI "\f2Command-Line Interfaces\fP"
.br
A service may include one or more utilities.
Some utilities are described with a combination of
a Capability List (described earlier) and a CLI.
The CLI is a subset of the sections described under
.IR "Utility Descriptions" .
.LI "\f2Utility Descriptions\fP"
.br
Some utilities are expected to be used primarily
by portable shell scripts and are presented using
the full template introduced by \*(ZC, enhanced as follows:
.VL 11 5
.LI \f3NAME\fP
.br
This section gives the name or names of the entry and briefly
states its purpose.
.LI \f3SYNOPSIS\fP
.br
The
.B SYNOPSIS
section summarises the syntax of the calling sequence
for the utility, including options, option-arguments and operands.
.LI \f3DESCRIPTION\fP
.br
The
.B DESCRIPTION
section describes the actions of the utility.
If the utility has a very complex set of subcommands
or its own procedural language, an
.B "EXTENDED DESCRIPTION"
section is also provided.
Most explanations of optional functionality are omitted here, as they are
usually explained in the
.B OPTIONS
section.
.LI \f3OPTIONS\fP
.br
The
.B OPTIONS
section describes the utility options and option-arguments,
and how they modify the actions of the utility.
.P
.B "Default Behaviour:"
When this section is listed as ``None'',
it means that the implementation need not support any options.
.LI \f3OPERANDS\fP
.br
The
.B OPERANDS
section describes the utility operands,
and how they affect the actions of the utility.
.P
.B "Default Behaviour:"
When this section is listed as ``None'',
it means that the implementation need not support any operands.
.LI \f3STDIN\fP
.br
The
.B STDIN
section describes the standard input of the utility.
This section is frequently merely a reference to
the following section, as many utilities treat standard input
and input files in the same manner.
Unless otherwise stated, all restrictions described in
.B "INPUT FILES"
apply to this section as well.
.P
.B "Default Behaviour:"
When this section is listed as ``Not used,'' it means
that the standard input will not be read when the utility is used as
described by this document.
.LI "\f3INPUT FILES\fP"
.br
The
.B "INPUT FILES"
section describes the files, other than the
standard input, used as input by the utility.
It includes files named as operands and option-arguments
as well as other files that are referred to, such as
startup and initialisation files, databases, etc.
Commonly-used files are generally described in one place and
cross-referenced by other utilities.
.P
Record formats are described in a notation
similar to that used by the C-language function,
.Fn printf .
See \*(ZC for a description of this notation.
.P
.B "Default Behaviour:"
When this section is listed as ``None'',
it means that no input files
are required to be supplied when
the utility is used as
described by this document.
.LI "\f3RESOURCES\fP"
.br
This section, which has no corresponding section in the \*(ZC,
lists the X Window System resources that affect
the operation of the utility.
.LI "\f3ENVIRONMENT VARIABLES\fP"
.br
The
.B "ENVIRONMENT VARIABLES"
section lists what variables
affect the utility's execution.
.P
.B "Default Behaviour:"
When this section is listed as ``None'', it means
that the behaviour of the utility is not directly affected by 
environment variables described by this document
when the utility is used as described by this document.
.LI "\f3ASYNCHRONOUS EVENTS\fP"
.br
The
.B "ASYNCHRONOUS EVENTS"
section lists how the utility reacts to such
events as signals and what signals are caught.
.P
.B "Default Behaviour:"
When this section is listed as ``Default'',
or it refers to ``the standard action for all other signals,''
it means that the action taken
as a result of the signal is one of the following:
.AL
.LI
The action is that inherited from the parent
according to the rules of inheritance of signal actions defined in the \*(ZK.
.LI
When no action has been taken to change the default, the
default action is that specified by the \*(ZK.
.LI
The result of the utility's execution 
is as if default actions had been taken.
.LE
.P
A utility
is permitted to catch a signal, perform some additional processing
(such as deleting temporary files), restore the default signal action
(or action inherited from the parent process) and resignal itself.
.LI \f3STDOUT\fP
.br
The
.B STDOUT
section describes the standard output
of the utility.
.P
.B "Default Behaviour:"
When this section is listed as ``Not used'',
it means that the standard output will not be written
when the utility is used as described by this document.
.LI \f3STDERR\fP
.br
The
.B STDERR
section describes the standard error output
of the utility.
Only those messages that are purposely sent by the
utility are described.
.P
.B "Default Behaviour:"
When this section is listed as ``Used only for diagnostic messages,''
it means that, unless otherwise stated,
the diagnostic messages are sent
to the standard error only when the exit status is non-zero
and the utility is used as described by this document.
.P
When this section is listed as ``Not used'', 
it means that the standard error will not be used
when the utility is used as described in this document.
.LI "\f3OUTPUT FILES\fP"
.br
The
.B "OUTPUT FILES"
section describes the files created or modified
by the utility.
.P
Record formats are described in a notation
similar to that used by the C-language function,
.Fn printf .
See the \*(ZC for a description of this notation.
.P
.B "Default Behaviour:"
When this section is listed as ``None'',
it means
that no files are created or modified as a consequence of direct
action on the part of the utility when the utility is used as described
by this document.
However, the utility may create or modify system files,
such as log files, that are outside the utility's normal
execution environment.
.LI "\f3EXTENDED DESCRIPTION\fP"
.br
The
.B "EXTENDED DESCRIPTION"
section provides a place for describing the actions of very
complicated utilities, such as text editors or language processors,
which typically have elaborate command languages.
.P
.B "Default Behaviour:"
When this section is listed as ``None'',
no further description is necessary.
.LI "\f3EXIT STATUS\fP"
.br
The
.B "EXIT STATUS"
section describes the values the utility will return to the
calling program, or shell,
and the conditions that cause these values to be returned.
Usually, utilities return zero for successful completion
and values greater than zero for various error conditions.
If specific numeric values are listed in this section,
the system will use those values for the
errors described.
In some cases, status values are listed more loosely,
such as ``>0''.
A portable application
cannot rely on any specific value in the range
shown and must be prepared to receive any value in the range.
.LI "\f3CONSEQUENCES OF ERRORS\fP"
.br
The
.B "CONSEQUENCES OF ERRORS"
section describes the effects on the environment, file systems,
process state, and so on, when error conditions occur.
It does not describe error messages produced
or exit status values used.
.P
When a utility encounters an error condition several actions are
possible, depending on the severity of the error and the state of the utility.
Included in the possible actions of various utilities are:
deletion of temporary or intermediate work files; deletion of
incomplete files; validity checking of the file system or directory.
.P
.B "Default Behaviour:"
When this section is listed as ``Default'',
it means that
any changes to the environment are unspecified.
.LI "\f3APPLICATION USAGE\fP"
.br
The
.B "APPLICATION USAGE"
section gives advice to the application programmer
or user about the way the utility should be used.
.LI \f3EXAMPLES\fP
.br
The
.B EXAMPLES
section gives one or more examples of usage, where appropriate.
.LI "\f3FUTURE DIRECTIONS\fP"
.br
The
.B "FUTURE DIRECTIONS"
section
should be used as a guide to current thinking;
there is not necessarily a commitment to implement
all of these future directions in their entirety.
.LI "\f3SEE ALSO\fP"
.br
The
.B "SEE ALSO"
section lists related entries.
.LI "\f3CHANGE HISTORY\fP"
.br
This section shows the derivation of the entry and any significant 
changes that have been made to it.
.LE
.P
In this list, all sections other than
.BR "APPLICATION USAGE" ,
.BR EXAMPLES ,
.BR "FUTURE DIRECTIONS" ,
.BR "SEE ALSO"
and
.BR "CHANGE HISTORY"
are related to conformance.
.LE
.eF e
