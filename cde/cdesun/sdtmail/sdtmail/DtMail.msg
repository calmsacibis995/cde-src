$
$ +SNOTICE
$
$  $Revision: 1.159 $
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
$  Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
$
$ +ENOTICE
$

$quote "

$
$ Strings from dtmail/RoamApp.C

$set 1
1 "OK" 
$ 	    /* NL_COMMENT
$ 	     * This is a menu label, please refer to the resource file "Dtmail" for
$            * appropriate mnemonics tranlation
$ 	     */
2 "Mail Options..." 
3 "Vacation" 
4 "Updating properties..." 


$
$ Strings from dtmail/RoamMenuWindow.C

5 "Cancel" 
6 "Mailer" 
7 "Mailer" 
8 "Mailer" 
9 "Delete" 
10 "Next" 
11 "Previous" 
12 "Reply to Sender" 
13 "Print One" 
14 "Sender" 
15 "Subject" 
16 "Date and Time" 
17 "Size" 
$ 	    /* NL_COMMENT
$            * Menu labels in Main Mailer (from RoamMenuWindow.C).
$            * The following menu labels require translation plus parenthesized 
$            * mnemonics for all Asian locales. Please refer to the resource file 
$            * "Dtmail" for each mnemonics.
$ 	     * Message 18-22,25,28,29 are menu labels for "Mailbox" pull down menu.
$            * Message 30,31,33-40 are menu labels for "Message" pull down menu. 
$            * Message 43-45 are menu labels for "Edit" pull down menu. 
$            * Message 47--55 are menu labels for "View" pull down menu. 
$            * Message 56-63 are menu labels for "Compose" pull down menu. 
$            * Message 64,65,221 are menu labels for "Move" pull down menu. 
$            * Message 66,68,69 are menu labels for "Attachment" pull down menu. 
$            * Message 71-77 are menu labels for "Help" pull down menu. 
$ 	     */
18 "Mailbox" 
19 "Check for New Mail" 
20 "Inbox" 
21 "Inbox" 
22 "New Mailbox..." 
23 "Mailer - New Mailbox" 
24 "New Mailbox" 
25 "Open..." 
26 "Mailer - Open Mailbox" 
27 "Open Mailbox" 
28 "Destroy Deleted Messages" 
29 "Close" 
30 "Open" 
31 "Save As Text..." 
32 "Mailer - Message - Save As Text" 
33 "Copy To Mailbox" 
34 "Print..." 
35 "Search..." 
36 "Select All Messages" 
37 "Delete" 
38 "Undelete Last" 
39 "Undelete From List..." 
40 "Message" 
41 "Move" 
42 "Mailer - Messages" 
43 "Copy Selected Text" 
44 "Select All Text" 
45 "Edit" 
46 "Mailer - Text" 
47 "Next" 
48 "Previous" 
49 "Abbreviated Headers" 
50 "By Date/Time" 
51 "By Sender" 
52 "By Subject" 
53 "By Size" 
54 "By Status" 
55 "View" 
56 "New Message" 
57 "New, Include All" 
58 "Forward" 
59 "Reply" 
60 "Reply to All" 
61 "Reply to Sender, Include Message" 
62 "Reply to All, Include Message" 
63 "Compose" 
64 "Move" 
65 "Select Mailbox..." 
66 "Save As..." 
67 "Mailer - Attachments - Save As" 
68 "Select All" 
69 "Attachments" 
70 "Mailer - Attachments" 
71 "Overview" 
72 "Tasks" 
73 "Reference" 
74 "On Item" 
75 "Using Help" 
76 "Help" 
77 "About Mailer..." 
78 "Mailer is confused about the state of this mailbox.\nClose the mailbox by choosing OK and then restart Mailer." 
79 "Converting... %d percent complete" 
80 "Mailer" 

$
$ Strings from dtmail/AttachArea.C

81 "Mailer" 
82 "Mailer" 
83 "Mailer" 


$
$ Strings from dtmail/Attachment.C

84 "%s is an executable attachment. Do you want to run it?" 
85 "Mailer" 
86 "Mailer" 
87 "Mailer" 
88 "Mailer" 


$
$ Strings from dtmail/RoamCmds.C

89 "Mailer - Other Mailboxes" 
90 "Move" 
91 "Printing..." 
92 "OK" 
93 "Save" 
94 "Save" 
95 "Save" 
96 "Empty" 
97 "Rename" 
98 "Mailer - Rename" 
99 "Mailer" 
100 "OK" 
101 "Cancel" 
102 "You are already using the forwarding facility for\nsomething other than Vacation. While Vacation is\nrunning, Vacation will be appended to this other\nforwarding activity. Is it still OK to start Vacation?\0" 
103 "Mailer" 
104 "You are already running the vacation program in your .forward file.\nConsult documentation on how to stop it and remove it from your .forward file.\nTry this command after fixing that problem.\0" 
105 "Cannot open .vacation.msg file -- No write permission." 
106 ".vacation.msg file exists. Replace with new text?" 
107 "Cannot open .vacation.msg file -- No write permission." 
108 "I am on vacation" 
$ 	    /* NL_COMMENT
$ 	     * This is the default value of the subject field in the
$ 	     * message that gets returned to the sender when vacation
$ 	     * is turned on.
$ 	     */
109 "I'm on vacation.\nYour mail regarding \"$SUBJECT\" will be read when I return.\n" 


