<chapter id=ChapterIntroducingHelpSystem > Introducing the \ &ProductName;

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
 * <xref OverviewOfOnlineHelp>
 * <xref HpHelpInformationModel>
 * <xref AuthorsJob>
 * <xref ProgrammersJob>
<\list>
<!-- IFDEF ONLINE ONLY END -->


&headerFix;

		<idx|introduction to &ProductName;|
		<idx|&ProductName;, introduction to|


The <term nogloss|&ProductName;| is a complete system for developing
online help for application software.  It allows authors to write online
help that includes rich graphics and text formatting, hyperlinks, and
communication with the application.  &HelpSystemName; provides a programmer's
toolkit for integrating the help facilities into an application.

More specifically, here's what the CDE Help System Developer's Kit includes:

<procedure>For Authors

<list bullet>

 * !!The &TagAppName; markup language!! -- a set of "tags" used in text files
   to mark organization and content of your online help.
 
 * !!The &TagAppName; software!! -- a set of software tools for converting
   the &TagAppName; files you write into run-time help files.

 * !!The &ViewerAppName; application!! -- a viewer program for displaying your
   online help so you can read it and interact with it just as your audience
   will.

<\list>

Refer to <xref AuthorsJob> to learn more about writing online help.

<procedure>For Programmers

<list bullet>

 * !!The DtHelp programming library!! -- an Application Programmer's Interface
(API) for integrating help windows into your application.

 * !!A sample program!! -- a simple example that shows how to
integrate the &ProductName; into an OSF/Motif application.

<\list>

Refer to <xref ProgrammerSJob> to learn more about
programming with the &ProductName;.

<newpage>

<!-- in a future revision, please include a graphic that shows "help as
part of the application" -->



<!-- SEE ALSO -->

<procedure>This Manual Online

This manual is available online.

<list bullet>

 * Use the &HelpMgrName; to open the "&ProductName;" family, then
   choose the "&BookName;" title.

 * !!Or,!! if you are not using the &DesktopName; execute the following
   command in a terminal window:

   <ex smaller>/opt/dt/bin/dthelpview -helpVolume online && <\ex>

   The name of the help volume for the &BookName; is ``online''.


<\list>

<!-- Copy/Paste not ready for Developer's Conf. per eng.review

<note><head>Tip!

The online version of this manual offers the time-saving advantage of
making the examples accessible electronically.  You can copy and paste them
into your &TagAppName; files and then edit them to fit your needs.
<\note>
-->

<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/online/Hinintr.SA -->
<!-- SEE ALSO ONLINE -->

<!-- per engineer review, copy/paste not for Developer's Conference

<procedure>Copying Examples


<note><head>Tip!

The examples in this guide can be copied electronically by highlighting the
text and pasting it into your &TagAppName; files.  Then you can edit the
examples to fit your needs.

<\note>
-->

<!-- IFDEF ONLINE ONLY END -->



<s1 id=OverviewOfOnlineHelp > Overview of Online Help

<!--BEGIN ONLINE -->
<!-- &OverviewOfOnlineHelpSubtopics; -->

<list bullet tight>
* <xref ObjectivesForOnlineHelp>
* <xref HowUsersGetHelp>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|overview: online help|
		<idx|online help: overview|

It's virtually impossible -- and certainly impractical -- for anyone to
learn and remember !!everything!!
there is to know about the computer hardware and software they use to 
do their job.  Nearly every computer user needs help at one time or
another.

Online help, unlike a printed manual, has the power of the computer at its
disposal.  Most importantly, this power makes it possible to adapt the
information to the user's current "context."  !!Context-sensitive!! help
provides just enough help to get the user back on task.  Too much help can
often be too much of an interruption or "context switch."
In developing your online help, it's important to keep in mind the
objectives for providing online help.




<s2 id=ObjectivesForOnlineHelp > Objectives for Online Help

		<idx|objectives for online help|
		<idx|goals for online help|
		<idx|online help: objectives|

The two most important objectives for designing quality online help are:

<list bullet>

 * <term nogloss|First:|  !!Get the user back on task as quickly and
			  successfully as possible.!!

 * <term nogloss|Second:| !!Educate the user to prevent future need for
			  assistance.!!

<\list>

<need begin>



<s2 id=HowUsersGetHelp > How Users Get Help

		<idx|how users get help|
		<idx|getting help|
		<idx|user: how get help|
		<idx|help, how users get|


Online help can be divided into three general categories:

<list bullet>

 * ++Automatic Help++ -- The application determines when help is needed and
   what to present.  This is sometimes called "system initiated" help.

 * ++Semi-Automatic Help++ -- The user decides when help is needed, but the
   system determines what to present.  Semi-automatic help is
   initiated by a user's gesture or request for help, such as pressing
   F1.  The system's response is called "context-sensitive" help because it
   considers the user's current context in deciding what information to
   display.

 * ++Manual Help++ -- The user requests specific information, such as from
   a Help menu.

<\list>

<need end>


<procedure>The Help Key

Within most applications, the primary way for a user to request help is by
pressing the ++help key++.  In recent years, the F1 function key has become
a defacto standard "help key" for many workstation and personal computer
products.

The <book|OSF/Motif Style Guide| recommends the use of F1 as the
help key, and the OSF/Motif programmer's toolkit even provides
some built-in behavior to make it easier to
implement the help key in OSF/Motif applications.

Some computers have a Help key on the keyboard.

<procedure>The Help Menu

The Help menu is a common way to provide access to help information.
OSF/Motif applications provide a Help menu, which is right-justified
in the menu bar.  The <book|OSF/Motif Style Guide| makes
recommendations regarding the commands contained in a Help menu.


<procedure>Help Buttons

Many dialogs also provide a Help button to get help on the dialog.
The <book|OSF/Motif Style Guide| recommends that choosing the Help button
in a dialog be equivalent to pressing the help key while using that dialog.
Exceptions should be made for complex dialogs, where help on individual
controls within the dialog is appropriate.



<s1 id=HpHelpInformationModel > The &HelpSystemName; Information Model

<!-- IFDEF ONLINE ONLY -->
<!-- &HpHelpInformationModelSubtopics; -->
<list bullet tight>
* <xref VolumeIsCollectionOfTopics>
* <xref FamilyIsGroupOfRelatedHelpVolumes>
<\list>
<!-- IFDEF ONLINE ONLY END -->


		<idx|information model, help|
		<idx|model, help information|

There are two general styles of online help:

<list bullet>

 * ++Application help++, whose primary role is to be an integrated part of
    an OSF/Motif application.

 * ++Stand-alone help++, whose primary role is to provide online access to
   task, reference, or tutorial information, independent of any application
   software.

<\list>

If you are developing online help for an application, you may choose to
organize the information exclusively for access within the application.
Or, you may design the information such that it can be browsed without the
application present, as in stand-alone help.

<procedure>Part of the Application

&HelpSystemName; promotes a high degree of integration between the application and
its online help.  From the user's perspective, the help is part of the
application.  This approach minimizes the perceived "distance" away from
the application that the user must travel to get help.

<figure nonumber entity=HPHelpKit-GeneralHelpModel>
<\figure>

Staying close to the application makes users more comfortable with online
help and gets them back on task as quickly as possible.

<newpage>



<s2 id=VolumeIsCollectionOfTopics > A "volume" is a collection of "topics"
		<idx|volume: as collection of topics|
		<idx|topics: volume as collection of|

A ++help volume++ is a collection of related topics.  Normally, the topics
within a volume are arranged in a hierarchy.

If you are developing application help, typically there's one help volume
per application.  However, for complex applications, or a collection of
related applications, you might develop several help volumes.   



<s2 id=FamilyIsGroupOfRelatedHelpVolumes > A "product family" is a group of related help volumes

		<idx|product family|
		<idx|family: definition of|
		<idx|group of related volumes, family as|
		<idx|related volumes, family as group of|
		<idx|volume: family of volumes|

Often, software is available as a set of related applications known as a
++product family++.  For example, a set of office productivity applications
may include a word processor, a spreadsheet application, and a drawing
program.  Because each application may have its own help volume, the
multi-volume set forms a product family.

Assembling your help volumes into a product family is optional.  It is
required only if you want your help to be available for browsing within a
++help browser++ such as the &HelpMgrName;.

The top level of the &HelpMgrName; lists product families.  The
second level, under each family, lists the volumes that are members of the
product family.

Even if you have only a single help volume, it must belong to a
product family to be browsable via the &HelpMgrName;.  



<s1 id=AuthorsJob > The Author's Job

<!-- &AuthorsJobSubtopics; -->

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
* <xref KnowYourAudience>
* <xref ConsiderHowYourHelpIsAccessed>
* <xref CollaborateWithApplicationProgrammer>
* <xref OrganizeAndWriteYourTopics>
* <xref CreateRunTimeHelpFiles>
* <xref ReviewHelpAsUserWillSeeIt>
<\list>
<!-- IFDEF ONLINE ONLY END -->

		<idx|author: responsibility|
		<idx|job of author|
		<idx|responsibility: author|

