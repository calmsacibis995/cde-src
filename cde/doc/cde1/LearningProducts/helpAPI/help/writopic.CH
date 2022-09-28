<chapter id=ChapterWritingHelpTopic> Writing a Help Topic
&headerFix;

		<idx|writing: topic|
		<idx|topic: writing|
		<idx|title: topic|
		<idx|element tag: ``&<chapter>''|
		<idx|``&<chapter>''|
		<idx|element tag: ``&<s1>'' .&empty;.&empty;. ``&<s9>''|
		<idx|``&<s1>'' .&empty;.&empty;. ``&<s9>''|


Each topic you write should have an element that marks the
start of the topic:

<ex>
&<%%element%% id=%%id%%> %%Topic Title%%
%%The body of the topic%%
<\ex>

Where %%element%% is one of the following: ``chapter'', ``s1'', ``s2'',
..., ``s9''.  The body of the topic may begin on any line after the
%%Topic Title%%.

The topic's position within the topic hierarchy is determined by the
%%element%% used to start the topic and by the %%element%% used to
start the immediately preceding topic.
For example, a topic that starts with ``&<s2>'' and
immediately follows a topic that starts with ``&<s1>'' makes the
``&<s2>'' topic a subtopic of the ``&<s1>'' topic.

The %%id%% is required if the topic is to be accessed either from the
application (if you are writing application help) or from a hyperlink.

		<idx|starting new line within title|
		<idx|new line: within title|
		<idx|line break: within title|
		<idx|title: line break within|

The %%Topic Title%% can be any string.  If the title string occupies more
than one line in your source file, end all but the last line with an
ampersand (``&&'').  To force a line break at a particular place within the
%%Topic Title%%, use a backslash character (``&\'').

<procedure>Example

The following line marks the start of a topic using the ``&<s1>'' tag:

<ex smaller>
&<s1 id=welcome>Welcome to My Application
<\ex>

To force the title to be displayed on two lines, you use a backslash
(``&\'') like this:

<ex smaller>
&<s1 id=welcome>Welcome to &\ My Application
<\ex>


<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref organizing-and-writing-a-volume> describes the general structure
   of a help volume, including how to create a topic hierarchy.
<\list>



<s1 id=CreatingStructureWithinTopicSI > Creating Structure Within a Topic

<!-- IFDEF ONLINE ONLY -->
<!-- &CreatingStructureWithinTopicSISubtopics; -->
<list bullet tight>
* <xref ToStartParagraph>
* <xref ToEnterList>
* <xref ToProvideSubheadingsWithinTopic>
* <xref ToShowComputerListingTA>
* <xref ToAddNoteCautionOrWarning>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|creating: structure within topic|
		<idx|structure within topic, creating|
		<idx|topic: creating structure within|



<!-- SEE ALSO -->

<!-- hardcopy introduction -->

Within the body of a help topic, you have the following elements to choose
from to organize and present your information:

<list bullet>

 * <term nogloss|Paragraphs| are used for bodies of text.

 * <term nogloss|Lists| are used for itemized information.  There are
   several types of lists including bulleted, ordered (numbered), and
   plain.

 * <term nogloss|Subheadings| let you partion sections within a topic.

 * <term nogloss|Graphics| can be included within your text as !!inline
   graphics!! or displayed between paragraphs as stand-alone !!figures!!.

 * <term nogloss|Hyperlinks| provide references to related topics.  A
   hyperlink may lead to a subtopic, deeper in the hierarchy, or branch to
   a topic in a completely different part of the hierarchy, or even in
   another help volume.

 * <term nogloss|Computer literals| are computer-recognized text, such as
   file names and variable names, that can be displayed as either separate
   example listings or inline elements.

 * <term nogloss|Notes, cautions, and warnings| call the reader's attention
   to important information.

 * <term nogloss|Emphasis| enables you to highlight important words and
   phrases within paragraph text.

<\list>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->


<!-- nothing needed here online. -->

<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToStartParagraph > To start a paragraph

		<idx|starting paragraph|
		<idx|paragraph: starting|
		<idx|``&<p>''|
		<idx|element tag: ``&<p>''|
		<idx|``&<image>''|
		<idx|element tag: ``&<image>''|
		<idx|``indent'' parameter|
		<idx|parameter: ``indent''|


<list bullet>

 * Insert a blank line after the previous paragraph or other element.

 * !!Or,!! use the ``&<p indent>'' element and parameter if the paragraph
   is to be indented.

 * !!Or,!! use the ``&<image>'' element if you want the paragraph to
   maintain the line breaks that you enter in your source file.

<\list>

An end tag for ``&<p>'' is not required.  However, the ``&<\image>'' end
tag !!is!! required with the ``&<image>'' element.

<procedure>Examples

Here are two paragraphs, separated by a blank line.  Because neither
paragraph has any special parameters, the ``&<p>'' tag does not have to be
entered (it is assumed when you enter one or more blank lines):

<ex smaller>
Many people prefer online help over printed
manuals because finding information can be
done more quickly.  For those people, the &HelpSystemName;
works well on screen.

For people who prefer to read a printed page,
&HelpSystemName; offers full WYSIWYG printing.  Printed
output includes multi-font text formatting and
graphics.
<\ex>

If you want a paragraph indented from the left margin, include the optional
``indent'' parameter:

<ex smaller>
&<p indent>An indented paragraph can be used
to draw the reader's attention to an idea.
<\ex>

The following paragraph overrides the automatic word wrap in help windows
and maintains the line breaks exactly as entered in the source file.  The
``&<image>'' element is especially useful for entering addresses.

<ex smaller>
&<image>
Brown and Reed Software Investors
100 Baltic Place  Suite 40
New York, New York
&<\image>
<\ex>



<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ToWrapTextAroundGraphicTA> later in this chapter shows how to
   include a graphic with a paragraph.
<\list>



<s2 id=ToEnterList > To enter a list

<need begin>

		<idx|entering: list|
		<idx|list: entering|
		<idx|list: ``&<list>''|
		<idx|``&<list>''|
		<idx|element tag: ``&<list>''|
		<idx|list: ``&<lablist>''|
		<idx|``&<lablist>''|
		<idx|element tag: ``&<lablist>''|
		<idx|list: ``&<labheads>''|
		<idx|``&<labheads>''|
		<idx|element tag: ``&<labheads>''|
		<idx|asterisk (*)|
		<idx|``*'' (asterisk)<sort>*|
		<idx|parameter: ``bullet''|
		<idx|parameter: ``order''|
		<idx|parameter: ``plain''|
		<idx|parameter: ``loose''|
		<idx|parameter: ``tight''|
		<idx|``bullet'' parameter|
		<idx|``order'' parameter|
		<idx|``plain'' parameter|
		<idx|``loose'' parameter|
		<idx|``tight'' parameter|


<list bullet>
 * Use the ``&<list>'' element as shown:

<ex>
&<list %%type%% %%spacing%%>
 &empty;* %%item%%
 &empty;* %%item%%
    &vellipsis;
 &empty;* %%item%%
&<\list>
<\ex>

Where %%type%% indicates the type of list you want: ``bullet'' (default),
``order'', or ``plain''; and %%spacing%% is ``loose'' (default) or
``tight''.  Each !!item!! in the list is marked with an asterisk (``*'').

