$quote "
$set 1
3	"clnt_create for rpc.ttdbserverd on %s failed%s"
4	"Error: rpc.ttdbserverd on %s is not running"
6	"server version (%s) does notmatch the version of the database tables \
(%s). Please run a version %s database server"
7	"Attempt to open database table %s:%s failed"
14	"could not delete the X root window property %s advertising this sess\
ion"
15	"address version is %d, but I only understand %d! (address: <%s>)"
17	"could not parse X display name: \"%s\""
18	"could not queue a message on file \"%s\" because of internal error %\
d"
19	"could not delete stale session <%s> from interest list of file \"%s\"\
 because of internal error %d"
20	"could not delete uninterested session <%s> from interest list of fil\
e \"%s\" because of internal error %d"
21	"tttracefile syntax error in line:\n%s"
22	"Could not process tttracefile in $%s"
23	"No tracing allowed if real UID does not equal effective UID"
24	"Accepting request"
25	"empty document and no file"
26	"cannot get messageID; operation may not be cancel-able, because tt_m\
essage_arg_val()"
27	"cannot get title; document will be untitled because tt_message_arg_v\
al()"
28	"unknown opnum offset"
29	"could not lock mount table:"
666     "_Tt_file_system::findBestMountPoint -- max_match_entry is null, aborting...\n"
$set 2
2	"indistinguishable signaturesfound for ptype %s -"
3	"No connection-oriented transport"
4	"A ToolTalk client died before it could be signalled to retrieve a mes\
sage it was due"
5	"Connection to ToolTalk client lost while signalling it to retrieve a \
message"
6	"Found another session running (host=%s, pid=%d)"
7	"Can't contact alleged session (host=%s, pid=%d); assuming it crashed.\
.."
8	"Can't parse advertised session id; overwriting it with my own..."
9	"%s is a version %d types database, and this version can only read ver\
sions %d and earlier"
10	"could not decode types from types database: %s. It may be damaged."
11	"Overwrote %s"
12	"Ignoring unknown attribute <%s> of ToolTalk signature..."
13	"ttce2xdr failed (status=%d); types in Classing Engine \"user\" datab\
ase not converted..."
14	"duplicate ISAM record"
15	"ISAM file not open"
16	"illegal ISAM argument"
17	"illegal ISAM key descriptor"
18	"too many ISAM files open"
19	"bad ISAM file format"
20	"ISAM non-exclusive access"
21	"ISAM record locked"
22	"ISAM key already exists"
23	"ISAM key is primary key"
24	"end or beginning of ISAM file"
25	"no ISAM record found"
26	"no current ISAM record"
27	"ISAM file locked"
28	"ISAM file name too long"
29	"ISAM cannot allocate memory"
30	"ISAM RPC timeout"
31	"Broken ISAM TCP/IP connection"
32	"Cannot connect to ISAM server"
33	"Cannot import ISAM data"
34	"no local SAM daemon"
35	"ISAM internal fatal error"
36	"ISAM Locale/LANG mismatch"
$set 3
2	"child (%d) exited due to signal %d%s"
3	" (core dumped)"
4	"child (%d) neither exited nor was signaled!"
5	"cannot maximize clients because %m"
6	"No scope to manage.  Use -c, -p, -d, or set $DISPLAY."
7	"Cannot use -S option with -c"
8	"child ttsession exited with status %d"
9	"child ttsession exited due to signal %d%s"
10	" (core dumped)"
11	"child ttsession neither exited nor was signaled!"
12	"starting"
13	"have re-read types"
14	"error in types; keeping old types"
15	"$OPENWINHOME not set"
16	"\nUsage: ttsession [-a unix|des|gss][-d display][-spStvhNX"
17	"][-c command]\n -c [command]	start a process tree session, and run c\
ommand in it.\n		Subsequent options are passed to command.  Default: $SH\
ELL\n -p		start a process tree session, and print its id\n -d display	st\
art an X session on display\n\n -a unix|des|gss	set server authentication le\
vel\n -s		silent. Don't print out any warnings\n  -o		allow_unauth_types_load=[yes|no]    Allow API to run tt_session_types_load without[with] -a option.\n -S		don't fork into the\
 background\n -N		maximize the number of clients allowed\n -t		turn on m\
essage tracing\n -X		use XDR databases for static types (default)\n"
18	" -E		use Classing Engine for static types\n"
19	"\n -v		print out version number\n -h		print out this message\n\nSign\
al interface:\n kill -USR1 ttsession_pid	toggle message tracing\n kill -\
USR2 ttsession_pid	re-read static types"
20	"exiting"
21	"Error: %s exists but is unreadable by ttsession."
22	"Error: %s exists but has syntax errors. See ttsession_file(4)."
23	"Warning: using COMPAT option in %s."
24	"Warning: using AUTH option in %s."
$set 4
2	"Usage:\ntt_type_comp [-s] [-d db] [-mM] source_file\ntt_type_comp [-s\
] [-d db] -r type ...\n-M	merge source types into specified database, no\
t updating existing types\n-m	merge, but update existing types.  Default\
.\n-r	remove source types from the specified database\n-d db	database to\
 operate on. One of: user, system, or network. Default: user\n-G	perform\
 garbage collection on the ToolTalk database server.\n\ntt_type_comp [-s\
E] -p|O|P [-d db]\ntt_type_comp [-s]  -p|O|P compiled_file\n-O	enumerate\
 on stdout the names of all otypes read\n-P	enumerate on stdout the name\
s of all ptypes read\n-p	pretty-print on stdout all the ToolTalk types r\
ead\n-E	use the Classing Engine database(s) instead of the XDR database(\
s)\n-d db	database to read from. One of: user, system, or network. Defau\
lt: all\n\ntt_type_comp [-s] -x [-o compiled_file] source_file\n-x	compi\
le types from source_file (or stdin, if file is \"-\")\n-o	write compile\
d types to compiled_file (or stdout, if file is \"-\")\n	Default: source\
_file.xdr, or \"types.xdr\" if source is stdin\n\ntt_type_comp [-hv]\n-v\
	print out version number\n-h	print out this message\n-s	do not print ou\
t any status messages.\n\nThese cpp options will be passed through:\n   \
     -undef -Dname -Idirectory -Uname -Ydirectory"