$
$ Strings from dtmail/MsgScrollingList.C

110 "N" 
$ /* NL_COMMENT
$  * In a mailer container window's message scrolling list, a "N" appears
$  * to the left of a mail message header indicating that the mail message
$  * is "new" (just arrived and not yet viewed by the user).
$  * There is only space to display 1 character.  If "N" needs to be translated,
$  * please make sure the translation is only 1 character.
$  */
111 "N" 
$ /* NL_COMMENT
$  * In a mailer container window's message scrolling list, a "N" appears
$  * to the left of a mail message header indicating that the mail message
$  * is "new" (just arrived and not yet viewed by the user).
$  * There is only space to display 1 character.  If "N" needs to be translated,
$  * please make sure the translation is only 1 character.
$  */
112 "N" 
$ /* NL_COMMENT
$  * In a mailer container window's message scrolling list, a "N" appears
$  * to the left of a mail message header indicating that the mail message
$  * is "new" (just arrived and not yet viewed by the user).
$  * There is only space to display 1 character.  If "N" needs to be translated,
$  * please make sure the translation is only 1 character.
$  */
113 "N" 
$ /* NL_COMMENT
$  * In a mailer container window's message scrolling list, a "N" appears
$  * to the left of a mail message header indicating that the mail message
$  * is "new" (just arrived and not yet viewed by the user).
$  * There is only space to display 1 character.  If "N" needs to be translated,
$  * please make sure the translation is only 1 character.
$  */
114 "N" 


$
$ Strings from dtmail/Undelete.C

115 "Undelete" 
116 "Close" 

$
$ Strings from dtmail/SendMsgDialog.C

$ 	    /* NL_COMMENT
$            * Menu labels in Compose dialog (from SendMsgDialog.C).
$            * The following menu labels require translation plus parenthesized 
$            * mnemonics for all Asian locales. Please refer to the resource file 
$            * "Dtmail" for each mnemonics.
$ 	     * Message 117,121,122,125,127-129 are menu labels for "File" pull down menu.
$            * Message 130-134,139,140,155,156 are menu labels for "Edit" pull down menu. 
$            * Message 141,142,145,147-150,226 are menu labels for "Attachment" pull down menu. 
$            * Message 152-154,157 are menu labels for "Format" pull down menu. 
$            * Message 56-63 are menu labels for "Compose" pull down menu. 
$            * Message 64,65,221 are menu labels for "Move" pull down menu. 
$            * Message 66,68,69 are menu labels for "Attachment" pull down menu. 
$            * Message 71-77 are menu labels for "Help" pull down menu. 
$ 	     */
117 "Send" 
118 "Close" 
119 "New Message" 
120 "Mailer" 
121 "File" 
122 "Include..." 
123 "Mailer - Include" 
124 "Include" 
125 "Save As Text..." 
126 "Mailer - Save As Text" 
127 "Save Outgoing Mail"
128 "Send As" 
129 "Close" 
130 "Edit" 
131 "Undo" 
132 "Cut" 
133 "Copy" 
134 "Paste" 
135 "Paste Special" 
136 "Bracketed" 
137 "Indented" 
138 "Clear" 
139 "Delete" 
140 "Select All" 
141 "Attachments" 
142 "Add File..." 
143 "Mailer - Attachments - Add" 
144 "Add" 
145 "Save As..." 
146 "Mailer - Attachments - Save As" 
147 "Delete" 
148 "Undelete" 
149 "Rename" 
150 "Select All" 
$       /* NL_COMMENT
$        * This is an obsolete message.  It is replaced by message 226
$        * in set 1.
$        */
151 "Show Pane" 
152 "Format" 
153 "Word Wrap" 
154 "Settings..." 
155 "Find/Change..." 
156 "Check Spelling..." 
157 "Use Template" 
158 "Mailer - Attachments" 
159 "Mailer - Compose" 
160 "New Message" 
$       /* NL_COMMENT
$        * This is the label for a toggle item in a menu.  When the item
$        * is set to "Show List", the Attachment List is mapped in the
$        * Compose Window.  This message replaces message 151 in set 1.
$        */


$
$ Strings from dtmail/ViewMsgDialog.C

161 "Copy" 
162 "Select All" 
163 "Edit" 
164 "Save As..." 
165 "Mailer - Attachments - Save As" 
166 "Select All" 
167 "Attachments" 
168 "Mailer - Attachments" 
169 "Mailer - Text" 
170 "Overview" 
171 "Tasks" 
172 "Reference" 
173 "On Item" 
174 "Using Help" 
175 "Help" 
176 "About Mailer..." 
177 "Mailer" 