<need end>

 * !!Or!!, to create a labeled list without headings, use the ``&<lablist>'' element as shown:

<ex>
&<lablist %%spacing%%>
  \%%label 1%%\ %%item 1 text%%
  \%%label 2%%\ %%item 2 text%%
    &vellipsis;
  \%%label N%%\ %%item N text%%
&<\lablist>
<\ex>

Where %%spacing%% is ``loose'' (default) or ``tight''.

 * !!Or!!, to create a labeled list with headings, use the ``&<lablist>''
and ``&<labheads>'' elements as shown:

<ex>
&<lablist %%spacing%%>
  &<labheads> \ %%heading for labels%% \ %%heading for items%%
  \%%label 1%%\ %%item 1 text%%
  \%%label 2%%\ %%item 2 text%%
    &vellipsis;
  \%%label N%%\ %%item N text%%
&<\lablist>
<\ex>

<\list>

<need begin>

<procedure>Examples

Here's a simple list.  Because the %%type%% isn't specified, it defaults to a
bulleted list.  Because %%spacing%% isn't specified, it defaults to loose,
which leaves a blank line between each item.

<ex smaller>
&<list>
* chocolate
* raspberry
* vanilla
&<\list>
<\ex>

To format the same list without bullets or numbers and with reduced spacing
between items, you would use:

<ex smaller>
&<list plain tight>
* chocolate
* raspberry
* vanilla
&<\list>
<\ex>

Here's a list of labeled chapter descriptions.  The optional label
headings are not provided.

<ex smaller>
&<lablist tight>
  &\Chapter 1\ An overview of the system.
  &\Chapter 2\ How to install the system.
  &\Chapter 3\ How to use the system.
  &\Appendix A\ A quick-reference description of all system features.
&<\lablist>
<\ex>

<need end>



<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref list-ReferenceSection> summarizes the use of the
   ``&<list>'' element and provides additional examples.
 * <xref lablist-ReferenceSection> summarizes the use of the
   ``&<lablist>'' and ``&<labheads>'' elements and provides
   additional examples.
<\list>





<s2 id=ToProvideSubheadingsWithinTopic > To provide subheadings within a topic

<need begin>

		<idx|providing: subheadings within topic|
		<idx|subheading, within topic|
		<idx|topic: providing subheadings within topic|
		<idx|``&<otherhead>''|
		<idx|element tag: ``&<otherhead>''|
		<idx|``&<procedure>''|
		<idx|element tag: ``&<procedure>''|

<list bullet>

 * For midsize headings (slightly smaller than the topic title),
   use the following markup:

<ex>
&<otherhead> %%Heading%%
<\ex>

 * !!Or,!! for small headings, use the following markup:

<ex>
&<procedure> %%Heading%%
<\ex>

<\list>

Subheadings add structure !!within!! a topic, but they do not
affect the topic hierarchy.

<need end>


<procedure>Example

Here the ``&<procedure>'' element is used to add a small heading just
before each paragraph.

<ex smaller>
&<procedure>Online Help is Preferred by Many

Many people prefer online help over printed
manuals because finding information can be
done more quickly.  For those people, &HelpSystemName;
works well on screen.

&<procedure>WYSIWYG Printing!

For people who prefer to read a printed page,
&HelpSystemName; offers full WYSIWYG printing.  Printed
output includes multi-font text formatting and
graphics.
<\ex>

<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hwtsubh.SA -->
<!-- SEE ALSO ONLINE -->

<list bullet tight>
 * <xref otherhead-ReferenceSection>
 * <xref procedure-ReferenceSection>
<\list>

<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToShowComputerListingTA > To show a computer listing

<need begin>

		<idx|showing computer listing|
		<idx|computer: listing, showing|
		<idx|listing: computer, showing|
		<idx|element tag: ``&<ex>''|
		<idx|``&<ex>''|
		<idx|element tag: ``&<vex>''|
		<idx|``&<vex>''|


<list bullet>

 * For computer listings that !!do not!! contain any special character
   sequences that will be interpreted as &TagAppName; markup, use the ``&<ex>''
   (!!example!!) element as show:

<ex>
&<ex %%size%%>
%%Computer text here.%%
&<\ex>
<\ex>

 * For computer listings that !!do!! contain special character sequences
   used by &TagAppName;, use the ``&<vex>'' (!!verbatim example!!) element as
   shown:

<ex>
&<vex %%size%%>
%%Computer text here.%%
&<\vex>
<\ex>

<\list>

The optional %%size%% attribute, which 
determines the size of the font used to display the example,
can be specified as ``smaller'' or ``smallest''.

Line breaks appear where you enter them in your source file.  If
the example is too wide for the help window, a horizontal scroll bar
appears so the reader can scroll to see all the example text.

<need end>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<list bullet tight>
 * <xref ToShowComputerListingTA>
 * <xref ToDisplayVariableTA>
<\list>




<!-- IFDEF ONLINE ONLY END -->

<!-- seealsos/Hwtcolst.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ToDisplayComputerLiteralTA>
 * <xref ex-ReferenceSection>
 * <xref vex-ReferenceSection>
<\list>



<s2 id=ToAddNoteCautionOrWarning > To add a note, caution, or warning

<need begin>

		<idx|adding: note, caution, or warning|
		<idx|note, adding|
		<idx|caution, adding|
		<idx|warning: adding|
		<idx|element tag: ``&<note>''|
		<idx|``&<note>''|
		<idx|element tag: ``&<caution>''|
		<idx|``&<caution>''|
		<idx|element tag: ``&<warning>''|
		<idx|``&<warning>''|
		<idx|predefined entities|
		<idx|entity: predefined names|
		<idx|``NoteElementDefaultIconFile'' entity|
		<idx|``CautionElementDefaultIconFile'' entity|
		<idx|``WarningElementDefaultIconFile'' entity|
		<idx|entity: ``NoteElementDefaultIconFile''|
		<idx|entity: ``CautionElementDefaultIconFile''|
		<idx|entity: ``WarningElementDefaultIconFile''|
		<idx|emphasizing: using note, caution, or warning|


<list bullet>

 * Include the ``&<note>'', ``&<caution>'', or ``&<warning>'' element as
   follows:

<ex>
&<note>
%%Body of note here.%%
&<\note>

&<caution>
%%Body of caution here.%%
&<\caution>

&<warning>
%%Body of warning here.%%
&<\warning>
<\ex>

<\list>

<need end>

To associate an icon with the note, caution, or warning
element, define a file entity that
identifies the graphics file containing the icon.  Use one of the
predefined entity names:

<ex>
&<!ENTITY NoteElementDefaultIconFile    FILE "%%filename%%">
&<!ENTITY CautionElementDefaultIconFile FILE "%%filename%%">
&<!ENTITY WarningElementDefaultIconFile FILE "%%filename%%">
<\ex>

If you do not want icons with notes, cautions, or warnings, simply don't
declare the corresponding entities.  (Remember, all entity declarations
must come before any other markup at the beginning of your help volume.)
If you include such an entity reference, be sure the graphics file
is in your &TagAppName;
search path (``helptag.opt'').  Sample icons are provided in
``/opt/dt/dthelp/dthelptag/icons/''. 

If you create your own icon images for notes, cautions, and warnings,
be sure to keep them small so they will fit into the area allocated.