3	"Semantic error in types file"
4	"Not a valid ToolTalk types file"
5	"Cannot read any ToolTalk types from Classing Engine database"
6	"Merging Classing Engine tables is no longer supported"
7	"Cannot read types in %s database - version mismatch"
8	"Cannot read types in %s database"
9	"$OPENWINHOME not set"
10	"Cannot initialize %s database for writing"
11	"Could not remove old definition for %s"
12	"Overwriting"
13	"Writing"
14	"Could not add new definition for %s"
15	"Could not remove old definition for %s"
16	"Overwriting"
17	"Writing"
18	"Could not add new definition for %s"
19	"Version mismatch in compiled types"
20	"Cannot read types in database"
21	"$OPENWINHOME not set"
22	"Cannot read types in %s database - version mismatch"
23	"Cannot read types in %s database"
24	"$OPENWINHOME not set"
25	"Removing otype %s\n"
26	"Removing ptype %s\n"
27	"output written to %s\n"
28	"Invalid database: %s"
29	"Specify only one of the options -O -P -m -M -p -r -x"
30	"no preprocessing done because: %s: %s"
31	"otype inheritance cycle involving %s"
32	"ancestor %s of otype %s does not exist"
33	"ptype %s does not exist, but was named as implementorby otype %s"
34	"multiple handlers defined in otype %s"
35	"inherited signature in otype %s does not exist in parent"
36	"two ptypes, %s and %s, with the same handler"
37	"ancestor %s of otype %s does not exist"
38	"cyclic otype inheritance hierarchy -\n  {"
39	"duplicated definitions of otype %s"
40	"duplicated definitions of ptype %s"
$set 5
3	"Security violation: RPC call wanted me to open a file that is not a T\
oolTalk database"
4	"Any data written using an old (<= 1.0.2) rpc.ttdbserverd after using \
a new (>= 1.1) rpc.ttdbserverd will be ignored"
5	"Any data written using an old (<= 1.0.2) rpc.ttdbserverd after using \
a new (>= 1.1) rpc.ttdbserverd will be ignored"
6	"rpc.ttdbserverd version (%s) does not match the version (%s) of the d\
atabase tables. Please install an rpc.ttdbserverd version %s (or greater\
)"
7	"Usage:\nrpc.ttdbserverd [-S] [-n] [-m DTMOUNTPOINT]\nrpc.ttdbserverd \
[-S] [-v]"
8	"Unable to start garbage collection thread. thr_create()\n"
9	"Unable to fork() for garbage collection.\n"
$set 6
2	"Usage:\nttdbck [-f file] [-k objkey] [-t type] [-bx] \n[-impa] [-IZ] \
[-F newfilename] [-T newtype] [mountpoints]\n"
3	"ttdbck: you must specify a selection [-fkt] option or a diagnosis [-b\
x] option\nif a repair [-FTZ] option is specified\n"
4	"Version mismatch in compiled types"
5	"Cannot read types in database"
6	"ttdbck: try 'ttdbck -I'.\n"
7	"Select by filename: %s\n"
8	"Select by objid key:"
9	"Select by type: %s\n"
10	"Diagnose badly formed entities\n"
11	"Diagnose references to non-existent entities\n"
12	"Display ids\n"
13	"Display mandatory data\n"
14	"Display properties and values data\n"
15	"Invoke NetISAM isrepair() function before inspecting\n"
16	"Repair by setting to type: %s\n"
17	"Repair by deleting\n"
18	"Debugging printout level %d\n"
19	"Repair by setting to file: %s\n"
20	"Error: no file for spec.\n"
21	"Error: no type for spec.\n"
22	"Error: \"%s\" is not an installed otype.\n"
23	"Error: spec has multiple values for type property.\n"
24	"Error: "
25	"ttdbck: no errors found.\n"
26	"Error: "
$set 7
2	"Usage: %s {ctx}[fhpPv[v]] [tarfile] pathname ...\n       %s {ctx}fL[h\
pPRv[v]] tttarfile pathname ...\n       %s -v\n       %s -h\n"
3	"\tc       create an archive\n\tt       list an archive's contents\n\t\
x       extract from an archive\n\tf       use next arg <tarfile> as arc\
hive\n\th       follow symbolic links\n\tL       do not invoke tar(1)\n\t\
p       preserve file modes\n\tP       (root) do not preserve objects' o\
wner, mod time, etc.\n\tR       do not recurse into directories\n\tv    \
   be verbose\n\tvv      be very verbose\n\t-v      print the version nu\
mber and quit\n\t-h[elp] print this message\n"
4	"%s: Could not read object kind from archive stream.\n"
5	"%s: Could not read archive version from archive stream.\n"
6	"%s: Found archive version %d, but expected version %d.\n"
7	"%s: found object of unknown kind %d in archive.\n"
8	"%s: Could not read object kind from archive stream.\n"
9	"%s: Could not read archive version from archive stream.\n"
10	"%s: Found archive version %d, but expected version %d.\n"
11	"%s: found object of unknown kind %d in archive.\n"
$set 8
2	"%s: Could not remove ToolTalk objects of %s because %s\n"
3	"%s: Could not move ToolTalk objects of \"%s\" to \"%s\" because %s\n"
4	"%s: Will not attempt to move the ToolTalk objects of:\n"
5	"Usage: %s [-] [-fL] path1 path2\n       %s [-] [-fL] path1 [path2 ...\
] dir\n       %s -v\n       %s -h\n"
6	"\t-L      do not perform a mv(1)\n\t-v      print the version number \
and quit\n\t-h      print this message\n"
7	"%s: Could not remove ToolTalk objects of %s because %s\n"
8	"Usage: %s [-] [-%s] %s ...\n       %s -v\n       %s -h\n"
9	"file"
10	"dir"
11	"\t-L      do not perform a %s(1)\n\t-v      print the version number\
 and quit\n\t-h[elp] print this message\n"
