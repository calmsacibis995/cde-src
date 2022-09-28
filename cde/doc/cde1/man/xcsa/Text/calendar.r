.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Calendar and Appointment Services"
.nr Ej 1
.xR1 calsvcs
.H 2 "Introduction"
.mc 4
The \*(Zx calendar and appointment services allow users to
schedule appointments and browse or update
other users' calendars when trying to
set up group meetings.
The key supported tasks for the calendar and appointment services are:
.BL
.LI
Schedule appointments
.LI
Create ToDo lists
.LI
Schedule repetitive appointments
.LI
Display reminders
.LI
Schedule group appointments
.LI
View a calendar that shows Day, Week, Month or Year time periods
.LI
Multi-browse calendars of multiple users
.LI
Send e-mail with a calendar appointment included to a group
.LI
Print various views of the calendar
.LI
Send e-mail reminders
.LI
Archive calendar data and restore that data
.LE
.mc
.H 2 "Functions" S
The majority of the C-language API to calendar and appointment services
is provided by the functions defined in the \*(Za.
The following functions are required on \*(Zx systems:
.P
.in 6
.Fn csa_add_entry
.br
.Fn csa_delete_calendar
.br
.Fn csa_delete_entry
.br
.Fn csa_free
.br
.Fn csa_list_calendar_attributes
.br
.Fn csa_list_calendars
.br
.Fn csa_list_entries
.br
.Fn csa_list_entry_attributes
.br
.Fn csa_list_entry_sequence
.br
.Fn csa_logoff
.br
.Fn csa_logon
.br
.Fn csa_query_configuration
.br
.Fn csa_read_calendar_attributes
.br
.Fn csa_read_entry_attributes
.br
.Fn csa_read_next_reminder
.br
.Fn csa_register_callback
.br
.Fn csa_restore
.br
.Fn csa_save
.br
.Fn csa_update_calendar_attributes
.br
.Fn csa_update_entry_attributes
.br
.P
.in -6
The following two functions, defined as optional by XAPIA,
may be implemented on \*(Zx systems:
.P
.in 6
.Fn csa_look_up
.br
.Fn csa_free_time_search
.P
.in -6
The remainder of this section defines an additional function
that provides \*(Zx calendar and appointment services
to support application portability at the C-language source level.
.so ../fx/csa_x_process_updates.mm
.ds XC Headers
.SK
.H 2 "Headers" S
The C-language API to calendar and appointment services
is provided by the header defined in the \*(Za.
The following header is required on \*(Zx systems:
.P
.in 6
.mc 6
.Hd csa/csa.h
.br
.mc
.in -6
.H 2 "Command-Line Interfaces" S
This section defines the utilities that provide
\*(Zx calendar and appointment services.
.so ../fx/dtcm_admin.mm
.so ../fx/dtcm_delete.mm
.so ../fx/dtcm_insert.mm
.so ../fx/dtcm_lookup.mm
.ds XC Actions
.SK
.H 2 "Actions" S
This section defines the actions that provide \*(Zx
calendar and appointment services
to support application portability at the C-language source
or shell script levels.
.so ../hx/dtcalendaraction.mm
.ds XC Messages
.SK
.H 2 "Messages"
The \*(Zx calendar and appointment services implement the
.I Display
and
.I Edit
requests for media type _DT_CM_APPOINTMENT.
See the \*(ZT.
.H 2 "Formats"
.so ../fx/dtcm_archive.mm
.so ../fx/dtcm_entry.mm
.ds XC Capabilities
.SK
.H 2 "Capabilities"
A conforming implementation of the \*(Zx calendar and appointment services
supports at least the following capabilities:
.AL
.LI
Provides calendar and appointment services
as described in the following subsections.
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
.H 3 "Calendar Main Window"
The main window of the \*(Zx calendar and appointment services
provides access to all calendar and appointment services
functionality and selects the desired calendar view.
At least the following views are supported:
.VL 12 5
.LI Day
The appointments for a single work day are displayed
within subdivisions.
(The times of a work day can be specified as a user option; see
.cX caloptions .)
Also within the view are three monthly calendars:
the preceding, current and following months
relative to the current Day view.
Choosing a day in this section changes the
current work day of appointments being viewed.
.LI Week
The appointments for a full week are displayed
within subdivisions, each representing one day.
Also within the view is a display that represents
graphically which times during the week are free,
are filled by appointments
and are filled by overlapping appointments.
Invoking the default action on the graphical display section opens the
appointment editor, set to the appropriate day and time.
.LI Month
The appointments for a full month are displayed
within subdivisions, each representing one day.
The display resembles a wall calendar,
with multiple rows of seven boxes,
arranged into weeks.
.LI Year
All of the days of the current year are displayed, by month.
Each month display resembles a wall calendar,
with multiple rows of seven boxes,
arranged into weeks.
Appointments need not be displayed.
Invoking the default action on a month display may open the
appointment editor, set to a day and time within that month.
.LE
.P
For the Day, Week and Month views,
the following rules apply to appointment displays:
.BL
.LI
The appointment text is associated with the start and stop times.
.LI
When viewed by another user,
the appointment text may be suppressed, depending on
privacy options, described later.
.LI
The number of appointments viewable may be limited
by the size of the subdivision.
.LI
Invoking the default action on an appointment subdivision opens the
appointment editor, set to the appropriate day and time.
.LE
.P
Each view has navigation buttons
(in addition to any control available in the window menu)
to change the date range in the view:
arrow buttons and a Today button.
The action of the navigation buttons depends on the current view
(Day, Week, Month or Year).
For example, in the Week view, the user can click on the
arrows on either side of the current day to go backward or forward one
week, and can return to the view of the current week by clicking on the
button that displays the current day.
.P
From any view, the user can enter a specific date
and go directly to the period of time that includes that date.
(For example, in Week view, entering a date will move to
the week that contains that date.)
.mc
.H 3 "Options/Properties"
.xR3 caloptions
The following options or properties can be set
and they persist through each session:
.BL
.LI
The default calendar displayed at startup.
This string may be of the form
.IR user @\c
.IR hostname ,
or another implementation-dependent format.
.LI
The default calendar view displayed at startup.
.LI
The time the work day starts and ends.
.LI
The access list for the user's calendar.
The possible permissions include browse, insert and delete.
Access can be granted on a individual user basis
or for all other users.
The default is that all other users can browse the
calendar, but they do not have insert or delete access to it.
.LI
Defaults for appointments:
reminder methods (including beep, flash, popup and e-mail) and preceding time;
privacy.
.LE
.H 3 "Appointment Editing"
.xR3 appointedit
The appointment editor may be a separate dialog box.
The system need not support more than one appointment editor
open at any one time.
.P
The user can use the appointment editor to accomplish the following:
.AL
.LI
Add new appointments.
Appointments have at least the following attributes:
.AL a
.LI
Date.
.LI
Start and stop times, both within that date.
.LI
Descriptive text, of at least four lines.
This text may be truncated in some calendar views.
.LI
Reminders.
The user can select one or more of at least the
following reminder methods, each with an amount of time
specified in advance of the appointment:
sounding an audible alarm,
if supported by the hardware; flashing the calendar icon and window;
displaying a message dialog with the information concerning the
appointment; sending an e-mail.
.LI
Frequency.
The user can schedule repetitive appointments
by selecting the number of repetitions and the frequency:
.BL
.LI
once (default)
.LI
daily
.LI
weekly on the same day
.LI
every two weeks on the same day
.LI
monthly on the same date
.LI
monthly by weekday
.LI
annually on the same date
.LI
daily on Monday to Friday only
.LI
every Monday, Wednesday and Friday
.LI
every Tuesday and Thursday
.LI
every
.I n
days, weeks or months, where
.I n
is an integer specified by the user
.LE
.LI
Privacy.
The user can choose to suppress the descriptive text,
or all knowledge of this appointment, from other users who
have browse access to the calendar.
.LE
.LI
Display a list of all appointments for the day.
.LI
Delete existing appointments.
.LI
Edit attributes of existing appointments.
.LE
.H 3 "Appointment Listing"
The appointment lister may be a separate dialog box.
The system need not support more than one appointment lister
open at any one time.
.P
The appointment lister lists of appointments
for the calendar view at the time the list is opened;
for example, in Week view,
the list displays all appointments for the current week.
Changing the calendar view may change an open
appointment list.
.P
The list includes at least the following for each appointment:
date; start time; the first line of the appointment description.
Appointments are sorted in ascending order by start date and time.
.P
Invoking the default action on an appointment in the list opens the
appointment editor, with that appointment selected for editing.
.H 3 "Appointment Finding"
The \*(Zx calendar and appointment services
can search for appointments meeting user-specified criteria:
.AL
.LI
The user can specify a string on which to search.
The search is independent of case.
.LI
The user can limit the search to dates within a specified range.
.LE
.P
All matching appointments are presented to the user.
Selecting one of the matches changes the
main window view to include the date of that appointment.
.H 3 "To-Do Editing"
The to-do editor may be a separate dialog box.
The system need not support more than one to-do editor
open at any one time.
.P
The user can use the to-do editor to accomplish the following:
.AL
.LI
Add new to-do tasks.
To-do tasks have at least the following attributes:
.AL a
.LI
Optional due date.
.LI
Descriptive text, of at least four lines.
This text may be truncated in some calendar views.
.LI
Status:
completed, pending or overdue.
Pending or overdue are in relation to the due date.
The user can change a pending or overdue task
to completed, or remove the completed status.
.LI
Reminders.
This attribute is the same as for an appointment,
except that only e-mail reminders need to be supported.
.LI
Frequency.
This attribute is the same as for an appointment.
.LE
.LI
Delete existing to-do tasks.
(The task is selected from the to-do list
described in the next section.)
.LI
Edit attributes of existing to-do tasks.
(The task is selected from the to-do list
described in the next section.)
.LE
.H 3 "To-Do Listing"
The to-do lister may be a separate dialog box.
The system need not support more than one to-do lister
open at any one time.
.P
The user can limit the displayed list of to-do tasks to only completed tasks
or to pending and overdue tasks.
.P
The user can limit the displayed list of to-do tasks to those tasks due
in the current day, week, month or year.
To-do tasks that do not have a due date appear in all views.
.P
The displayed list includes at least the following for each to-do task:
status; due date; the first line of the task description.
.P
Invoking the default action on a task in the list opens the
to-do editor, with that task selected for editing.
.H 3 "Multi-User Calendar Accessing"
.xR3 multical
The user can identify calendars (such as with the
.IR user @\c
.I hostname
notation)
to be in a list of frequently accessed calendars.
The user can add and delete calendars from the list.
The list contents persist through each session.
.P
The user can access some or all of the calendars
from those in the frequently accessed calendar list,
to identify common free times.
A graphical display depicts time periods
in which conflicts occur and the user can determine
which calendars are busy for those periods of time.
The user can access a \*(Zx mail services Compose window (see
.cX mailsvcs )
that is pre-addressed
to all users whose calendars are selected.
.P
The user can schedule an appointment on all accessible calendars
from the preceding step and optionally e-mail the
appointment to those not accessible.
The user can access a \*(Zx mail services Compose window (see
.cX mailsvcs )
that is pre-addressed
to all users whose calendars are selected and the text
of the mail message contains the new or edited appointment.
.H 3 "Drag and Drop Capabilities"
The \*(Zx calendar and appointment services
provide drag and drop capabilities as follows:
.AL
.LI
The user can drag a calendar appointment from an icon in the
appointment editor or from any list of appointments
to any drop site registered to accept buffer drops.
.LI
The user can drop a buffer containing an appointment
onto the main calendar window,
causing the appointment to be scheduled.
.LE
.P
The format of the appointment to be dragged and dropped
is described in
.cX calapptfmt .
.H 3 "Printing"
The following printing capabilities are supported:
.AL
.LI
From any view in the main window,
the user can print a version of that view.
.LI
From any view in the main window,
the user can print a version of any other
view, without changing the view being displayed.
.LI
The user can print any main view, the appointment list
or to-do list and specify
a range of times, to be printed as if individual view-printing
requests had been made.
For example, in Week view, two or more weeks can be printed in
a single request.
.LE
.H 3 "Other Capabilities"
The following additional capabilities are supported:
.AL
.LI
The user can change the calendar display to that of
another user, allowing direct browsing or updating (given
appropriate access permissions).
Each calendar identified in
the list of frequently accessed calendars,
described in
.cX multical ,
is available directly as a menu item.
The user can also change to an individual calendar
without adding it to the list.
.LI
The user can change the view of appointments and to-do tasks
so that they appear to be in a specified time zone other
than the local one.
.LE
.H 2 "Rationale and Open Issues" S
.xR 2 calendarrat
.I
The following is text copied from the
.R
CDE X/Open Specification Framework Proposal, Draft 2.
.I
It is not intended to remain in this specification when it is published
by X/Open.
.R
.P
.mc 3
The calendar APIs are required for application portability.
ToolTalk message protocols are required for application portability.
.mc 1
Actions are required for application portability.
The
.mc 4
.Cm dtcm_admin ,
.Cm dtcm_lookup ,
.Cm dtcm_insert ,
and
.Cm dtcm_delete
CLIs are required for application interfaces from a non-GUI environment.
.mc 1
The
.Cm dtcm
and
.Cm dtcm_editor
CLIs are not planned for standardisation because Actions
are superior interfaces for these services.
Capabilities are required to ensure that conforming
.mc
implementations provide the minimum expected services, without overly
constraining tool design.
.P
.mc 4
The exact format of the attribute names in the Formats
sections is subject to change before Draft 6.
The XAPIA is in the process of registering the names
using ISO 9070 procedures and there may be minor modifications required.
.P
.mc 3
The protocols for calendar daemon communications across
systems will be omitted from the \*(Zx 1.0 standard.
Although these are required for true open system interoperability,
the CDE sponsors cannot agree on publishing a protocol based on ONC RPCs.
The statement in Draft 2 that they would be provided was in error.
.br
.mc
.eF e
