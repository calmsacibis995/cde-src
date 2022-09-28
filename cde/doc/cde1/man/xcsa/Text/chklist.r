.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Application Style Checklist"
.xR1 checklist
.nr Ej 1
.mc 5
This chapter provides the list of style requirements for
\*(Zx applications.
\*(Zx requirements consist of the
\*(XX Motif requirements with \*(Zx-specific additions.
.iN
This chapter contains normative style requirements for applications.
It also contains recommended and optional behaviour
from the \*(Zx Style Guide,
which is offered as a courtesy to readers
who wish to refer to this informative material within the
context of the normative material.
For the purposes of classifying input
in the \*(XX Fast Track review of \*(Zx,
the sections marked as recommended and optional behaviour
should be considered as rationale,
subject only to comments and editorial review.
.sA
.P
.mc
The checklist describes keys using a model keyboard mechanism.
Wherever keyboard input is specified, the keys
are indicated by the engravings that they have on the \*(XX
Motif model keyboard.
Mouse buttons are described using a
virtual button mechanism to better describe behaviour
independent from the number of buttons on the mouse.
For more information on the model keyboard and virtual button
mechanisms, see the Preface and Section 2.2.1, ``Pointing
Devices'' of the \*(ZY.
.P
This checklist uses typographical conventions for keyboard
and mouse inputs that differ from those used in the \*(ZY.
.P
By default, this checklist assumes that the application is
being designed for a left-to-right language environment in
an English-language locale.
Some sections of the checklist
may require appropriate changes for other locales.
.P
Each item in this checklist contains the corresponding
section number from the checklist in the \*(ZY, if the item
came from that list.
Each item is also followed by a note containing a brief explanation or
justification.
.P
The headings used in this checklist correspond to the
headings in the \*(ZY and the checklist items are labelled
with the numbers used in that document.
The \*(Zx-specific
additions are labelled with alphabetic identifiers.
.P
Each checklist item also has a priority label:
Required,
Recommended or Optional.
The required items must be
followed for an application to be \*(Zx compliant.
Recommended items should be followed where feasible.
Optional items are alternative implementations that the
interface designer can choose.
.H 2 "Preface"
.VL 11 5
.LI "\f31-1:\f1\0[Required]"
.br
Each of the non-optional keys described on the \*(XX Motif
model keyboard is available either as specified or by using
other keys or key combinations if the specified key is
unavailable (Preface).
.Ns
The model keyboard does not correspond directly to any
existing keyboard;
rather, it assumes a keyboard with an
ideal set of keys.
However, to ensure consistency across
applications, the non-optional keys or substitutes for them
must always be available.
.Ne
.LE
.H 2 "Input Models"
.H 3 "The Keyboard Focus Model"
.VL 11 5
.LI "\f32-1:\f1\0[Required]"
.br
Only one window at a time has the keyboard focus.
The window that has the focus is highlighted.
Within the
window that has the keyboard focus, only one component at a
time has the focus
(Section 2.1 of the \*(ZY).
.Ns
The keyboard focus determines which component on the screen
receives keyboard events.
This requirement prevents confusion
about which window and component have the focus.
.Ne
.LI "\f32-2:\f1\0[Required]"
.br
When the application uses an explicit focus policy,
pressing BSelect does not move focus to a component that is
not traversable or does not accept input
(Section 2.1.2 of the \*(ZY).
.Ns
An explicit focus policy requires the user to explicitly
select which window or component receives the keyboard
focus.
Generally, the user gives the focus to a window or
component by pressing BSelect over it.
However, this
policy must not allow the user to give focus to a component
that is not traversable or does not accept input.
.Ne
.LI "\f32-3:\f1\0[Required]"
.br
When the application uses an explicit focus policy, the
component with the keyboard focus is highlighted by a
location cursor
(Section 2.1.2 of the \*(ZY).
.Ns
The user must know the location of the keyboard focus
to be able to control an application.
.Ne
.LE
.H 3 "The Input Device Model"
.VL 11 5
.LI "\f32-4:\f1\0[Required]"
.br
The application supports methods of interaction for
keyboard-only users.
All features of the application are
available from the keyboard
(Section 2.2 of the \*(ZY).
.Ns
Not all users have access to a pointing device.
These users must be able to access the full functionality of the
application from the keyboard.
Additionally, advanced
users will be able to use the keyboard to perform some
tasks more quickly than with a pointing device.
.Ne
.LI "\f32-5:\f1\0[Required]"
.br
The application uses the following bindings for mouse
buttons
(Section 2.2.1 of the \*(ZY):
.VL 11 5
.LI "\f3BSelect\f1\0[Optional]"
.br
This button should be used for selection, activation and
setting the location cursor, is the leftmost button, except
for left-handed users, where it can be the rightmost
button.
.LI "\f3BTransfer\f1\0[Optional]"
.br
This button should be used for moving and copying elements,
is the middle mouse button, unless dragging is integrated
with selection or the mouse has fewer than three buttons.
.LI "\f3BMenu\f1\0[Optional]"
.br
This button should be used for popping up menus, is the
rightmost button, except for left-handed users, where it
can be the leftmost button or unless the mouse has fewer
than three buttons.
If the mouse has one button, BMenu is
bound to Alt+BSelect.
.LE
.Ns
These bindings ensure a consistent interface for using
standard mouse-based operations across applications.
.Ne
.LI "\f32-6:\f1\0[Required]"
.br
The application does not warp the pointer unless the
application have given the user a means of disabling the
behaviour
(Section 2.2.4 of the \*(ZY).
.Ns
The pointer position is intended only as input to
applications, not as an output mechanism.
An application
warps the pointer when it changes the pointer's position.
This practice is confusing to users and reduces their sense
of control over an application.
Warping the pointer can
also cause problems for users of absolute location pointing
devices.
.Ne
.mc 6
.LI "\f3a:\f1\0[Required]"
.br
Components and applications that have functions
corresponding to the Motif\-\*(Zx virtual keys must
support those keys.
.Ns
.mc
If these virtual keys are available, the following mappings
should be used.
Priorities indicate the importance of
implementing these functions in the application.
.VL 11 5
.mc 6
.LI "\f3Help = F1\f1\0[Required]"
.br
Pressing the Help key must provide the user with help
information in a window or in the status area.
.LI "\f3Properties = Control+I\f1\0[Required]"
.br
Pressing the Properties key must invoke a dialog box for
making object-specific settings.
.LI "\f3Undo = Control+Z\f1\0[Required]"
.br
Pressing the Undo key must reverse the effect of the last
applied operation.
.mc
This is the primary key mapping for
Undo.
.LI "\f3Undo = Alt+Backspace\f1\0[Optional]"
.br
This is a secondary key mapping for Undo.
It should be
supported in addition to Control+Z to help users migrating
from previous versions of Motif, Microsoft Windows or
OS/2.
.br
.mc 6
.LI "\f3Cut = Control+X\f1\0[Required]"
.br
Pressing the Cut key must remove the selected object and
places it in the clipboard.
.mc
This is the primary key
mapping for Cut.
.LI "\f3Cut = Shift+Delete\f1\0[Optional]"
.br
This is a secondary key mapping for Cut.
It should be
supported in addition to Control+X to help users migrating
from previous versions of Motif, Microsoft Windows or
OS/2.
.br
.mc 6
.LI "\f3Copy = Control+C\f1\0[Required]"
.br
Pressing the Copy key must place a copy of the selected
object in the clipboard.
.mc
This is the primary key mapping
for Copy.
.LI "\f3Copy = Control+Insert\f1\0[Optional]"
.br
This is a secondary key mapping for Copy.
It should be
supported in addition to Control+C to help users migrating
from previous versions of Motif, Microsoft Windows or
OS/2.
.br
.mc 6
.LI "\f3Paste = Control+V\f1\0[Required]"
.br
Pressing the Paste key must place the contents of the
clipboard at the selected location.
.mc
This is the primary
key mapping for Paste.
.LI "\f3Paste = Shift+Insert\f1\0[Optional]"
.br
This is a secondary key mapping for Paste.
It should be
supported in addition to Control+V to help users migrating
from previous versions of Motif, Microsoft Windows or
OS/2.
.br
.mc 6
.LI "\f3Open = Control+O\f1\0[Optional]"
.br
Pressing the Open key opens the object,
which is typically the default action.
.br
.mc
.LI "\f3Stop = Control+S\f1\0[Optional]"
.br
Pressing the Stop key cancels an operation.
.LI "\f3Again = Control+A\f1\0[Optional]"
.br
Pressing the Again key repeats the last operation.
.LI "\f3Print = Control+P\f1\0[Optional]"
.br
Pressing the Print key initiates printing.
.LI "\f3Save = Control+S\f1\0[Optional]"
.br
Pressing the Save key saves the current file.
.LI "\f3New = Control+N\f1\0[Optional]"
.br
.mc 6
Pressing the New key creates a new object.
.br
.mc
.LI "\f3Bold = Control+B\f1\0[Optional]"
.br
Pressing the Bold key makes the selected text bold.
.LI "\f3Italic = Control+I\f1\0[Optional]"
.br
Pressing the Italic key italicises the selected text.
.LI "\f3Underline = Control+U\f1\0[Optional]"
.br
Pressing the Underline key underlines the current text.
.Ne
.LE
.LE
.H 2 "Navigation"
.H 3 "Mouse-Based Navigation"
.VL 11 5
.LI "\f33-1:\f1\0[Required]"
.br
When the keyboard focus policy is explicit, pressing
BSelect on a component moves focus to it, except for
components, such as scroll bars, that are used to adjust
the size and location of other elements
(Section 3.1 of the \*(ZY).
.Ns
BSelect provides a convenient mechanism for using the mouse
to move focus when the keyboard focus policy is explicit.
.Ne
.LI "\f33-2:\f1\0[Required]"
.br
When the pointer is on a menu, the application uses BSelect
Press to activate the menu in a spring-loaded manner
(Section 3.1 of the \*(ZY).
.Ns
A spring-loaded menu is one that appears when the user
presses a mouse button, remains on the screen for as long
as the button is pressed and disappears when the user
releases the button.
BSelect, the first mouse button,
provides a means of activating spring-loaded menus that is
consistent across applications.
.Ne
.LI "\f33-3:\f1\0[Required]"
.br
When the pointer is in an element with an inactive pop-up
menu and the context of the element allows the pop-up menu
to be displayed, the application uses BMenu Press to
activate the pop-up menu in a spring-loaded manner
(Section 3.1 of the \*(ZY).
.Ns
The availability of a pop-up menu can depend on the
location of the pointer within an element, the contents of
an element or the selection state of an element.
BMenu,
the third mouse button, provides a consistent means of
activating a spring-loaded pop-up menu.
.Ne
.LI "\f33-4:\f1\0[Required]"
.br
If the user takes an action to post a pop-up menu and a
menu can be posted for both an inner element and an outer
element that contains the inner element, the pop-up menu
for the internal element is posted
(Section 3.1 of the \*(ZY).
.Ns
This requirement ensures that the pop-up menu for an
internal element is always accessible.
.Ne
.LI "\f33-5:\f1\0[Required]"
.br
Once a pop-up menu is posted, BMenu behaves just as BSelect
does for any menu system
(Section 3.1 of the \*(ZY).
.Ns
The specified operation of BMenu is for manipulating pop-up
menus.
.Ne
.LI "\f33-6:\f1\0[Required]"
.br
BSelect is also available from within posted pop-up menus
and behaves just as in any menu system
(Section 3.1 of the \*(ZY).
.Ns
Once a pop-up menu is posted, the user can select an
element from it using the standard selection mechanism,
BSelect.
.Ne
.LI "\f33-7:\f1\0[Required]"
.br
When a menu is popped up or pulled down in a posted manner,
the application places the location cursor on the menu's
default entry or on the first entry in the menu if there is
no default entry
(Section 3.1 of the \*(ZY).
.Ns
A posted menu remains visible until it is explicitly
unposted.
Placing the location cursor on the default entry
allows the user to select the default operation easily.
When there is no default entry, placing the location cursor
on the first entry yields uniform behaviour across
applications.
.Ne
.LI "\f33-8:\f1\0[Required]"
.br
The application removes a spring-loaded menu system when
the mouse button that activated it is released, except when
the button is released on a cascade button in the menu
hierarchy
(Section 3.1 of the \*(ZY).
.Ns
The concept of a spring-loaded menu system requires that
the menu disappear when the mouse button is released.
.Ne
.LI "\f33-9:\f1\0[Required]"
.br
While a spring-loaded menu system is popped up or pulled
down, moving the pointer within the menu system moves the
location cursor to track the pointer
(Section 3.1 of the \*(ZY).
.Ns
Once a spring-loaded menu system has appeared on the
screen, the user must be able to maneuver the location
cursor through the menu system using the mouse.
.Ne
.LI "\f33-10:\f1\0[Required]"
.br
When a spring-loaded menu system is popped up or pulled
down and the pointer rests on a cascade button, the
associated menu is pulled down and becomes traversable.
The associated menu is removed, possibly after a short
delay, when the pointer moves to a menu item outside of the
menu or its cascade button
(Section 3.1 of the \*(ZY).
.Ns
The user must be able to use the mouse to access all of
the associated menus of a menu system.
This feature allows
the user to move quickly to any menu in a menu system.
.Ne
.LI "\f33-11:\f1\0[Required]"
.br
When a spring-loaded menu system that is part of the menu
bar is pulled down, moving the pointer to any other element
on the menu bar unposts the current menu system and posts
the pull-down menu associated with the new element
(Section 3.1 of the \*(ZY).
.Ns
This feature of a spring-loaded menu system allows the user
to browse quickly through all of the menus attached to a
menu bar.
.Ne
.LI "\f33-12:\f1\0[Required]"
.br
When a spring-loaded menu system is popped up or pulled
down and the button that activated the menu system is
released within a component in the menu system, that
component is activated.
If the release is on a cascade
button or an option button, the associated menu is
activated in a posted manner if it was not posted prior to
the associated button press
(Section 3.1 of the \*(ZY).
.Ns
Releasing the mouse button that activated a spring-loaded
menu provides a means of activating a menu element that is
consistent across applications.
.Ne
.LI "\f33-13:\f1\0[Required]"
.br
When the pointer is in an area with a pop-up menu, the
application uses BMenu Click to activate the menu in a
posted manner if it was not posted prior to the BMenu Click
(Section 3.1 of the \*(ZY).
.Ns
BMenu Click provides a means of posting a pop-up menu that
is consistent across applications.
.Ne
.LI "\f33-14:\f1\0[Required]"
.br
Once a pull-down or option menu is posted, BSelect Press in
the menu system causes the menu to behave as a
spring-loaded menu
(Section 3.1 of the \*(ZY).
.Ns
This feature of a posted pull-down or option menu allows
the user to switch easily between using a posted menu and a
spring-loaded menu.
.Ne
.LI "\f33-15:\f1\0[Required]"
.br
If a button press unposts a menu and that button press is
not also passed to the underlying component, subsequent
events up to and including the button release are not
passed to the underlying component
(Section 3.1 of the \*(ZY).
.Ns
When a button press unposts a menu, the press can be passed
to the underlying component.
Whether or not it is passed
to the underlying component, the press can have additional
effects, such as raising and giving focus to the underlying
window.
If the press is not passed to the underlying
component, events up to and including the release must not
be passed to that component.
.Ne
.LI "\f33-16:\f1\0[Required]"
.br
Once a pop-up menu is posted, BSelect Press or BMenu Press
in the menu system causes the menu to behave as a
spring-loaded menu
(Section 3.1 of the \*(ZY).
.Ns
This feature of a posted pop-up menu allows the user to
switch easily between using a posted menu and a
spring-loaded menu.
.br
.Ne
.LI "\f3b:\f1\0[Optional]"
.br
BMenu Press or BMenu Click on a menu bar item displays the
menu.
.br
.mc 6
.LI "\f3c:\f1\0[Required]"
.br
.mc
BMenu Press or BMenu Click on an option button displays the
option menu.
.LI "\f3d:\f1\0[Required]"
.br
BSelect Press on a text entry field causes the text cursor
to be inserted at the mouse cursor position.
.LE
.H 3 "Keyboard-Based Navigation"
.VL 11 5
.LI "\f33-17:\f1\0[Required]"
.br
In a text component, the text cursor is shown differently
when the component does and does not have the keyboard
focus
(Section 3.2.1 of the \*(ZY).
.Ns
In a text component, the text cursor serves as the location
cursor and, therefore, must indicate whether the component
has keyboard focus.
.Ne
.LI "\f33-18:\f1\0[Required]"
.br
If a text component indicates that it has lost the keyboard
focus by hiding the text cursor and if the component
subsequently regains the focus, the cursor reappears at the
same position it had when the component lost focus
(Section 3.2.1 of the \*(ZY).
.Ns
To ensure predictability, the text
cursor must not change position when a text component loses and
then regains the keyboard focus.
.Ne
.LI "\f33-19:\f1\0[Required]"
.br
If a small component, such as a sash, indicates that it has
the keyboard focus by filling, no other meaning is
associated with the filled state
(Section 3.2.1 of the \*(ZY).
.Ns
This requirement reduces possible confusion about the significance
of filling in a small component.
.Ne
.LI "\f33-20:\f1\0[Required]"
.br
All components are designed and positioned within the
application so that adding and removing each component's
location cursor does not change the amount of space that
the component takes up on the screen
(Section 3.2.1 of the \*(ZY).
.Ns
For visual consistency, the sizes and positions of
components should not change when keyboard focus moves from
one component to another.
.Ne
.LI "\f33-21:\f1\0[Required]"
.br
ControlTab moves the location cursor to the next field and
Control+Shift+Tab moves the location cursor to the previous
field.
Unless Tab and Shift+Tab are used for internal
navigation within a field, Tab also moves the location
cursor to the next field and Shift+Tab also moves the
location cursor to the previous field
(Section 3.2.3 of the \*(ZY).
.Ns
These keys provide a consistent means of navigating among
fields in a window.
.Ne
.LI "\f33-22:\f1\0[Required]"
.br
Tab (if not used for internal navigation) and Control+Tab
move the location cursor forward through fields in a window
according to the following requirements
(Section 3.2.3 of the \*(ZY):
.BL
.LI
If the next field is a control, Tab (if not used for
internal navigation) and Control+Tab move the location
cursor to that control.
.LI
If the next field is a group, Tab (if not used for internal
navigation) and Control+Tab move the location cursor to a
traversable component within the group.
.LI
If the next field contains no traversable components, Tab
(if not used for internal navigation) and Control+Tab skip
the field.
.LE
.Ns
These requirements ensure the consistent operation of Tab (if not
used for internal navigation) and Control+Tab across
applications.
.Ne
.LI "\f33-23:\f1\0[Required]"
.br
Shift+Tab (if not used for internal navigation) and
Control+Shift+Tab move the location cursor backward through
fields in the order opposite to that of Tab (if not used
for internal navigation) and Control+Tab
(Section 3.2.3 of the \*(ZY).
.Ns
These requirements result in the uniform operation of Shift+Tab
(if not used for internal navigation) and Control+Shift+Tab
across applications.
.Ne
.LI "\f33-24:\f1\0[Required]"
.br
When a window acquires focus, the location cursor is placed
on the control that last had focus in the window, providing
that all the following conditions are met
(Section 3.2.3 of the \*(ZY):
.BL
.LI
The window uses an explicit keyboard focus policy.
.LI
The window acquires the focus through keyboard navigation
or through a button press other than within the client area
of the window.
.LI
The window had the focus at some time in the past.
.LI
The control that last had focus in the window is still
traversable.
.LE
.Ns
This requirement ensures that when the user returns to a window
after navigating away, the focus returns to the component
where the user left it.
.Ne
.LI "\f33-25:\f1\0[Required]"
.br
Field navigation wraps between the first and last fields in
the window.
(Section 3.2.3 of the \*(ZY).
.Ns
This feature of field navigation provides the user with a
convenient way to move through all of the fields in a window.
.Ne
.LI "\f33-26:\f1\0[Required]"
.br
When the Down Arrow and Up Arrow keys are used for
component navigation within a field, they behave according
to the following requirements
(Section 3.2.3 of the \*(ZY):
.P
In a left-to-right language environment, the Down Arrow key
moves the location cursor through all traversable controls
in the field, starting at the upper left and ending at the
lower right, then wrapping to the upper left.
If the
controls are aligned in a matrix-like arrangement, Down
Arrow first traverses one column from top to bottom, then
traverses the column to its right and so on.
In a right-to-left language environment, Down Arrow moves the
location cursor through all traversable controls, starting
at the upper right and ending at the lower left.
.BL
.LI
Up Arrow moves the location cursor through all traversable
controls in the field in the order opposite to that of Down
Arrow.
.LE
.Ns
These requirements ensure a consistent means of navigating among
components using the directional keys.
.Ne
.LI "\f33-27:\f1\0[Required]"
.br
When the Right Arrow and Left Arrow keys are used for
component navigation within a field, they behave according
to the following rules
(Section 3.2.3 of the \*(ZY):
.BL
.LI
In a left-to-right language environment, Right Arrow moves
the location cursor through all traversable controls in the
field, starting at the upper left and ending at the lower
right, then wrapping to the upper left.
If the controls
are aligned in a matrix-like arrangement, Right Arrow first
traverses one row from left to right, then traverses the
row below it and so on.
In a right-to-left language
environment, Right Arrow moves the location cursor through
all traversable controls, starting at the lower left and
ending at the upper right.
.LI
Left Arrow moves the location cursor through all
traversable controls in the field in the order opposite to
that of Right Arrow.
.LE
.Ns
These requirements ensure a consistent means of navigating among
components using the directional keys.
.Ne
.LI "\f33-28:\f1\0[Required]"
.br
If a control uses Right Arrow and Left Arrow for internal
navigation, Begin moves the location cursor to the leftmost
edge of the data or the leftmost element in a left-to-right
language environment.
In a right-to-left language
environment, Begin moves the location cursor to the
rightmost edge of the data or the rightmost element
(Section 3.2.3 of the \*(ZY).
.Ns
This requirement permits convenient navigation to the left or
right edge of the data or the left or right element in a control.
.Ne
.LI "\f33-29:\f1\0[Required]"
.br
If a control uses the Right Arrow and Left Arrow keys for
internal navigation, the End key moves the location cursor
to the rightmost edge of the data or the rightmost element
in a left-to-right language environment.
In a right-to-left language environment, End moves the location
cursor to the leftmost edge of the data or the leftmost
element
(Section 3.2.3 of the \*(ZY).
.Ns
This requirement permits convenient navigation to the left or
right edge of the data or the left or right element in a control.
.Ne
.LI "\f33-30:\f1\0[Required]"
.br
If a control uses the Up Arrow and Down Arrow keys for
internal navigation, Control+Begin moves the location
cursor to one of the following
(Section 3.2.3 of the \*(ZY):
.BL
.LI
The first element
.LI
The topmost edge of the data
.LI
In a left-to-right language environment, the topmost left
edge of the data;
in a right-to-left language environment,
the topmost right edge of the data
.LE
.Ns
This requirement permits convenient navigation to the beginning of
the data in a control.
.Ne
.LI "\f33-31:\f1\0[Required]"
.br
If a control uses the Up Arrow and Down Arrow keys for
internal navigation, Control+End moves the location cursor
to one of the following
(Section 3.2.3 of the \*(ZY):
.BL
.LI
The last element
.LI
The bottommost edge of the data
.LI
In a left-to-right language environment, the bottommost
right edge of the data;
in a right-to-left language
environment, the bottommost left edge of the data
.LE
.Ns
This requirement permits convenient navigation to the end of the
data in a control.
.Ne
.LI "\f3e:\f1\0[Optional]"
.br
Each time a new window is opened, keyboard focus is placed
in the first field or location within the window or in a
default location, if this is appropriate for the particular
window.
.LI "\f3f:\f1\0[Required]"
.br
The Tab key moves input focus between push buttons within a
group.
.Ns
The arrow keys also move the selected focus per the \*(ZY.
.Ne
.LI "\f3g:\f1\0[Required]"
.br
The application uses the Control, Shift and Alt keys only to modify the
function of other keys or key combinations.
.LI "\f3h:\f1\0[Optional]"
.br
The application should use the Alt key only to provide access to mnemonics.
.LE
.H 3 "Menu Traversal"
.VL 11 5
.LI "\f33-32:\f1\0[Required]"
.br
If the user traverses to a menu while the keyboard focus
policy is implicit, the focus policy temporarily changes to
explicit and reverts to implicit whenever the user
traverses out of the menu system
(Section 3.3 of the \*(ZY).
.Ns
Menus must always be traversable, even when the keyboard
focus policy is generally implicit.
.Ne
.LI "\f33-33:\f1\0[Required]"
.br
The application uses the F10 key to activate the menu bar
system if it is inactive.
The location cursor is placed on
the first traversable cascade button in the menu bar.
If there are no traversable cascade buttons, the key does
nothing
(Section 3.3 of the \*(ZY).
.Ns
F10 provides a consistent means of traversing to the menu
bar using the keyboard.
.Ne
.LI "\f33-34:\f1\0[Required]"
.br
When the keyboard focus is in an element with an inactive
pop-up menu and the context of the element allows the
pop-up menu to be displayed, the application uses the menu
key to activate the pop-up menu.
The location cursor is
placed on the default item of the menu or on the first
traversable item in the pop-up menu if there is no default
item
(Section 3.3 of the \*(ZY).
.Ns
The Menu key provides a uniform way of activating a pop-up
menu from the keyboard.
.Ne
.LI "\f33-35:\f1\0[Required]"
.br
When the keyboard focus is in an option button, the
application uses the Select key or the Spacebar to post the
option menu.
The location cursor is placed on the
previously selected item in the option menu;
or, if the
option menu has been pulled down for the first time, the
location cursor is placed on the default item in the menu.
If there is an active option menu, the Return, Select or
Spacebar keys select the current item in the option menu,
unpost the menu system and return the location cursor to
the option button
(Section 3.3 of the \*(ZY).
.Ns
These keys provide a means of posting an option menu from
the keyboard that is consistent across applications.
.Ne
.LI "\f33-36:\f1\0[Required]"
.br
The application uses the Down Arrow, Left Arrow, Right
Arrow and Up Arrow keys to traverse through the items in a
menu system
(Section 3.3 of the \*(ZY).
.Ns
The Down Arrow, Left Arrow, Right Arrow and Up Arrow
directional keys provide a consistent means of navigating
among items in a menu system.
.Ne
.LI "\f33-37:\f1\0[Required]"
.br
When a menu traversal action traverses to the next or
previous component in a menu or menu bar, the order of
traversal and the wrapping behaviour are the same as that
of the corresponding component navigation action within a
field, as described in Section 3.2.3
(Section 3.3 of the \*(ZY).
.Ns
This requirement provides consistency between menu
traversal and component navigation within a field.
.Ne
.LI "\f33-38:\f1\0[Required]"
.br
If the application uses any two-dimensional menus, they do
not contain any cascade buttons
(Section 3.3 of the \*(ZY).
.Ns
Cascade buttons in a two-dimensional menu would restrict
the user's ability to navigate to all of the elements of
the menu using the keyboard.
.Ne
.LI "\f33-39:\f1\0[Required]"
.br
When focus is on a component in a menu or menu bar system,
the Down Arrow key behaves according to the following rules
(Section 3.3 of the \*(ZY):
.BL
.LI
If the component is in a vertical or two-dimensional menu,
traverse down to the next traversable component, wrapping
within the menu if necessary.
.LI
If the component is in a menu bar and the component with
the keyboard focus is a cascade button, post its associated
pull-down menu and traverse to the default entry in the
menu or, if the menu has no default, to the first
traversable entry in the menu.
.LE
.Ns
This requirement results in consistent operation of the
directional keys in a menu or menu bar system.
.Ne
.LI "\f33-40:\f1\0[Required]"
.br
When focus is on a component in a menu or menu bar system,
the Up Arrow key behaves according to the following rules
(Section 3.3 of the \*(ZY):
.BL
.LI
If the component is in a vertical or two-dimensional menu,
this action traverses up to the previous traversable
component, wrapping within the menu if necessary and
proceeding in the order opposite to that of the Down Arrow key.
.LE
.Ns
This requirement results in consistent operation of the
directional keys in a menu or menu bar system.
.Ne
.LI "\f33-41:\f1\0[Required]"
.br
When focus is on a component in a menu or menu bar system,
the Left Arrow key behaves according to the following rules
(Section 3.3 of the \*(ZY):
.BL
.LI
If the component is in a menu bar or two-dimensional menu,
but not at the left edge, traverse left to the previous
traversable component.
.LI
If the component is at the left edge of a menu bar, wrap
within the menu bar.
.LI
If the component is at the left edge of a vertical or
two-dimensional menu that is the child of a vertical or
two-dimensional menu, unpost the current menu and traverse
to the parent cascade button.
.LI
If the component is at the left edge of a vertical or
two-dimensional menu that is the child of a menu bar,
unpost the current menu and traverse left to the previous
traversable entry in the menu bar.
If that entry is a
cascade button, post its associated pull-down menu and
traverse to the default entry in the menu or, if the menu
has no default, to the first traversable entry in the menu.
.LE
.Ns
This requirement results in consistent operation of the
directional keys in a menu or menu bar system.
.Ne
.LI "\f33-42:\f1\0[Required]"
.br
When focus is on a component in a menu or menu bar system,
the Right Arrow key behaves according to the following rules
(Section 3.3 of the \*(ZY):
.BL
.LI
If the component is a cascade button in a vertical menu,
post its associated pull-down menu and traverse to the
default entry in the menu or, if the menu has no default,
to the first traversable entry in the menu.
.LI
If the component is in a menu bar or two-dimensional menu,
but not at the right edge, traverse right to the next
traversable component.
.LI
If the component is at the right edge of a menu bar, wrap
within the menu bar.
.LI
If the component is not a cascade button and is at the
right edge of a vertical or two-dimensional menu and if the
current menu has an ancestor cascade button (typically in a
menu bar) from which the Down Arrow key posts its
associated pull-down menu, unpost the menu system pulled
down from the nearest such ancestor cascade button and
traverse right from that cascade button to the next
traversable component.
If that component is a cascade
button, post its associated pull-down menu and traverse to
the default entry in the menu or, if the menu has no
default, to the first traversable entry in the menu.
.LE
.Ns
This requirement results in consistent operation of the
directional keys in a menu or menu bar system.
.Ne
.LI "\f33-43:\f1\0[Required]"
.br
All menu traversal actions, with the exception of menu
posting, traverse to tear-off buttons in the same way as
for other menu entries
(Section 3.3 of the \*(ZY).
.Ns
Traversal of tear-off buttons must be consistent with
traversal of other menu items.
.Ne
.LI "\f33-44:\f1\0[Required]"
.br
If the application uses the F10, Menu or Cancel key to
unpost an entire menu system and an explicit focus policy
is in use, the location cursor is moved back to the
component that had it before the menu system was posted
(Section 3.3 of the \*(ZY).
.Ns
Returning the location cursor to the component that had it
previously allows the user to resume a task without
disruption.
.Ne
.LE
.H 3 "Scrollable Component Navigation"
.VL 11 5
.LI "\f33-45:\f1\0[Required]"
.br
Any scrollable components within the application support
the appropriate navigation and scrolling operations.
The application uses the page navigation keys Page Up, Page
Down, Control+Page Up (for Page Left) and Control+Page Down
(for Page Right) for scrolling the visible region by a page
increment
(Section 3.4 of the \*(ZY).
.Ns
A user must be able to view and access the entire
contents of a scrollable component.
.Ne
.LI "\f33-46:\f1\0[Required]"
.br
When scrolling by a page, the application leaves at least
one unit of overlap between the old and new pages
(Section 3.4 of the \*(ZY).
.Ns
The overlap between one page and the next yields visual
continuity for the user.
.Ne
.LI "\f33-47:\f1\0[Required]"
.br
Any keyboard operation that moves the cursor to or in the
component or that inserts, deletes or modifies items at the
cursor location scrolls the component so that the cursor is
visible when the operation is complete
(Section 3.4 of the \*(ZY).
.Ns
The user must be able to see the results of moving the
location cursor or operating on the contents of the
scrollable component.
.Ne
.LI "\f33-48:\f1\0[Required]"
.br
If a mouse-based scrolling action is in progress, the
Cancel key cancels the scrolling action and returns the
scrolling device to its state prior to the start of the
scrolling operation
(Section 3.4 of the \*(ZY).
.Ns
The Cancel key provides a convenient way for the user to
cancel a scrolling operation.
.Ne
.LE
.H 2 "Selection"
.H 3 "Selection Models"
.VL 11 5
.LI "\f34-1:\f1\0[Required]"
.br
The system supports five selection models:
single
selection, browse selection, multiple selection, range
selection and discontiguous selection
(Section 4.1 of the \*(ZY).
.P
Each collection has one or more appropriate selection models.
The model limits the kinds of choices the user can
make in the collection.
Some collections enforce a
selection model, while others allow the user or application
to change it.
.LE
.H 4 "Mouse-Based Single Selection"
.VL 11 5
.LI "\f34-2:\f1\0[Required]"
.br
In a collection that uses single selection, when BSelect is
clicked in a deselected element, the location cursor moves
to that element, that element is selected and any other
selection in the collection is deselected
(Section 4.1.1 of the \*(ZY).
.Ns
Single selection is the simplest selection model, used to
select a single element.
BSelect, the first mouse button,
provides a consistent means of selecting an object within a
group using the mouse.
.Ne
.LE
.H 4 "Mouse-Based Browse Selection"
.VL 11 5
.LI "\f34-3:\f1\0[Required]"
.br
In a collection that uses browse selection, when BSelect is
released in a selectable element, that element is selected
and any other selection in the collection is deselected.
As BSelect is dragged through selectable elements, each
element under the pointer is selected and the previously
selected element is deselected.
The selection remains on
the element where BSelect is released and the location
cursor is moved there
(Section 4.1.2 of the \*(ZY).
.Ns
Browse selection is used to select a single element.
It also allows the user to browse through the collection by
dragging BSelect.
See
.cX mousebasedmultsel .
.Ne
.LI "\f34-4:\f1"
This item of the \*(ZY is not applicable.
.LE
.H 4 "Mouse-Based Multiple Selection"
.xR4 mousebasedmultsel
.VL 11 5
.LI "\f3i:\f1\0[Required]"
.br
If the application contains collections that follow the
multiple selection model, BAdjust is supported and behaves
equivalent to BSelect, when the BTransfer button is currently
configured to behave as BAdjust.
.Ns
On a three-button mouse, button 2 is typically used for the
BTransfer (or BSelect) function.
However, in a \*(Zx
environment, the user may change an environment setting
indicating that mouse button 2 should be used for the
BAdjust function instead.
BAdjust can be used to toggle
the selection state of elements under the multiple
selection model.
.Ne
.LI "\f3j:\f1\0[Required]"
.br
In a collection that uses multiple selection, clicking
BSelect or BAdjust on an unselected element adds that
element to the current selection.
Clicking BSelect or
BAdjust on a selected element removes that element from the
current selection.
Clicking BSelect or BAdjust moves the
location cursor to that element.
.LE
.H 4 "Mouse-Based Range Selection"
.VL 11 5
.LI "\f34-5:\f1\0[Required]"
.br
This item of the \*(ZY has been replaced by items
.B k
and
.BR l .
.LI "\f3k:\f1\0[Required]"
.br
In a collection that uses range selection, pressing BSelect
on an unselected element sets an anchor on the element or
at the position where BSelect was pressed and deselects all
elements in the collection.
If BSelect is released before
the drag threshold has been exceeded, then the element
under the pointer is selected.
If BSelect Motion
exceeds the drag threshold, then a new selection begins.
The anchor and the current position of the pointer
determine the current range.
As BSelect is dragged through
the collection, the current range is highlighted.
When BSelect is released, the anchor does not move and all the
elements within the current range are selected
(Section 4.1.4 of the \*(ZY).
.Ns
Range selection allows the user to select multiple
contiguous elements of a collection by pressing and
dragging BSelect.
.Ne
.LI "\f3l:\f1\0[Required]"
.br
In a collection that uses range selection, pressing BSelect
on an currently selected element causes none of the other
elements in the selection set to be deselected.
If BSelect
is released before the drag threshold is exceeded, then, at
that point, all other elements are deselected and the
element under the pointer remains selected.
If BSelect Motion exceeds the drag threshold, then no element
is deselected and a drag operation begins.
.LI "\f34-6:\f1\0[Required]"
.br
In a text-like collection that uses range selection, the
anchor point is the text pointer position when BSelect is
pressed and the current range consists of all elements
between the anchor point and the current text pointer position
(Section 4.1.4 of the \*(ZY).
.Ns
In text-like collections, elements are ordered linearly and
a text pointer is always considered to be between elements
at a point near the actual pointer position.
.Ne
.LI "\f34-7:\f1\0[Required]"
.br
In a graphics-like or list-like collection that uses a
marquee to indicate the range of a range selection, the
current range consists of those elements that fall
completely within the marquee.
If there is an anchor
element, the marquee is always made large enough to enclose
it completely.
Otherwise, an anchor point is used and is
the point at which BSelect was pressed;
the anchor point
determines one corner of the marquee.
If the collection is
not arranged as a list or matrix, the marquee is extended
to the pointer position.
If the collection is arranged as
a list or matrix, the marquee is either extended to
completely enclose the element under the pointer or
extended to the pointer position.
Clicking BSelect on a
selectable element makes it an anchor element, selects it
and deselects all other elements
(Section 4.1.4 of the \*(ZY).
.Ns
A marquee or highlighted rectangle, is often used to
indicate the range of a selection in graphics-like and
list-like collections.
.Ne
.LI "\f34-8:\f1"
This item of the \*(ZY is not applicable.
.LI "\f3m:\f1\0[Required]"
.br
If the application contains collections that follow the
range selection model, BAdjust is supported and behaves
equivalent to Shift+BSelect, when the BTransfer button is
currently configured to behave as BAdjust.
.Ns
On a three-button mouse, button 2 is typically used for the
BTransfer function.
However, in \*(Zx, the
user may change an environment setting indicating that
mouse button 2 should be used for the BAdjust function instead.
BAdjust can be used to extend the selection set
in the same manner as Shift+BSelect.
.Ne
.LI "\f3n:\f1\0[Required]"
.br
In a collection that uses range selection, when the user
presses Shift+BSelect or BAdjust, the anchor remains
unchanged and an extended range for the selection is
determined, based on one of the extension models:
.VL 11 5
.LI "\f3Reselect\f1\0[Optional]"
.br
The extended range is determined by the anchor and the
current pointer position, in the same manner as
when the selection was initially made.
.LI "\f3Enlarge Only\f1\0[Optional]"
.br
The selection can only be enlarged.
The extended range is
determined by the anchor and the current pointer position,
but then is enlarged to include the current selection.
.LI "\f3Balance Beam\f1\0[Optional]"
.br
A balance point is defined at the midpoint of the current
selection.
When the user presses Shift+BSelect or BAdjust
on the opposite side of the balance point from the anchor,
this model works equivalent to the reselect model.
When the user presses Shift+BSelect, BAdjust or starts a navigation
action modified by Shift on the same side of the balance
point as the anchor, this model moves the anchor to the
opposite end of the selection and then works equivalent to
the reselect model.
.Ns
When the user releases BSelect or BAdjust, the anchor does
not move, all the elements within the extended range are
selected and all the elements outside of it are
deselected.
.Ne
.LE
.LE
.H 4 "Mouse-Based Discontiguous Selection"
.VL 11 5
.LI "\f34-9:\f1\0[Required]"
.br
In a collection that uses discontiguous selection, the
behaviour of BSelect is equivalent to the range selection model.
After the user sets the anchor with
BSelect, Shift+BSelect is equivalent to the range selection model
(Section 4.1.5 of the \*(ZY).
.Ns
Discontiguous selection is an extension of range selection
that allows the user to select multiple discontiguous
ranges of elements.
.Ne
.LI "\f34-10:\f1\0[Required]"
.br
In a collection that uses discontiguous selection, when the
current selection is not empty and the user clicks
Control+BSelect, the anchor and location cursor move to
that point.
If the current selection is not empty and the
user clicks Control+BSelect on an element, the selection
state of that element is toggled and that element becomes
the anchor element
(Section 4.1.5 of the \*(ZY).
.Ns
In discontiguous selection, Control+BSelect Click provides
a convenient means of moving the anchor and toggling the
selection state of the element under the pointer.
.Ne
.LI "\f34-11:\f1\0[Required]"
.br
In a collection that uses discontiguous selection,
Control+BSelect Motion toggles the selection state of a
range of elements.
The range itself is determined as for BSelect Motion.
Releasing ControlBSelect toggles
the selection state of the elements in the range according
to one of two models
(Section 4.1.5 of the \*(ZY):
.VL 11 5
.LI "\f3Anchor Toggle\f1\0[Optional]"
.br
Toggling is based on an anchor element.
If the range is
anchored by a point and is not empty, the anchor element is
set to the element within the range that is nearest to the
anchor point.
Toggling sets the selection state of all
elements in the range to the inverse of the initial state
of the anchor element.
.LI "\f3Full Toggle\f1\0[Optional]"
.br
The selection state of each element in the extended range
is toggled.
.Ns
In discontiguous selection, Control+BSelect provides a
convenient means of toggling the selection state of
elements in a range.
.Ne
.LE
.LI "\f34-12:\f1\0[Required]"
.br
In a collection that uses discontiguous selection, after
Control+BSelect toggles a selection, Shift+BSelect or
Control+Shift+BSelect extends the range of toggled
elements.
The extended range is determined in the
same way as when Shift BSelect is used to extend a range
selection.
When the user releases Control+Shift+BSelect,
the selection state of elements added to the range is
determined by the toggle model in use (either Anchor Toggle
or Full Toggle).
If elements are removed from the range,
they either revert to their state prior to the last use of
Control+BSelect or change to the state opposite that of the
elements remaining within the extended range
(Section 4.1.5 of the \*(ZY).
.Ns
Shift+BSelect and Control+Shift+BSelect provide a
convenient means of extending the range of toggled
elements.
.Ne
.LI "\f3o:\f1\0[Required]"
.br
In a collection that uses discontiguous selection, BAdjust
can be used to extend the range of a discontiguous
selection.
The extended range is determined in the
same way as when BAdjust is used to extend a range
selection.
.Ns
On a three-button mouse, the middle button is typically
used for the BTransfer function.
However, in a \*(Zx
environment, the user may change an environment setting
indicating that the middle button should be used for the
BAdjust function instead.
BAdjust can be used to extend
the selection set in the same manner as Shift+BSelect.
.Ne
.LE
.H 4 "Keyboard Selection"
.VL 11 5
.LI "\f34-13:\f1\0[Required]"
.br
The selection models support keyboard selection modes
according to the following rules
(Section 4.1.6 of the \*(ZY):
.BL
.LI
Single selection supports only add mode.
.LI
Browse selection supports only normal mode.
.LI
Multiple selection supports only add mode.
.LI
Range selection supports normal mode.
If it also supports
add mode, normal mode is the default.
.LI
Discontiguous selection supports both normal mode and add mode.
Normal mode is the default.
.LE
.Ns
Selection must be available from the keyboard.
In normal
mode, used for making simple contiguous selections from the
keyboard, the location cursor is never disjoint from the
current selection.
In add mode, used for making more
complex and possibly disjoint selections, the location
cursor can move independently of the current selection.
.Ne
.LI "\f34-14:\f1\0[Required]"
.br
If a collection supports both normal mode and add mode,
Shift+F8 switches from one mode to the other.
Mouse-based
selection does not change when the keyboard selection mode
changes.
In editable components, add mode is a temporary
mode that is exited when the user performs an operation on
the selection or deselects the selection
(Section 4.1.6 of the \*(ZY).
.Ns
Shift+F8 provides a convenient means of switching between
normal mode and add mode.
.Ne
.LE
.HU "Keyboard-Based Single Selection"
.VL 11 5
.LI "\f34-15:\f1\0[Required]"
.br
In a collection that uses single selection, the navigation
keys move the location cursor independently from the
selected element.
If the user presses the Select key or
the Spacebar on an unselected element, the element with the
location cursor is selected and any other selection in the
collection is deselected
(Section 4.1.6.1 of the \*(ZY).
.Ns
Single selection supports only add mode.
Pressing the
Select key or the Spacebar is similar to clicking BSelect.
.Ne
.LE
.HU "Keyboard-Based Browse Selection"
.VL 11 5
.LI "\f34-16:\f1\0[Required]"
.br
In a collection that uses browse selection, the navigation
keys move the location cursor and select the cursored
element, deselecting any other element.
If the application
has deselected all elements or if the cursor is left
disjoint from the selection, the Select key or the Spacebar
selects the cursored element and deselects any other
element
(Section 4.1.6.2 of the \*(ZY).
.Ns
Browse selection supports only normal mode.
A navigation
operation is similar to dragging BSelect.
.Ne
.LE
.HU "Keyboard-Based Multiple Selection"
.VL 11 5
.LI "\f34-17:\f1\0[Required]"
.br
In a collection that uses multiple selection, the
navigation keys move the location cursor independently from
the current selection.
The Select key or the Spacebar on
an unselected element adds the element to the current
selection.
Pressing the Select key or the Spacebar on a
selected element removes the element from the current
selection
(Section 4.1.6.3 of the \*(ZY).
.Ns
Multiple selection supports only add mode.
Pressing the
Select key or the Spacebar is similar to clicking BSelect.
.Ne
.LE
.HU "Keyboard-Based Range Selection"
.VL 11 5
.LI "\f34-18:\f1\0[Required]"
.br
In a collection that uses range selection and is in normal
mode, the navigation keys move the location cursor and
deselect the current selection.
If the cursor is on an
element, it is selected.
The anchor moves with the
location cursor.
.Ns
Text-like collections can use a different model in which
the navigation keys leave the anchor at its current
location, except that, if the current selection is not
empty, it is deselected and the anchor is moved to the
location of the cursor prior to navigation
(Section 4.1.6.4 of the \*(ZY).
.P
Range selection supports normal mode and, if the collection
also supports add mode, normal mode is the default.
.Ne
.LI "\f34-19:\f1\0[Required]"
.br
In a collection that uses range selection, whether in
normal mode or add mode, the Select key or Spacebar (except
in a text component) moves the anchor to the cursor,
deselects the current selection and, if the cursor is on an
element, selects the element.
Unless the anchor is on a
deselected item, Shift+Select or Shift+Spacebar (except in
text) extends the selection from the anchor to the cursor,
based on the extension model used by Shift+BSelect
(Reselect, Enlarge Only or Balance Beam)
(Section 4.1.6.4 of the \*(ZY).
.Ns
In range selection, pressing the Select key or Spacebar is
similar to clicking BSelect and pressing Shift+Select or
Shift+Spacebar extends the range as with Shift+BSelect.
.Ne
.LI "\f34-20:\f1\0[Required]"
.br
In a collection that uses range selection and is in normal
mode, using Shift in conjunction with the navigation keys
extends the selection, based on the extension model used by
Shift+BSelect.
If the current selection is empty, the
anchor is first moved to the cursor.
The cursor is then
moved according to the navigation keys and the selection is
extended based on the extension model used by Shift+BSelect
(Section 4.1.6.4 of the \*(ZY).
.Ns
In range selection, shifted navigation extends the
selection in a similar manner to dragging Shift+BSelect.
.Ne
.LI "\f34-21:\f1\0[Required]"
.br
In a collection that uses range selection and is in add
mode, the navigation keys move the location cursor,
but leave the anchor unchanged.
Shifted navigation moves the
location cursor according to the navigation keys and the
selection is extended based on the extension model used by
Shift+BSelect
(Section 4.1.6.4 of the \*(ZY).
.Ns
Shifted navigation in add mode is similar to shifted
navigation in normal mode, except that when the selection
is empty the anchor does not move to the cursor prior to
navigation.
.Ne
.LE
.HU "Keyboard-Based Discontiguous Selection"
.VL 11 5
.LI "\f34-22:\f1\0[Required]"
.br
In a collection that uses discontiguous selection and is in
normal mode, all keyboard operations have the same effect
as in the range selection model
(Section 4.1.6.5 of the \*(ZY).
.Ns
Normal mode does not permit multiple discontiguous
selections.
.Ne
.LI "\f34-23:\f1\0[Required]"
.br
In a collection that uses discontiguous selection and is in
add mode, the Select key or Spacebar moves the anchor to
the location cursor and initiates toggling.
If the cursor
is on an element, the selection state of that element is
toggled, but the selection state of all other elements
remains unchanged.
Shift+Select or Shift+Spacebar and
shifted navigation operations extend the selection between
the anchor and the location cursor, based on the toggle
mechanism used by Control+BSelect (Anchor Toggle or Full
Toggle)
(Section 4.1.6.5 of the \*(ZY).
.Ns
Add mode permits use of the keyboard to make multiple
discontiguous selections.
.Ne
.LE
.H 4 "Canceling a Selection"
.VL 11 5
.LI "\f34-24:\f1\0[Required]"
.br
The application uses the Cancel key to cancel or undo any
incomplete motion operation used for selection.
Once the
user presses the Cancel key to cancel a motion operation,
the application ignores subsequent key and button releases
until after all buttons and keys are released.
Pressing
the Cancel key while extending or toggling leaves the
selection state of all elements as they were prior to the
button press
(Section 4.1.7 of the \*(ZY).
.Ns
The Cancel key allows the user to cancel an incomplete
selection operation quickly and consistently.
.Ne
.LE
.H 4 "Autoscrolling and Selection"
.VL 11 5
.LI "\f34-25:\f1\0[Required]"
.br
If the user drags the pointer out of a scrollable
collection during a motion-based selection operation,
autoscrolling is used to scroll the collection in the
direction of the pointer.
If the user presses the Cancel
key with BSelect pressed, the selection operation is
canceled as described in Section 4.1.7
(Section 4.1.8 of the \*(ZY).
.Ns
Autoscrolling provides a convenient means of extending a
selection to elements outside the viewport of a scrollable
collection.
.Ne
.LE
.H 4 "Selecting and Deselecting All Elements"
.VL 11 5
.LI "\f34-26:\f1\0[Required]"
.br
In a collection that uses multiple, range or discontiguous
selection, Control+/ selects all the elements in the
collection, places the anchor at the beginning of the
collection and leaves the location cursor at its previous
position
(Section 4.1.9 of the \*(ZY).
.Ns
Control+/ provides the user with a convenient means of
selecting all of the objects in a collection.
.Ne
.LI "\f34-27:\f1\0[Required]"
.br
In a collection that is in add mode, Control+\e deselects
all the elements in the collection.
In a collection that
is in normal mode, Control+\e deselects all the elements in
the collection, except the element with the location cursor
if the location cursor is being displayed.
In either mode,
Control+\e leaves the location cursor at its current
position and moves the anchor to the location cursor
(Section 4.1.9 of the \*(ZY).
.Ns
Control+\e allows the user to deselect all of the selected
objects quickly and uniformly.
.Ne
.LE
.H 4 "Using Mnemonics for Elements"
.VL 11 5
.LI "\f34-28:\f1\0[Required]"
.br
If the application supports mnemonics associated with
selectable elements, typing a mnemonic while the collection
has the keyboard focus is equivalent to moving the location
cursor to the element and pressing the Select key or
Spacebar
(Section 4.1.10 of the \*(ZY).
.Ns
Mnemonics within a collection of selectable elements
provide an additional selection method.
.Ne
.LE
.H 3 "Selection Actions"
.VL 11 5
.LI "\f34-29:\f1\0[Required]"
.br
When the keyboard focus policy is explicit, the destination
component is the editable component that last had the
keyboard focus.
When the keyboard focus policy is
implicit, the destination component is the editable
component that last received mouse button or keyboard input
(Section 4.2.1 of the \*(ZY).
.Ns
The destination component is used to identify the component
on which certain operations, primarily data transfer
operations, act.
There is only one destination component
at a time.
.Ne
.LI "\f34-30:\f1\0[Required]"
.br
If the keyboard focus is in a component (or a pop-up menu
of a component) that supports selections, operations that
act on a selection act on the selection in that component
(Section 4.2.2 of the \*(ZY).
.Ns
A selection operation acts on the component that has focus,
if that component supports selections.
.Ne
.LI "\f34-31:\f1\0[Required]"
.br
If the keyboard focus is in a component (or a pop-up menu
of a component) that supports some operation that does not
act on a selection, invoking the operation acts on that
component
(Section 4.2.2 of the \*(ZY).
.Ns
An operation that does not act on a selection acts on the
component that has focus, if that component supports the
operation.
.Ne
.LI "\f34-32:\f1\0[Required]"
.br
Inserting or pasting elements into a selection, except for
a primary transfer operation at the bounds of the primary
selection, first deletes the selection if pending delete is
enabled
(Section 4.2.3 of the \*(ZY).
.Ns
Pending delete controls the conditions under which the
selection is deleted.
It is enabled by default.
.Ne
.LI "\f34-33:\f1\0[Required]"
.br
In normal mode, inserting or pasting elements disjoint from
the selection also deselects the selection, except for
primary transfer operations whose source and destination
are in the same collection.
In add mode, the selection is
not deselected
(Section 4.2.3 of the \*(ZY).
.Ns
In add mode, a transfer operation that is disjoint from the
selection does not affect the selection.
.Ne
.LI "\f34-34:\f1\0[Required]"
.br
In editable list-like and graphics-like collections, Delete
deletes the selected elements
(Section 4.2.3 of the \*(ZY).
.Ns
Delete provides a consistent means of deleting the
selection.
.Ne
.LI "\f34-35:\f1\0[Required]"
.br
In editable text-like collections, Delete and Backspace
behave as follows:
.BL
.LI
If the selection is not empty and the control is in normal
mode, the selection is deleted.
.LI
If the selection is not empty, the control is in add mode
and the cursor is not disjoint from the selection, the
selection is deleted.
.LI
If the selection is not empty and the control is in add
mode, but the cursor is disjoint from the selection, Delete
deletes one character forward and Backspace deletes one
character backward.
.LI
If the selection is empty, Delete deletes one character
forward and Backspace deletes one character backward.
.LE
.Ns
In text, Delete and Backspace provide a convenient way to
delete the entire selection or single characters.
.Ne
.LE
.H 3 "Transfer Models"
.VL 11 5
.LI "\f34-36:\f1\0[Required]"
.br
If the move, copy or link operation the user requests is
not available, the transfer operation fails
(Section 4.3 of the \*(ZY).
.Ns
Three transfer operations are generally available:
copy, move and link.
The user requests one of these operations
by pressing the buttons or keys appropriate for the type of
transfer.
In general, for mouse-based operations, the
modifier Control forces a copy, Shift forces a move and
Control+Shift forces a link.
However, any requested
transfer operation must fail if that operation is not
available.
.Ne
.LI "\f34-37:\f1\0[Required]"
.br
If a collection does not have a fixed insertion point or
keep elements ordered in a specific way, the insertion
position for transferred data is determined as follows
(Section 4.3 of the \*(ZY):
.BL
.LI
For BTransfer-based (or BSelect) primary and drag transfer
operations, the insertion position is the position at which
the user releases BTransfer.
.LI
In a text-like collection, the insertion position for other
transfer operations is the location cursor and the data is
pasted before it.
.LI
In a list-like collection, the insertion position for other
transfer operations is the element with the location cursor
and the data is pasted before it.
.LE
.Ns
The insertion position is the position in the destination
where transferred data is placed.
Some mouse-based
transfer operations place data at the pointer position if
possible.
Other operations, including keyboard-based
transfer, generally place the data at the location cursor.
.Ne
.LI "\f3p:\f1\0[Required]"
.br
The application supports the use of mouse button 1 to
perform drag-and-drop operations.
.Ns
In \*(XX Motif, drag and drop is typically performed using
button 2 on a three-button mouse (BTransfer).
However, in \*(Zx, mouse button 1 (BSelect) should be
supported for drag and drop to support mouse usage
compatible with other graphical user interface (GUI)
environments.
That is, in \*(Zx, BTransfer is integrated with BSelect.
A drag can be initiated with either mouse
button 1 or mouse button 2.
.Ne
.LI "\f3q:\f1\0[Required]"
.br
When button 2 of a three-button mouse is configured to
operate as BAdjust, the application does not perform any
BTransfer (or BSelect) operations when clicking mouse
button 2.
.Ns
On a three-button mouse, button 2 is typically used for the
BTransfer (or BSelect) function.
However, in a \*(Zx
environment, the user can change an environment setting
indicating that mouse button 2 should be used for the
BAdjust function instead.
When this is the case, BAdjust
click should not result in the transfer of any data.
.Ne
.LE
.H 4 "Clipboard Transfer"
.VL 11 5
.LI "\f34-38:\f1\0[Required]"
.br
Keyboard-based clipboard selection actions are available in
every editable collection in the application
(Section 4.3.1 of the \*(ZY).
.Ns
Clipboard selection actions must be available from the keyboard.
.Ne
.LI "\f34-39:\f1\0[Required]"
.br
The application uses the Cut key (or Shift+Delete) and the
Cut entry on the Edit menu to cut the selected elements
from an editable component to the clipboard
(Section 4.3.1 of the \*(ZY).
.Ns
The Cut key (or Shift+Delete) and the Cut entry on the Edit
menu offer a consistent means of cutting the selection to
the clipboard from the keyboard.
.Ne
.LI "\f34-40:\f1\0[Required]"
.br
The application uses the Copy key (or Control+Insert) and
the Copy entry on the Edit menu to copy the selected
elements to the clipboard
(Section 4.3.1 of the \*(ZY).
.Ns
The Copy key or (Control+Insert) and the Copy entry on the
Edit menu offer a consistent means of copying the selection
to the clipboard from the keyboard.
.Ne
.LI "\f34-41:\f1\0[Required]"
.br
The application uses the Paste key (or Shift+Insert) to
paste the contents of the clipboard into an editable
component
(Section 4.3.1 of the \*(ZY).
.Ns
The Paste key (or Shift+Insert) offers a consistent way of
pasting the contents of the clipboard from the keyboard.
.Ne
.LI "\f34-42:\f1\0[Required]"
.br
If Paste or Paste Link is invoked using a component's
pop-up menu, the data is pasted at the insertion position
of the component.
However, if the pop-up menu is popped up
over a selection, the selection is first deleted, even if
pending delete is disabled and the pasted data replaces it,
if possible
(Section 4.3.1 of the \*(ZY).
.Ns
Popping up a pop-up menu over a selection indicates that a
Paste or Paste Link operation should replace the
selection.
.Ne
.LI "\f34-43:\f1\0[Required]"
.br
If Paste or Paste Link is invoked from the Edit menu or by
a keyboard operation and the insertion position in the
target component is not disjoint from a selection, the
pasted data replaces the selection contents if pending
delete is enabled
(Section 4.3.1 of the \*(ZY).
.Ns
Pending delete determines whether the selection is deleted
when the insertion position is not disjoint from the
selection and Paste or Paste Link is invoked from the Edit
menu or by a keyboard operation.
.Ne
.LE
.H 4 "Primary Transfer"
.VL 11 5
.LI "\f34-44:\f1\0[Required]"
.br
In an editable collection, BTransfer Click,
Control+BTransfer Click, Alt, Copy and Control+Alt+Insert
copy the primary selection to the insertion position, as
defined in Section 4.3 of the \*(ZY.
(Note that the insertion position
is usually different for mouse and keyboard operations.)
(Section 4.3.2 of the \*(ZY)
.Ns
These operations provide a convenient way for the user to
force a copy operation.
.Ne
.LI "\f34-45:\f1\0[Required]"
.br
In an editable collection, Shift+BTransfer Click, Alt+Cut
and Alt+Shift+Delete move the primary selection to the
insertion position, as defined in Section 4.3 of the \*(ZY.
(Note that
the insertion position is usually different for mouse and
keyboard operations.)
(Section 4.3.2 of the \*(ZY)
.Ns
These operations provide a convenient way for the user to
force a move operation.
.Ne
.LI "\f34-46:\f1\0[Required]"
.br
In an editable collection, Control+Shift+BTransfer Click
places a link to the primary selection at the insertion
position, as defined in Section 4.3
(Section 4.3.2 of the \*(ZY).
.Ns
Control+Shift+BTransfer provides a convenient way for the
user to force a link operation.
.Ne
.LI "\f34-47:\f1\0[Required]"
.br
A Primary Move moves the primary selection as well as the
elements selected;
that is, the element moved to the
destination becomes selected as the primary selection.
Primary Copy and Primary Link do not select transferred
data at the destination
(Section 4.3.2 of the \*(ZY).
.Ns
This requirement provides the expected treatment of the selection
in a move, copy and link operation.
.Ne
.LE
.H 4 "Quick Transfer"
.VL 11 5
.LI "\f34-48:\f1\0[Required]"
.br
All text components support quick transfer
(Section 4.3.3 of the \*(ZY).
.Ns
Quick transfer is used to make a temporary selection and
then immediately move, copy or link that selection to the
insertion position of the destination component.
In text, quick transfer provides a convenient way to move, copy or
link text without disturbing the primary selection.
.Ne
.LI "\f34-49:\f1\0[Required]"
.br
If a component supports quick transfer, Alt+BTransfer
Motion or Control+Alt+BTransfer Motion temporarily selects
elements in the specified range and, on release, copies
them to the insertion position of the destination component
(Section 4.3.3 of the \*(ZY).
.Ns
These operations provide a convenient way to perform a quick copy.
.Ne
.LI "\f34-50:\f1\0[Required]"
.br
If a component supports quick transfer, Alt+Shift+BTransfer
Motion temporarily selects elements in the specified range
and, on release, moves them to the insertion position of
the destination component
(Section 4.3.3 of the \*(ZY).
.Ns
This operation provides a convenient way to perform a quick cut.
.Ne
.LI "\f34-51:\f1\0[Required]"
.br
If a component supports quick transfer,
Control+Alt+Shift+BTransfer Motion temporarily selects
elements in the specified range and, on release, places a
link to them at the insertion position of the destination
component
(Section 4.3.3 of the \*(ZY).
.Ns
This operation provides a convenient way to perform a quick link.
.Ne
.LI "\f34-52:\f1\0[Required]"
.br
Quick transfer does not disturb the primary selection or
affect the clipboard, except when the destination of the
transfer is within or on the boundaries of the primary
selection and pending delete is enabled.
In this case,
quick transfer deletes the contents of the primary
selection, leaving an empty primary selection, before
pasting the transferred elements
(Section 4.3.3 of the \*(ZY).
.Ns
Quick transfer is a secondary selection mechanism, so it
cannot disrupt the primary selection.
When the destination
of the transfer is in the primary selection, quick transfer
replaces the primary selection with the secondary
selection.
.Ne
.LI "\f34-53:\f1\0[Required]"
.br
With quick transfer, the range of the temporary selection
is determined by using the same model as when BSelect
Motion determines the range of a primary selection
(Section 4.3.3 of the \*(ZY).
.Ns
This requirement provides consistency between primary selection
and quick transfer operations.
.Ne
.LI "\f34-54:\f1\0[Required]"
.br
If the user drags the pointer out of a scrollable
collection while making the temporary selection,
autoscrolling is used to scroll the collection in the
direction of the pointer.
If the user releases BTransfer
with the pointer outside of the collection or if the user
presses the Cancel key with BTransfer pressed, the
highlighting is removed and a transfer is not performed
(Section 4.3.3 of the \*(ZY).
.Ns
Autoscrolling provides a convenient means of extending a
temporary selection to elements outside the viewport of a
scrollable collection.
.Ne
.LE
.H 4 "Drag Transfer"
.VL 11 5
.LI "\f34-55:\f1\0[Required]"
.br
In a collection that supports selection, Shift+BTransfer
Release forces a drag move operation.
If a move is not
possible, the operation fails
(Section 4.3.4 of the \*(ZY).
.Ns
This mechanism offers a convenient way for the user to
force a move operation.
.Ne
.LI "\f34-56:\f1\0[Required]"
.br
In a collection that supports selection, Control+BTransfer
Release forces a drag copy operation.
If a copy is not
possible, the operation fails
(Section 4.3.4 of the \*(ZY).
.Ns
This mechanism offers a convenient way for the user to
force a copy operation.
.Ne
.LI "\f34-57:\f1\0[Required]"
.br
In a collection that supports selection,
Control+Shift+BTransfer Release forces a drag link
operation.
If a link is not possible, the operation fails
(Section 4.3.4 of the \*(ZY).
.Ns
This mechanism offers a convenient way for the user to
force a link operation.
.Ne
.LI "\f34-58:\f1\0[Required]"
.br
When a drag move operation moves a selection within the
same component, the selection moves along with the elements
selected
(Section 4.3.4 of the \*(ZY).
.Ns
In other words, when selected elements are moved with a
drag operation, they should stay selected after the move.
This mechanism offers a convenient way to move the
selection within a component.
.Ne
.LI "\f34-59:\f1\0[Required]"
.br
In text-like collections, initiating a drag within a
selected region drags the entire text selection
(Sections 4.3.4 and 4.3.5 of the \*(ZY).
.Ns
To be consistent, drag-and-drop actions must operate on
the entire selection.
.Ne
.LI "\f34-60:\f1\0[Required]"
.br
In list-like and graphics-like collections, initiating a
drag on a selected element drags the entire selection
(Sections 4.3.4 and 4.3.5 of the \*(ZY).
.Ns
To be consistent, drag-and-drop actions must operate on
the entire selection.
.Ne
.LI "\f34-61:\f1\0[Required]"
.br
In list-like and graphics-like collections, initiating a
drag with BTransfer on an unselected element drags just
that element and leaves the selection unaffected
(Section 4.3.4 of the \*(ZY).
.Ns
Unselected elements can be dragged without affecting the
selection.
.Ne
.LI "\f34-62:\f1\0[Required]"
.br
When a drag is initiated in an unselected region and the
pointer is over two possible draggable elements, the drag
uses the draggable element highest in the stacking order
(Section 4.3.4 of the \*(ZY).
.Ns
This requirement ensures the consistency of drag operations.
.Ne
.LI "\f34-63:\f1\0[Required]"
.br
When the application starts a drag operation, the pointer
is replaced with a drag icon
(Section 4.3.4.1 of the \*(ZY).
.Ns
A drag icon provides visual feedback that a drag operation
is in progress.
.Ne
.LI "\f34-64:\f1\0[Required]"
.br
All drag icons used by the application include a source
indicator
(Section 4.3.4.1 of the \*(ZY).
.Ns
A source indicator gives a visual representation of the
elements being dragged.
.Ne
.LI "\f34-65:\f1\0[Required]"
.br
Pressing the Cancel key ends a drag-and-drop operation by
canceling the drag in progress
(Section 4.3.4.2 of the \*(ZY).
.Ns
The Cancel key provides a consistent way for the user to
cancel a drag operation.
.Ne
.LI "\f34-66:\f1\0[Required]"
.br
Releasing BTransfer ends a drag-and-drop operation
(4.3.4.3 of the \*(ZY).
.Ns
Releasing BTransfer offers a consistent means of ending a
drag operation.
.Ne
.LI "\f34-67:\f1\0[Required]"
.br
When BTransfer (or BSelect) is released, the drop operation
ordinarily occurs at the location of the hotspot of the
drag icon pointer and into the highest drop zone in the
stacking order.
However, if a drop occurs within a
selection and pending delete is enabled, the transferred
data replaces the contents of the entire selection
(Section 4.3.4.3 of the \*(ZY).
.Ns
This requirement provides consistency in the treatment of
mouse-based transfer operations.
.Ne
.LI "\f34-68:\f1\0[Required]"
.br
After a successful transfer, the data is placed in the drop
zone and any transfer icon used by the application is
removed
(Section 4.3.4.4 of the \*(ZY).
.Ns
A transfer icon can be used to represent the type of data
being transferred during a drop operation.
A successful
drop operation results in the transfer of data.
.Ne
.LI "\f34-69:\f1\0[Required]"
.br
After a failed transfer, the data remains at the drag
source and is not placed in the drop zone.
Any transfer
icon used by the application is removed
(Section 4.3.4.4 of the \*(ZY).
.Ns
A failed drop operation does not result in the transfer of data.
.Ne
.LI "\f3r:\f1\0[Recommended]"
.br
In a collection that supports selection, if BTransfer
Motion (or BSelect Motion) results in the start of a drag
operation, feedback should be presented to the user that
indicates that a copy, move or link operation is in
progress.
Whether the operation is a copy, move or link
depends on the type of object created at the drop zone and
whether the source object is removed.
.Ns
Although an unmodified drag typically results in a move
operation, depending on the location of the source object
and the target drop zone, the drag may in fact result in a
copy or link operation.
For example, dragging an icon
representing an attachment to a mail message typically
results in a copy of the attachment being created as
opposed to the original being removed from the mail
message.
Any feedback presented should incorporate use of
a drag icon that portrays the source object being
manipulated.
.Ne
.LI "\f3s:\f1\0[Recommended]"
.br
In a collection that supports selection, if
Control+BTransfer Motion results in the start of a drag
operation, feedback should be presented to the user that
indicates that a copy operation is in progress.
.Ns
The feedback presented should incorporate use of a drag
icon that portrays the source object being copied.
.Ne
.LI "\f3t:\f1\0[Recommended]"
.br
In a collection that supports selection, if
Control+Shift+BTransfer Motion results in the start of a
drag operation, feedback should be presented to the user
that indicates that a link operation is in progress.
.Ns
The feedback presented should incorporate use of a drag
icon that portrays the source object being linked.
.Ne
.LI "\f3u:\f1\0[Recommended]"
.br
In a collection that supports copy, move or link operations
that can be performed by dragging, the feedback presented
to the user during the drag operation should indicate
whether a single object or multiple objects are being
manipulated.
.Ns
Feedback provided during the drag operation should ensure
that the user feels confident that the desired set of
objects is being dragged.
The drag icon used for
multi-object drag operations should integrate the feedback
used to indicate whether the operation is a move, copy or link.
.Ne
.LI "\f3v:\f1\0[Optional]"
.br
If the application allows the user to paste data into its
data pane, it allows the user to drag and drop files from
the file manager into the data pane.
.Ns
The user should be able to drag and drop files into
application data panes.
The result should be the inclusion
of some element of the file or the display of an error
message indicating that the file selected cannot be
incorporated into the application's data.
Drag transfers
that are accepted can result in a number of different
responses from the application:
.AL
.LI
The icon image for the
file might be inserted at the drop point
.LI
The application might perform some activity using the data
contained within the file as its input
.LI
The data contained within the file might be inserted at the drop point
.LI
The name of the file might be inserted at the drop point
.LE
.Ne
.LE
.H 2 "Component Activation"
.H 3 "Basic Activation"
.VL 11 5
.LI "\f35-1:\f1\0[Required]"
.br
The application uses BSelect to activate a button
(Section 5.1 of the \*(ZY).
.Ns
BSelect, the first mouse button, provides a consistent
means of activating a button using the mouse.
.Ne
.LI "\f35-2:\f1\0[Required]"
.br
When a button has the focus, the application uses the
Select key or Spacebar to activate the button
(Section 5.1 of the \*(ZY).
.Ns
The Select key and Spacebar provide a uniform way of
selecting a button.
Selecting a button is equivalent to
activating the button.
.Ne
.LI "\f35-3:\f1\0[Required]"
.br
When an activatable menu entry has the focus, the
application uses the Select, Spacebar, Enter or Return key
to activate the entry
(Section 5.1 of the \*(ZY).
.Ns
The Select, Spacebar, Enter and Return keys offer a
consistent means of activating a menu entry using the
keyboard.
.Ne
.LI "\f35-4:\f1\0[Required]"
.br
When BSelect is pressed over a button, the appearance of
the button changes to indicate that releasing BSelect will
activate the button.
If, while BSelect is pressed, the
pointer is moved outside of the button, the visual state is
restored.
If, while BSelect is still pressed, the pointer
is moved back inside of the button, the visual state is
again changed to indicate the pending activation.
If BSelect is pressed and released within a button, the button
is activated, regardless of whether the pointer has moved
out of the button while it was pressed
(Section 5.1 of the \*(ZY).
.Ns
The visual state of a button offers a cue to the user about
whether or not the button will be activated when the mouse
button is released.
.Ne
.LI "\f35-5:\f1\0[Required]"
.br
If a selectable element of a collection is activatable,
BSelect Click, the Select key and Spacebar (except in text)
select it.
BSelect Click 2 selects and activates it
(Section 5.1 of the \*(ZY).
.Ns
This requirement provides for consistent integration of activation
and selection in a collection where elements can be both
selected and activated.
.Ne
.LI "\f3w:\f1\0[Required]"
.br
The time allowed to detect a double click
(*doubleClickTime: 500) is no less than 500 milliseconds.
.LE
.H 3 "Accelerators"
.VL 11 5
.LI "\f35-6:\f1\0[Required]"
.br
If the application uses accelerators, the component with
the accelerator displays the accelerator key or key
combination following the label of the component
(Section 5.2 of the \*(ZY).
.Ns
An accelerator is a key or key combination that invokes the
action of some component regardless of the position of the
location cursor when the accelerator is pressed.
So that
the user knows that there is an accelerator associated with
a component, the accelerator must be displayed.
.Ne
.LI "\f35-7:\f1\0[Required]"
.br
If a button with an accelerator is within a primary or
secondary window or within a pull-down menu system from its
menu bar, it is activatable whenever the input focus is in
the window or the menu bar system.
If a button with an
accelerator is within a pop-up menu system, it is
activatable whenever the focus is in the pop-up menu system
or the component with the pop-up menu
(Section 5.2 of the \*(ZY).
.Ns
An accelerator must be able to be activated from the window or
component associated with the accelerator.
.Ne
.LE
.H 3 "Mnemonics"
.VL 11 5
.LI "\f35-8:\f1\0[Required]"
.br
If the application uses mnemonics, the label for the
component with the mnemonic contains the character that is
its mnemonic.
If the label does not naturally contain the
character, the mnemonic is placed in parentheses following
the label
(Section 5.3 of the \*(ZY).
.Ns
A mnemonic is a single character that can be associated
with any component that contains a text label.
Mnemonics
provide a fast way of selecting a component from the
keyboard.
To let the user know that there is a mnemonic
associated with a selection, the mnemonic is underlined in
the label of the selection by the toolkit.
For a mnemonic
to be underlined, the label for a selection must
contain the mnemonic character.
If the label does not
contain the mnemonic, putting the mnemonic in parentheses
following the label provides visual consistency.
.Ne
.LI "\f3x:\f1\0[Required]"
.br
Mnemonic characters are chosen for ease-of-location
within the text of a label.
Wherever possible, the first character of the label is used.
If that is not possible,
the last character of the label or, if there is more
than one word, the first character of the second word is used.
.LI "\f35-9:\f1\0[Required]"
.br
All mnemonics are case insensitive for activation
(Section 5.3 of the \*(ZY).
.Ns
The user must be able to activate a mnemonic by pressing
either the lowercase or the uppercase variant of the
mnemonic key.
.Ne
.LI "\f35-10:\f1\0[Required]"
.br
When the location cursor is within a menu or a menu bar,
pressing the mnemonic key of a component within that menu
or menu bar moves the location cursor to the component and
activates it.
If a mnemonic is used for an option button
or for a cascade button in a menu bar, pressing Alt and the
mnemonic anywhere in the window or its menus moves the
cursor to the component with that mnemonic and activates it
(Section 5.3 of the \*(ZY).
.Ns
A mnemonic is generally activatable when the location
cursor is within the component that contains the mnemonic.
Pressing Alt and the mnemonic provides a way to activate a
visible mnemonic when the location cursor is within the
window that contains the mnemonic.
.Ne
.LE
.H 3 "Tear-Off Activation"
.VL 11 5
.LI "\f35-11:\f1\0[Required]"
.br
Activating a tear-off button tears off the menu that
contains the button
(Section 5.4 of the \*(ZY).
.Ns
A tear-off button is similar to a push button with the special
interaction of tearing off the menu from its cascade button.
Tear-off buttons use the same basic activation as other buttons.
.Ne
.LI "\f35-12:\f1\0[Required]"
.br
When a menu with a tear-off button is posted, pressing
BTransfer in the tear-off button starts a tear-off action.
As long as BTransfer is held, a representation of the menu
follows the movement of the pointer.
Releasing BTransfer
ends the tear-off action by unposting the menu system,
creating a new window at the current pointer location that
contains the contents of the menu and giving focus to the
new window in explicit pointer mode
(Section 5.4 of the \*(ZY).
.LE
.H 3 "Help Activation"
.VL 11 5
.LI "\f35-13:\f1\0[Required]"
.br
The application uses the Help key on a component to invoke
any context-sensitive help for the component or its nearest
ancestor with context-sensitive help available
(Section 5.5 of the \*(ZY).
.Ns
The Help key offers the user a consistent mechanism for
invoking context-sensitive help.
.Ne
.LI "\f3y:\f1\0[Required]"
.br
The application provides context-sensitive help at all
locations.
.Ns
The user must never get a ``help not available'' message.
.Ne
.LE
.H 3 "Default Activation"
.VL 11 5
.LI "\f35-14:\f1\0[Required]"
.br
If the application uses default push buttons in a window,
the current default push button is highlighted.
When the
focus is on a push button, its action is the default action
and the push button shows default highlighting.
If the
default action in a window varies, some push button always
has default highlighting, except when there is no current
default action
(Section 5.6 of the \*(ZY).
.Ns
Placing emphasis on the default push button in a dialog box
provides the user with a visual cue about the expected
reply to the dialog box.
.Ne
.LI "\f35-15:\f1\0[Required]"
.br
When focus is in a window with a default action and an
activatable menu does not have the focus, the Enter key and
Control+Return invoke the default action.
If focus is in a
component other than multi-line text or an activated menu,
Return also invokes the default action.
These actions have
no other effect on the component with the focus, unless the
default action has some effect on that component
(Section 5.6 of the \*(ZY).
.Ns
These requirements ensure that the means of invoking a default
action are consistent across applications.
.Ne
.LI "\f35-16:\f1\0[Required]"
.br
Except in the middle of a button motion operation, pressing
the Cancel key anywhere in a dialog box is equivalent to
activating the Cancel push button in the dialog box
(Section 5.6 of the \*(ZY).
.Ns
The Cancel key provides a uniform means of canceling dialog
box from the keyboard.
.Ne
.LE
.H 3 "Expert Activation"
.VL 11 5
.LI "\f35-17:\f1\0[Required]"
.br
If the application supports expert activation, expert
actions exist only as shortcuts to application features
that are available through another mechanism
(Section 5.7 of the \*(ZY).
.Ns
Expert activation, using mouse double-clicking on buttons,
provides a convenient way for experienced users to perform
certain tasks quickly.
However, new users and
keyboard-only users must be able to perform the same tasks.
.Ne
.LI "\f35-18:\f1\0[Required]"
.br
When the focus is on a button used for expert activation,
no default action is available, unless the default and
expert actions are the same
(Section 5.7 of the \*(ZY).
.Ns
This requirement minimises possible confusion between default and
expert activation.
.Ne
.LI "\f35-19:\f1\0[Required]"
.br
If a component with an expert action is selectable,
activating the expert action first selects the component
and then performs the expert action
(Section 5.7 of the \*(ZY).
.Ns
A user must be able to select a component, even if it
has an expert action associated with it.
.Ne
.LE
.H 3 "Previewing and Autorepeat"
.VL 11 5
.LI "\f35-20:\f1\0[Required]"
.br
If the application supports activation preview using
BSelect, the previewing information is removed when the
user releases BSelect
(Section 5.8 of the \*(ZY).
.Ns
Activation preview presents the user with additional
information that describes the effect of activating a
button.
This information cannot interfere with the normal
operation of the application.
.Ne
.LE
.H 3 "Cancel Activation"
.VL 11 5
.LI "\f35-21:\f1\0[Required]"
.br
Pressing the Cancel key stops current interaction in the
following contexts
(Section 5.9 of the \*(ZY):
.BL
.LI
During a mouse-based selection or drag operation, it
cancels the operation.
.LI
During a mouse-based scrolling operation, it cancels the
scrolling action and returns the system to its state prior
to the start of the scrolling operation.
.LI
Anywhere in a dialog box that has a Cancel push button, it
is equivalent to activating that push button, except during
a mouse-based selection or drag operation.
.LI
In a pull-down menu, it either dismisses the menu and moves
the location cursor to the cascade button used to pull it
down or unposts the entire menu system.
In a pop-up menu,
option menu, tear-off menu or menu bar, it unposts the menu
system.
.LI
When the focus is in a torn off menu window, it closes the
torn off menu window.
.LE
.Ns
These requirements for the Cancel key ensure the consistent
operation of the key across applications.
.Ne
.LE
.H 2 "Window Management"
.H 3 "Window Support"
This section corresponds to section 7.2 of the \*(ZY.
The different window types are discussed throughout the \*(ZY and
this chapter.
In particular, see
.cX appldesign .
.VL 11 5
.LI "\f3z:\f1\0[Required]"
.br
Application windows are clearly distinguishable as
primary or secondary windows based on appearance and behaviour.
.P
Primary Window:
.BL
.LI
Primary window decoration (see
.cX windowdec )
.LI
Primary window management (see
.cX winmgmtactions )
.LI
Window stacking, workspace placement and minimisation can
be independent of other primary windows
.LE
.P
Secondary Window:
.BL
.LI
Secondary window decoration (see
.cX windowdec )
.LI
Secondary window management (see
.cX winmgmtactions )
.LI
Window stacking, workspace placement and minimisation tied
to the associated primary window
.LE
.LE
.H 3 "Window Decorations"
.xR3 windowdec
.VL 11 5
.LI "\f3aa:\f1\0[Required]"
.br
Windows that support particular window management
functionality must request the corresponding window
decoration (for example, a window that can be minimised
must request the minimise button).
.LI "\f3ab:\f1\0[Required]"
.br
Windows that support any window management functionality
(move, resize, minimise, maximise, close and others)
have a window menu with items for that functionality.
.LI "\f3ac:\f1\0[Required]"
.br
The application follows \*(Zx window decoration conventions, as shown in
the following table.
.br
.ne 10
.TS
center, box;
cf3 ssssss
l cf3 cf3 cf3 cf3 cf3 cf3
l c c c c c c .
\*(Zx Window Decoration Conventions
_
	Border	Title	Menu	Min	Max	Resize
_
Primary Window:
\0\0\0\0Default	Yes	Yes	Yes	Yes	Yes\u\s-21\s+2\d	Yes\u\s-21\s+2\d
\0\0\0\0Front Panel	Yes\u\s-22\s+2\d	No	Yes\u\s-22\s+2\d	Yes	No	No
.sp 0.5v
Secondary Window:
\0\0\0\0Default	Yes	Yes	Yes	No	No\u\s-23\s+2\d	No\u\s-23\s+2\d
\0\0\0\0Front Panel	No	Yes	Yes	No	No	No
.TE
.AL
.LI
Decorations for resize and maximise are provided for
primary windows, if appropriate.
.LI
The Front Panel has custom visuals for the window decorations.
.LI
Secondary windows are designed such that resizing and
maximisation are not necessary or appropriate.
If a secondary window must be resizable and maximisable, the
associated decorations are displayed.
.LE
.LI "\f3ad:\f1\0[Required]"
.br
The application follows \*(Zx window menu conventions.
Items appear in the window menu if they are applicable to the window or
its minimised window icon.
.BL
.LI
Restore (\s+6\z_\s-6R)
.LI
Move (\s+6\z_\s-6M)
.LI
Size (\s+6\z_\s-6S)
.LI
Minimize (\s+6\z_\s-6n)
.LI
Maximize (\s+6\z_\s-6x)
.LI
Lower (\s+6\z_\s-6L)
.LI
Occupy Workspace .\|.\|.
(\s+6\z_\s-6O)
.LI
Occupy All Workspaces (\s+6\z_\s-6A)
.LI
Unoccupy Workspace (\s+6\z_\s-6U)
.LI
Close (\s+6\z_\s-6C) Alt+F4
.LE
.LI "\f3ae:\f1\0[Optional]"
.br
The application should not add items to the window menu.
If an extraordinary requirement has an application add items
to the window menu, the items should be appended to the end
of the menu with a separator between Close and the
application items.
.LI "\f3af:\f1\0[Optional]"
.br
Accelerators, other than Alt+F4 for Close, should not be
used in the window menu.
.Ns
This minimises conflict with other
uses of the Alt key for application accelerators,
localisation and others.
.Ne
.LE
.H 3 "Window Navigation"
This section corresponds to section 7.4 of \*(ZY.
There are no checklist items for application developers.
.H 3 "Icons"
.VL 11 5
.LI "\f3ag:\f1\0[Optional]"
.br
The application should provide unique window icons for their
primary windows.
The window icon image should have a
similar appearance to the associated file or front panel
icon image.
.LI "\f3ah:\f1\0[Optional]"
.br
The window icon label should contain the same text as the
title of the corresponding primary window or an abbreviated
form of it.
See
.cX layout
for window title guidelines.
.LI "\f3ai:\f1\0[Optional]"
.br
The window icon image should have a similar appearance to
the associated file or Front Panel icon image.
.LE
.H 3 "Application Window Management"
.H 4 "Window Placement"
.VL 11 5
.LI "\f3aj:\f1\0[Recommended]"
.br
The application should not require or force windows or window
icons to be positioned at a particular screen location.
.LI "\f3ak:\f1\0[Recommended]"
.br
The application should place a secondary window relative to
the associated primary window.
It should be placed close
to, but not obscuring, the component that caused it to be
displayed and the information that is necessary to interact
with the dialog box.
.LI "\f3al:\f1\0[Optional]"
.br
Some suggestions are given in section 6.2.4.3,
``Determining Dialog Box Location and Size,'' of the \*(ZY.
Additional or modified recommendations include:
.P
If a dialog box does not relate to specific items in the
underlying window, it should be placed below the menu bar
(if there is one) and centered (horizontally) over the work area.
.LI "\f3am:\f1\0[Recommended]"
.br
If a secondary window is allowed to be stacked below its
associated primary window (not constrained to stay on top
of the primary window), it should be placed such that it is
not completely covered by the primary window.
This recommendation takes precedence over other placement
recommendations.
.Ns
.mc 6
This recommendation is included to accommodate certain
legacy applications and is applicable only when the
user has selected this behaviour.
Newly written \*(Zx applications must not rely on
this type of window placement.
.Ne
.mc
.LI "\f3an:\f1\0[Recommended]"
.br
If a menu or dialog box is already on display, reinvoking
the command that caused it to be displayed should
automatically bring that window or menu to the front of the
window stack without changing its position on the screen.
.LE
.H 4 "Window (Document) Clustering"
.VL 11 5
.LI "\f3ao:\f1\0[Optional]"
.br
Windows that are closely related in supporting a particular
task should be placed in a window cluster.
Secondary
windows are automatically placed in a window cluster with
the associated primary window.
Windows in a window cluster
are stacked together, minimised or normalised together and
kept in the same workspace.
.Ns
Currently the only mechanism for forming a window cluster
that is supported by the window manager is to indicate a
primary-secondary relationship.
.Ne
.LE
.H 4 "Window Management Actions"
.xR4 winmgmtactions
.VL 11 5
.LI "\f3ap:\f1\0[Required]"
.br
Windows should follow \*(Zx window management functionality
conventions, as shown in
the following table.
.br
.ne 10
.TS
center, box;
cf3 ssssss
l cf3 cf3 cf3 cf3 cf3 cf3
l c c c c c c .
\*(Zx Window Management Conventions
_
	Close	Move	Lower	Min	Max	Resize
_
Primary Window:
\0\0\0\0Default	Yes	Yes	Yes	Yes	Yes\u\s-21\s+2\d	Yes\u\s-21\s+2\d
\0\0\0\0Front Panel	No	Yes	Yes	Yes	No	No
.sp 0.5v
Secondary Window:
\0\0\0\0Default	Yes	Yes	Yes	No	No\u\s-22\s+2\d	No\u\s-22\s+2\d
\0\0\0\0Front Panel	Yes	Yes	Yes	No	No	No
.TE
.AL
.LI
Resize and maximise
functionality should be provided for primary windows if
appropriate.
.LI
.mc 6
Secondary windows can contain the Maximize and Resize
window manager functions, if appropriate.
.LE
.mc
.LI "\f3aq:\f1\0[Required]"
.br
Windows that support particular window management
functionality should request corresponding window
decoration (for example, a window that can be minimised
should request the minimise button).
.LI "\f3ar:\f1\0[Required]"
.br
Windows that have form factor constraints set
window manager hints for minimum size, maximum size, aspect
ratio and resize increment as appropriate.
.br
.mc 6
.LI "\f3as:\f1\0[Recommended]"
.br
.mc
Maximising a window should show more content (objects or
controls) if appropriate (as opposed to scaling up the
sizes of objects and controls).
.LI "\f3at:\f1\0[Required]"
.br
Windows that have Close or Exit functionality
support the window management protocol for Close if there
is a window menu.
In the case of dialog boxes, the Close
item on the window menu corresponds to the Cancel
functionality or dialog box dismissal with no further
action taken.
.LI "\f3au:\f1\0[Recommended]"
.br
When the application creates a new window, it should come
up in the user's current workspace and only occupy that
single workspace.
.LI "\f3av:\f1\0[Recommended]"
.br
Application windows that are related to a particular task
should move together between workspaces.
.LE
.H 3 " Session Management Support"
.VL 11 5
.LI "\f3aw:\f1\0[Required]"
.br
The application should support Interclient Communications
Conventions Manual (ICCCM\(emsee the \*(Zi) mechanisms for session
management of their primary windows and key properties.
.LI "\f3ax:\f1\0[Required]"
.br
The application should support ICCCM mechanisms for session
management of all associated windows (that is, secondary
windows that may include help windows).
.LI "\f3ay:\f1\0[Optional]"
.br
The application should accept messages from the \*(Zx Session
Manager that inform it the user is logging out and should
save the application state at that time.
.LI "\f3az:\f1\0[Optional]"
.br
An application that has a single primary window that is open
at the time the user logs out should restore the primary
window, in the workspace last occupied, when the user logs in again.
.LI "\f3ba:\f1\0[Optional]"
.br
Save user context wherever possible.
For example,
an application that supports the editing of files should save
the state of the file at logout and should restore the file
in the application window when users log in again.
.LI "\f3bb:\f1\0[Optional]"
.br
An application that has multiple primary windows that are
open at the time the user logs out should restore all
primary windows, in their respective workspaces, when the
user logs in again.
.LE
.H 2 "Application Design Principles"
.xR2 appldesign
.H 3 "Layout"
.xR3 layout
.H 4 "Main Window"
.VL 11 5
.LI "\f36-1:\f1\0[Required]"
.br
The application should be composed of at least one main
window
(Section 6.2.1.1 of the \*(ZY).
A main window contains a client area and, optionally, a
menu bar, a command area, a message area and scroll bars.
The client area contains the framework of the application.
.Ns
The use of a main window ensures interapplication consistency.
.Ne
.LI "\f3bc:\f1\0[Required]"
.br
The default size of the application's main window is
large enough to accommodate a typical amount of data, but
does not fill the entire physical display size
(and thus minimises visual conflicts with other applications).
.Ns
Each application potentially must share the display with
other applications.
The default window size should not
take up all the available screen space.
.Ne
.LI "\f3bd:\f1\0[Required]"
.br
Resize corners should be included in any main window that
incorporates a scrolling data pane or list.
.Ns
Any changes to the overall size of the window should result in a
corresponding increase or decrease in the size of the
scrollable portion.
Additionally, the application might
reorganise elements within the window based on the
increased or decreased amount of space (for example, it
might reorganise a row of buttons into two rows).
.Ne
.LI "\f36-2:\f1\0[Required]"
.br
If the application has multiple main windows that serve the
same primary function, each window closes and iconifies
separately
(Section 6.2.1.1 of the \*(ZY).
.Ns
For example, a text editor might allow the user to edit
multiple documents, each in its own main window.
Each window is then treated as a separate application and can be
closed or iconified when it is not being used.
.Ne
.LI "\f36-3:\f1\0[Required]"
.br
If the application has multiple main windows that serve
different primary functions, each window should be able to
iconify independently of the other windows
(Section 6.2.1.1 of the \*(ZY).
.Ns
For example, a debugger might provide separate main windows
for editing source code, examining data values and viewing
results.
Each window can be iconified when it is not being
used, but it is up to the application to decide whether
each window closes separately or whether closing one window
closes the entire application.
.Ne
.LE
.H 4 "Window Titles"
.VL 11 5
.LI "\f3be:\f1\0[Optional]"
.br
The title of the primary window (the main window the
application displays to the user) should be the name of the
application.
.Ns
The title does not have to be the actual name of the
executable invoked by the user.
.P
The application developer
should carefully consider how the title chosen for
the primary window works when it is used in icons and
pop-up windows.
If the name of the pop-up window is too
long, the application may remove the application title, but
without the title, users might have difficulty
telling which pop-up windows belong with the originating
primary window.
.Ne
.LI "\f3bf:\f1\0[Optional]"
.br
The application should use initial capital letters for each word in the title (in
languages that support capitalisation) .
.LI "\f3bg:\f1\0[Optional]"
.br
The application should
follow the application name for each property window, as a
.br
minimum, with the title Properties and the name of the
object it affects.
.LI "\f3bh:\f1\0[Optional]"
.br
The title of each pop-up window should begin with the application
title followed by a colon, then the title of the pop-up window.
The colon should have a space both before and
after it for readability.
.LI "\f3bi:\f1\0[Optional]"
.br
When the application has files that can be loaded or saved,
the application should use a hyphen to denote the current file name.
The hyphen should have a space before and after it.
Only the base name of the file should be displayed, not the entire pathname.
.Ns
The hyphen is used to denote specific instances of a window
or data.
The colon serves to delimit general categories or
commands.
For example, a file manager might have the
following title for a Properties dialog box:
.Cs I
File Manager : Properties - myfile
.Ce
.Ne
.LI "\f3bj:\f1\0[Optional]"
.br
The application should
follow the application name for each command window with
the same title that is on the window button or window item
users choose to display that window.
.LI "\f3bk:\f1\0[Optional]"
.br
In the case of multiple primary windows, the
application name should be included at the beginning of each window title,
with an added name that uniquely identifies that primary window.
No separator should be provided for these names (for
example,
Calendar Manager Multi-Browse, Catalogue Search,
Admintool Databases).
.LI "\f3bl:\f1\0[Optional]"
.br
Pop-up windows should indicate which primary window
they are associated with (which primary window invoked that pop-up).
.LI "\f3bm:\f1\0[Optional]"
.br
An abbreviated name for the application may be used on
other windows, so long as it is done on all windows.
.LE
.H 4 "Menu Bar"
The following requirements apply only in a left-to-right
language environment in an English-language locale.
The application must make the appropriate changes for other locales.
.VL 11 5
.LI "\f36-4:\f1\0[Required]"
.br
If the application has a menu bar, it is a horizontal bar
at the top edge of the application, just below the title
.mc 6
area of the window frame
(Section 6.2.1.5 of the \*(ZY).
.mc
.Ns
A menu bar organises the most common features of an
application.
It contains a list of menu topics in cascade
buttons;
each button is associated with a distinct
pull-down menu containing commands that are grouped by
common functionality.
The use of a menu bar yields
consistency across applications.
.Ne
.LI "\f36-5:\f1\0[Required]"
.br
The menu bar for the application contains only cascade
buttons
(Section 6.2.1.5 of the \*(ZY).
.Ns
When other buttons are included as topics in a menu bar,
they inhibit menu browsing.
.Ne
.LI "\f36-6:\f1"
This item of the \*(ZY is not applicable.
It is replaced by the following recommendation.
.LI "\f3bn:\f1\0[Recommended]"
.br
If any of the standard menus are present in the menu bar,
they should be arranged in one of the following orders with
respect to each other:
.Cs I
menubar1: File Edit View Options Help
menubar2: <app-label> <object-type> Edit View Options Help
.Ce
.P
The application should exclude from the menu bar any item
shown in the preceding text if the application does not
support the associated function.
For example, if the
application does not support the ability to display its
data in different views, then the application should not
include a View menu.
.P
An application that allows the user to load and save files
should use menubar1 and augment this with additional menus
as necessary.
Menubar1 is the traditional menu bar style
used by many file- (document-) oriented applications.
An application that is not file-oriented in nature (or that
manages files transparently, not exposing this activity to
the user) should use menubar2.
For example, system
management applications, games and workflow automation
applications might fall into this category.
Like the File
menu in menubar1, the <app-label> menu may contain commands
that are global to the application and must contain Exit as
the last item.
The actual string used for <app-label>
should represent the global function or service that the
application provides.
If the application is solely used
for creating and managing objects of a particular type and
does not provide functionality separate from this, then the
application might choose to omit the <app-label> menu and
therefore have the <object-type> menu as the first menu.
In any event, Exit should be the last item of the first menu.
.P
An <object-type> menu should be used in applications that
display and manage multiple objects of a specific type.
For example, a Print Manager might contain a Printer menu,
a mail application might contain a Message menu, a User
Account Manager might have an Account menu and so on.
An application that manages multiple object-types might have
several <object-type> menus.
In this scenario, however, it
is preferred that there be a single <app-label> menu that
allows the user to create new instances of the different
object types.
The application would then provide a single
Selected menu in place of the multiple <object-type>
menus.
The Selected menu should be context-sensitive,
containing commands related to the type of object currently
selected by the user.
.LE
.H 4 "File Menu Contents"
The following requirements apply only in a left-to-right
language environment in an English-language locale.
The application must make the appropriate changes for other locales.
.VL 11 5
.LI "\f3bo:\f1\0[Required]"
.br
If the user chooses Exit or in any other manner indicates
that the application should be terminated, but there are
changes to the current file that have not been saved, the
application displays a dialog box asking whether the
changes should be saved before exiting.
.Ns
The user must always be given the opportunity to explicitly
state whether unsaved changes should be saved or
discarded.
A dialog box similar to the one described
should also be displayed if the user chooses the Open File
menu item, but has not saved changes to the current file.
.Ne
.LI "\f36-7:\f1\0[Required]"
.br
If the application uses a File menu, it contains the
following choices, with the specified functionality, when
the actions are actually supported by the application
(Section 6.2.1.5.1 of the \*(ZY).
.VL 11 5
.LI "\f3\s+6\z_\s-6New\f1\0[Required]"
.br
This choice creates a new file.
If the current client area will be
used to display the new file, the application clears the
existing data from the client area.
If changes made to the
current file will be lost, the application displays a
dialog box, asking the user about saving changes.
The mnemonic is N.
.LI "\f3\s+6\z_\s-6Open.\|.\|.\f1\0[Required]"
.br
This choice opens an existing file by prompting the user for a file
name with a dialog box.
If changes made to the current
file will be lost, the application displays a dialog box
asking the user about saving changes.
The is mnemonic is O.
.LI "\f3\s+6\z_\s-6Save\f1\0[Required]"
.br
This choice saves the currently opened file without removing the
existing contents of the client area.
If the file has no
name, the application displays a dialog box, prompting the
user to enter a file name.
The mnemonic is S.
.LI "\f3Save \s+6\z_\s-6As.\|.\|.\f1\0[Required]"
.br
This choice saves the currently opened file under a new name by
prompting the user for a file name with a dialog box.
If the user tries to save the file using an existing name, the
application displays a dialog box that warns the user about
a possible loss of data.
This choice does not remove the existing
contents of the client area.
The mnemonic is A.
.LI "\f3\s+6\z_\s-6Print\f1\0[Recommended]"
.br
This choice schedules a file for printing.
If the application needs
specific information to print, it should display a dialog
box requesting the information from the user.
In this
case, the menu entry is followed by an ellipsis
(such as ``Print.\|.\|.'').
The mnemonic is P.
.br
.mc 6
.LI "\f3\s+6\z_\s-6Close\f1\0[Recommended]"
.br
.mc
This choice closes the current primary window and its associated
secondary windows.
If the application uses only a single
primary window or multiple dependent primary windows, this
action is not supplied.
The mnemonic is C.
.br
.mc 6
.LI "\f3E\s+6\z_\s-6xit\f1\0[Required]"
.br
.mc
This choice ends the current application and all windows associated
with it.
If changes made to the current file will be lost,
the application displays a dialog box, asking the user
about saving changes.
The mnemonic is X.
.LE
.Ns
The use of a File menu with these common file operations
yields consistency across applications.
.Ne
.LE
.HU "<Object-Type> / Selected Menu Contents"
.VL 11 5
.LI "\f3bp:\f1\0[Recommended]"
.br
If the application uses an <object-type> menu or a Selected
menu, it should contain the following choices, with the
specified functionality, when the actions are actually
supported by the application.
Items should be presented to
the user in the order listed below:
.P
The <object-type> menu contains controls that allow the
user to create instances of the object-type.
Both the
<object-type> and Selected menus allow the user to
manipulate object instances.
Additional items should be
added to the <object-type> or Selected menus if they relate
solely to the manipulation of objects managed by the
application (as opposed to more generic services that the
application might provide).
.VL 11 5
.LI "\f3New.\|.\|.\f1\0[Recommended]"
.br
This choice creates a new instance of the object-type.
.mc 6
If appropriate, a dialog box
should be presented allowing the user to specify the values
.mc
for settings associated with that object.
.LI "\f3Move To.\|.\|.\f1\0[Optional]"
.br
This choice allows the user to move the selected objects into a
folder.
A FileSelection dialog box is displayed allowing
the user to select the desired folder.
.LI "\f3Copy To.\|.\|.\f1\0[Optional]"
.br
This choice allows the user to copy the selected objects into a
folder.
A FileSelection dialog box is displayed allowing
the user to select the desired folder.
.LI "\f3Put in Workspace\f1\0[Optional]"
.br
This choice allows the user to put a link for the object onto the \*(Zx
desktop in the current workspace.
.LE
.P
Any of the preceding three menu choices should be provided
only if the objects managed by the application are able to
reside as separate entities outside of the application's
main window.
For example, a printer object created by a
printer management application might be able to be placed
in a Folder window and function as an application unto itself.
The application should also support drag-and-drop
as a method for performing any of these actions.
.VL 11 5
.LI "\f3Delete\f1\0[Optional]"
.br
This choice removes the selected objects.
A confirmation dialog box
should be presented to the user before the object is
actually deleted.
.LI "\f3Properties\f1\0[Recommended]"
.br
This choice displays a Properties window that should show the current
values for settings associated with the selected object.
.LE
.mc 6
.LI "\f3<Default Action>\f1\0[Recommended]"
.br
This choice should enact the default action for the selected object.
``Open'' is a typical default.
.LE
.mc
.HU "Edit Menu Contents"
The following requirements apply only in a left-to-right
language environment in an English-language locale.
The application must make the appropriate changes for other locales.
.VL 11 5
.LI "\f36-8:\f1\0[Required]"
.br
If the application uses an Edit menu, it contains the
following choices, with the specified functionality, when
the actions are actually supported by the application
(Section 6.2.1.5.2 of the \*(ZY):
.VL 11 5
.LI "\f3\s+6\z_\s-6Undo\f1\0[Optional]"
.br
This choice reverses the most recently executed action.
The mnemonic is U.
.LI "\f3Cu\s+2\z_\s-2t\f1\0[Optional]"
.br
This choice removes the selected portion of data from the client area
and puts it on the clipboard.
The mnemonic is T.
.LI "\f3\s+6\z_\s-6Copy\f1\0[Optional]"
.br
This choice copies the selected portion of data from the client area
and puts it on the clipboard.
The mnemonic is C.
.LI "\f3Copy Lin\s+6\z_\s-6k\f1\0[Optional]"
.br
This choice copies a link of the selected portion of data from the
client area and puts it on the clipboard.
The mnemonic is K.
.LI "\f3\s+6\z_\s-6Paste\f1\0[Optional]"
.br
This choice pastes the contents of the clipboard into the client area.
The mnemonic is P.
.LI "\f3Paste \s+6\z_\s-6Link\f1\0[Optional]"
.br
This choice pastes a link of the data represented by the contents of
the clipboard into the client area.
The mnemonic is L.
.LI "\f3Cl\s+2\z_\s-2ear\f1\0[Optional]"
.br
This choice removes a selected portion of data from the client area
without copying it to the clipboard and does not compress
the remaining data.
The mnemonic is E.
.LI "\f3\s+6\z_\s-6Delete\f1\0[Optional]"
.br
This choice removes a selected portion of data from the client area
without copying it to the clipboard.
The mnemonic is D.
.LI "\f3Select All \f1\0[Optional]"
.br
This choice sets the primary selection to be all the elements in a
component of the client area.
.LI "\f3Deselect All\f1\0[Optional]"
.br
This choice removes from the primary selection all the elements in a
component of the client area.
.LI "\f3Select Pasted\f1\0[Optional]"
.br
This choice sets the primary selection to the last element or elements
pasted into a component of the client area.
.LI "\f3Reselect\f1\0[Optional]"
.br
This choice sets the primary selection to the last selected element or
elements in a component of the client area.
This action is
available only in components that do not support persistent
selections and only when the current selection is empty.
.LI "\f3Promote\f1\0[Optional]"
.br
This choice promotes to the primary selection the current selection of
a component of the client area.
This action is available
only for components that support persistent selections.
.LE
.Ns
The use of an Edit menu with these common editing
operations yields consistency across applications.
.Ne
.LI "\f3bq:\f1\0[Recommended]"
.br
If the application does not provide an <object-type> or
Selected menu, but allows the user to select data within
the window and manage settings for the selected data, then
it should provide a ``Properties .\|.\|.''
choice as the last item in the Edit menu.
.LI "\f36-9:\f1"
This item of the \*(ZY is not applicable.
.LE
.HU "View Menu"
.VL 11 5
.LI "\f3br:\f1\0[Recommended]"
.br
If the application provides a View menu, it should only
contain functions that affect the way the current data is
presented.
It should not contain any option that alters
the data itself.
.LE
.HU "Options Menu"
.VL 11 5
.LI "\f3bs:\f1\0[Recommended]"
.br
If the application has global settings that control the way
the application behaves, it should provide an Options menu
from which these can be set.
.LE
.H 4 "Help Menu Contents"
The following requirements apply only in a left-to-right
language environment in an English-language locale.
The application must make the appropriate changes for other locales.
.VL 11 5
.LI "\f3bt:\f1\0[Recommended]"
.br
If the application includes a Help menu, it should contain
the following set of choices, with the specified
functionality, when the actions are actually supported by
the application.
.mc 6
The Help choices included here supersede
those listed for Motif 1.2.
.VL 11 5
.mc
.LI "\f3Over\s+6\z_\s-6view \f1\0[Required]"
.br
This choice provides general information about the window from which
help was accessed or about the application overall.
The mnemonic is V.
A separator is placed after this choice.
.LI "\f3\s+2\z_\s-2Index\f1\0[Optional]"
.br
This choice provides an index listing topics for all help information
available for the application.
The mnemonic is I.
.LI "\f3Table of \s+6\z_\s-6Contents\f1\0[Recommended]"
.br
This choice provides a table of contents listing topics for all help
information available for the application.
The mnemonic is C.
.LI "\f3\s+6\z_\s-6Tasks\f1\0[Recommended]"
.br
This choice provides access to help information indicating how to
perform different tasks using the application.
The mnemonic is T.
.LI "\f3\s+6\z_\s-6Reference\f1\0[Recommended]"
.br
This choice provides access to reference information.
The mnemonic is R.
.LI "\f3Tutoria\s+2\z_\s-2l \f1\0[Optional]"
.br
This choice provides access to the application's tutorial.
The mnemonic is L.
.LI "\f3\s+6\z_\s-6Keyboard\f1\0[Optional]"
.br
This choice provides information about the application's use of
function keys, mnemonics and keyboard accelerators.
It also provides information on general \*(Zx use of such keys.
The mnemonic is K.
.LI "\f3\s+6\z_\s-6Mouse\f1\0[Optional]"
.br
This choice provides information about using a mouse with the
application.The mnemonic is M.
.LI "\f3\s+6\z_\s-6Mouse and Keyboard\f1\0[Optional]"
.br
This choice provides information about the application's use of
function keys, mnemonics, keyboard accelerators and using a
mouse with the application.
It also provides information on general \*(Zx use of such keys.
The mnemonic is M.
This choice should be used
instead of separate mouse and keyboard choices if this
information is best presented together.
.LI "\f3\s+6\z_\s-6On Item\f1\0[Recommended]"
.br
Initiates context-sensitive help by changing the shape of
the pointer to the question mark pointer.
When the user
moves the pointer to a component and presses BSelect, any
available context-sensitive help for the component is
presented.
The mnemonic is O.
This choice is set off with separators on
both sides.
.LI "\f3\s+6\z_\s-6Using Help\f1\0[Required]"
.br
This choice provides information on how to use the \*(Zx Help
Facility.
The mnemonic is U.
This choice is set off with separators on both sides.
.LI "\f3\s+6\z_\s-6About applicationname\f1\0[Required]"
.br
Displays a dialog box indicating, minimally, the name and
version of the application and displaying its icon or some
other signature graphic for the application.
The mnemonic is A.
.LE
.LI "\f36-10:\f1"
This item of the \*(ZY is not applicable.
It is replaced by item
.BR bt .
.LE
.H 4 "Pop-up Menus"
The following requirements apply only in a left-to-right
language environment in an English-language locale.
The application must make the appropriate changes for other locales.
.VL 11 5
.LI "\f3bu:\f1\0[Recommended]"
.br
If the application provides functions that apply to a data
pane and not any specific element therein, then a pop-up
menu should be provided that contains the frequently used
data pane functions and is accessible by pressing BMenu
when the mouse pointer is over the background of the pane
or a non-selectable element within the pane.
.LI "\f3bv:\f1\0[Recommended]"
.br
The application should provide a pop-up menu for any
element that is selectable within its data pane.
.Ns
Pop-up menus provide access to frequently used functions
and should be used pervasively throughout the \*(Zx desktop
environment.
A pop-up menu may contain a collection of
options that appear in different menus available from the
menu bar.
For example, it may contain items from both the
File and Edit menus.
.Ne
.LI "\f3bw:\f1\0[Recommended]"
.br
When a pop-up menu is displayed over an unselected object,
any action selected from the pop up applies to that object
only and not to any other objects that might currently be
selected.
.Ns
This helps to protect users from inadvertently applying an
action to objects that they might not realise are currently selected.
Pressing the menu button invokes a pop-up menu
pertinent to the object under the mouse cursor whether it
is selected or not or, if the object under the mouse cursor
and other objects are selected, the pop up is pertinent to
the selected set.
.Ne
.LI "\f3bx:\f1\0[Recommended]"
.br
Every pop-up menu in the application should have a title
that indicates the function the menu performs or the
element on which it operates.
.LI "\f3by:\f1\0[Recommended]"
.br
The functions accessible from within the application's
pop-up menus should also be accessible from buttons
displayed within the window or menus accessed through the
menu bar.
.Ns
Because pop-up menus are hidden, they should only provide
redundant access to functions available from more visible
controls within the application's windows.
.Ne
.mc 6
.LI "\f3bz:\f1\0[Recommended]"
.br
If the application uses an Attachment menu, it should contain the
following choices, with the specified functionality, when
.mc
the actions are actually supported by the application.
.VL 11 5
.mc 6
.LI "\f3Add File.\|.\|.\f1\0[Recommended]"
.br
This choice should select files and other items to be attached.
A file
selection box is displayed allowing the user to select the
.mc
desired files to attach.
The default button in the file
selection box is Attach.
.br
.mc 6
.LI "\f3Save As.\|.\|.\f1\0[Recommended]"
.br
This choice should save the currently selected attachments.
The user is
prompted with a FileSelection Box for indicating where in
.mc
the file system the attachments will be saved.
When multiple attachments are selected, the name field is
inactive and the current names of the attachments are used
as the name of the new file.
This menu item is active only
when one or more attachments are selected.
.LI "\f3Rename.\|.\|.\f1\0[Recommended]"
.br
This choice renames the attachment icon.
The application should
provide in-line renaming of attachment icons such as File
Manager uses.
If the application cannot provide in-line
renaming, then Rename allows the user to rename an
attachment by displaying a dialog box, requesting the name
from the user.
This menu item is active only when a single
attachment is selected.
It is not active when multiple
attachments are selected.
.LI "\f3Delete\f1\0[Recommended]"
.br
This choice deletes attachments from the attachment list.
This menu
item is active only when an attachment is selected.
.LI "\f3Select All\f1\0[Recommended]"
.br
This choice selects all the attachments in the attachment list.
.LI "\f3Actions\f1\0[Recommended]"
.br
This choice lists all the available actions on the selected
attachment.
The menu item should be inactive when no
attachment or multiple attachments are selected.
.LE
.LI "\f36-11:\f1\0[Optional]"
.br
If the application uses any of the common pop-up menu
actions, the actions function according to the following
specifications
(Section 6.2.1.6 of the \*(ZY).
See item
.B ca
for supplemental guidelines.
.VL 11 5
.LI "\f3Properties\f1\0[Optional]"
.br
This choice displays a properties dialog box that the user can use to
set the properties of the component.
.LI "\f3Undo\f1\0[Optional]"
.br
This choice reverses the most recently executed action.
.LI "\f3Primary Move\f1\0[Optional]"
.br
This choice moves the contents of the primary selection to the
component.
This action is available only in editable
components.
.LI "\f3Primary Copy\f1\0[Optional]"
.br
This choice copies the contents of the primary selection to the
component.
This action is available only in editable
components.
.LI "\f3Primary Link\f1\0[Optional]"
.br
This choice places a link to the primary selection in the component.
This action is available only in editable components.
.LI "\f3Cut\f1\0[Optional]"
.br
This choice cuts elements to the clipboard.
If the menu is popped up
in a selection, cuts the entire selection to the
clipboard.
.LI "\f3Copy\f1\0[Optional]"
.br
This choice copies elements to the clipboard.
If the menu is popped up
in a selection, this action copies the entire selection to
the clipboard.
.LI "\f3Copy Link\f1\0[Optional]"
.br
This choice copies a link of elements to the clipboard.
If the menu is
popped up in a selection, copies a link to the entire
selection to the clipboard.
.LI "\f3Paste\f1\0[Optional]"
.br
This choice pastes the contents of the clipboard to the component.
This action is available only in editable components.
.LI "\f3Paste Link\f1\0[Optional]"
.br
This choice pastes a link of the contents of the clipboard to the
component.
This action is available only in editable
components.
.LI "\f3Clear\f1\0[Optional]"
.br
This choice removes a selected portion of data from the client area
without copying it to the clipboard.
If the menu is popped
up in a selection, deletes the selection.
.LI "\f3Delete\f1\0[Optional]"
.br
This choice removes a selected portion of data from the client area
without copying it to the clipboard.
If the menu is popped
up in a selection, deletes the selection.
.LI "\f3Select All\f1\0[Optional]"
.br
This choice sets the primary selection to be all of the elements in the
collection with the pop-up menu.
.LI "\f3Deselect All\f1\0[Optional]"
.br
This choice deselects the current selection in the collection with the
pop-up menu.
.LI "\f3Select Pasted\f1\0[Optional]"
.br
This choice sets the primary selection to be the last element or
elements pasted into the collection with the pop-up menu.
.LI "\f3Reselect\f1\0[Optional]"
.br
This choice sets the primary selection to be the last selected element
or elements in the component with the pop-up menu.
This action is available only in components that do not support
persistent selections and only when the current selection
is empty.
.LI "\f3Promote\f1\0[Optional]"
.br
This choice promotes the current selection to the primary selection.
It is available only in components that support persistent
selections.
.LE
.Ns
The use of pop-up menus with these common actions yields
consistency across applications.
.Ne
.LI "\f3ca:\f1\0[Recommended]"
.br
Pop-up menus for selectable objects should contain the
following set of choices, with the specified functionality,
when the actions are actually supported by the
application.
These guidelines supplement item 6-11:.
.VL 11 5
.LI "\f3Move To .\|.\|.\f1\0[Optional]"
.br
This choice allows the user to move the selected objects into a
folder.
A FileSelection dialog box is displayed allowing
the user to select the desired folder.
.LI "\f3Copy To .\|.\|.\f1\0[Optional]"
.br
This choice allows the user to copy the selected objects into a
folder.
A FileSelection dialog box is displayed allowing
the user to select the desired folder.
.br
.mc 6
.LI "\f3Put on Workspace\f1\0[Optional]"
.br
.mc
This choice allows the user to put a link for the selected objects onto
the \*(Zx desktop in the current workspace.
.LI "\f3Delete\f1\0[Optional]"
.br
This choice deletes the selected object.
A confirmation is displayed
to the user before actually removing the object.
.LI "\f3Properties .\|.\|.\f1\0[Recommended]"
.br
This choice displays a dialog box indicating the current settings for
attributes associated with the selected object.
.LI "\f3Help .\|.\|.\f1\0[Recommended]"
.br
This choice displays a help window pertaining to objects of the type
selected.
.LE
.LI "\f3cb:\f1\0[Optional]"
.br
Choices within the pop-up menus are organised in the
following manner:
.Cs
<choices that manage the object, such as open, save or properties>
.sp 0.5v
----------- separator ----------------
.sp 0.5v
<standard edit menu choices, such as cut, copy or paste>
.sp 0.5v
----------- separator ----------------
.sp 0.5v
<other choices>
.Ce
.LI "\f36-12:\f1\0[Required]"
.br
When a pop-up menu is popped up in the context of a
selection, any action that acts on elements acts on the
entire selection
(Section 6.2.1.6 of the \*(ZY).
.Ns
In the context of a selection, pop-up menu actions affect
the entire selection.
.Ne
.LE
.H 4 "Dialog Boxes"
.VL 11 5
.LI "\f36-13:\f1\0[Required]"
.br
Information dialog boxes do not interrupt the user's
interaction with the application
(Section 6.2.1.7.5 of the \*(ZY).
.Ns
An information dialog box conveys information to the user
that does not require immediate attention, so it does not
need to be modal.
.Ne
.LE
.H 4 "Menu Design"
.VL 11 5
.LI "\f3cc:\f1\0[Recommended]"
.br
Menus accessed from within the application should contain
at least two menu items.
.Ns
No menu should contain only one item.
If the application
provides a menu with only one item, the application should
move that item into another menu or make it a
button within the window.
.Ne
.LI "\f3cd:\f1\0[Optional]"
.br
Submenus accessed from within the application contain at
least three menu items.
.Ns
Submenus may be used to group like items into a single
secondary cascaded menu where putting the items into the
primary cascaded menu would make it too long.
However, if the submenu contains only two options, the application
should remove the secondary cascaded
menu and put the options into the primary cascaded menu
since it takes more effort for the user to access options
located in a submenu.
.Ne
.LI "\f3ce:\f1\0[Recommended]"
.br
No menu in the application should contain more than 15
choices.
.Ns
The longer the menu the more effort is needed for the user
to access choices near the bottom.
If the menu has a lot
of choices, the application should break it up
into two or more menus or group some items into submenus.
.Ne
.LI "\f3cf:\f1\0[Optional]"
.br
If the application contains a menu that is expected to be
accessed frequently, then a tear-off menu option is
provided in that menu.
.Ns
The user should be able to tear-off frequently accessed
menus so that these can remain posted on the desktop as the
user uses the application.
.Ne
.LI "\f3cg:\f1\0[Optional]"
.br
The application provides keyboard accelerators where appropriate.
If specific menu items within a menu are expected to be used
frequently, not the menu as a whole, then the application
provides keyboard accelerators for these items and displays
the keyboard accelerators in the associated menu to the
right of the item to which they relate.
.LI "\f3ch:\f1\0[Recommended]"
.br
The labels used for items in the menu bar should not appear
as options within the menus themselves.
.Ns
The names of items in the menu bar serve as titles for the
options the menu contains.
The name of the menu bar item
should provide a term that accurately describes the concept
of the category relating all of the menu items and should
not be used as the name of any item within the menu
itself.
.Ne
.LI "\f3ci:\f1\0[Required]"
.br
Any menu choice that is not currently an appropriate
selection is dimmed (insensitive).
.Ns
Dimmed controls cannot be activated by the user and should
appear only when the inactive state is short-term (that is,
there is something the user can do within the application
or the desktop environment to make the control become
active).
When the control is persistently inactive
(because of the current configuration of the application or
system or a particular set of companion software is not
currently installed), the control should be removed rather
than dimmed.
.Ne
.LI "\f36-14:\f1\0[Required]"
.br
If the application uses a tear-off button in a menu, the
tear-off button is the first element in the menu
(Section 6.2.3 of the \*(ZY).
.Ns
When a tear-off button is activated, the menu changes into
a dialog box.
The tear-off button must be the first
item in the menu so that the entire contents of the menu
are torn off.
.Ne
.mc 6
.LI "\f36-15:\f1\0[Required]"
.br
All menus must be wide enough to accommodate their widest
elements
(Section 6.2.3 of the \*(ZY).
.mc
.Ns
The ability to see the full label of each menu element
allows the user to browse through a menu.
.Ne
.LE
.H 4 "Dialog Box Design"
The following requirements apply only in a left-to-right
language environment in an English-language locale.
The application must make the appropriate changes for other locales.
.VL 11 5
.LI "\f3cj:\f1\0[Recommended]"
.br
The title of dialog boxes used within the application
should adhere to the conventions listed in
the following table.
.br
.ne 10
.TS
center, box;
cf3 s
lf3 | lf3
l   | l .
Dialog Box Title Conventions
_
Window Usage	Window Title Format
_
Message	<app or object name> \- <action or situation>
Progress	<app or object name> \- <action> in Progress
Action (Command)	<app name> \- <action>
Object Properties	<app name> \- <object-type> Properties
Application Options	<app name> \- <type> Options
.TE
.LI "\f3ck:\f1\0[Required]"
.br
Every dialog box in the application has at least one button
that either performs the dialog box action and dismisses it
or dismisses the dialog box without taking any action.
.LI "\f36-16:\f1\0[Optional]"
.br
This item of the \*(ZY has been replaced by item
.BR cl .
.LI "\f3cl:\f1\0[Recommended]"
.br
If the application uses common dialog box actions, the
actions should have the following specified functionality
and labels:
.VL 11 5
.LI "\f3Yes\f1\0[Optional]"
.br
This label indicates an affirmative response to a question posed in
the dialog box.
.LI "\f3No\f1\0[Optional]"
.br
This label indicates a negative response to a question posed in the
dialog box.
.LI "\f3OK\f1\0[Optional]"
.br
This label applies any changes made to components in the dialog box
and dismisses the dialog box.
.LI "\f3<command>\f1\0[Optional]"
.br
This label applies any changes made to components in the dialog box,
performs the action associated with <command> and dismisses
the dialog box.
.P
This label should be used in lieu of OK, Yes or No as a button label
when it provides more meaning to the user as to the action
that will be performed when that button is clicked.
.LI "\f3Apply\f1\0[Optional]"
.br
This label applies any changes made to components in the dialog box
and does not dismiss it.
.LI "\f3Retry\f1\0[Optional]"
.br
This label causes the task in progress to be attempted again.
.LI "\f3Stop\f1\0[Optional]"
.br
This label ends the task in progress at the next possible break
point.
.LI "\f3Pause\f1\0[Optional]"
.br
This label causes the task in progress to pause.
.LI "\f3Resume\f1\0[Optional]"
.br
This label causes a task that has paused to resume.
.LI "\f3Save As Defaults\f1\0[Optional]"
.br
This label saves the current settings as the default settings that
will appear the next time the window is displayed.
The settings are not applied to any selected object and the
dialog box is not dismissed.
.P
A Save As Defaults button should be provided if it is
expected that a user would want to use different default
values for a set of controls within a dialog box than those
that the application provide as the factory settings.
For example, a Save As Defaults button might be provided in a
``New <object-type>'' window, allowing the user to indicate
that whenever a new instance of that object-type is
created, the current values should be displayed as the
default settings instead of the values given by the
application.
.LI "\f3Reset\f1\0[Optional]"
.br
This label cancels any changes that have not yet been applied by the
application.
The controls within the dialog box are reset
to their state since the last time the dialog box action
was applied.
If no changes have been applied within the
current invocation of the dialog box, the controls are
reset to this state when the dialog box was first
displayed.
.LI "\f3Reset to Factory\f1\0[Optional]"
.br
This label cancels any changes that have not yet been applied.
Components in the dialog box are reset to their default
state and value as specified by the vendor that delivered
the application (that is, the controls are restored to the
original factory settings).
.LI "\f3Cancel\f1\0[Optional]"
.br
This label dismisses the dialog box without performing any actions not
yet applied.
.LI "\f3Help\f1\0[Recommended]"
.br
This choice provides help for the dialog box.
.LE
.mc 6
.LI "\f3cm:\f1\0[Recommended]"
.br
Any visible control that is not currently active or whose
setting is currently invalid should be dimmed.
.Ns
.mc
Dimmed controls cannot be activated by the user and should
appear only when the inactive state is short-term (that is,
there is something the user can do within the application
or the desktop environment to make the control become
active).
When the control is persistently inactive
(because of the current configuration of the application or
system or a particular set of companion software is not
currently installed), the control should be removed rather
than dimmed.
.Ne
.LI "\f3cn:\f1\0[Optional]"
.br
The application should keep the size of the dialog boxes to a minimum.
.Ns
On low-resolution displays, dialogs may take up most
of the screen real estate and may run off the edge of
the screen if not designed correctly.
.Ne
.LI "\f3co:\f1\0[Optional]"
.br
The application should avoid complexity in the dialog boxes.
If the dialog box
must support many functions, it should use an expandable
dialog box (see
.cX expandablewindows )
or use more than one dialog in a nested fashion.
.LI "\f3cp:\f1\0[Optional]"
.br
The application should avoid the use of resize handles in the dialog box.
However, the application can use resize handles when
resizing is useful in allowing users to see more
information; for example, when the dialog contains a
scrolling list that is likely to be long and users
will frequently need to search the list.
.LI "\f3cq:\f1\0[Optional]"
.br
Every dialog box in the application has exactly one default
button that is activated when the Enter key is pressed.
.P
The default button should be associated with the most
likely response from the user and should not be potentially
destructive or irreversible.
Some applications may have
dialog boxes that do not reveal a default button until a
specific set of fields has been filled out or otherwise
manipulated.
.LI "\f3cr:\f1\0[Optional]"
.br
If a dialog box displayed by the application has controls
that are considered to be advanced features, it hides these
controls in a separate options page and provides a
.mc 6
<category>
option button that allows a user to navigate to this page.
.P
.mc
Controls that relate to advanced features should not be
displayed with the set of options initially displayed to
the user.
The typical user should be presented with only
those options that are necessary to use the basic
functionality of the application.
Users wanting to access
advanced functionality within the dialog box can use the
.mc 6
<category> option button.
If the number of advanced controls
is very few or the settings for these controls are highly
.mc
related to the settings of basic controls displayed in the
dialog box (that is, the settings of the advanced controls
change when the user changes settings for basic controls),
the application might choose to provide a Show Other
Options checkbox that reveals the set of advanced controls.
.LE
.HU "Property Windows"
.VL 11 5
.LI "\f3cs:\f1\0[Required]"
.br
If the application provides settings that control the
behaviour of the application, these settings are displayed
in an application properties window that is accessible from
an Options menu.
.LI "\f3ct:\f1\0[Recommended]"
.br
If the application manages objects and allows the user to
see or modify settings for these objects, these settings
should be displayed in an object properties window that is
accessible from a ``Properties .\|.\|.''
choice in the Edit,
<object-type> or Selected menus, as well as from the pop-up
menu associated with the object.
.LI "\f3cu:\f1\0[Recommended]"
.br
If the application provides access to a Properties or
Options window, this window should include the following
set of buttons in the order listed, with the specified
functionality, when supported by the application.
.VL 11 5
.LI "\f3OK\f1\0[Required]"
.br
This button applies any changes made to components in the dialog box
and dismisses it.
OK may be replaced by a more appropriate label;
for example, Add.
The alternative label should be a verb phrase.
.LI "\f3Apply\f1\0[Optional]"
.br
This button applies any changes made to components in the dialog box
and does not dismiss it.
.LI "\f3Reset\f1\0[Required]"
.br
This button cancels any changes that have not yet been applied by the
application.
The controls within the dialog box are reset
to their state since the last time the dialog box action
was applied.
If no changes have been applied within the
current invocation of the dialog box, the controls are
reset to their state as of when the dialog box was first
displayed.
.LI "\f3Reset to Factory\f1\0[Optional]"
.br
This button cancels any changes that have not yet been applied.
Components in the dialog box are reset to their default
state or value as specified by the vendor that delivered
the application (that is, the controls are restored to the
original factory settings).
.LI "\f3Cancel\f1\0[Required]"
.br
This button dismisses the dialog box without performing any actions not
yet applied.
.LI "\f3Help\f1\0[Required]"
.br
This button provides help for the dialog box.
.LE
.LI "\f3cv:\f1\0[Recommended]"
.br
If the application provides a properties window that
displays settings for a selected object, the properties
window should track the current selection and modify the
state of any controls to reflect the properties
of the currently selected object accurately.
.LE
.H 4 "File Selection Dialog Box"
.VL 11 5
.LI "\f3cw:\f1\0[Optional]"
.br
If the application allows the user to open or save files,
then it uses the standard \*(Zx file selection dialog box
to allow the user to select specific files and
directories.
.Ns
All user interactions with the file system should be
facilitated by providing a point-and-click style of
choosing files and directories.
The user should never be
forced to memorise and type in file paths.
The user must
be able to explore the contents and structure of the file
system using scrolling lists.
The expert user, however,
should be able to directly enter a complete file path, as
well as be able to use relative paths and environment
variables such as
.IR HOME .
.P
The labels and contents of the standard file selection
dialog box may be modified as appropriate to make clear the
particular context in which it is being used within the
application.
.Ne
.LI "\f3cx:\f1\0[Recommended]"
.br
If the application allows the objects it manages to exist
as separate entities within folders or toolboxes within the
desktop environment, a Copy To menu option or button should
be provided that displays a file selection dialog box that
allows the user to select the desired folder in which an
icon for the object should be placed.
.LI "\f3cy:\f1\0[Recommended]"
.br
The file selection dialog box should not display hidden
(dot) directories or files, unless the user depends on
using these types of files.
If the application does
support displaying hidden files, the application should
supply a checkbox allowing users to toggle between showing
and not showing hidden files.
.LI "\f3cz:\f1\0[Recommended]"
.br
The file selection dialog box should not show the full pathnames
for files and directories, but should only show the
relative names, except for the directory text field.
.Ns
The global \*(Zx setting should be:
.Cs I
XmFileSelectionBox.fullPathMode: false
.Ce
.P
Unless the application overrides this behaviour, the file
selection dialog box should not show full pathnames in the
list boxes.
.Ne
.LI "\f3da:\f1\0[Required]"
.br
In general, the file selection dialog box should recall the
directory location that was previously set by the user.
.Ns
For example, if the user brings up ``Save As'' and
navigates to
.B /users/jay/letters
to save the file, the next
time the user brings up ``Save As,'' the file selection box
should be in the directory
.BR /users/jay/letters .
This information, however, should not be recalled once the user
has closed the primary window; it should resort to the
default directory.
.Ne
.LE
.H 4 "About Dialog Box"
.VL 11 5
.LI "\f3db:\f1\0[Optional]"
.br
The About Box should contain a minimum set of information
about the application that is visible in a single text pane.
That minimum set should be:
.BL
.LI
Application Name
.LI
Version Number
.LI
Release Date
.LI
Copyright
.LE
.LI "\f3dc:\f1\0[Required]"
.br
The About box contains a Close button.
Other buttons, such as Help and More, are optional.
.LE
.P
Other information contained in the About box might be:
.VL 11 5
.LI "\f3dd:\f1\0[Recommended]"
.br
Information about the operating system or other aspects
required to run the application;
for example, \*(Zx 1.0.
.LI "\f3de:\f1\0[Optional]"
.br
A ``More Information'' dialog box for additional
information such as development team credits, licencing,
client or xhost information.
.LI "\f3df:\f1\0[Optional]"
.br
A ``Comments'' button for people to type in comments about
the application and send them to a customer feedback group.
If the application provide a ``Comments'' dialog
box, it must have a Help button.
.LE
.H 4 "Dialog Box Layout"
The following requirements apply only in a left-to-right
language environment in an English-language locale.
The application must make the appropriate changes for other locales.
.VL 11 5
.LI "\f3dg:\f1\0[Optional]"
.br
Controls within the dialog box are placed in a left-right,
top-down layout based on the order in which the user is
expected to fill out or choose options within the dialog box.
.br
.mc 6
.LI "\f3dh:\f1\0[Required]"
.br
Push buttons that affect the dialog box as a whole, either
by modifying its contents or layout, invoking the action of
the dialog box or dismissing the dialog box, must be located at
the bottom of the dialog box.
.Ns
.mc
In general, there should only be one row of buttons at the
bottom of a dialog box.
If the application has dialog
boxes that contain several global buttons, it may be
necessary to create two or more rows of buttons at the
bottom of the dialog box.
The last row should contain the
standard dialog box buttons (OK, Reset, Cancel, Help).
If a dialog box contains buttons that are not related to the
dialog box as a whole, but relate to a specific control
within the dialog box, the buttons should be located with
the control to which they relate.
.Ne
.LI "\f3di:\f1\0[Required]"
.br
If the application provides an Apply button within a dialog
box, it also provides an OK button or command button that
performs the dialog box action then dismisses it.
.LI "\f3dj:\f1\0[Optional]"
.br
The application should not use cascade buttons within dialog boxes.
.Ns
In general, cascade buttons should only be used within
menus and menu bars.
The application should avoid their
use in all other locations unless absolutely necessary.
.Ne
.LE
.H 4 "Designing Drag and Drop"
.VL 11 5
.LI "\f3dk:\f1\0[Required]"
.br
The application should provide a drag-and-drop (DND) method
for all objects represented as icons and
for all elements that the user can directly manipulate.
.LI "\f3dl:\f1\0[Recommended]"
.br
Any basic function that the application supports through
drag and drop also should be supported through menus,
buttons or dialog boxes.
.Ns
Drag and drop is considered an accelerator to functionality
that is accessible through other user interface controls
supported within the application.
There should be no basic
operation that is supported solely through drag and drop.
.Ne
.LI "\f3dm:\f1\0[Required]"
.br
The application supports the same drag-and-drop operation
on either BSelect or BTransfer.
The application does not
map one type of drag-and-drop operation to BSelect and a
different one to BTransfer.
.Ns
Drag and drop should be consistent regardless of which
operations are mapped to mouse buttons 1 and 2 by the user.
.Ne
.LI "\f3dn:\f1\0[Recommended]"
.br
The application should use an icon graphic in a dialog box
or window to indicate that objects within the dialog box or
window can be dragged.
The same icon graphic should be
used to represent the draggable object in File Manager.
The icon should be placed adjacent to any display of the
contents of the object, if such display exists.
If there
is no such display, the icon should be placed in the upper
right corner of the dialog box or window, unless a more
suitable placement is determined.
The icon should be
32\|\(mu\|32 in size and have a label under it.
The label
should indicate what kind of object the icon graphic
represents.
The icon graphic should also be used as the
source indicator in the drag icon.
.LI "\f3do:\f1\0[Required]"
.br
During a drag operation, the application changes the
current pointer to a drag icon.
.Ns
A drag icon provides visual feedback that a drag operation
is in progress.
.Ne
.LI "\f3dp:\f1\0[Recommended]"
.br
During a drag operation, the application should change the
current drag cursor to include a source indicator.
.Ns
A source indicator gives a visual representation of the
elements being dragged.
.Ne
.LI "\f3dq:\f1\0[Recommended]"
.br
During a drag operation, the application should change the
current drag cursor to indicate invalid drop zones.
It uses the standard \*(Zx Not Possible pointer.
.Ns
The user must receive feedback as to where an object can
and cannot be dropped.
Minimally, feedback should be
provided as to what are invalid drop zones.
Feedback for valid drop zones should be enhanced by use of
animation, recessing of the target drop zone and other such
drag-over effects.
.Ne
.LI "\f3dr:\f1\0[Recommended]"
.br
During a drag operation, the application should change the
drop zone feedback to indicate a valid drop zone.
.Ns
Preferably, feedback for valid drop zones is enhanced by
use of animation, recessing of the target drop zone and
other such drag-over effects.
.Ne
.LI "\f3ds:\f1\0[Required]"
.br
Pressing Cancel ends a drag-and-drop operation by canceling
the drag in progress.
.Ns
Cancel provides a consistent way for the user to cancel a
drag operation.
.Ne
.LI "\f3dt:\f1\0[Required]"
.br
Releasing BTransfer (or BSelect) when not over a drop
target ends a drag-and-drop operation.
.Ns
Releasing BTransfer (or BSelect) offers a consistent means
of ending a drag operation.
.Ne
.LI "\f3du:\f1\0[Optional]"
.br
Any cursor change or drag-over effect the application uses
occurs within 0.2 seconds of the mouse pointer reaching the
target area and does not interfere, in any noticeable way,
with the interactive performance of the drag operation.
.LI "\f3dv:\f1\0[Recommended]"
.br
In a collection that supports copy, move or link operations
that can be performed by dragging, the feedback presented
to the user during the drag operation should indicate
whether a single object or multiple objects are being
manipulated.
.Ns
Feedback provided during the drag operation should ensure
that the user feels confident that the desired set of
objects is being dragged.
The drag icon used for
multi-object drag operations should integrate the feedback
used to indicate whether the operation is a move, copy or link.
.Ne
.LI "\f3dw:\f1\0[Required]"
.br
After a successful transfer, the data is placed in the drop
zone and any transfer icon used by the application is
removed.
.Ns
A transfer icon can be used to represent the type of data
being transferred during a drop operation.
A successful
drop operation results in the transfer of data.
.Ne
.LI "\f3dx:\f1\0[Required]"
.br
If the application removes data upon the completion of a
drag and drop, it does so only if the drag-and-drop
transfer has completed successfully.
.Ns
If a drag-and-drop operation has been canceled or failed,
the data or object that was the source of the drag must not
be removed.
.Ne
.LI "\f3dy:\f1\0[Required]"
.br
After a failed transfer, the data remains at the drag
source and is not placed in the drop zone.
Any transfer
icon used by the application is removed.
.Ns
A failed drop operation does not result in the transfer of data.
.Ne
.LI "\f3dz:\f1\0[Recommended]"
.br
If the user drops an object at an inappropriate drop zone
within the application's window, the application should
participate in the display of a snap back effect and also
should post an error dialog box indicating the reason the
drop was disallowed.
.Ns
The error message should state the context (for example,
running action A on object B), what happened (for example,
could not connect to system X) and how to correct the
problem (for example, press the Help button to obtain
information on diagnosing remote execution problems).
.Ne
.LI "\f3ea:\f1\0[Recommended]"
.br
An application that accepts only single items should reject
all multiple-item drops.
.Ns
There is no consistent method to determine which of the
selected items the user really wants to drop.
.Ne
.LI "\f3eb:\f1\0[Recommended]"
.br
If the application supports drag and drop as a means of
loading a file into the application, the application should
respond to this operation in a manner similar to when the
file is loaded through more conventional means such as
choosing Open from the File menu.
.Ns
As an accelerator, drag-and-drop loading of files should
provide the same kind of feedback and behaviour as choosing
Open from the File menu.
For example, if changes to a
currently loaded file have not yet been saved, the
application should display a message dialog box asking
whether the changes should first be saved before loading
the new file.
.Ne
.LI "\f3ec:\f1\0[Required]"
.br
When mail messages or calendar appointments are dragged
from those applications and dropped onto File Manager, they
must be named.
The underlying API supports a label for the
item being dragged.
This label should be used as the name
of the mail message or appointment.
The label should be
determined in a manner consistent with the application from
where it came.
If there is no appropriate label, as in
dropping a text selection in File Manager, File Manager
should name the resulting file ``untitled''.
If there is a
name conflict, File Manager should put up a dialog box and
ask the user to rename the dropped file.
.br
.mc 6
.LI "\f36-17:\f1\0[Required]"
.br
.mc
If the application provides any drag-and-drop help dialog
boxes, they contain a Cancel button for canceling the
drag-and-drop operation in progress
(Section 6.2.5.4 of the \*(ZY).
.Ns
The Cancel button in the help dialog box provides a
convenient way for the user to cancel a drag-and-drop
operation.
.Ne
.LE
.H 3 "Attachments"
.VL 11 5
.mc 6
.LI "\f3ed:\f1\0[Recommended]"
.br
Drag and drop should not be the only method for attaching
objects.
.LI "\f3ee:\f1\0[Recommended]"
.br
Double-clicking should be a shortcut for selecting the attachment
and choosing the Open menu item for attachments and should
.mc
never be the only way to access attachments.
.br
.mc 6
.LI "\f3ef:\f1\0[Recommended]"
.br
When the user attempts to drop something into the
attachment list that is not attachable, then the drop fails
and the item should be snapped back to its source.
.LI "\f3eg:\f1\0[Recommended]"
.br
.mc
When the user has one or more attachments open for editing
and attempts to do any operation that would result in
potentially losing their edits, they should be clearly
warned and given the opportunity to save changes.
.br
.mc 6
.LI "\f3eh:\f1\0[Recommended]"
.br
When the user chooses something to attach from the File
Selection box that is not an attachable item, then the
user should receive an error message explaining why the chosen
item cannot be attached.
.mc
For example:
The folder
``My.Stuff'' cannot be attached because it is a folder.
.P
Only documents, applications and scripts can be attached.
.LE
.H 3 "Installation"
.VL 11 5
.LI "\f3ei:\f1\0[Required]"
.br
An application should be installed to folders in the
Application Manager, not directly to the front panel or subpanels.
For consistency, only \*(Zx desktop components
install to these locations.
Users may choose to
rearrange their front panel, but the application must not
do this without user consent.
.LE
.H 3 "Interaction"
.VL 11 5
.mc 6
.LI "\f36-18:\f1\0[Required]"
.br
.mc
A Warning dialog box allows the user to cancel the
destructive action about which the dialog box is providing
a warning
(Section 6.3.2.2 of the \*(ZY).
.Ns
The user should have a way to cancel an operation that
can cause destructive results.
.Ne
.LI "\f3ej:\f1\0[Required]"
.br
When the application displays a dialog box, it places the
input focus at the first text field into which the user is
allowed to type an entry or at the first control within the
dialog box with which the user should interact.
.Ns
Input focus should always be placed at a predictable and
intuitive location.
The user should not be forced to set
focus at the control most likely to be used when the window
is displayed.
.Ne
.LI "\f3ek:\f1\0[Recommended]"
.br
As the user presses the Tab key within dialog boxes of the
application, the input focus should move to different
controls within the window in a left-right, top-down
order.
.Ns
These requirements apply only in a left-to-right language
environment in an English-language locale.
The application must make the appropriate changes for other locales.
.Ne
.LI "\f3el:\f1\0[Required]"
.br
There is always exactly one control within any window of
the application that has the input focus if the window in
which it resides has the input focus.
.Ns
If any window within the application has focus, some
control within that window must have focus.
The user
should not have to explicitly set focus to a control within
the window.
.Ne
.LI "\f3em:\f1\0[Optional]"
.br
When a text field within the application does not have the
input focus, the text cursor is not displayed within that
field.
.Ns
Although use of inactive text cursors is allowed within the
Motif style, it is better to hide the text cursor on focus
out rather than display the inactive text cursor.
This makes it easier for the user to quickly scan the screen or
a window and determine which text field currently has
focus.
.Ne
.LI "\f3en:\f1\0[Optional]"
.br
The application provides keyboard mnemonics for all
buttons, menus and menu items displayed within the
application.
.Ns
Once the user becomes adept at using the application,
keyboard mnemonics provide the user a quick way to access
functionality.
Mnemonics also facilitate access to
functionality from within keyboard-centric applications or
windows.
The user need not frequently switch between use
of the mouse or use of the keyboard.
Mnemonics should be
provided pervasively throughout the user interface.
.Ne
.LI "\f3eo:\f1\0[Optional]"
.br
The application provides keyboard accelerators for those
functions that are expected to be used frequently by the user.
.Ns
Keyboard accelerators provide the user who has become
expert at using the application a quick way to access
application functionality without having to go through
menus and dialog boxes.
.Ne
.LI "\f3ep:\f1\0[Required]"
.br
Dialog boxes displayed by the application never block input
to other applications within the desktop (that is, they are
not system modal) unless it is absolutely essential that
the user perform no other action in the desktop until the
user responds to the dialog box.
.Ns
The application must allow the user the freedom to access
information and tools within the user's desktop environment.
An application should rarely block access to other applications and
services within the environment.
.Ne
.LI "\f3eq:\f1\0[Required]"
.br
Dialog boxes displayed by the application never block
access to other functionality within the application
(application modal) unless it is essential that the state
of the application remains unchanged until the user
responds to the dialog box.
.LI "\f3er:\f1\0[Required]"
.br
.mc 6
If the application does not use the values of global environment
settings (such as multiclick timeout intervals, drag
thresholds, window colour settings, mouse left- or
right-handedness),
but instead uses its own values for these settings,
then the application provides one or more Options dialog
.mc
boxes that allow the user to change the values for these
settings.
.Ns
In general, the application should not override the value
of settings treated as global environment settings.
These settings are controlled by the user through the \*(Zx Style
Manager.
If the application choose to ignore these
settings and specify the own settings, then the application
will behave inconsistently with other applications in the
\*(Zx desktop.
If the application nevertheless choose to
provide the own values, then the application must provide
the user a way to make the settings consistent with the
rest of the desktop.
.Ne
.LE
.H 3 "Visuals"
.VL 11 5
.LI "\f3es:\f1\0[Recommended]"
.br
Any icons or graphics displayed by the application should
be designed to be distinguishable on low- (640\|\(mu\|480),
medium- (800\|\(mu\|600) and high- (mega-pixel) resolution
displays.
Alternatively, the application provides
different sized visuals for low-, medium- and
high-resolution displays.
.Ns
Desktop system configurations are including more
high-resolution monitors.
The user must be able to discern
any visuals used by the application on these type of
monitors.
The embedded base, however, still contains many
standard VGA monitors.
The application's visuals must
display well on these systems and should not appear overly
large.
.Ne
.LI "\f3et:\f1\0[Recommended]"
.br
Any icons or graphics displayed by the application should
be designed to display well on black-and-white and
grey-scale monitors.
These visuals also display well on
low-colour (16) systems.
.LI "\f3eu:\f1\0[Recommended]"
.br
Icons should be used to represent only objects and
applications.
.Ns
Icons provide a visual representation for objects and
facilitate direct manipulation.
If icons are used for
other purposes (for example, as illustrations) where the
user cannot drag them, select them and so on, it creates a
confusing inconsistency.
.Ne
.LI "\f3ev:\f1\0[Recommended]"
.br
Icons should use only the palette of 22 colours.
.Ns
The \*(Zx icon palette was chosen to maximise
attractiveness and readability without using an unnecessary
number of colours.
Use of additional colours may cause
undesirable colour shifting on the display.
.Ne
.LI "\f3ew:\f1\0[Recommended]"
.br
Icons should be designed for international use.
.Ns
The application should not use text, symbols, humour, animals and other items
that may be interpreted differently in other cultures.
.Ne
.LI "\f3ex:\f1\0[Recommended]"
.br
Icons of sizes 16\|\(mu\|16 and 32\|\(mu\|32 should be left-aligned;
any
empty bits should be on the right side of the bounding
box.
.LI "\f3ey:\f1\0[Recommended]"
.br
Icons of size 48\|\(mu\|48 should be centered in the bounding
box.
.LE
.H 3 "Toolbars"
.VL 11 5
.LI "\f3ez:\f1\0[Required]"
.br
If the application use a tool bar, it should be used only
.mc 6
in windows with a menu bar.
.br
.mc
.LI "\f3fa:\f1\0[Required]"
.br
Tool bars should contain only operations that are already
available to the user in the application menus.
All items
in a tool bar should be redundant.
.LI "\f3fb:\f1\0[Required]"
.br
When an action represented by a tool bar icon is
unavailable to the user, that icon should be made
insensitive, with the associated stippled appearance.
Whenever a menu item is made insensitive, the corresponding
tool bar item is made insensitive as well.
.LI "\f3fc:\f1\0[Recommended]"
.br
The application should give users the option to hide the
tool bar.
.LI "\f3fd:\f1\0[Required]"
.br
The tool bar container is placed directly under the menu
bar and should be the same width as the window, as well as
similar height to the menu bar.
.LI "\f3fe:\f1\0[Required]"
.br
If the application use a tool bar in the application, then
the application should provide a status line in the same
primary window as the tool bar.
.Ns
This status line should provide immediate feedback to the
user as to the purpose of the button that the mouse is
currently over or that has the keyboard focus.
When the
arrow is over a tool bar icon, the status line should
display a brief definition of what the icon represents or
what will happen when the user clicks the icon.
.Ne
.LI "\f3ff:\f1\0[Recommended]"
.br
The application may provide labels under tool bar icons.
These labels should serve to explain the purpose of the icon.
.LI "\f3fg:\f1\0[Recommended]"
.br
Drawn buttons in the tool bar should be the same width and height.
Similar or related items should be grouped and
groups should be evenly spaced across the tool bar.
.LI "\f3fh:\f1\0[Recommended]"
.br
All pixmaps in the tool bar should be the same size.
.Ns
This ensures that all the tool bar buttons are the same size.
.Ne
.LI "\f3fi:\f1\0[Recommended]"
.br
The pixmap should be 24\|\(mu\|24.
The default for the drawn
button is to resize itself according to the size of its
label type, which, in this case, would be a pixmap.
.LE
.H 3 "Expandable Windows"
.xR3 expandablewindows
.VL 11 5
.LI "\f3fj:\f1\0[Recommended]"
.br
The primary pane of the dialog box or window should contain
all of the controls needed to complete the task.
This should include all critical and frequently used
functionality.
.LI "\f3fk:\f1\0[Recommended]"
.br
It is assumed that infrequently used features are placed in
the secondary pane.
The core functionality of the
application should not depend on any controls placed in
secondary panes.
.LI "\f3fl:\f1\0[Required]"
.br
Command buttons are aligned along the bottom of the dialog
box.
When the window is expanded to show a secondary pane,
then buttons are moved to the bottom of the secondary pane.
See
.cX appldesign
for information about layout of action buttons in dialog boxes.
.LI "\f3fm:\f1\0[Recommended]"
.br
If important controls must be placed in the secondary pane,
the application can specify that the window in question
should be displayed in its expanded state by default.
.P
.mc 6
.Bl
.mc
.LI "\f3fn:\f1\0[Recommended]"
.br
The secondary pane should expand in the direction most
consistent with users' expectations, the reading pattern of
the language in which it will be displayed and the content
of the information displayed.
.LI "\f3fo:\f1\0[Recommended]"
.br
.mc 6
If possible, the panes should have the same default width.
.br
.mc
.LI "\f3fp:\f1\0[Required]"
.br
A separator is used to separate the primary from the secondary pane.
.Ns
The user must have clear visual feedback as to which
elements are in the primary and which in the secondary
panes of the expandable window.
.Ne
.LI "\f3fq:\f1\0[Required]"
.br
If a window is resizable, any sizing changes are
allocated to the pane containing scrolling lists or text
fields whose displayed length is less than their stored length.
If both panes contain scrollable controls, size
changes are distributed evenly between the two panes.
If neither pane contains scrollable controls, the
window is not resizable.
.LI "\f3fr:\f1\0[Required]"
.br
The expandable window has one button that changes
its label based on the state of the window.
.LI "\f3fs:\f1\0[Required]"
.br
The expand button has two labels that reflect the
two states of the expandable window accurately.
The current label indicates to the user what will happen
if the user clicks on the button
.Ns
Examples of possible labels are Basic/Options,
Expand/Contract, More/Less.
.Ne
.LI "\f3ft:\f1\0[Recommended]"
.br
The button should appear in the lower left-hand corner of
the window or dialog box for expansion in the vertical
direction and in the lower right hand corner for expansion
in the horizontal direction.
.LI "\f3fu:\f1\0[Required]"
.br
If the window or dialog box contains a scrolling list
positioned to the far right side of the pane, the drawn
button is not aligned with the scroll bar.
For example, the button should be aligned with the list, not
the scroll bar.
.LI "\f3fv:\f1\0[Required]"
.br
The application remembers the state of each window or
dialog box (expanded or not expanded) independently (not collectively).
The state must be changed only by the
user and must be preserved until explicitly
altered by the user.
.LI "\f3fw:\f1\0[Recommended]"
.br
The application should remember the state of each expandable
window or dialog box across sessions, so that users do not
have to manually configure the expandable windows each time
the application is run.
.Ns
If appropriate, applications can provide a mechanism, as an
option, to allow users to set the state of an expandable
window globally for the application.
.Ne
.LE
.H 3 "Messages"
.VL 11 5
.LI "\f3fx:\f1\0[Recommended]"
.br
Messages displayed by the application should not assume that
the user has any expert knowledge about computer systems in
general or any operating system in particular.
.Ns
It is appropriate to assume that the user has knowledge
about basic terms used within the desktop such as files or
programs.
Such knowledge can be assumed to have been
learned by the user through Tutorials, Help and user
documentation.
However, terminology that is typically
understood only by an expert or frequent computer user
should be avoided unless the application is specifically
targeted at computer professionals.
Likewise, messages
returned to the application by the underlying operating
system should not be passed through to the user, but
instead, should be ``translated'' into language that can be
understood by the novice user.
.Ne
.LI "\f3fy:\f1\0[Recommended]"
.br
Error messages displayed by the application should indicate
the possible cause of the error and indicate the possible
actions the user can take in response.
.LI "\f3fz:\f1\0[Optional]"
.br
The application uses audio feedback, in addition to any
messages displayed, to signal error conditions and events.
.LI "\f3ga:\f1\0[Optional]"
.br
The application should not rely on error messages from the
underlying operating system and its library routines.
Error messages from such routines
are normally not seen by the user and even when the user
does see them, they are usually too low-level and cryptic
to be understood by non-programmers.
The application should
check for error conditions and use an Error dialog box to
present an appropriate error message in terms of the user's
actions and intentions.
.LI "\f3gb:\f1\0[Recommended]"
.br
The application should display a confirmation or warning
message dialog box to the user when an action instigated by
the user will be irreversible and potentially destructive
with respect to the information stored within the system or
the operation of the system or desktop environment.
.LI "\f3gc:\f1\0[Optional]"
.br
Urgent conditions that require immediate attention by the
user, no matter which application or desktop service the
user is currently accessing, are brought to the user's
attention using audiovisual notification.
The alarm is
signaled in the current workspace regardless of the
workspace in which the application resides.
.Ns
Some applications, such as network monitors or stock watch
programs, may need to alert the user to some event.
Both visual and audio alarms should be used
to signal the user.
The user should be able to acknowledge
the alarm and cause it to cease.
.Ne
.LI "\f3gd:\f1\0[Recommended]"
.br
The application should use footer messages only to
communicate status, progress or information (help)
messages.
It should not use the footer to present error
messages.
.Ns
The footer is a good location for prompt messages that help
the user to determine how to choose options within a window
or fill out a particular field.
It should not be used to
present error messages to the user or informational
messages that are important for the user to notice.
These should be presented in the appropriate style Message dialog box.
.Ne
.LI "\f3ge:\f1\0[Recommended]"
.br
The application should provide a Help button in all Message
dialog boxes, except those that contain self-explanatory
messages.
.Ns
An application should be designed with both the expert and
novice user in mind.
The novice user must be able to
access additional information clarifying the message, the
circumstances under which it might have been displayed and
what the user should do in response to the message.
.Ne
.LI "\f3gf:\f1\0[Recommended]"
.br
The application should use the appropriate style dialog box
for the display of messages to the user.
.LI "\f3gg:\f1\0[Optional]"
.br
An information dialog box is used to display status,
completion of activity or other informative types of
messages to which the user need not necessarily respond
other than to acknowledge having read the message.
.Ns
Minimally, information dialog boxes should have an OK
button so that the user can dismiss the dialog box.
If there is additional information available about the
situations under which the message would be displayed or
other references for the topic to which the message
relates, then a Help button should be included.
.Ne
.LI "\f3gh:\f1\0[Optional]"
.br
An Error dialog box is used to display error messages to
the user.
The Error dialog box displayed states what the
error is and specifies why it occurred.
The Error dialog
box contains a Help button so that the user may get
additional information, unless the message is
self-explanatory.
The Error dialog box contains an OK
button that dismisses the dialog box.
.Ns
A Cancel button is not required for Error dialog boxes
unless the error resulted in the suspension of an activity
that was in progress.
In this case, the message should
indicate whether the user has the option to continue the
activity or stop it and the buttons for the dialog box
should be Continue, Cancel and Help.
In general, Error
dialog boxes should not be modal unless it is critical that
the user not continue interacting with the application
until the user has acknowledged having read the error
message.
.Ne
.LI "\f3gi:\f1\0[Optional]"
.br
A question dialog box is used to ask questions of the user.
The question is clearly worded to indicate what a
Yes response or a No response means.
The buttons displayed
are Yes, No and Help.
Help provides additional information
as to what the application will do in response to a Yes or
No choice.
.Ns
Where possible, the application should extend the label for
the Yes and No buttons to make it clear what action will be
performed as a result of choosing either option.
For example, if the user has made changes to a document and has
not saved these but has chosen the application's Exit
option, the application might display a question dialog box
that asks ``Changes have not been saved.
Do you want to save these before exiting?''
The buttons could be
Yes or Save, No or Discard, Cancel and Help.
.Ne
.LI "\f3gj:\f1\0[Optional]"
.br
A warning dialog box is used to communicate the
consequences of an action requested by the user that may
result in the loss of data, system or application
accessibility or some other undesirable event.
The dialog
box is presented before the action is performed and offers
the user the opportunity to cancel the requested
operation.
The buttons displayed are Yes, No and Help or
Continue, Cancel and Help.
Help provides additional
information on the consequences of performing the action
requested.
.Ns
The use of Yes and No or Continue and Cancel depends on the
wording of the message.
The labels for Yes and No should
be extended as suggested previously.
Continue may be
replaced with a label more specific to the action that will
be performed.
.Ne
.LI "\f3gk:\f1\0[Optional]"
.br
A working dialog box is used to display in-progress
information to the user when this information is not
displayed in the footer of the application's window.
The dialog box contains a Stop button that allows the user to
terminate the activity.
The operation is terminated at the
next appropriate breakpoint and a confirmation might be
displayed asking whether the user really wants to stop the
activity.
The confirmation message might state the
consequences of stopping the action.
.LI "\f3gl:\f1\0[Optional]"
.br
The application writes error messages to the \*(Zx error
log when it is not appropriate to display these to the user
in a message dialog box, but when the message may
nevertheless be useful in diagnosing problems.
.Ns
The application might also write error messages that are
displayed to the user in the error log if it would be
valuable to the user or an administrator to refer to these
messages at some later time.
Messages written to the error
log should provide additional information about the error
and should state the context in which the error occurred.
.Ne
.LI "\f3gm:\f1\0[Optional]"
.br
Informational messages should be left aligned and displayed
in a light font in keeping with their unobtrusive nature.
The margin where informational messages are
displayed should not accept mouse focus.
.LI "\f3gn:\f1\0[Optional]"
.br
Progress messages should normally be displayed only while
the operation is in progress.
Notices and other
information that is no longer valid should be removed
within a few seconds to avoid confusion about whether or
not the information is current.
.LE
.H 3 "Work-in-Progress Feedback"
.VL 11 5
.LI "\f3go:\f1\0[Recommended]"
.br
If any command chosen by the user is expected to take
longer than two seconds to complete, but less than ten
seconds, the application should display the standard Busy
Pointer as feedback that the command is executing.
.Ns
The user must receive assurance that the application has
``heard'' the request and is working on it.
If the results
of the request cannot be displayed immediately, some
feedback must be provided.
The busy pointer should be
displayed within 0.5 seconds of execution of the command.
.Ne
.LI "\f3gp:\f1\0[Recommended]"
.br
If any command chosen by the user is expected to take
longer than ten seconds to complete, the application should
display a working dialog box or other feedback of similar
character that indicates that the application is working on
the request.
The feedback should reveal progress toward
completion of the activity.
.Ns
If an activity is expected to take a significant amount of
time (ten seconds or more), the application should display
feedback stronger than the busy pointer.
Displaying the
busy pointer for long amounts of time may lead the user to
conclude that the application has become ``hung.'' A
progress indicator should be displayed in these scenarios
that indicates that the application is still functioning
and is working on the user's request.
The progress
indicator should show how much of the activity has been
completed and what amount remains.
.Ne
.LI "\f3gq:\f1\0[Recommended]"
.br
When the application displays work-in-progress feedback to
the user, it should not block access to other applications
and services within the desktop environment.
.Ns
Multitasking should always be supported and as such the
application should allow the user to access other services
while it is busy performing some activity.
Preferably, the
user is also able to access other features within the
application even though it is currently working on another
request.
When this is supported, the application should
display an enhanced busy pointer that indicates that the
application is busy but still willing to accept input.
.Ne
.LE
.H 2 "Controls, Groups and Models"
.H 3 "CheckButton"
.VL 11 5
.LI "\f37-1:\f1\0[Required]"
.br
The application uses check buttons to select settings that
are not mutually exclusive.
A check button graphically
indicates its state with the presence or absence of a check
mark
(Chapter 9 of the \*(ZY).
.Ns
A check button is used to select settings that are not
mutually exclusive.
The user must know whether the
button is set or not.
.Ne
.LI "\f37-2:\f1\0[Required]"
.br
When the user presses BSelect in a check button, the check
button is armed.
If the check button was previously unset,
it is shown in the set state.
If the check button was
previously set, it is shown in the unset state
(Chapter 9 of the \*(ZY).
.Ns
BSelect Press arms a check button and shows the result of
activating it by releasing BSelect.
.Ne
.LI "\f37-3:\f1\0[Required]"
.br
When the user releases BSelect in the same check button in
which the press occurred:
.BL
.LI
If the check button was previously unset, it is set.
.LI
If the check button was previously set, it is unset.
.LE
.P
In all cases the check button is disarmed and, if the check
button is in a menu, the menu is unposted
(Chapter 9 of the \*(ZY).
.P
BSelect Release activates a check button.
.LI "\f37-4:\f1\0[Required]"
.br
When the user presses the Enter or Return key in a check
button, if the check button is in a window with a default
action, the default action is activated.
If the check
button is in a menu:
.BL
.LI
If the check button was previously unset, it is set.
.LI
If the check button was previously set, it is unset.
.LI
In both cases, the check button is disarmed and the menu is
unposted
(Chapter 9 of the \*(ZY).
.LE
.Ns
The Enter and Return keys perform the default action of a
window or activate a check button in a menu.
.Ne
.LI "\f37-5:\f1\0[Required]"
.br
When the user presses the Select key or Spacebar in a check
button, if the check button was previously unset, it is
set.
If the check button was previously set, it is unset.
In both cases, the check button is disarmed and, if the
check button is in a menu, the menu is unposted
(Chapter 9 of the \*(ZY).
.Ns
The Select key and Spacebar activate a check button.
.Ne
.LE
.H 3 "CommandBox"
.VL 11 5
.LI "\f37-6:\f1\0[Required]"
.br
If the application uses a command box, it is composed of a
text component with a command-line prompt for text input
and a list component for a command history area.
The list
uses either the single selection or browse selection model
(Chapter 9 of the \*(ZY).
.Ns
This requirement ensures the consistent appearance and
operation of a command box across applications.
.Ne
.LI "\f37-7:\f1\0[Required]"
.br
When an element of a command box list is selected, its
contents are placed in the text area
(Chapter 9 of the \*(ZY).
.Ns
This requirement provides a convenient way of selecting a
previously entered command.
.Ne
.LI "\f37-8:\f1\0[Required]"
.br
The list navigation actions Up Arrow, Down Arrow,
Control+Begin and Control+End are available from the text
component for moving the cursored element within the list
and thus changing the contents of the text
(Chapter 9 of the \*(ZY).
.Ns
These actions provide a convenient way to choose a command
from the list while focus remains in the text component.
.Ne
.LI "\f37-9:\f1\0[Required]"
.br
The default action of the command box passes the command in
the text area to the application for execution and adds the
command to the end of the list
(Chapter 9 of the \*(ZY).
.Ns
Maintaining a history of commands provides a convenient
means of entering often-used commands.
.Ne
.LE
.H 3 "FileSelectionBox"
.VL 11 5
.LI "\f37-10:\f1\0[Required]"
.br
If the application uses a file selection box, it contains
the following components
(Chapter 9 of the \*(ZY):
.BL
.LI
A directory text component showing the current directory path.
The user can edit the directory text component and
press Return or Enter to change the current directory.
.LI
An optional Format option button allowing users to specify
the format when saving a file.
.LI
A file name text component for displaying and editing a
file name.
This component is optional when the file
selection box is used to choose an existing file or
directory.
.LI
A group of push buttons, including a command button, an
update button, Cancel and Help.
The command button is
typically labeled Open or Save, but if there is another
label that better describes the resulting action (such as
Include), that label should be used.
Activating the
command button carries out the corresponding action and
dismisses the file selection box.
.LE
.P
When the file selection box is used to specify an existing
file (for example, to open a document), the command button
is normally labeled Open and it should be the default
action.
If the Update button is activated while a
directory is selected in the contents list, the directory
is opened, its contents are displayed in the contents list
and the directory text is updated.
If the Open button is
activated while the appropriate file is selected in the
contents list, the file is utilised by the application and
the file selection box is dismissed.
.P
When the file selection box is used to choose an existing
directory (for example, to install a set of files into the
chosen directory) or to specify a new directory, the
command button should be given an appropriate label, such
as Install, Choose, Create or OK.
If this button is
activated while the appropriate directory is selected in
the contents list, the directory is utilised by the
application and the file selection box is dismissed.
There is an additional button, labeled Update, that is
enabled whenever a directory is selected in the contents
list and opens the directory.
This Update button is the default action.
.P
When the file selection box is used to specify a new file
name (for example, a Save As dialog box), the command
button is normally labeled Save and is the default action.
This ensures the uniform appearance of a file
selection box across applications.
.LI "\f3gr:\f1\0[Optional]"
.br
Double-clicking BSelect on an item in the contents list
selects that item and activates the default action.
In all
cases, double-clicking BSelect on a directory in the
contents list opens that directory and displays its
contents in the contents list (the default action is
Open).
.BL
.LI
When the file selection box is used to choose an existing
file, double-clicking BSelect on an appropriate file in the
contents list chooses that file and dismisses the file
selection box (the default action is Open).
.LI
When the file selection box is used to choose an existing
directory or to specify a new directory or file, the files
list should not appear.
Double-clicking BSelect on a
disabled file name has no effect.
.LE
.LI "\f3gs:\f1\0[Required]"
.br
The normal text navigation and editing functions are
available in the text components for moving the cursor
within each text component and changing the contents of the text.
.Ns
These actions provide a convenient way to choose a
directory or file name from the corresponding list while
focus remains in the text component.
.Ne
.LI "\f37-11:\f1"
This item of the \*(ZY is not applicable.
.LI "\f37-12:\f1\0[Required]"
.br
Double-clicking BSelect on an item in the contents list
selects that item and activates the default action.
In all
cases, double-clicking BSelect on a directory in the
contents list opens that directory and displays its
contents in the contents list (the default action is
Open).
.BL
.LI
When the file selection box is used to choose an existing
file, double-clicking BSelect on an appropriate file in the
contents list chooses that file and dismisses the file
selection box (the default action is Open).
.LI
When the file selection box is used to choose an existing
directory or to specify a new directory or file, files are
shown in the contents list but they are all disabled.
Double-clicking BSelect on a disabled file name has no
effect.
.LE
.LI "\f37-13:\f1\0[Required]"
.br
The normal text navigation and editing functions are
available in the text components for moving the cursor
within each text component and changing the contents of the text.
.LI "\f37-14:\f1"
This item of the \*(ZY is not applicable.
.LI "\f37-15:\f1\0[Optional]"
.br
The application allows the user to select a file by
scrolling through the list of file names and selecting the
desired file or by entering the file name directly into the
file selection text component.
Selecting a file from the
list causes that file name to appear in the file selection
text area
(Chapter 9 of the \*(ZY).
.Ns
This method for selecting a file should be consistent
across applications.
.Ne
.LI "\f37-16:\f1\0[Required]"
.br
The application makes use of the selection when one of the
following occurs
(Chapter 9 of the \*(ZY):
.BL
.LI
The user activates the command push button while an
appropriate item is selected in the contents list.
.LI
The user double-clicks BSelect on an appropriate file in
the contents list.
.LI
The user presses Return or Enter while the file name text
component has the keyboard focus and contains an
appropriate item.
.LE
.LI "\f37-17:\f1\0[Required]"
.br
The file selection box displays the contents of a directory
in the contents list when the file selection box is
initialised, when the user presses Enter or Return in the
directory text component and when the user opens a
directory in the contents list.
The contents list is
updated each time the contents of the directory changes.
.Ns
This requirement ensures the consistent operation of a
directory and file search in a file selection box.
.Ne
.LI "\f3gt:\f1\0[Recommended]"
.br
If the user has opened the application without supplying a
file name argument, the Open dialog box should use the
user's home directory as the default directory.
.Ns
An exception to this requirement might be made if a clearly more
useful directory can be identified;
for example, the icon editor might default to
.BR $HOME/.dt/icons .
An applications that
allows editing should never default to a directory in which the
user does not have read and write permission.
.Ne
.LI "\f3gu:\f1\0[Required]"
.br
If the user has opened the application with a file name
argument, the open dialog box should default to the
directory in which that file resides.
.LI "\f3gv:\f1\0[Optional]"
.br
When using the file selection dialog box in Save As
capacity, the application provides a default name of Untitled, places the
location cursor in the file name field and highlights the
file name text to create a ``delete pending type-in'' mode.
If the current directory already has a file of that
name, it creates a name Untitled2, and so forth.
.LI "\f3gw:\f1\0[Optional]"
.br
When using the file selection dialog box in Save As
capacity, the application adds a file name extension if the application
supports file typing by extension and makes this extension
visible in the file name field.
It does not highlight the
extension to create a ``delete pending type-in'' mode, but
allows the user to modify the extension or delete it explicitly.
.LI "\f3gx:\f1\0[Optional]"
.br
The file selection box should display a directory that
makes sense for the task.
For example, when saving a new
file from an editor, the file selection box should come up
in the user's home directory.
If the user navigates to
some other directory within the file selection box, the
application should remember that directory the next time it
is brought up.
.LI "\f3gy:\f1\0[Optional]"
.br
Users should never be allowed to overwrite an existing file
through the file selection box without a warning dialog box
prompt.
.LI "\f3gz:\f1\0[Optional]"
.br
Keyboard focus should be placed in the file name field each
time users bring up a file selection box.
.LI "\f3ha:\f1\0[Optional]"
.br
Directory and file name lists should be presented
alphabetically, case insensitive.
The first item on the
directory list should be the parent directory and it should
be labeled ``..(go up)''.
.LI "\f3hb:\f1\0[Optional]"
.br
Labels should be clear.
In the English language, the application should use the
following labels for the file selection dialog box fields
and lists:
.br
.ne 10
.TS
center, box;
cf3 s
lf3 | lf3
l   | lf5 .
File Selection Dialog Box Labels
_
Component	Label
_
Directory text field	Enter Path or Folder Name:
Filter text Field	Filter:
Directory list	Folders:
Contents list	Files:
File text field	Enter File Name:*
.TE
.LI "\f3hc:\f1\0[Optional]"
.br
Application developers can make this label more
instructive and specific, such as ``Enter File to Open''
for Open dialog boxes, and so forth.
.Ns
These labels should be the default labels.
If they are not
set by default, the application must set them via
resources in the application's app-defaults file.
.Ne
.LE
.H 3 "List"
.VL 11 5
.LI "\f37-18:\f1\0[Required]"
.br
Within a list component, the application uses the Up Arrow
key to move the location cursor to the previous item in the
list and the Down Arrow key to move the location cursor to
the next item in the list.
In a scrollable list, the Left
Arrow key scrolls the list one character to the left and
the Right Arrow key scrolls the list one character to the right
(Chapter 9 of the \*(ZY).
.Ns
The arrow keys provide a consistent means of moving the
location cursor within a list component.
.Ne
.LI "\f37-19:\f1\0[Required]"
.br
Within a list component, the application uses Control+Begin
to move the location cursor to the first item in the list
and Control+End to move the location cursor to the last
item in the list.
In a scrollable list, the Begin key
moves the horizontal scroll region so that the leftmost
edge of the list is visible and the End key moves the
horizontal scroll region so that the rightmost edge of the
list is visible
(Chapter 9 of the \*(ZY).
.Ns
These keys offer a convenient mechanism for moving the
location cursor quickly through a list.
.Ne
.LI "\f37-20:\f1\0[Required]"
.br
Within a scrollable list, the Page Up key moves the
location cursor to the item one page up in the list and the
Page Down key moves the location cursor to the item one
page down in the list.
In a scrollable list, the Page Left
key (or Control+Page Up) scrolls the list one page to the
left and the Page Right key (or Control+Page Down) scrolls
the list one page to the right
(Chapter 9 of the \*(ZY).
.Ns
These keys offer a convenient mechanism for paging through a list.
.Ne
.LI "\f37-21:\f1\0[Required]"
.br
Within a list component, the application uses BSelect Click 2
to select the item that was double-clicked and then
initiate any default action for the window
(Chapter 9 of the \*(ZY).
.Ns
Double-clicking using BSelect provides a consistent way of
activating the default action for a list.
.Ne
.LE
.H 3 "Option Button"
.VL 11 5
.LI "\f37-22:\f1\0[Required]"
.br
If the application uses option buttons, the label for the
button is the last selection made from the option button
(Chapter 9 of the \*(ZY).
.Ns
An option button is used to post an option menu which
allows the user to select from a number of choices.
The label of an option button must display the most recent
selection from the associated option menu.
.Ne
.LI "\f37-23:\f1\0[Required]"
.br
When the user presses BSelect or BMenu in an option button,
the associated option menu is posted
(Chapter 9 of the \*(ZY).
.Ns
BSelect Press is a consistent way of activating an option
button.
.Ne
.LI "\f37-24:\f1\0[Required]"
.br
When the user releases BSelect or BMenu within the same
option button that the press occurred in, the associated
option menu is posted if it was not posted at the time of
the press.
When the user releases BSelect or BMenu outside
of the option button, the associated option menu is
unposted
(Chapter 9 of the \*(ZY).
.Ns
BSelect Release or BMenu Release posts or unposts an option
menu, depending on whether the release occurs inside the
option button and whether the option menu was posted at the
time of the press.
.Ne
.LI "\f37-25:\f1\0[Required]"
.br
When the user presses the Select key or Spacebar in an
option button, the associated option menu is posted
(Chapter 9 of the \*(ZY).
.Ns
The Select key or Spacebar posts an option menu from the
keyboard.
.Ne
.LE
.H 3 "Paned Window"
.VL 11 5
.LI "\f37-26:\f1\0[Required]"
.br
If the application uses paned windows, they are composed of
any number of groups of components, called panes, each
separated by a sash and a separator.
The panes, sashes and
separators are grouped linearly, either horizontally or
vertically.
A sash is the handle on a separator between
two panes that is used to adjust the position of the
separator
(Chapter 9 of the \*(ZY).
.Ns
This requirement ensures the consistent appearance of a
paned window across applications.
.Ne
.LE
.H 3 "Panel"
.VL 11 5
.LI "\f37-27:\f1\0[Required]"
.br
The Down Arrow, Left Arrow, Right Arrow and Up Arrow
directional keys navigate among components in a Panel
(Chapter 9 of the \*(ZY).
.Ns
A panel group organises a collection of basic controls in a
horizontal, vertical or two-dimensional layout.
The directional keys are used to navigate among the controls.
.Ne
.LE
.H 3 "Push Button"
.VL 11 5
.LI "\f37-28:\f1\0[Required]"
.br
When the user presses BSelect in a push button, the push
button is armed.
When the user releases BSelect in the
same push button that the press occurred in, the push
button is disarmed and activated.
When the user releases
BSelect outside the push button, the push button is
disarmed but not activated
(Chapter 9 of the \*(ZY).
.Ns
BSelect provides a consistent means of activating a push
button.
.Ne
.LI "\f37-29:\f1\0[Required]"
.br
When the user presses the Enter or Return key in a push
button that is in a window with a default action, the push
button is activated.
When the user presses the Enter or
Return key in a push button in a menu, the push button is
activated and the menu is unposted
(Chapter 9 of the \*(ZY).
.Ns
The Enter and Return keys activate a dialog box or a push
button in a menu.
.Ne
.LI "\f37-30:\f1\0[Required]"
.br
When the user presses the Select key or Spacebar in a push
button, the push button is activated.
If the push button
is in a menu, the menu is unposted
(Chapter 9 of the \*(ZY).
.Ns
The Select key and Spacebar activate a push button.
.Ne
.LE
.H 3 "Radio Button"
.VL 11 5
.LI "\f37-31:\f1\0[Required]"
.br
If the application uses radio buttons, each button
graphically indicates its state
(Chapter 9 of the \*(ZY).
.Ns
Radio buttons are used to represent a panel of mutually
exclusive selections.
The user must know which button
in the panel is set.
.Ne
.LI "\f37-32:\f1\0[Required]"
.br
When the user presses BSelect in a radio button, the radio
button is armed.
If the radio button was previously unset,
it is shown in the set state
(Chapter 9 of the \*(ZY).
.Ns
BSelect Press arms a radio button and shows the result of
activating it by releasing BSelect.
.Ne
.LI "\f37-33:\f1\0[Required]"
.br
When the user releases BSelect in the same radio button
that the press occurred in and the radio button was
previously unset, it is set and any other radio button in
the same panel that was previously set is unset.
The radio
button is disarmed and, if the radio button is in a menu,
the menu is unposted
(Chapter 9 of the \*(ZY).
.Ns
BSelect Release activates a radio button.
.Ne
.LI "\f37-34:\f1\0[Required]"
.br
When the user presses the Enter or Return key in a radio
button, if the radio button is in a window with a default
action, the default action is activated.
If the radio
button is in a menu
(Chapter 9 of the \*(ZY):
.BL
.LI
If the radio button was previously unset, it is set and any
other radio button in the same panel that was previously
set is unset.
.LI
The radio button is disarmed and the menu is unposted.
.LE
.Ns
The Enter and Return keys perform the default action of a
window or activate a radio button in a menu.
.Ne
.LI "\f37-35:\f1\0[Required]"
.br
When the user presses the Select key or Spacebar in a radio
button, if the radio button was previously unset, it is set
and any other radio button in the same panel that was
previously set is unset.
The radio button is disarmed and,
if the radio button is in a menu, the menu is unposted
(Chapter 9 of the \*(ZY).
.Ns
The Select key and Spacebar activate a radio button.
.Ne
.LE
.H 3 "Sash"
.VL 11 5
.LI "\f37-36:\f1\0[Required]"
.br
Within a paned window, the application uses a sash to
adjust the position of a separator, which adjusts the sizes
of the panes next to it.
As a sash is moved, the pane in
the direction of the sash movement gets smaller and the
opposite pane gets larger by an equal amount
(Chapter 9 of the \*(ZY).
.Ns
This requirement results in the uniform operation of a
paned window across applications.
.Ne
.LI "\f37-37:\f1\0[Required]"
.br
Within a sash, BSelect Motion or BTransfer Motion causes
the sash to track the movement of the pointer.
In a vertically oriented paned window, the sash tracks the
vertical position of the pointer.
In a horizontally
oriented paned window, the Pane tracks the horizontal
position of the pointer
(Chapter 9 of the \*(ZY).
.Ns
BSelect, mouse button 1 and BTransfer, mouse button 2,
provide a consistent means of moving a sash in a paned
window using the mouse.
.Ne
.LI "\f37-38:\f1\0[Required]"
.br
The Up Arrow and Down Arrow keys (for a sash that can move
vertically) and the Left Arrow and Right Arrow keys (for a
sash that can move horizontally) move the sash one
increment in the specified direction
(Chapter 9 of the \*(ZY).
.Ns
The arrow keys offer a uniform means of moving a sash in a
paned window.
.Ne
.LI "\f37-39:\f1\0[Required]"
.br
Control+Up Arrow and Control+Down Arrow (for a sash that
can move vertically) and Control+Left Arrow and
Control+Right Arrow (for a sash that can move horizontally)
move the sash one large increment in the specified
direction
(Chapter 9 of the \*(ZY).
.Ns
These keys provide a convenient way of moving a sash
quickly in a paned window.
.Ne
.LE
.H 3 "Scale"
.VL 11 5
.LI "\f37-40:\f1\0[Required]"
.br
If a scale has arrow buttons, the application uses BSelect
Press in an arrow button to move the slider one increment
in the direction of the side of the slider on which the
button was pressed and autorepeats until the button is
released
(Chapter 9 of the \*(ZY).
.Ns
BSelect Press provides a consistent means of adjusting a
scale component using the mouse.
.Ne
.LI "\f37-41:\f1\0[Required]"
.br
In a scale trough, if the scale has tick marks, BSelect
Press moves the slider one major tick mark in the direction
of the side of the slider on which the trough was pressed
and autorepeats until the button is released.
If the scale
does not have tick marks, BSelect Press in the trough moves
the slider one large increment in the direction of the side
of the slider on which the trough was pressed and
autorepeats until the button is released
(Chapter 9 of the \*(ZY).
.Ns
BSelect Press provides a consistent means of adjusting a
scale component using the mouse.
.Ne
.LI "\f37-42:\f1\0[Required]"
.br
Within a scale slider, BSelect Motion causes the slider to
track the position of the pointer.
In a vertical scale,
the slider tracks the vertical position of the pointer.
In a horizontal scale, the slider tracks the horizontal
position of the pointer
(Chapter 9 of the \*(ZY).
.Ns
BSelect Motion offers a convenient way to adjust a scale
component precisely using the mouse.
.Ne
.LI "\f37-43:\f1\0[Required]"
.br
Within a scale slider or trough, BTransfer Motion positions
the slider to the point of the button press and then causes
the slider to track the position of the pointer.
In a vertical scale, the slider tracks the vertical position of
the pointer.
In a horizontal scale, the slider tracks the
horizontal position of the pointer
(Chapter 9 of the \*(ZY).
.Ns
BTransfer Motion provides another convenient way to adjust
a scale component precisely using the mouse.
.Ne
.LI "\f37-44:\f1\0[Required]"
.br
If a mouse-based sliding action is in progress, the Cancel
key cancels the sliding action and returns the slider to
its position prior to the start of the sliding operation
(Chapter 9 of the \*(ZY).
.Ns
The Cancel key provides a consistent way for the user to
cancel a mouse-based sliding action.
.Ne
.LI "\f37-45:\f1\0[Required]"
.br
In a vertical scale, the Up Arrow and Down Arrow keys move
the slider one increment in the specified direction.
In a horizontal scale, the Left Arrow and Right Arrow keys move
the slider one increment in the specified direction
(Chapter 9 of the \*(ZY).
.Ns
The arrow keys provide a uniform way of adjusting the
slider in a scale component using the keyboard.
.Ne
.LI "\f37-46:\f1\0[Required]"
.br
In a vertical scale, Control+Up Arrow and Control+Down
Arrow move the slider one large increment in the specified
direction.
In a horizontal scale, Control+Left Arrow and
Control+Right Arrow move the slider one large increment in
the specified direction
(Chapter 9 of the \*(ZY).
.Ns
These keys provide a convenient way of adjusting the slider
in a scale component quickly using the keyboard.
.Ne
.LI "\f37-47:\f1\0[Required]"
.br
The application uses the Begin key or Control+Begin to move
the slider to its minimum value.
The End key or
Control+End moves the slider to its maximum value
(Chapter 9 of the \*(ZY).
.Ns
These keys provide a convenient mechanism for setting a
scale to its minimum or maximum value using the keyboard.
.Ne
.LE
.H 3 "ScrollBar"
.VL 11 5
.LI "\f37-48:\f1\0[Required]"
.br
Within a scroll bar, the application uses BSelect Press in
an arrow button to move the slider one increment in the
direction of the side of the slider on which the button was
pressed and autorepeats until the button is released
(Chapter 9 of the \*(ZY).
.Ns
BSelect Press provides a consistent means of adjusting a
scroll bar using the mouse.
.Ne
.LI "\f37-49:\f1\0[Required]"
.br
In the trough of a scroll bar, BSelect Press moves the
slider one page in the direction of the side of the slider
on which the trough was pressed and autorepeats until the
button is released
(Chapter 9 of the \*(ZY).
.Ns
BSelect Press provides a consistent means of adjusting a
scroll bar using the mouse.
.Ne
.LI "\f37-50:\f1\0[Required]"
.br
Within a scroll-bar slider, BSelect Motion causes the
slider to track the position of the pointer.
In a vertical
scroll bar, the slider tracks the vertical position of the
pointer.
In a horizontal scroll bar, the slider tracks the
horizontal position of the pointer
(Chapter 9 of the \*(ZY).
.Ns
BSelect Motion offers a convenient way to adjust a scroll
bar precisely using the mouse.
.Ne
.LI "\f37-51:\f1\0[Required]"
.br
Within a scroll-bar slider or trough, BTransfer Motion
positions the slider to the point of the button press and
then causes the slider to track the position of the
pointer.
In a vertical scroll bar, the slider tracks the
vertical position of the pointer.
In a horizontal scroll
bar, the slider tracks the horizontal position of the
pointer
(Chapter 9 of the \*(ZY).
.Ns
BTransfer Motion offers another convenient way to adjust a
scroll bar precisely using the mouse.
.Ne
.LI "\f37-52:\f1\0[Required]"
.br
If a mouse-based scrolling action is in progress, pressing
the Cancel key cancels the scrolling action and returns the
slider to its position prior to the start of the scrolling
operation
(Chapter 9 of the \*(ZY).
.Ns
The Cancel key provides a consistent way for the user to
cancel a mouse-based scrolling action.
.Ne
.LI "\f37-53:\f1\0[Required]"
.br
In a vertical scroll bar, the Up Arrow and Down Arrow keys
move the slider one increment in the specified direction.
In a horizontal scroll bar, the Left Arrow and Right Arrow
keys move the slider one increment in the specified
direction
(Chapter 9 of the \*(ZY).
.Ns
The arrow keys provide a uniform means of adjusting a
scroll bar using the keyboard.
.Ne
.LI "\f37-54:\f1\0[Required]"
.br
In a vertical scroll bar, Control+Up Arrow and Control+Down
Arrow move the slider one large increment in the specified
direction.
Control+Left Arrow and Control+Right Arrow move
the slider one large increment in the specified direction
(Chapter 9 of the \*(ZY).
.Ns
These keys provide a convenient way of adjusting a scroll
bar quickly using the keyboard.
.Ne
.LI "\f37-55:\f1\0[Required]"
.br
The application uses the Page Up and Page Down keys to move
the slider in a vertical scroll bar one page in the
specified direction.
The Page Left key (or Control+Page
Up) and the Page Right key (or Control+Page Down) move the
slider in a horizontal scroll bar one page in the specified
direction
(Chapter 9 of the \*(ZY).
.Ns
These keys allow for the convenient movement of the slider
in a scroll bar using the keyboard.
.Ne
.LI "\f37-56:\f1\0[Required]"
.br
The application uses the Begin key or Control+Begin to move
the slider to the minimum value.
The End key or
Control+End moves the slider to the maximum value
(Chapter 9).
.Ns
These keys offer a convenient mechanism for setting a
scroll bar to its minimum or maximum value using the
keyboard.
.Ne
.LE
.H 3 "SelectionBox"
.VL 11 5
.LI "\f37-57:\f1\0[Required]"
.br
If the application uses a selection box, it is composed of
at least a text component for the selected alternative and
a list component above the text component for presenting
alternatives.
The list uses either the single selection or
browse selection model.
Selecting an element from the list
places the selected element in the text component
(Chapter 9 of the \*(ZY).
.Ns
This requirement ensures the consistent appearance and
operation of a selection box across applications.
.Ne
.LI "\f37-58:\f1\0[Required]"
.br
The list navigation actions Up Arrow, Down Arrow,
Control+Begin and Control+End are available from the text
component for moving the cursored element within the list
and thus changing the contents of the text
(Chapter 9 of the \*(ZY).
.Ns
These actions provide a convenient way to choose an element
from the list while focus remains in the text component.
.Ne
.LE
.H 3 "Text"
.VL 11 5
.LI "\f37-59:\f1\0[Required]"
.br
In a multi-line text component, the Up Arrow key moves the
location cursor up one line and the Down Arrow key moves
the location cursor down one line.
In a single-line text
component, the Up Arrow key navigates upward to the
previous component and the Down Arrow key navigates
downward to the next component, if the text component is
designed to act like a basic control
(Chapter 9 of the \*(ZY).
.Ns
The up and down arrow keys provide a uniform means of
navigation within text components.
.Ne
.LI "\f37-60:\f1\0[Required]"
.br
The Left Arrow key moves the location cursor left one
character and The Right Arrow key moves the location cursor
right one character
(Chapter 9 of the \*(ZY).
.Ns
The left and right arrow keys offer a consistent way of
navigating within text components.
.Ne
.LI "\f37-61:\f1\0[Required]"
.br
In a text component used generally to hold multiple words,
Control+Right Arrow moves the location cursor to the right
by a word and Control+Left Arrow moves the location cursor
to the left by a word
(Chapter 9 of the \*(ZY).
.Ns
Control+Right Arrow and Control+Left Arrow provide a
uniform way of navigating by words in a text component.
Moving right by a word means that the location cursor is
placed before the first character that is not a space, tab
or newline character after the next space, tab or newline.
Moving left by a word means that the location cursor is
placed after the first space, tab or newline character
preceding the first previous character that is not a space,
tab or newline.
.Ne
.LI "\f37-62:\f1\0[Required]"
.br
In a text component used generally to hold multiple words,
the Begin key moves the location cursor to the beginning of
the line and the End key moves the location cursor to the
end of the line
(Chapter 9 of the \*(ZY).
.Ns
These keys allow the user to move quickly to the beginning
or end of a line of text in a text component.
.Ne
.LI "\f37-63:\f1\0[Required]"
.br
In a multi-line text component, Control+Begin moves the
location cursor to the beginning of the file and
Control+End moves the location cursor to the end of the
file
(Chapter 9 of the \*(ZY).
.Ns
These keys permit the user to move quickly to the beginning
or end of a file in a text component.
.Ne
.LI "\f37-64:\f1\0[Required]"
.br
The application uses Spacebar or Shift+Spacebar to insert a
space in a text component.
Modifying these with Control
invokes the normal selection function
(Chapter 9 of the \*(ZY).
.Ns
This requirement ensures that selection is available from
the keyboard in a text component.
.Ne
.LI "\f37-65:\f1\0[Required]"
.br
Return in a multi-line text component inserts a carriage
return.
The Enter key or Control+Return invokes the
default action
(Chapter 9 of the \*(ZY).
.Ns
This requirement ensures that activation is available
from the keyboard in a text component.
.Ne
.LI "\f37-66:\f1\0[Required]"
.br
In a multi-line text component, Tab is used for tabbing.
In a single-line text component, Tab is used either for
tabbing or to move to the next field
(Chapter 9 of the \*(ZY).
.LI "\f37-67:\f1\0[Required]"
.br
If a text component supports replace mode, Insert toggles
between insert mode and replace mode.
.P
By default, the component starts in insert mode, where the
location cursor is between two characters.
In insert mode,
typing a character inserts the character at the position of
the location cursor.
.P
In replace mode, the location cursor is on a character.
Typing a character replaces the current character with that
newly entered character and moves the location cursor to
the next character, selecting it
(Chapter 9 of the \*(ZY).
.Ns
These requirements ensure the uniform operation of a text
component with a replace mode.
.Ne
.LI "\f37-68:\f1\0[Required]"
.br
The application uses BSelect Click 2 to select text a word
at a time
(Chapter 9 of the \*(ZY).
.Ns
Double-clicking with mouse button 1 provides a convenient
mechanism for selecting words in a text component.
.Ne
.LE
.H 3 "Gauge"
.VL 11 5
.LI "\f3hd:\f1\0[Required]"
.br
A gauge is similar to a scale except that a gauge is a
display-only device with no user interactions.
The appearance of a gauge is similar to a scale, but the gauge
lacks a scale slider.
.LI "\f3he:\f1\0[Optional]"
.br
Despite being a display-only device, a gauge should get
keyboard focus so that the user can access Help or Settings
for that control.
.LE
.H 2 "Accessibility"
.VL 11 5
.LI "\f3hf:\f1\0[Recommended]"
.br
All application functions should be accessible from the keyboard.
.LI "\f3hg:\f1\0[Recommended]"
.br
Colours should not be hard coded (in other words,
they should not be compiled into the program
and made unchangeable by the user).
.LI "\f3hh:\f1\0[Recommended]"
.br
Graphic attributes such as line, border and shadow should
not be hard coded.
.LI "\f3hi:\f1\0[Recommended]"
.br
Font sizes and styles should not be hard coded.
.LI "\f3hj:\f1\0[Recommended]"
.br
The application should use descriptive names for widgets.
.Ns
Such descriptive names for widgets using graphics instead
of text (for example, palette items and icons) allow screen
reading software to provide descriptive information to blind users.
.Ne
.LI "\f3hk:\f1\0[Recommended]"
.br
Interactions should not depend upon the assumption that a
user will hear an audible notification.
.LI "\f3hl:\f1\0[Recommended]"
.br
Users should be be able to choose to
receive cues as audio or visual information,
where appropriate.
.LI "\f3hm:\f1\0[Recommended]"
.br
The application should not overuse or rely exclusively on
audible information.
.LI "\f3hn:\f1\0[Recommended]"
.br
Users should be able to choose to configure the frequency
and volume of audible cues.
.LI "\f3ho:\f1\0[Recommended]"
.br
Tear-off menus and user configurable menus for key
application features should be provided for users with
language and cognitive disabilities.
.LI "\f3hp:\f1\0[Recommended]"
.br
Application key mappings should not conflict with existing
system level key mappings reserved for access features in
the X Windows server as shown in
the following table.
.LE
.ne 10
.TS
box;
cf3 s
lf3 | lf3
l   | l .
Keyboard Mappings for Server-Level Access Features
_
Keyboard Mapping	Reserved For
_
Five consecutive clicks of Shift key	On/Off for StickyKeys
Shift key held down 8 seconds	On/Off for SlowKeys and RepeatKeys
Six consecutive clicks of Control key	On/Off for screen reader numeric keypad functions
Six consecutive clicks of Alt key	Reserved for future access use
.TE
.eF e