Writing online help differs from writing printed manuals, so it is
important to understand who you are writing for, how the information
is accessed, and how the information fits into an application.



<s2 id=KnowYourAudience > Know your audience

		<idx|knowing audience|
		<idx|audience: knowing|
		<idx|user: knowing|


Just as with any writing, to do a good job you must know your audience and
understand what they require from the information you are writing.  Most
importantly, with online help you need to know the tasks they are
attempting and the problems they may encounter.



<s2 id=ConsiderHowYourHelpIsAccessed > Consider how your help is accessed

		<idx|considering how help accessed|
		<idx|how help accessed, considering|
		<idx|accessed, considering how help|
		<idx|application help|
		<idx|stand-alone help|


It is just as important to understand how users will access your help as it
is to identify your audience correctly.

<procedure>Application Help

If you are writing help for an application, you need to decide which topics
are browsable and which topics are available from the application as
++context-sensitive help++.   A topic is "browsable" if you can navigate 
to it using hyperlinks.  Topics designed exclusively for context-sensitive
help might not be browsable because the only way to display the topic may
be from within a particular context in the application.

You must also decide if you want your application's help volume to be
"registered."  Registered help volumes can be displayed by other
applications (such as the &HelpMgrName;), making the information more
widely accessible.  If another help volume contains hyperlinks to topics
in your help volume, your help volume must be registered.


<procedure>Stand-Alone \ Help Volumes

If you are writing a stand-alone help volume (a help volume not associated
with an application) you may choose to do things very differently.

First, you must provide a path for users to get to all the topics you've
written.  That is, every topic must be browsable via at least one
hyperlink.  Also, because there's no application associated with your help,
you must rely on a help viewer (such as &ViewerAppName;) to display your help volume.



<s2 id=CollaborateWithApplicationProgrammer > Collaborate with the application programmer

		<idx|collaborating: with application programmer|
		<idx|application programmer, collaborating with|
		<idx|programmer, application: collaborating with|


If you are writing application help, you should work closely with the
application programmer.  The degree to which the &ProductName; is
integrated into an application is a design decision that you make
collectively.

If an application and its help
have very loose ties, there may be only a handful of topics that the
application is able to display directly.  This is easier to implement.

In contrast, the application could provide specific help for nearly every
situation in the application.  This requires more work, but benefits the
user if done well.

It's up to you and your project team to determine the right level of help
integration for your project.



<s2 id=OrganizeAndWriteYourTopics > Organize and write your topics

		<idx|organizing: topics|
		<idx|writing: topics|
		<idx|topics: organizing|
		<idx|topics: writing|


The &ProductName; best supports a hierarchical organization of topics.  This
familiar way of organizing information helps users know where they are
when viewing a particular topic.


The General Help Dialog provided by &HelpSystemName; includes a "Topic
Hierarchy" list. It displays all of the topics and subtopics of the current
help volume in a table of contents.  The ++home topic++ is at the
top of the list (the top of the hierarchy).  An arrow ( -> ) points
to the current topic and shows your location in the help volume.
Essentially, this is "you are here" information for the user.


<figure nonumber entity=HPHelpKit-TopicHierarchy>
<\figure>

Although &HelpSystemName; has been optimized for information that is organized in a
hierarchy, you are free to create any kind of organization you want.  The
Quick Help Dialog is a simpler help window that does not include the Topic
Hierarchy.  In this window you can present topics that are not organized
in a hierarchy.  Using <term "hyperlink"|hyperlinks| you can connect
related topics organized in any way you want, including "webs," "chains,"
and "loops."

<procedure>Writing \ with &TagAppName;

Online help is written in ordinary text files.  You use special codes, or
<term tag|tags|, to markup <term element|elements| within the information.
The tags form a markup language called &TagAppName;.


The &TagAppName; markup language defines a hierarchy of <term element|elements|
that define high-level elements such as chapters, sections,
and subsections, and low-level elements such as paragraphs,
lists, and emphasized words.

<xref GeneralMarkupGuidelinesSI> describes the details of using markup.
<xref markup-summary> includes a description of each element and the tags
needed to enter it.

<procedure>Think structure, \ not format

If you are familiar with other publishing systems, you may be accustomed to
formatting information as you like to see it.  Authoring with &TagAppName;
requires you to think about structure and content, not format.

