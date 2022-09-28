<chapter id=organizing-and-writing-a-volume > Organizing and Writing \ a Help Volume

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
 * <xref IntroducingThePartsOfAHelpVolumeTO>
 * <xref GeneralMarkupGuidelinesSI>
 * <xref CompleteStandAloneExample>
 * <xref CreatingTopicHierarchySI>
 * <xref CreatingMetaInformationTopicsSI>
 * <xref AccessingTopicsSI>
 * <xref UsingEntitiesSI>
<\list>
<!-- IFDEF ONLINE ONLY END -->



&headerFix;

		<idx|organizing: help volume|
		<idx|writing: help volume|
		<idx|volume: organizing and writing help|


Within a help volume, information is organized into five major sections:
the home topic, topics and subtopics, entity declarations, meta information, 
and the glossary.

<procedure>Home Topic
		<idx|home topic|
		<idx|topic: home|

The ++home topic++ is the first in the topic hierarchy.  All other topics
are "subtopics."  Your topic hierarchy may be several levels deep.
However, to help prevent users from getting lost, you should keep your
hierarchy as shallow as possible.


<procedure>Topics and Subtopics

		<idx|subtopics|
		<idx|topics: subordinate|
		<idx|element tag: ``&<chapter>''|
		<idx|element tag: ``&<s1>'' .&empty;.&empty;. ``&<s9>''|
		<idx|``&<chapter>''|
		<idx|``&<s1>'' .&empty;.&empty;. ``&<s9>''|


Topics and subtopics form a hierarchy below the home topic.  The first
level of subtopics (following ``&<hometopic>'') may begin with
``&<chapter>'' or ``&<s1>''.
The ``&<chapter>'' level is the only level you can skip.  That is,
subtopics of an ``&<s1>'' must be entered with ``&<s2>'', subtopics of an
``&<s2>'' must be entered with ``&<s3>'', and so on.  There is no visible
difference to the user if you start your hierarchy with ``&<chapter>'' or
``&<s1>''.  The only difference is the number of run-time help files that
will be generated (one per chapter).


<procedure>Entities

		<idx|entities|
		<idx|entity: declaration|


An author-defined <term nogloss|entity| can represent a string of
characters or a file name.  An entity <term nogloss|declaration| defines
the entity name and the string or file it represents.  All entity
declarations must be entered before any other markup in your help volume.
Entity <term nogloss|references| can be used anywhere within your help
volume.  When you process your help volume with the HelpTag software
(``dthelptag'' command), each entity reference is replaced with the text or
file that the entity represents.


<procedure> Meta Information

		<idx|meta information|
		<idx|information, meta|


Meta information is !!information about your information!!.  It includes
information such as the volume's title, copyright notice, and abstract.
Meta information also includes general help topics that are
!!not!! part of the normal topic hierarchy.


<procedure>Glossary

		<idx|glossary|
		<idx|definition: in glossary|
		<idx|definition link: for term|
		<idx|element tag: ``&<term>''|


The glossary includes definitions for terms that you've used throughout your
help volume.  If a term is entered using the ``&<term>'' element, then it
automatically becomes a !!definition link!! that, when selected, displays
the glossary entry for that term.



<s1 id=IntroducingThePartsOfAHelpVolumeTO>A Help Volume at a Glance

		<idx|overview: help volume|
		<idx|help volume: overview|
		<idx|volume: overview|
		<idx|sample help volume markup|
		<idx|help volume: sample markup|
		<idx|markup: sample help volume|
		<idx|element tag: ``&<helpvolume>''|
		<idx|element tag: ``&<metainfo>''|
		<idx|element tag: ``&<title>''|
		<idx|element tag: ``&<abstract>''|
		<idx|element tag: ``&<hometopic>''|
		<idx|element tag: ``&<glossary>''|
		<idx|element tag: ``&<s1>'' .&empty;.&empty;. ``&<s9>''|
		<idx|``&<helpvolume>''|
		<idx|``&<metainfo>''|
		<idx|``&<title>''|
		<idx|``&<abstract>''|
		<idx|``&<hometopic>''|
		<idx|``&<glossary>''|
		<idx|``&<s1>'' .&empty;.&empty;. ``&<s9>''|


The following markup illustrates important elements of a help volume and
the tags used to enter them.  Indentation is used to highlight the
hierarchical relationship of the elements; you don't need to indent the
help files that you write.

<ex smaller>
%%All entity declarations go here (before any other markup).%%

&<helpvolume>

     &<metainfo>

          &<title> %%Volume Title%%

          &<copyright>
          %%Copyright topic goes here ...%%

          &<abstract>
          %%The abstract describing your help volume goes here.%%

             &vellipsis;  %%There may be other meta information topics.%%

     &<\metainfo>

     &<hometopic> %%Home Topic Title%%
                 %%Body of the home topic goes here ...%%

          &<s1> %%Title of First Subtopic Goes Here%%
               %%Body of the first subtopic goes here ...%%

          &<s1> %%Title of Second Subtopic%%
               %%Body of the second subtopic goes here ...%%

            &vellipsis;

     &<glossary>
     %%The body of the glossary, which contains term definitions, goes here ...%%