$
$ Strings from dtmail/DtMailGenDialog.C

$  /* NL_COMMENT
$   * 178 and 179 are obsolete messages, replaced by messages 235 and 236
178 "Mailer - About Dtmail" 
179 "Dtmail Version "
180 "OK" 
181 "OK" 
182 "Cancel" 
183 "Show More"

$
$ Strings from dtmail/FindDialog.C

$  /* NL_COMMENT
$   * This is an obsolete message.  Replaced by message 220 in set 1
$   */
184 "Find & Select All" 
185 "Clear" 
186 "Close" 
187 "Help" 
188 "To:" 
189 "From:" 
190 "Subject:" 
191 "Cc:" 
192 "Mailer - Search" 
193 "Forward" 
194 "Backward" 
$  /* NL_COMMENT
$   * This message replaces message 184 in set 1
$   */



$
$ Strings from dtmail/Editor.C

195 "------------- Begin Forwarded Message -------------\n\n" 
196 "------------- Begin Included Message -------------\n\n" 
197 "\n------------- End Forwarded Message -------------\n\n" 
198 "\n------------- End Included Message -------------\n\n" 

$
$ Strings from dtmail/XmTextEditor.C

199 "\n------------- Begin Included Message -------------\n\n" 
200 "\n------------- End Included Message -------------\n" 

$
$ Strings from dtmail/DtEditor.C

201 "\n------------- Begin Included Message -------------\n\n" 
202 "\n------------- End Included Message -------------\n" 

$
$ Strings from dtmail/ComposeCmds.C

203 "Mailer" 
204 "Unable to create a compose window." 
205 "Mailer" 
206 "Unable to create a compose window." 
207 "Mailer" 
208 "Unable to create a compose window." 
209 "Mailer" 
210 "Unable to create a compose window." 
211 "Mailer" 
212 "The template does not exist." 
213 "Mailer" 
214 "The template appears to be corrupt." 
215 "Mailer" 
216 "There is not enough memory to load the template." 
217 "Mailer" 
218 "The template appears to be corrupt." 

$
$ Strings from dtmail/SortCmd.C

219 "Sorting..." 

220 "Select All" 
221 "Inbox" 

222 "%a %b %e %k:%M" 
$ /* NL_COMMENT
$  * The "%a %b %d %k:%M" is the time and date format, please refer to 
$  * strftime man page for explanation of each format.
$  */
223 "%a %b %e %H:%M" 
$ /* NL_COMMENT
$  * The "%a %b %d %H:%M" is the time and date format, please refer to 
$  * strftime man page for explanation of each format.
$  */
224 "%a %b %d %k:%M" 
$ /* NL_COMMENT
$  * The "%a %b %d %k:%M" is the time and date format, please refer to 
$  * strftime man page for explanation of each format.
$  */
225 "%a %b %d %H:%M" 
$ /* NL_COMMENT
$  * The "%a %b %d %H:%M" is the time and date format, please refer to 
$  * strftime man page for explanation of each format.
$  */
226 "Show List" 

$ String from Undelete.C, Deleted Messages window title
227 "Mailer - Deleted Messages"

$ Strings from SendMsgDialog.C, Add Bcc, Delete Bcc menu buttons
228 "Add"
229 "Delete"
230 "Send"

$  /* NL_COMMENT
$   * This string is displayed on the find dialog status line
$   * when searching for a matching message.
$   */
231 "Searching..."

$        /* NL_COMMENT
$         * These strings are displayed on the find dialog status line
$         * when one or more matching messages are found.  The first
$         * string is displayed when there is one matching message,
$         * and the second string is displayed when there is more than
$         * one.  The %d is the number of messages that matched.
$         */
232 "1 message selected"
233 "%d messages selected"

$  /* NL_COMMENT
$   * This string is displayed on the find dialog status line when
$   * no matching messages were found.
$   */
234 "No matches were found"

$  /* NL_COMMENT
$   * These strings replace strings 178 and 179.  They are used in
$   * the "About Mailer" dialog box.
$   */
235 "Mailer - About Mailer"
236 "Mailer Version "
$  /* NL_COMMENT
$   * String 237 is the Copy button on Move Other Mailboxes dialog (from RoamCmds.C)
$   */
237 "Copy"
238 "%m %d %Y" 
$ /* NL_COMMENT
$  * The "%m %d %Y" is the date format, please refer to 
$  * strftime man page for explanation of each format.
$  */
239 "%x" 
$ /* NL_COMMENT
$  * The "%x" is the date format, please refer to 
$  * strftime man page for explanation of each format.
$  */