12	"%s: Could not remove ToolTalk objects of %s because %s\n"
13	"Usage: %s [-pL] file1 file2\n       %s [-prRL] path1 [path2 ...] dir\
\n       %s -v\n       %s -h\n"
14	"\t-L      do not perform a cp(1)\n\t-v      print the version number\
 and quit\n\t-h      print this message\n"
$set 9
2	"Usage: %s [-0FCa][-o outfile] [-S session | command [options]]\n     \
  %s [-e script | -f scriptfile][-S session | command [options]]\n -0		T\
urn off message tracing in session, or run command\n		without message tr\
acing (i.e. only API tracing)\n -F		Follow all children forked by comman\
d or subsequently\n		started in session by ttsession(1)\n -C		Do not tra\
ce ToolTalk API calls\n -a		Print all attributes, arguments, and context\
 slots of\n		traced messages.  Default is single-line summary.\n -e scri\
pt	Read tttracefile(4) settings from script\n -f scriptfile	Read tttrace\
file(4) settings from scriptfile. \"-\": stdin.\n -o outfile	Output. \"-\
\": stdout. default: stdout for session tracing,\n		stderr (of tttrace) \
for command tracing\n -S session	Session to trace.  default: see tt_defa\
ult_session()\n command	ToolTalk client command to invoke and trace\n"
3	"%s: session <%s> does not support Session_Trace.  Use kill -USR1 inst\
ead. See ttsession(1).\n"
$set 11
2	"TT_OK\tRequest successful."
3	"TT_WRN_NOTFOUND\tThe object was not removed because it was not found.\
"
4	"TT_WRN_STALE_OBJID\tThe object attribute in the message has been repl\
aced with a newer one. Update the place from which the object id was obt\
ained."
5	"TT_WRN_STOPPED\tThe query was halted by the filter procedure."
6	"TT_WRN_SAME_OBJID\tThe moved object retains the same objid."
7	"TT_WRN_START_MESSAGE\tThis message caused this process to be started.\
  This message should be replied to even if it is a notice."
