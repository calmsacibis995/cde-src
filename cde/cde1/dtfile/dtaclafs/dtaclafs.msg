$ ************************************<+>*************************************
$ ****************************************************************************
$ *
$ *   FILE:           dtaclafs.msg
$ *
$ *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
$ *
$ *   DESCRIPTION:    Contains messages for nls translation (message catalog).
$ *
$ *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
$ *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
$ *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
$ *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
$ *
$ ****************************************************************************
$ ************************************<+>*************************************

$ **
$ **  This file is the source for the message catalog for dtaclafs,
$ **  the Andrew File System Access Control List editor.
$ **  Any additional messages are to be added to this file by hand,
$ **  and the associated index number placed in the code.
$ **

$ *************************************<L>*************************************
$ **  -----------------GENERAL LOCALIZATION NOTES SECTION---------------------
$ * Comment lines begin with a $ except $set which indicates the start of a
$ * new set.
$ *
$ * Do not delete any comments; you may add comments for your use.
$ *
$ **  ----------------GENERAL LOCALIZATION NOTES SECTION END------------------
$ *************************************<L>*************************************

$ *****************************************************************************
$
$ ***** NOTE FOR MESSAGE CATALOG TRANSLATORS *****
$
$ There may be three types of messages in this file:
$
$ 1. Messages that appear in dialogs or are diaplayed to the user.
$
$     These messages are the default and they should ALL BE LOCALIZED.
$     Note that these messages do NOT have any identification (see the
$     comments for type 2 and 3 below).
$
$ 2. Messages that only appear in the DT error log file ($HOME?.dt/errorlog).
$
$     The localization of these messages is OPTIONAL.  These messages are
$     identified by the following:
$
$     MESSAGES xx-yy WILL ONLY APPEAR IN THE DT ERRORLOG FILE
$
$ 3. Messages that should not be localized.
$
$     These messages are identified by the following:
$
$     DO NOT TRANSLATE or CHANGE or LOCALIZE MESSAGES xx-yy
$
$ ***** END (NOTE FOR MESSAGE CATALOG TRANSLATORS) *****
$
$ *****************************************************************************

$set 1
$ ****** Module: dtaclafs ****(developer comment only)********
$ ****** Messages for all dtaclafs files are included here ***
$

$ an error message
1 Bad argument list\nUsage: %s path <-advanced>\n

$ window title
2 File Manager - AFS Folder Properties

$ labels
3 File: %s\nis inside\nFolder:
4 AFS permissions for folder: %s

$ labels for action area buttons
5 OK
6 Apply
7 Reset
8 Cancel
9 Help

$ label used in basic mode when the rights cannot be represented
10 Use advanced mode

$ more labels for AFS items
11 Positive Permissions
12 Negative Permissions
13 Users and Groups

$ the warning about directory level ACL's
14 AFS permissions can only be set for folders.\n\
All files in a folder inherit that folder's permissions.

$ error message
15 Cannot get AFS Permissions
16 It is possible this directory is not in AFS or that you do not have a valid token
17 Cannot set AFS Permissions for
18 Possible reasons include:\n \
     -File not in AFS\n \
     -Too many users on access control list\n \
     -Tried to add non-existent user to access control list

$ labels for dialog mode radio box
19 Permissions List:
20 Advanced
21 Basic

$ titles for error dialogs
22 AFS Folder Properties Error

$ message 23 is not used
$ 23

$ acl error and informational messages
24 The AFS Permissions list cannot have more than %d entries
25 \"%s\" is not known to AFS
26 %s is a duplicate name\nThe permissions for the existing entry will be replaced

$ titles for error dialogs
27 AFS Folder Properties Information
28 AFS Folder Properties Warning 

$ names of advanced-mode (the usual AFS names) rights
29 read
30 lookup
31 insert
32 delete
33 write
34 lock
35 administer

$ names of basic-mode rights
36 read
37 write
38 administer

$ button labels
39 Add user ...
40 Delete user ...

$ message 41 is not used
$ 41

$ error message for add user/group dialog
42 You pressed OK but either no name was entered or no permissions were set

$ titles for error dialogs
43 AFS Folder Properties Question

$ more button lables
44 Yes
45 No

$ a question for the user
46 One or more users (or groups) will be deleted because no permissions are set.\n\
Do you wish to continue?

$ titles for popup dialogs
47 Add User or Group to Positive Permissions
48 Add User or Group to Negative Permissions

$ another question for the user
49 Do you want to ignore the changes that\nyou've made to the AFS Permissions?

$ titles for popup dialogs
50 Delete User or Group from Positive Permissions
51 Delete User or Group from Negative Permissions

$ some instructions which appear in the "delete" dialog box
52 Select the Users or Groups\n\
you wish to delete and press OK

$ a warning message
53 There are no users to delete

$ an error message
54 Unable to check the validity of \"%s\".\nEither I can't contact the AFS server or the ACL is full.

$ labels for "show negative permissions" radio box
55 Negative Permissions:
56 Show
57 Hide

$ messages summarizing number of entries in "Negative permissions"
58 (currently no entries)
59 (currently 1 entry)
60 (currently %d entries)

$ additional action area button label
61 Options
