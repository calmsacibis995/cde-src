<chapter id=CommandSummary > Command Summary

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
 * <xref ProcessingHelpTagSI>
 * <xref DisplayingHelpSI>
 * <xref PrintingHelpTopicsSI>
<\list>
<!-- IFDEF ONLINE ONLY END -->



&headerFix;

		<idx|command summary|
		<idx|summary: commands|






<s1 id=ProcessingHelpTagSI>Processing &TagAppName; Files (`dthelptag')

		<idx|``dthelptag'' command|
		<idx|command: ``dthelptag''|

The &TagAppName; software, invoked with the ``dthelptag'' command, compiles
your &TagAppName; source files into run-time help files.  You run
``dthelptag'' in the directory where your ``%%volume%%.htg'' file is
located.

<procedure>Command Syntax

<ex>
dthelptag  [%%command-options%%]  %%volume%%  [%%parser-options%%]
<\ex>

Where %%command-options%% are options entered before the %%volume%% name and
%%parser-options%% are options entered after the %%volume%% name.  

<procedure>Command Options

		<idx|``clean'' parameter|
		<idx|parameter: ``clean''|
		<idx|``shortnames'' parameter|
		<idx|parameter: ``shortnames''|
		<idx|``verbose'' parameter|
		<idx|parameter: ``verbose''|

<lablist>

 \ ``-clean'' \	   Removes all files generated from any previous run of
                   &TagAppName; for the given %%volume%%.

 \ ``-shortnames'' \ Causes the names of all generated files to be limited
                     to a maximum of eight characters for the base name and
		     three characters for the extension.  This allows
		     run-time help files to be moved to systems where
		     longer names may not be supported.

 \ ``-verbose'' \  Displays the progress of the ``dthelptag'' command and
                   displays any parser errors that occur.  Parser errors
                   are also saved in a file named ``%%volume%%.err''.

<\lablist>

<procedure>Parser Options

Parser options, which are entered after the %%volume%% name, are passed
directly to the <term nogloss|parser|, which is the part of the &TagAppName;
software that converts your marked-up files into run-time files.

Any of these options can be either entered on the command line after the
%%volume%% name, or listed in a file named ``helptag.opt'' located
in the current directory:

		<idx|``onerror'' parameter|
		<idx|parameter: ``onerror''|
		<idx|``charset'' parameter|
		<idx|parameter: ``charset''|
		<idx|``search'' parameter|
		<idx|parameter: ``search''|
		<idx|``clearsearch'' parameter|
		<idx|parameter: ``clearsearch''|
		<idx|``memo'' parameter|
		<idx|parameter: ``memo''|
		<idx|``nomemo'' parameter|
		<idx|parameter: ``nomemo''|

<lablist>

 \ ``onerror'' \   Specifies whether the ``dthelptag'' command should continue
		   if a parser error is encountered.  The default is
		   ``onerror=stop'', which causes the command to stop even
                   if one parser error is encountered.  If
		   you specify ``onerror=go'', processing will continue,
		   but the created run-time help files may not
		   work properly.

 \ ``charset'' \   Specifies which character set was used to author the text
		   files.  The correct character set name is needed to
                   ensure that the help topics are displayed in the proper
                   font.  The default is ``charset=iso8859-1''.

		   You can also specify a character set within your help
		   volume by declaring an entity named
		   "LanguageElementDefaultCharset". (This is demonstrated
		   in the ``/opt/dt/dthelp/dthelptag/helplang.ent'' file.)  See
		   also <xref ChapterNativeLanguageSupport>.

 \ ``search'' \	   Adds another directory to the list of directories that
		   are searched to find referenced file entities.  To
		   specify multiple directories, use multiple
		   ``search=%%directory%%'' options.  If no search options
		   are used, only the current directory is searched.

 \ ``clearsearch'' \ Clears the list of search directories.  This option is
                   useful in the command line to override search options
                   specified in the ``helptag.opt'' file.

 \ ``memo'' \	   Causes author's memos (which are entered using the
		   ``&<memo>'' element) to be included.	 The default is
		   ``nomemo'', which causes &TagAppName; to ignore memos.

 \ ``nomemo'' \	   Causes &TagAppName; to ignore author's memos (which are entered
                   with the ``&<memo>'' element).  This is the default.

<\lablist>


<procedure>See Also

<list bullet tight>
 * <xref CreatingRunTimeHelpFiles>
 * <xref GatheringRunTimeHelpFilesSI>
 * <xref ViewingHelpVolume>
<\list>





<s1 id=DisplayingHelpSI>Displaying Help Topics (`dthelpview')

		<idx|``dthelpview'' command|
		<idx|command: ``dthelpview''|

<procedure>Command Syntax

Here are the various ways to invoke &ViewerAppName;:
<ex>
dthelpview  -helpVolume  %%volume%% [ -locationId %%id%% ] &&

dthelpview -man &&

dthelpview -manPage %%man%% &&

dthelpview -file %%filename%% &&
<\ex>

Where:

<lablist width=1.5in>

		<idx|``helpVolume'' parameter|
		<idx|parameter: ``helpVolume''|

\ ``-helpVolume %%volume%%'' \ Specifies the name of the ``%%volume%%.sdl''
			       file you want to view.  A path is not
			       required unless the volume is not in the
			       current directory !!and!! the volume has not
			       been "registered."

\ ``-locationId %%id%%'' \ Specifies an ID.  &ViewerAppName; displays the topic
			   that contains %%id%%.  If you do not specify an
			   ID, &ViewerAppName; uses ``_hometopic'' by default.

\ ``-man'' \ Displays a dialog that prompts for a man page to view,
             then displays the requested man page.

\ ``-manPage %%man%%''\ Specifies that a particular man page be displayed.

\ ``-file %%filename%%''\ Specifies that a particular text file be displayed.

<\lablist>

The default %%volume%% and %%id%% can be set in dthelpview's app-defaults
file (``/opt/dt/app-defaults/C/Dthelpview'').

<procedure>See Also

<list bullet tight>
 * <xref RegisteringYourHelpSI>
 * <xref ViewingHelpVolume>
<\list>






<s1 id=PrintingHelpTopicsSI>Printing Help Topics (`dthelpprint')

		<idx|``dthelpprint'' command|
		<idx|command: ``dthelpprint''|


<note>
Printing online help is disabled in the &DesktopName;
software snapshot (October 1993).  If you select the Print
option in a Help window, a message states that printing is
currently unavailable. 
<\note>

<!-- COMMENT OUT TILL PRINTING IS IMPLEMENTED

The &ProductName; uses the Dthelpprint application to print help topics.
Dthelpprint can be run manually (with the ``dthelpprint'' command) or directly
from &ViewerAppName; (by choosing Print from the File menu).


A second printing program is provided for printing help volumes that
contain multi-byte characters (such as Japanese or Korean).  The
``helpprintrst'' command operates just like the ``helpprint'' command
except that its output does not depend on printer fonts.  Instead,
``helpprintrst'' creates a page-size graphic image of each help topic.

Both printing programs are in the ``/usr/vhelp/bin/'' directory.  If you
are using HP VUE 3.0, they are also in the ``/usr/vue/bin/'' directory.

<procedure>Command Syntax

<ex>
helpprint -helpVolume %%volume%% [ -locationId %%id%% ] [ -R ]

<\ex>

Where:

		<idx|``helpVolume'' parameter|
		<idx|parameter: ``helpVolume''|

<lablist width=1.75in>

\ ``helpVolume'' %%volume%% \	Specifies a full path to the help
                volume (``.sdl'' file) that contains the topics to be
                printed.  This parameter is required.

\ ``-locationID'' %%id%% \ Identifies the topic to print.
                The default topic is "``_hometopic''" &emdash; which applies
                if this parameter is not specified.

\ ``-R'' \	Recursively prints all the subtopics that are beneath
                the topic specified in the ``-locationID'' parameter.
                If the ``-R'' parameter is not used, only the specified
                topic is printed.

<\lablist>

<procedure>Examples

Each of the following commands prints topics from the &BookName; help volume
(which is the online version of this guide):

To print just the copyright topic:

<ex smaller>
dthelpprint -helpVolume /opt/dt/dthelp/help/C/HelpKit/online.sdl -locationId _copyright
<\ex>

To print the entire volume:

<ex smaller>
dthelpprint -helpVolume /opt/dt/dthelp/help/C/HelpKit/online.sdl -R
<\ex>

<need begin>

To print the "Command Summary" section and all its subtopics:

<ex smaller>
dthelpprint -helpVolume /opt/dt/dthelp/help/C/HelpKit/online.sdl -locationId CommandSummary -R
<\ex>

<need end>
-->

<procedure>See Also

<list bullet tight>
 * <xref ProcessingHelpTagSI>
 * <xref ViewingHelpVolume>
<\list>