<need begin>

<procedure>Example

If you declare the following entity:

<ex smaller>
&<!ENTITY CautionElementDefaultIconFile  FILE  "cauticon.pm">
<\ex>

and include the following note and caution in your help volume:

<ex smaller>
&<note>
  Pay attention -- this is important.
&<\note>

&<caution>
  Don't try this at home!
&<\caution>
<\ex>

then the note is displayed !!without!! an icon, and the warning is
displayed using the ``cauticon.pm'' graphic as its icon.
(The ``cauticon.pm'' file must be in
your &TagAppName; search path, which is specified in your ``helptag.opt'' file.)

<need end>


<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<list bullet tight>
 * <xref ToShowComputerListingTA>
 * <xref ToDisplayVariableTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->

<!-- seealsos/Hwtcaut.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ToRunHelpTagTA> includes information about using a ``helptag.opt''
   file.
 * <xref UsingEntitiesSI>
<\list>



<s1 id=EnteringInlineElements > Entering Inline Elements

<!-- IFDEF ONLINE ONLY -->
<!-- &EnteringInlineElementsSubtopics; -->
<list bullet tight>
* <xref ToEmphasizeWordOrPhrase>
* <xref ToEnterABookTitleTA>
* <xref ToDisplayComputerLiteralTA>
* <xref ToDisplayVariableTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|entering: inline elements|
		<idx|inline elements, entering|
		<idx|element: entering inline|


Inline elements are used to mark words or phrases within a paragraph of text.
These elements affect the font used to format particular items.



<s2 id=ToEmphasizeWordOrPhrase > To emphasize a word or phrase

		<idx|emphasizing: word or phrase|
		<idx|word, emphasizing|
		<idx|phrase, emphasizing|
		<idx|``&<emph>''|
		<idx|element tag: ``&<emph>''|
		<idx|italic font|
		<idx|font: italic|


<list bullet>

 * Use the ``&<emph>'' element (!!emphasis!!) as shown:

<ex>
&<emph>%% text %%&<\emph>
<\ex>

!!Or,!! use the shorthand form:
<ex>
!!%% text %%!!
<\ex>

<\list>

Emphasized text is displayed using an italic font.

<procedure>Example

Here's how you might emphasize an important word:

<ex smaller>
A thousand times &<emph>no&<\emph>
<\ex>

Or, using the shorthand form:

<ex smaller>
A thousand times !!no!!
<\ex>

In both cases, the word "no" is displayed in italics.



<s2 id=ToEnterABookTitleTA > To enter a book title

<need begin>

		<idx|book title: entering|
		<idx|title: entering|
		<idx|``&<book>''|
		<idx|element tag: ``&<book>''|
		<idx|italic font|
		<idx|font: italic|


<list bullet>

 * Use the ``&<book>'' element as shown:

<ex>
&<book>%% title %%&<\book>
<\ex>

!!Or,!! use the short form:

<ex>
&<book|%% title %%|
<\ex>

<\list>

Book titles are displayed using an italic font.

<procedure>Example

Here's how you would enter the title of this guide:

<ex smaller>
&<book|The &HelpSystemName; System Developer's Guide|
<\ex>

<need end>


<s2 id=ToDisplayComputerLiteralTA > To display a computer literal

		<idx|displaying: computer literal|
		<idx|computer: literal, displaying|
		<idx|literal, displaying computer|
		<idx|quotation mark: repeated single|
		<idx|``&<computer>''|
		<idx|element tag: ``&<computer>''|
		<idx|font: for computer literal|


<list bullet>

 * Use the ``&<computer>'' element as shown:

<ex>
&<computer>%% text %%&<\computer>
<\ex>

   !!Or,!! use the shorthand form:

<ex>
<esc|``|%% text %%<esc|''|
<\ex>

<\list>

<procedure>Example

Computer text is useful for identifying a file name.  Here the
``helptag.opt'' file name is marked to be displayed in computer text.

<ex smaller>
Add the search path to your <esc|``|helptag.opt<esc|''| file.
<\ex>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<list bullet tight>
 * <xref ToShowComputerListingTA>
 * <xref ToDisplayVariableTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->

<!-- seealsos/Hwtcolit.SA -->
<!-- SEE ALSO -->



<s2 id=ToDisplayVariableTA > To display a variable

<need begin>

		<idx|displaying: variable|
		<idx|variable: displaying|
		<idx|``&<var>''|
		<idx|element tag: ``&<var>''|
		<idx|percent symbol (``%'')|
		<idx|``%'' (percent symbol)<sort>%|
		<idx|italic font|
		<idx|font: italic|


<list bullet>

 * Use the ``&<var>'' element (!!variable!!) as shown:

<ex>
&<var>%% text %%&<\var>
<\ex>

   !!Or,!! use the short form:

<ex>
&<var<esc>|<\esc>%% text %%<esc>|<\esc>
<\ex>

   !!Or,!! use the shorthand form:

<ex>
<esc|%%|%% text %%<esc|%%|
<\ex>

<\list>

Variables are displayed using an italic font.

<procedure>Example

Here's a variable within a normal sentence.

<ex smaller>
When you enter your <esc|%%|password<esc|%%|, the
computer unlocks your screen and keyboard.
<\ex>

Variables can appear within computer text or computer example
listings.  This example specifies "%%volume%%" as a variable part of a file
name:

<ex smaller>
The &TagAppName; software takes your 
<esc|``%%|volume<esc|%%|.htg<esc|''| as input.
<\ex>

In both of these examples, the ``<esc|%%|'' pairs could have been entered
with the long form (``&<var>'' ... ``&<\var>'') or the short form
(``&<var|'' ... ``|'').

<need end>





<s1 id=CreatingHyperlinksSI > Creating Hyperlinks

<!-- IFDEF ONLINE ONLY -->
<!-- &CreatingHyperlinksSISubtopics; -->
<list bullet tight>
* <xref ToCreateJumpLinkTA>
* <xref ToCreateDefinitionLinkTA>
* <xref ToCreateManPageLinkTA>
* <xref ToCreateAnExecutionLinkTA>
* <xref ToCreateAnApplicationDefinedLinkTA>
* <xref ToLinkToAMetaInfoTopicTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->

		<idx|creating: hyperlink|
		<idx|hyperlink: creating|
		<idx|hyperlink: types|
		<idx|hyperlink: jump type|
		<idx|hyperlink: definition link|
		<idx|hyperlink: man page link|
		<idx|hyperlink: execution link|
		<idx|hyperlink: application-defined link|
		<idx|jump type hyperlink|
		<idx|definition hyperlink|
		<idx|man page: hyperlink|
		<idx|execution hyperlink|
		<idx|application-defined hyperlink|


&HelpSystemName; supports five types of hyperlinks:

<list bullet>

 * !!Traditional hypertext!! to "jump" to another help topic.  By default
   the new topic is displayed in the same window, but you
   may request that the new topic be displayed in a new window.

 * !!Definition links!! to display a topic in a simple pop-up help
   window.  Most frequently, definition links are used to access the
   definition of a new term or phrase used within a sentence.

 * !!Man page links!! to display any man page installed on the system.

 * !!Execution links!! to execute a shell command or program.	 This
   greatly expands the possibilities for what happens when the user
   activates a hyperlink.

 * !!Application-defined links!! to create
   custom links that the application interprets.  This provides
   facilities for communication between the help system and the
   application.	 

