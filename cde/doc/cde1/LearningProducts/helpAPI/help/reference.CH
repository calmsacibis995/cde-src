<chapter id=markup-summary> &TagAppName; Markup Reference
		<idx|&TagAppName; markup reference|
		<idx|markup reference|
		<idx|summary: &TagAppName; markup|
		<idx|tags, element|

<!-- IFDEF ONLINE ONLY -->
<procedure>&TagAppName; Elements Listed Alphabetically ...

<list bullet tight>
 * <xref comment-ReferenceSection>
 * <xref abbrev-ReferenceSection>
 * <xref abstract-ReferenceSection>
 * <xref book-ReferenceSection>
 * <xref caution-ReferenceSection>
 * <xref chapter-ReferenceSection>
 * <xref computer-ReferenceSection>
 * <xref copyright-ReferenceSection>
 * <xref dterm-ReferenceSection>
 * <xref emph-ReferenceSection>
 * <xref entity-ReferenceSection>
 * <xref esc-ReferenceSection>
 * <xref ex-ReferenceSection>
 * <xref figure-ReferenceSection>
 * <xref glossary-ReferenceSection>
 * <xref graphic-ReferenceSection>
 * <xref head-ReferenceSection>
 * <xref helpvolume-ReferenceSection>
 * <xref hometopic-ReferenceSection>
 * <xref idx-ReferenceSection>
 * <xref image-ReferenceSection>
 * <xref item-ReferenceSection>
 * <xref lablist-ReferenceSection>
 * <xref link-ReferenceSection>
 * <xref list-ReferenceSection>
 * <xref location-ReferenceSection>
 * <xref memo-ReferenceSection>
 * <xref metainfo-ReferenceSection>
 * <xref newline-ReferenceSection>
 * <xref note-ReferenceSection>
 * <xref otherfront-ReferenceSection>
 * <xref otherhead-ReferenceSection>
 * <xref p-ReferenceSection>
 * <xref procedure-ReferenceSection>
 * <xref quote-ReferenceSection>
 * <xref rsect-ReferenceSection>
 * <xref s1-ReferenceSection>
 * <xref term-ReferenceSection>
 * <xref title-ReferenceSection>
 * <xref user-ReferenceSection>
 * <xref var-ReferenceSection>
 * <xref vex-ReferenceSection>
 * <xref warning-ReferenceSection>
 * <xref xref-ReferenceSection>
<\list>

<procedure>&TagAppName; Elements Listed by Function ...

