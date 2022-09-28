.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Application Building Services"
.xR1 appbuildsvcs
.H 2 "Introduction"
.mc 4
The \*(Zx application building services
provide an interactive, graphical environment that facilitates the
development of \*(Zx-compliant applications.
Two basic services are provided:
aid in assembling graphical objects into the desired
application user interface, and generation of appropriate calls to the
routines that support \*(Zx desktop services (such as ToolTalk messaging,
drag and drop, and so forth).
.P
The key supported tasks for the application building services are:
.BL
.LI
Interactive layout of the user interface for an application,
constructing it piece-by-piece from a collection of objects from the
\*(Zx and Motif toolkits.
.LI
Managing an application
.I project
with its constituent
.I module
subdivisions accessible separately
by multiple developers and able
to be imported and exported to and from projects.
.LI
Definition of window resizing behaviour.
.LI
Definition of connections between objects to provide elements of
application interface behaviour, and a limited test mode that allows
connections to be exercised.
.LI
Interactive specification of the interconnections desired between the
application and CDE desktop services.
.LI
Drag and drop specifications for individual objects.
.LI
Editing of applications previously created using
the \*(Zx application building services.
.LI
Generation of C-language source code and associated project files
(for example, makefiles and message catalogues) for the application.
.LI
Generation (compilation) and invocation of the application from within
the application building services, allowing the developer to execute the
build/run/debug cycle all from a common environment (and without having
to exit and restart the application building services).
.LE
.mc
.H 2 "Command-Line Interfaces" S
This section defines the utility that provides
\*(Zx application building services.
.so ../fx/dtcodegen.mm
.ds XC Actions
.SK
.H 2 "Actions" S
This section defines the actions that provide \*(Zx application building
services to support application portability at the C-language source
or shell script levels.
.so ../hx/dtbuilderaction.mm
.ds XC Capabilities
.SK
.H 2 "Capabilities"
A conforming implementation of the \*(Zx application building services
supports at least the following capabilities:
.AL
.LI
Provides application building services
as described in the following subsections.
.LI
.mc 5
Conforms to the Required items in the Application Style Checklist in
.cX checklist ,
with the following exceptions that may exist on some implementations:
.mc
.AL a
.LI
There are certain operations that need not comply with checklist
item 2-4, concerning the Input Device Model for keyboard-only users.
The drag and drop of objects from the palette to the workspace
and the ability to resize objects or move them around within a window
or dialogue need not be available from the keyboard.
.LI
Another possible exception to drag and drop style is described in
.cX appbuilddnd .
.LI
The placement of secondary windows need not comply with checklist
items az and ba, concerning Application Window Management.
.LE
.LI
Has been internationalised using the standard interfaces
in the \*(ZK, the \*(Zl, and the \*(ZM,
and supports any locale available in the underlying X Window System
Xlib implementation.
.LE
.H 3 "Project and Module Files"
.xR3 appbuildfiles
The application building services support two file formats:
.AL
.LI
Application projects and modules are expressed in an
implementation-dependent format.
The application building services can read, write
and drag and drop files in this format.
.LI
Modules can be imported from a file or exported to a file
in the Motif User Interface Language (UIL) format.
See the \*(ZM.
.LE
.H 3 "Project Management"
The application building services provide the following project
management capabilities:
.AL
.LI
Manipulating a collection of modules
as a single project that can be
processed as a single entity.
.LI
Saving modules or the project, using existing or new names
.LI
Removing modules from a project
.LI
Sharing a module among multiple projects
.LI
Exporting a module from a project
(Exporting is distinguished from saving because it
typically uses a different file format, such as UIL.
Exporting using the implementation-dependent format
for modules is the equivalent of a regular save or
save-as operation.)
.LI
Hiding modules from viewing (to simplify the workspace)
.LI
Building (making, compiling, linking, and so forth)
and executing the application
.LE
.H 3 "Object Palette"
The object palette is a panel that holds iconic
representations for objects that can be instantiated as part of the
application under construction.
Construction is accomplished for these palette objects by dragging
them from the palette and dropping them into the
.IR workspace ,
where the windows and dialogues of the application are arranged.
.P
Objects on the palette include those in the following table.
.DS
.TS
center, box;
cf3 s
cf3 | cf3
l | lf3 .
Application Building Services Objects
_
Object Type	Motif Class
_
.sp 0.2v
Main Window	XmMainWindow + ApplicationShell
Dialogue Box	XmDialogShell
File Selection Dialogue	XmFileSelectionBox + XmDialogShell
Control Pane	XmForm
Text Pane	XmText
Drawing Pane	XmDrawingArea
Term Pane	DtTerm
Button	\f11.\fP  XmPushButton
	\f12.\fP  XmDrawnButton
	\f13.\fP  XmArrowButton
Menu Button	DtMenuButton
Combo Box	DtComboBox
Option Menu	XmOptionMenu
Radio Box	XmRowColumn + XmToggleButton(s)
Check Button	XmRowColumn + XmToggleButton(s)
Gauge	XmScale
Scale	XmScale
Separator	XmSeparator
Text Field	XmTextField
Label	XmLabel
List Box	XmList + XmScrolledWindow
Spin Box	DtSpinButton
.TE
.DE
.P
The column labelled Motif Class describes the functionality
of the object in terms of named Motif widgets.
The object created includes the combined user interface behaviour
of the named widgets, although the implementation
may include additional or alternative widgets to
achieve this behaviour.
An object in the table with several numbered widgets listed as its
Motif Class means that implementations may select the type of widget
used to support the object, based on the
layout behaviour and properties selected by the developer.
An object with a ``Dt'' widget listed in its Motif Class
need not be supported in exported modules using the Motif UIL format.
.P
There are other objects created for the application
that need not be available directly on the object palette.
These include:
.BL
.LI
groups of objects
.LI
menus
.LI
message boxes
.LI
paned windows
.LE
.P
Unless noted otherwise, all references to ``objects''
.ne 2
in this Capability section apply to the palette
objects as well as these additional objects.
.H 3 "Object Layout"
The developer can position palette objects
placed on the workspace as follows:
.AL
.LI
The drag and drop operation establishes the initial position.
.LI
The developer can center objects and align them with each other,
horizontally or vertically.
.LI
The developer can specify relative positioning of objects.
.LI
The developer can group objects and affect their
positions relative to each other following
window resize operations performed by the
developer or the application end-user.
.LE
.H 3 "Object Properties"
The \*(Zx application building services
allow the developer to display and modify a wide range of
attributes for each object, some of which correspond
to X Windows resources for the underlying Motif or \*(Zx widgets.
For each of the following objects, the developer can
specify the associated attributes:
.VL 11 5
.LI "Main Window"
.AL
.LI
Object name
.LI
Window title
.LI
Icon filename
.LI
Icon label
.LI
User resize mode (fixed/adjustable)
.LI
Window areas present (menubar, toolbar, footer)
.LI
Geometry:
x, y, width, height
.LI
Initial state (iconic, visible)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Custom Dialogue"
.AL
.LI
Object name
.LI
Dialogue title
.LI
Window parent
.LI
User resize mode (fixed/adjustable)
.LI
Dialogue areas present (button panel, footer)
.LI
Default button (choose among the buttons in the dialogue)
.LI
Geometry:
x, y, width, height
.LI
Initial state (visible)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "File Chooser"
.AL
.LI
Object name
.LI
Window parent
.LI
Title
.LI
Initial directory
.LI
Search pattern type (files, folders, both)
.LI
Search pattern
.LI
OK button label
.LI
Initial state (visible)
.LI
Popdown behaviour (automatically dismiss)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Control Pane"
.AL
.LI
Object name
.LI
Geometry (x,y,width height)
.LI
Initial state (visible, active)
.LI
Border frame (none/shadow out/shadow in/etched out/etched
in)
.LI
Background colour
.LI
Name of attached menu
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Text Pane"
.AL
.LI
Object name
.LI
Scrollbars (always/never)
.LI
Position:
x, y
.LI
Size:
width and height in either characters or pixels
.LI
Word wrap
.LI
Operation (read-write/read-only)
.LI
Initial value
.LI
Initial state (visible, active)
.LI
Border frame (none/shadow out/shadow in/etched out/etched
in)
.LI
Background colour
.LI
Foreground colour
.LI
Name of attached menu
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Draw Area Pane"
.AL
.LI
Object name
.LI
Scrollbars (always/as needed/never)
.LI
Position:
x, y
.LI
Visible size:
width, height
.LI
Total size:
width, height
.LI
Initial state (visible, active)
.LI
Border frame (none/shadow out/shadow in/etched out/etched
in)
.LI
Background colour
.LI
Foreground colour
.LI
Name of attached menu
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Label"
.AL
.LI
Object name
.LI
Size policy (size of label/fixed)
.LI
Geometry:
x, y, width, height
.LI
Label type (string/graphic)
.LI
Label
.LI
Label alignment (left/right/centered)
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Spin Box"
.AL
.LI
Object name
.LI
Label type (string/graphic)
.LI
Label Position (left/above)
.LI
Spin box type (numeric/string list)
.LI
Arrow Style (flat beginning/flat end/beginning/end/split)
.LI
Value range:
minimum, maximum
.LI
Increment value:
integer portion and decimal point portion
.LI
Initial value
.LI
Items list
.LI
Geometry:
x, y, width, height
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Choice"
.AL
.LI
Object name
.LI
Choice type (radio box/check box/option menu)
.LI
Position:
x, y
.LI
Label type (string/graphic)
.LI
Label
.LI
Label position (left/above)
.LI
Layout (rows/columns)
.LI
Number of rows/columns
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
List of choice items, specifying for each:
.AL a
.LI
Item label (string/graphic)
.LI
Item state (active, selected)
.LI
Item help text
.LI
Additional item help information:
help volume, location ID
.LE
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Separator"
.AL
.LI
Object name
.LI
Geometry:
x, y, width, height
.LI
Line style (none/etched in/etched out/etched in dash/etched
out dash/single line/double line/single dashed line/double
dashed line)
.LI
Orientation (horizontal/vertical)
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Button"
.AL
.LI
Object name
.LI
Button type (push/drawn/menu)
.LI
Size policy (size of label/fixed)
.LI
Geometry:
x, y, width, height
.LI
Label type (string/graphic/arrow)
.LI
Label
.LI
Arrow direction (up/down/left/right)
.LI
Label alignment (left/right/centered)
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Combo Box"
.AL
.LI
Object name
.LI
Label type (string/graphic)
.LI
Label
.LI
Label position (left/above)
.LI
Combo box type (static/editable)
.LI
List items, choosing for each
.AL a
.LI
Item label
.LI
Item state (selected/not selected)
.LE
.LI
Geometry:
x, y, width, height
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Text Field"
.AL
.LI
Object name
.LI
Position:
x, y
.LI
Width
.LI
Width units (characters/pixels)
.LI
Label type (string/graphic)
.LI
Label
.LI
Label position (left/above)
.LI
Operation (editable/read-only)
.LI
Maximum characters
.LI
Initial value
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
Attached menu name
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Scale"
.AL
.LI
Object name
.LI
Label type (string/graphic)
.LI
Label position (left/above)
.LI
Scale type (scale/gauge)
.LI
Orientation (horizontal/vertical
.LI
Direction (left to right/right to left/top to bottom/bottom
to top)
.LI
Value range:
minimum, maximum
.LI
Increment value:
integer portion, decimal point portion
.LI
Initial value
.LI
Show value (yes/no)
.LI
Geometry:
x, y, width, height
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "List"
.AL
.LI
Object name
.LI
Selection mode (single select/browse select/multiple
select/browse multiple select)
.LI
Width policy (longest item/fixed)
.LI
Geometry:
x, y, width, height
.LI
Label type (string/graphic)
.LI
Label
.LI
Label position (left/above)
.LI
Number of items visible
.LI
Initial state (visible, active)
.LI
Background colour
.LI
Foreground colour
.LI
Attached menu name
.LI
List of items, specifying for each
.AL a
.LI
Item label
.LI
Item state (selected/not selected)
.LE
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Menu"
.AL
.LI
Object name
.LI
Tearoff (enabled/disabled)
.LI
Background colour
.LI
Foreground colour
.LI
List of items, specifying for each
.AL a
.LI
Item label
.LI
Item mnemonic
.LI
Item accelerator
.LI
Item submenu
.LI
Item initial state (active)
.LE
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Group"
.AL
.LI
Object name
.LI
Position:
x, y
.LI
Layout type (as is/vertical/horizontal/grid)
.LI
Vertical alignment (left/on colons/center/right)
.LI
Horizontal alignment (top/center/bottom)
.LI
Grid arrangement (rows/columns)
.LI
Number of rows/columns in grid
.LI
Spacing type (proportional/absolute)
.LI
Spacing value
.LI
Border frame (none/shadow out/shadow in/etched out/etched
in)
.LI
Initial state (visible, active)
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Paned Window"
.AL
.LI
Object name
.LI
List of panes
.LI
Pane geometry:
x, y
.LI
Pane height:
minimum, maximum
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LI "Message Dialogues"
.AL
.LI
Object Name
.LI
Dialogue title
.LI
Message type (error/information/working/question/warning)
.LI
Message text
.LI
Action #1 enabled (yes/no)
.LI
Action #1 name
.LI
Action #2 enabled (yes/no)
.LI
Action #2 name
.LI
Cancel action enabled (yes/no)
.LI
Help action enabled (yes/no)
.LI
Default button (choosing among all those present and
enabled)
.LI
Help text
.LI
Additional help information:
help volume, location ID
.LE
.LE
.H 3 "Browser Window"
.xR3 browser
The browser window provides a tree-structured representation of the
objects in a project, using glyphs to identify each type of
object, and supports editing and manipulation of objects.
.P
More than one browser can be open at a time, but a browser need
not support the display of more than one module at a time.
If a module is being viewed by multiple browsers on the desktop,
updates in one browser cause the appropriate changes in the other browsers.
.P
The browser window supports the following operations:
.AL
.LI
View the objects by instance name or object type only
.LI
Expand/collapse subtrees in the hierarchy to hide them from view
.LI
Change the name of an object
.LI
Edit the properties of an object
.LI
Delete an object from a module
.LE
.H 3 "Application Framework"
The \*(Zx application building services
create one or more C-language source files
that the developer uses as a framework
for the application; the framework includes
sufficient source code to implement the
visual appearance and interactive characteristics of
the application, but lacks the specific
data processing logic required to achieve all
of the application's business purpose.
The developer can cause the
\*(Zx application building services to
include additional framework
source code that addresses the following \*(Zx-related activities:
.VL 11 5
.LI "ToolTalk"
.br
The developer can specify one of the following desktop message handling
capabilities desired for the application:
.BL
.LI
No message handling
.LI
.I Basic
participation in the ToolTalk desktop message set;
messages are handled in the default manner
.LI
.I Advanced
participation in the ToolTalk desktop message set;
a connection is established, but the application
is expected to handle its messages in a custom manner
.LE
.LI "Session Management"
.br
The developer can control application participation in
desktop session management, choosing among no sessioning,
command-line based sessioning (ICCCM), save-file based
sessioning (CDE) and both command-line and save-file
sessioning.
If sessioning is desired, the developer can
identify the functions to be used to save and restore
application state.
.LI "Generated Code Resource File Control"
.br
The developer can control which object-related resources
(in the X and Motif sense) will be written out to an
application resource file.
Object resources are identified
by type (colours, label strings, initial values, geometry,
other strings, and other) and each type can be selected to
appear in the application resource file produced by the
code generator.
.LI "Internationalisation"
.br
The developer can cause the code generator to determine
whether internationalisation message handling functions
should be used in the application and whether a message source text
file should be generated.
.LI "Drag and Drop"
.br
The developer can specify for any application user
interface object:
.AL
.LI
The operations the object, acting as a drag source, is
prepared to support:
Move, Copy, Link.
.LI
The pathname of the pixmap to be used as the base of the
composited drag icon.
.LI
The data types into which the application is prepared to
convert the drag object at the request of the receiving
application.
.LI
The operations the object, acting as a drop destination, is
prepared to support:
Move, Copy, Link.
.LI
The data types the receiving application is prepared to
receive.
.LI
An indication that dropping onto a child object should be
treated as a drop onto the parent object, provided the child does not
have its own drop properties explicitly defined.
.LE
.LI Help
.br
The developer can
associate help text with any object.
At least three types of help can be established:
.AL
.LI
Context-sensitive help \(em Help information for a specific
user interface element, provided for the object with input
focus whenever the user presses the
.tK help
key.
.LI
Application help \(em Help information above and beyond
context-sensitive help and provided by the application
either when the user presses a button labeled ``Help''
(typically on a dialogue) or selects a
help-related item from a menu.
.LI
On-item help \(em An interactive mode in which the
application allows the user to select (with the mouse) an object
for which help is needed.
This allows the user to
get context-sensitive help on an object that cannot take
input focus (for example, a control pane or gauge).
.LE
.LI Messages
.br
The developer can
associate message text with any object.
The following types of messages can be constructed.
.VL 11 5
.LI
Question
.br
A question that can be answered with a simple yes or no.
.LI
Warning
.br
A request for confirmation before dangerous actions are performed.
.LI
Information
.br
A message where the user's response will not alter application actions.
.LI
Error
.br
A notification that a requested action could not be performed.
.LI
Working
.br
An indication of the progress of a requested action.
.LE
.P
Each of the preceding five message types
is populated with buttons appropriate
for the category:
Help, Cancel and generic action buttons.
The developer can associate a callback routine name
with each button.
.LE
.H 3 "Connections"
The \*(Zx application building services allow the
developer to specify interrelationships that should exist
between two objects.
Each connection is defined conceptually as a rule of the form:
.Cs I
For <\f6SourceObject\fP> when <\f6Event\fP> perform <\f6Action\fP> on <\f6TargetObject\fP>
.Ce
.P
where:
.VL 11 5
.LI \f2SourceObject\fP
.br
The application or an object defined in the application
.LI \f2Event\fP
.br
Any of a set of occurrences, typically consistent with
the Xt Intrinsics and Motif event model, and tailored to
suit the type of the
.I SourceObject
.LI \f2Action\fP
.br
Any of a set of instructions that should be carried out,
consistent with the Xt Intrinsics and Motif event mode, but
augmented to include ``pseudo-events'' and suit the type of
the
.I TargetObject
.LI \f2TargetObject\fP
.br
The object upon which the action should be taken
.LE
.P
An example of such a rule is:
.Cs I
For HelpPushButton when ClickSelect perform PopUp on HelpDialog
.Ce
.H 3 "Drag and Drop Capabilities"
.xR3 appbuilddnd
The \*(Zx application building services
provide standard \*(Zx drag and drop capabilities as follows:
.AL
.LI
The developer can drag project files to the
application building services,
which in turn loads a new project.
The application building services need not
support multiple projects open simultaneously.
.LI
The developer can drag module files to the main
window of the application building services,
which in turn includes the module in the current project.
(This is equivalent to importing a module.)
.LE
.P
When the developer drags objects from the palette to the workspace,
this action does not use the
standard \*(Zx drag and drop facilities;
because of the requirements for exact positioning of
the dropped object,
there are implementation-dependent differences in the visual feedback
given to the developer as the drag proceeds.
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
CLIs and Actions are required for application portability.
.mc 1
The
.Cm dtbuilder
CLI is not planned for standardisation because Actions
are superior interfaces for this service.
Capabilities are required to ensure that conforming implementations
.mc
provide the minimum expected services,
without overly constraining tool design.
The UIL format was chosen (rather than the internal BIL) to
allow third party application builders to interface using a widely
implemented format.
.P
.mc 1
The
.Cm dtbuilder
deposits project files that represent the current state
of the build process.
This BIL format is not standardised by \*(Zx.
Thus, a user cannot transport the project
files to another CDE implementation and be assured that she can resume
building an application in progress;
just as with the
.B \&.o
files from a compiler, the BIL files
are intended to be used only on the system on which they were created.
.br
.mc
.eF e
