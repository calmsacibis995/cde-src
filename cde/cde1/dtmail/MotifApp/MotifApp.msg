$
$ +SNOTICE
$
$  $Revision: 1.2 $
$
$  RESTRICTED CONFIDENTIAL INFORMATION:
$
$  The information in this document is subject to special
$  restrictions in a confidential disclosure agreement between
$  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
$  document outside HP, IBM, Sun, USL, SCO, or Univel without
$  Sun's specific written approval.  This document and all copies
$  and derivative works thereof must be returned or destroyed at
$  Sun's request.
$
$  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
$
$ +ENOTICE
$

$ -------------------------------------
$ MotifApp/Application.C
$

$quote "

$ -------------------------------------
$ MotifApp/AskFirstCmd.C
$

$quote "


$set 1
1 "Do you really want to execute this command?" 
$ -------------------------------------
$ MotifApp/DialogManager.C
$

$quote "


2 "OK" 
3 "Cancel" 
$ -------------------------------------
$ MotifApp/Help.C
$

$quote "


4 "DtMail Version Dialog" 
5 "DtMail Version Dialog" 
6 "You must select a component withing your app.\n" 

$ Messages 7 and 8 are no longer used.  They are both
$ replaced by message 9.

7 "DtMail Help" 
8 "DtMail Help" 


$quote "


9 "Close this folder?" 
$ -------------------------------------
$ MotifApp/WarnNoUndoCmd.C
$

$quote "


10 "This command cannot be undone. Proceed anyway?" 
11 "Show hidden folders and files"

$quote "

$ /* NL_COMMENT
$  * This message replaces messages 7 and 8.
$  */
12 "Mailer : Help"

$set 2
1 "Selection Error, cannot continue\n" 
2 "Selection aborted by user.\n" 
$ -------------------------------------
$ MotifApp/QuitCmd.C
$