As you write, you use tags to mark certain types of information.  When you
do so, you are identifying !!what!! the information is, but not how it
should be formatted.

For instance, to refer to a book title, include markup like this:

<ex smaller>
&<book>OSF/Motif Style Guide&<\book>
<\ex>

This abstraction separates structure and content from format which allows
the same information to be used by other systems and perhaps formatted
differently.  For instance, &HelpSystemName; displays book titles using an italic
font. However, on another system an italic font may not be available, so
the formatter could decide that book titles are underlined.



<s2 id=CreateRunTimeHelpFiles > Create run-time help files

		<idx|creating: run-time help files|
		<idx|run-time files: creating|
		<idx|files, run-time help: creating|


The text files you write must be "compiled" using the &TagAppName; software to
create ++run-time help files++.  It's the run-time help files that are
accessed when the user requests help.




<s2 id=ReviewHelpAsUserWillSeeIt > Review help as the user will see it

		<idx|reviewing: help as user will see it|
		<idx|user's perspective, seeing help from|
		<idx|perspective, seeing help from user's|


During the authoring process you will need to display your help so
you can interact with it just as your audience will.  There
are two ways to do this:

<list bullet>

 * Using the ``dthelpview'' command, you can display any help topic in any
   help volume.

 * If you are writing application help and the &ProductName; has been
   integrated into your application, you can view your help by running the
   application and making help requests just as the user will.

<\list>



<s1 id=ProgrammersJob > The Programmer's Job

<!-- IFDEF ONLINE ONLY -->

<!-- &ProgrammersJobSubtopics; -->
<list bullet tight>
* <xref ConsiderHowHelpAccessed>
* <xref CollaborateWithHelpAuthor>
* <xref CreateAndManageHelpDialogs>
<\list>

<!-- IFDEF ONLINE ONLY END -->

		<idx|programmer, application: responsibility|
		<idx|responsibility: programmer|

The programmer adds code to an application so that when a user
requests context-sensitive help, the application displays 
help information that is relevant to what the application
is doing at that time.

<note><head>Example Program
The ``/opt/dt/contrib/dthelp/dthelpdemo/'' directory contains the source code
for a sample program called ``dthelpdemo''.  It demonstrates how to add help
dialogs to an OSF/Motif application.
<\note>



<s2 id=ConsiderHowHelpAccessed > Consider how your help is accessed

		<idx|considering how help accessed|
		<idx|accessed, considering how help|

Providing useful information to the user requires taking into account:

<list bullet>

* What confusions commonly arise?  Specific help in these situations
  can save users lots of time.

* Why is the user asking for help now instead of earlier or later?
  If there are several steps in a process and the user is not at the
  first step, branch to
  information that is specific to the step being done.  This is more
  helpful than displaying the same information at each step.
  If the user is at the first step, make available both detailed
  information about the first step and an overview of all the steps.

* Is the user requesting context-specific help or just browsing the
  help information?  If it is context-specific, supply
  information that's relevant to the task now being done.

<\list>

<!-- in a future release, add a section on "know your audience" -->




<s2 id=CollaborateWithHelpAuthor > Collaborate with the help author

		<idx|collaborating: with help author|
		<idx|author: collaborating with|

Close collaboration with the online help author is needed because 
the author needs to know how each context-specific topic is reached
and the programmer needs to know what is explained in each
context-specific topic.  Without such coordination, the user may see
irrelevant, ambiguous, or misleading information.

Collaboration makes the best use of the programmer's understanding of
the application and the author's understanding of how to best communicate
relevant information to the user.





<s2 id=CreateAndManageHelpDialogs > Create and manage help dialogs

		<idx|creating: help dialog|
		<idx|managing help dialogs|
		<idx|dialog: creating and managing help dialogs|
		<idx|widgets provided by &HelpSystemName;|


&HelpSystemName; is designed especially for use with OSF/Motif applications.
Specifically, &HelpSystemName; extends the OSF/Motif widget set by providing
two new widget classes (plus convenience functions to manipulate them):

<list bullet>

 * The !!quick help dialog!!, which provides a simple help window with a topic 
   display area and a few dialog buttons.

 * The !!general help dialog!!, which provides a help window that includes
   a menu bar and a topic hierarchy in addition to the help topic display
   area.

<\list>

You can use either or both of these types of help windows within your
application.  Once the application is compiled (with the &HelpSystemName;
library), the help windows become part of the application.