&<\helpvolume>

<\ex>

<!-- seealsos/HvParts.SA -->
<!-- IFDEF ONLINE ONLY -->

<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ChapterWritingHelpTopic>
<\list>

<!-- IFDEF ONLINE ONLY END -->

<!-- SEE ALSO -->

The rest of this chapter describes how to organize your help volume.
Writing individual topics is covered in the following chapter.




<s1 id=GeneralMarkupGuidelinesSI>General Markup Guidelines

		<idx|markup: guidelines|
		<idx|general markup guidelines|
		<idx|guidelines, markup|
		<idx|tag|
		<idx|element|


Online help is written in ordinary text files.  You use special codes, or
<term tag|tags|, to markup <term element|elements| within the information.
The tags form a markup language called &TagAppName;.


The &TagAppName; markup language defines a hierarchy of elements that define
high-level elements such as chapters, sections, and subsections, and
low-level elements such as paragraphs, lists, and emphasized words.


<procedure>Creating Your ``%%volume%%.htg'' File

		<idx|``.htg'' file|
		<idx|file: ``.htg''|


Online help is written in ordinary text files.  You process, or "compile,"
these files with the &TagAppName; software to create run-time help files that
can be read by the &ProductName;.

&TagAppName; expects a primary input file named ``%%volume%%.htg'', where
%%volume%% is a name you choose.  Be sure your %%volume%% name is unique
and meaningful.  If your %%volume%% name is too general, it may conflict
with another volume that someone else has created.

If you are writing application help, one recommended practice is to use the
application's class name.  For example, the class name for the CDE Icon
Editor is "Dticon," so its help volume was named ``Dticon.htg''.

The details of running &TagAppName; are covered in <xref ToRunHelpTagTA>.


<procedure>Multiple\ Source Files

		<idx|entities|
		<idx|source files, multiple|
		<idx|input files, multiple|
		<idx|multiple source files|


Although &TagAppName; expects a single ``%%volume%%.htg'' file as input, you can
separate your work into multiple source files.  Additional files are
sourced into the ``%%volume%%.htg'' file using <term nogloss|file
entities|.  A file entity is like a pointer to another file.  That file,
in effect, is inserted wherever the entity's name appears in the
``%%volume%%.htg'' file.  The referenced files can also contain entity
references to yet other files.  (Entities can also be used to reference
text strings.)



<procedure>Markup in Your\ Source Files

		<idx|end tag|
		<idx|start tag|
		<idx|element: marking|
		<idx|markup: language|
		<idx|backslash (``&\'')|
		<idx|``&\'' (backslash)<sort>&\|


The markup for most elements consists of a start tag and an end tag.
Start tags are entered with
the element name between angle brackets (``&<'' and ``>'').  End tags are
similar, but the element name is preceded by a backslash (``&\'').

<ex>&<%%element%%> %%... text ...%% &<\%%element%%><\ex>

<need begin>

For example, to mark the start and end of a book title you use markup like
this:

<ex>&<book>The OSF/Motif Style Guide&<\book><\ex>

Where ``&<book>'' is the "start tag," and ``&<\book>'' is the "end
tag."

<need end>

<procedure>Short Form\ Markup

		<idx|short form markup|
		<idx|markup: short form|


<term|Short form markup| provides another way to enter
the markup for many inline elements.  Rather than entering a begin and end
tag, vertical bars are used to delimit the text like this:

<ex>&<%%element%%| %%... text ...%% |<\ex>

For example, here's the short form of the ``&<book>'' element
shown above:

<ex>&<book|The OSF/Motif Style Guide|<\ex>

		<idx|parameters in element|
		<idx|element: parameters|

If the element has parameters, they're entered before the first vertical
bar like this:

<ex>&<%%element%% %%parameters%%| %%... text ...%% |<\ex>


<procedure>Shorthand\ Markup

		<idx|shorthand markup|
		<idx|markup: shorthand|


Some elements support an even shorter form where the begin and end tags are
replaced with a special two-character shortcut.  For example, the
``&<emph>'' (!!emphasis!!) element, whose normal syntax looks like this:

<ex>&<emph> %%... text ...%% &<\emph><\ex>

can be entered using this shorthand form:

<ex>!! %%... text ...%% !!<\ex>



<procedure>Displaying\ &TagAppName; Symbols

		<idx|ampersand (``&&'')|
		<idx|``&&'' (ampersand)<sort>&&|
		<idx|symbol: ampersand (``&'')|
		<idx|angle bracket (``&<'')|
		<idx|``&<'' (angle bracket)<sort>&<|
		<idx|symbol: left angle bracket (``&<'')|
		<idx|less than symbol (``&<'')|
		<idx|backslash (``&\'')|
		<idx|``&\'' (backslash)<sort>&\|
		<idx|symbol: backslash (``&\'')|


At times, you may need to use the left angle bracket (``&<''), the
backslash (``&\''), or the ampersand (``&&'') as text characters.  To do
so, precede each with an ampersand (``&&&<'', ``&&&\'', or ``&&&&'').




