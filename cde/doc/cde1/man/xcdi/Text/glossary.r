.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 2 "Terms Defined by \*(Zx"
The following terms are used in this document:
.gS
.br
.mc 3
.gT "abandoned action"
(Execution Management Services)
An action that is no longer managed by the execution service.
.br
.mc 3
.gT "action label"
(Execution Management Services)
A localised string that provides a textual identification of the action
to the user.
.br
.mc 3
.gT "action"
(Execution Management Services)
A desktop construct that provides a method for running applications,
sending messages and executing commands.
.br
.mc 3
.gT "actions and data types database"
.xR11 actdattypdb
(Execution Management, Data Typing)
The text files used to control the \*(Zx data typing (\c
.cX datatypechap )
and execution management (\c
.cX execmgmtchap )
services.
.br
.mc 4
.gT "actions table"
(Execution Management Services)
The portion of the actions and data types database
that describes actions.
.br
.mc 5
.gT "attachment"
(Mail Services)
An encapsulated data object inside a document.
In the \*(Zx mail services, an attachment is
a data object within an electronic mail
message that is displayed as an icon in the Attachments list.
An attachment can be text, sound or graphic.
Multiple messages can be added (attached) to a single email message.
An attached message is displayed or activated by
selecting it with the mouse button.
.br
.mc 3
.gT "auto wraparound"
(Terminal Emulation Services) A special mode in which text
automatically moves to the next line when
it reaches the right margin.
.br
.mc 3
.gT "auto-repeat key"
(Terminal Emulation Services)
A keyboard character that keeps repeating as long as the key is pressed.
.br
.mc
.gT "backdrop"
(Workspace Management Services)
A background the user selects via the style manager to make the
workspace visually distinctive.
.br
.mc 3
.gT "base height"
(Terminal Emulation Services)
The height of the terminal window before accounting for the total height of
the lines of text to be displayed.
.br
.mc 3
.gT "base width"
(Terminal Emulation Services)
The width of the terminal window before accounting for the total width
of the columns of text to be displayed.
.br
.mc 3
.gT "bell"
(Terminal Emulation Services)
An indicator that a special event has taken place.
It can be an audible beep from the computer or a visual indicator.
.br
.mc 3
.gT "blanking mode"
(Terminal Emulation Services)
A special mode for terminal emulator windows to avoid visual clutter.
In this mode the pointer is made invisible if it remains stationary for
a period of time.
The pointer re-appears when it is moved (usually by a mouse) and
disappears after a selectable number of seconds or when keyboard input begins.
.br
.mc 3
.gT "bounding box"
(Terminal Emulation Services)
A rectangle used as a guide when designing fonts.
Characters in a font are designed to fit generally inside their font's
bounding box.
.br
.mc 3
.gT "buffer argument"
(Execution Management Services)
.mc 4
An argument that specifies an object in memory.
This contrasts with a file argument,
.mc 3
which specifies a file generally located on disk.
.br
.mc
.gT "category"
(ToolTalk)
Attribute of a pattern that indicates whether the application
is prepared to
handle requests that match the pattern or only observe the requests.
.br
.mc 3
.gT "character protection attribute"
(Terminal Emulation Services)
An attribute that applies to text displayed in a terminal emulator window.
Text characters for which this attribute is set cannot be erased.
.br
.mc 3
.gT "character-spaced font"
(Terminal Emulation Services)
A special kind of monospaced font whose characters each fit into the
font's bounding box.
.br
.mc 3
.gT "client data"
(Execution Management Services)
Data supplied by the client and associated with a callback function.
The meaning of the data depends entirely on the client.
Client data is often used by a client to send data that it will receive
again later.
.br
.mc
.gT "colour set"
(Workspace Management Services)
A set of five colours used to represent a single logical colour
in the Motif toolkit.
For each background colour (the ``logical'' colour),
there are associated top shadow, bottom shadow, foreground and
select colours, all generated from the background colour.
These associated colours are the mechanism for giving widgets their
three-dimensional appearance.
.br
.mc
.gT "contexts"
.xR11 contexts
(ToolTalk)
Sets of
.I <name.value>
explicitly included in both messages and patterns.
ToolTalk contexts allow fine-grain matching.
.P
The application can use contexts to associate arbitrary pairs with
ToolTalk messages and patterns, and to restrict the set of possible
recipients of a message.
One common use of the restricted pattern matching provided by ToolTalk
context attributes is to create sub-sessions.
For example, two different programs could
be debugged simultaneously with tools such as a browser, an editor, a
debugger, and a configuration manager active for each program.
The message and pattern context slots for each set of tools contain
different values; the normal ToolTalk pattern matching of these values
keep the two sub-sessions separate.
.P
Another use for the restricted pattern matching provided by ToolTalk context
attributes is to provide information in environment variables and
command-line arguments to tools started by the ToolTalk service.
.br
.mc 4
.gT "current session"
(Session Management Services)
The collection of applications, settings
and resources that are currently present on the user's desktop.
If a user selects to restore the current session
upon login, the desktop will be restored to the same state
it was in when that user ended the previous session and logged out.
.br
.mc
.gT "current workspace"
(Workspace Management Services)
The workspace whose windows are currently visible to the user.
.br
.mc 3
.gT "data attributes"
(Data Typing)
The attributes that determine user-visible interfaces to file or
byte-stream data:
a human-readable description of the type, the icon to represent it
graphically and the actions that apply to it.
.br
.mc 4
.gT "data attributes table"
(Data Typing)
The portion of the actions and data types database
that describes data attributes.
.mc 5
See
.cX DataAttributeRecords .
.br
.mc 4
.gT "data criteria table"
(Data Typing)
The portion of the actions and data types database
that describes data criteria.
.mc 5
See
.cX DataCriteriaRecords .
.br
.mc 5
.gT "data type"
(Data Typing)
A name characterising the data in a file or byte vector.
A data type is named by the value of the DATA_ATTRIBUTES_NAME
field in the matching record of the data criteria table
of the actions and data types database.
.br
.mc 3
.gT "data types database"
(Data Typing)
See
.cX actdattypdb .
.br
.mc 3
.gT "data typing"
A method of determining the data attributes of a file or byte
vector, based on its name, file permissions, symbolic links and content.
.br
.mc 4
.gT "display area"
(Help Services)
An area in help dialogs that shows a help topic.
.br
.mc 3
.gT "drag"
(Drag and Drop)
A user interaction in which elements or their representations
change their position or appearance in conjunction with
the movement of the pointer.
.br
.mc 3
.gT "drag and drop"
(Drag and Drop)
A user interaction in which a user drags source elements
to a target element on which they are dropped.
.br
.mc 3
.gT "drop"
(Drag and Drop)
A user action that terminates a drag,
identifying the destination of the drag and drop
interaction as the element under the pointer.
.br
.mc 3
.gT "drop zone"
(Drag and Drop)
An area of the workspace, including the Trash Can,
Print Manager and Mailer controls, that accepts a dropped icon.
Icons can be dropped on the workspace for quick access.
.br
.mc
.gT "dynamic message patterns"
(ToolTalk)
A message pattern provided by the application
that is created while the application is running.
See
.Fn tt_pattern_create
and
.Fn tt_pattern_register
in
.cX msgchap . 1
.br
.mc
.gT "deserialise"
(ToolTalk)
Decode a data structure from an architecturally neutral stream of bytes.
.br
.mc
.gT "edict"
(ToolTalk)
A notice that looks like a request.
For example, if a request returns
no data (or if the sender does not care about the returned data),
the application
can use an edict to broadcast that request to a set of tools.
Since the message is a notice, no data will be returned, no replies will be
received, and the sender is not told whether any tool receives the message.
.br
.mc 3
.gT "escape character"
(Terminal Emulation Services)
A character that generally indicates the beginning of a command to be performed,
not text to be displayed or passed to the application.
The character code in the \*(Z8 is decimal 27.
.br
.mc 3
.gT "execution host"
(Execution Management Services)
A host computer that runs an application invoked by an action.
This may be the same computer where the action resides,
or it may be another computer on the network.
.br
.mc 3
.gT "execution string"
(Execution Management Services)
A string that specifies the command to invoke.
.br
.mc
.gT "fail a request"
(ToolTalk)
Inform a sending application that the requested operation cannot be
performed, by calling
.Fn tt_message_fail
in
.cX msgchap . 1
This is a voluntary failure, as opposed to a message
not being sendable.
.br
.mc 6
.gT "folder"
(File Management Services)
A representation of a directory in the underlying file system.
.br
.mc 5
.gT "front panel"
(Front Panel Services)
A centrally located window containing controls
for accessing applications and utilities.
The front panel occupies all workspaces.
.br
.mc 4
.gT "general help dialog"
.xR11 genhelp
(Help Services)
A window that displays help information and provides full
navigation capabilities.
See also
.cX quickhelp ,
which offers more limited capabilities.
.br
.mc
.gT "handler"
(ToolTalk)
The distinguished recipient procid of a message.
.br
.mc
.gT "handle a request"
(ToolTalk)
Perform the operation requested by the sending application.
.br
.mc 3
.gT "hard reset"
(Terminal Emulation Services)
An operation that fully restores the terminal emulator to a specific
startup state, as defined by the emulation.
This includes everything done by a soft reset.
.br
.mc 3
.gT "height increment"
(Terminal Emulation Services)
The height in pixels of a single line of text.
It is used to calculate the height of the terminal emulator window
.br
.mc 5
.gT "HelpTag"
(Help Services)
The markup language used for creating \*(Zx help volumes.
HelpTag complies with the referenced SGML standard.
.br
.mc 4
.gT "help topic"
(Help Services)
The smallest addressable piece of help information.
It may be authored in HelpTag or system
manual-page macros, or be a text file or string in the \*(Z8 codeset.
Help topics authored in HelpTag may reference additional topics.
.br
.mc 4
.gT "help type"
(Help Services)
The format of the data in a given help topic.
Formats include HelpTag topic, string data, system manual page
and text file.
.br
.mc 4
.gT "help volume"
(Help Services)
A collection of help topics authored in HelpTag.
.br
.mc 4
.gT "home session"
(Session Management Services)
The collection of applications, settings
and resources on the desktop.
If a user selects to restore the home session upon login,
the desktop is restored to the same initial state every
time the user logs in, regardless of its state when the
user last logged out.
.br
.mc 3
.gT "icon name"
(Execution Management Services)
The name of a field in the action database.
It is generally used by applications to construct the location where
the bitmap of the icon is located.
.br
.mc
.gT "initial session"
(ToolTalk)
The ToolTalk session in which the application was started.
.br
.mc 3
.gT "jump scrolling"
(Terminal Emulation Services)
A specific scrolling behaviour in which the screen may be scrolled by
more than one line at a time.
.br
.mc 4
.gT "location ID"
(Help Services)
The identifier for a particular location in a help
topic that can be accessed directly.
.br
.mc 3
.gT "login shell"
(Terminal Emulation Services)
The initial shell invoked when a user logs in.
For
.Cm ksh
and
.Cm sh
users,
the login shell startup is controlled by the system
.B profile
and the user
.B \&.profile
files.
.br
.mc 5
.gT "mail header"
(Mail Services)
The lines in a mail message that precede the empty line
that marks the beginning of the message text,
as described in the referenced RFC-822 specification.
.br
.mc 3
.gT "margin bell"
(Terminal Emulation Services)
An indicator that user input has reached the right margin of the window.
.br
.mc
.gT "mark"
(ToolTalk)
An integer that represents a location on the API storage stack;
see
.Fn tt_mark
and
.Fn tt_release
in
.cX msgchap . 1
.br
.mc
.gT "message"
(ToolTalk)
A structure that the ToolTalk service delivers to processes.
A ToolTalk
message consists of an operation name, a vector of type arguments, a
status value or string pair, and ancillary addressing information.
.br
.mc
.gT "message callback"
(ToolTalk)
A client function.
The ToolTalk service invokes this function to report
information about the specified message back to the sending application;
for example, the message failed or the message caused a tool to start.
.br
.mc
.gT "message pattern"
(ToolTalk)
Defines the message the application is prepared to receive.
.br
.mc
.gT "message protocol"
(ToolTalk)
A set of ToolTalk messages that describe
operations the applications agree to perform.
.br
.mc 4
.gT "module"
(Application Building Services)
An arbitrary subdivision of a project; see
.cX project .
Projects can be subdivided into modules for
clarity of architectural structure, for
ease of controlling multiple programmers
on the same project, and so forth.
.br
.mc 3
.gT "monospaced font"
(Terminal Emulation Services)
A font composed of characters that are all of the same width.
.br
.mc
.gT "netfilename"
(ToolTalk)
A canonical form
of a pathname that can be passed to other hosts on the network and converted
back to a local pathname for the same file.
See
.Fn tt_file_netfile
in
.cX msgchap . 1
.br
.mc
.gT "notice"
(ToolTalk)
An informational message used by an application to announce an event.
Zero or more tools may receive a given notice.
The sender is not informed whether any tools receive its notice
because replies cannot be sent for a notice.
.br
.mc 6
.gT "object"
(File Management Services)
A representation of a file in the underlying file system.
Examples of objects are text files, actions or directories.
.br
.mc
.gT "object content"
(ToolTalk)
A piece, or pieces, of an ordinary file,
managed by the application that creates or manages the object;
for example, a
paragraph, a source code function or a range of spreadsheet cells.
.br
.mc
.gT "object-oriented messages"
(ToolTalk)
Messages addressed to objects managed by applications.
.br
.mc
.gT "object specification (spec)"
.xR11 objectspecification
(ToolTalk)
Standard properties of an object,
such as the type of object, the name of the file in which the object
contents are located, and the object owner.
.br
.mc
.gT "object type (otype)"
(ToolTalk)
Addressing information that the ToolTalk service uses when delivering
object-oriented messages to an application.
.br
.mc
.gT "object type identifier (otid)"
(ToolTalk)
Identifies the object type.
.br
.mc
.gT "objid"
(ToolTalk)
The identifier for a ToolTalk object.
The same ToolTalk object can
have different objids as it is moved from one file system to another;
see
.Fn tt_objid_equal
in
.cX msgchap . 1
Objids are returned by
.Fn tt_spec_create .
.br
.mc
.gT "observe a message"
(ToolTalk)
View a message without performing any operation that may be requested.
.br
.mc
.gT "observe promise"
(ToolTalk)
Guarantee that the ToolTalk service will deliver a copy of each
matching message to
ptypes
with an observer signature of start or queue disposition.
The ToolTalk service will deliver the message either to a
running instance of the
ptype,
by starting an instance, or by queueing
the message for the
ptype.
.br
.mc 3
.gT "opaque"
(ToolTalk, Terminal Emulation Services)
A value or structure that has meaning only when
passed through a particular interface.
.br
.mc
.gT "opname (op)"
(ToolTalk)
The name of the message.
.br
.mc
.gT "opnum"
(ToolTalk)
A mechanism for indicating the static pattern that caused
the message to be received.
Opnums and opnum callbacks are to static patterns as
pattern callbacks are to dynamic patterns.
.br
.mc 3
.gT "option argument keyword"
(Execution Management Services)
A special symbolic name that refers to a value determined at run time.
For example,
.B %Arg_1%
refers to the first argument passed to
.Fn DtActionInvoke .
.br
.mc 3
.gT "option string"
(Execution Management Services)
A string containing command-line options to an execution string.
.br
.mc 3
.gT "overstriking"
(Terminal Emulation Services)
Rendering the same characters several times at close to the same position.
This technique originated to emulate bold fonts on a typewriter.
.br
.mc 4
.gT "palette object"
(Application Building Services)
A user interface object that is included in an application
by dragging it from the object palette of the
application building services.
.br
.mc 5
.gT "paragraph"
(Text Editing Services)
One or more words preceded by a blank line or the beginning
of the text and followed by a blank line or the end of the text.
.br
.mc
.gT "pattern callback"
(ToolTalk)
A client function.
The ToolTalk service invokes this function when a
message is received that matches the specified pattern.
.br
.mc 3
.gT "pixel offset"
(Terminal Emulation Services)
The distance in pixels between two locations.
.br
.mc
.gT "procedural message"
(ToolTalk)
A message addressed by operation name.
.br
.mc
.gT "procid"
(ToolTalk)
The process identifier.
The procid is a principal that can send and receive ToolTalk messages.
It is an identity, created and passed by
the ToolTalk service, that a process must assume
to send and receive messages.
A single operating system process can use multiple procids.
A procid is the value returned by
.Fn tt_open
in
.cX msgchap . 1
.br
.mc 4
.gT "project"
.xR11 project
(Application Building Services)
An application under construction by the
application building services.
A project can consist of zero or more modules.
.br
.mc 3
.gT "proportional font"
(Terminal Emulation Services)
A font whose characters are various widths.
.br
.mc 3
.gT "pseudo-terminal"
(Terminal Emulation Services)
An implementation-dependent driver that simulates
the action of a terminal device; a software abstraction of the
communications path between the system and a terminal.
A pseudo-terminal connection can be used between an
application or shell and a terminal emulation window.
.br
.mc
.gT "ptid"
(ToolTalk)
The process type identifier.
.br
.mc
.gT "ptype"
(ToolTalk)
The process type.
.br
.mc 4
.gT "quick help dialog"
.xR11 quickhelp
(Help Services)
A window that displays a help topic.
See also
.cX genhelp ,
which offers additional capabilities.
.br
.mc
.gT "registration context"
(Workspace Management Services)
A handle that uniquely identifies a callback function.
A registration context is returned when
a workspace callback function is added.
It must be saved if the callback function is to be removed.
.br
.mc
.gT "reject a request"
(ToolTalk)
Tell the ToolTalk service, using
.Fn tt_message_reject
in
.cX msgchap , 1
that the receiving application is unable to
perform the requested operation and that the message should be given to
another tool.
.br
.mc 3
.gT "reparenting window manager"
(Terminal Emulation Services)
Applications create toplevel windows that they request to be children
of the root window.
A reparenting window manager makes toplevel windows children of its
windows instead.
The window manager windows generally act as a frame around the
application's toplevel window and contain such elements as resize
handles and title bars.
.br
.mc
.gT "request"
(ToolTalk)
A call for an action.
The results of the action are recorded in the message, and the message
is returned to the sender as a reply.
A request asks that an operation be performed.
It has a distinguished recipient (handler) responsible for performing
the indicated operation.
.br
.mc 3
.gT "reverse wraparound"
(Terminal Emulation Services)
The cursor movement that wraps the cursor up to the previous line if
the cursor is located at the left margin and the user presses backspace.
.br
.mc
.gT "rpc.ttdbserverd"
(ToolTalk)
The ToolTalk database server process.
.br
.mc
.gT "scope"
(ToolTalk)
The attribute of a message or pattern that determines how widely the
ToolTalk service looks for matching messages or patterns.
.br
.mc 3
.gT "selection extension"
(Terminal Emulation Services)
An increase in the amount of text selected.
.br
.mc
.gT "serialise"
(ToolTalk)
Encode a data structure into an architecturally neutral stream of bytes.
.br
.mc
.gT "sessid"
(ToolTalk)
The identifier for a ToolTalk session.
.mc 3
It is an opaque character string.
.br
.mc
.gT "session"
(ToolTalk)
A group of ToolTalk
processes that are related either by being in the same desktop or the
same process tree.
.br
.mc 3
.gT "signal handler"
(Terminal Emulation Services)
A software routine that deals with signals the application may receive.
.br
.mc
.gT "signature"
.xR 11 signature
(ToolTalk)
A pattern in a ptype or otype.
A signature can contain values for
disposition and operation numbers.
.BL
.LI
A
ptype
signature
.RI ( psignature )
describes the procedural messages that the application
is prepared to receive.
.LI
An
otype
signature
.RI ( osignature )
defines the messages that can be addressed to objects of the type.
.LE
.br
.mc 3
.gT "slave device"
(Terminal Emulation Services)
A system device that provides to application processes a terminal
device interface as described in the General Terminal Interface
.RI ( termios )
of the \*(Zb.
.br
.mc
.gT "slotname"
(ToolTalk)
The name of a ToolTalk context.
See
.cX contexts .
.br
.mc 3
.gT "soft reset"
(Terminal Emulation Services)
An operation that partially restores the terminal emulator to a specific
startup state, as defined by the emulation.
It is a subset of a hard reset.
.br
.mc 5
.gT "source"
(Drag and Drop)
The object that is selected, dragged and dropped in a drag-and-drop action.
.br
.mc 3
.gT "source indicator"
(Drag and Drop)
The part of the pointer displayed during drag and drop
that describes the source.
.br
.mc
.gT "spec"
(ToolTalk)
See
.cX objectspecification .
.br
.mc 3
.gT "starting shell"
(Terminal Emulation Services)
The initial shell controlling interaction when the terminal window appears.
.br
.mc 3
.gT "state indicator"
(Drag and Drop)
The part of the pointer displayed during drag and drop that indicates
whether the pointer is at a place where a drop is likely to result in a
successful operation.
.br
.mc
.gT "static message pattern"
(ToolTalk)
A signature.
See
.cX signature .
.br
.mc 5
.gT "subpanel"
(Front Panel Services)
A component associated with a control on the front panel.
It provides additional elements associated with that control.
.br
.mc 3
.gT "terminal emulator"
(Terminal Emulation Services)
A window that emulates a particular type of terminal for running
non-window programs.
Terminal emulator windows are most commonly used for typing commands to
interact with the computer's operating system.
.br
.mc 3
.gT "text rendering"
(Terminal Emulation Services)
Drawing text on a video screen.
.br
.mc 5
.gT "tool"
(ToolTalk)
An application or utility that can be manipulated using
ToolTalk services.
.br
.mc
.gT "ToolTalk API"
(ToolTalk)
The functions and headers in
.cX msgchap . 1
.br
.mc
.gT "ToolTalk service"
(ToolTalk)
The implementation of the ToolTalk API and components.
.br
.mc
.gT "ToolTalk types database"
(ToolTalk)
The database that stores ToolTalk type information.
.br
.mc 4
.gT "topic tree"
(Help Services)
An area in the general help dialog that aids navigation of
topics that can be browsed in the current help volume.
.br
.mc 3
.gT "underscore"
(Terminal Emulation Services)
The character ``_''.
.br
.mc
.gT "value type (vtype)"
(ToolTalk)
An application-defined string that describes what kind of data a message
argument contains.
The ToolTalk service only uses
vtypes
to match sent message instances with registered message patterns.
.br
.mc 3
.gT "virtual keys"
(Terminal Emulation Services)
Logical key translations, as defined by the \*(ZM.
.br
.mc 3
.gT "width increment"
(Terminal Emulation Services)
The width in pixels of a single column of text.
It is used to calculate the width of the terminal emulator window.
.br
.mc
.gT "window menu"
(Workspace Management Services)
The menu displayed by choosing the Window menu button.
The menu provides choices that manipulate the location or size of the
window, such as Move, Size, Minimise and Maximise.
.br
.mc 5
.gT "word"
(Text Editing Services)
One or more non-white-space characters preceded by the
beginning of the text, the beginning of a line, or a
white-space character and trailed by the end of the text,
the end of a line, or a white-space character.
.br
.mc
.gT "workspace"
(Workspace Management Services)
A ``virtual screen'' that contains a set of windows.
Workspaces provide a way of grouping together logically related windows.
Each workspace is shown independent of the other workspaces
so that only those windows related to the immediate task are visible.
Workspaces organise windows by task and efficiently use screen space.
.br
.mc
.gT "workspace identifier"
(Workspace Management Services)
An X atom that indicates the name of the workspace.
.br
.mc
.gT "workspace functions"
(Workspace Management Services)
Capabilities that appear in a window menu to allow
the user to specify which workspace a window occupies.
.br
.mc
.gT "workspace manager"
(Workspace Management Services)
A program that implements workspaces.
.br
.mc
.gT "workspace name"
(Workspace Management Services)
The resource name for a workspace.
This name is converted to an X atom and used in the workspace management API.
The workspace name is generated dynamically by the workspace manager.
Since it is a resource name, the characters are limited to those in the
X Portable Character Set.
(See
.cX wstitle .)
.br
.mc
.gT "workspace title"
(Workspace Management Services)
.xR11 wstitle
The user-visible title associated with a workspace.
It is displayed in the workspace button in the front panel.
It is interpreted in the locale in which the workspace manager is running.
The user can change the workspace title.
.br
.mc
.gT "XDR"
(ToolTalk)
Serialise data for transmission using the
External Data Representation data description
language and data representation protocol described in
RFC 1014.
.gE
.H 2 "Terms From Other Standards"
The following terms used in this document are defined in \*(Zb:
.P
.I
Editor's Note:
.mc 5
This is the full list; it will be pared down
.mc 6
in the final standard to match only those terms used herein.
.R
.P
.mc
.2C
.nf
absolute pathname
access mode
additional file access control mechanism
address space
affirmative response
alert
alert character
alias name
alternate file access control mechanism
angle brackets
appropriate privileges
argument
assignment
asterisk
background job
background process
background process group (or background job)
backquote
backslash
backspace character
base character
basename
basic regular expression
blank character
blank line
block-mode terminal
block special file
braces
brackets
built-in utility (or built-in)
byte
carriage-return character
character
character array
character class
character set
character special file
character string
child process
circumflex
clock tick
coded character set
codeset
collating element
collation
collation sequence
column position
command
command language interpreter
composite graphic symbol
control character
control operator
controlling process
controlling terminal
conversion descriptor
current working directory
cursor position
device
device ID
directory
directory entry (or link)
directory stream
display
dollar sign
dot
dot-dot
double-quote
downshifting
effective group ID
effective user ID
eight-bit transparency
empty directory
empty line
empty string (or null string)
empty wide character string
epoch
equivalence class
era
executable file
execute
expand
extended regular expression
extended security controls
feature test macro
field
FIFO special file (or FIFO)
file
file access permissions
file description
file descriptor
file group class
file hierarchy
file mode
file mode bits
filename
file offset
file other class
file owner class
file permission bits
file serial number
file system
file times update
file type
filter
foreground job
foreground process
foreground process group (or foreground job)
foreground process group ID
form-feed character
graphic character
group database
group ID
group name
hard link
home directory
incomplete line
Inf
interactive shell
internationalisation
invoke
ISO/IEC\ 646:\|1983
ISO/IEC\ 6937:\|1983
ISO/IEC\ 8859-1:\|1987
job
job control
job control job ID
line
link
link count
local customs
locale
localisation
login
login name
message catalogue
message catalogue descriptor
mode
mount point
multi-character collating element
name
NaN (not a number)
native language
negative response
newline character
non-spacing characters
NUL
null byte
null pointer
null string
null wide character code
number sign
object file
open file
open file description
operand
operator
option
option-argument
orphaned process group
parameter
parent directory
parent process
parent process ID
pathname
pathname component
pathname resolution
path prefix
pattern
period
permissions
pipe
positional parameter
portable character set
portable filename character set
printable character
printable file
privilege
process
process group
process group ID
process group leader
process group lifetime
process ID
process lifetime
program
radix character
read-only file system
real group ID
real user ID
redirection
redirection operator
refresh
regular expression
regular file
relative pathname
root directory
saved set-group-ID
saved set-user-ID
screen
scroll
seconds since the epoch
session
session leader
session lifetime
shell
shell, the
shell script
signal
single-quote
slash
source code
special parameter
space character
standard error
standard input
standard output
standard utilities
stream
string
subshell
supplementary group ID
suspended job
system
system process
system scheduling priority
tab character
terminal (or terminal device)
text column
text file
tilde
token
upshifting
user database
user ID
user name
utility
variable
variable assignment
vertical-tab character
white space
wide character code
wide character string
word
working directory (or current working directory)
world-wide portability interface
write
zombie process
[n, m] and [n, m)
.1C
.fi
.sp
.fi
The following terms used in this document are defined in the \*(ZM:
.P
.tr = \"
.TS
center;
le le le le .
atom	icon	pointer	scrollbar
border	menu	protocol	widget
class	Motif	Resource	window
compound=text	object	scroll	window=manager
drag=and=drop
.TE
.P
The following terms used in this document are defined in
.I "<other \*(XX X Windows docs>" :
.tr = \"
.TS
center;
le le le le .
access=control=list	event=mask	monochrome	server
active=grab	event=propagation	object	server=grabbing
ancestors	event=source	obscure	session=manager
API	event=synchronisation	occlude	sibling
application	exposure=event	padding	stacking=order
application=programmer	extension	parent=window	staticColor
atom	focus=window	passive=grab	staticGray
background	font	pixel	stipple
backing=store	gadget	pixel=value	string=equivalence
BDF	GContext	pixmap	style=guide
bit=gravity	glyph	plane	tile
bit=plane	grab	plane=mask	timestamp
bitmap	graphics=context	pointer	TrueColor
border	gravity	pointer=grabbing	type
button=grabbing	grayScale	pointing=device	UI=platform
byte=order	GUI	policy	UI=specification
children	hotspot	property	UIDL
class	ICCCM	property=list	user
client	identifier	pseudoColor	user=interface
clipping=region	inferiors	redirecting=control	viewable
colormap	input=focus	reply	visible
compound=text	input=manager	request	widget
connection	InputOnly=window	resource	widget=programmer
containment	InputOutput=window	RGB=values	window=gravity
coordinate=system	instance	root	window=manager
cursor	key=grabbing	root=window	X=Protocol
depth	keyboard=grabbing	save=set	XLFD
device	KEYSYM	scanline	Xlib
directColor	mapped	scanline=order	Xt=Intrinsics
display	method	screen	XYFormat
drawable	modifier=keys	selection	ZFormat
event
.TE
.tr ==
.eF e