<\list>



<s2 id=ToCreateJumpLinkTA > To create a "jump" link

		<idx|creating: jump link|
		<idx|jump link, creating|
		<idx|link: creating jump|
		<idx|``&<link>''|
		<idx|element tag: ``&<link>''|
		<idx|``JumpNewView'' parameter|
		<idx|parameter: ``JumpNewView''|
		<idx|``&<xref>''|
		<idx|element tag: ``&<xref>''|
		<idx|parameter: ``hyperlink''|
		<idx|``hyperlink'' parameter|


<list bullet>

 * To jump to a topic within the same volume, use the ``&<link>'' element as
   shown:

<ex>
&<link %%id%%>%%text%%&<\link>
<\ex>

   Where %%id%% is an ID declared somewhere in the help volume, and
   %%text%% is the portion of your help text that is underlined to
   indicate it is an active hyperlink.

 * !!Or,!! to jump to a topic (within the same volume) that has a predefined ID,
   use the ``&<link>'' element as shown:

<ex>
&<link hyperlink="%%id%%">%%text%%&<\link>
<\ex>

   All the predefined IDs start with an underscore character (``_''),
   so this makes it necessary to use the ``hyperlink="%%id%%"'' form.


 * !!Or,!! to jump to a topic in another help volume:

<ex>
&<link hyperlink="%%volume%% %%id%%" JumpNewView>%%text%%&<\link>
<\ex>

If the other volume is "registered," the %%volume%% parameter is just the
base name of the volume file.  If the volume is not registered, you must
include a complete path to the volume's ``%%volume%%.sdl'' file.

The ``JumpNewView'' parameter is recommended for links to other volumes so
that the readers realize that they have jumped into another volume.
The previous view remains displayed so they can see where they came from.

 * !!Or,!! if you are linking to an element with a title, use the ``&<xref>''
   element as shown:

<ex>
&<xref %%id%%>
<\ex>

When you use ``&<xref>'' to create a link, the title of the topic with the
ID of %%id%% is inserted in place of the ``&<xref>'' element and becomes
the active hyperlink.

Also, ``&<xref>'' always creates a standard "jump" link; other types of
links must be created using the ``&<link>'' element.

<\list>


<note>
   You !!cannot!! use ``&<xref>'' to jump to topics that have built-in IDs
   (such as ``&<hometopic>'' or ``&<glossary>'').  To create a hyperlink to
   any of those elements, you must use the ``&<link>'' element.
<\note>

<procedure>Examples

Here's a simple hyperlink to the topic with the ID "Welcome." (Notice that
capitalization of the ID is not significant.)

<ex smaller>
This is a &<link welcome>simple jump&<\link> link.
<\ex>

Here's the same link, but the title of the ``Welcome'' topic is inserted
using an ``&<xref>'':

<ex smaller>
Refer to the &<xref welcome> topic.
<\ex>

<need begin>

Here's a link to the same topic, but this one requests a new window:

<ex smaller>
This is a &<link welcome JumpNewView>new-view jump&<\link> link.
<\ex>

<need end>

This link jumps to the home topic of the current volume:

<ex smaller>
Return to &<link hyperlink="_hometopic">Introduction&<\link>.
<\ex>

This link jumps to the home topic of the ``FileMgr'' help volume: 

<ex smaller>
Return to &<link hyperlink="FileMgr _hometopic">Introduction&<\link>.
<\ex>

<!-- seealsos/Hwtjump.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref link-ReferenceSection>
 * <xref xref-ReferenceSection>
<\list>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateDefinitionLinkTA>
 * <xref ToCreateManPageLinkTA>
 * <xref ToCreateAnExecutionLinkTA>
 * <xref ToCreateAnApplicationDefinedLinkTA>
 * <xref ToLinkToAMetaInfoTopicTA>
 * <xref link-ReferenceSection>
 * <xref xref-ReferenceSection>
<\list>

<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToCreateDefinitionLinkTA > To create a definition link

		<idx|creating: definition link|
		<idx|definition link: creating|
		<idx|link: creating definition|
		<idx|element tag: ``&<term>''|
		<idx|``&<term>''|
		<idx|plus (``+'')|
		<idx|``+'' (plus)<sort>+|
		<idx|``&<link>''|
		<idx|element tag: ``&<link>''|


<list bullet>

 * If you are linking to a term in the Glossary, use the
   ``&<term>'' element as shown:

<ex>
&<term>%%text%%&<\term>
<\ex>

   !!Or,!! use the shorthand form:

<ex>
<esc|++|%%text%%<esc|++|
<\ex>

Whenever you use the ``&<term>'' element, be sure you include the
corresponding definition in the Glossary.

 * !!Or!!, if you are linking to a topic within the same help volume, use the
   ``&<link>'' element as shown: 

<ex>
&<link %%id%% Definition>%%text%%&<\link>
<\ex>

Where %%id%% is a topic ID (or the ID of an element within the topic) and
%%text%% is the portion of your help text that you want to be the active
hyperlink.  The ``Definition'' keyword specifies that the link should
pop-up a quick help dialog.

<need begin>

 * !!Or,!! if you are linking to a topic in another help volume, use the
   ``&<link>'' element as shown: 

<ex>
&<link hyperlink="%%volume%% %%id%%" Definition>%%text%%&<\link>
<\ex>

   If the other volume is "registered," the %%volume%% parameter is just
   the base name of the volume file.  If the volume is not registered, you
   must include a complete path to the volume's ``%%volume%%.sdl'' file.

<need end>

<\list>

<procedure>Example

The following link creates a definition link that displays the copyright
topic in the meta information:

<ex smaller>
&<link hyperlink="_copyright" type=Definition>Version Information&<\link>
<\ex>

The phrase "Version Information" becomes the (underlined) hyperlink text.

<!-- seealsos/Hwtdeflk.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref CreatingGlossarySI> later in this chapter.
 * <xref term-ReferenceSection>
 * <xref link-ReferenceSection>
<\list>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref CreatingGlossarySI>
 * <xref ToCreateJumpLinkTA>
 * <xref ToCreateManPageLinkTA>
 * <xref ToCreateAnExecutionLinkTA>
 * <xref ToLinkToAMetaInfoTopicTA>
 * <xref ToCreateAnApplicationDefinedLinkTA>
 * <xref link-ReferenceSection>
 * <xref term-ReferenceSection>
<\list>

<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToCreateManPageLinkTA > To create a man page link

		<idx|creating: man page link|
		<idx|man page: creating link|
		<idx|page, man: creating link|
		<idx|link: creating man page|
		<idx|``&<link>''|
		<idx|element tag: ``&<link>''|
		<idx|``hyperlink'' parameter|
		<idx|parameter: ``hyperlink''|
		<idx|``Man'' parameter|
		<idx|parameter: ``Man''|


<list bullet>
 * Use the ``&<link>'' element as shown:

<ex>
&<link %%manpage%% Man>%%text%%&<\link>
<\ex>

   To request a man page from a particular "section," use the
   ``hyperlink'' parameter like this:

<ex>
&<link hyperlink="%%section%% %%manpage%%" Man>%%text%%&<\link>
<\ex>