<s1 id=CompleteStandAloneExample> Writing Your First Help Volume: A Step-by-Step Example

		<idx|example: complete stand-alone help volume|
		<idx|sample stand-alone help volume|
		<idx|stand-alone help volume, sample|
		<idx|volume: sample stand-alone help volume|
		<idx|step-by-step instructions for creating help volume|
		<idx|instructions for creating full help volume|
		<idx|complete sample stand-alone help volume|
		<idx|full sample stand-alone help volume|


Here is a complete step-by-step example that demonstrates
how to create and view a stand-alone help volume.  (As a stand-alone
volume, it does not involve interaction with an application.)

The markup language used in the text files is explained in 
<xref organizing-and-writing-a-volume>,
<xref ChapterWritingHelpTopic>, and
<xref markup-summary>.

<list order>

* Create a directory where you will put most of your text files.

* Create one or more text files that explain the system
you are documenting.  For this simple example, all the
information is put into a single file named ``Commands''
in the directory just created.

<note><head>Tip!
If you want to try this example, copy and paste the example text directly
from online help into your text editor, instead of typing the text.
<\note>

Here is what the ``Commands'' file contains.  The element
tags within the angle brackets (``<'' and ``>'')
indicate the structure of the information.

<ex smaller>
&<hometopic> Command Summary
                        &<idx|commands|

Your &&product; is capable of the following operations:

&<list bullet>
 * &<xref ChannelChange>
 * &<xref VolumeUp>
 * &<xref VolumeDown>
 * &<xref VolumeMute>
&<\list>

Choose one of the hyperlinks (underlined phrases)
to find out how to perform that operation.

&<s1 id=ChannelChange> Changing the Channel
                        &<idx|channel, changing|

Speak the command:

&<ex>channel&<\ex>

followed by a number from ``one'' to ``ninety nine''.

&<s1 id=VolumeUp> Turning Up the Volume
                        &<idx|volume, changing|

Speak the command:

&<ex>volume up&<\ex>

For additional volume, speak the command:

&<ex>more&<\ex>

(See also &<xref VolumeDown>)

&<s1 id=VolumeDown>Turning Down the Volume
                        &<idx|volume, changing|

Speak the command:

&<ex>volume down&<\ex>

To further reduce the volume, speak the command:

&<ex>more&<\ex>

(See also &<xref VolumeUp> and &<xref VolumeMute>)


&<s1 id=VolumeMute>Turning Off the Sound
                        &<idx|volume, changing|
                        &<idx|sound, on/off|

Speak the command:

&<ex>sound off&<\ex>

To restore the sound, speak the command:

&<ex>sound on&<\ex>

(See also &<xref VolumeDown> and &<xref VolumeUp>)

<\ex>

* Create a text file that gives the information a title,
provides copyright information, and provides other information
about the online help.  In this example, the following text
is put into a file called ``Metainfo'' in the same directory
as the ``Commands'' file.

<ex smaller>
&<metainfo>

     &<title>Using the &&product;

     &<copyright>
     &&copy; 1997 Voice Activation Company.  All rights reserved.

     &<abstract>Help for Using the &&product;.

&<\metainfo>
<\ex>


<\list>

<procedure>Create a "build" directory

<list order continue>

* Create a new subdirectory named ``build/'' that is below the
directory that contains the ``Commands'' and ``Metainfo'' files.

<\list>

<procedure>Create the master &TagAppName; file

<list order continue>

* In the ``build/'' subdirectory, create a text file whose name is
of the form ``%%volume%%.htg''.  In this example, the file is
named ``voiceact.htg''.

* In the ``.htg'' file, define <term entity>entities<\term> that
associate the names of the ``Commands'' and ``Metainfo'' files
with entity names.  Also, define any entities that are used
(either directly or indirectly) in the ``Commands'' and ``Metainfo''
files.  Finally, refer to the ``Commands'' and ``Metainfo'' files by
their entity names.

In this example, the contents of the ``voiceact.htg'' file
look like this.  The text within the ``&<!--'' ... ``-->''
elements are comments, which are ignored.

<ex smaller>

&<!-- Declare an entity for each of the source text files. -->

&<!entity MetaInformation     FILE  "MetaInfo">
&<!entity Commands            FILE  "Commands">


&<!-- Define an entity that names the product and includes
     the trademark symbol (&&tm;). -->

&<!entity product "VoAc&&tm; Voice-Activated Remote Control">


&<!-- Include the text files. -->

&&MetaInformation;
&&Commands;


<\ex>


<\list>

<procedure>Prepare to run &TagAppName;

<list order continue>

* In the ``build/'' subdirectory, create a file named ``dthelptag.opt''
and put the following text into it.  This information selects
&TagAppName; options and indicates where to search for any files defined
in FILE entity declarations.

<ex smaller>
memo
onerror=go
search=./
search=../
<\ex>


* Add the ``/opt/dt/bin/'' directory to your PATH environment variable.
If you're not sure how to do this, consult the online help or documentation
for your operating system or see your system administrator.


<\list>

<procedure>Create the run-time help files

<list order continue>

* From the ``build/'' subdirectory, execute the following command:

<ex smaller>
``dthelptag -verbose voiceact''
<\ex>


This command executes the &TagAppName; software to create a run-time
version of your online help volume.  The ``-verbose'' option tells
&TagAppName; to display, on your screen, its progress.


* If &TagAppName; reports that errors occurred, fix them by editing or
renaming the text files as needed.  (If the errors are parser errors,
they are listed in a file named ``voiceact.err''.)


<\list>

<procedure>Display the help volume

<list order continue>

* From the ``build/'' subdirectory, execute the following command.
It displays the new help volume:

<ex smaller>
``dthelpview -h voiceact &&''
<\ex>

You can now scroll the information
and jump to related information by choosing hyperlinks.

<\list>




<s1 id=CreatingTopicHierarchySI>Creating a Topic Hierarchy

<!-- IFDEF ONLINE ONLY -->
<!-- &CreatingTopicHierarchySISubtopics; -->
<list bullet tight>
* <xref ToCreateHomeTopicTA>
* <xref ToAddTopicToHierarchyTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->


		<idx|creating: topic hierarchy|
		<idx|topic: creating hierarchy|
		<idx|hierarchy: topic, creating|
		<idx|element tag: ``&<chapter>''|
		<idx|element tag: ``&<s1>'' .&empty;.&empty;. ``&<s9>''|


The topic hierarchy within your help volume begins with the ++home topic++.
Each help volume must have one home topic.  The first level of subtopics
below the home topic may be entered with ``&<chapter>'' or ``&<s1>''.

Additional levels of subtopics are entered with ``&<s2>'', ``&<s3>'', and
so on.  The &TagAppName; markup language supports hierarchies down to ``&<s9>''.
However, information more than three or four levels deep often leads many
readers to feel lost.

Within each topic that has subtopics, you should provide a path for the
reader to get to the subtopics.  This requires assigning unique IDs to all
topics and creating hyperlinks within the body of each topic to its
subtopics.


<procedure>Example

Suppose you want to create a hierarchy to match this simple outline:

<ex smaller>
%%Tutorial for New Users%%
     %%Module 1:  Getting Started%%
     %%Module 2:  Creating Your First Report%%
     %%Module 3:  Printing the Report%%
     %%Module 4:  Saving Your Work and Quitting%%
%%Task Reference%%
     %%Starting and Stopping%%
          %%To start the program%%
          %%To quit the program%%
     %%Creating Reports%%
          %%To create a detailed report%%
          %%To create a summary report%%
%%Concepts for Advanced Users%%
     %%How Report Hot Links Work%%
     %%Sharing Reports within a Workgroup%%
%%Reference%%
     %%Command Summary%%
     %%Report Attributes Summary%%
<\ex>

<need begin>
Then the general outline of your help volume would look like this. (The
body of each topic and IDs for the topics are not shown.)

<ex smaller>
&<hometopic> %%Welcome to Report Master%%

  &<chapter> %%Tutorial for New Users%%
    &<s1> %%Module 1:  Getting Started%%
    &<s1> %%Module 2:  Creating Your First Report%%
    &<s1> %%Module 3:  Printing the Report%%
    &<s1> %%Module 4:  Saving Your Work and Quitting%%
  
  &<chapter> %%Task Reference%%
    &<s1> %%Starting and Stopping%%
      &<s2> %%To start the program%%
      &<s2> %%To quit the program%%
    &<s1> %%Creating Reports%%
      &<s2> %%To create a detailed report%%
      &<s2> %%To create a summary report%%
  
  &<chapter> %%Concepts for Advanced Users%%
    &<s1> %%How Report Hot Links Work%%
    &<s1> %%Sharing Reports within a Workgroup%%
  
  &<chapter> %%Reference%%
    &<s1> %%Command Summary%%
    &<s1> %%Report Attributes Summary%%
<\ex>
<need end>


		<idx|run-time files: multiple|
		<idx|output files|
		<idx|files, output|

You could have created an identical hierarchy by starting with ``&<s1>''s in
place of the ``&<chapter>'' tags, ``&<s2>''s for the next level, and
``&<s3>''s for the third level.  The only difference -- not seen by the
reader -- is the number of files created by &TagAppName; when you process the
help volume.

Again, indentation is used here to make it easier to see the structure of
the help volume.  You do not have to indent your files.

<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hovtohie.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref AccessingTopicsSI> describes assigning IDs to topics.
 * <xref CreatingHyperLinksSI> describes how to create hyperlinks.
<\list>
<!-- IFDEF ONLINE ONLY END -->

<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref AccessingTopicsSI> in this chapter describes assigning IDs to topics.
 * <xref CreatingHyperLinksSI> describes how to create hyperlinks.
<\list>


<s2 id=ToCreateHomeTopicTA > To create a home topic

<need begin>

		<idx|creating: home topic|
		<idx|home topic: creating|
		<idx|topic: home, creating|
		<idx|element tag: ``&<hometopic>''|
		<idx|``&<hometopic>''|


<list bullet>
 * Use the ``&<hometopic>'' element as follows:

