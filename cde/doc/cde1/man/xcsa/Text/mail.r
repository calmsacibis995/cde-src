.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Mail Services"
.xR1 mailsvcs
.H 2 "Introduction"
.mc 5
The \*(Zx mail services provide a GUI for manipulating
electronic mail messages that may have attachments.
Users can use the interface to perform activities such as:
compose a message,
.mc 6
view the contents of a message,
view the list of messages in a mailbox,
copy or move messages from one mailbox to
.mc 5
another, delete messages, reply to messages, add/delete
attachments to a message, and view contents of attachments
in a message.
The key supported tasks for the mail services are:
.BL
.LI
Receive and view mail messages and their attachments
.LI
Compose, reply, forward, reply-include and send messages
.LI
Include files and add attachments to outgoing messages
.LI
File incoming messages in different mailboxes
.LI
Create, open and close mailboxes
.LI
Move or copy messages from one mailbox to another
.LI
Delete and undelete messages
.LI
Print messages
.LI
Sort or find messages based on prescribed criteria
.LI
Provide a public ToolTalk API to support
a mail-pervasive desktop environment
.LE
.mc
.H 2 "Actions" S
This section defines the actions that provide \*(Zx mail
services to support application portability at the C-language source
or shell script levels.
.so ../hx/dtmailaction.mm
.ds XC Messages
.SK
.H 2 "Messages"
The mail services implement the
.I Display
and
.I Mail
media messages for media type RFC_822_MESSAGE.
See the \*(ZT.
.P
When a
.I Display
message is received, the mail services display the mailbox
specified by the
.I Display
message's
.I file
attribute.
.P
When a
.IR Mail (TT_IN)
message is received,
the mail services attempt to send the given data.
.P
When a
.IR Mail (TT_OUT)
message is received,
the mail services open an empty Compose window.
.P
When a
.IR Mail (TT_INOUT)
message is received,
the mail services parse the given
data and open a Compose window with the data filled into
the appropriate areas (header fields, text and attachment areas).
.P
For
.IR Mail (TT_IN)
and
.IR Mail (TT_INOUT)
messages, the data must be a fully
constructed mail message in the format described in
.cX mailfmts .
.mc 6
If the data has attachments,
it must be in the format described in the \*(Zm.
.br
.mc
.H 2 "Formats"
.xR2 mailfmts
The \*(Zx mail services transmit and receive mail messages
formatted in accordance with the \*(Zh;
some or all of the following header fields, as defined in the \*(Zm,
are also included in each message:
.Cs I
Content-Description
Content-ID
Content-Transfer-Encoding
Content-Type
Mime-Version
.Ce
.P
The \*(Zx mail services read and write mailboxes in the
format described in the
.Cm mailx
command in the \*(ZC,
with each mail message formatted as described previously,
except that an additional header line is included in each message:
.Cs I
Content-Length: \f2n\fP
.Ce
.P
where
.I n
is an integer representing the number of octets in the body
of the message (in other words, those octets that occur after
the empty line separating the message header from the message body).
When a mailbox is being read, the Content-Length
fields of the messages are verified.
If the content length points exactly to the beginning of a
.B From
line that denotes the start of a new mail message,
or if it points to the end of the mailbox file,
the Content-Length value is honoured;
otherwise, the mail services scan the mailbox for
the next message and correct the value of the
Content-Length header when the mailbox is later written.
.P
When messages are saved in a mailbox file, the
\*(Zx mail services may write additional header lines
into each mail message stored in the file.
.H 2 "Capabilities"
A conforming implementation of the \*(Zx mail services
supports at least the following capabilities:
.AL
.LI
Provides mail services
as described in the following subsections.
.LI
Formats mail messages and mailbox files as described in
.cX mailfmts .
.LI
Conforms to the Required items in the Application Style Checklist in
.cX checklist .
.LI
Has been internationalised using the standard interfaces
in the \*(ZK, the \*(Zl, and the \*(ZM,
and supports any locale available in the underlying X Window System
Xlib implementation.
.LE
.H 3 "Managing Mailboxes"
.xR3 mailmgmt .
Mailboxes are text files formatted as described in
.cX mailfmts .
The user has a mailbox referred to as
the Inbox where incoming mail is received.
The user can also have additional mailboxes.
(Unless otherwise noted, the term
.I mailbox
applies to both the Inbox and any additional mailbox files.)
The following capabilities are supported for managing mailboxes:
.AL
.LI
The user can open the Inbox and any additional mailboxes
for which he or she has read permission.
Multiple mailboxes can be open simultaneously.
Mailboxes for which the user has read permission,
but does not have write permission,
are opened as read-only mailboxes,
which prevents deletion of messages.
.LI
The user can create new, empty mailboxes.
.LI
The user can close mailboxes.
.LI
The user can set the method by which the
mail services update a mailbox's list of mail messages
to account for newly received, moved or copied messages.
This can be a recurring time period as well as an immediate one-time update.
.LI
The user can select audible alarms or flashing windows
as a means of announcing that the mailbox has been updated.
.LI
The user can specify a default directory for mailboxes
(other than the Inbox).
.LE
.H 3 "Managing Message Lists"
The following capabilities are supported for viewing and manipulating
the message list associated with a mailbox:
.AL
.LI
When a mailbox is open, it presents a list of its constituent mail messages.
The list contains at least the following for each message:
.AL a
.LI
Name or login name of sender
.LI
Subject (which may be empty)
.LI
Date and time the message was sent,
if that information is available in the message,
expressed in the time zone of the local system
.LI
Size of the message in octets;
messages larger than 1\|024 octets may be expressed in
terms of ``K'' (1\|024) octets
.LI
Status of the message (new or already read)
.LI
Whether there are attachments to the message
.LE
.LI
The user can delete messages from the list.
The deleted message is placed on a list of deleted message
and is not physically removed from the mailbox at that time.
.LI
The user can restore deleted messages by selecting them
from the deleted message list.
.LI
The user can physically remove all deleted messages
from the mailbox by destroying the deleted message list.
This destruction can occur when the user chooses
a control to do such and it can be set up to occur
automatically when the mailbox is closed.
The user may be offered the choice of removing
individual mail messages from the deletion list
based on the age of the message or other user-selected criteria.
.LI
The user can sort the list of messages in any of the following sequences:
.AL a
.LI
Date and time of sending \(em sorted in date/time sequence
.LI
Name or login name of sender \(em sorted in case-sensitive character sequence
.LI
Subject \(em sorted in case-sensitive character sequence,
with replies to messages sorted as if the leading characters
``Re: '' were not present
.LI
Size \(em sorted in size sequence
.LI
Status \(em sorted in an unspecified sequence
.LE
.LI
The user can search for messages in the list,
selecting either the next message meeting the
criteria or all of the messages doing so,
at the user's option.
The search matches substrings of any or all
of the following header values, in a case-insensitive manner:
To, From, Subject, Cc.
.LI
The user can select one or more messages and copy them
to another mailbox.
All of a message, including attachments, is copied.
.LI
The user can select one or more messages and move them
to another mailbox.
The movement is equivalent to a copy operation
followed by a delete operation.
.LI
The user can select one or more messages and print them.
The data transmitted for printing of each message includes
the text and mail header of the message.
The user may be able to choose to print attachments
along with the text and header.
The method used to print the text is based on the
.B Print
action for the DTMAIL_FILE file type; see the \*(Zz.
.LE
.H 3 "Viewing and Manipulating Existing Messages"
The following capabilities are supported for
viewing and manipulating existing messages in a mailbox:
.AL
.LI
The user can display a message.
The user can choose to display additional
messages by either replacing the current
display with the new message or opening additional display
windows so that multiple messages are displayed at once.
.LI
The user can display the text of the message.
.LI
The user can display the mail header of the message
and can choose to see all of the header lines
or only a subset.
.LI
The user can suppress the display of header fields by name.
.LI
The user can save the text of the message,
including the mail header
.mc 6
lines that are chosen for display, as a text file,
creating a new file or replacing an existing one.
.mc
.LI
The message attachments are displayed as icons.
The icon selected for an attachment is based on
the data typing information described in the \*(Zy.
.LI
.mc 6
The user can save the data from an attachment as a file,
creating a new file or replacing an existing one.
.LI
The user can access a menu with Actions for a selected attachment.
Actions are invoked for an attachment by selecting an attachment
and then selecting an action from the menu.
If the ACTIONS attribute is defined for an attachment's data type,
invoking the default action on an attachment activates the
first Action listed in the Action attribute.
.LE
.mc
.H 3 "Composing New Messages"
The following capabilities are supported for composing new messages:
.AL
.LI
The user can compose a message that includes
both text and attachments and can specify and edit the
To addressees, the Cc addressees, the Bcc (blind Cc) addressees
and the Subject header field.
.LI
The user can compose multiple messages simultaneously.
.LI
The user can send messages to a file by including the
file name as one of the To, Cc or Bcc addressees.
The file name must be expressed as an absolute pathname
or using a notation of ``+\c
.IR file ,''
which selects the file named
.I file
in the default mailbox directory (see
.cX mailmgmt ).
.LI
The user can send messages to a mail alias,
which represents one or more people or files,
by including the alias name
as one of the To, Cc or Bcc addressees.
.LI
The user can compose a new message starting with an empty
body, Subject and addressees.
.LI
The user can select one or more messages from the
mailbox message list and compose a new message
that includes the text of these messages.
The new message includes all of the attachments
of the selected messages.
.LI
The user can select one or more messages from the
mailbox message list and compose a forwarded message
that includes the text and all the attachments from these messages.
A forwarded message differs from a new message
only in that the Subject header is pre-entered for the user,
based on the Subject of one of the selected messages,
and the text of the selected messages is identified
as being forwarded.
.LI
The user can select one or more messages from the
mailbox message list and compose a reply message
that includes the text of these messages.
If multiple messages are selected,
multiple reply compositions are set up,
so that each can be processed simultaneously
and each has the correct addressees.
The following variations on replies are supported:
.AL a
.LI
A Reply to Sender starts with an empty body,
but pre-enters the To and Subject headers for the user,
based on the sender and Subject of the original message.
The reply contains no attachments unless
the user takes explicit action to include them.
.LI
A Reply to All is the same as a Reply to Sender,
but all of the To and Cc addressees of the original message
are pre-entered as To and Cc addressees, as appropriate, on the reply.
The reply contains no attachments unless
the user takes explicit action to include them.
.LI
A Reply to Sender with Include is the same as a Reply to Sender,
but the message body starts with the text of the original message.
The user can choose to indent each line of the text with a
string of characters (such as ``>\0'').
The reply includes all attachments from the original message.
.LI
A Reply to All with Include is the same as a Reply to All,
but the message body starts with the text of the original message.
The user can choose to indent each line of the text with a
string of characters (such as ``>\0'').
The reply includes all attachments from the original message.
.LE
.LI
The user can include a text file at the insertion point
of the message being composed.
.LI
The user can write the header and text of the
message being composed to a text file,
creating a new file or replacing an existing one.
.LI
The user can manipulate the contents of the message text
using the facilities described in
.cX editcaplist :
word wrapping, finding and replacing text, and spell checking.
.LI
The user can select a mailbox into which all outgoing messages are saved.
.LI
The user can select a directory into which
the mail services will attempt to save partial messages
in case of interrupts or delivery errors.
.LI
The user can affect the attachments of mail as it is being composed:
.AL a
.LI
Add attachments
.LI
Delete and undelete attachments
.LI
Save the attachments as files
.LI
Rename the attachments (which affects the name in the message,
not the name of the file being attached)
.LE
.LE
.H 3 "Drag and Drop Capabilities"
The \*(Zx mail services
provide drag and drop capabilities as follows:
.AL
.LI
The user can drag the selected messages from the mailbox message list
to any drop site registered to accept buffer drops.
This achieves the following results:
.AL a
.LI
If dropped onto another mailbox message list,
the messages are moved into the target mailbox.
.LI
If dropped onto the text area of a message being composed,
the texts of the messages are inserted into the new message.
.LI
If dropped onto the attachments area of a message being composed,
the messages are attached as a mailbox file.
.LI
If dropped onto another application,
the data should be treated as either a file or a collection
of individual mail messages, as appropriate.
.LE
.LI
The user can drag text from the text
area of a message being composed or viewed
to any drop site registered to accept text drops.
This achieves the following results:
.AL a
.LI
If dropped onto the text area of a message being composed,
the text is inserted into the new message.
.LI
If dropped onto the attachments area of a message being composed,
the text is attached as a text attachment.
.LI
If dropped onto another application,
the data should be treated as text.
.LE
.LI
The user can drag an attachment from the attachment
area of a message being composed or viewed
to any drop site registered to accept buffer drops.
This achieves the following results:
.AL a
.LI
If dropped onto the text area of a message being composed,
text in the attachment is inserted into the new message.
.LI
If dropped onto the attachments area of a message being composed,
the attachment is copied into the attachments area of the new message.
.LI
If dropped onto another application,
the data should be treated in accordance with the type of data.
.LE
.LI
The user can drop files, text or data buffers into the attachment area
of a message being composed, which causes new attachments
to be added to the message.
.LE
.H 3 "Other Capabilities"
The following other capabilities are supported:
.AL
.LI
The user can maintain (display, add entries, delete entries,
change entries) a list of personal mail aliases.
.LI
The user can set up the mail services so that
any incoming message is replied to automatically.
The message is stored in the Inbox as usual.
The user can specify the text and Subject of the
reply message.
The mail services may limit the number of these
automatic replies sent to the same mail address.
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
.mc 4
The MIME format is required for interoperability.
ToolTalk message protocols are required for application portability.
.mc
An API is not provided because the interface between
.B libDtMail
and the application or
.Cm dtmail
has not stabilised.
.mc 5
It was originally intended that the system inbox name and locking
protocols would be documented so that ISVs could provide
fully portable replacement mail tools.
However, it turns out that this differs significantly
across operating systems and consensus could not be reached
on common (new) interfaces.
Furthermore, \*(Zx is an inappropriate place to standardise
such interfaces.
.P
Actions are required for application portability.
The
.Cm dtmail
CLI is not planned for standardisation because Actions
are superior interfaces for this service.
.mc 4
In earlier drafts, a
.Cm dtmailconv
utility was identified.
This has been removed because it was intended to
convert mailboxes to and from the Bento format,
which is no longer included in CDE 1.0.
Capabilities are required to ensure that
.mc
conforming implementations provide the minimum expected services,
without overly constraining tool design.
.eF e
