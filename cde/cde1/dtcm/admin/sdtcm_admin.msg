
$quote "


$set	1
3	"usage: %s\n"
$ 		/* NL_COMMENT
$ 
$ 		   Messages 3-24:
$ 
$ 		   These messages are used to create the usage message.
$ 		   The individual options (e.g. -a, -z, etc) should
$ 		   not be translated.
$ 
$ 		   usage: ./sdtcm_admin
$ 		     -a [ -c <calendar@host> ]          Add a new calendar.
$ 	       	     -d [ -c <calendar@host> ] [ -f ]   Remove a calendar. [-f don't prompt ]
$ 		     -i [ -c <calendar@host> ]          Print information about a calendar.
$ 		     -l [ -h <hostname> ]               List calendars.
$ 		     -p <symbolic-mode-list>            Modify calendar's access list.
$ 			[ -c <calendar@host> ]
$ 			[ -n <name> ]
$ 		     -x [ -c <calendar@host> ]          List calendar's access and rights.
$ 		     -z [ -c <calendar@host> ]          Remove name from access list.
$ 			[ -n <name> ]
$ 
$ 		*/
4	"  %-35sAdd a new calendar.\n"
5	"-a [ -c <calendar@host> ]"
6	"  %-35sRemove a calendar. [-f don't prompt ]\n"
7	"-d [ -c <calendar@host> ] [ -f ]"
8	"  %-35sPrint information about a calendar.\n"
9	"-i [ -c <calendar@host> ]"
10	"  %-35sList calendars.\n"
11	"-l [ -h <hostname> ]"
12	"  %-35sModify calendar's access list.\n"
13	"-p <symbolic-mode-list>"
14	"\t[ -c <calendar@host> ]\n"
15	"\t[ -n <name> ]\n"
20	"  %-35sList calendar's access and rights.\n"
21	"-x [ -c <calendar@host> ]"
22	"  %-35sRemove name from access list.\n"
23	"-z [ -c <calendar@host> ]"
24	"\t[ -n <name> ]\n"
25	"%s: Could not delete calendar because: %s.\n"
26	"%s: Could not get calendar info because: %s.\n"
27	"-n <name> must be used in "
28	"conjuction with the -p, -r or -z options.\n"
29	"%s: Could not change permission because: %s.\n"
30	"%s: Could not list permissions because: %s.\n"
32	"Delete the calendar %s (y/n)? "
33	"Delete aborted.\n"
34	"%s: Could not delete calendar because: %s.\n"
35	"%s: Could not create calendar because: %s.\n"
36	"%s: Could not list calendars because: %s.\n"
37	"No calendars to list.\n"
38	"%s: Could not get calendar info because: %s.\n"
40	"unsupported: %s\n"
41	"%C"
$ 			/* NL_COMMENT
$ 
$ 			   Message 41: This message represents the time the
$ 			   calendar was created.  It is handed to strftime.
$ 			   It will be later used in a string that looks 
$ 			   like this:
$ 
$ 			   date created:   Tue Sep 19 08:34:23 PDT 1995
$ 
$ 			*/
42	"calendar:       %s\n"
43	"unknown"
44	"owner:          %s\n"
46	"date created:   %s\n"
48	"data version:   %d\n"
49	"data version:   unknown\n"
50	"server version: %d\n"
51	"server version: unknown\n"
52	"size (bytes):   %d\n"
53	"size (bytes):   unknown\n"
54	"entries:        %d\n"
55	"entries:        unknown\n"
56	"character set:  %s\n"
58	"product id:     %s\n"
60	"access rights:\n"
61	"%20s %s\n"
$ 				/* NL_COMMENT
$ 
$ 				   Message 61: The %20s %s conversions hold 
$ 				   space for the strings:
$ 
$ 				                Public: View Insert Change 
$ 				           Semiprivate: --
$ 					       Private: --
$ 			           Calendar Attributes: --
$ 					         Roles: None
$ 
$ 				  The idea is that they are all colon aligned.
$ 
$ 				   The field size is currently 20 but should
$ 				   be changed to match the the length of the
$ 				   ``Calendar Attributes:'' string when 
$ 				   translated.  
$ 
$ 				*/
62	"Public:"
63	"%20s %s\n"
64	"Semiprivate:"
65	"%20s %s\n"
66	"Private:"
67	"%20s %s\n"
68	"Calendar Attributes:"
69	"%20s %s\n\n"
70	"Roles:"
71	"unknown\n"
72	"Calendar already exists"
73	"Entry was not found"
74	"Calendar does not exist"
75	"Invalid entry handle"
76	"Invalid session handle"
77	"Permission denied"
78	"Invalid parameter"
79	"Read only"
80	"Invalid attribute value"
81	"Unsupported attribute"
82	"Not supported"
83	"Invalid enumerator"
84	"Insufficient memory"
85	"Disk is full"
86	"Service is unavailable"
87	"Invalid server location"
88	"Calendar service is not registered"
89	"Server is not responding"
90	"Unknown failure"
91	"Unsupported error code"
92	"View "
93	"Insert "
94	"Change "
104	"Owner "
105	"Sponsor "
106	"Author "
107	"Browse "
108	"Insert "
109	"Delete "
110	"None"
111	"--"
112	"%s: Syntax error. "
113	"Check your command options format.\n"
120	"%s: Could not change permission"
121	" because: %s.\n"
122	"%s: Could not remove permission"
123	" because: %s not in access list.\n"
126	"%s: Could not change permission because: %s.\n"
127	"no access\n"
133	"%s: Access rights are not supported "
134	"on pre-version 4 calendars.\n"
139	"%s: Could not change permission"
140	" because: %s.\n"