240 "Template..."
$ /* NL_COMMENT
$  * The popup for saving a files as a template from
$  * the compose window. 
$  */
241 "Draft Message..."
$ /* NL_COMMENT
$  * Saving compose window as draft file.
$  * From the compose window. 
$  */
242 "Text..."
$ /* NL_COMMENT
$  * Saving compose window as text file.
$  * From the compose window. 
$  */
243 "Save As"
$ /* NL_COMMENT
$  * Save as on the compose window
$  * Pullright menu for the above three selections.
$  */
244 "New, Use Template"
245 "New Message"
246 "New, with Text"
247 "Reply, Include Message"
248 "Reply to All, Include Message"

$ /* More template messages */
249 "The template is empty."

$ /* Alias Messages */
250 "Aliases"
251 "Alias List..."

$ /* Mail filing  Messages */
252 "Copy to Other Mailboxes"

$ /* Open Pullright on Mailbox Menu
253 "Drafts" 
254 "Select Mailbox"
255 "Sent Mail"
256 "Auto-filing Mailboxes..."
257 "Auto-filing Mailboxes"
258 "Mailer - Auto-filing Mailboxes"
259 "Mailer - Select Mailbox"

$ /* Options Menu 
260 "Options"
261 "Basic..." 
262 "Toolbar..." 
263 "Message View..." 
264 "Move Menu Setup..." 
265 "Compose Window..." 
266 "Signature..." 
267 "Vacation Message..." 
268 "Templates..." 
269 "Aliases..." 
270 "Advanced..." 

$ /*Compose Menu
271 "Forward, no Attachments" 

272 "Basic" 
273 "Toolbar" 
274 "Message View" 
275 "Move Menu Setup" 
276 "Compose Window" 
277 "Signature" 
278 "Vacation Message" 
279 "Templates" 
280 "Aliases" 
281 "Advanced" 
282 "Notification"
283 "Mailer - Rename Template"
284 "To"
285 "Empty Folder"
286 "Continue Composing Draft"
287 "The draft does not exist." 
288 "The draft appears to be corrupt."
289 "There is not enough memory to load the draft."
290 "The draft is empty."
291 "Mailer - Save As Draft"
292 "Mailer - Save As Template"
293 "Add Existing..."
294 "Mailer - Add Existing"
295 "Add Existing"
296 "Select..."
297 "Mailer - Select"
298 "Select"
299 "Include Signature"
300 "Do not specify a folder\nname when saving the template."
301 "Messages"
302 "Show Views..."
303 "Basic"
304 "Advanced"
305 "Cancel"
306 "Entire Message:"
307 "Search Type:"
308 "Example:"
309 "Save As View..."
310 "Show Views..."
311 "Clear"
312 "Error displaying search results"
313 "Invalid search string"
314 "Enter View Name:"
315 "Mailer - Save As View"
316 "Change Char Set"
317 "No Valid Character Set"
318 "Default Character Set"
319 "Unable to save as view, invalid search string"
320 "Move to Other Mailboxes"
321 "Open Other Mailboxes"
$ String displayed in the secondary window if no subject
322 "NO SUBJECT!"
323 "Headers"
324 "Dates"
325 "States"
326 "Operators"
327 "Notification..."
328 "\n\nCopyright (c) 1993-1997:\n\n  Sun Microsystems, Inc.\n  Novell, Inc.\n  International Business Machines Corp.\n  Hewlett-Packard Company"
329 "\n\nMailer is brought to you by:\n\n  Jennifer Chang\n  John Cooper\n  Patrick Curran\n  Gary Gere\n  Nancy Howes\n  Greg Layer\n  Andrea Mankoski\n  Tuan Nguyen\n  Dragomir Popovic\n  Marina Sum\n  Sara Swanson\n  John Togasaki\n  Esther Tong\n  Vivian Wong"
330 "Search Up"
331 "Search Down"
332 "Template Name:"
333 "Draft Name:"
334 "Login:"
335 "Password:"
336 "Always"
337 "When Open"
338 "Login"
339 "\"good morning\""
340 "Open Mailbox"
341 "Retrieve attachments:"
342 "Append"
343 "Rename Mailbox..."
344 "Mailer - Rename Mailbox"
345 "Rename Mailbox"
346 "Delete Mailbox..."
347 "Mailer - Delete Mailbox"
348 "Delete Mailbox"
349 "New mailbox name:"
350 "Rename"
$ Messages 351 and 352 are buttons for the dialog that comes up if
$ you try to rename a mailbox and the new name already exists.
351 "Overwrite"
352 "Choose New Name..."
353 "Delete"

$set 2
1 "Mailer" 
2 "ToolTalk is not initialized. Mailer cannot run without ToolTalk.\nTry starting /usr/dt/bin/ttsession, or contact your System Administrator." 

3 "Unable to initialize windows. Exiting.\n" 
4 "Mailer has not been properly installed,\nand cannot run because the execution group\nis incorrectly set."
5 "You do not have permission to view %s" 
$ 	    /* NL_COMMENT
$ 	     * The %s is the name of the mailbox the user doesn't have
$ 	     * permission to view.
$ 	     */
6 "The mailbox %s is a directory and can not be opened." 
7 "Unable to empty %s. It does not exist." 
8 "Container %s was emptied" 
9 "Error - wasn't able to empty %s" 
10 "Mailer" 
11 "%s" 
12 "Mailer" 
13 "Mailer" 
14 "An attachment needs to be selected before issuing the\n\"Save As\" command to save to a file." 