<\list>

For man page links, the ``hyperlink'' parameter is the same string you
would enter if executing the ``man'' command by hand.

<note>

If you are writing help for an application and you
include any man page
links, your application must include special support for man pages.  See
<xref ToDisplayAManPageTA>.
(The &ViewerAppName; application includes support for man page links.)

<\note>

<procedure>Example

Here's a link that displays the man page for the ``grep'' command:

<ex smaller>
Refer to the &<link grep Man>grep(1)&<\link> command.
<\ex>

"Man" is a keyword for the ``&<link>'' element,
so if you want to create a link that displays the
man page for the ``man'' command, you must use the ``hyperlink'' parameter:

<ex smaller>
Refer to the &<link hyperlink="man" Man>man(1)&<\link> command.
<\ex>

To display a man page in a particular section, precede the man page name
with the section number.  The following link displays the "mkdir" man page
from section 2 (which is different than the man page of the same name in
section 1):

<ex smaller>
Refer to the &<link hyperlink="2 mkdir" Man>mkdir(2)&<\link> command.
<\ex>

<!-- seealsos/Hwtmanpg.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref link-ReferenceSection>
<\list>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateJumpLinkTA>
 * <xref ToCreateDefinitionLinkTA>
 * <xref ToCreateAnExecutionLinkTA>
 * <xref ToCreateAnApplicationDefinedLinkTA>
 * <xref ToLinkToAMetaInfoTopicTA>
 * <xref link-ReferenceSection>
<\list>
<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToCreateAnExecutionLinkTA > To create an execution link

		<idx|creating: execution link|
		<idx|execution link, creating|
		<idx|link: creating execution|
		<idx|``&<link>''|
		<idx|element tag: ``&<link>''|
		<idx|parameter: ``hyperlink''|
		<idx|``hyperlink'' parameter|
		<idx|parameter: ``Execute''|
		<idx|``Execute'' parameter|
		<idx|ampersand (``&&'')|
		<idx|``&&'' (ampersand)<sort>&&|


<list bullet>

 * Use the ``&<link>'' element with the ``Execute'' parameter as shown:

<ex>
&<link hyperlink="%%command%% &" Execute>%%text%%&<\link>
<\ex>

Where %%command%% is the command string you want to execute and %%text%%
is the portion of your help text that you want to use as the
underlined hyperlink text.

<\list>

<note>

  If the command you are executing doesn't finish immediately, you should
  run it in the background by appending an ampersand (``&&'') to the
  command.  If you don't, the help window will not operate until the
  command finishes.

<\note>

<procedure>Example

The following link starts the ``xclock'' program running in the background:

<ex smaller>
&<link hyperlink="xclock &" Execute>Start the clock&<\link>
<\ex>

The phrase "Start the clock" becomes the underlined hyperlink text.

<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hwtexlnk.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateJumpLinkTA>
 * <xref ToCreateDefinitionLinkTA>
 * <xref ToCreateManPageLinkTA>
 * <xref ToCreateAnApplicationDefinedLinkTA>
 * <xref ToLinkToAMetaInfoTopicTA>
 * <xref link-ReferenceSection>
<\list>
<!-- IFDEF ONLINE ONLY END -->




<s2 id=ToCreateAnApplicationDefinedLinkTA > To create an application-defined link

		<idx|creating: application-defined link|
		<idx|application-defined: link, creating|
		<idx|link: creating application-defined|
		<idx|``&<link>''|
		<idx|element tag: ``&<link>''|
		<idx|parameter: ``hyperlink''|
		<idx|``hyperlink'' parameter|
		<idx|parameter: ``AppDefined''|
		<idx|``AppDefined'' parameter|


<list bullet>

 * Use the ``&<link>'' element with the ``AppDefined'' parameter as shown:

<ex>
&<link hyperlink="%%data%%" AppDefined>%%text%%&<\link>
<\ex>

Where %%data%% is a text string passed to the application when the link is
invoked, and %%text%% is the underlined hyperlink text.

<\list>

<procedure>Example

Suppose you are writing help for an application that prints three styles of
reports.  You might create three hyperlinks like this:

<ex smaller>
Choose a report type:

&<list plain tight>
  * &<link hyperlink="Report-Daily" AppDefined>Daily Report&<\link>
  * &<link hyperlink="Report-Month-To-Date" AppDefined>MTD Report&<\link>
  * &<link hyperlink="Report-Year-To-Date" AppDefined>YTD Report&<\link>
&<\list>
<\ex>

If your application is prepared to handle these special links and interpret
the hyperlink strings, it could generate the appropriate report based on
the hyperlink chosen by the user.

<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hwtaplnk.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref RespondingToHyperlinkEventsSI> describes how an application can
   provide support for application-defined hyperlinks.
 * <xref link-ReferenceSection> summarizes the use of the ``&<link>''
   element. 
<\list>
<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToLinkToAMetaInfoTopicTA>To link to a meta information topic

<need begin>

		<idx|linking to meta information topic|
		<idx|meta information: topic, linking to|
		<idx|topic: linking to meta information|
		<idx|``&<link>''|
		<idx|element tag: ``&<link>''|
		<idx|parameter: ``hyperlink''|
		<idx|``hyperlink'' parameter|


<list bullet>
 * Use the ``&<link>'' element as shown:

<ex>
&<link hyperlink="_%%id%%">%%text%%&<\link>
<\ex>

Where %%id%% is the predefined ID associated with the element you want to
link to and %%text%% is the word or phrase that you want to be the active
hyperlink.

<\list>

Most topics within the meta information section have predefined IDs,
so they !!do not!! allow author-defined IDs.  The predefined IDs
consist of the element name preceded by an underscore character.
For example, the ID for the ``&<copyright>''
topic is ``_copyright''. (Case is not significant).

Topics entered with the ``&<otherfront>'' element can be linked to just
like any normal topic in the topic hierarchy.

<need end>

<!-- seealsos/Hmetalnk.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ToAddAnIdToTopic> lists the predefined IDs for meta information
   topics.
<\list>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateJumpLinkTA>
 * <xref ToCreateDefinitionLinkTA>
 * <xref ToCreateAnExecutionLinkTA>
 * <xref ToCreateAnApplicationDefinedLinkTA>
 * <xref link-ReferenceSection>
 * <xref ToCreateMetaInformationSectionTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->




<s1 id=DisplayingGraphicsSI > Displaying Graphics

<!-- IFDEF ONLINE ONLY -->
<!-- &DisplayingGraphicsSISubtopics; -->
<list bullet tight>
* <xref ToCreateFigure>
* <xref ToDisplayAnInlineGraphic>
* <xref ToWrapTextAroundGraphicTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->


		<idx|displaying: graphics|
		<idx|graphic: displaying|
		<idx|Tagged Image File Format (TIFF)|
		<idx|TIFF (Tagged Image File Format)|
		<idx|X Window Dump|
		<idx|Window Dump|
		<idx|dump, X Window|
		<idx|pixmap|
		<idx|bitmap|


&HelpSystemName; supports four graphics formats:

<list bullet>

 * ++Tagged Image File Format (TIFF)++ -- Color, grayscale, and black and
   white images created by many standard drawing and scanning applications
   (``%%filename%%.tif'').

 * ++X Window Dump++ -- Screen dumps from the X Window System created with
   the ``xwd'' utility (``%%filename%%.xwd'').

 * ++X Pixmap++ -- Color icon images (``%%filename%%.pm''). 

 * ++X Bitmap++ -- Two-color icon images (``%%filename%%.bm'').

<\list>

Each graphic is maintained as a separate file.  The file format is
determined using the filename extensions listed above.



<s2 id=ToCreateFigure > To create a figure

		<idx|creating: figure|
		<idx|figure: creating|
		<idx|``&<figure>''|
		<idx|element tag: ``&<figure>''|
		<idx|caption: figure|
		<idx|figure: caption|
		<idx|number, figure|
		<idx|figure: number|
		<idx|``nonumber'' parameter|
		<idx|parameter: ``nonumber''|
		<idx|``entity'' parameter|
		<idx|parameter: ``entity''|
		<idx|``ghyperlink'' parameter|
		<idx|parameter: ``ghyperlink''|
		<idx|``glinktype'' parameter|
		<idx|parameter: ``glinktype''|
		<idx|ID: figure|
		<idx|figure: ID|


<list order>

 * Declare a file entity to identify the image file to be included in the
   figure.  

<ex>
&<!entity %%graphic-entity%%  FILE "%%filename%%.%%ext%%">
<\ex>

   Remember, all entity declarations must come before any other markup at
   the top of your help volume.

 * Use the ``&<figure>'' element as shown:

<ex>
&<figure entity=%%graphic-entity%%>
%%caption string%%
&<\figure>
<\ex>

Where %%graphic-entity%% is the entity name for the graphic file you want
to display, and %%caption string%% is an optional string to be displayed
above the graphic.

<\list>

By default, figures are numbered and the number is prepended to your 
%%caption string%%.  To create a non-numbered figure, include the
``nonumber'' parameter (as shown in one of the following examples).

If you want the figure to be a hyperlink, use the the ``ghyperlink''
(!!graphic hyperlink!!) and ``glinktype'' (!!graphic link type!!)
parameters as shown:

<ex smaller>
&<figure entity=%%graphic-entity%% ghyperlink="%%id%%" glinktype=%%type%%>
%%caption string%%
&<\figure>
<\ex>

The ``ghyperlink'' and ``glinktype'' parameters work just like the
``hyperlink'' and ``type'' parameters for the ``&<link>'' element.

<procedure>Examples

For these examples, assume that you've declared these two file entities at
the top of your help volume:

<ex smaller>
&<!entity FirstPicture   FILE  "first.tif">
&<!entity SecondPicture  FILE  "second.pm">
<\ex>

The following figure displays the graphic in the ``first.tif'' file
and displays a number (by default) and caption:

<ex smaller>
&<figure entity=FirstPicture>
Here's the First Picture
&<\figure>
<\ex>

Here's a figure that displays the ``second.pm'' file without a number or a
caption: 

<ex smaller>
&<figure nonumber entity=SecondPicture>
&<\figure>
<\ex>

If you add an ID to a figure, you must have a caption.  The caption is
needed in case an ``&<xref>'' uses the figure's ID,
in which case the caption is
inserted in place of the ``&<xref>'' and becomes a hyperlink to the figure.

The following figure is an execution hyperlink that runs the
``xclock'' program:

<ex smaller>
&<figure entity=SecondPicture ghyperlink="xclock &" glinktype=execute>
Choose This Figure to Start the Clock
&<\figure>
<\ex>


<!-- seealsos/Hwtfig.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref figure-ReferenceSection>
 * <xref link-ReferenceSection>
<\list>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref figure-ReferenceSection>
 * <xref ToDisplayAnInlineGraphic>
 * <xref ToWrapTextAroundGraphicTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToDisplayAnInlineGraphic > To display an inline graphic

		<idx|displaying: inline graphic|
		<idx|inline graphic, displaying|
		<idx|graphic: displaying inline|
		<idx|``&<graphic>''|
		<idx|element tag: ``&<graphic>''|


<list order>

 * Declare a file entity to identify the image file to be used in the
   figure.

<ex>
&<!entity %%graphic-entity%%  FILE "%%filename%%.%%ext%%">
<\ex>

   Remember, all entity declarations must come before any other markup at
   the top of your help volume.

 * Use the ``&<graphic>'' element as shown:

<ex>
... %%text%% &<graphic entity=%%graphic-entity%%> %%text%% ...
<\ex>

   Where %%graphic-entity%% is the entity name for the graphic file you
   want to display.

<\list>

To use a graphic as a hyperlink, place it inside a ``&<link>'' element:

<ex>
&<link %%parameters%%>&<graphic entity=%%graphic-entity%%>&<\link>
<\ex>

<note>
 The ``&<graphic>'' element is intended for small graphics, although
 larger images can be used.  Additional white space is added between
 lines to accomodate the height of the graphic.
<\note>

<procedure>Example

Here's an example that uses a small X pixmap image in the middle of a
sentence.  First, at the top of the volume, 
the pixmap file must be declared as a file entity:

<ex smaller>
&<!entity SmallStopSign  FILE "stopsign.pm">
<\ex>

Within the help text, the image is inserted using the ``&<graphic>''
element:

<ex smaller>
Whenever you see the &<graphic entity=SmallStopSign> symbol, 
stop and think about what you are doing.
<\ex>




<s2 id=ToWrapTextAroundGraphicTA > To wrap text around a graphic

		<idx|wrapping: text around graphic|
		<idx|text: wrapping around graphic|
		<idx|graphic: wrapping text around|
		<idx|``&<p>''|
		<idx|element tag: ``&<p>''|
		<idx|``gentity'' parameter|
		<idx|parameter: ``gentity''|


<list order>

 * Declare a file entity to identify the image file to be included with the
   paragraph.  

<ex>
&<!entity %%graphic-entity%%  FILE "%%filename%%.%%ext%%">
<\ex>

   Remember, all entity declarations must come before any other markup at
   the top of your help volume.

 * Use the ``&<p>'' element (!!paragraph!!) with the ``gentity'' parameter
   as shown:

<ex>
&<p gentity=%%graphic-entity%%>%%Paragraph text here ...%%
<\ex>

Where %%graphic-entity%% is an entity name that refers to the graphic file
you want inserted.

<\list>

<procedure>Example

Suppose you want to display an icon named ``sample.pm'' and wrap
paragraph text around it.  First, declare the file entity:

<ex smaller>
&<!entity sample-multicolor-icon  FILE "sample.pm">
<\ex>

Then, enter the paragraph:

<ex smaller>
&<p gentity=sample-multicolor-icon>Many &DesktopName; components 
support multicolor icons, in addition to two-color 
images.
<\ex>

To right-justify the graphic, add the ``gposition'' parameter like
this: 

<ex smaller>
&<p gentity=sample-multicolor-icon gposition=right>Many 
&DesktopName; components support multicolor icons, in addition 
to two-color images.
<\ex>

Here's the markup for a paragraph wrapped around an icon, where the
icon is a hyperlink
that displays a topic with the ID ``icon-editor'' in a new window:

<ex smaller>
&<p gentity=my-icon ghyperlink="icon-editor" 
glinktype=JumpNewView>Many &DesktopName; components support 
multicolor icons, in addition to the two-color images 
used in previous versions of &DesktopName;.  
<\ex>