8	"TT_WRN_APPFIRST\tThis code should be unused."
9	"TT_WRN_LAST\tThis code should be unused."
10	"TT_ERR_CLASS\tThe Tt_class value passed is invalid."
11	"TT_ERR_DBAVAIL\tA required database is not available. The condition \
may be temporary, trying again later may work."
12	"TT_ERR_DBEXIST\tA required database does not exist. The database mus\
t be created before this action will work."
13	"TT_ERR_FILE\tFile object could not be found."
14	"TT_ERR_MODE\tThe Tt_mode value is not valid."
15	"TT_ERR_ACCESS\tAn attempt was made to access a ToolTalk object in a \
way forbidden by the protection system."
16	"TT_ERR_NOMP\tNo ttsession process is running, probably because tt_op\
en() has not been called yet. If this code is returned from tt_open() it\
 means ttsession could not be started, which generally means ToolTalk is\
 not installed on this system."
17	"TT_ERR_NOTHANDLER\tOnly the handler of the message can do this."
18	"TT_ERR_NUM\tThe integer value passed is not valid."
19	"TT_ERR_OBJID\tThe object id passed does not refer to any existing ob\
ject spec."
20	"TT_ERR_OP\tThe operation name passed is not syntactically valid."
21	"TT_ERR_OTYPE\tThe object type passed is not the name of an installed\
 object type."
22	"TT_ERR_ADDRESS\tThe Tt_address value passed is not valid."
23	"TT_ERR_PATH\tOne of the directories in the file path passed does not\
 exist or cannot be read."
24	"TT_ERR_POINTER\tThe opaque pointer (handle) passed does not indicate\
 an object of the proper type."
25	"TT_ERR_PROCID\tThe process id passed is not valid."
26	"TT_ERR_PROPLEN\tThe property value passed is too long."
27	"TT_ERR_PROPNAME\tThe property name passed is syntactically invalid."
28	"TT_ERR_PTYPE\tThe process type passed is not the name of an installe\
d process type."
29	"TT_ERR_DISPOSITION\tThe Tt_disposition value passed is not valid."
30	"TT_ERR_SCOPE\tThe Tt_scope value passed is not valid."
31	"TT_ERR_SESSION\tThe session id passed is not the name of an active s\
ession."
32	"TT_ERR_VTYPE\tThe value type name passed is not valid."
33	"TT_ERR_NO_VALUE\tNo property value with the given name and number ex\
ists."
34	"TT_ERR_INTERNAL\tInternal error (bug)"
35	"TT_ERR_READONLY\tThe attribute cannot be changed."
36	"TT_ERR_NO_MATCH\tNo handler could be found for this message, and the\
 disposition was not queue or start."