15 "Error - Unable to get mailrc." 
16 "No message selected." 
$ 	/* NL_COMMENT
$ 	 * No mail message has been selected by the user.
$ 	 */


17 "You have edited \"%s\". Delete anyway?" 
18 "Error: Cannot include file %s" 
19 "An attachment needs to be selected before issuing the\n\"Save As\" command to save to a file." 


20 "An attachment needs to be selected before issuing the\n\"Save As\" command to save to a file." 

21 "Mailer" 
22 "There were unrecoverable syntax errors found in the .mailrc file.\nCheck for more messages in the ~/.dt/errorlog file.\nFix the errors and restart Mailer. If you choose to continue,\nyou will not be able to save to the .mailrc file any changes made\nin the Options dialog box."
23 "Continue"
24 "Exit"
25 "There were unrecoverable syntax errors found in the .mailrc file.\nYou can apply Mail Options settings ONLY in the current session (the\nsettings will take effect now as long as Mailer is running) but will\nnot be saved to the .mailrc file." 
26 "Mailer cannot check if you have new email."
27 "Mailer cannot open this mailbox (%s)\nbecause it contains null characters."
28 "Mailer cannot check if new mail has arrived\nbecause your mailbox contains null characters."
29 "Make sure that the mailbox folder or the mailbox exists,\nand that you have write permission for them."
30 "Mailer cannot update your mail setup (.mailrc) file, so your new\nsettings will apply only to the current session."
31 "Mailer encountered an error while processing the contents of the attachment."

$set 3
1 "Auto-saving..." 

2 "Initializing..." 
3 "The mailbox %s does not exist.\nCreate a mailbox with this name?"
4 "Mailer" 
5 "OK" 
6 "The mailbox %s is locked.\nYou can manually unlock the mailbox and try again\nor contact your System Administrator."
7 "Mailer" 
8 "OK" 
9 "OK" 
10 "OK" 
11 "OK" 
12 "%s" 
13 "Message %d of %d, %d new, %d deleted" 
14 "Message %d of %d, %d new, %d deleted" 
$ 	/* NL_COMMENT
$ 	 * The user will see the following message display as:
$ 	 * "Message 3 of 10, 2 new, 6 deleted"
$ 	 * This means ??? -- Explain to translator.
$ 	 */
15 "Saving..." 
16 "Mailer" 
$ 		/* NL_COMMENT
$ 		 * This dialog comes up when the user tries to quit the
$ 		 * mailbox and the user is asked if he wants to destroy
$ 		 * the messages marked for deletion.
$ 		 */
$		/* NL_COMMENT
$		 * Messages 16 and 17 are no longer being used.  They are
$		 * being replaced by messages 86 and 87.
$		 */
17 "Destroy the messages you have marked\nfor deletion in this mailbox?"
18 "OK" 
19 "Cancel" 
20 "Opening mailbox..." 
21 "You are creating a new mailbox, but one already exists with the\nname you specified.\n\nClick Delete and Replace to delete the existing mailbox and replace it\nwith a new, empty mailbox.  Otherwise, click Cancel."
22 "Mailer" 
23 "Unable to overwrite %s.\nCheck file permissions and retry." 
24 "Mailer" 
25 "Loading container..." 
26 "Folder Summary Information" 
27 "Loading..." 
28 "Empty container" 
29 "OK" 
30 "OK" 
31 "OK" 
32 "Destroying deleted messages..." 
33 "OK" 

34 "%s does not exist." 
35 "Unable to open %s." 
36 "Unable to allocate memory." 
37 "Summary of attachments" 
38 "attachment" 
39 "attachments" 
$ 	/* NL_COMMENT
$ 	 * "attachments" is the plural form of "attachment".
$ 	 */
40 "attachments" 
41 "deleted" 

42 "%s already exists. Replace?" 
43 "Unable to replace %s." 
44 "Unable to create %s." 
45 "Unable to create %s." 


46 "Empty container" 
47 "%s already exists.\nOverwrite?" 
48 "Mailer" 
49 "Unable to overwrite %s.\nCheck file permissions and retry."
50 "Mailer" 
51 "Unable to create %s." 
52 "Mailer" 
53 "Unable to write to %s." 
54 "Mailer" 
55 "Unable to create %s." 
56 "Mailer" 
57 "Rename attachment as" 
58 "If you close this window\nyou will lose your edits."
59 "No Memory" 
60 "There is not enough memory to load the existing .vacation.msg file." 
61 "Mailer" 
62 "The existing .vacation.msg file appears to be corrupt." 


