.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Message Services"
.xR1 msgchap
.H 2 "Introduction"
The \*(Zx messaging service provides APIs and
supporting components for passing multicast and point-to-point messages
between desktop applications, both across networks and within hosts.
Message protocols, defined in
.cX protomsgs ,
use the messaging service to achieve control and data integration
between desktop applications.
.P
The messaging service is based on pattern matching:
applications wishing to receive messages
register patterns that describe the desired messages;
applications sending messages format the message as a description of
the service being requested or the event being announced.
The messaging service then routes the messages to the interested application.
Since applications need not directly address each other, this provides the
ability to restructure applications and swap in different
implementations without modifying other applications.
.P
The messaging service
supports messaging both within a single user's session
(session-scoped messaging) and between users (file-scoped messaging).
The messaging service explicitly supports messages that ask for services
(requests, and the associated replies), and messages that announce
events (notices).
.P
The messaging service permits applications to register
patterns at installation time;
by consulting these patterns, the
messaging service can determine that an application not currently running
is interested in the message, and start the application.
.P
The \*(Zx messaging service
is based on the ToolTalk facilities from Sun Microsystems, Inc.;
this document generally refers to the messaging service as
``the ToolTalk service.''
.H 2 "Functions" S
.xR2 ttfuncs
This section defines the functions, macros and
external variables that provide \*(Zx message services
to support application portability at the C-language source level.
.so ../fx/tt_X_session.mm
.so ../fx/tt_bcontext_join.mm
.so ../fx/tt_bcontext_quit.mm
.so ../fx/tt_close.mm
.so ../fx/tt_context_join.mm
.so ../fx/tt_context_quit.mm
.so ../fx/tt_default_file.mm
.so ../fx/tt_default_file_set.mm
.so ../fx/tt_default_procid.mm
.so ../fx/tt_default_procid_set.mm
.so ../fx/tt_default_ptype.mm
.so ../fx/tt_default_ptype_set.mm
.so ../fx/tt_default_session.mm
.so ../fx/tt_default_session_set.mm
.so ../fx/tt_error_int.mm
.so ../fx/tt_error_pointer.mm
.so ../fx/tt_fd.mm
.so ../fx/tt_file_copy.mm
.so ../fx/tt_file_destroy.mm
.so ../fx/tt_file_join.mm
.so ../fx/tt_file_move.mm
.so ../fx/tt_file_netfile.mm
.so ../fx/tt_file_objects_query.mm
.so ../fx/tt_file_quit.mm
.so ../fx/tt_free.mm
.so ../fx/tt_host_file_netfile.mm
.so ../fx/tt_host_netfile_file.mm
.so ../fx/tt_icontext_join.mm
.so ../fx/tt_icontext_quit.mm
.so ../fx/tt_initial_session.mm
.so ../fx/tt_int_error.mm
.so ../fx/tt_is_err.mm
.so ../fx/tt_malloc.mm
.so ../fx/tt_mark.mm
.so ../fx/tt_message_accept.mm
.so ../fx/tt_message_address.mm
.so ../fx/tt_message_address_set.mm
.so ../fx/tt_message_arg_add.mm
.so ../fx/tt_message_arg_bval.mm
.so ../fx/tt_message_arg_bval_set.mm
.so ../fx/tt_message_arg_ival.mm
.so ../fx/tt_message_arg_ival_set.mm
.so ../fx/tt_message_arg_mode.mm
.so ../fx/tt_message_arg_type.mm
.so ../fx/tt_message_arg_val.mm
.so ../fx/tt_message_arg_val_set.mm
.so ../fx/tt_message_arg_xval.mm
.so ../fx/tt_message_arg_xval_set.mm
.so ../fx/tt_message_args_count.mm
.so ../fx/tt_message_barg_add.mm
.so ../fx/tt_message_bcontext_set.mm
.so ../fx/tt_message_callback_add.mm
.so ../fx/tt_message_class.mm
.so ../fx/tt_message_class_set.mm
.so ../fx/tt_message_context_bval.mm
.so ../fx/tt_message_context_ival.mm
.so ../fx/tt_message_context_set.mm
.so ../fx/tt_message_context_slotname.mm
.so ../fx/tt_message_context_val.mm
.so ../fx/tt_message_context_xval.mm
.so ../fx/tt_message_contexts_count.mm
.so ../fx/tt_message_create.mm
.so ../fx/tt_message_create_super.mm
.so ../fx/tt_message_destroy.mm
.so ../fx/tt_message_disposition.mm
.so ../fx/tt_message_disposition_set.mm
.so ../fx/tt_message_fail.mm
.so ../fx/tt_message_file.mm
.so ../fx/tt_message_file_set.mm
.so ../fx/tt_message_gid.mm
.so ../fx/tt_message_handler.mm
.so ../fx/tt_message_handler_ptype.mm
.so ../fx/tt_message_handler_ptype_set.mm
.so ../fx/tt_message_handler_set.mm
.so ../fx/tt_message_iarg_add.mm
.so ../fx/tt_message_icontext_set.mm
.so ../fx/tt_message_id.mm
.so ../fx/tt_message_object.mm
.so ../fx/tt_message_object_set.mm
.so ../fx/tt_message_op.mm
.so ../fx/tt_message_op_set.mm
.so ../fx/tt_message_opnum.mm
.so ../fx/tt_message_otype.mm
.so ../fx/tt_message_otype_set.mm
.so ../fx/tt_message_pattern.mm
.so ../fx/tt_message_print.mm
.so ../fx/tt_message_receive.mm
.so ../fx/tt_message_reject.mm
.so ../fx/tt_message_reply.mm
.so ../fx/tt_message_scope.mm
.so ../fx/tt_message_scope_set.mm
.so ../fx/tt_message_send.mm
.so ../fx/tt_message_send_on_exit.mm
.so ../fx/tt_message_sender.mm
.so ../fx/tt_message_sender_ptype.mm
.so ../fx/tt_message_sender_ptype_set.mm
.so ../fx/tt_message_session.mm
.so ../fx/tt_message_session_set.mm
.so ../fx/tt_message_state.mm
.so ../fx/tt_message_status.mm
.so ../fx/tt_message_status_set.mm
.so ../fx/tt_message_status_string.mm
.so ../fx/tt_message_status_string_set.mm
.so ../fx/tt_message_uid.mm
.so ../fx/tt_message_user.mm
.so ../fx/tt_message_user_set.mm
.so ../fx/tt_message_xarg_add.mm
.so ../fx/tt_message_xcontext_join.mm
.so ../fx/tt_message_xcontext_set.mm
.so ../fx/tt_netfile_file.mm
.so ../fx/tt_objid_equal.mm
.so ../fx/tt_objid_objkey.mm
.so ../fx/tt_onotice_create.mm
.so ../fx/tt_open.mm
.so ../fx/tt_orequest_create.mm
.so ../fx/tt_otype_base.mm
.so ../fx/tt_otype_derived.mm
.so ../fx/tt_otype_deriveds_count.mm
.so ../fx/tt_otype_hsig_arg_mode.mm
.so ../fx/tt_otype_hsig_arg_type.mm
.so ../fx/tt_otype_hsig_args_count.mm
.so ../fx/tt_otype_hsig_count.mm
.so ../fx/tt_otype_hsig_op.mm
.so ../fx/tt_otype_is_derived.mm
.so ../fx/tt_otype_opnum_callback_add.mm
.so ../fx/tt_otype_osig_arg_mode.mm
.so ../fx/tt_otype_osig_arg_type.mm
.so ../fx/tt_otype_osig_args_count.mm
.so ../fx/tt_otype_osig_count.mm
.so ../fx/tt_otype_osig_op.mm
.so ../fx/tt_pattern_address_add.mm
.so ../fx/tt_pattern_arg_add.mm
.so ../fx/tt_pattern_barg_add.mm
.so ../fx/tt_pattern_bcontext_add.mm
.so ../fx/tt_pattern_callback_add.mm
.so ../fx/tt_pattern_category.mm
.so ../fx/tt_pattern_category_set.mm
.so ../fx/tt_pattern_class_add.mm
.so ../fx/tt_pattern_context_add.mm
.so ../fx/tt_pattern_create.mm
.so ../fx/tt_pattern_destroy.mm
.so ../fx/tt_pattern_disposition_add.mm
.so ../fx/tt_pattern_file_add.mm
.so ../fx/tt_pattern_iarg_add.mm
.so ../fx/tt_pattern_icontext_add.mm
.so ../fx/tt_pattern_object_add.mm
.so ../fx/tt_pattern_op_add.mm
.so ../fx/tt_pattern_opnum_add.mm
.so ../fx/tt_pattern_otype_add.mm
.so ../fx/tt_pattern_print.mm
.so ../fx/tt_pattern_register.mm
.so ../fx/tt_pattern_scope_add.mm
.so ../fx/tt_pattern_sender_add.mm
.so ../fx/tt_pattern_sender_ptype_add.mm
.so ../fx/tt_pattern_session_add.mm
.so ../fx/tt_pattern_state_add.mm
.so ../fx/tt_pattern_unregister.mm
.so ../fx/tt_pattern_user.mm
.so ../fx/tt_pattern_user_set.mm
.so ../fx/tt_pattern_xarg_add.mm
.so ../fx/tt_pattern_xcontext_add.mm
.so ../fx/tt_pnotice_create.mm
.so ../fx/tt_pointer_error.mm
.so ../fx/tt_prequest_create.mm
.so ../fx/tt_ptr_error.mm
.so ../fx/tt_ptype_declare.mm
.so ../fx/tt_ptype_exists.mm
.so ../fx/tt_ptype_opnum_callback_add.mm
.so ../fx/tt_ptype_undeclare.mm
.so ../fx/tt_release.mm
.so ../fx/tt_session_bprop.mm
.so ../fx/tt_session_bprop_add.mm
.so ../fx/tt_session_bprop_set.mm
.so ../fx/tt_session_join.mm
.so ../fx/tt_session_prop.mm
.so ../fx/tt_session_prop_add.mm
.so ../fx/tt_session_prop_count.mm
.so ../fx/tt_session_prop_set.mm
.so ../fx/tt_session_propname.mm
.so ../fx/tt_session_propnames_count.mm
.so ../fx/tt_session_quit.mm
.so ../fx/tt_session_types_load.mm
.so ../fx/tt_spec_bprop.mm
.so ../fx/tt_spec_bprop_add.mm
.so ../fx/tt_spec_bprop_set.mm
.so ../fx/tt_spec_create.mm
.so ../fx/tt_spec_destroy.mm
.so ../fx/tt_spec_file.mm
.so ../fx/tt_spec_move.mm
.so ../fx/tt_spec_prop.mm
.so ../fx/tt_spec_prop_add.mm
.so ../fx/tt_spec_prop_count.mm
.so ../fx/tt_spec_prop_set.mm
.so ../fx/tt_spec_propname.mm
.so ../fx/tt_spec_propnames_count.mm
.so ../fx/tt_spec_type.mm
.so ../fx/tt_spec_type_set.mm
.so ../fx/tt_spec_write.mm
.so ../fx/tt_status_message.mm
.so ../fx/tt_trace_control.mm
.so ../fx/tt_xcontext_quit.mm
.so ../fx/ttdt_Get_Modified.mm
.so ../fx/ttdt_Revert.mm
.so ../fx/ttdt_Save.mm
.so ../fx/ttdt_close.mm
.so ../fx/ttdt_file_event.mm
.so ../fx/ttdt_file_join.mm
.so ../fx/ttdt_file_notice.mm
.so ../fx/ttdt_file_quit.mm
.so ../fx/ttdt_file_request.mm
.so ../fx/ttdt_message_accept.mm
.so ../fx/ttdt_open.mm
.so ../fx/ttdt_sender_imprint_on.mm
.so ../fx/ttdt_session_join.mm
.so ../fx/ttdt_session_quit.mm
.so ../fx/ttdt_subcontract_manage.mm
.so ../fx/ttmedia_Deposit.mm
.so ../fx/ttmedia_load.mm
.so ../fx/ttmedia_load_reply.mm
.so ../fx/ttmedia_ptype_declare.mm
.so ../fx/tttk_Xt_input_handler.mm
.so ../fx/tttk_block_while.mm
.so ../fx/tttk_message_abandon.mm
.so ../fx/tttk_message_create.mm
.so ../fx/tttk_message_destroy.mm
.so ../fx/tttk_message_fail.mm
.so ../fx/tttk_message_reject.mm
.so ../fx/tttk_op_string.mm
.so ../fx/tttk_string_op.mm
.ds XC Headers
.SK
.H 2 "Headers"
.xR2 tthdrs
This section describes the contents of headers used
by the \*(Zx message service functions, macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in
.cX ttfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/Tttt_c.mm
.so ../hx/Tttttk.mm
.ds XC Command-Line Interfaces
.SK
.H 2 "Command-Line Interfaces"
This section defines the utilities that provide \*(Zx message services.
.so ../fx/tt_type_comp.mm
.so ../fx/ttcp.mm
.so ../fx/ttmv.mm
.so ../fx/ttrm.mm
.so ../fx/ttrmdir.mm
.so ../fx/ttsession.mm
.so ../fx/tttar.mm
.ds XC Data Formats
.SK
.H 2 "Data Formats"
.xR2 msgdatafmts
This section defines the data formats of static message patterns,
used in ptype and otype files.
.P
The static messaging method allows an application
to specify the message
pattern information if it wants to receive a defined set of messages.
To use the static method, the application
must define its process types and object types
and compile them with the ToolTalk type compiler,
.Cm tt_type_comp .
When the application declares
its process type, the ToolTalk service creates
message patterns based on that type.
These static message patterns
remain in effect until the application closes
communication with the ToolTalk service.
.H 3 "Defining Process Types
An application can be considered a potential message receiver
even when no process is running the application.
To do this, the application developer must
provide message patterns and instructions on how to start the
application in a process type (ptype) file.
These instructions tell
the ToolTalk service to perform one of the following actions when a
message is available for an application but the application is not
running:
.BL
.LI
Start the application and deliver the message
.LI
Queue the message until the application is running
.LI
Discard the message
.LE
.P
To make the information available to the ToolTalk service, the ptype
file must be compiled with the ToolTalk type compiler,
.Cm tt_type_comp ,
at application installation time.
.P
When an application registers a ptype with the ToolTalk service, the
message patterns listed in it are automatically registered, too.
.P
Ptypes provide application information that the ToolTalk service can
use when the application is not running.
This information is used to
start your process if necessary to receive a message or queue messages
until the process starts.
.P
A ptype begins with a process-type identifier (ptid).
Following the ptid are:
.AL
.LI
An optional start command string, which the ToolTalk service will execute,
if necessary, to start a process running the program.
.LI
Signatures, which describe the TT_PROCEDURE-addressed messages that the
program wants to receive.
Messages to be observed are described
separately from messages to be handled.
.LE
.HU "Signatures
Signatures describe the messages that the program wants to receive.
A signature is divided by an arrow (the two characters
.Cw => )
into two parts.
The first part of a signature specifies matching attribute values.
The more attribute values specified in a signature,
the fewer messages the signature will match.
The second part of a signature specifies receiver values that
the ToolTalk service will copy into messages that match the first part
of the signature.
.P
A ptype signature can contain values for disposition and operation
numbers (opnums).
The ToolTalk service uses the disposition value
.RB ( start ,
.B queue
or the default
.BR discard )
to determine what to do with a
message that matches the signature when no process is running the program.
The opnum value is provided as a convenience to message receivers.
When two signatures have the same operation name but
different arguments, different opnums makes incoming messages easy to
identify.
.P
The following is the syntax for a ptype file.
.Cs I
ptype		::=	'ptype' ptid `{'
			\f6property\fP*
			[`observe:' \f6psignature\fP*]
			[`handle:' \f6psignature\fP* ]
			`}' [`;']
property	::=	\f6property_id\fP \f6value\fP `;'
property_id	::=	`start'
value		::=	\f6string\fP
ptid		::=	\f6identifier\fP
psignature	::=	[\f6scope\fP] \f6op\fP \f6args\fP [\f6contextdcl\fP]
			[`=>'
			[`start'][`queue']
			[`opnum='\f6number\fP]]
			`;'
scope		::=	`file'
		|	`session'
		|	`file_in_session'
args		::=	`(' \f6argspec\fP {, \f6argspec\fP}* `)'
		|	`(void)'
		|	`()'
contextdcl	::=	`context' `(' \f6identifier\fP {, \f6identifier\fP}* `)' `;'
argspec	::=	\f6mode\fP \f6type\fP \f6name\fP
mode		::=	`in' | `out' | `inout'
type		::=	\f6identifier\fP
name		::=	\f6identifier\fP
.Ce
.HU "Property_id Information
.VL 8
.LI ptid
The process type identifier (ptid)
identifies the process type.
A ptid must be unique for every implementation.
It is recommended that the name selected include the trademarked name of
the application product or company as a prefix.
The ptid cannot exceed
32 bytes and must not be one of the reserved identifiers:
.BR ptype ,
.BR otype ,
.BR start ,
.BR opnum ,
.BR queue ,
.BR file ,
.BR session ,
.B observe
or
.BR handle .
.LI start
The start string for the process.
If the ToolTalk service needs to start a
process, it executes this command using the shell described in the \*(ZC.
.P
Before executing the command, the ToolTalk service defines
.I TT_FILE
as an environment variable with the value of the file attribute of the
message that started the application.
This command runs in the
environment of
.Cm ttsession ,
not in the environment of the sender of the
message that started the application, so any context information must
be carried by message arguments or contexts.
.LE
.HU "Psignature Matching Information
.VL 8
.LI scope
This pattern attribute is matched against the scope attribute in messages.
.LI op
Operation name.
This name is matched against the op attribute in messages.
.P
.Ns
If the application specifies
message signatures in both its ptype and otypes,
it must use unique operation names in each.
For example, it cannot specify a
display operation in both its ptype and otype.
.Ne
.LI args
Arguments for the operation.
If the argument list is
.BR (void) ,
the signature matches only messages with no arguments.
If the argument list is empty
(that is, ``(\|)''), the signature matches without regard to the arguments.
.LI contextdcl
.br
The context name.
When a pattern with this named
context is generated from the signature, it contains an empty value list.
.LE
.HU "Psignature Actions Information
.VL 8
.LI start
If the psignature matches a message and no running process of this
ptype has a pattern that matches the message,
the ToolTalk service starts a process of this ptype.
.LI queue
If the psignature matches a message and no running process of this
ptype has a pattern that matches the message,
the ToolTalk service queues the message until a
process of this ptype registers a pattern that matches it.
.LI opnum
The application
should fill in the message's opnum attribute with the specified number to
enable it to identify the signature that matched the message.
When the message matches the signature, the
the ToolTalk service fills the
opnum from the signature into the message.
The application can then retrieve the opnum with the
.Fn tt_message_opnum
call.
By giving each signature a unique
opnum, the application
can determine which signature matched the message.
It can attach a callback routine to the opnum with the
.Fn tt_ptype_opnum_callback_add
call.
When the message is matched, the
ToolTalk service will check for any callbacks attached to the opnum
and, if any are found, run them.
.LE
.H 3 "Defining Object Types
When a message is addressed to a specific object or a type of object,
the ToolTalk service must be able to determine to which application the
message is to be delivered.
Applications provide this information in an object type (otype).
An otype names the ptype of the application
that manages the object and describes message patterns that pertain to
the object.
These message patterns also contain instructions that tell
the ToolTalk service what to do if a message is available but the
application is not running.
In this case, the ToolTalk service performs one of the
following instructions:
.BL
.LI
Start the application and deliver the message
.LI
Queue the message until the application is running
.LI
Discard the message
.LE
.P
To make the information available to the ToolTalk service, the otype
file must be compiled with the ToolTalk type compiler
.Cm tt_type_comp
at application installation time.
When an application that manages
objects registers with the ToolTalk service, it declares its ptype.
When a ptype is registered, the ToolTalk service checks for otypes that
mention the ptype and registers the patterns found in these otypes.
The otype for the application provides addressing information that the
ToolTalk service uses when delivering object-oriented messages.
The number of otypes, and what they represent, depends on the
nature of the application.
For example, a word processing application
might have otypes for characters, words, paragraphs and documents;
a diagram editing application might have otypes for nodes, arcs,
annotation boxes and diagrams.
.P
An otype begins with an object-type identifier (otid).
Following the otid are:
.AL
.LI
An optional start command string, which the ToolTalk service will execute,
if necessary, to start a process running the program.
.LI
Signatures, which define the messages that can be addressed to
objects of the type (that is, the operations that can be invoked on
objects of the type).
.LE
.HU "Signatures
Signatures define the messages that can be addressed to objects of the type.
A signature is divided by an arrow (the two characters
.Cw => )
into two parts.
The first part of a signature define matching criteria for incoming
messages.
The second part of a signature defines receiver values that
the ToolTalk service adds to each message that matches the first part
of the signature.
These values specify the ptid of the program that implements the
operation and the message's scope and disposition.
.HU "Creating Otype Files
The following is the syntax for an otype file:
.Cs I
otype		::=	\f6obj_header\fP	'{' \f6objbody\fP* '}' [';']
obj_header	::=	'otype' \f6otid\fP [':' \f6otid\fP+]
objbody	::=	`observe:' \f6osignature\fP*
		|	`handle:' \f6osignature\fP*

osignature	::=	\f6op\fP \f6args\fP [\f6contextdcl\fP] [rhs][\f6inherit\fP] `;'
rhs		::=	[`=>' \f6ptid\fP [\f6scope\fP]]
			[`start'][`queue']
			[`opnum='\f6number\fP]
inherit	::=	`from' \f6otid\fP
args		::=	`(' \f6argspec\fP {, \f6argspec\fP}* `)'
		|	`(void)'
		|	`()'
contextdcl	::=	`context' `(' \f6identifier\fP {, \f6identifier\fP}* `)' `;'
argspec	::=	\f6mode\fP \f6type\fP \f6name\fP
mode		::=	`in' | `out' | `inout'
type		::=	\f6identifier\fP
name		::=	\f6identifier\fP
otid		::=	\f6identifier\fP
ptid		::=	\f6identifier\fP
.Ce
.HU "Obj_Header Information
.VL 8
.LI otid
The object type identifier (otid)
identifies the object type.
A otid must be unique for every implementation.
It is recommended that the identifier
begin with the ptid of the tool that implements the otype.
The otid is limited to 64 bytes and must not be one of
the reserved identifiers:
.BR ptype ,
.BR otype ,
.BR start ,
.BR opnum ,
.BR start ,
.BR queue ,
.BR file ,
.BR session ,
.B observe
or
.BR handle .
.LE
.HU "Osignature Information
The object body portion of the otype definition is a list of
osignatures for messages about the object that the application wants
to observe and handle.
.VL 8
.LI op
Operation name.
This name is matched against the op attribute in messages.
.LI args
Arguments for the operation.
If the argument list is
.BR (void) ,
the signature matches only messages with no arguments.
If the argument list is empty
(just ``(\|)''), the signature matches messages without regard to the
arguments.
.LI contextdcl
.br
Context name.
When a pattern with this named context is generated from
the signature, it contains an empty value list.
.LI ptid
The process type identifier for the application that manages this type of
object.
.LI opnum
The application
should fill in the message's opnum attribute with the specified number to
enable it to identify the signature that matched the message.
When the message matches the signature, the
the ToolTalk service fills the
opnum from the signature into the message.
The application can then retrieve the opnum with the
.Fn tt_message_opnum
call.
By giving each signature a unique
opnum, the application
can determine which signature matched the message.
It can attach a callback routine to the opnum with the
.Fn tt_ptype_opnum_callback_add
call.
When the message is matched, the
ToolTalk service will check for any callbacks attached to the opnum
and, if any are found, run them.
.LI inherit
Otypes form an inheritance hierarchy in which operations can be
inherited from base types.
The ToolTalk service requires the otype
definer to name explicitly all inherited operations and the otype from
which to inherit.
This explicit naming prevents later changes (such as
adding a new level to the hierarchy, or adding new operations to base
types) from unexpectedly affecting the behaviour of an otype.
.LI scope
This pattern attribute is matched against the scope attribute in
messages.
It appears on the rightmost side of the arrow and is filled
in by the ToolTalk service during message dispatch.
This means the
definer of the otype can specify the attributes instead of requiring
the message sender to know how the message should be delivered.
.LE
.HU "Osignature Actions Information
.VL 8
.LI start
If the osignature matches a message and no running process of this
otype has a pattern that matches the message,
the ToolTalk service will start a process of this otype.
.LI queue
If the osignature matches a message and no running process of this
otype has a pattern that matches the message,
the ToolTalk service will queue the message until a
process of this otype registers a pattern that matches it.
.LE
.H 2 "Protocol Message Sets"
.xR2 protomsgs
This section describes standard ToolTalk messages.
.mc 5
Many of the \*(Zx services can be controlled or accessed by
sending them ToolTalk messages; those services that do
support ToolTalk interaction
list the messages in a section named ``Messages''
in the appropriate chapters of the \*(Zs.
.P
.mc
Each message is described on a separate reference page,
similar to the format used for a C-language function.
The
.B SYNOPSIS
section is a representation of the message in a
syntax similar to that understood by the ToolTalk type compiler,
.Fn tt_type_comp .
The synopsis format is:
.Cs I
[\f6file\fP] \f6opName\fP(\f6requiredArgs\fP, [\f6optionalArgs\fP]);
.Ce
.P
The components of the synopsis are as follows:
.VL 6
.LI \f2file\fP
If the synopsis begins with
[\f2file\fP],
this is an indication that the file attribute of the
message can or should be set.
ToolTalk allows each
message to refer to a file, and has a mechanism
(called file-scoping) for delivering messages to
clients who are ``interested'' in the named file.
See the
.Fn tt_message_file_set
function.
.LI \f2opName\fP
.br
The name of the operation or event.
.LI \f2requiredArgs,\ optionalArgs\fP
.br
In the synopsis, arguments are expressed as:
.Cs I
\f6mode vtype argumentName\fP
.Ce
.P
The
.I mode
part is one of
.BR in ,
.B out
or
.BR inout ,
indicating the direction(s) in which the data
of that argument flow.
.P
The
.I vtype
and
.I argumentName
parts describe a particular argument.
The
.I vtype
is a programmer-defined string that
describes what kind of data a message argument contains.
ToolTalk uses vtypes for the sole purpose
of matching sent message instances with registered
message patterns.
Every vtype should by convention map to a single,
well-known data type.
The data type of a ToolTalk
argument is either integer, string or bytes.
The data type of a message or pattern argument is
determined by the ToolTalk API function used to set its value.
The
.I argumentName
is merely a comment hinting to human
readers at the semantics of the argument, much like a
parameter name in an ISO C function prototype.
.P
The
.I requiredArgs
shown without [\|] brackets are required to form a valid message.
The
.I optionalArgs
shown enclosed in [\|] brackets are optional.
The extra arguments that may be included in a message.
Any optional
arguments in a message must be in the specified order, and must follow
the required arguments.
.LE
.P
The
.B ERRORS
section describes
integer status codes that can be extracted from a reply via
.Fn tt_message_status .
This status defaults to zero (TT_OK),
or can be set by the handler via
.Fn tt_message_status_set .
In extraordinary circumstances such as
no matching handler, ToolTalk itself sets the message status to a
.B Tt_status
code.
.H 3 "Desktop Message Set"
.xR2 desktopmessages
The
.I Desktop
message policies apply to any tool in an XPG4 or X Window System environment.
In addition to standard messages for these environments, the
.I Desktop
policies define data types and error codes that apply
to all of the ToolTalk message policies.
.P
The following types and argument names are used in message
.B SYNOPSIS
descriptions:
.VL 6
.LI \f3boolean\fP
.br
A vtype for logical values.
The underlying data type of boolean is integer;
that is, arguments of this vtype should
be manipulated with \f2tt_*_arg_ival\fP[\f2_set\fP](\|) and
\f2tt_*_iarg_add(\|)\fP functions.
Zero means false;
non-zero means true.
.LI \f3string\fP
.br
A vtype for character strings.
Arguments of this
vtype should be manipulated with \f2tt_*_arg_val\fP[\f2_set\fP](\|) and
.IR tt_*_arg_add (\|)
functions.
.LI \f3messageID\fP
.br
A vtype for uniquely identifying messages.
The underlying data type of
.B messageID
is
.BR string .
The
.B messageID
of a
.B Tt_message
is returned by
.Fn tt_message_id .
.LI \f3width\fP
.nr Ls 0
.LI \f3height\fP
.LI \f3xOffset\fP
.LI \f3yOffset\fP
.br
.nr Ls 6
Vtypes for integer geometry values, in pixels.
.LI \f2type\fP
Any of the vtypes that are the name of the kind of objects
in a particular system of persistent objects.
For example, the
vtype for the kind of objects in file systems is
.BR File .
The vtype for ToolTalk objects is
.BR ToolTalk_Object .
.LE
.so ../fx/Get_Environment.mm
.so ../fx/Get_Geometry.mm
.so ../fx/Get_Iconified.mm
.so ../fx/Get_Locale.mm
.so ../fx/Get_Mapped.mm
.so ../fx/Get_Modified.mm
.so ../fx/Get_Situation.mm
.so ../fx/Get_Status.mm
.so ../fx/Get_Sysinfo.mm
.so ../fx/Get_XInfo.mm
.so ../fx/Lower.mm
.so ../fx/Modified.mm
.so ../fx/Pause.mm
.so ../fx/Quit.mm
.so ../fx/Raise.mm
.so ../fx/Resume.mm
.so ../fx/Revert.mm
.so ../fx/Reverted.mm
.so ../fx/Save.mm
.so ../fx/Saved.mm
.so ../fx/Set_Environment.mm
.so ../fx/Set_Geometry.mm
.so ../fx/Set_Iconified.mm
.so ../fx/Set_Locale.mm
.so ../fx/Set_Mapped.mm
.so ../fx/Set_Situation.mm
.so ../fx/Signal.mm
.so ../fx/Started.mm
.so ../fx/Status.mm
.so ../fx/Stopped.mm
.ds XC Media Exchange Message Set
.SK
.H 3 "Media Exchange Message Set"
.xR3 mediamessages
The
.I Media
conventions allow a tool to be a container for arbitrary media,
or to be a media player/editor that can be driven from such a container.
These conventions allow a container
application to compose, display, edit, print or transform a document
of an arbitrary media type, without understanding anything about
the format of that media type.
The ToolTalk service routes container requests
to the user's preferred tool for the given media type and operation.
This includes routing the request to an already-running instance of
the tool if that instance is best-positioned to handle the request.
.P
The following types and argument names are used in message
.B SYNOPSIS
descriptions:
.VL 6
.LI \f3boolean\fP
.br
A vtype for logical values.
The underlying data type of boolean is integer;
that is, arguments of this vtype should
be manipulated with \f2tt_*_arg_ival\fP[\f2_set\fP](\|) and
\f2tt_*_iarg_add(\|)\fP functions.
Zero means false;
non-zero means true.
.LI \f3string\fP
.br
A vtype for character strings.
Arguments of this
vtype should be manipulated with \f2tt_*_arg_val\fP[\f2_set\fP](\|) and
.IR tt_*_arg_add (\|)
functions.
.LI \f3bytes\fP
A vtype for character strings that can include null characters.
.LI \f3messageID\fP
.br
A vtype for uniquely identifying messages.
The underlying data type of
.B messageID
is
.BR string .
The
.B messageID
of a
.B Tt_message
is returned by
.Fn tt_message_id .
.LI \f3title\fP
A vtype for character strings intended to be used for
document names or titles.
.LI \f2mediaType\fP
.br
The name of a media format.
The media type of a document
allows messages about that document to be dispatched to the
appropriate tool.
\*(Zx conforming systems support at least the media types in
.mc 4
.cX datafmtnaming .
.LE
.mc
.so ../fx/Deposit.mm
.so ../fx/Display.mm
.so ../fx/Edit.mm
.so ../fx/Mail.mm
.so ../fx/Print.mm
.so ../fx/Translate.mm
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
The message set protocol is required for interoperability.
The ToolTalk wire protocol is also required for
interoperability, but it cannot be standardised in this version due to
the complexity of describing the current version.
The three ``1M'' man
pages in the sample implementation:
.DS I
.Cm rpc.ttdbserverd
.Cm ttdbck
.Cm ttdbserverd
.DE
were omitted because they are system administration commands outside
the scope of the standard.
The disk format for patterns was omitted
because a portable interface is available to access it\*(EMthe
.Cm tt_type_comp
utility.
.mc 2
The
.Cm tttrace
command was listed in Draft 1 in error.
This is a diagnostic tool used for ToolTalk debugging
and is inappropriate for standardisation.
.P
Several of the messages listed in Draft 1 were omitted from Draft 2,
for the following reasons:
.VL 6
.LI "File Lifecycle"
.br
Draft 1 listed notices relating to file updates
.RI ( Modified ,
.IR Saved ,
.IR Reverted )
and lifecycle
.RI ( Created ,
.IR Moved ,
.IR Deleted ).
One reason for
excluding the lifecycle notices is that the update messages are used in
\*(Zx (by
.Cm dtmail ),
but the lifecycle messages are not.
.P
However, the primary reason is that discussions in ANSI X3H6 (CASE
messaging standards) have shown that lifecycle events are not as simple
as they might seem.
For example, when a file is created in UFS, the
directory containing it is considered to have been modified.
When a file is moved, both the old and new directories are modified.
If a tool wants to observe file creations, it is probably better to observe
directory modification events rather than to anticipate the name of the
new file and register for its creation event.
.P
X3H6 may or may not attempt to address this issue.
Given that (a) the
Draft 1-listed interfaces alone did not do complete justice to the issue, and
(b) the lifecycle interfaces are not used in \*(Zx 1.0, it is probably not
worth trying to address this issue right now.
.LI "Buffers, Views"
.br
Many of the Desktop messages are overloaded to also apply to
``buffers'' and ``views,'' which cannot be created using the
Desktop/Media interfaces alone.
Rather, they are created by the
.I Open
request that is part of a draft 'Editor' message set being explored by X3H6.
.P
Since the Editor message interface is not being submitted to \*(XX,
references to buffers and views have been removed from the
Desktop/Media submissions.
This includes both the
.I Opened
and
.I Closed
notices, which only applied to buffers and views.
.LI "Window Events"
.br
Draft 1 listed a set of notices related to windows:
.IR Raised ,
.IR Lowered ,
.IR Iconified ,
.IR DeIconified ,
.IR Mapped
and
.IR UnMapped .
These were included mainly
on an impulse for completeness, because of the analogous requests to,
for example,
``Raise a tool's windows.''
However, these messages are problematic on at least two fronts.
First, it's not clear that the benefit derived
from their observation would be worth the price of diligently sending
them every time the relevant event occurred.
Second, these events
could easily be misinterpreted given the games played by window
managers (such as olvwm and dtwm) that support virtual desktops.
Since
no component of \*(Zx has any requirements for these messages, they are
being omitted from the \*(XX submission.
.LI "\f2Set_XInfo\fP"
.br
.I Get_XInfo
was defined to satisfy a DeskSet requirement for learning
another tool's display, visual and depth.
In anticipation of other
possible similar requirements, the message was defined to also query X
resources, and a related
.I Set_XInfo
message was defined.
It turns out
that X resources are a real pain to make sense of across address
spaces, and these other attributes (display/visual/depth) are in
practice not settable after an application has completed its startup.
So the \*(XX submission includes a
.I Get_XInfo
for only
display/visual/ depth, and no
.IR Set_XInfo .
.LE
.br
.mc
.eF e