<ex>
&<hometopic>%%Title%%
%%Body of topic.%%
<\ex>
<\list>

If you include a meta information section (``&<metainfo>''), the home topic
must follow it.

<need end>


<procedure>Examples

Here's a home topic with a title and a single sentence as its body:

<ex smaller>
&<hometopic>Welcome to My Application

Congratulations, you've entered 
the online help for My Application.
<\ex>

Here's a sample home topic that includes hyperlinks to its four subtopics:

<ex smaller>
&<hometopic>Welcome to Report Master

Welcome to the online help for Report Master.
Choose one of the following hyperlinks:

&<list bullet>
 &empty;* &<xref Tutorial>
 &empty;* &<xref Tasks>
 &empty;* &<xref Concepts>
 &empty;* &<xref Reference>
&<\list>

If you need help, press F1.
<\ex>

To help users who may be new to hyperlinks, you may want to include a
reminder to use F1 to get help on help.

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateMetaInformationSectionTA>
 * <xref ToAddTopicToHierarchyTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->

<!-- seealsos/Hovhomt.SA -->



<s2 id=ToAddTopicToHierarchyTA>To add a topic to the hierarchy

<need begin>

		<idx|adding: topic to hierarchy|
		<idx|topic: adding to hierarchy|
		<idx|hierarchy: adding topic to|
		<idx|subtopics: creating|
		<idx|element tag: ``&<s1>'' .&empty;.&empty;. ``&<s9>''|
		<idx|``&<s1>'' .&empty;.&empty;. ``&<s9>''|


<list bullet>
 * To add another topic at the same level, repeat the same element.

 * !!Or,!! to add a subtopic (a topic one level deeper in the hierarchy),
   use the element that is one level deeper than the preceding topic.
<\list>

<need end>

<need begin>
<procedure>Example

If the current topic is an ``&<s1>'', enter a subtopic using ``&<s2>''.

<ex smaller>
&<s1 id=getting-started> Getting Started

%%The body of this getting started topic should%%
%%include a hyperlink to each of the subtopics.%%

&<s2 id=starting-the-program> Starting the Program

%%Here's the body of the first subtopic.%%

&<s2 id=stopping-the-program> Stopping the Program

%%Here's the body of the second subtopic.%%
<\ex>
<need end>

<need begin>
The second ``&<s2>'' is also a subtopic of the ``&<s1>''.

<note><head> The Parent-Child Metaphor

Sometimes a parent-child-sibling metaphor is used to describe
the relationships between topics in a hierarchy.  In the
above example, the ``&<s1>'' topic is the "parent" of both
``&<s2>''s (the "children" topics).  The two ``&<s2>''s
are "siblings" of one another.  All three topics are "descendents"
of the home topic.

<\note>

<need end>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref CreatingMetaInformationTopicsSI> describes how to create topics
   that are !!not!! part of the topic hierarchy.
<\list>
<!-- IFDEF ONLINE ONLY END -->

<!-- seealsos/Hovato.SA -->



<s1 id=CreatingMetaInformationTopicsSI > Creating Meta Information Topics

<!-- IFDEF ONLINE ONLY -->
<!-- &CreatingMetaInformationTopicsSISubtopics; -->
<list bullet tight>
* <xref ToCreateMetaInformationSectionTA>
* <xref ToAddNonHierarchicalTopic>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|creating: meta information topic|
		<idx|meta information: topic, creating|
		<idx|information, meta: creating topic|
		<idx|topic: creating meta information topic|


The meta information section is primarily intended for !!information about
information!!. Similar to providing a "Notice" page in a book, this section
includes information such as the volume title, copyright, trademark, and other
notices.

A secondary use of the meta information section is to enter general help
topics that are not part of the normal topic hierarchy.  These topics are
useful for creating custom definition links that pop-up a topic in a quick
help dialog.



<s2 id=ToCreateMetaInformationSectionTA > To create a meta information section

		<idx|creating: meta information topic|
		<idx|meta information: topic, creating|
		<idx|element tag: ``&<metainfo>''|
		<idx|``&<metainfo>''|
		<idx|element tag: ``&<title>''|
		<idx|``&<title>''|
		<idx|element tag: ``&<copyright>''|
		<idx|``&<copyright>''|
		<idx|copyright: notice|
		<idx|element tag: ``&<legal>''|
		<idx|``&<legal>''|
		<idx|element tag: ``&<abstract>''|
		<idx|``&<abstract>''|


<list order>

 * Enter the ``&<metainfo>'' tag to start the section, and enter the required
   subelements ``&<title>'' and ``&<copyright>'' as shown:

<ex>
&<metainfo>

&<title>%%Volume Title Here%%

&<copyright>
%%Body of copyright topic here.%%

    &vellipsis;

<\ex>

 * Enter any of the optional elements as shown:

<ex>
&<abstract>
%%Body of the abstract topic here.%%
%%Do not use any &TagAppName; markup within the abstract!%%
<\ex>

<need begin>

 * Enter the ``&<\metainfo>'' end tag to end the section.

<ex>

    &vellipsis;
&<\metainfo>
<\ex>
<\list>

<need end>