63 "Select a message to move." 
64 "Select a message to copy." 
65 "%d messages moved to %s" 
66 "%d message moved to %s" 
67 "%d messages copied to %s" 
68 "%d message copied to %s" 
69 "View already exists. Raising it." 
$ 	/* NL_COMMENT
$ 	* The current mail message selected is already displayed in a separate
$ 	* window.  Therefore this 'separate' window will be raised in front
$ 	* of existing windows so the user can see it.
$ 	*/



70 "Writing dead letter..." 
71 "Mailer" 
72 "OK" 
73 "Cancel" 
74 "OK" 

75 "OK" 
76 "OK" 

77 "You have an attachment open that may have unsaved changes.\nSending this message will break the connection to the open\n attachment. Any unsaved changes will not be part of the\n message. You can use Save As to save Changes after the\n connection is broken, but the changes will not be part of\n the attachment."
78 "Initializing Mail Options..."
79 "Destroy and Quit"
80 "Don't destroy, Quit"
81 "This attachment may contain commands that can cause serious\n\damage.  It is recommended that you only execute it after you are certain it is safe to do so.  Press OK if you are certain it is safe, Cancel to cancel execution."
82 "Someone else is using this mailbox.\nWould you like to demand exclusive access?"
83 "Mailer cannot obtain exclusive access to this mailbox.  You can choose to\nto open this mailbox read-only or open it read-write without exclusive access.\n\nCaution: Click Read-Write only if you are sure that neither you nor\nanyone else has this mailbox open in another mail application.\nIf this mailbox is open in another mail application, clicking\nRead-Write can corrupt or destroy your mailbox contents."

$            /* NL_COMMENT
$             * The following sentence means %d number of mail messages have
$             * been deleted from the mail folder.  This is the plural form
$             * of the message that gets printed if more than one message
$             * is moved.
$             */
84 "%d messages deleted"

$            /* NL_COMMENT
$             * The following sentence means %d number of mail messages have
$             * been deleted from the mail folder.  This is the singular
$             * form of the message that gets printed if only one message
$             * is moved.
$             */
85 "%d message deleted"
86 "Checking for new mail..."
$		/* NL_COMMENT
$		 * This dialog comes up when the user tries to quit the
$		 * mailbox.  The user is asked if they want to destroy
$		 * the deleted messages.
$		 */
87 "Mailer - Close"
88 "Destroy the deleted messages and close this mailbox?"
89 "Destroy and Close"
90 "Retain and Close"
$        /* NL_COMMENT
$         * Post a dialog explaining that the action was invalid
$         */
91 "Cannot execute invalid action."
$        /* NL_COMMENT
$         * Post a dialog explaining that the action failed.
$         */
92 "Executing action failed!"
93 "The selected icon %s is an executable.\nClick OK to run it or Cancel to quit."

94 "Mailer is unable to obtain exclusive access to this\nmailbox because the system is not responding.\n(The file $HOME/.dt/errorlog may contain additional information).\n\nFor this time only, you can choose to open this mailbox\nread-only, or to open it read-write without exclusive access.\n\nCaution: Click Read-Write only if you are sure that neither you nor \nanyone else has this mailbox open in another mail application.\nIf this mailbox is open in another mail application, clicking \nRead-Write can corrupt or destroy your mailbox contents."
95 "Read-only"
96 "Read-Write"
97 "Specify the name of the file to save this message"

$
$ Strings from OptCmd.C
98 "You did not specify a file for saving outgoing mail."
99 "Use Default File"
100 "Turn Off Feature"

