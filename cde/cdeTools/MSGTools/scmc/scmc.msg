$ $XConsortium: scmc.msg /main/cde1_maint/1 1995/07/19 12:40:24 drk $
$ @(#)scmc.msg	1.11  @(#)scmc.msg	1.11 01/20/96 15:41:30
$ COMPONENT_NAME: (PIITOOLS) AIX PII Tools

$ FUNCTIONS: Standard header file for scmc

$ ORIGINS: 27

$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved

$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
                                                                   

$ scmc msg file created Mon Feb 20 09:46:53 1989
$ user:  howard

$quote "         define quote character.

$set MS_scmc        Define initial set#

E_MSG_0              "Done.                         "
E_MSG_1              "Only NLTEXTMAX messages in any one file please"
M_MSG_2              "Bad word:  file '%s', set '%s', msg '%s', word '%s'\n"
E_MSG_3              "Unable to allocate enough memory for internal tables ( rmalloc ), aborting"
E_MSG_4              "Unable to allocate memory for constant table, exiting."
E_MSG_5              "Unable to allocate enough memory for internal tables ( rmalloc ), aborting"
E_MSG_6              "Unable to allocate enough memory for internal tables ( rmalloc ), aborting"
E_MSG_7              "Unable to allocate memory for message table, exiting."
E_MSG_8              "Unable to open input file, aborting"
E_MSG_9              "Unable to realloc, file may be to large."
E_MSG_10             "Unable to read input file."
ECM_MSG_11           "Press Enter to exit."
ECM_MSG_12           "Old:  "
ECM_MSG_13           "New:  "
M_MSG_14             "Press Enter to make substitution, ESC to skip."
ECM_MSG_15           "Message:"
M_MSG_16             "Enter a message_name:"
E_MSG_20             "An unexpected end of text was encountered, aborting."
E_MSG_23             "Signal recieved, aborting"
E_MSG_24             "Searching..."
E_MSG_25             "Attempt to free a null pointer.  tsk tsk."
E_MSG_26             "Unable to find conversion method name."
E_MSG_27             "Sorry only 30 message files can be updated at one time."
ECM_MSG_31           "Unable to find parameter %d"
E_MSG_33             "An unexpected eof of cfig file was encountered."
E_MSG_36             "Unable to open .msg file for output."
E_MSG_37             "Check for access rights on the file and the directory."
E_MSG_49             "Unable to get enough memory to preform memmove"
M_MSG_50             "Warning %s not found."
E_MSG_51             "Unable to read _msg.h file"
E_MSG_52             "Warning Not all constants have been defined. ( too many to define )"
E_MSG_54             "Unable to get enough memory for edit array."
E_MSG_55             "The input message had more than 30 lines, and was truncated."
E_MSG_56             "Unable to get enough memory for edit array."
E_MSG_57             "Unable to read message file."
E_MSG_58             "Unable to read message file."
E_MSG_59             "Invalid quote character, ignoring."
E_MSG_60             "Invalid set number, ignoring."
E_MSG_61             "Invalid set number, ignoring."
E_MSG_65             "Unable to open config file."
E_MSG_66             "Enexpected eof of cfig file encountered."
E_MSG_67             "Unable to read configuration file."
E_MSG_68             "Enexpected eof of cfig file encountered."
E_MSG_69             "Unable to read configuration file."
E_MSG_70             "Enexpected eof of cfig file encountered."
M_MSG_71             "String was too long:\n\
%200s\n\
Exiting."
M_MSG_73             "-f specified, but no cfig file name followed it."
M_MSG_74             "A number did not follow the -l<num> command flag"
E_MSG_75             "Usage: scmc [-q] [-f<file>] [-l<num>] [-b] file"
M_MSG_78             "Bad conversion table line\n\
%s\n\
Aborting."
E_MSG_79             "File name too long."
M_MSG_81             "Bad line in key table\n\
%s\n\
Aborting.\n"
M_MSG_82             "Bad line in key table (too many commas)\n\
%s\n\
Aborting.\n"
M_MSG_83             "Bad line in key table (too many commas)\n\
%s\n\
Aborting.\n"
M_MSG_84             "Commands:"
M_MSG_85             "   None of the above"
M_MSG_86             "   Display source code text"
M_MSG_87             "   Override and leave the illegal word unchanged"
M_MSG_88             "   Edit the current version"
ECM_MSG_89           "The highlighted word is illegal:"
M_MSG_90             "Select new phrase or command ( Esc to accept and edit )"
E_MSG_91             "Unable to malloc memory."
E_MSG_92             "Unable to copy string longer than 2048 bytes"
E_MSG_93             "The message name contains an invalid character."
E_MSG_94             "This message name has already been used."
E_MSG_95             "Not all conversion methods have been defined."
E_MSG_96             "Saving files....                    "
E_MSG_98             "Unable to open file. Aborting."
E_MSG_101            "Unable to write to output file."
E_MSG_38             "Unable to stat '_msg.h' file."
E_MSG_40             "Unable to stat '.msg' file."
E_MSG_42             "The _msg.h file did not exist for this file, creating it."

$set MS_SCMC        Redefine set#

M_SPELLHLP           "You are in the scmc spell checker, this facility finds\n\
words which are not permitted and allows the operator to make substitutions\n\
for these words.  The first choices on the list are suggested synonyms for\n\
the illegal words.  The choices after the word \"Commands:\" are \n\
(guess what) commands.  Moving the highlight bar to the appropriate choice\n\
will either execute the command or make the substitution, whichever is\n\
appropriate.  \n\
\n\
  - Once the message text is as close to correct as possible,\n\
    press ESC or select the \"Edit the current version\" command.  This \n\
    will allow editing of the final message. \n\
    - Press return when through editing.\n\
\n\
  - To override select the \"Override and leave the illegal word unchanged\" \n\
    command.\n\
    -You will still have the opportunity to edit the final message."
MCHKSUB              "The screen is currently displaying a section of the source\n\
code being processed.  The bottom part of the screen should show the \n\
parameter being substituted out, and the one that will replace it.  The \n\
PAGE-UP, PAGE-DOWN, and the arrow keys will cause different sections of the \n\
source code to be displayed.  To make the substitution (substitute old for \n\
new), press Enter, to leave this section of source code unchanged press ESC."
