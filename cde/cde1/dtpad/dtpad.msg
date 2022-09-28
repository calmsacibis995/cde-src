$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dtpad.msg
$ **
$ **  Project:     DT Text Editor client (sometimes called dtpad)
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dtpad
$ **
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1992, 1993 Hewlett-Packard Company
$ **     All Rights reserved
$ **
$ **
$ *****************************************************************************
$ **
$ *****************************************************************************
$ **  (c) Copyright 1993, 1994 Hewlett-Packard Company
$ **  (c) Copyright 1993, 1994 International Business Machines Corp.
$ **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
$ **  (c) Copyright 1993, 1994 Novell, Inc.
$ *****************************************************************************
$ **

$set 2
$ **********************************************
$ **  General messages used in multiple files **
$ **********************************************

$ Label used for all "Help" buttons in all DIALOGS.
$ It is not used for the Help menu.
1 Help
$ Unrecognized arguments
2 dtpad: illegal option: %s 
$ Extra arguments
3 dtpad: extra argument: %s
$ Invalid options
4 Usage: dtpad [-options] [file]

$set 4
$ *****************************************************************
$ **                                                    fileCB.c **
$ ** These are the strings which change between the Open dialog  **
$ ** and the Include dialog.  These two operations use the same  **
$ ** Motif dialog, and simply change these two labels.           **
$ *****************************************************************

$ Title of the Open dialog
1 Open a File

$ label for the File text field in the file selection box
$ ***UNUSED***	This label is no longer used and need not be localized.
2 File to Open:

$ Title of the Include dialog
3 Include a File

$ Label for the File text field in the file selection box
$ ***UNUSED***	This label is no longer used and need not be localized.
4 File to Include:

$ Error when printing a file and a temporary copy can not be saved.
5 Unable to create a temporary file.


$set 5
$ ***************************************************************************
$ **                                                               dtpad.c **
$ **                                                             fileDlg.c **
$ **                                                                       **
$ ** Messages and strings used in dialogs for all operations involving the **
$ ** "File" menu items.                                                    **
$ ***************************************************************************
$ These compose the dialog which appears if the user attempts to exit, open
$ a different file, drop a file, or start a "New" session with unsaved
$ changes to the current file.

$ This is used to build: "Save changes to FILENAME?"
1 Save changes to %s?

$ This is used as-is, whenever the current file is unnamed.
2 The current file has no name.\nDo you want to save it?

$ This is used to build a message like "Do you want to save the current
$ copy of FILENAME before you Edit NEWFILENAME?" when a new file is droped
$ on the edit window.
$ ***UNUSED***	Currently, dropped files are INSERTED in the text rather than
$		opened for editing so this message is no longer used and need
$		not be localized.
3 Do you want to save the current\ncopy of %s\nbefore you Edit %s?

$ Used when about to write over an existing file.
$ The "%s" is replaced by the file name.
5 The file %s already exists.\nDo you want to over write that file\nwith the contents within this edit session?

$ Title of "file already exists" dialog.  Also used for other "Warning" type
$ dialogs.
6 Warning

$ Title of "Save As" dialog.
7 Save As

$ Labels for add/don't-add newlines radio buttons in the Save As dialog
$ ***UNUSED***	These labels are no longer used and need not be localized.
$		They've been replaced by messages 44 and 45
8 Add newline characters to the end of word-wrapped lines.
9 Do not add newlines.  Only line breaks created by [Return]\nwill be preserved.

$ Labels for the Save Warning dialog. Also used in the confirmation, or
$ Editor Return dialog.
14 No
15 Yes
$ Title of the dialog - used when user selects "New", "Open", or "Exit" with
$ unsaved changes in the text.
16 Save?

$ Title of general error dialog
17 Error

$ Title of the resource-specified "confirmation" dialog
$ This dialog is generally specified by programs such as mailers, which
$ want to know if the user wants to send the message.
$ There is no default message for the dialog, since it is _only_ posted if
$ the message string is specified via a resource (dtpad*confirmationString:)
$ This message is only used if the "dtpad*mainTitle:" resource is not set.
$ If this resource _is_ set, the the mainTitle is also used as the title
$ for the exit confirmation dialog.
$ DO NOT LOCALIZE - the user should never see this message
18 Exit Confirmation

$ Title for general information dialog. e.g. Find dialog can't find word in
$ file.
20 Information

$ Default file name in Save As dialog box.
21 UNTITLED

$ Error message when a file can not be saved.  The "%1$s" gets replaced by
$ the file name and the "%2$s" gets replaced by the reason why the file could
$ not be saved (messages 31-39).
30 Unable to save %1$s.\n%2$s

$ Messages 31-39 are reasons why a file could not be saved.
$ Messages 34-39 are substitued in message 30 for "%2$s".
31 Unable to save file.\nFile name was not specified.

34 File does not allow writing by anyone.

35 File is a directory.

36 File is a character special device.