$
$ Strings for templates and drafts.
101 "Specify a template name"
102 "This will delete the template menu label.\nDo you also want to delete the template file:\n%s?"
103 "Unable to create folder: %s.\n Draft not saved"
104 "Unable to create folder: %s.\n Template not saved"
105 "This will rename the template menu label.\nDo you also want to rename the template file:\n%s?"
106 "Unable to delete the template file:\n%s"
107 "Unable to rename the template file:\n%s"
108 "Use Draft"
109 "Error getting text from compose editor.\nTemplate not saved."
110 "The template %s\ndoes not exist. Cannot rename template."
111 "The template %s\ndoes not exist. Cannot delete template."
112 "The template %s\nalready exists. Overwrite?"
113 "You cannot save attachments with templates.\nAttachments not saved.\nThe draft was saved to:\n%s"
114 "No Templates"
115 "Specify a draft name"
116 "Draft saved to %s"
117 "Template saved to %s"
118 "Sending the message will destroy the Draft:\n%s"
119 "Send and Destroy"
120 "Unable to destroy the draft file:\n%s"
121 "Delete File"
122 "Do Not Delete File"
123 "The menu label %s already exists.\n Rename failed."
124 "Your email setup does not indicate whether you have a\nlocal or IMAP mailbox.  Specify your choice.  If you do\nnot know the answer, click Local."
125 "Local"
126 "IMAP"
127 "No Drafts"
128 "Mailer cannot display the message you selected."
129 "Mailer cannot delete the message you selected."
130 "Mailer cannot undelete the messages you marked for deletion."
131 "Mailer cannot undelete the last message you marked for deletion."
132 "Mailer cannot display the message headers in the Deleted Messages window."
133 "Mailer cannot update the Message Header List."
134 "Mailer cannot destroy the messages you deleted."
135 "Mailer cannot update the message headers in the Deleted Messages window."
136 "Rename File"
137 "Do Not Rename File"
138 "Do you also want to rename the template file?"
139 "Mailer cannot access the message you selected."
140 "Reloading View..."
141 "This mailbox contains messages marked for deletion that\nare not included in the deleted count for this view. Clicking\nOK destroys those messages as well as the ones in this view."
142 "Mailer cannot save the state of your mailbox.  Your messages remain intact."
143 "Your message contains attachments \nthat will not be saved to the text file."
144 "Drafts directory (%s) has no write permission."
145 "Templates directory (%s) has no write permission."
146 "Mailer has opened a read-only mailbox in which you can read\nmessages.  You cannot change this mailbox or check for new\nmail from it."
147 "Delete and Replace"
148 "This mailbox is now open in another Mailer session.\nYour mailbox is now read-only."
149 "Keep Read-Only"
$ Strings for rename and delete dialogs
$ You get message 150 when you try to rename a mailbox and a mailbox with the new name already exists.
150 "A mailbox named %s already exists.  Either overwrite the mailbox,\nspecify another name, or cancel the operation."
$ You get message 151 when you try to rename your Inbox.
151 "You have specified your Inbox to be renamed.  Renaming your Inbox\nchanges it to a regular mailbox.  Mailer will create a new Inbox to receive\nmail."
$ Message 152 is the generic error message when we weren't able to rename a mailbox, but we are not sure why.
152 "Mailer cannot rename the mailbox %s."
$ Messages 153 and 154 are confirmation dialogs making sure you want to delete your mailbox and inbox, respectively.
153 "Mailer will delete the mailbox %s.\nYou cannot undelete it afterward."
154 "You have specified your Inbox to be deleted.  You cannot undelete it\nafterward."
155 "Mailer cannot delete the mailbox %s."
$ Messages 156 and 157 are status messages that appear in the status line when the delete or rename
$ operation has finished.
156 "%s deleted"
157 "Renaming completed"




$
$ Strings from Options dialog which are not generated by dtbuilder, and error 
$ messages from Rule Editor dialog
$
$set 4
1 "Rules..."
2 "ON"
3 "OFF"
4 "You did not specify a rule name."
5 "You did not specify the criteria for this rule."
6 "You did not specify the mailbox path name for this rule."
7 "Mailer cannot display some of the settings that you use\nto create this rule.  Clicking Add or Change in the Rule\nEditor dialog box may cause deletion of these settings."

$set 5
1 "Mailer" 
2 "Mailer" 
3 "Sending this message as is will\n	    cause you to lose any changes\n	    to the attachments."
$    /* NL_COMMENT
$     * The user tried to send a messages without saving changes in
$     * some open attachments.  This warning makes sure that is what
$     * the user intended.
$     */
4 "Select only one attachment\n	    and then choose rename"
$       /* NL_COMMENT
$        * Mailer ran out of memory.  Ask the user to quit some other
$        * applications so there will be more memory available.
$        */
5 "Some of the addresses in the message are incorrect,\nand do not refer to any known users in the system.\nPlease make sure all of the addresses are valid and try again."
$       /* NL_COMMENT
$        * Mailer ran out of memory.  Ask the user to quit some other
$        * applications so there will be more memory available.
$        */
6 "Mailer does not have enough memory\navailable to send this message.\nTry quitting other applications and\nresend this message."
$       /* NL_COMMENT
$        * There was an error from the mail transport (sendmail).
$        */
7 "An error occurred while trying to send your message.\nCheck to make sure the message was received.  If not,\nyou may have to resend this message."
$       /* NL_COMMENT
$        * User trying to send a message that has no addressed recipient.
$        */
8 "Try Send after specifying  recipient(s) of the message in \n\the To:, Cc:, or Bcc: fields."
$       /* NL_COMMENT
$        * User sending message greater in size than the value set with mailrc resource msgsizelimit
$        */
9 "The size of this message ( %s+ ) exceeds the\nmessage size limit ( %dKbytes ) you have set\nin your mail options. You can:"
10 "Cannot set headers for new message."
11 "Cannot get size for new message."


$set 12
1 "Attachment" 
2 "Attachments" 
3 "displayCount" 
4 "Cannot attach FIFO files: %s" 
5 "Cannot attach character special files: %s" 
6 "Cannot attach directories: %s" 
7 "Cannot attach block special files: %s" 
8 "Cannot attach socket files: %s" 
9 "Cannot create message attachment" 
10 "Cannot set contents for attachment"
11 "Cannot set name for attachment"
12 "Cannot delete attachment: %s"
13 "Cannot undelete attachment: %s"