<!-- seealsos/Hwtwrap.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref p-referenceSection>
<\list>




<s1 id=IncludingSpecialCharacters > Including Special Characters

<!-- IFDEF ONLINE ONLY -->
<!-- &IncludingSpecialCharactersSubtopics; -->
<list bullet tight>
* <xref ToIncludeSpecialCharacter>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|including: special character|
		<idx|inserting: special character|
		<idx|special character, inserting|
		<idx|character, inserting special|
		<idx|symbol: inserting|
		<idx|entity reference|
		<idx|reference: entity reference|
		<idx|file: ``helpchar.ent''|
		<idx|``helpchar.ent'' file|


Many special characters and symbols are available within &TagAppName;.  You
display a particular character by entering the appropriate entity
reference.

Some special character entities are declared in the file ``helpchar.ent''.
To access them, either copy the particular entity
declaration into your own volume, or include the entire ``helpchar.ent''
file.  Unused entity declarations are ignored.

Refer to <xref ChapterSpecialCharacterEntities> for a complete list of the
available characters.



<s2 id=ToIncludeSpecialCharacter > To include a special character

		<idx|including: special character|
		<idx|inserting: special character|
		<idx|special character, inserting|
		<idx|character, inserting special|
		<idx|symbol: inserting|
		<idx|entity reference|
		<idx|reference: entity reference|
		<idx|``helpchar.ent'' file|
		<idx|file: ``helpchar.ent''|


<list order>

 * Refer to <xref ChapterSpecialCharacterEntities> to determine the entity
   name for the character you want to display.	Also note whether it is a
   built-in special character.

 * If the character is !!not!! a built-in special character, add the
   following two lines among your other entity declarations (where
   %%entity-name%% is a meaningful name to you):

<ex>
&<!entity %%entity-name%% FILE "helpchar.ent">
&&%%entity-name%%;
<\ex>

		<idx|``helptag.opt'' file|
		<idx|file: ``helptag.opt''|
		<idx|``search'' parameter|
		<idx|parameter: ``search''|

Also, add this line to your ``helptag.opt'' file:

<ex>
search=/opt/dt/dthelp/dthelptag
<\ex>

   If the character !!is!! built into &TagAppName;, you can skip step 2.

 * Wherever you want to display the special character, enter its entity
   reference:

<ex>
&&%%entity-name%%;
<\ex>

<\list>


<procedure>Examples

The entity for the copyright symbol (&copy;) is a built-in special
character, so all you have to do to display it is use this entity:

<ex smaller>
&&copy;
<\ex>

To display the uppercase greek letter sigma (&Usigma;), you must
first include the ``helpchar.ent'' file (at the top of your help volume
with your other entity declarations) as shown here:

<ex smaller>
&<!entity  SpecialCharacterEntities  FILE  "helpchar.ent">
&&SpecialCharacterEntities;
<\ex>

Then you can place the following entity reference where the sigma character
is to appear:

<ex smaller>
&&Usigma;
<\ex>

As with any entity, case is not significant in the entity names for
special characters.



<s1 id=IncludingCommentsAndWriterSMemos > Including Comments and Writer's Memos

<!-- IFDEF ONLINE ONLY -->
<!-- &IncludingCommentsAndWriterSMemosSubtopics; -->
<list bullet tight>
* <xref ToInsertCommentTA>
* <xref ToInsertWritersMemoTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|including: comment|
		<idx|including: writer's memo|
		<idx|writer's memo, inserting|
		<idx|comment, inserting|


Frequently it is useful to include within your source files comments that are
not intended to be part of the help text.  Text marked with the !!comment!!
element is always ignored by the &TagAppName; software.
Comments can be used to make notes
to yourself or another author, or to exclude some markup without taking it
out of the file.

In addition to standard comments, &TagAppName; also provides a ``&<memo>''
element for entering writer's memos.  Memo notes appear
in your help topics during reviews, but not when you make
your final help files.  Authors commonly use the ``&<memo>'' element to write
questions or make notes to reviewers.



<s2 id=ToInsertCommentTA > To insert a comment

		<idx|inserting: comment|
		<idx|comment, inserting|
		<idx|``&<<esc>!-- ... --><\esc>''<sort>&<-|
		<idx|element tag: ``&<<esc>!-- ... --><\esc>''<sort>aaa|


<list bullet>

 * Use the comment begin marker (``&<!--'') and end marker (``-->'') as
   shown:

<ex>
&<!-- %%text here is completely ignored%% -->
<\ex>

<\list>

The &TagAppName; software ignores all markup between the ``&<!--'' and ``-->''.
A comment !!cannot!! be nested within another comment.

<procedure>Example

Here's an example that has two comments, a line before the paragraph, and a
single word within the paragraph.

<ex smaller>
&<!-- Here is my rough draft of the introduction: -->

Welcome to my application.  This software
is &m-lt;<esc|!--| perhaps <esc|-->| the fastest and most 
efficient software you'll ever own.
<\ex>

<!-- seealsos/Hwticomn.SA -->
<!-- SEE ALSO -->

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToInsertWritersMemoTA>
<\list>



<!-- IFDEF ONLINE ONLY END -->


<s2 id=ToInsertWritersMemoTA > To insert a writer's memo

<need begin>

		<idx|inserting: writer's memo|
		<idx|writer's memo, inserting|
		<idx|memo, inserting writer's|
		<idx|``&<memo>''|
		<idx|element tag: ``&<memo>''|
		<idx|``helptag.opt'' file|
		<idx|file: ``helptag.opt''|
		<idx|``memo'' option|
		<idx|option, ``memo''|


<list bullet>

 * Use the ``&<memo>'' element as shown:

<ex>
&<memo> %%text%% &<\memo>
<\ex>

<\list>

By default, the text within the ``&<memo>'' element is ignored by the
&TagAppName; software (just like a comment).  However, if you add the ``memo''
option to your ``helptag.opt'' file (or specify the ``memo'' option with
the ``dthelptag'' command), all memos within your help volume appear in
a bold font.

<need end>

<procedure>Example

Suppose you are writing about your application and have a question for the
project team.  You can include the question within the text using the
``&<memo>'' element like this:

<ex smaller>
&<memo>Team:  Will the product also 
support 32-bit characters?&<\memo>
<\ex>

If you process the help volume with the following command (or include
``memo'' in your ``helptag.opt'' file), the memo appears in the help
text in a bold font.

<ex smaller>
dthelptag  %%volume%%  memo
<\ex>

If the memo option is not used (or the ``nomemo''
option is used), the text within the memo is ignored and does not
appear in the help text.


<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hwtmemo.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToRunHelpTagTA>
 * <xref ToInsertCommentTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->



<s1 id=CreatingKeywordIndex > Creating a Keyword Index

<!-- IFDEF ONLINE ONLY -->
<!-- &CreatingKeywordIndexSubtopics; -->
<list bullet tight>
* <xref ToMarkAnIndexEntry>
<\list>

<!-- IFDEF ONLINE ONLY END -->


		<idx|creating: keyword index|
		<idx|keyword index: creating|
		<idx|index, keyword: creating|
		<idx|Search menu|
		<idx|menu: Search|