<note><head>Notes
<list bullet >

 * Some elements in the meta information section require a ``&<head>'' tag
   before the topic heading.  

 * The ``&<abstract>'' section is recommended.	Applications that access
   help volumes may use the abstract to present a brief description of the
   volume.  Because the abstract might be displayed in plain text windows
   (that are not capable of multi-font or graphics formatting), you should
   avoid including any &TagAppName; markup in the abstract.

<\list>
<\note>


<procedure>Example

Here's a typical meta information section:
<ex smaller>
&<metainfo>

  &<title> Report Master, Version 1.0

  &<copyright>
    &<otherhead>Report Master

    &<image>
    Version 1.0
    &&copy; Copyright Reports Incorporated 1993
    All rights reserved.
    &<\image>

  &<abstract>
    This is the online help for the mythical Report Master
    application.  This help includes a self-guided tutorial,
    a summary of common tasks, general concepts, and quick
    reference summaries.

&<\metainfo>
<\ex>

<need begin>

The ``&<image>'' element is used to preserve the author's line breaks.
The ``&&copy;'' entity inserts the copyright symbol.

<!-- seealsos/Hovmetas.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ToLinkToAMetaInfoTopicTA>
<\list>

<need end>



<s2 id=ToAddNonHierarchicalTopic > To add a non-hierarchical topic

		<idx|adding: non-hierarchical topic|
		<idx|non-hierarchical topic, adding|
		<idx|hierarchical, adding non-hierarchical topic|
		<idx|topic: adding non-hierarchical|
		<idx|element tag: ``&<otherfront>''|
		<idx|``&<otherfront>''|


<list bullet>

 * Add the topic just before the end of your meta information section
   using the ``&<otherfront>'' element as follows:

<ex>
&<otherfront id=%%id%%>&<head> %%Topic Title%%
%%Body of topic.%%
<\ex>
<\list>

The ID parameter and ``&<head>'' tag are required.

You may add as many ``&<otherfront>'' topics as you want.  They may be in
any order, but they must be the last topics in the ``&<metainfo>'' ...
``&<\metainfo>'' section.


<procedure>Example

This partial help volume shows how a general topic is added to the meta
information section.  The topic's title is "Popup!" and its ID is
``my-popup-topic''. 

<ex smaller>
&<metainfo>
  &<title>My Help
  &<copyright>
     This is My Help, Version 1.0.  &&copy; 1993.

        &vellipsis;

  &<otherfront id=my-popup-topic>&<head>Popup!

  This is a popup topic, displayed via a definition link
  somewhere in my help volume.

&<\metainfo>

&<hometopic> Welcome to My Help

    &vellipsis;
<\ex>

<need begin>

Presumably, within some other topic in the help volume, there's a definition
link to display this topic.  The link might look like this:

<ex smaller>
Here's a sample of a pop-up 
&<link my-popup-topic Definition>definition link&<\link>.
<\ex>

The words "definition link" become the active hyperlink and will be
displayed with a dashed underline.  Selecting the link displays the "Popup!"
topic in a quick help dialog.

<!-- seealsos/Hovnohie.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref CreatingHyperlinksSI>
<\list>

<need end>



<s1 id=AccessingTopicsSI > Accessing Topics

<!-- IFDEF ONLINE ONLY -->
<!-- &AccessingTopicsSISubtopics; -->
<list bullet tight>
* <xref ToAddAnIdToTopic>
* <xref ToAddAnIdToAnElementWithinTopic>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|accessing: topic|
		<idx|topic: accessing|
		<idx|ID: naming rules|


Many elements in the &TagAppName; language support an ID attribute.  An ID is a
unique name used internally to identify topics and elements within
topics.  An ID is defined only once, but multiple hyperlinks and
cross-references can refer to the same ID.  IDs are not seen by the user.

If you are writing help for an application, IDs are also used by the
application to identify particular topics to display when the user requests
help.

<note><head>Rules for ID Names

<list bullet>

 * ID strings may contain letters (A - Z and a - z), digits (0 - 9),
   plus (+), and minus (&minus;), and must begin with a letter.

 * Author-defined IDs may !!not!! use the underscore character (_); it is 
   reserved for IDs that are built into some &TagAppName; elements.

 * Case is !!not!! significant, but is often used to increase readability.

 * ID strings cannot be longer than 64 characters.

 * Each ID within a single help volume must be unique.

<\list>

<\note>




<s2 id=ToAddAnIdToTopic > To add an ID to a topic

<need begin>

		<idx|adding: ID to topic|
		<idx|ID: adding to topic|
		<idx|topic: adding ID to|
		<idx|``id'' parameter: in element|
		<idx|ID: predefined names|


<list bullet>

 * Use the ``id'' parameter for the element as follows:

<ex>
&<%%element%% id=%%id%%> ...
<\ex>
<\list>

The elements that start a new topic and support an author-defined ID are:

<ex smaller>
&<chapter id=%%id%%>
&<otherfront id=%%id%%>
&<rsect id=%%id%%>
&<s1 id=%%id%%>
&<s2 id=%%id%%>
    &vellipsis;
&<s9 id=%%id%%>
<\ex>