37	"TT_ERR_UNIMP\tFunction not implemented."
38	"TT_ERR_OVERFLOW\tToo many active messages (try again later)."
39	"TT_ERR_PTYPE_START\tAttempt to launch instance of ptype failed."
40	"TT_ERR_CATEGORY\tPattern object has no category set."
41	"TT_ERR_DBUPDATE\tThe database is inconsistent: another tt_spec_write\
 updated object first."
42	"TT_ERR_DBFULL\tTooltalk database is full."
43	"TT_ERR_DBCONSIST\tDatabase is corrupt or access information is incom\
plete (run ttdbck)."
44	"TT_ERR_STATE\tThe Tt_message is in a Tt_state that is invalid for th\
e attempted operation."
45	"TT_ERR_NOMEM\tNo more memory."
46	"TT_ERR_SLOTNAME\tThe slot name is syntactically invalid."
47	"TT_ERR_XDR\tThe XDR proc. passed evaluated to zero len, or failed du\
ring invocation."
48	"TT_ERR_NETFILE\tThe netfilename passed is not syntactically valid."
49	"TT_DESKTOP_UNMODIFIED\tOperation does not apply to unmodified entiti\
es"
50	"TT_MEDIA_ERR_SIZE\tThe specified size was too big or too small"
51	"TT_MEDIA_ERR_FORMAT\tThe data do not conform to their alleged format\
"
52	"TT_ERR_APPFIRST\tThis code should be unused."
53	"TT_ERR_LAST\tThis code should be unused."
54	"TT_STATUS_LAST\tThis code should be unused."
55	"Unknown Tt_status value."
56	"TT_DESKTOP_EPERM\tNot super-user"
57	"TT_DESKTOP_ENOENT\tNo such file or directory"
58	"TT_DESKTOP_EINTR\tInterrupted system call"
59	"TT_DESKTOP_EIO\tI/O error"
60	"TT_DESKTOP_EAGAIN\tNo more processes"
61	"TT_DESKTOP_ENOMEM\tNot enough space"
62	"TT_DESKTOP_EACCES\tPermission denied"
63	"TT_DESKTOP_EFAULT\tBad address"
64	"TT_DESKTOP_EEXIST\tFile exists"
65	"TT_DESKTOP_ENODEV\tNo such device"
66	"TT_DESKTOP_ENOTDIR\tNot a directory"
67	"TT_DESKTOP_EISDIR\tIs a directory"
68	"TT_DESKTOP_EINVAL\tInvalid argument"
69	"TT_DESKTOP_ENFILE\tFile table overflow"
70	"TT_DESKTOP_EMFILE\tToo many open files"
71	"TT_DESKTOP_ETXTBSY\tText file busy"
72	"TT_DESKTOP_EFBIG\tFile too large"
73	"TT_DESKTOP_ENOSPC\tNo space left on device"
74	"TT_DESKTOP_EROFS\tRead-only file system"
75	"TT_DESKTOP_EMLINK\tToo many links"
76	"TT_DESKTOP_EPIPE\tBroken pipe"
77	"TT_DESKTOP_ENOMSG\tNo message of desired type"
78	"TT_DESKTOP_EDEADLK\tDeadlock condition"
79	"TT_DESKTOP_ENODATA\tNo data available"
80	"TT_DESKTOP_EPROTO\tProtocol error"
81	"TT_DESKTOP_ENOTEMPTY\tDirectory not empty"
82	"TT_DESKTOP_ETIMEDOUT\tConnection timed out"
83	"TT_DESKTOP_EALREADY\tOperation already in progress"
84	"TT_DESKTOP_ECANCELED\tOperation canceled"
85	"TT_DESKTOP_ENOTSUP\tNot supported"
