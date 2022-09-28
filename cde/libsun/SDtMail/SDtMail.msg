$ @(#)SDtMail.msg	1.30 97/06/12 SMI
$ (c) Copyright 1996 Sun Microsystems, Inc.
$ 
$ Message file for SDtMail library
$

$quote "

$set 1 libSDtMail API set
$
$	All of the following error messages are specific to the library
$	that dtmail uses to access message stores and message transports.
$	The sdmporttest program 'errormessages' command can be used to generate this list,
$	the output of which can be directly imported into this message set.
$	Note: message numbers must start no lower than '1' (e.g. 0 is illegal)
$

1 "A Mailer operation has failed. Retry.\n"
2 "Internal Condition: The operation requested is not supported by the object being accessed.\n"
3 "The requested operation was cancelled.\n"
4 "You did not specify a server name for this mailbox.\n"
5 "Internal Condition: object is shutdown and cannot process the request.\n"
6 "Mailer has closed this mailbox.\n"
7 "Internal Condition: object is attached and cannot process the request.\n"
8 "Internal Condition: object is open and cannot process the request.\n"
9 "Internal Condition: A mailbox name is required to access the requested object.\n"
10 "Mailer cannot open this mailbox.\n"
11 "Internal Condition: object is opened read only and cannot process the request.\n"
12 "You did not specify a login name to access this mailbox.\n"
13 "You did not specify a password to access this mailbox.\n"
14 "Internal Condition: request made on a message that is deleted and expunged.\n"
15 "Internal Condition: specified message is missing structure information.\n"
16 "Internal Condition: Sdm_EC_ServiceFunctionAlreadyRegistered\n"
17 "Internal Condition: Sdm_EC_ServiceFunctionNotRegistered\n"
18 "Internal Condition: Sdm_EC_ConnectionAlreadyStarted\n"
19 "Internal Condition: Sdm_EC_ConnectionNotStarted\n"
20 "Internal Condition: Sdm_EC_SessionAlreadyStarted\n"
21 "Internal Condition: Sdm_EC_SessionNotStarted\n"
22 "Internal Condition: Sdm_EC_SessionAlreadyAttached\n"
23 "Internal Condition: Sdm_EC_SessionNotAttached\n"
24 "Internal Condition: the request for data provided a container object that was not empty.\n"
25 "Internal Condition: the requested data was not located.\n"
26 "Internal Condition: memory allocation request failed.\n"
27 "Internal Condition: request issued to start a message store that is already started.\n"
28 "Internal Condition: request issued on message store that has not yet been started.\n"
29 "Internal Condition: no server process registered for specified object.\n"
30 "Internal Condition: request to manipulate a message improperly used on a nested message.\n"
31 "Internal Condition: request to access or open a service is missing a required 'servicetype' option.\n"
32 "Internal Condition: request contains an argument that specifies an invalid or illegal value.\n"
33 "Internal Condition: request to access an existing message that does not exist.\n"
34 "Internal Condition: request to send a message that does not exist in an outgoing message store.\n"
35 "Internal Condition: request to create object for existing body part that does not exist.\n"
36 "You did not specify any recipients for this message.\nFill in the To, Cc, or Bcc field and send it again.\n"
37 "You cannot address an email message to Bcc recipients only.\nFill in the To or Cc field and send it again.\n"
38 "Internal Condition: request to send a message that is deleted.\n"
39 "Internal Condition: token used in request to open outgoing store contains invalid conditions.\n"
40 "Internal Condition: message committed to outgoing store while disconnected state.\n"
41 "Internal Condition: request to reconnect to already connected service.\n"
42 "Internal Condition: request to disconnect already disconnected service.\n"
43 "Internal Condition: specified type of service function cannot be used where specified.\n"
44 "Internal Condition: inactivity interval has not yet been reached.\n"
45 "An email address is invalid.\n"
46 "The search criteria are not valid.\n"
47 "Mailer cannot obtain a lock to open this mailbox.\n"
48 "An error occurred while Mailer was processing your mail setup.\n"
49 "An error occurred while Mailer was updating a mailbox (for example) after\nmoving messages between mailboxes or deleting messages.\n"
50 "Internal Condition: The source and target message stores are of incompatible types.\n"
51 "Internal Condition: The message sequence is either empty or contains duplicates.\n"
52 "Internal Condition: The contents of a message/rfc822 message body is missing a \"From\" line from the beginning of the contents.\n"
53 "Mailer cannot access the specified service.\n"
54 "Mailer cannot find the full path name of the current working directory.\n\n%.2048s\n"
55 "Mailer cannot access this portion (%.2048s)\n of the path for your mailbox (%.2048s).\n\n%.2048s\n"
56 "Mailer found NULLS in new e-mail: cannot check for new mail.\n"
57 "Mailer cannot send the message because one of the addresses may\nbe incorrect. Make sure the addresses are valid and try again.\n"
58 "Mailer cannot contact an SMTP mail server to send this message.\nEither you cannot send mail from your local host or\nyour SMTP mail server is not available.\n"
59 "Mailer has encountered an error.\n"
60 "Mailer cannot check for new email.\n"
61 "Mailer cannot connect to the IMAP server because \nthat server is not running on your host (%.2048s).\n"
62 "Mailer cannot append the message to the mailbox.\n"
63 "Mailer cannot copy or move the message to your mailbox.\n"
64 "Mailer cannot delete your mailbox.\n"
65 "Mailer cannot rename your mailbox.\n"
66 "Mailer cannot create your mailbox.\n"
67 "Mailer could not send your message because\nyour SMTP mail server is not responding.\nTry resending this message after a short wait.\nContact your system administrator for assistance if necessary.\n"
68 "Error messages from your SMTP mail server:\n%.2048s\n"
69 "Mailer encountered an error while decoding the attachment data.\nTry asking the sender to resend the attachment to you.\n"
2000 "Your alias or email address (%.2048s) contains a colon (:), which is an invalid character.\n"
2001 "Your alias or email address (%.2048s) contains too many @ symbols.\nUse only one @ symbol.\n"
2002 "Your alias or email address (%.2048s) contains an extra > symbol.\n"
2003 "The personal name placed before the address (%.2048s) is incorrect,\nas in: \"John Smith\" <john.smith@sun.com>.  Be sure to separate\nmultiple addresses or aliases with commas or commas and spaces.\n"
2004 "Your alias or email address (%.2048s) contains extra commas.\n"
2005 "Your alias or email address (%.2048s) is missing a domain name.  For\nexample, in the address mary.smith@eng.sun.com, eng.sun.com is the domain name.\n"
2006 "Your alias or email address (%.2048s) ends with a backslash (\\).  Backslashes\nmust be followed by another character.\n"
2007 "Your alias or email address (%.2048s) does not contain any addresses.  The\ncorrect format is: alias-group-name: address, address.\n"
2008 "Your alias or email address (%.2048s) does not end with a semicolon (;).\nAlways put a semi-colon at the end, as in:\nalias-group-name: address, address;\n"
2009 "Your alias or email address (%.2048s) does not contain a host name\nafter the @ symbol.\n"
2010 "Your alias or email address (%.2048s) contains a > symbol without the <\nsymbol.  A < symbol must precede the > symbol.\n"
2011 "Your alias or email address (%.2048s) contains a ] symbol without the [\nsymbol.  A [ symbol must precede the ] symbol.\n"
2012 "Your alias or email address (%.2048s) contains a ) symbol without the (\nsymbol.  A ( symbol must precede the ) symbol.\n"
2013 "Your alias or email address (%.2048s) is missing an end quote (\")\nafter an open quote.\n"
2014 "Your alias or email address (%.2048s) is missing a > symbol\nafter an initial < symbol.\n"
2015 "Your alias or email address (%.2048s) is missing a ] symbol\nafter an initial [ symbol.\n"
2016 "Your alias or email address (%.2048s) is missing a ) symbol\nafter an initial ( symbol.\n"
2017 "Your alias or email address (%.2048s) contains extra commas.\nUse only one comma to separate addresses; do not end an alias or\nemail address with a comma.\n"
2018 "Your alias or email address (%.2048s) contains an invalid character,\nsuch as a semicolon or comma in the middle of an incomplete alias or email address,\nor a space after the end of a complete alias or email address.\n"
2100 "You have not typed any search criteria: To, From, and so on.\n"
2101 "You did not specify any operator in your search criteria.\nChoose one from the Operators menu.\n"
2102 "Your search criteria contain an invalid space or tab in a text field.\n"
2103 "Your search criteria contain an invalid open parenthesis symbol\nin a text field.  The correct format is: (SUBJECT a or SUBJECT b).\n"
2104 "Your search criteria contain an invalid open parenthesis symbol after a\nheader keyword, such as SUBJECT.  The correct format is SUBJECT a, *not* SUBJECT (a).\n"
2105 "Your search criteria contain an alpha character in a text field\nwhere only numeric characters are allowed.\n"
2106 "Your search criteria contain an invalid end parenthesis symbol after a\nheader keyword, such as SUBJECT.  The correct format is: SUBJECT a, *not* SUBJECT )a).\n"
2107 "Your search criteria contain an end parenthesis symbol followed\nby an invalid character.  Type a space or tab after the ) symbol.\n"
2108 "Your search criteria contain an end parenthesis symbol after a\nkeyword, such as SUBJECT FROM.  Type a description after the keyword.\n"
2109 "Your search criteria contain an invalid keyword or operator.\n"
2110 "Your search criteria do not contain an end quote (\") after an open quote.\n"
2111 "Your search criteria are missing a description after a keyword, such\nas SUBJECT.  Type a description after the keyword.\n"
2112 "Your search criteria contain an operator keyword for which the criterion\nis missing.  For example, if you type: SUBJECT foo AND, add a criterion\nafter AND or delete AND.\n"
2113 "Your search criteria contain open parenthesis symbols that do not\nhave corresponding end parenthesis symbols.\n"
2114 "Your search criteria contain dates that are in an incorrect format.\nThe correct format is dd-mmm-yyy.\n"
2115 "The syntax for your search criterion is incorrect: Place the NOT (~) operator before the keyword, for example, ~ from foobar.\n"
2200 "Mailer cannot create a lock file (%.2048s) for this mailbox.\n\n%.2048s\n"
2201 "Mailer cannot create a lock file for this mailbox because the lock file\nname (%.2048s) is the same as an existing folder name. %.2048s\n"
2202 "Mailer cannot create a lock file (%.2048s) for this mailbox because you do not have\npermissions to access either the mailbox or the folder that contains the mailbox.\n\n%.2048s\n"
2203 "Mailer cannot create a lock file (%.2048s) for this mailbox because part of the\npath name for the lock file is not a directory.\n\n%.2048s\n"
2204 "Mailer cannot create a lock file (%.2048s) for this mailbox because it cannot\naccess the remote host on which the file is located.\n\n%.2048s\n"
2205 "Mailer cannot remove or release the mailbox lock (%.2048s).\n\n%.2048s\n"
2206 "Mailer cannot remove an obsolete mailbox lock (%.2048s) to open this mailbox.\nBecome root and remove the obsolete lock file.  Contact your system\nadministrator for assistance if necessary.\n\n%.2048s\n"
2207 "Mailer cannot obtain information on the mailbox lock (%.2048s).\n\n%.2048s\n"
2208 "A ToolTalk error occurred while Mailer was performing a\nToolTalk task (%.2048s) on the mailbox lock.\n\n%.2048s\n"
2209 "A ToolTalk error occurred while Mailer was performing a\nToolTalk task on the mailbox lock (%.2048s).\n\n%.2048s\n"
2210 "Mailer cannot obtain exclusive access to this mailbox because the\nhost on which it is located is not responding.\n"
2211 "Mailer cannot obtain exclusive access to this mailbox (%.2048s)\nbecause someone else is using it.\n"
2212 "Mailer cannot start ToolTalk to create a mailbox lock.\n\n%.2048s\n"
2213 "Mailer cannot obtain exclusive access to this mailbox (%.2048s)\nbecause it is already open.\n"
2300 "Mailer cannot open the file for your mail setup (%.2048s).\n\n%.2048s\n"
2301 "Cannot open mail rule file for updating.\nMail rule file: %.2048s\n\n%.2048s\n"
2302 "Mailer cannot rename your mail setup file (%.2048s)\nwith the new one (%.2048s).\n\n%.2048s\n"
2303 "Cannot rename new mail rule file over existing mail rule file.\nNew Mailrc file: %.2048s\nExisting Mailrc file: %.2048s\n\n%.2048s\n"
2304 "An error exists in your mail setup file (%.2048s).  A conditional (if)\nstatement does not have a corresponding endif statement.\n"
2401 "The user name (%.2048s) in the path name for this mailbox (%.2048s) does not exist.\n"
2402 "You must specify a mailbox name (%.2048s) after the tilde symbol (~)\nthat denotes your home directory in the path name.  The correct format\nis: ~/mailbox-name or ~user-name/mailbox-name.\n"
2403 "Mailer cannot update the mailbox (%.2048s).\n\n%.2048s\n"
2404 "Mailer cannot open the mailbox (%.2048s).\n\n%.2048s\n"
2405 "Mailer cannot remove this mailbox (%.2048s).\n\n%.2048s\n"
2406 "Mailer cannot obtain information on this mailbox (%.2048s).\n\n%.2048s\n"
2500 "The service port number after the host name must be numeric characters,\nfor example, host-name:500.\n"
2501 "The internet address that you specified for the host name is invalid.\n"
2502 "The host that you specified does not exist.\n"
2503 "Mailer cannot connect to the network.\nContact your system administrator for assistance.\n"
2504 "The server you specified does not provide the service required, for example,\nIMAP or SMTP services.  Contact your system administrator for assistance.\n"
2600 "Error messages from your mail server:\n%.2048s\n"
2601 "Mailer cannot create a session lock and will open the mailbox read-only.\n"
2602 "Mailer is awaiting the mailbox lock and will attempt to open the mailbox.\n"
2603 "Mailer is attempting to obtain the mailbox lock.\n"
2604 "The mailbox contains a message with an invalid \"From \" line format.\n"
2605 "The mailbox format is invalid.  Try restarting Mailer.\n"
2606 "The mailbox size has shrunk due to possible data corruption.  Try restarting Mailer.\n"
2607 "Mailer cannot lock the mailbox.  Try restarting Mailer.\n"
2608 "Mailer cannot check for new mail: it contains NULL strings.\n"
2609 "The mailbox is already in use.\n"
2610 "Mailer cannot set the group id for the mailbox.\n"
2611 "Mailer cannot read the mailbox.  Restart.\n"
2612 "Mailer is awaiting the destination mailbox lock.\n"
2613 "Mailer cannot log on to the IMAP server.  Verify that the\nserver name, login, and password are correct.\n"
2614 "The file you specified is not in a mailbox format that can be\nrecognized by Mailer.\n"
2615 "Error messages from your mailbox:\n%.2048s\n"
2616 "Mailer can no longer access your mailbox; it may have been moved,\ndeleted, or replaced. Close and reopen your mailbox.\n"

$set 2 system error number replacement set
$
$	All of the following error messages are replacements for those
$	errors defined in </usr/include/errno.h> and returned by strerr(3C).
$	When it is necessary to convert a system error number into an error
$	message to be displayed to the user, the message is first looked up in
$	this list; if not found then strerr(3C) is used to produce a message.
$

$ // EPERM
1 "You do not have the permissions to access this mailbox, folder, or file."

$ // ENOENT
2 "The mailbox, folder, or file does not exist."

$ // ESRCH
3 "No such process."

$ // EINTR
4 "interrupted system call."

$ // EIO
5 "A read or write error occurred with this mailbox, folder, or file."

$ // ENXIO
6 "No such device or address."

$ // E2BIG
7 "Arg list too long."

$ // ENOEXEC
8 "Exec format error."

$ // EBADF
9 "Bad file number."

$ // ECHILD
10 "No children."

$ // EAGAIN
11 "Resource temporarily unavailable."

$ // ENOMEM
12 "Your system is out of memory."

$ // EACCES
13 "You do not have the permissions to access this mailbox, folder, or file."

$ // EFAULT
14 "Bad address."

$ // ENOTBLK
15 "Block device required."

$ // EBUSY
16 "Mount device busy."

$ // EEXIST
17 "File exists."

$ // EXDEV
18 "Cross-device link."

$ // ENODEV
19 "No such device."

$ // ENOTDIR
20 "You specified a file name in error.  Specify a directory."

$ // EISDIR
21 "You specified a directory name in error.  Specify a file."

$ // EINVAL
22 "Invalid argument."

$ // ENFILE
23 "You have opened the maximum number of files allowed."

$ // EMFILE
24 "You have reached the maximum number of files you can open on your system."

$ // ENOTTY
25 "Inappropriate ioctl for device."

$ // ETXTBSY
26 "Text file busy."

$ // EFBIG
27 "File too large."

$ // ENOSPC
28 "You have run out of disk space on your system."

$ // ESPIPE
29 "Illegal seek."

$ // EROFS
30 "The mailbox, folder, or file you specified is on a disk or network device\nfor which you do not have write permission."

$ // EMLINK
31 "Too many links."

$ // EPIPE
32 "Broken pipe."

$ // EDOM
33 "Math arg out of domain of func."

$ // ERANGE
34 "Math result not representable."

$ // ENOMSG
35 "No message of desired type."

$ // EIDRM
36 "Identifier removed."

$ // ECHRNG
37 "Channel number out of range."

$ // EL2NSYNC
38 "Level 2 not synchronized."

$ // EL3HLT
39 "Level 3 halted."

$ // EL3RST
40 "Level 3 reset."

$ // ELNRNG
41 "Link number out of range."

$ // EUNATCH
42 "Protocol driver not attached."

$ // ENOCSI
43 "No CSI structure available."

$ // EL2HLT
44 "Level 2 halted."

$ // EDEADLK
45 "Deadlock condition."

$ // ENOLCK
46 "No record locks available."

$ // ECANCELED
47 "Operation canceled."

$ // ENOTSUP
48 "Operation not supported."

$ // EDQUOT
49 "You have exceeded the disk limit for your system."

$ // EBADE
50 "invalid exchange."

$ // EBADR
51 "invalid request descriptor."

$ // EXFULL
52 "exchange full."

$ // ENOANO
53 "no anode."

$ // EBADRQC
54 "invalid request code."

$ // EBADSLT
55 "invalid slot."

$ // EDEADLOCK
56 "file locking deadlock error."

$ // EBFONT
57 "bad font file fmt."

$ // ENOSTR
60 "Device not a stream."

$ // ENODATA
61 "no data (for no delay io)."

$ // ETIME
62 "timer expired."

$ // ENOSR
63 "out of streams resources."

$ // ENONET
64 "Machine is not on the network."

$ // ENOPKG
65 "Package not installed."

$ // EREMOTE
66 "The object is remote."

$ // ENOLINK
67 "the link has been severed."

$ // EADV
68 "advertise error."

$ // ESRMNT
69 "srmount error."

$ // ECOMM
70 "Communication error on send."

$ // EPROTO
71 "Protocol error."

$ // EMULTIHOP
74 "multihop attempted."

$ // EBADMSG
77 "trying to read unreadable message."

$ // ENAMETOOLONG
78 "This path name has exceeded the maximum number of characters allowed."

$ // EOVERFLOW
79 "value too large to be stored in data type."

$ // ENOTUNIQ
80 "given log. name not unique."

$ // EBADFD
81 "f.d. invalid for this operation."

$ // EREMCHG
82 "Remote address changed."

$ // ELIBACC
83 "Can't access a needed shared lib."

$ // ELIBBAD
84 "Accessing a corrupted shared lib."

$ // ELIBSCN
85 ".lib section in a.out corrupted."

$ // ELIBMAX
86 "Attempting to link in too many libs."

$ // ELIBEXEC
87 "Attempting to exec a shared library."

$ // EILSEQ
88 "Illegal byte sequence."

$ // ENOSYS
89 "Unsupported file system operation."

$ // ELOOP
90 "Symbolic link loop."

$ // ERESTART
91 "Restartable system call."

$ // ESTRPIPE
92 "if pipe/FIFO, don't sleep in stream head."

$ // ENOTEMPTY
93 "directory not empty."

$ // EUSERS
94 "Too many users (for UFS)."

$ // ENOTSOCK
95 "Socket operation on non-socket."

$ // EDESTADDRREQ
96 "Destination address required."

$ // EMSGSIZE
97 "Message too long."

$ // EPROTOTYPE
98 "Protocol wrong type for socket."

$ // ENOPROTOOPT
99 "Protocol not available."

$ // EPROTONOSUPPORT
120 "Protocol not supported."

$ // ESOCKTNOSUPPORT
121 "Socket type not supported."

$ // EOPNOTSUPP
122 "Operation not supported on socket."

$ // EPFNOSUPPORT
123 "Protocol family not supported."

$ // EAFNOSUPPORT
124 "Address family not supported by protocol family."

$ // EADDRINUSE
125 "Address already in use."

$ // EADDRNOTAVAIL
126 "Can't assign requested address."

$ // ENETDOWN
127 "Network is down."

$ // ENETUNREACH
128 "Network is unreachable."

$ // ENETRESET
129 "Network dropped connection because of reset."

$ // ECONNABORTED
130 "Software caused connection abort."

$ // ECONNRESET
131 "Connection reset by peer."

$ // ENOBUFS
132 "Your system is out of memory."

$ // EISCONN
133 "Socket is already connected."

$ // ENOTCONN
134 "Socket is not connected."

$ // ESHUTDOWN
143 "Can't send after socket shutdown."

$ // ETOOMANYREFS
144 "Too many references: can't splice."

$ // ETIMEDOUT
145 "Connection timed out."

$ // ECONNREFUSED
146 "Connection refused."

$ // EHOSTDOWN
147 "Host is down."

$ // EHOSTUNREACH
148 "No route to host."

$ // EALREADY
149 "operation already in progress."

$ // EINPROGRESS
150 "operation now in progress."

$ // ESTALE
151 "Stale NFS file handle."