37 File is a block mode device.

$ ***UNUSED***	This message should never appear since currently there is
$		only one supported file format - text.  And this type is
$		hard coded into dtpad.
38 Invalid file format.

$ This is a "catch all" when the file can not be saved and none of the above
$ errors apply.
39 Unknown reason - possibly not enough disk space.

$ This is displayed when a ToolTalk edit request on a buffer is received,
$ a Save operation is done and there are unsaved changes.
40 Save changes?

$ This is displayed when a ToolTalk edit request on a file or buffer is
$ received with a document name specified in the request, a Save operation
$ is done and there are unsaved changes.  The "%s" in the message is
$ replaced by the document name (instead of the filename).
41 Save changes to "%s"?

$ Title of "Copy As" dialog which is substitued for the "Save As" dialog
$ when file name changes are disallowd.  This should match the menu button
$ label specified in message 108 of set 11.
$ ***UNUSED***	This title is no longer used and need not be localized.
42 Copy As

$ Title of "Copy To File" dialog which is substitued for the "Save As" dialog
$ when file name changes are disallowd.  This should match the menu button
$ label specified in message 110 of set 11.
43 Copy To File

$ Labels for add/don't-add newlines radio buttons in the SaveAs and Save dialogs
44 Add newline characters to the end of wrap-to-fit lines.
45 Do not add newlines.  Only line breaks created by [Return]\nwill be preserved.

$ Show hidden files and folders
46 Show hidden folders and files

$set 6
$ ************************************************************************
$ **                                                           fileIo.c **
$ **                                                                    **
$ ** Error messages generated when there are problems reading a file.   **
$ ************************************************************************

1 This file has been opened for reading only.
2 This file does not exist.
3 Unable to edit a Directory.
4 Unable to edit a character special device file.
5 Unable to edit a block mode device file.
6 Embedded Nulls stripped from file.
7 Unable to load file (insufficient memory).
8 Unable to access file (can't determine it's status).
9 Unable to read from file (probably no read permission).

$ Message placed in the status line message area based on the -viewOnly
$ resource and whether or not the file being edited is read only
10 View Only
11 Read Only
12 Unable to read from the file (unknown reason).


$set 7
$ ************************************
$ **                         main.c **
$ **                        dtpad.c **
$ ** Prefix string for all dialogs. **
$ ************************************
$ bogus?? blank string gotten but otherwise unused in main()
$ DO NOT LOCALIZE string 1
1 

$ The name used in the title bar of every window.
$ Be sure to end this message with a space character.
2 Text Editor - 

$ Default file name used in the main window title.
$ ***UNUSED***	This mesage has been replaced by message 6 of this set and
$		need not be localized.
3 (unnamed)

$ Terminal error message displayed to stdout when the desk top services
$ can not be initialized (via DtInitialize())
4 Unable to initialize Desktop services ... exiting Text Editor.

$ Terminal error message displayed to stdout when a connection to the
$ display can not be opend (via XtDisplay())
5 Can not open display ... exiting Text Editor.

$ Default file name used in the main window title.
$ This should correspond to the default file name used in the "Save As"
$ dialog box - specified in message 21 of set 5.
6 (UNTITLED)

$set 10
$ ************************************************************************
$ **                                                          session.c **
$ ** THESE MESSAGES ARE _ONLY_ WRITTEN TO THE ERRORLOG FILE.
$ **
$ ************************************************************************
$ Strings used to build error messages issued when the session save
$ file cannot be read or written.
1 Check the file permissions.
2  %s is the file that would have been used to save your session.


$set 11
$ ***********************************************************
$ **                                              dtpad.c **
$ **                                                       **
$ ** These are all of the menu button labels and mnemonics **
$ ***********************************************************

$ ***UNUSED*** 	Messages 1-14 are no longer used and need not be localized.
$		All of the mnemonics and labels relating to the "Help" menu
$		have been replaced by messages 81-96.

$ *
$ * Mnemonics and labels for the "Help" pulldown menu and items.
$ *
$ Mnemonic and label for the "Help" pulldown menu
1 H
2 Help

$ mnemonic and label for the "Overview" item
3 V
4 Overview...

$ mnemonic and label for the "Tasks" item
5 T
6 Tasks...

$ nmemonic and label for the "Table of Contents" item are message nos. 71 & 72

$ mnemonic and label for the "Reference" item
7 R
8 Reference...

$ mnemonic and label for the "On Item" item
9 O
10 On Item

$ mnemonic and label for "Using Help" item
11 U
12 Using Help...

$ mnemonic and label for "About Text Editor" item
13 A
14 About Text Editor...

$ *
$ * Mnemonics and labels for the "File" pulldown menu and items.
$ *
$ mnemonic and label for the "File" pulldown menu
15 F
16 File

$ mnemonic and label for the "New" item
17 N
18 New

$ mnemonic and label for the "Open" item
19 O
20 Open...

