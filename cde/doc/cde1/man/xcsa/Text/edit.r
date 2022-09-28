.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Text Editing Services"
.nr Ej 1
.xR1 texteditsvcs
.H 2 "Introduction"
.mc 5
The \*(Zx text editing services provide a tablet to create and edit short
documents such as memos and mail messages in the character set of the current
locale.
.br
.mc
.H 2 "Widgets" S
This section defines the widget classes
that provide \*(Zx text editing services
to support application portability at the C-language source level.
.so ../fx/DtEditor.mm
.ds XC Functions
.SK
.H 2 "Functions"
.xR2 editfuncs
This section defines the functions, macros and
external variables that provide \*(Zx text editing services
to support application portability at the C-language source level.
.so ../fx/DtCreateEditor.mm
.so ../fx/DtEditorAppend.mm
.so ../fx/DtEditorAppendFromFile.mm
.so ../fx/DtEditorChange.mm
.so ../fx/DtEditorCheckForUnsavedChanges.mm
.so ../fx/DtEditorClearSelection.mm
.so ../fx/DtEditorCopyToClipboard.mm
.so ../fx/DtEditorCutToClipboard.mm
.so ../fx/DtEditorDeleteSelection.mm
.so ../fx/DtEditorDeselect.mm
.so ../fx/DtEditorFind.mm
.so ../fx/DtEditorFormat.mm
.so ../fx/DtEditorGetContents.mm
.so ../fx/DtEditorGetInsertionPosition.mm
.so ../fx/DtEditorGetLastPosition.mm
.so ../fx/DtEditorGetSizeHints.mm
.so ../fx/DtEditorGoToLine.mm
.so ../fx/DtEditorInsert.mm
.so ../fx/DtEditorInsertFromFile.mm
.so ../fx/DtEditorInvokeFindChangeDialog.mm
.so ../fx/DtEditorInvokeFormatDialog.mm
.so ../fx/DtEditorPasteFromClipboard.mm
.so ../fx/DtEditorReplace.mm
.so ../fx/DtEditorReplaceFromFile.mm
.so ../fx/DtEditorReset.mm
.so ../fx/DtEditorSaveContentsToFile.mm
.so ../fx/DtEditorSelectAll.mm
.so ../fx/DtEditorSetContents.mm
.so ../fx/DtEditorSetContentsFromFile.mm
.so ../fx/DtEditorSetInsertionPosition.mm
.so ../fx/DtEditorTraverseToEditor.mm
.so ../fx/DtEditorUndoEdit.mm
.ds XC Headers
.SK
.H 2 "Headers"
This section describes the contents of headers used
by the \*(Zx text editing service functions,
macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in
.cX editfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/DtEditor.mm
.ds XC Command-Line Interfaces
.SK
.H 2 "Command-Line Interfaces"
This section defines the utility that provides
\*(Zx text editing services.
.so ../fx/dtpad.mm
.ds XC Actions
.SK
.H 2 "Actions"
This section defines the actions that provide \*(Zx text editing
services to support application portability at the C-language source
or shell script levels.
.so ../hx/dttextaction.mm
.ds XC Messages
.SK
.H 2 "Messages"
The \*(Zx text editing services support the ToolTalk
Desktop and Media Exchange messages listed below for the
following media types:
.VL 11 5
.LI C_STRING
.br
Text in an arbitrary codeset
.LI _DT_DATA
.br
Data that does not match any other data type
.LE
.P
In addition, the Text Editor supports the messages below
for any media type that does not have a specific editor registered.
.P
The following messages are supported from the \*(ZT:
.VL 11 5
.LI \f2Instantiate\fP
.br
Opens a new edit window for composing arbitrary file(s).
.LI \f2Edit\fP
Opens a new edit window for editing an existing file or
buffer or for composing a specific new file or buffer.
.LI \f2Display\fP
.br
Opens a new edit window for displaying an existing file or buffer.
.LE
.P
The following messages are supported from the \*(Zv:
.VL 11 5
.LI \f2Quit\fP
Terminates the text editing services or closes
a specific Text Editor edit window as specified by the
.I operation2Quit
argument.
The
.I operation2Quit
argument must be the message ID of the
.mc 6
Media Exchange request that created the edit window.
.P
The default actions for notifying the user, saving or
returning text and closing edit windows are:
.BL
.LI
If
.I operation2Quit
is specified, the specified edit window is closed;
otherwise, all edit window(s) are closed and the text
editing services are terminated
.LI
If there are unsaved changes, the user is notified and
allowed to save the text and/or abort the
.IR Quit ;
otherwise, the user is not notified and the text is not
saved (or returned if a buffer is being edited)
.LE
.P
Both the
.I silent
and
.I force
arguments are supported.
However, the semantics of
.I silent
differ from those described in \*(Zv in
that the text editing services provides user notification
only when there are unsaved changes, rather than user
notification when an edit window is terminated.
The following table describes variances in the default action
for various combination of
.I silent
and
.IR force .
.DS "" F
.TS
center, box, tab(@);
lf4 | lf4 | lf4
lf3 | lf3 | lw(4i) .
silent@force@action
_
False@False@\f2default\fP
True@False@T{
.na
If there are unsaved changes, the user is
not notified, the text is not saved and the
edit window is not terminated.
T}
False@True@T{
.na
If there are unsaved changes, the user is
still notified and allowed to save the text,
but cannot abort the
.IR Quit .
T}
True@True@T{
.na
If there are unsaved changes, the user is
not notified, the text is not saved and the
edit window is closed.
T}
.TE
.DE
.P
Whenever the
.I Quit
request is not carried out (i.e., in the default case when
the user explicitly aborts the
.I Quit
or when
.I silent
is True and
.I force
is not specified or is False), the
.I Quit
request is failed with
.Cn TT_DESKTOP_ECANCELED .
.LI \f2Save\fP
.mc
Saves a specific edit window opened via an
.I Edit
request.
The ID argument must have the
.B messageID
vtype and have the value of the message ID of the
.I Edit
request that created the edit window.
.LI \f2Saved\fP
.br
Sent when a file has been saved, as the result of a
.I Save
request or a user action.
.LE
.H 2 "Capabilities" S
.xR2 editcaplist
A conforming implementation of the \*(Zx text editing services
supports at least the following capabilities:
.AL
.LI
Provides text editing services
as described in the following subsections.
.LI
Conforms to the Required items in the Application Style Checklist in
.cX checklist .
.LI
Has been internationalised using the standard interfaces
in the \*(ZK, the \*(Zl, and the \*(ZM,
and supports any locale available in the underlying X Window System
Xlib implementation.
.LE
.H 3 "File Management"
The following capabilities are supported for managing files:
.AL
.LI
The user can create a new document.
.LI
The user can open the document contained in an existing text file.
.LI
The user can save the document to a new file.
.LI
The user can save the document to the original text file.
.LI
The user can include an existing text file in a document.
The contents of an included files is
inserted into the current document at the location of the
insertion cursor.
.LI
The user can drop file icons, buffer icons and selected
text on the edit area.
Such a drop is equivalent to an include
of an existing file, but no file selection dialog is posted.
.LI
The user can drag selected text out of the edit area.
.LE
.H 3 "Presentation"
The following capabilities are supported for presenting the text:
.AL
.LI
The editing area provides scroll bars to see text not visible in the window.
.LI
The user can find and optionally replace text in the document.
.LI
The user can print the current document.
.LI
The user can select any the following types of formatting for a single
paragraph and for the whole document:
.AL a
.LI
Align along the left margin
.LI
Align along the right margin
.LI
Align along both margins
.LI
Center each line
.LE
.LI
The user can set both left and right margins.
.LI
Formatting supports paragraph indents.
The difference
between the positions of the left-most characters of the
first and second lines of a paragraph is the indent and
is preserved for all lines of the paragraph.
.LE
.H 3 "Text Editing"
The following capabilities are supported for editing text:
.AL
.LI
The user can cut text to the clipboard.
.LI
The user can copy text to the clipboard.
.LI
The user can paste text from the clipboard into the document.
.LI
The user can delete text from the document.
.LI
The user can replace text in the document with spaces.
This is also called ``clearing.''
.LI
The user can undo the most recent edit operation.
.LI
The user can select all text in the document.
.LI
The user can select either insert mode or replace mode for text entry.
.LI
The user can copy and move text through drag and drop operations.
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
CLIs and Actions are required for application portability.
ToolTalk message protocols are required for application portability.
Capabilities are required to ensure that conforming implementations
provide the minimum expected services, without overly constraining tool
design.
.P
The API is in the process of being redesigned, so a detailed function
list is not available.
.eF e