<procedure> Built-in IDs

A few elements have built-in IDs and therefore do not support
an author-defined ID.  Each of the following elements also starts
a new topic, but these elements have predefined IDs
(shown in parentheses):

		<idx|``_abstract'' ID|
		<idx|``_copyright'' ID|
		<idx|``_glossary'' ID|
		<idx|``_hometopic'' ID|
		<idx|``_title'' ID|
		<idx|predefined ID: ``_abstract''|
		<idx|predefined ID: ``_copyright''|
		<idx|predefined ID: ``_glossary''|
		<idx|predefined ID: ``_hometopic''|
		<idx|predefined ID: ``_title''|

<ex smaller>
&<abstract>   (_abstract)
&<copyright>  (_copyright)
&<glossary>   (_glossary)
&<hometopic>  (_hometopic)
&<title>      (_title)
<\ex>

<need end>

<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hovadid.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref abstract-ReferenceSection>
 * <xref chapter-ReferenceSection>
 * <xref copyright-ReferenceSection>
 * <xref glossary-ReferenceSection>
 * <xref hometopic-ReferenceSection>
 * <xref otherfront-ReferenceSection>
 * <xref rsect-ReferenceSection>
 * <xref s1-ReferenceSection>
 * <xref title-ReferenceSection>
<\list>
<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToAddAnIdToAnElementWithinTopic>To add an ID to an element within a topic

<need begin>

		<idx|adding: ID to element within topic|
		<idx|ID: adding to element within topic|
		<idx|element: within topic, adding ID to|
		<idx|topic: adding ID to element within topic|
		<idx|element tag: ``&<location>''|
		<idx|``&<location>''|


<list bullet>

 * If the element supports an author-defined ID, use the ``id'' parameter
   for the element as follows:

<ex>
&<%%element%% id=%%id%%> ...
<\ex>

   The elements (within a topic) that support an ID attribute are:

<ex smaller>
&<figure id=%%id%%>
&<graphic id=%%id%%>
&<location id=%%id%%>
&<p id=%%id%%>
<\ex>

 * !!Or,!! use the ``&<location>'' element to set an ID at an arbitrary
   point within the topic as follows:

<ex>
&<location id=%%id%%> %%text%% &<\location>
<\ex>

   Where %%text%% is any word or phrase where you want to add an ID.
   The ``&<\location>'' end tag is required.

<\list>

<need end>



<need begin>

<procedure>Examples

Here's a figure with the ID ``my-big-picture''.  If you add an ID
to a figure, you must have a caption.  The caption is needed in case
an ``&<xref>'' uses the figure's ID, in which case the caption is
inserted in place of the ``&<xref>'' and becomes a hyperlink to the
figure.

<ex smaller>
&<figure id=my-big-picture entity=big-picture-TIFF>
Here's My Figure
&<\figure>
<\ex>

Here's a paragraph where the phrase "easier than ever" has been assigned
the ID ``easy-spot'':

<ex smaller>
Getting help is &<location id=easy-spot>easier 
than ever&<\location> -- just press F1.
<\ex>

<!-- seealsos/Hovid.SA -->
<!-- SEE ALSO -->

<need end>
<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref figure-ReferenceSection>
 * <xref graphic-ReferenceSection>
 * <xref location-ReferenceSection>
 * <xref p-ReferenceSection>
<\list>

<need end>
<!-- IFDEF ONLINE ONLY END -->



<s1 id=UsingEntitiesSI > Using Entities

<!-- IFDEF ONLINE ONLY -->
<!-- &UsingEntitiesSISubtopics; -->
<list bullet tight>
* <xref ToCreateTextEntity>
* <xref ToCreateFileEntity>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|using: entities|
		<idx|entities: using|
		<idx|string of text: multiple occurrences of same|
		<idx|multiple occurrences of same string of text|
		<idx|external file, referencing|
		<idx|referencing external file|
		<idx|inserting contents of another file|
		<idx|file: inserting contents of|
		<idx|``&<!entity>''<sort>&<!|
		<idx|element tag: ``&<!entity>''|
		<idx|``&<figure>''|
		<idx|element tag: ``&<figure>''|
		<idx|``&<graphic>''|
		<idx|element tag: ``&<graphic>''|

An ++entity++ can represent a string of characters or the contents of a
file.  An ++entity declaration++ defines the entity by associating the
entity name with a specific character string or file name.  An ++entity
reference++ is replaced by the string or file contents when you process the
help volume with the ``dthelptag'' command.

Entities are useful for:

<list bullet>

 * !!Referencing a common string of text.!!  This is useful if there is
   some likelihood that the text may change or you simply don't want to
   type it repeatedly.	Each place you want the text inserted, you
   reference the entity name.

 * !!Referencing an external file.!!  Entities are required for accessing
   graphics files.  The ``&<figure>'' and ``&<graphic>'' elements have a
   required parameter that you use to specify an entity name, which refers
   to a graphic image file.

   File entities are also useful for splitting your &TagAppName; source into
   multiple files.  Use entity references to include other files into your
   master &TagAppName; file for processing.