$set 13
1 "Cannot create" 
2 "Unable to write" 

$
$ Strings from dtmail/AttachCmds.C

$set 14
1 "Add Attachment" 
2 "Unable to open %s" 
3 "Cannot attach FIFO files: %s" 
4 "Cannot attach character special files: %s" 
5 "Cannot attach directories: %s" 
6 "Cannot attach block special files: %s" 
7 "Cannot attach socket files: %s" 
8 "Unable to open %s" 
9 "Mailer - Rename Attachment" 
10 "Description" 
11 "Save Attachment As" 

$set 15
1 "You have made unsaved changes.\nYou can save your changes, discard your changes,\nor return to your previous place in the dialog."
2 "Save"
3 "Discard"
4 "Treat the following as: \"%s\" ="
5 "No start date was specified.\nThe vacation notifier will not take effect."
6 "The start date has already passed.\nThe vacation notifier will not take effect."
7 "The end date occurs before the start date.\nThe vacation notifier will not take effect."
8 "A system error occurred setting up the vacation notifier:\n%s\nThe vacation notifier will not take effect."
9 "You did not turn on the vacation notifier.\nTo do so, check the On button under Vacation"
10 "The start date format is incorrect.\nUse the format shown next to the End Date.\nThe vacation notifier will not take effect."
11 "The end date format is incorrect.\nUse the format shown next to the End Date.\nThe vacation notifier will not take effect."
12 "You have not entered a start date.\nThe vacation notifier will not take effect."
13 "The start date format is incorrect.\nUse the format shown next to the End Date."
14 "The end date format is incorrect.\nUse the format shown next to the End Date."
15 "You already started the vacation message on the host %s.\nGo to this host and stop your vacation message,\nor it may start or stop prematurely."
16 "To check for new mail in mailboxes,\nselect the mailboxes and click Check Selected."
17 "To open a mailbox, select it and click Open."
18 "A custom header name cannot contain spaces."
19 "An alias name cannot contain spaces."
20 "A header field cannot contain spaces."
21 "This alias name is already on the Alias list."
22 "You have typed in an invalid address list.\nBe sure to separate multiple addresses\nor aliases with commas or commas and spaces."
23 "You have typed in an invalid alias name. Alias names\nmust not contain spaces, commas, or quotes."
24 "You have specified an invalid email address or alias in\nthe To, Cc, or Bcc field. Be sure you have separated multiple\naddresses or aliases with commas or commas and spaces."
25 "The signature file that you specified already exists.\nDo you want to overwrite its contents with this signature text?"
26 "Mailer cannot open the mailbox."
27 "The mailbox name already exists."
28 "The new setting for your IMAP inbox server\n will not take effect until Mailer restarts.\nDo you want to restart Mailer?"
29 "Restart"
30 "Apply - Do Not Restart"
31 "Cancel Apply"
32 "Deselecting the option, \"Save messages in MIME format,\" causes a\nsignificant decrease in performance when you open mailboxes."
$ This should be translated according to the output from the at(1) job command
$ in the OS. When the at(1) job is executed "job jobid" (in C locale), 
$ is sent to stdout, so "job" should be translated in accordance with whatever 
$ the OS strings get translated to for this command.
33 "job"

$set 16
1 "Mailer - Login" 
2 "Local"
3 "IMAP Server:"
4 "Auto-save failed."
5 "Check for new mail failed."
6 "Connecting to the IMAP server..."
7 "An error occurred in accessing the IMAP server."
8 "You did not specify a name for the IMAP server."
9 "You have lost exclusive access to this mailbox."
10 "The IMAP server connection does not exist for %s on %s."
11 "Close and Reopen"
12 "Close"
13 "Your mailbox lock no longer exists.  Mailer will close all your\nlocal mailboxes, except your InBox, to avoid data corruption."
14 "This mailbox has been changed by another instance of Mailer or another\nmailer, or by another program or process. \n\nSome changes made to this mailbox in this Mailer session may be lost.\nFor instance, if you marked some messages for deletion and did not\nclick Destroy Deleted Messages, and if auto-save did not run on this\nMailer session before the mailbox was changed outside of this Mailer\nsession, those messages will not be deleted.\n\nClick Close to close the mailbox or Close and Reopen to close the\nmailbox and reopen it."
$ Strings from Views.C
$
$set 17
1 "All"
2 "New"
3 "Inbox"
4 "Views"
5 "Default Views                 New   Total"
6 "Custom Views                  New   Total"
7 "Current View:"
8 "New..."
9 "Edit..."
10 "Delete"
11 "Load"
12 "Update"
13 "Close"
14 "Help"
15 "Selected view is empty"
16 "Loading view..."
17 "No view selected"
18 "Error loading view"
19 "Default Views"
20 "New"
21 "Total"
22 "Custom Views"
23 "Error editing view"
24 "Mailer cannot update the message statistics for your views"
25 "Error updating statistics"
26 "The view \"%s\" is invalid.\n"
27 "Updating Views..."