The keyword index for a help volume is similar to the index for a book.
The keyword index is displayed as a sorted list of the keywords you marked
using the ``&<idx>'' element.

<memo|Dex: Filter field doesn't really "reduce the number of items" does
it?  Richard|

In any general help dialog displaying your volume, the user can access the
keyword index by choosing Keyword from the Search menu (Ctrl+K).
The Filter field in the dialog lets the user reduce the number of items by
typing an initial letter or two.
When a keyword is selected, the titles of the topics in which it occurs
are listed.  To display a topic, the user selects one of the listed titles.



<s2 id=ToMarkAnIndexEntry > To mark an index entry

		<idx|marking: index entry|
		<idx|index, keyword: marking entry|
		<idx|keyword index: marking entry|
		<idx|index entry|
		<idx|``&<idx>''|
		<idx|element tag: ``&<idx>''|
		<idx|``&<sort>''|
		<idx|element tag: ``&<sort>''|


<list bullet>

 * Within the topic you want to index, use the ``&<idx>'' element as shown:

<ex>
&<idx>%%keyword%%&<\idx>
<\ex>

   !!Or,!! the short form:

<ex>
&<idx|%%keyword%%|
<\ex>


 * !!Or,!! to control how the entry is sorted, use the
``&<sort>'' subelement as shown:

<ex>
&<idx>%%keyword%%&<sort>%%sortkey%%&<\idx>
<\ex>

Where %%keyword%% is the text you want to display in the index and
%%sortkey%% is the text used during sorting.

<\list>

The ``&<idx>'' element can be used anywhere within the topic.  Neither the
%%keyword%% nor the optional %%sortkey%% are displayed in the topic.


<need begin>

<procedure>Examples

Here's the start of a topic with two keyword index entries:

<ex smaller>
&<s1 id=getting-started>Getting Started with &ViewerAppName;

&<idx>starting &ViewerAppName;&<\idx>
&<idx>&ViewerAppName;, starting&<\idx>

Welcome ...

     &vellipsis;
<\ex>


The following example indexes the plus character (+), putting it
in the keyword index where the word "plus" would appear:

<ex smaller>
&<idx>+&<sort>plus&<\idx>
<\ex>

<need end>



<s1 id=CreatingGlossarySI > Creating a Glossary

<!-- IFDEF ONLINE ONLY -->
<!-- &CreatingGlossarySISubtopics; -->
<list bullet tight>
* <xref ToMarkGlossaryTermTA>
* <xref ToDefineTermInGlossaryTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->

		<idx|creating: glossary|
		<idx|glossary: creating|
		<idx|``&<glossary>''|
		<idx|element tag: ``&<glossary>''|
		<idx|``&<term>''|
		<idx|element tag: ``&<term>''|


Like a glossary in a book, your help volume can contain a glossary that
defines important terms.  The glossary, which is marked using the
``&<glossary>'' element, is the last topic in your help volume.

Throughout your help volume, each key word or phrase that you
enter with the ``&<term>'' element automatically becomes a
definition hyperlink to the term's definition in the glossary.



<!-- seealsos/Hwtgloss.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref dterm-ReferenceSection>
 * <xref glossary-ReferenceSection>
 * <xref term-ReferenceSection>
<\list>



<s2 id=ToMarkGlossaryTermTA > To mark a glossary term

		<idx|marking: glossary term|
		<idx|glossary: term, marking|
		<idx|term: glossary, marking|
		<idx|``&<term>''|
		<idx|element tag: ``&<term>''|
		<idx|``+'' (plus)<sort>+|
		<idx|plus (``+'')|
		<idx|``nogloss'' parameter|
		<idx|parameter: ``nogloss''|


<list bullet>
 * Use the ``&<term>'' element as shown:

<ex>
&<term>%%word or phrase%%&<\term>
<\ex>

   !!Or,!! use the short form:

<ex>
&<term|%%word or phrase%%|
<\ex>

   !!Or,!! use the shorthand form:

<ex>
<esc|++|%%word or phrase%%<esc|++|
<\ex>

<\list>

If the term within the help text isn't spelled exactly the same
as the definition in the glossary, you can specify the
"glossary form" of the term like this:

<ex>
&<term "%%glossary form%%">%%word or phrase%%&<\term>
<\ex>

Where %%glossary form%% is the term exactly as it appears in the glossary.
This is useful if the term must be plural in a help topic (because of its
context), but must be singular in the glossary.

Terms are displayed using a bold font and automatically become a definition
hyperlink.  When the term is chosen, its glossary definition appears in a
quick help dialog.

<note>

 If you mark a term that you intentionally do not define in the glossary,
 add the ``nogloss'' attribute to the ``&<term>'' element.  This allows the
 term to be displayed in the bold font used for terms, but without creating a
 link to the glossary.

<\note>

<procedure>Examples

If your glossary has a definition for the term "widget," you can enter it
as a term like this:

<ex smaller>
A <esc|++|widget<esc|++| is the fundamental building
block of OSF/Motif user interfaces.
<\ex>

If the glossary entry is "widget," but you need to use the plural form
within the sentence, you could enter the term like this:

<ex smaller>
&<term "widget">Widgets&<\term> are the fundamental
building blocks of OSF/Motif user interfaces.
<\ex>

If you want to enter the same term, but you either don't want to include it
in the glossary or you don't want it to be a hyperlink, use the ``nogloss''
parameter like this:

<ex smaller>
&<term nogloss>Widgets&<\term> are the fundamental
building blocks of OSF/Motif user interfaces.
<\ex>

The equivalent short form is:

<ex smaller>
&<term nogloss|Widgets| are the fundamental
building blocks of OSF/Motif user interfaces.
<\ex>


<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hwtglost.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToDefineTermInGlossaryTA>
 * <xref dterm-ReferenceSection>
 * <xref glossary-ReferenceSection>
 * <xref term-ReferenceSection>
<\list>
<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToDefineTermInGlossaryTA > To define a term in the glossary

<need begin>

		<idx|defining: term in glossary|
		<idx|term: defining in glossary|
		<idx|glossary: defining term in|
		<idx|``&<term>''|
		<idx|element tag: ``&<dterm>''|
		<idx|``&<glossary>''|
		<idx|element tag: ``&<glossary>''|


<list bullet>
 * Enter the ``&<dterm>'' element into the glossary as shown:

<ex>
&<glossary>

     &vellipsis;
&<dterm>%%word or phrase%%
%%Definition of the term%%
     &vellipsis;
<\ex>

<\list>

Be sure to keep the ``&<dterm>'' words and phrases sorted within the glossary.

<procedure>Example

Here's part of a glossary that includes the definition of the term "SGML":

<ex smaller>
&<glossary>

     &vellipsis;
&<dterm>SGML

<esc|!!|Standard Generalized Markup Language<esc|!!|.  An 
international standard [ISO 8859: 1986] that 
establishes a method for information interchange.  
SGML prescribes constructs for marking the 
structure of information separate from its 
intended presentation or format.  The CDE HelpTag 
markup language is based on the SGML standard.

<\ex>

<need end>



<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hwtdefgl.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToMarkGlossaryTermTA>
 * <xref dterm-ReferenceSection>
 * <xref glossary-ReferenceSection>
 * <xref term-ReferenceSection>
<\list>


<!-- IFDEF ONLINE ONLY END -->