<\list>

		<idx|entity: declaration rules|

<note><head>Rules for Entity Declarations

<list bullet>

 * All entity declarations must come before any other markup in your help volume.

 * Entity names may contain letters (A - Z and a - z), digits (0 - 9), plus
   (+), and minus (&minus;), and must begin with a letter.

 * Case is !!not!! significant in entity names, but is
   often used to increase readability.

 * Entity names cannot be longer than 64 characters.

 * Each entity name must be unique within a single volume.

<\list>

<\note>



<s2 id=ToCreateTextEntity > To create a text entity

		<idx|creating: text entity|
		<idx|text: entity, creating|
		<idx|entity: creating text|
		<idx|``&<!entity>''<sort>&<!|
		<idx|``&<!entity>''<sort>&<!|
		<idx|element tag: ``&<!entity>''|
		<idx|ampersand (``&&'')|
		<idx|``&&'' (ampersand)<sort>&&|
		<idx|semicolon (``;'')|
		<idx|``;'' (semicolon)<sort>;|


<list order>

 * Declare an entity as follows:

<ex>
&<!entity %%EntityName%% "%%text%%">
<\ex>

   Where %%EntityName%% is the name of the entity and %%text%% is the string
   that you want substituted for every reference to the entity.  Remember, all
   entity declarations must come before any other markup in your help volume.

 * For each location where you want the %%text%% string to be inserted, 
   enter the entity reference as follows:

<ex>
&&%%EntityName%%;
<\ex>

   The "``&&''" and "``;''" characters are required for the &TagAppName;
   software to properly recognize the entity reference.

<\list>


<procedure>Example

The following line declares a text entity named "ProductName" that contains
the string "Common Desktop Environment":

<ex smaller>
&<!entity ProductName "Common Desktop Environment">
<\ex>

The following sentence includes a reference to the entity:

<ex smaller>
Welcome to the &&ProductName;!
<\ex>

When the help volume is processed with the &TagAppName; software, the entity
reference is replaced with the value of the entity.  So, the sentence
reads:

<ex smaller>
Welcome to the Common Desktop Environment!
<\ex>



<s2 id=ToCreateFileEntity > To create a file entity

		<idx|creating: file entity|
		<idx|file: entity, creating|
		<idx|entity: creating file entity|
		<idx|``&<!entity>''<sort>&<!|
		<idx|``&<!entity>''<sort>&<!|
		<idx|element tag: ``&<!entity>''|
		<idx|``FILE'' parameter|
		<idx|parameter: ``FILE''|


<list order>

 * Declare an entity as follows:

<ex>
&<!entity %%EntityName%% FILE "%%filename%%">
<\ex>

Where %%EntityName%% is the name of the entity and %%filename%% is the name
of the file.  The keyword ``FILE'' is required.

 * Reference the entity as follows:

 <list bullet loose>
 * If the file is a text file, enter the following entity reference 
   at each location where you want the contents of the file inserted.

<ex>
&&%%EntityName%%;
<\ex>

   The ampersand (``&&'') and semicolon (``;'') characters are required
   for the &TagAppName; software to properly recognize the entity reference.

 * If the file is a graphics file, include the name of the entity
   as a parameter in one of the following markup lines:

<ex>
&<figure entity=%%EntityName%% ... >
<\ex>

!!Or:!!

<ex>
&<graphic entity=%%EntityName%% ... >
<\ex>

!!Or:!!

<ex>
&<p gentity=%%EntityName%% ... >
<\ex>

 <\list>

<\list>

!!Do not!! include any path in the file name.  If the file is not in the
current directory when you run the &TagAppName; software, add the appropriate
search path to the ``dthelptag.opt'' file.  (See <xref ToRunHelpTagTA>.)

<need begin>

<procedure>Example: \ Text File Entities

Suppose you wrote the text for your help volume in three files named
``file1'', ``file2'', and ``file3'', plus a fourth file containing your
``&<metainfo>'' ... ``&</metainfo>'' section.  You could include them in
your ``%%volume%%.htg'' file like this:

<ex smaller>
&<!entity MetaInformation   FILE "metainfo">
&<!entity MyFirstFile       FILE "file1">
&<!entity MySecondFile      FILE "file2">
&<!entity MyThirdFile       FILE "file3">

&&MetaInformation;

&<hometopic> My Home Title

Welcome to my application's help volume.

&&MyFirstFile;
&&MySecondFile;
&&MyThirdFile;
<\ex>

<need end>


<procedure>Example: \ A Graphic File Entity

Suppose a simple help volume has a figure in the home topic and the graphic
image for the figure is stored in a file named ``picture.tif''.  The
following example shows how that image would be used in a figure.


<ex smaller>
&<!entity MetaInformation  FILE "metainfo">
&<!entity MyPicture        FILE "picture.tif">

&&MetaInformation;

&<hometopic>A Sample Graphic

Welcome to my application's help volume.

&<figure entity=MyPicture>
A Picture
&<\figure>
<\ex>

The text "A Picture" is the figure's caption.

<!-- seealsos/Hovcfent.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref DisplayingGraphicsSI>
<\list>