<!-- if you edit any of the following, be sure the appropriate similar
changes are made to the hardcopy version of the same stuff (which differs
only in format, not information -->

!!Meta information (information about your volume):!!

<list plain tight>
 * <xref metainfo-ReferenceSection>
 * <xref title-ReferenceSection>
 * <xref copyright-ReferenceSection>
 * <xref abstract-ReferenceSection>
<\list>


!!Structure of a help volume:!!

<list plain tight>
 * <xref entity-ReferenceSection>
 * <xref helpvolume-ReferenceSection>
 * <xref hometopic-ReferenceSection>
 * <xref chapter-ReferenceSection>
 * <xref s1-ReferenceSection> (heading)
 * <xref rsect-ReferenceSection>  (reference section)
 * <xref otherhead-ReferenceSection>
 * <xref procedure-ReferenceSection>
 * <xref p-ReferenceSection>  (paragraph)
<\list>


!!Inline elements:!!

<list plain tight>
 * <xref book-ReferenceSection>
 * <xref computer-ReferenceSection>  (short form: ``<esc|``|''%%text%%``<esc|''|'')
 * <xref emph-ReferenceSection>  (emphasis)  (short form: ``<esc|!!|''%%text%%``<esc|!!|'')
 * <xref ex-ReferenceSection>  (example) and  <xref vex-ReferenceSection>  (verbatim example)
 * <xref image-ReferenceSection>
 * <xref term-ReferenceSection>  (short form: ``<esc|++|''%%text%%``<esc|++|'')
 * <xref user-ReferenceSection>  (user input)
 * <xref var-ReferenceSection>  (variable)  (short form: ``<esc|%%|''%%text%%``<esc|%%|'')
 * <xref newline-ReferenceSection>
 * <xref p-ReferenceSection>  (paragraph)
 * <xref quote-ReferenceSection>
 * ``&&'' ... ``;'' (see <xref entity-ReferenceSection>)
<\list>


!!Important information:!!

<list plain tight>
 * <xref note-ReferenceSection>
 * <xref caution-ReferenceSection>
 * <xref warning-ReferenceSection>
 * <xref emph-ReferenceSection>  (emphasis)  (short form: ``<esc|!!|''%%text%%``<esc|!!|'')
<\list>


!!Lists:!!

<list plain tight>
 * <xref list-ReferenceSection>
 * <xref lablist-ReferenceSection>  (labeled list)
 * <xref item-ReferenceSection>  (``*'')
<\list>


!!Graphics:!!

<list plain tight>
 * <xref figure-ReferenceSection>
 * <xref graphic-ReferenceSection>
<\list>


!!Glossary and keyword index:!!

<list plain tight>
 * <xref glossary-ReferenceSection>
 * <xref dterm-ReferenceSection>  (definition of term)
 * <xref term-ReferenceSection>  (short form: ``<esc|++|''%%text%%``<esc|++|'')
 * <xref idx-ReferenceSection>  (index)
<\list>


!!Cross-references and hyperlinks:!!

<list plain tight>
 * <xref xref-ReferenceSection>  (cross-reference)
 * <xref link-ReferenceSection>
 * <xref location-ReferenceSection>
 * <xref term-ReferenceSection>
<\list>


!!Hidden text:!!

<list plain tight>
 * <xref comment-ReferenceSection>  (comment)
 * <xref memo-ReferenceSection>
<\list>


!!Titles and headings:!!

<list plain tight>
 * <xref abbrev-ReferenceSection>
 * <xref head-ReferenceSection>
 * <xref otherhead-ReferenceSection>
 * <xref procedure-ReferenceSection>
<\list>


!!Override meaning of &TagAppName; markup:!!

<list plain tight>
 * <xref esc-ReferenceSection>  (escape from markup recognition)
 * <xref vex-ReferenceSection>  (verbatim example)
<\list>
<!-- IFDEF ONLINE ONLY END -->




<rsect id=comment-ReferenceSection > &m-lt;<esc>!--<\esc> ... <esc>--><\esc>

		<idx|comment, inserting|
		<idx|``&<<esc>!-- ... --><\esc>''<sort>&<-|
		<idx|element tag: ``&<<esc>!-- ... --><\esc>''<sort>aaa|


!!Comment!!

Text you want the &TagAppName; software to ignore.  Comments cannot be nested.



<procedure> Syntax

<ex>
&<!-- %%comment text here%% -->
<\ex>

The comment text can contain any text except two dashes (``--'').


<procedure> Example

The following markup hides both a comment and a figure:

<ex smaller>
&<!-- Let's leave out this figure for now:
&<figure entity=DeltaGee>
Before and After Processing
&<\figure>  -->
<\ex>


<procedure> See Also

<list plain tight>
* <xref memo-ReferenceSection>
<\list>


<\rsect>



<rsect id=abbrev-ReferenceSection > &<abbrev>

		<idx|``&<abbrev>''|
		<idx|element tag: ``&<abbrev>''|
		<idx|abbreviating long titles|
		<idx|title: abbreviating|


!!Abbreviated title!!

Indicates an alternate, typically shorter, heading for a topic that has
a long title.  The abbreviated title is used within the &ProductName;'s
dialogs whenever the title of a topic is displayed in a list (such as in
the Topic Hierarchy and the History dialog).


<procedure> Syntax

<ex>
&<%%topic-element%%>%%title%%
&<abbrev>%%short title%%
<\ex>

Where %%topic-element%% is ``&<hometopic>'', ``&<chapter>'', ``&<s1>'', or
any other element that begins a new topic.

The ``&<abbrev>'' tag must appear on the line immediately following
the heading.

An end tag is not required.

<procedure> Examples

Here is a simple example:

<ex smaller>
&<chapter>Ways of Treating Headings that are Too Long 
&<abbrev>Long Headings
<\ex>

Suppose you want to have a topic that doesn't have its title displayed
in the help text display area, but you do want a title to appear in the
Topic Hierarchy.  The following markup shows how this can be done:

		<idx|``&&empty;''|

<ex smaller>
&<chapter>&&empty;
&<abbrev>%%chapter title%%
<\ex>


<procedure> See Also

<list plain tight>
* <xref chapter-ReferenceSection>
* <xref rsect-ReferenceSection>
<\list>


<\rsect>








<rsect id=abstract-ReferenceSection > &<abstract>

		<idx|``&<abstract>''|
		<idx|element tag: ``&<abstract>''|
		<idx|abstract for help volume|
		<idx|volume: abstract|

!!Abstract!!

Short description of the help volume


<procedure> Syntax

<ex>
&<metainfo>
    &vellipsis;
  &<abstract>
  %%abstract text here ...%%
  &<\abstract>
    &vellipsis;
&<\metainfo>
<\ex>

The abstract text should not contain &TagAppName; markup because the abstract
may be read and displayed by applications that don't recognize the markup.

The ``&<abstract>'' element is automatically assigned the ID string
``_abstract''.  An author-defined ID cannot be assigned.
The ``_abstract'' ID can be used with the ``&<link>'' element, but
not with the ``&<xref>'' element.


<procedure> Example

This markup briefly describes the contents of a help volume:

<ex smaller>
&<abstract>
Online help for the CDE Application Manager Version 1.0.
&<&\abstract>
<\ex>

<procedure> Note

When creating a link to an element within the ``&<metainfo>''
element, be sure it is a ``type=Definition'' link.  The following markup
shows how to create a link to the abstract:

<ex smaller>
&<link hyperlink="_abstract" type=Definition>
Choose this link for an abstract.&<\link>
<\ex>


<procedure> See Also

<list plain tight>
* <xref metainfo-ReferenceSection>
<\list>


<\rsect>



<rsect id=book-ReferenceSection > &<book>

		<idx|``&<book>''|
		<idx|element tag: ``&<book>''|
		<idx|book title|
		<idx|title: book|
		<idx|quotation mark: double|
		<idx|mark, quotation: double|


!!Book title!!

Identifies the title of a book.


<procedure> Syntax

<ex>
&<book>%%book title%%&<\book>
<\ex>

!!Or:!!

<ex>
&<book|%%book title%%|
<\ex>

&TagAppName; formats book titles using an italic font.



<procedure> Examples

Either of the following two variations:

<ex smaller>
Refer to &<book>The Elements of Style&<\book> 
for further details.
<\ex>

!!Or:!!

<ex smaller>
Refer to &<book|The Elements of Style| 
for further details.
<\ex>

produce:

<p indent>Refer to <book>The Elements of Style<\book>
for further details.

<\rsect>








<rsect id=caution-ReferenceSection > &<caution>

		<idx|``&<caution>''|
		<idx|element tag: ``&<caution>''|


!!Caution notice!!

Specifies information that warns the user about a potential loss of data.


<procedure> Syntax

<ex>
&<caution>
%%text of caution%%
&<\caution>
<\ex>


The default heading for the caution is "Caution".  To specify a
different heading, use the ``&<head>'' tag as shown here:

<ex>
&<caution>&<head>%%alternate heading%%
%%text of caution%%
&<\caution>
<\ex>


The ``&<\caution>'' end tag is required.

To specify that an icon be displayed with the caution, define
a file entity at the top of your help volume as follows:

<ex>
&<!entity CautionElementDefaultIconFile FILE "%%filename%%">
<\ex>

Where %%filename%% is the name of the icon graphic.  A sample caution icon
named ``cauticon.pm'' is provided in the
``/opt/dt/dthelp/dthelptag/icons/'' directory.


<procedure> Example

Here is a caution message:

<ex smaller>
&<caution>
Do not press the DELETE key at this time.
&<\caution>
<\ex>

For an example that shows how to use the ``&<head>'' element to specify a
non-default heading, refer to <xref note-ReferenceSection>.


<procedure> See Also

<list plain tight>
* <xref note-ReferenceSection> includes an example of changing a heading.
* <xref warning-ReferenceSection>
* <xref figure-ReferenceSection>
* <xref head-ReferenceSection>
<\list>


<\rsect>








<rsect id=chapter-ReferenceSection > &<chapter>

		<idx|``&<chapter>''|
		<idx|element tag: ``&<chapter>''|
		<idx|topic: starting new|
		<idx|chapter|
		<idx|``id'' parameter: in ``&<chapter>''|
		<idx|parameter: ``id'' in ``&<chapter>''|
		<idx|title: topic|
		<idx|topic: title|


!!Chapter!!

Indicates the start of a new topic with a new title.


<procedure> Syntax

<ex>
&<chapter id=%%id%%>%%title%%
%%topic text ...%%
<\ex>

An end tag is not required.

If the topic title is long, you may want to provide an alternate
abbreviated title using ``&<abbrev>''.


<procedure> Examples

Here are two markups that begin a new topic:

<ex smaller>
&<chapter>A Manual of Style

&<chapter id=writer>The Careful Writer

<\ex>

<procedure> See Also

<list plain tight>
* <xref abbrev-ReferenceSection>
* <xref link-ReferenceSection>
* <xref rsect-ReferenceSection>
* <xref s1-ReferenceSection>
* <xref xref-ReferenceSection>
<\list>


<\rsect>










<rsect id=computer-ReferenceSection > &<computer>

		<idx|``&<computer>''|
		<idx|element tag: ``&<computer>''|
		<idx|quotation mark: repeated single|
		<idx|mark, quotation: repeated single|
		<idx|apostrophe|


!!Computer literal!!

Displays text that represents computer input or output.


<procedure> Syntax

<ex>
&<computer>%%text%%&<&\computer>
<\ex>

!!Or:!!

<ex>
<esc|``|%%text%%<esc|''|
<\ex>


The shorthand form uses two left apostrophes (``<esc>``<\esc>'') and
two right apostrophes (``<esc>''<\esc>'').


<procedure> Examples

The following markup:

<ex smaller>
&<computer>Enter the correct numerical value.&<&\computer>
<\ex>

produces the following output:

<p indent>``Enter the correct numerical value.''


The following markup uses the shorthand form:

<ex smaller>
<esc|Everything in ``computer'' comes out looking ``like this''.|
<\ex>

and it produces:

<p indent>Everything in ``computer'' comes out looking ``like this''.

Variables can be nested within computer text.  For example, the
following markup:

<ex smaller>
<esc|``void  DisplayTopic (%%topic%%);''|
<\ex>

produces:

<ex smaller>
``void  DisplayTopic (%%topic%%);''
<\ex>


<procedure> See Also

<list plain tight>

 * <xref ex-ReferenceSection>
 * <xref user-ReferenceSection>
 * <xref var-ReferenceSection>

<\list>

<\rsect>








<rsect id=copyright-ReferenceSection > &<copyright>

		<idx|copyright: notice|
		<idx|``&<copyright>''|
		<idx|element tag: ``&<copyright>''|


!!Copyright notice!!

Used to enter text for the copyright notice.


<procedure> Syntax

<ex>
&<metainfo>
  &<title>%%Title (always before copyright)%%
  &<copyright>
  &&copy; %%Copyright notice here ...%%
<\ex>

This element is required within the ``&<metainfo>'' section and must follow
the ``&<title>'' element.

The end tag is not required.

The predefined entity ``&&copy;'' produces the copyright symbol (&copy;).


<procedure> Example

The following markup:

<ex smaller>
&<metainfo>

&<title>XYZ World Almanac

&<copyright>
&&copy; Copyright 1993 XYZ Company. 
All rights reserved.
<\ex>

produces:

!!XYZ World Almanac!!
&copy; Copyright 1993 XYZ Company.
All rights reserved.


<procedure> See Also

<list plain tight>
* <xref metainfo-ReferenceSection>
* <xref title-ReferenceSection>
<\list>


<\rsect>








<rsect id=dterm-ReferenceSection > &<dterm>

		<idx|``&<dterm>''|
		<idx|element tag: ``&<dterm>''|


!!Defined term!!

Identifies a term and the term's definition within the glossary.



<procedure> Syntax

<ex>
&<glossary>

  &<dterm>%%first term%%
  %%definition of first term%%
    &vellipsis;
  &<dterm>%%Nth term%%
  %%definition of Nth term%%
<\ex>

This element is used within the ``&<glossary>'' section.

		<idx|line: definition spanning more than one|
		<idx|multiple lines, definition spanning|
		<idx|ampersand (``&&'')|
		<idx|``&&'' (ampersand)<sort>&&|

The name of the term follows the ``&<dterm>'' tag and appears
on the same line.  The term's definition begins on the line
following the ``&<dterm>'' tag.

An end tag is not required.  


<procedure> Example

The following markup defines the first two words in a glossary:

<ex smaller>
&<glossary>

&<dterm>ex libris
From the books. Used before the owner's name on bookplates.

&<dterm>key word
A word exemplifying the meaning or value of a letter or symbol.
<\ex>


<procedure> See Also

<list plain tight>
* <xref glossary-ReferenceSection>
* <xref term-ReferenceSection>
<\list>


<\rsect>








<rsect id=emph-ReferenceSection > &<emph>

		<idx|``&<emph>''|
		<idx|element tag: ``&<emph>''|
		<idx|``!'' (exclamation mark)<sort>!|
		<idx|exclamation mark (``!'')|
		<idx|emphasizing: text|
		<idx|important words, calling attention to|
		<idx|attention to important words, calling|


!!Emphasized text!!

Formats the text in a font that draws attention to the text.


<procedure> Syntax

<ex>
&<emph>%%text%%&<\emph>
<\ex>

!!Or:!!

<ex>
!!%%text%%!!
<\ex>


The shorthand form for the ``&<emph>'' element is a set of
double exclamation marks (``<esc|!!|'') before and after the text.

If you use the ``&<emph>'' start tag, the ``&<\emph>'' end tag is required.


<procedure> Examples

Either of the following two markups:

<ex>
A thousand times &<emph>no&<\emph>.
<\ex>

<ex>
A thousand times !!no!!.
<\ex>

produce:

<p indent>A thousand times !!no!!.


<procedure> See Also

<list plain tight>
* <xref book-ReferenceSection>
* <xref quote-ReferenceSection>
* <xref term-ReferenceSection>
* <xref var-ReferenceSection>
<\list>


<\rsect>








<rsect id=entity-ReferenceSection > &<!entity>

		<idx|``&<!entity>''<sort>&<!|
		<idx|element tag: ``&<!entity>''|
		<idx|entity: declaration|
		<idx|reference: to entity|
		<idx|figure: entity|
		<idx|entity: with figure|
		<idx|``FILE'' parameter|
		<idx|parameter: ``FILE''|


!!Entity declaration!!

Assigns an entity name to a string of characters or to an external file.


<procedure> Syntax

<ex>
&<!entity %%entityname%% "%%string%%">
<\ex>

!!Or:!!
<ex>
&<!entity %%entityname%% FILE "%%filename%%">
<\ex>


An entity name can contain up to 64 letters, digits, and hyphens.  Case is
not significant in entity names, but is often used to improve readability
for the author.  The first character must be a letter.  No space is
permitted between the left angle bracket (``&<''), the exclamation mark
(``!'') and ``entity'' in an ``&<!entity>'' declaration.

Entity declarations must always precede any other markup or text in
the help volume.

		<idx|ampersand (``&&'')|
		<idx|``&&'' (ampersand)<sort>&&|
		<idx|semicolon (``;'')|
		<idx|``;'' (semicolon)<sort>;|

Where you want the defined entity to appear, insert an entity reference
using this syntax:

<ex>
&&%%entityname%%;
<\ex>

The entity reference consists of an ampersand (``&&''), followed by the
entity name (as defined in the entity declaration), and ending with
a semicolon (``;'').

<procedure>Purposes for Entities

There are four common reasons for defining an entity:

<list bullet tight>

* Text that is associated with an entity name appears only once
  so that changing the text requires making a change
  in only one place.  All references to the entity automatically change
  when &TagAppName; reprocesses the files.

* The inefficiency of typing the same long or complex text string many times
  can be avoided (along with typing mistakes) by typing just a short
  entity reference wherever that text string will appear.  The full
  text string needs to be typed only once.

* The ``&<figure>'' and ``&<graphic>'' elements do not accept a file name.
  The name of the file that contains the figure must be specified in an
  entity declaration.

* It is convenient to put the help text into multiple files, yet
  &TagAppName; accepts only one source file.  These needs can be balanced by 
  creating one file that contains entity declarations and
  entity references that refer to the files that contain the actual
  help text.

<\list>


<procedure> Examples

The ``%%volume%%.htg'' source file can contain the following entity
declarations and entity references so that the actual text can be put
into the named files:

<ex smaller>
&<!entity topic1 FILE "topic1">
&<!entity topic2 FILE "topic2">
&<!entity topic3 FILE "topic3">

&&topic1;
&&topic2;
&&topic3;
<\ex>


The following entity declaration causes the words "Architectural Analysis of
Aircraft Precision Components" to be displayed wherever the ``&&aaapc;'' entity
reference appears in the marked-up files.

<ex smaller>
&<!entity aaapc "Architectural Analysis of Aircraft Precision Components">
<\ex>


The following entity declaration for a !!figure!! is placed at
the beginning of the source file:

<ex smaller>
&<!entity CloseUpFig FILE "figname.tif">
<\ex>


and the figure would be inserted where the following markup appears:

<ex smaller>
&<figure entity=CloseUpFig>
Close Up View
&<\figure>
<\ex>


<procedure> See Also

<list plain tight>
* <xref UsingEntitiesSI>
* <xref figure-ReferenceSection>
* <xref xref-ReferenceSection>
* <xref ChapterSpecialCharacterEntities>
<\list>


<\rsect>








<rsect id=esc-ReferenceSection > &<esc>

		<idx|``&<esc>''|
		<idx|element tag: ``&<esc>''|
		<idx|untranslated text|
		<idx|escape text|
		<idx|text: untranslated|


!!Escape!!

Causes text to be passed directly to the run-time help files without
being interpreted by &TagAppName;.


<procedure> Syntax

<ex>
&<esc>%%text%%&<\esc>
<\ex>

!!Or:!!

<ex>
&<esc|%%text%%|
<\ex>

<note>
If the long form is used, the text cannot contain the three-character
sequence ``&<&\''%%x%% (the less-than symbol followed by a backslash
followed by a letter).  

If the short form is used, the text cannot contain the vertical bar
character (``|'').
<\note>

The ``&<\esc>'' end tag is required.  
<procedure> Example

The following markup:

<ex>
&m-lt;<esc>esc|characters !! or \|<\esc>
<\ex>

produces:

<p indent> <esc|characters !! or \|


<procedure> See Also

<list plain tight>

* <xref GeneralMarkupGuidelinesSI>
* <xref vex-ReferenceSection>
<\list>


<\rsect>








<rsect id=ex-ReferenceSection > &<ex>

		<idx|``&<ex>''|
		<idx|element tag: ``&<ex>''|
		<idx|example: displaying|
		<idx|displaying: computer input/output examples|
		<idx|computer: display|
		<idx|computer: listing|
		<idx|listing: computer|
		<idx|``number'' parameter|
		<idx|``smaller'' parameter|
		<idx|``smallest'' parameter|
		<idx|parameter: ``number''|
		<idx|parameter: ``smaller''|
		<idx|parameter: ``smallest''|
		<idx|line breaks, preserving|
		<idx|spacing, preserving|


!!Computer example!!

Shows computer text without changing the spacing or line breaks.


<procedure> Syntax

<ex>
&<ex [nonumber | number] [smaller | smallest]>
%%example text here ...%%
&<\ex>
<\ex>

Where:

<lablist>

\``nonumber''\ (Default.)  Omits the adding of line numbers to the
beginning of each line.  

\ ``number''\ Puts a line number at the beginning of each line.

\``smaller'' or ``smallest'' \ Displays the example using smaller
fonts.  This makes long lines fit within a narrower width.

<\lablist>


Examples are printed in ``computer'' font, and they are indented from
the left text margin.

If you include the ``number'' attribute, the line numbers of the
example will be numbered.  This is useful for referring to specific lines.

The following character pairs, which have special meanings in other
contexts, are treated as ordinary text within an example:

<ex>
<esc|!!|
<esc|--|
<esc|++|
<esc|"|
<\ex>

<!-- in the next release we should elaborate on why these are special. -->

The ``&<\ex>'' end tag is required.


<newpage>

<procedure> Example


The following markup:

<ex smaller>
&<ex>
Examples are printed 
in computer font.
&<\ex>
<\ex>

produces:

<ex smaller>
Examples are printed 
in computer font.
<\ex>


<procedure> See Also

<list plain tight>
* <xref computer-ReferenceSection>
* <xref user-ReferenceSection>
* <xref vex-ReferenceSection>
<\list>


<\rsect>








<rsect id=figure-ReferenceSection > &<figure>

		<idx|``&<figure>''|
		<idx|element tag: ``&<figure>''|
		<idx|graphic: including|
		<idx|figure: including|
		<idx|including: graphic|
		<idx|``id'' parameter: in ``&<figure>''|
		<idx|parameter: ``id'' in ``&<figure>''|
		<idx|``entity'' parameter|
		<idx|``number'' parameter|
		<idx|``nonumber'' parameter|
		<idx|parameter: ``entity''|
		<idx|parameter: ``number''|
		<idx|parameter: ``nonumber''|


!!Figure!!

Inserts a graphical image.


<procedure> Syntax

<ex>
&<figure entity=%%entity%% [id=%%id%% [nonumber | number=%%n%%] 
[ghyperlink=%%id%% [glinktype=%%type%%] [gdescription=%%text%% ]]] >
%%caption string%%
&<\figure>
<\ex>


<lablist width=1.5in>

\``entity=%%name%%''\ Specifies a file entity that identifies the file
		      which contains the graphic image to be inserted.

		<idx|ID: identifier|
		<idx|entity: definition|

\``id=%%name%%''\ Optional. Defines an ID name that can be used in
		cross-references to this figure.


\``nonumber''\  Optional.  Suppresses the word "Figure" and the
		automatically generated figure number.

		
\``number=%%n%%''\  Optional.  Used to override the automatically generated
		    figure number.

\``ghyperlink="%%id%%"''\  Optional.  Specifies that the graphic portion of
			   the figure be a hyperlink.  References to this
			   location would use the specified %%id%% identifier.

\``glinktype=%%type%%''\  Optional.  Specifies the type of hyperlink.  The
			  default type is ``Jump''.  Other type values
			  include ``JumpNewView'', ``Definition'', ``Man'',
			  ``Execute'', and ``AppDefined''.  The
                          ``ghyperlink'' parameter and %%id%% value are
                           required when using parameter.


\``gdescription="%%text%%"''\  Optional.  Provides a description
                          of the hyperlink.  This description is used by the
			  topic access functions.  The
                          ``ghyperlink'' parameter and %%id%% value are
                           required when using this parameter.

<\lablist>

The ``&<\figure>'' end tag is required.

To integrate an external graphics file into a help topic, you must have an
entity declaration (``&<!entity %%entityname%% FILE "%%filename%%">'')
that associates the entity name with the graphic's file name.


<need begin>

<procedure> Examples


The following markup inserts a graphic with the specified caption and
an automatically generated figure number:

<ex smaller>
&<!entity MapFigure FILE "mappic.xwd">
   &vellipsis;
&<figure entity=MapFigure>
Caption for Figure
&<\figure>
<\ex>


The following markup inserts a figure that is numbered but does not have
a caption.  The figure is referred to later in the text.

<ex smaller>
&<!entity MyPicture FILE "mappic.xwd">
   &vellipsis;
&<figure id=Layout entity=MapFigure>
&<\figure>
   &vellipsis;
&<xref Layout> shows the layout of ...
<\ex>

		<idx|backslash (``&\'')|
		<idx|``&\'' (backslash)<sort>&\|
		<idx|caption: starting new line|
		<idx|line: starting new caption|
		<idx|wrapping: line|

The following markup inserts a figure using a specific figure number
and a caption.  The caption is split into two lines where the backslash (&\)
character appears.

<ex smaller>
&<figure number=99 entity=SchemDiag>
Schematic that Illustrates\the Overall System Design
&<\figure>
<\ex>

<need end>


<procedure> See Also

<list plain tight>
* <xref entity-ReferenceSection>
* <xref graphic-ReferenceSection>
* <xref link-ReferenceSection>
* <xref xref-ReferenceSection>
<\list>


<\rsect>









<rsect id=glossary-ReferenceSection > &<glossary>

		<idx|``&<glossary>''|
		<idx|element tag: ``&<glossary>''|
		<idx|glossary|
		<idx|term: definition|
		<idx|definition: of term|


!!Glossary!!

Starts the glossary section which contains the definitions for all
the terms that are marked with the ``&<term>'' element.


<procedure> Syntax

<ex>
&<glossary>

&<dterm>%%first term%%
%%definition of first term can continue
over multiple lines or paragraphs%%

&<dterm>%%second term%%
%%definition of second term ...%%

    &vellipsis;
<\ex>

"Glossary" is automatically used as the heading for the glossary section.

A ``&<dterm>'' element identifies each term and its definition.

All terms marked with ``&<term>'' without the
``nogloss'' parameter are required to be in the glossary. 
If the term is not in the glossary, omitted terms are listed in
the %%filename.%%``err'' file, which is created when you
run &TagAppName;.

An end tag for ``&<glossary>'' is not required.


<procedure> Example

Here is a simple glossary with two definitions:

<ex smaller>
&<glossary>

&<dterm>oxymoron
A combination of contradictory words.

&<dterm>veritable
Being in fact the thing named. Authentic.
<\ex>


<procedure> See Also

<list plain tight>
* <xref term-ReferenceSection>
* <xref dterm-ReferenceSection>
<\list>


<\rsect>









<rsect id=graphic-ReferenceSection > &<graphic>

		<idx|``&<graphic>''|
		<idx|element tag: ``&<graphic>''|
		<idx|graphic: element within text|
		<idx|``entity'' parameter|
		<idx|parameter: ``entity''|


!!Inline graphic!!
  
Used for inserting a graphical element within a line of text.


<procedure> Syntax

<ex>
&<graphic entity=%%name%%>
<\ex>

Where:

<lablist>

\%%name%%\  An entity name which is defined in an entity declaration.
The entity declaration associates the entity name with the name of
the file that contains the graphic to be inserted.

<\lablist>

The ``&<graphic>'' element is similar to ``&<figure>'' except that
the ``&<graphic>'' element is intended for embedding !!small!! graphics
within text, whereas the ``&<figure>'' element inserts figures between
paragraphs.


<procedure> Examples:

The following markup first defines an entity (``mini-icon'') as being
associated with the contents of a graphics file (named "mini.pm").
Then the ``&<graphic>'' element indicates
the location of the graphic within a line of text.

<ex smaller>
&<!entity mini-icon FILE "mini.pm">
  &vellipsis;
The &<graphic entity=mini-icon> icon 
is used for very small images.
<\ex>

The following markup defines the inline graphic as a hyperlink
to a topic whose ID is ``mini-icon-topic'':

<ex smaller>
The &<link mini-icon-topic>&<graphic entity=mini-icon>&<\link> icon 
is used for very small things.
<\ex>


<procedure> See Also

<list plain tight>
 * <xref entity-ReferenceSection>
 * <xref figure-ReferenceSection>
 * <xref link-ReferenceSection>
 * <xref p-ReferenceSection>
 * <xref ToIncludeSpecialCharacter>
<\list>


<\rsect>








<rsect id=head-ReferenceSection > &<head>

		<idx|``&<head>''|
		<idx|element tag: ``&<head>''|
		<idx|title: section|
		<idx|title: note|
		<idx|title: list|


!!Heading!!

Indicates the title for elements that normally do not have a title
(such as image, lablist, list, and otherfront) or have
a default title (such as note, caution and warning).


<procedure> Syntax

<ex>
&<%%element%%>&<head>%%title text%%
<\ex>


A heading starts with the first non-blank character after
the ``&<head>'' tag.  The ``&<head>'' tag can appear on the
same line as the element to which a heading is being added, or on
the following line.

The ``&<head>'' element can be used with elements that expect a title,
but it is not required in those cases.

		<idx|wrapping: line|
		<idx|line: wrapping|
		<idx|backslash (``&\'')|
		<idx|``&\'' (backslash)<sort>&\|
		<idx|heading, starting new line|
		<idx|line: starting new heading|

Headings that are wider than the heading area are automatically wrapped
onto successive lines.  To force a specific line break,
put a backslash (\) where you want the line to break.

A heading ends at the end of the line in the source file unless the line
ends with an ampersand (&&). If a heading spans multiple lines in your source
file, put an ampersand after all the lines except the last.

The ``&<\head>'' end tag is not required.


<procedure> Examples

The following markup adds a title to a list and specifies the start of
a new line where the backslash appears:

<ex smaller>
&<list>&<head>Printing Options\for the QRZ Hardware
<\ex>


<!-- alternate example:

<ex>
&<otherfront>Disclaimer&&
for the&&
Quasi Constructs&&
Software
<\ex>

-->


		<idx|line: continuing heading|
		<idx|continuing heading line|
		<idx|heading line, continuing|


The following markup overrides the default "Note" heading.
The ampersand (``&&'') indicates that the heading continues
on the following line.

<ex smaller>
&<note>&<head>Tips&&
& Traps
Take special note of this.
&<\note>
<\ex>


<procedure> See Also

<list plain tight>
* <xref otherfront-ReferenceSection>
* <xref note-ReferenceSection>
* <xref caution-ReferenceSection>
* <xref warning-ReferenceSection>
<\list>
  

<\rsect>








<rsect id=helpvolume-ReferenceSection > &<helpvolume>

		<idx|``&<helpvolume>''|
		<idx|element tag: ``&<helpvolume>''|
		<idx|volume: ``&<helpvolume>''|


&empty;

!!Application help volume!!

This is the "root" structural element; it contains all the markup for an
entire help volume.


<procedure>  Syntax

<ex>
%%all entity declarations%%
    &vellipsis;

&<helpvolume>
    &vellipsis;
  %% all of your help is included here, either %%
  %% literally or using file entity references %%
    &vellipsis;
&<\helpvolume>
<\ex>


If you do not enter this tag, its presence is automatically
assumed by the &TagAppName; software.

All entity declarations must appear before the ``&<helpvolume>'' 
start tag.


<procedure> See Also

<list plain tight>
 * <xref IntroducingThePartsOfAHelpVolumeTO>
 * <xref entity-ReferenceSection>
 * <xref hometopic-ReferenceSection>
 * <xref metainfo-ReferenceSection>
<\list>


<\rsect>




<rsect id=hometopic-ReferenceSection > &<hometopic>

		<idx|``&<hometopic>''|
		<idx|element tag: ``&<hometopic>''|
		<idx|topic: ``&<hometopic>''|


!!"Home" or top-level help topic!!   

Identifies the start of the top-level help topic.


<procedure> Syntax

<ex>
&<hometopic>%%heading%%

%%topic text begins here ...%%
<\ex>

There is only one home topic for a help volume.  It comes after the meta
information (``&<metainfo>'') and before the first ``&<chapter>'' or
``&<s1>''.


<note><head>Linking to the Home Topic

The ``&<hometopic>'' element does not support an author-defined ID
identifier.  The &TagAppName; software assigns the predefined identifier
``_hometopic''.  To create a hyperlink to the home topic, use ``&<link
hyperlink="_hometopic">...&<\link>''.

<\note>


<procedure> Example

<ex smaller>
&<hometopic>Welcome to Online Help

This is the home topic for the online help ...

&<chapter>First Subtopic

This is the first subtopic ...

&<chapter>Second Subtopic

This is the second subtopic ...

&vellipsis;
<\ex>


<procedure> See Also

<list plain tight>
 * <xref IntroducingThePartsOfAHelpVolumeTO>
 * <xref ToCreateHomeTopicTA>
 * <xref metainfo-ReferenceSection>
<\list>


<\rsect>








<rsect id=idx-ReferenceSection > &<idx>

		<idx|``&<idx>''|
		<idx|element tag: ``&<idx>''|
		<idx|indexing|
		<idx|keyword index: marking entry|
		<idx|marking: index entry|
		<idx|index, keyword: marking entry|

		<idx|``&<sort>''|
		<idx|element tag: ``&<sort>''|
		<idx|index, keyword: sort order|
		<idx|sort order, index|
		<idx|order, keyword index sort|
		<idx|changing: index sort order|


!!Index entry!!

Defines an entry to appear in the keyword index.


<procedure> Syntax

<ex>
&<idx>%%text%%&<\idx>
<\ex>

!!Or:!!

<ex>
&<idx|%%text%%|
<\ex>

!!Or:!!

<ex>
&<idx>%%text%%&<sort>%%sort key%%&<\idx>
<\ex>

Where:

<lablist>
\%%text%%\ The text string that appears in the keyword index.
\%%sort key%%\ An optional text string used when sorting the index.
           The %%sort key%% influences where the %%text%% appears
           in the keyword index.
           The %%sort key%% string does not appear in
           the keyword index.
<\lablist>

The keyword index is displayed by choosing Keyword from the Search menu
in a general help dialog.  (The keyword index is not available in quick
help dialogs.)
When the index entry is chosen in the Keyword Index dialog, the
topics that contain the index entry are listed.  Choosing one of the listed
topics displays that topic.  The Keyword Index dialog
remains available for further keyword index access.

Either the ``&<idx>'' start and end tags or the short form can be used.

The ``&<sort>'' element changes the sort order for a keyword index
entry.  Specifically, the ``&<sort>'' element is used within the ``&<idx>''
element to request that the keyword appear at the location
indicated by the %%sort key%% string.  No end tag for ``&<sort>''
is required.


<need begin>

<procedure> Examples

The following markup shows the definition of some simple index entries.
The index entries are indented to make the source text easier to read.

<ex smaller>
          &<idx|keyboard|
          &<idx|disk drive|
          &<idx|screen, LCD|
          &<idx|personal computer, portable|
          &<idx|portable, personal computer|

An portable PC has a full
size keyboard, built-in disk drives and
a detachable LCD screen.

<\ex>

The following example displays "+" in the keyword index,
but it appears where "plus" would appear.

<ex smaller>
&<idx>+&<sort>plus&<\idx>
<\ex>

<need end>

<\rsect>








<rsect id=image-ReferenceSection > &<image>

		<idx|``&<image>''|
		<idx|element tag: ``&<image>''|
		<idx|unchanged: text|
		<idx|text: unchanged|
		<idx|unformatted text|
		<idx|``id'' parameter: in ``&<image>''|
		<idx|parameter: ``id'' in ``&<image>''|
		<idx|``indent'' parameter|
		<idx|parameter: ``indent''|
		<idx|line breaks, preserving|
		<idx|spacing, preserving|


!!As-is image!!

Shows text with the same line breaks as are in the source text.


<procedure> Syntax

<ex>
&<image [id=%%id%%] [indent]>
%%text%%
&<\image>
<\ex>

<!-- in the next release, we should expose the graphics capabilities of the
image element -->

Text between the ``&<image>'' and ``&<\image>'' tags is shown with the same
spacing, indentation and line breaks that appear in the actual text.
No justification, word wrapping or removal of empty lines is done.  However,
a proportional font is used, so columns of text that are lined up on a
computer screen may not line up in the displayed help information.  

All in-line text elements and special characters are recognized.

The ``indent'' parameter causes the displayed %%text%% to be indented
from the left margin.

Either the start and end tags (``&<image>'' and ``&<\image>'') or the
short form (``&<image|...|'') can be used.

If the displayed text is too wide to fit within the display area,
a horizontal scroll bar automatically appears.


<procedure> See Also

<list plain tight>
* <xref ex-ReferenceSection>
* <xref vex-ReferenceSection>
* <xref p-ReferenceSection>
<\list>




<\rsect>








<rsect id=item-ReferenceSection > &<item>

		<idx|``&<item>''|
		<idx|element tag: ``&<item>''|
		<idx|list: ``&<item>''|
		<idx|list: item in|
		<idx|item in list|
		<idx|cross-reference: listing|
		<idx|ID: in list|
		<idx|``id'' parameter: in ``&<item>''|
		<idx|parameter: ``id'' in ``&<item>''|


!!List item!!

Identifies an item in a list.  


<procedure> Syntax

<ex>
&<list>
 * List item
 * List item
&<\list>
<\ex>

!!Or:!!

<ex>
&<list order>
  &<item id=%%name%%> List item
  &<item id=%%name%%> List item
  &<item id=%%name%%> List item
    &vellipsis;
&<\list>
<\ex>


The shorthand form, which is an asterisk (``*''), is almost always used.

The long form allows you to cross-reference an item in a list.
You can only cross-reference items in an ordered list because the 
automatically-assigned item numbers are used in the cross-reference text
(which &TagAppName; substitutes for the ``&<xref>'' element).


<procedure> See Also

<list plain tight>
* <xref list-ReferenceSection>
* <xref head-ReferenceSection>
* <xref xref-ReferenceSection>
<\list>


<\rsect>








<rsect id=lablist-ReferenceSection > &<lablist>

		<idx|labeled list|
		<idx|list: labeled|

		<idx|``&<lablist>''|
		<idx|element tag: ``&<lablist>''|
		<idx|list: ``&<lablist>''|

		<idx|list: ``&<labheads>''|
		<idx|``&<labheads>''|
		<idx|element tag: ``&<labheads>''|
		<idx|list: labeled, heading|

		<idx|column heading in labeled list|
		<idx|``loose'' parameter|
		<idx|``tight'' parameter|
		<idx|parameter: ``loose''|
		<idx|parameter: ``tight''|


!!Labeled list!!

Starts a labeled list in which the labels appear in the left column and the
items (to which the labels refer) appear in the right column.


<procedure> Syntax

<syntax>
&<lablist [loose | tight]>
<\syntax> 
<ex>
[ &<labheads> \Heading 1 \ Heading 2 ]

\%%label%%\ %%text for the first item%%
\%%label%%\ %%text for the second item%%
   &vellipsis;
&<\lablist>
<\ex>

Where:

<lablist>

\``loose''\ (Default.)  Requests a vertical gap between the items in the list.

\``tight''\ Requests no extra vertical space between items in the list.

<\lablist>

		<idx|backslash (``&\'')|
		<idx|``&\'' (backslash)<sort>&\|

Backslashes (``&\'') indicate the start and end of a label; leading and
trailing spaces are ignored.  The text of the labeled item follows the
second backslash, either on the same line or on the following line.  The
end of the item is indicated by one of the following:

<list bullet tight>
* An empty line.
* The start of another labeled item.
* The ``&<\lablist>'' end tag.
<\list>

If a labeled item consists of more than one paragraph, leave an empty line
between the paragraphs.  The end of the labeled list is indicated by the
required ``&<\lablist>'' end tag.

Labels that are wider than the predefined label area extend into the right
column.

		<idx|backslash (``&\'')|
		<idx|``&\'' (backslash)<sort>&\|

The optional column headings, one for each column, immediately follow the
``&<labheads>'' tag (on the same line).  The column headings are
separated from one another by the backslash (``\'') character.  The
``&<\labheads>'' end tag is not required.  However, the ``&<lablist>''
end tag is required.


<procedure> Example

The following markup:

<ex smaller>
&<lablist tight>
&<labheads> \ Unit \ Meaning
  &\in\ inches
  &\pc\ picas
  &\pt\ points
  &\mm\ millimeters
  &\cm\ centimeters
&<\lablist>
<\ex>

produces this output:

<lablist tight>
<labheads> \ Unit \ Meaning
  \in\ inches
  \pc\ picas
  \pt\ points
  \mm\ millimeters
  \cm\ centimeters
<\lablist>


<procedure> See Also

<list plain tight>
* <xref head-ReferenceSection>
* <xref list-ReferenceSection>
<\list>


<\rsect>








<rsect id=link-ReferenceSection > &<link>

		<idx|``&<link>''|
		<idx|element tag: ``&<link>''|
		<idx|hyperlink|
		<idx|link: hypertext|
		<idx|cross-reference|


!!Hyperlink!!

Delimits text or an inline ``&<graphic>'' to be used as a hyperlink.

<procedure> Syntax

<ex>
&<link %%hyperlink%% [%%type%%] ["%%description%%"]>%%text%%&<\link>
<\ex>

!!Or,!!

<ex>
&<link hyperlink="%%hyperlink%%" [type=%%type%%] [description="%%description%%"]>
<\ex>


The %%hyperlink%% attribute, which is required, is a value that identifies
the destination or the behavior for the link.  For a standard "jump" link,
%%hyperlink%% is the ID of the element you want to jump to.

The %%type%% parameter can have the following values:

		<idx|``Jump'' parameter|
		<idx|parameter: ``Jump''|
		<idx|``JumpNewView'' parameter|
		<idx|parameter: ``JumpNewView''|
		<idx|``Definition'' parameter|
		<idx|parameter: ``Definition''|
		<idx|``Execute'' parameter|
		<idx|parameter: ``Execute''|
		<idx|``Man'' parameter|
		<idx|parameter: ``Man''|
		<idx|``AppDefined'' parameter|
		<idx|parameter: ``AppDefined''|

<lablist>
 \ ``Jump'' \	(Default.) Jumps to the topic that contains the ID %%hyperlink%%.

 \ ``JumpNewView'' \ 	Jumps to the topic that contains the ID %%hyperlink%%,
			but requests that the hosting application display the 
			topic in a new window.

 \ ``Definition'' \	Displays, in a temporary definition window, the topic
			which contains the ID %%hyperlink%%.

 \ ``Execute'' \	Executes the %%hyperlink%% string as a command.

 \ ``Man'' \	Displays a man page using the %%hyperlink%% string as the
		parameter(s) to the ``man'' command.

 \ ``AppDefined'' \	Sends the %%hyperlink%% string to the hosting
			application for special processing.  
<\lablist>

The %%text%% between the begin and end tags becomes the "hot spot" that the
user will choose to invoke the link.  

Capitalization is not significant for the %%hyperlink%% and %%type%% values.


<procedure> Notes

<list bullet>
* Avoid using the %%type%% keywords (listed above) as values for
  %%hyperlink%%.  If you must do so, explicitly identify the
  parameters as shown in the second syntax line above.
<\list>

<list bullet>
* The ``&<link>'' element is not needed in a cross-reference that uses the
  ``&<xref>'' element because a hyperlink is automatically created
  where the ``&<xref>'' element is used.

<\list>


<procedure> Examples

The following markup defines a simple hyperlink to the topic with the
ID ``Welcome''.  Notice that capitalization of the ID is not significant.

<ex smaller>
Refer to the &<link welcome>Welcome&<\link> topic.
<\ex>


The following markup defines the same hyperlink jump as
in the previous example but the ``&<link>'' element is not used because a
cross-reference (``&<xref...>'') is automatically a hyperlink.
In this case, the title of the ``Welcome'' topic is automatically
supplied by &TagAppName;.

<ex smaller>
Refer to the &<xref welcome> topic.
<\ex>


The following markup defines a hyperlink that is activated when the
inline graphic is chosen.  A new window is opened to display the
referenced help information.

<ex smaller>
The &<link clockInfo JumpNewView>
&<graphic entity=ClockIcon>&<\link> icon ...
<\ex>


The following markup creates a link that displays the man page for the
``grep'' command:

<ex smaller>
For more details, refer to the 
&<link grep Man>grep man page&<\link>.
<\ex>


<procedure> See Also

<list plain tight>
 * <xref abstract-ReferenceSection>
 * <xref figure-ReferenceSection>
 * <xref graphic-ReferenceSection>
 * <xref hometopic-ReferenceSection>
 * <xref idx-ReferenceSection>
 * <xref location-ReferenceSection>
 * <xref xref-ReferenceSection>
<\list>


<\rsect>








<rsect id=list-ReferenceSection > &<list>

		<idx|``&<list>''|
		<idx|element tag: ``&<list>''|
		<idx|bullet|
		<idx|bulleted list|
		<idx|list: bulleted|
		<idx|numbered list|
		<idx|list: numbered|
		<idx|plain list|
		<idx|list: plain|
		<idx|``&<item>''|
		<idx|leader: dotted|
		<idx|dotted leader|
		<idx|leader: blank|
		<idx|blank leader|
		<idx|list: leader|
		<idx|``bullet'' parameter|
		<idx|``order'' parameter|
		<idx|``plain'' parameter|
		<idx|``continue'' parameter|
		<idx|``loose'' parameter|
		<idx|``tight'' parameter|
		<idx|parameter: ``bullet''|
		<idx|parameter: ``order''|
		<idx|parameter: ``plain''|
		<idx|parameter: ``continue''|
		<idx|parameter:``loose''|
		<idx|parameter: ``tight''|

!!List!!

Starts a list consisting of items that are optionally marked with
bullets or automatically-generated numbers.

<procedure> Syntax

<ex>
&<list [bullet | order | plain] [continue] [loose | tight]>
  * %%first item%%
  * %%second item%%
     &vellipsis;
&<\list>
<\ex>


Where:

<lablist>

\ ``bullet'' \ (Default.)  Displays a bullet before each item.

\``plain''\ Does not put a bullet, number or letter in front of each item.

\``order'' \ Displays a number in front of each item.  The numbers are
automatically generated and begin with the number one.

\``continue''\ Requests that the numbering of items continue from the
previous list.  

\``loose'' \ (Default.)  Requests a vertical gap between the items.

\``tight''\ Requests no extra vertical spacing between the items.
<\lablist>


Each item must start on a new line preceded by either an asterisk (``*'')
or the ``&<item>'' tag.  The asterisk is the shorthand form of the
``&<item>'' tag.  Spaces and tabs may appear on either side of the asterisk.
Items may continue over multiple lines.  An item can consist of multiple
paragraphs, in which case an empty line must separate the paragraphs.
The nesting of lists is allowed, so a list can appear within a list.

The ``&<\list>'' end tag is required.

<newpage>

<procedure> Examples

The following markup:

<ex smaller>
&<list>
* chocolate
* raspberry
* vanilla
&<\list>
<\ex>

produces:

<list>
* chocolate
* raspberry
* vanilla
<\list>


The following markup:

<ex smaller>
&<list plain tight>
* Word Processing
* Graphics
* Printing
&<\list>
<\ex>

produces:

<list plain tight>
* Word Processing
* Graphics
* Printing
<\list>


<procedure> See Also

<list plain tight>
* <xref item-ReferenceSection>
* <xref lablist-ReferenceSection>
* <xref head-ReferenceSection>
<\list>


<\rsect>








<rsect id=location-ReferenceSection > &<location>

		<idx|``&<location>''|
		<idx|element tag: ``&<location>''|
		<idx|indexing|
		<idx|hyperlink: destination|
		<idx|destination: hyperlink|
		<idx|cross-reference: destination|
		<idx|destination: cross-reference|
		<idx|``id'' parameter: in ``&<location>''|
		<idx|parameter: ``id'' in ``&<location>''|


!!Location!!

Defines an ID as referring to the location of the ``&<location>'' element.
The ID is usually used as a destination for a hyperlink or cross-reference.


<procedure> Syntax

<ex>
&<location id=%%id%%>%%text%%&<\location>
<\ex>

!!Or:!!

<ex>
&<location id=%%id%%|%%text%%|
<\ex>

Where:

<lablist>
\%%id%%\ The identifier for the current location, which can be used as a
	 destination for hyperlinks.

\%%text%%\ The block of text where you want to assign the ID.  

<\lablist>

The ``&<location>'' element is not needed at locations where there is
already an element (such as ``&<hometopic>'' and ``&<figure>'') that
has a built-in ID or accommodates an author-defined ``id'' parameter.

The ``&<location>'' element enables portions of a topic or section to serve
as destinations for hyperlinks and cross-references.  


<procedure> Example

The following markup names a location and elsewhere creates a hyperlink to
the location.  

<ex smaller>
&<s1 id=ConfigTopic> Configuration
    ...
&<location id=SecondHalfConfigTopic>%%some text%%&<\location>
    ...
&<s1 id=UseTopic> Usage
    ...
See &<link SecondHalfConfigTopic>Configuration&<\link>
for additional information.
<\ex>

The advantage of linking to the ID in the ``&<location>'' element is that
the help window automatically scrolls to the point where the
``&<location>'' is entered.  In contrast, a link to the topic's ID
("ConfigTopic" in this case), always goes to the top of the topic.


<procedure> See Also

<list plain tight>
* <xref xref-ReferenceSection>
* <xref link-ReferenceSection>
<\list>


<\rsect>








<rsect id=memo-ReferenceSection > &<memo>

		<idx|``&<memo>''|
		<idx|element tag: ``&<memo>''|
		<idx|draft comment or question|

!!Memo!!

Identifies a writer's comments or questions, which do not appear in the
final help volume.


<procedure> Syntax

<ex>
&<memo>
%%memo text%%
&<\memo>
<\ex>

!!Or:!!

<ex>
&<memo|%%memo text%%|
<\ex>


Memo text is printed in drafts of your help volume 
!!if!! you specify ``memo'' in the ``helptag.opt'' file.
Otherwise, memo text is not printed, especially when you create 
the ``final'' version of the help volume.
Memo text, when it appears, is printed in a different typeface.


<procedure> Examples

Here is an example of a memo:

<ex smaller>
&<memo>
Patti: We need a drawing to illustrate this.
&<\memo>
<\ex> 

The following markup uses the short form of the ``&<memo>'' element:

<ex smaller>
&<memo|Mike: Please explain how the following
command is supposed to work|
<\ex>


<\rsect>







<rsect id=metainfo-ReferenceSection > &<metainfo>

		<idx|``&<metainfo>''|
		<idx|element tag: ``&<metainfo>''|
		<idx|meta information|
		<idx|front matter|
		<idx|information about information|
		<idx|``&&copy''; entity|
		<idx|copyright: notice|


!!Meta information!!  

Starts the meta information section, which contains information about the
information contained in the help volume.  This information includes the
volume's title and a copyright notice.


<procedure> Syntax

<ex smaller>
&<helpvolume>
  &<metainfo>
    &<title> %%volume title%%
    &<copyright>
    &&copy; %%Copyright XYZ Company 1993...%%
    &<abstract>
    %%brief description of help volume%%
      &vellipsis;
    &<\metainfo>
&<hometopic>...
  &vellipsis;
<\ex>

The meta information section is required.  The title and copyright
subsections are required within the meta information section.  
Inclusion of the abstract subsection is strongly recommended.

The ``&<otherfront>'' element can be used to define subsections
other than the predefined title, copyright and abstract subsections.

The ``&<\metainfo>'' end tag is required.


<procedure> Example

<ex smaller>
&<metainfo>

&<title>Inventory Tracking Software

&<copyright>
&&copy; Copyright 1993 XYZ Company.  
All rights reserved.

&<abstract>
Explains how to use the Inventory Tracking Software
    
&<\metainfo>
<\ex>


<procedure> See Also

<list plain tight>
* <xref title-ReferenceSection>
* <xref copyright-ReferenceSection>
* <xref abstract-ReferenceSection>
* <xref otherfront-ReferenceSection>
<\list>


<\rsect>








<rsect id=newline-ReferenceSection > &<newline>

		<idx|``&<newline>''|
		<idx|element tag: ``&<newline>''|
		<idx|line break: forcing|
		<idx|break, forcing line|
		<idx|new line: forcing start of|

!!New line!!

Starts a new line within a paragraph.


<procedure> Syntax

<ex>
%%text%%&<newline>%%text on next line%%
<\ex>


Text that follows the ``&<newline>'' element begins on a new line.


<procedure> Example

The following markup ensures that the file name begins on a new
line:

<ex smaller>
Put your files for the manual in the special directory 
&<newline>``/userguide/draftdoc''.
<\ex>


<procedure> See Also

<list plain tight>
* <xref vex-ReferenceSection>
* <xref ex-ReferenceSection>
* <xref image-ReferenceSection>
<\list>


<\rsect>








<rsect id=note-ReferenceSection > &<note>

		<idx|``&<note>''|
		<idx|element tag: ``&<note>''|
		<idx|important information, calling attention to|
		<idx|attention to important information, calling|


!!Note!!

Creates a special format that attracts attention to text which
makes an important point.


<procedure> Syntax

<ex>
&<note>
%%text of note%%
&<\note>
<\ex>

The default heading for the note is "Note".  To specify a different heading,
use the ``&<head>'' element. 

If you want an icon to appear with the note, define
``NoteElementDefaultIconFile'' in an ``&<!entity &ellipsis>'' declaration.

The ``&<\note>'' end tag is required.


<procedure> Examples

Here is a note that uses the default heading:

<ex smaller>
&<note>
Pay attention; this is important.
&<\note>
<\ex>


The following markup specifies a different heading:

<ex smaller>
&<note>&<head>Read This
Pay attention; this is important.
&<\note>
<\ex>


<procedure> See Also

<list plain tight>
* <xref caution-ReferenceSection>
* <xref warning-ReferenceSection>
* <xref head-ReferenceSection>
<\list>


<\rsect>








<rsect id=otherfront-ReferenceSection > &<otherfront>

		<idx|``&<otherfront>''|
		<idx|element tag: ``&<otherfront>''|
		<idx|front matter: uncategorized|
		<idx|``id'' parameter: in ``&<otherfront>''|
		<idx|parameter: ``id'' in ``&<otherfront>''|


!!Other meta information (front matter)!!


Used for meta information (front matter) that does not fit within
one of the predefined categories.


<procedure> Syntax

<ex>
&<metainfo>
  &vellipsis;
&<otherfront [id=%%id%%] >&<head>%%title of section%%

  %%text%%
<\ex>


If a heading is needed, use the ``&<head>'' element.

``&<otherfront>'' must follow all other subsections of ``&<metainfo>''.


<procedure> See Also
<list plain tight>
* <xref metainfo-ReferenceSection>
* <xref head-ReferenceSection>
<\list>


<\rsect>








<rsect id=otherhead-ReferenceSection > &<otherhead>

		<idx|``&<otherhead>''|
		<idx|element tag: ``&<otherhead>''|
		<idx|subheading, within topic|
		<idx|topic: subheading within|


!!Other heading!!

Creates a subheading within a topic.  


<procedure> Syntax

<ex>
&<otherhead>%%heading%%
<\ex>


Headings may occur anywhere within the text of a topic.  The
``&<otherhead>'' element does not alter the topic hierarchy and
its title does not appear in the Topic Hierarchy list.


The ``&<\otherhead>'' end tag is not needed.


<procedure> Example

Here is an example in which ``&<otherhead>'' elements identify
two subsections within an ``&<s1>'' topic:

<ex smaller>
&<s1>Getting Started
%%text%%

&<otherhead>Copying Files
%%text%%

&<otherhead>Editing Configuration Files
%%text%%
<\ex>


<procedure> See Also

<list plain tight>
 * <xref head-ReferenceSection>
 * <xref procedure-ReferenceSection>
 * <xref rsect-ReferenceSection>
 * <xref s1-ReferenceSection>
<\list>



<\rsect>








<rsect id=p-ReferenceSection > &<p>

		<idx|``&<p>''|
		<idx|element tag: ``&<p>''|
		<idx|paragraph: indenting|
		<idx|indenting paragraph|
		<idx|starting paragraph|
		<idx|new paragraph, starting|
		<idx|``id'' parameter: in ``&<p>''|
		<idx|parameter: ``id'' in ``&<p>''|
		<idx|``indent'' parameter|
		<idx|``gentity'' parameter|
		<idx|``gposition'' parameter|
		<idx|``ghyperlink'' parameter|
		<idx|``glinktype'' parameter|
		<idx|parameter: ``indent''|
		<idx|parameter: ``gentity''|
		<idx|parameter: ``gposition''|
		<idx|parameter: ``ghyperlink''|
		<idx|parameter: ``glinktype''|


!!New paragraph!!

Starts a paragraph that is indented or wrapped around a graphic.


<procedure> Syntax

<ex>
&<p [indent] [gentity=%%graphic-ent%% [gposition=%%pos%%]
[ghyperlink=%%gid%% [glinktype=%%type%%]]] [id=%%id%%] >%%text...%%
<\ex>

Where:

<lablist>

\``indent''\ Optional.  Specifies that the paragraph be indented from the
  current left margin.

\%%graphic-ent%%\ Optional.  The name of a graphic entity around which
  the paragraph is to be wrapped.  The ``gentity'' parameter and
  %%graphic-ent%% value are required if the ``gposition'', ``ghyperlink'',
  or ``glinktype'' parameter is used.

\%%pos%%\ Optional. Either ``left'' or ``right'' to indicate whether
  the optional graphic is to be left-justified or right-justified.

\%%gid%%\  Optional.  Specifies that the graphic be a hyperlink and
  specifies the destination of the hyperlink.
  The ``ghyperlink'' parameter and %%gid%% value are required if the
  ``glinktype'' parameter is used.  (The %%id%% value, not
  the %%gid%% value, would be used to reference this paragraph's location.)

\%%type%%\  Optional.  Specifies the type of hyperlink.  The
  default type is ``Jump''.  Other type values
  include ``JumpNewView'', ``Definition'', ``Man'',
  ``Execute'', and ``AppDefined''.

\%%id%%\ Optional. Defines an ID name that can be used in
  cross-references to this location.

\%%text%%\ The text of the paragraph that wraps around the graphic.

<!-- in the next release, we should also expose the gdescription field -->

<\lablist>


Use the ``&<p>'' element only if you need to indent a paragraph or
wrap the paragraph around a graphic.

A ``&<\p>'' end tag is not required. 


<need begin>

<procedure> Example

Here are two paragraphs, the second of which is indented:

<ex smaller>
Some people do not like to read manuals.

&<p indent>This is not always a good idea.
<\ex>

<need end>


<procedure> See Also

<list plain tight>
 * <xref head-ReferenceSection>
 * <xref procedure-ReferenceSection>
 * <xref rsect-ReferenceSection>
 * <xref s1-ReferenceSection>
 * <xref ToWrapTextAroundGraphicTA>
<\list>

<\rsect>






<rsect id=procedure-ReferenceSection > &<procedure>

		<idx|``&<procedure>''|
		<idx|element tag: ``&<procedure>''|
		<idx|procedure, describing|
		<idx|section: describing procedure|


!!Procedure!!

Starts a section within a topic.


<procedure> Syntax

<ex>
&<procedure>%%heading%%
%%procedure text...%%
<\ex>

Procedures may occur anywhere within the text of a topic.  They are
not included in the Topic Hierarchy's table of contents.

An end tag is not needed.


<procedure>Example

This paragraph and the "Example" heading were
produced using the following markup:


<ex smaller>
&<procedure>Example

This paragraph and the "Example" heading were
produced using the following markup:
<\ex>


<procedure> See Also

<list plain tight>
 * <xref head-ReferenceSection>
 * <xref otherhead-ReferenceSection>
 * <xref s1-ReferenceSection>
<\list>


<\rsect>








<rsect id=quote-ReferenceSection > &<quote>

		<idx|``&<quote>''|
		<idx|element tag: ``&<quote>''|
		<idx|quotation marks, printing text within|
		<idx|mark, quotation: printing text within|


!!Quote!!

Puts text within directional quotation marks.  


<procedure> Syntax

<ex>
&<quote>%%text%%&<\quote>
<\ex>

!!Or:!!

<ex>
<esc>"<\esc>%%text%%<esc>"<\esc>
<\ex>


Use the start and end tags (``&<quote>'' ... ``&<\quote>'') or
a pair of double quotation marks (<esc>"<\esc>...<esc>"<\esc>)
to delimit the text.


<procedure> Example

The following markup: 

<ex>
... referred to in this manual as "the Standard" ...
<\ex>

produces:

<p indent>
... referred to in this manual as "the Standard" ...


<procedure> See Also

<list plain tight>
 * <xref book-ReferenceSection>
 * <xref computer-ReferenceSection>
 * <xref term-ReferenceSection>
 * <xref user-ReferenceSection>
 * <xref var-ReferenceSection>
<\list>


<\rsect>








<rsect id=rsect-ReferenceSection > &<rsect>

		<idx|``&<rsect>''|
		<idx|element tag: ``&<rsect>''|
		<idx|reference: section|
		<idx|section: reference|

		<idx|``&<rsub>''|
		<idx|element tag: ``&<rsub>''|
		<idx|reference: subsection|
		<idx|subsection within reference section|
		<idx|section: subsection within reference|
		<idx|``id'' parameter: in ``&<rsect>''|
		<idx|parameter: ``id'' in ``&<rsect>''|


!!Reference section!!

Identifies an entry in the reference section.


<procedure> Syntax

<ex>
&<rsect [id=%%id%%] >%%reference section heading%%
  &vellipsis;
&<rsub>%%reference subsection heading%%
<\ex>

A reference section (``&<rsect>'') is used within a
topic or section, typically for a series of similar
sections.  For example, each reference section could describe
one software command.

An ``&<rsect>'' consists of:

<list tight>
* Required heading.
* Optional introductory text.
* Optional reference subsections or ``&<rsub>''s.
<\list>

Each ``&<rsect>'' section can have multiple ``&<rsub>'' sections.
Each ``&<rsub>'' element must have a heading, but the heading
does not appear in the table of contents.
A cross-reference to a reference subsection is not allowed.

``&<rsect>'' headings (but not ``&<rsub>'' headings) appear in the table of
contents.

End tags (for either ``&<rsect>'' or ``&<rsub>'') are not required.  


<procedure> Example

The following markup illustrates the use of this element:

<ex smaller>
&<rsect>purge
  &vellipsis;
&<rsub>Syntax
purge <esc|%%|filename<esc|%%|
&<rsub>Example
purge file01

<\ex>


<procedure> See Also

<list plain tight>
 * <xref abbrev-ReferenceSection>
 * <xref chapter-ReferenceSection>
 * <xref s1-ReferenceSection>
<\list>


<\rsect>







<rsect id=s1-ReferenceSection > &<s1>...&<s9>

		<idx|``&<s1>'' .&empty;.&empty;. ``&<s9>''|
		<idx|element tag: ``&<s1>'' .&empty;.&empty;. ``&<s9>''|
		<idx|section: heading within topic|
		<idx|section: heading|
		<idx|section: topic|
		<idx|heading, section|
		<idx|topic|
		<idx|``id'' parameter: in ``&<s1>'' .&empty;.&empty;. ``&<s9>''|
		<idx|parameter: ``id'' in ``&<s1>'' .&empty;.&empty;. ``&<s9>''|
		<idx|title: topic|
		<idx|topic: title|


!!Subsection (!!``&<s1>'', ``&<s2>'', .&empty;.&empty;. , ``&<s9>''!!)!!

Starts a topic in the hierarchy.


<procedure>Syntax

<ex>
&<s%%n%% [id=%%name%%] >%%heading%%
%%topic text%%...
<\ex>

Where %%n%% is the level number (1, 2, ..., or 9).  


Topics entered with ``&<chapter>'' can have subtopics entered with
``&<s1>'', ``&<s1>'' topics can have ``&<s2>'' subtopics, and so on.
You !!cannot!! skip a level.

The heading for a section can be on the same line as the ``&<s%%n%%>'' tag
or on the next line; a heading is required.  Text within a section is optional.

The end tag is usually omitted, but in some instances the end tag may
be necessary.  For example, when a section is followed by an ``&<rsect>''
element that is on the same level, an end tag for the section is required.
Without the end tag the ``&<rsect>'' element would be considered a subsection
of the section preceding it.


<procedure> Examples

The following example illustrates a three-level hierarchy within a topic.

<ex smaller>
&<chapter>Running the Processor
%%topic text...%%

&<s1>Getting Started
%%To run the program, type in the user
code and your password.%%

&<s1>Customizing
%%You may now set up this conversion program
to change your computer from beige to red.%%

&<s2>Configuration
%%Use either the disk drive or the tape drive
to archive your files.%%

&<s3>Disk Drive Advantages
%%See data sheet for specifications.%%

&<s3>Tape Drive Advantages
%%See data sheet for specifications.%%

&<s2>Support
%%If you really need help, call technical support.%%

<\ex>

In the following markup, a section end tag (``&<\s1>'') is used to
make the ``&<rsect>'' section be at the same level in the hierarchy.

<ex smaller>
&<s1>%%first level heading%%
%%text%%

&<s1>%%first level heading%%
%%text%%
&<\s1>

&<rsect>%%first level heading%%
%%text%%
<\ex>


In contrast, leaving out the end tag causes the ``&<rsect>''
section to become a subtopic of the second ``&<s1>'' section:

<ex smaller>
&<s1>%%first level heading%%
%%text%%

&<s1>%%first level heading%%
%%text%%

&<rsect>%%second level heading%%
%%text%%
<\ex>


<procedure> See Also

<list plain tight>
 * <xref chapter-ReferenceSection>
 * <xref head-ReferenceSection>
 * <xref rsect-ReferenceSection>
<\list>


<\rsect>








<rsect id=term-ReferenceSection > &<term>

		<idx|``&<term>''|
		<idx|element tag: ``&<term>''|
		<idx|``+'' (plus)<sort>+|
		<idx|plus (``+'')|
		<idx|symbol: plus (``+'')|
		<idx|glossary: term, newly introduced|
		<idx|term: newly introduced|
		<idx|defining: newly introduced term|
		<idx|``gloss'' parameter|
		<idx|parameter: ``gloss''|
		<idx|``nogloss'' parameter|
		<idx|parameter: ``nogloss''|


!!Glossary term!!

Writes a newly introduced term in a special font and
establishes a hyperlink to its definition in the glossary.


<procedure> Syntax

<syntax>
&<term %%baseform%% [gloss | nogloss]>%%text%%&<\term>
<\syntax>

!!Or:!!

<syntax>
&<term %%baseform%% [gloss | nogloss] |%%text%%|
<\syntax>

!!Or:!!

<ex>
<esc|++|%%text%%<esc|++|
<\ex>


Where:

<lablist>
\%%baseform%%\ The form of the term as it appears in the glossary
if it is not the same as used in the text.  This difference can
occur, for example, when the term is used in the text in its plural
form but appears in the glossary in its singular form.  If the term
includes spaces or special characters, put the %%baseform%% string
in quotes.

\``gloss''\ (Default.)  Requests that &TagAppName verify that the term is
in the glossary.

\``nogloss''\ Allows the term to be missing from the glossary.
<\lablist>

The shorthand form for ``&<term>'' is double plus signs
(``<esc|++|'') used before and after the term.

<note>
If your help volume does not include a glossary, use
the ``nogloss'' parameter.
<\note>

When &TagAppName; processes the help volume, warning messages are issued to
indicate glossary terms that both do not appear in the glossary and
do not use the ``nogloss'' parameter.

Tagging a term with the ``&<term>'' element automatically creates a
hyperlink to the glossary.  If there is no glossary, the link will
not work.

A ``&<\term>'' end tag is required if the long form is used.


<procedure> Example

The following markup puts "structural elements" in a special font to
indicate it is a glossary term and creates a hyperlink to the glossary.
Because the ``nogloss'' parameter is not used, &TagAppName; ensures that the
singular form ("structural element") appears in the glossary.

<ex smaller>
SGML views a document as a hierarchy of
&<term "structural element"|structural elements|.
<\ex>


<procedure> See Also

<list plain tight>
 * <xref glossary-ReferenceSection>
 * <xref dterm-ReferenceSection>
<\list>


<\rsect>








<rsect id=title-ReferenceSection > &<title>

		<idx|``&<title>''|
		<idx|element tag: ``&<title>''|
		<idx|meta information: volume title|
		<idx|guide title|
		<idx|manual title|
		<idx|volume: title|
		<idx|document title|
		<idx|title: help volume|


!!Help volume title!!

Specifies the title of the help volume.


<procedure> Syntax

<ex>
&<metainfo>

&<title>%%help volume title%%
<\ex>


The ``&<title>'' element is a required subelement of the ``&<metainfo>''
(meta information) element.  It follows immediately after the
``&<metainfo>'' tag.  Because this is the title of the volume, and the title
may be displayed by other applications (help managers, for example) that
may not be able to format the title, you should avoid anything other than
plain text within the title.

The ``&<\title>'' end tag is not required.


<procedure> Example

Here is a sample volume title:

<ex smaller>
&<metainfo>

&<title>%%The Super Hyperlink User's Guide%%
<\ex>



<procedure> See Also

<list plain tight>
* <xref metainfo-ReferenceSection> 
<\list>


<\rsect>








<rsect id=user-ReferenceSection > &<user>

		<idx|``&<user>''|
		<idx|element tag: ``&<user>''|
		<idx|example: using ``&<user>''|
		<idx|computer: text|
		<idx|user: response to computer|
		<idx|prompt, user response to computer|
		<idx|computer: prompt, user response to|


!!User's response!!

Indicates the user's response to a computer prompt.  


<procedure> Syntax

<ex>
&<user>%%response%%&<\user>
<\ex>

!!Or:!!

<ex>
&<user|%%response%%|
<\ex>


This element is used to distinguish user input from computer output in
a computer dialogue.  It is typically used within the ``&<ex>'' element,
where spaces and line breaks between the ``&<user>'' start tag and the
``&<\user>'' end tag are significant.

If used within a paragraph, ``&<user>'' text must not break across lines in
your source file.

The ``&<user>'' end tag is required if the long form is used.


<procedure> Example

The following markup produces two different fonts, one to indicate
what the computer displays and another to indicate what the user types:

<ex smaller>
&<ex>
Do you wish to continue? (Yes or No) &<user>Yes&<\user>
&<\ex>
<\ex>


<procedure> See Also

<list plain tight>
* <xref computer-ReferenceSection>
* <xref ex-ReferenceSection>
* <xref var-ReferenceSection>
* <xref vex-ReferenceSection>
<\list>


<\rsect>








<rsect id=var-ReferenceSection > &<var>

		<idx|``&<var>''|
		<idx|element tag: ``&<var>''|
		<idx|italic font|
		<idx|font: italic|
		<idx|variable: marking in command|
		<idx|command variable, marking|
		<idx|marking: command variable|
		<idx|``%'' (percent symbol)<sort>%|


!!Variable!!

Indicates a user-supplied variable in a command.


<procedure> Syntax

<ex>
&<var>
%%text%%
&<\var>
<\ex>

!!Or:!!

<ex>
<esc|%%|text<esc|%%|
<\ex>


The ``&<\var>'' end tag is required if the long form is used.

In the shorthand form, the text is delimited with double percent
signs (``<esc|%%|'').


<procedure> Examples

These markups: 

<ex>
INPUT <esc|%%|filename<esc|%%|
<\ex>

!!Or:!!

<ex>
INPUT &<var>filename&<\var>
<\ex>

produce:

<p indent>INPUT %%filename%%


<procedure> See Also

<list plain tight>
* <xref ex-ReferenceSection>
* <xref computer-ReferenceSection> includes an example of a variable
  within computer text.
* <xref user-ReferenceSection>
<\list>


<\rsect>








<rsect id=vex-ReferenceSection > &<vex>

		<idx|``&<vex>''|
		<idx|element tag: ``&<vex>''|
		<idx|verbatim example|
		<idx|example: displayed verbatim|
		<idx|unchanged: example displayed|
		<idx|``smaller'' parameter|
		<idx|``smallest'' parameter|
		<idx|parameter: ``smaller''|
		<idx|parameter: ``smallest''|


!!Verbatim example!!

Indicates a verbatim example in which &TagAppName; elements are not
interpreted as elements.


<procedure> Syntax

<ex>
&<vex [number | nonumber] [smaller | smallest] >
%%example text%%
&<\vex>
<\ex>


Where:

<lablist>

\``nonumber''\ (Default.)  Omits the adding of line numbers to the
beginning of each line.  

\ ``number''\ Puts a line number at the beginning of each line.

\``smaller'' or ``smallest'' \ Displays the example using smaller
fonts.  This makes long lines fit within a narrower width.

<\lablist>

Within a verbatim example, no &TagAppName; elements are recognized 
except ``&<&\'' which is assumed to be an end tag.

		<idx|line breaks, preserving|
		<idx|spacing, preserving|

Use this element when you need to use shorthand forms of
tags that would otherwise be interpreted as markup.  The effect is
similar to using ``&<esc>'' in text, with output similar to
``&<ex>''.  Line breaks and spacing are preserved as they appear
in the source file.

The ``smaller'' and ``smallest'' fonts enable wide examples to
fit within the margins.


<procedure> Example

The following markup:

<ex smaller>
&<vex smaller>
You can use shorthand form characters, such as <esc|%%
or !! or ++| without using the &<esc> element to allow
them to print.
&<\vex>

<\ex>

produces:

<vex smaller>
You can use shorthand form characters, such as %%
or !! or ++ without using the <esc> element to allow
them to print.
<\vex>


<procedure> See Also

<list plain tight>
* <xref esc-ReferenceSection>
* <xref ex-ReferenceSection>
* <xref image-ReferenceSection>
<\list>


<\rsect>








<rsect id=warning-ReferenceSection > &<warning>

		<idx|``&<warning>''|
		<idx|element tag: ``&<warning>''|
		<idx|danger, warning of|
		<idx|warning: of danger|


!!Warning!!

Calls the reader's attention to a situation that could be
dangerous to the user.


<procedure> Syntax

<ex>
&<warning>
%%text%%
&<\warning>
<\ex>


The text of the warning message is printed in boldface.

The default heading for the warning is "Warning".  To specify a
different heading, use the ``&<head>'' element.

To display a graphic with the warning,
define ``WarningElementDefaultIconFile'' in an
``&<!entity>'' declaration.

The ``&<\warning>'' end tag is required.


<procedure> Examples

The following markup creates a warning message:

<ex smaller>
&<warning>
Failure to follow these guidelines could result
in serious consequences.
&<\warning>
<\ex>


The following markup specifies a different heading for the warning
message:

<ex smaller>
&<warning>&<head>Danger!
Do not open the high voltage compartment.
&<\warning>
<\ex>


<procedure> See Also

<list plain tight>
* <xref note-ReferenceSection> 
* <xref caution-ReferenceSection> 
* <xref head-ReferenceSection> 
<\list>

<\rsect>








<rsect id=xref-ReferenceSection > &<xref>

		<idx|``&<xref>''|
		<idx|element tag: ``&<xref>''|
		<idx|cross-reference|
		<idx|cross-reference: ID value|
		<idx|ID: used in ``&<xref>''|

!!Cross-reference!!

Inserts text that identifies another location of the help volume and
creates a hyperlink to that location.


<procedure> Syntax

<ex>
&<xref %%id%%>
<\ex>

Where:

<lablist>
\%%id%%\ The identifier of the topic or location that is being
cross-referenced.
<\lablist>

Cross-references are translated into section titles or topic, figure,
list item or line numbers.  The cross-reference
text becomes a hyperlink that, when chosen by a user, jumps to the
cross-referenced location.

The ``id'' parameter must be defined in the element
to which ``&<xref>'' refers.  In both the ``xref''
parameter and the ``id'' parameter, the name must be spelled
exactly the same. Capitalization, however, is not significant.

The ``id'' parameter is important because it allows you to cross-reference
an element that you want to refer to.  For example, if you want to refer
readers to a chapter for more information, you can use the chapter's ``id'' to
create a reference.  To refer to a chapter titled "Window
Management" whose ``id'' is %%windowmgr%%, you would write, "Refer to
&<xref windowmgr> for details."  In your online help volume, the result is:
"Refer to Window Management for details." In addition, the chapter title,
"Window Management", is a hyperlink.


The ``id'' parameter can appear with:

<list plain tight>
* ``&<chapter>''
* ``&<s1>, &<s2>, ... &<s9>''
* ``&<rsect>''
* ``&<appendix>''
* ``&<figure>''
* ``&<item>''
<\list>

A cross-reference to an %%id%% that contains an underscore (such as
"_abstract" or "_hometopic") is not allowed. 


<need begin>

<procedure> Example

The ``id'' parameter is important because it allows you to cross-reference
an element that you wish to refer to.  Suppose an ID named "analyzer" were
defined in the following markup:

<ex smaller>
&<s1 id=analyzer>Logic Analyzers
<\ex>

Here is markup that contains a cross-reference to the above topic:

<ex smaller>
The DX16500A logic analysis system, described in
&<xref analyzer>, can be configured to a user's needs.
<\ex>

After translation by the ``dthelptag'' command, the ``&<xref>'' element
would be replaced by "Logic Analyzers" as shown here:

<ex smaller>
The DX16500A logic analysis system, described in
Logic Analyzers, can be configured to a user's needs.
<\ex>

The text "Logic Analyzers" would appear as a hyperlink that, when
chosen by a user, jumps to the cross-referenced help topic.

<need end>


<procedure> See Also

<list plain tight>
* <xref entity-ReferenceSection> 
* <xref link-ReferenceSection> 
* <xref location-ReferenceSection> 
* <xref term-ReferenceSection> 
* <xref figure-ReferenceSection> 
* <xref chapter-ReferenceSection> 
* <xref s1-ReferenceSection> 
* <xref otherhead-ReferenceSection> 
* <xref rsect-ReferenceSection> 
<\list>


<\rsect>

