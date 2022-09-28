.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "File Management Services"
.xR1 filemgmtsvcs
.H 2 "Introduction"
.mc 5
The \*(Zx file management services provide the primary
interface to the objects used in the \*(ZX.
These services provide a graphical interface
for object and folder manipulation and for application execution.
.P
The \*(Zx file management services interface consists of a
.mc 6
window that shows the contents of a single
folder or a set of nested folders.
.mc 5
These services provide
many manipulation functions, such as folder traversal,
creating, moving, deleting objects and invoking actions
(such as Edit or Print) on objects.
.P
.mc 6
The \*(Zx file management services also provide windows for
two special types of folders:
application and trash.
Application folders are used to organise application
objects such as actions.
The trash folder is a holding
area for objects that a user wishes to delete.
.br
.mc
.H 2 "Actions" S
.xR2 filemgractions
This section defines the actions that provide \*(Zx file management
services to support application portability at the C-language source
or shell script levels.
.so ../hx/dtfileaction.mm
.so ../hx/dttrashaction.mm
.ds XC Messages
.SK
.H 2 "Messages"
The \*(Zx file management services implement the
.I Display
and
.I Edit
requests for media type FILE_NAME.
See the \*(ZT.
These services also respond to the
.I Quit
desktop message.
See the \*(Zv.
.H 2 "Capabilities" S
A conforming implementation of the \*(Zx file management services
supports at least the following capabilities:
.AL
.LI
Provides file management services
as described in the following subsections.
.LI
Conforms to the Required items in the Application Style Checklist in
.cX checklist ,
with the following exception that may exist on some implementations:
.AL a
.LI
There are certain operations that need not comply with
checklist item 3-1.
The selection of a folder in the
iconic representation of a pathname and the ability to
invoke the default action of the folder using the Enter key
need not be available.
.LE
.LI
Has been internationalised using the standard interfaces
in the \*(ZK, the \*(Zl, and the \*(ZM,
and supports any locale available in the underlying X Window System
Xlib implementation.
.LE
.H 3 "Folder Window"
The \*(Zx file management services provide a
.mc 6
window that shows the contents of a single folder or a set
of nested folders.
The user can configure how objects in a folder are displayed:
as icons or textually.
More than one folder window can be displayed at a time.
The user can update folder windows and select/unselect individual or all
objects in a folder.
When a set of nested folders are
displayed, the user can expand or collapse folder branches
and control whether all objects in a folder or just the
folder is shown.
.P
.mc
The user can select from the following display modes:
.BL
.LI
Objects are displayed in a sorted grid or can be placed
by the user
.LI
.mc 6
Objects can be displayed by name, by name with a large icon, by name with
a small icon, or by name, date and size.
.LI
.mc
Objects can be sorted alphabetically, by file type, by date
or by size;
objects sorted by file type are first sorted alphabetically by
file type and are then sorted alphabetically by name
within each file type
.LI
Objects can be sorted in ascending or descending order
.LI
Objects can be displayed based on the object type
.LE
.mc 6
.H 3 "Application Folder Window"
The \*(Zx file management services provide
a special window that shows the contents
of an application folder.
An application folder is a
restricted folder that is used to organise application
objects such as actions.
.mc
The user is not allowed to
traverse above the root application folder.
.mc 6
.H 3 "Trash Folder Window"
.xR3 trash
The \*(Zx file management services provide a special window
that shows the contents of the trash folder.
The trash folder is a restricted folder that is used to store objects
.mc
temporarily until the user asks to permanently remove them.
The user is not allowed to traverse out of the trash folder.
The user can restore objects from the trash folder
if they have not been permanently removed.
.H 3 "Workspaces"
.mc 6
The user can place frequently used objects
on the workspace for easier accessibility.
The user can also select actions for that object via a pop-up menu.
.mc
.H 3 "Object Movement and Modification"
The \*(Zx file management services allow manipulation of
objects in several different ways,
based on appropriate permissions and the type of the object.
The user can create,
move, copy, create symbolic links
to and delete applicable objects by using menu options
and by using drag-and-drop.
Objects can be
renamed by using menu options or by mouse selection;
the objects that cannot be renamed
are implementation-dependent.
Object permissions can be changed using menu options.
.H 3 "Object Search"
The \*(Zx file management services allow searching for
objects by name or by content.
Searching by name
.mc 6
performs a case-sensitive search for
an object (or objects).
Searching by content performs a case-insensitive search
.mc
for a specified character string.
By default, the current folder and its sub-folders are searched;
the user can specify a different folder (and its sub-folders)
to be searched.
.mc 6
A list of objects matching the search criteria is shown to the user.
For any object in the list, the user can choose
.mc
to create a window to display the folder where the object
is located or to place the object on the current workspace.
.H 3 "Folder Traversal"
Simple folder traversal can be accomplished by invoking the
default action on a folder icon.
This action displays the contents of the selected folder.
Traversal to a new folder
by typing in the folder name and menu options for traversal
to the user's home directory and parent folders are also supported.
.H 3 "Object Type/Action Association"
.mc 6
An Actions menu is provided to show the Actions specified for each
selected object.
Actions are invoked for an object by selecting an object
.mc
and then selecting an action from the Actions menu.
.mc 6
If the ACTIONS attribute is defined for an object's data type,
invoking the default action on an object activates the
first Action listed in the Action attribute.
.mc
.H 3 "Registering Objects as Drop Sites"
Every \*(Zx data type has three associated drop attributes:
MOVE_TO_ACTION, COPY_TO_ACTION and LINK_TO_ACTION;
see the \*(Zy.
The \*(Zx file management services register every object whose
data type has a value for at least one of these attributes
as a drop site.
Objects can be dragged between different
file management services
.mc 6
folder windows, to workspaces and to
cooperating clients.
Direct manipulation can be used to
.mc
supply objects as input to any user-defined action (for
example, a Move action defined such that the dragged
objects are moved to the dropped-on object).
.H 3 "Exit Services"
Menu options are provided that allow the user to close file
management services views.
.P
.mc 6
The following information is saved as a consequence
of exiting a desktop session:
.AL
.mc
.LI
The number and location of file management windows
.LI
The number and location of workspace objects
.LI
.mc 6
The number and location of objects placed on the
workspace from file management services
.LE
.mc
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
Actions are required for application portability.
ToolTalk message
protocols are required for application portability.
Capabilities are
required to ensure that conforming implementations provide the minimum
expected services, without overly constraining tool design.
The
.Cm dtfile
CLI is not planned for standardisation because manipulation of this
component is typically accomplished after invocation.
.eF e