$ mnemonic and label for the "Include" item
21 I
22 Include...

$ mnemonic and label for the "Save" item
$ NOTE: The "Save" label can be either Message 24 or message 73
23 S
24 Save

$ mnemonic and label for the "Save As" item
25 A
26 Save As...

$ mnemonic and label for the "Print" item
27 P
28 Print...

$ mnemonic and label for the "Close" item
29 C
30 Close

$ *
$ * Mnemonics and labels for the "Edit" pulldown menu and items.
$ *
$ mnemonic and label for the "Edit" pulldown menu
31 E
32 Edit

$ mnemonic and label for the "Undo" menu item
33 U
34 Undo

$ mnemonic and label for the "Cut" menu item
35 t
36 Cut

$ mnemonic and label for the "Copy" menu item
37 C
38 Copy

$ mnemonic and label for the "Paste" menu item
39 P
40 Paste

$ mnemonic and label for the "Clear" menu item
41 e
42 Clear

$ mnemonic and label for the "Delete" menu item
43 D
44 Delete

$ ***UNUSED***	Messages 45-50 are no longer used and need not be localized.
$		They have been replaced by messages 101-106 of this set.
$ mnemonic and label for the "Select All" menu item
$ 45 A
$ 46 Select All

$ mnemonic and label for the "Find/Change" menu item
$ 47 F
$ 48 Find/Change...

$ mnemonic and label for the Check Spelling menu item
$ 49 S
$ 50 Check Spelling...

$ *
$ * Mnemonics and labels for the "Format" pulldown menu and items.
$ *
$ mnemonic and label for the "Format" pulldown menu
51 r
52 Format

$ mnemonic and label for the "Settings..." menu item
53 S
54 Settings...

$ mnemonic and label for the "Paragraph" menu item
55 P
56 Paragraph

$ mnemonic and label for the "All" menu item
57 l
58 All

$ *
$ * Mnemonics and labels for the "Options" pulldown menu and items.
$ *
$ mnemonic and label for the "Format" pulldown menu
59 O
60 Options

$ mnemonic and label for the "Overstrike" toggle button
61 O
62 Overstrike

$ mnemonic and label for the "Word Wrap" toggle button
$ ***UNUSED***	This label and mnemonic are no longer used and need not be
$		localized.  They've been replaced by messages 111 and 112.
63 W
64 Word Wrap

$ mnemonic and label for the "Status Line" toggle button
65 S
66 Status Line

$ *
$ * Late additions.
$ *

$ nmemonic and label for the "Table of Contents" item of the "Help" menu
$ ***UNUSED*** 	This nmemonic and label are no longer used and need not be
$		localized.  They've been replaced by messages 87 and 88.
71 C
72 Table of Contents...

$ label for the "Save" item in the "File" pulldown menu
$ NOTE: This label is used in place of message 24 when text has been
$       changed but has not been saved
73 Save (needed)


$ *
$ * Mnemonics and labels for the "Help" pulldown menu and items.
$ *
$ Mnemonic and label for the "Help" pulldown menu
81 H
82 Help

$ mnemonic and label for the "Overview" item
83 v
84 Overview

$ mnemonic and label for the "Tasks" item
85 T
86 Tasks

$ nmemonic and label for the "Table of Contents" item
87 C
88 Table of Contents

$ mnemonic and label for the "Reference" item
89 R
90 Reference

$ mnemonic and label for the "On Item" item
91 O
92 On Item

$ mnemonic and label for "Using Help" item
93 U
94 Using Help

$ mnemonic and label for "About Text Editor" item
95 A
96 About Text Editor


$ *
$ * Changes for "Edit" pulldown menu messages 45-50.
$ *

$ mnemonic and label for the "Select All" menu item
101 S
102 Select All

$ mnemonic and label for the "Find/Change" menu item
103 F
104 Find/Change...

$ mnemonic and label for the Check Spelling menu item
105 k
106 Check Spelling...

$ mnemonic and label for the "Copy As" File menu item
$ (should match the dialog title specified in message 42 of set 5)
$ ***UNUSED***	This label is no longer used and need not be localized.
107 A
108 Copy As...

$ mnemonic and label for the "Copy To File" File menu item
$ (should match the dialog title specified in message 42 of set 5)
109 F
110 Copy To File...

$ mnemonic and label for the "Wrap To Fit" toggle button
111 W
112 Wrap To Fit

$ ***********************************************************
$ **                                              helpCB.c **
$ **                                                       **
$ ** Messages and strings associated with online help.     **
$ **                                                       **
$ ***********************************************************
$set 12
$ Message used for "Help - On Item" if the user clicks outside application
1 You must select an item within the Text Editor.


$ ***********************************************************
$ **                                        ttMsgSupport.c **
$ **                                                       **
$ ** Messages and strings associated with ToolTalk.        **
$ **                                                       **
$ ***********************************************************
$set 13
1 Checkpoint save failed.
2 Checkpoint reply failed.
