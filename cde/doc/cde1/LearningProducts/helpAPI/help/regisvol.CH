<chapter id=ChapterRegisteringHelpVolumes > Preparing Your Product
&headerFix;

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
 * <xref HowHelpFilesAreFoundSI>
 * <xref GatheringRunTimeHelpFilesSI>
 * <xref RegisteringYourHelpSI>
 * <xref ProductPreparationChecklistSI>
<\list>

<!-- IFDEF ONLINE ONLY END -->


  <idx|registering a help volume|
  <idx|registering a help family|
  <idx|volume: registering|

When it comes time to prepare your final product, you must be sure that all
your help files are created and installed properly.  

<procedure>Where to Install \ Your Help Files

There are no restrictions regarding where your run-time help files can be
installed.  However, these conventions are recommended:

<list bullet>

    * Application-specific help is usually installed with the rest of the
      application's files. Because the &DesktopName; uses the System V.4 
      filesystem, you may want to conform to its guidelines.  If so, install
      your application and its supporting files in ``/opt/%%appname%%''.
      For installation instructions, refer to the
      <book>CDE Application Installation and Configuration Guide<\book>.


   * If you've written a stand-alone help volume, your run-time help files
     should go in their own subdirectory named ``/usr/lib/dthelp/%%product%%/'',
     where %%product%% is a meaningful directory name for your product.

     Within your ``%%product%%/'' subdirectory, an additional subdirectory is
     recommended for the run-time help files for each help volume
     (``%%volume%%/'').  For example, help volumes for an application named
     %%Librarian%% would be installed in ``/usr/lib/dthelp/Librarian/'',
     with a subdirectory for each separate help volume.

<\list>

You should also provide a ``%%language%%/'' subdirectory to accommodate
help in multiple languages (where %%language%% matches the user's LANG
environment variable).  For example, a German version of the Librarian
online help would be stored in ``/usr/lib/dthelp/Librarian/german/''.  The
default version would be stored in ``/usr/lib/dthelp/Librarian/C/''.

<procedure>Registration

An additional important step in installing your help files is <term
nogloss|registration|.  The registration process enables two important
features of the &HelpSystemName; System:

<list bullet>

   * <term nogloss|Cross-volume hyperlinks| &emdash; A hyperlink in one help volume
     can refer to another help volume using just the volume name and an ID
     within the volume.  If the destination volume is registered, the link
     does not have to specify where the volume is stored on the filesystem.

   * <term nogloss|Help family browsing| &emdash; If you also register a "product
     family" that contains one or more help volumes, then your help volumes
     will be browsable using the &HelpMgrName;.

<\list>



<s1 id=HowHelpFilesAreFoundSI > How a Help Volume is Found

<!-- IFDEF ONLINE ONLY -->
<!-- &HowHelpFilesAreFoundSISubtopics; -->
<list bullet tight>
  * <xref ToChangeHelpSearchPathTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->


		  <idx|how help files found|
		  <idx|finding help files|
		  <idx|volume: finding|
		  <idx|family: finding|
		  <idx|product family: finding|
                  <idx|environment variable: user help search path|
                  <idx|environment variable: system help search path|

When you specify a help volume, you can use a complete path to the
``%%volume%%.sdl'' file, or you can provide just the base name of the
volume.  If you specify a complete path, the &HelpSystemName; System looks
only for the exact directory and file name you provide.

However, if you provide just the base name (%%volume%%), the help system
searches several directories for the volume.  The search ends when the
first matching ``%%volume%%.sdl'' file is found.  The value of the user's
LANG environment variable is also used to locate help in the proper
language (if it's available).

<!-- COMMENT from Mike Wilson 9-14

#define DTHELPSEARCHPATH "
/etc/opt/dt/appconfig/help/C/%H:\
/etc/opt/dt/appconfig/help/C/%H.sdl:\
/etc/opt/dt/appconfig/help/%L/%H:\
/etc/opt/dt/appconfig/help/%L/%H.sdl:\


#define DTUSERHELPSEARCHPATH "
<$HOME>/.dt/help/%%H:\
<$HOME>/.dt/help/%%H.sdl:\
-->

<need begin>
<procedure>Personal \ Help Volumes

&HelpSystemName; first looks in the user's home directory and searches
this subdirectory:

<ex smaller>
%%HomeDirectory/%%.dt/help/

<\ex>
<need end>

Individual users can override this list of directories by setting the
DTUSERHELPSEARCHPATH environment variable.  

		 <idx |environment variable: DTUSERHELPSEARCHPATH|
                 <idx |DTUSERHELPSEARCHPATH|
                 
<need begin>
<procedure>System-Wide \ Help Volumes

If no match is found after searching for personal help volumes, the
system-wide locations are searched.  They are:

<ex smaller>
/etc/opt/dt/appconfig/help/%%language%%
/etc/opt/dt/appconfig/help/C
<\ex>
<need end>

                <idx |environment variable: DTHELPSEARCHPATH|
                <idx |DTHELPSEARCHPATH|
                 
The default system search paths can be overridden by setting the
DTHELPSEARCHPATH environment variable.   System-wide search path 
configuration files are located in the directory ``/etc/opt/dt/Xsession.d.''
Generally, this environment variable is set for all users by the system
administrator. 



<procedure>Specifying Paths

Both environment variables use a colon-separated list of directories.  That
is, you separate multiple directories with the colon character (``:'').  In
addition, the paths use a special variable syntax in which a percent character (``%'') followed by a letter indicates where values should be
inserted before the search takes place.

<need begin>
For example, the default system-wide search paths are specified using this
syntax: 

<ex smaller>
/etc/opt/dt/appconfig/help/%L/%H: \
/etc/opt/dt/appconfig/help/%L/%H.sdl: \
/etc/opt/dt/appconfig/help/C/%H: \
/etc/opt/dt/appconfig/help/C/%H.sdl:

<\ex>
<need end>

Where ``%L'' is replaced by the value of
the user's LANG environment variable, and ``%H'' is the name of the file
being searched for.  Both ``%H'' and ``%H.sdl'' are specified for each
directory, which means the ``.sdl'' extension is optional when specifying a
help volume.


<procedure>Using &ViewerAppName;

When you run &ViewerAppName; to display a help volume, you specify a help volume
using the ``-helpVolume'' command line option:

<ex>dthelpview -helpVolume %%volume%% && <\ex>

If you specify a full path to a ``.sdl'' file, then the normal search paths
are not used.  If you specify just the name of the volume, &ViewerAppName;
looks in these places for the help volume:

<list bullet tight>
   * The current directory.
   * The directories specified using the DTUSERHELPSEARCHPATH environment
     variable.
   * The directories specified using the DTHELPSEARCHPATH environment
     variable. 
   * Otherwise, the built-in default search paths.

<\list>

By searching the current directory first, &ViewerAppName; makes it easier to
display a volume immediately after running &TagAppName;, without having to
register the volume or type a full path.



<s2 id=ToChangeHelpSearchPathTA > To change the help search paths

		  <idx|changing: help search path|
		  <idx|search path, help, changing|
		  <idx|path, help search, changing|
                  <idx|environment variable, help search path|


<list bullet>

   * For a personal configuration, set the DTUSERHELPSEARCHPATH environment
     variable to override the default personal search path.  Edit 
     ``%%HomeDirectory%%/.dtprofile'' to add or remove directories
     from the search path.  This is useful during project development to
     access a help volume that you don't want others to access.

   * For a system-wide configuration, set the DTHELPSEARCHPATH
     environment variable to override the system search path.  System-wide
      search path configuration files are located in the directory
      ``/etc/opt/dt/Xsession.d.''  Changing this variable is useful
      for accessing help installed on another system in your network.  
      Typically, this task is performed by the system adminstrator for
      all users.

<\list>

<need begin>
To edit the system-wide search path configuration files, you must
have superuser permissions.  When you're done editing, save the
configuration file, then log out and back in.  To update your Help
Manager, run this command:

                  <idx|``dthelpgen'' command|
		  <idx|command: ``dthelpgen''|

<ex smaller>
     /opt/dt/bin/dthelpgen -register
<\ex>

Changes made to the configuration files take effect when a user logs into the &DesktopName;.  

<need end>


If both personal and system-wide search paths are specified, the help search
path includes directories in each of the variables.  Personal search paths are
searched before system-wide paths.



<!-- COMMENT OUT THIS EXAMPLE
<procedure>Example

This example shows how the DTHELPSEARCHPATH environment variable is
used within &DesktopName; to access help installed on another computer on the
network.  (These tasks require superuser permission to mount a new
filesystem and edit the ``Xsession'' script.)

To make the remote files accessible, "mount" the remote system's ``/etc/''
disk volume on your local system in the directory
``/etc/%%hostname%%/etc/''.  (Refer to the operating system documentation
or online help to learn how to "mount" a network file system.)

Once the remote files are physically available, you must tell the
&HelpSystemName; System where to look for them.  You do this in
&DesktopName; by editing the ``/opt/dt/config/Xsession'' file.  Search for
the section of the file that defines DTHELPSEARCHPATH, then uncomment and
edit the lines so that they look like this:

<need begin>
<ex smaller>
DTHELPSEARCHPATH=\
/etc/opt/dt/appconfig/help/%L/%H:\
/etc/opt/dt/appconfig/help/%L/%H.sdl:\
/etc/opt/dt/appconfig/help/C/%H:\
/etc/opt/dt/appconfig/help/C/%H.sdl:\
/nfs/%%hostname%%/etc/opt/dt/appconfig/help/%L/%H:\
/nfs/%%hostname%%/etc/opt/dt/appconfig/help/%L/%H.sdl:\
/nfs/%%hostname%%/etc/opt/dt/appconfig/help/C/%H:\
/nfs/%%hostname%%/etc/opt/dt/appconfig/help/C/%H.sdl:\
<\ex>

<need end>

Where %%hostname%% is the name of the directory you created for the remote
system.
-->


<s1 id=GatheringRunTimeHelpFilesSI>Gathering Run-Time Help Files

<!-- IFDEF ONLINE ONLY -->
<!-- &GatheringRunTimeHelpFilesSISubtopics; -->
<list bullet tight>
* <xref ToGatherRunTimeHelpFilesTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->
		  
                  <idx|run-time files: gathering|
		  <idx|gathering run-time help files|
		  <idx|files, run-time help: gathering|

<procedure>
Master \ Help Volume


&TagAppName creates a single run-time help volume, ``%%volume.sdl%%''.
The base name, %%volume%%, is the same as the base name of your
``%%volume%%.htg'' file.  &ViewerAppName; uses information stored
in this master help volume and also accesses any associated graphic
files.


<procedure>Graphic \ Files

If your help volume uses graphic files, the files are typically stored in a
separate directory for convenience.  However, you may choose to store them
in the same location as your ``%%volume.%%htg'' file.


A help volume does not include actual graphic images.  Instead, it contains
a "reference" to the location of each graphic file.  When you run
&TagAppName;, the ``dthelptag'' compiler incorporates the pathnames of the
graphic files into the help volume.


Therefore, if you choose to move the run-time file, ``%%volume%%.sdl'', or any of
its graphic files to a new location, you must maintain the same relative
location of the files.  Otherwise, the help volume will be unable to locate
the graphic files.


<caution>
Never rename a run-time help file or graphics file after running &TagAppName;.
The information stored in the ``%%volume%%.sdl'' file depends on the
original names.  

If you rename your ``%%volume%%.htg'' file or any of your
graphic files, be sure to rerun &TagAppName;.
<\caution>



<s2 id=ToGatherRunTimeHelpFilesTA >To gather the run-time help files for a volume

		  <idx|gathering run-time help files|
		  <idx|run-time files: gathering|
		  <idx|files, run-time help: gathering|

<list bullet>

   * From the directory where you ran &TagAppName;, copy these files into the
     installation directory:

<ex>
  %%volume%%.sdl
<\ex>

 Where the base name, %%volume%%, is the
 same as the base name of your ``%%volume%%.htg'' file.


   * If your volume includes any graphics, copy the graphic files too.

<\list>

<note>

Graphic files !!must!! be installed in the same relative position to
the ``%%volume%%.sdl'' file that they were in when the "dthelptag"
command was run.

For example, if your graphics files are in a subdirectory named "graphics/"
one level below your ``%%volume%%.htg'' file, then when you install
your help files in a different location, the graphics must again be
placed in a subdirectory named "graphics/" one level below the
``%%volume%%.sdl'' file.

<\note>

You don't need to ship the ``%%volume%%.htg'' or any additional files
generated by the &TagAppName; software.



<s1 id=RegisteringYourHelpSI>Registering Your Online Help

<!-- IFDEF ONLINE ONLY -->
<!-- &RegisteringYourHelpSISubtopics; -->
<list bullet tight>
* <xref ToCreateHelpFamilyTA>
* <xref ToUpdateSystemHelpVolumeTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->


		  <idx|registering a help volume|
		  <idx|registering a help family|

Registering your online help is important because it makes it easier to
access the help you provide.  For authors and programmers, it's easier
because references to your volume can use just the volume name &emdash; 
without specifying the volume's actual location.

If you register a product family with one or more help volumes, you make
your help available for general browsing.  This allows access to
application-specific help without using the application.  Or, if you are
writing stand-alone help, this is the only way for users to get to your
help.

<procedure>Registering Help Volumes

After the run-time files for a help volume have been installed, the volume
is registered by creating a symbolic link to the volume's
``%%volume%%.sdl'' file.  The link is created in one of the directories
that the help system searches for volumes.  For most help volumes, the
appropriate place for the link is
``/etc/opt/dt/appconfig/help/%%language%%/'', where %%language%% is ``C''
for the default computer language (which is usually English).


<procedure>Registering a Product Family

If you are also registering a product family, you create and install a
help family file (``%%product%%.hf'') with the rest of the product's help
files.  You register the family file by creating a symbolic link to the
``%%product%%.hf'' file.  For most products, the appropriate place for the
link is ``/etc/opt/dt/appconfig/help/%%language%%/''.

		  <idx|``dthelpgen'' command|
		  <idx|command: ``dthelpgen''|

Family files are read by the ``dthelpgen'' program (which is part of
&DesktopName;), which uses them to create a special help volume that lists
the families (and the volumes within each family) installed on the system.

<note>
Refer to the <book>Common Desktop Environment Application Installation and Configuration Guide<\book>
for instructions to install and register your help family and help volumes. The
&DesktopName; provides a script, ``dtappintegrate'', which creates the
appropriate symbolic links from the registration directory to your help
volume and family file.
<\note>

<need begin>


<s2 id=ToCreateHelpFamilyTA > To create a help family

		  <idx|creating: help family|
		  <idx|family: creating|

<list order>

   * Pick a file name that is unique to your product.  Use the ``.hf''
     extension to identify the file as a !!help family!!.

  <ex>%%family%%.hf<\ex>

   * Enter the following lines into the file:

<ex>
  *.charSet:      %%character-set%%
  *.title:        %%family title%%
  *.bitmap:       %%icon file%%
  *.abstract:     %%family abstract%%
  *.volumes:      %%volume  volume  volume ...%%
<\ex>

     Where %%character-set%% specifies the character set used by the %%family
     title%% and %%family abstract%% strings.  The %%family title%% and
     %%family abstract%% should not contain any &TagAppName; markup;
     this file is !!not!! processed with the &TagAppName; software.

     The %%icon file%% is optional.  If you provide one, the path you use
     to specify the location of the file should be %%relative%% to
     the location of the family file.  (See the
     example below.)  If you do not provide an icon, do not include
     the ``*.bitmap'' resource in your family file.

     The list of %%volume%% names identifies which volumes belong to the
     family.  The volumes will be listed in the order they appear on this
     line.  A volume may be listed in more than one family.

     Any line in the file that begins with an exclamation mark (``!'') is
     a comment line and is ignored.

   * When you prepare your final product, you should install your
     ``%%family%%.hf'' file with the rest of your help files.  When
      the script, ``dtappintegrate'' is executed, it creates the
      symbolic links to your family file.


<\list>

If any of the values occupy more than one line, end each line &emdash; except the
last &emdash; with a backslash (``\'').

<need end>
&empty;
<need begin>
<procedure>Example

Here's a family file for the &DesktopName; online help.  Comments 
at the top of the file identify the family and release version.


<vex smaller>

!##############################################
!#                                            #
!#          CDE Desktop  Help Family          #
!#                                            #
!#                 Version 1.0                #
!#                                            #
!##############################################


*.charSet:      iso8859-1
*.title:        CDE Desktop Version 1.0
*.bitmap:       ../../../../opt/dt/help/C/cdelogo.pm
*.abstract:     Overview and Basic Desktop Skills \
                * General Configuration \
                * Front Panel \
                * File Manager and the Desktop \
                * Style Manager \
                * Text Editor \
                * Icon Editor

*.volumes: Intromgr.sdl Misctopic.sdl \ 
           FPanel.sdl Filemgr.sdl \
           Stylemgr.sdl Textedit.sdl \
           Iconed.sdl
<\vex>

The help family file actually included with the &DesktopName; software may not
exactly match this example. 
<need end>



<s2 id=ToUpdateSystemHelpVolumeTA > To update the "browser" help volume

		<idx|updating system help volume|
		<idx|system help volume, updating|
		<idx|volume: system help, updating|

<list bullet>

 * The ``dtappintegrate'' script executes this command after
   creating the links for registration:

		<idx|``dthelpgen'' command|
		<idx|command: ``dthelpgen''|

<ex>/opt/dt/bin/dthelpgen -register <\ex>

<\list>

The ``dthelpgen'' program creates a new volume stored in
``/opt/dt/help/%%language%%/Browser/'' that contains a two-level
hierarchy.  The first level lists each of the help families; the
second level lists the volumes within each family.  (The ``dthelpgen''
program is part of the &DesktopName;.)

<need begin>
To display the browser volume, execute this command:

<ex smaller>
dthelpview -helpVolume browser &&
<\ex>

<note>
If you run ``dthelpgen'' while the browser volume is displayed in a help
window, you should close the window, then rerun ``dthelpview''.
<\note>

<need end>


<s1 id=ProductPreparationChecklistSI>Product Preparation Checklists

		<idx|product preparation checklist|
		<idx|checklist: product preparation|
		<idx|preparing product|
		<idx|delivery, preparing for product|

The following checklists should help you verify that you've prepared your
product correctly.  Of course, there's no substitute for testing
your product by using it just as a user will.

<procedure>For Authors

		<idx|checklist: author's|

<list bullet>

 * !!A final version of the run-time help file was created.!!

   Here are the recommended commands for creating the run-time files:

<ex smaller>
   dthelptag -clean %%volume%%
   dthelptag %%volume%% nomemo onerror=stop
<\ex>

   The ``-clean'' option removes files from any previous ``dthelptag''
   command, the ``nomemo'' option ensures that writer's memos are not
   displayed, and the ``onerror=stop'' option stops processing if
   any parser errors occur.  You should not distribute a help volume
   that has any parser errors.

 * !!All hyperlinks have been tested!!.	 Each hyperlink displays the proper
   topic or performs the correct action.

 * !!All graphics are acceptable.!!  The graphics have been tested on various
   color, grayscale, and monochrome displays.

<\list>

<procedure>For Product\ Integrators

		<idx|checklist: product integrator's|

<list bullet>

 * !!The %%volume%%.sdl run-time file is installed.!!

 * !!All graphics are installed in the proper locations.!!  Each graphics
   file must be installed in the same relative position to the ``.sdl'' file
   that it was in relative to the ``.htg'' file when the &TagAppName;
   software was run.

 * !!The help volume is registered.!!  The ``dtappintegrate'' script
   was run to create symbolic links from the registration directory to
   the installation directory.


 * !!A product family file is installed and registered.!!  The family file
   is installed with the other help files.  When ``dtappintegrate'' is run,
   it creates a symbolic link for the family file.  Registering a family
   file for your help volume is optional.  However, if you choose not to
   register a family file, your help volume will not be added to the 
   list of volumes accessible from the &HelpMgrName; Browser.
<\list>


<procedure>For Programmers

		<idx|checklist: application programmer's|

<list bullet>
 * !!The application sets the correct values for these required
   resources:!!

<ex smaller>
   %%App-class%%*helpVolume:        %%volume%%
   %%App-class%%*helpOnHelpVolume:  %%help-on-help-volume%%
<\ex>

   The ``helpVolume'' resource identifies the help volume for your
   application.	 The ``helpOnHelpVolume'' identifies the help volume that
   contains the help on using the help system.

 * !!The application sets the desired values for the following
   optional resources:!!

<ex smaller>
   %%App-class%%*DtHelpDialogWidget*onHelpDialog*rows:     %%rows%%
   %%App-class%%*DtHelpDialogWidget*onHelpDialog*columns:  %%columns%%

   %%App-class%%*DtHelpDialogWidget*definitionBox*rows:    %%rows%%
   %%App-class%%*DtHelpDialogWidget*definitionBox*columns: %%columns%%
<\ex>

The ``onHelpDialog'' resources control the size of the quick help dialogs
used to display Help On Help.  The ``definitionBox'' resources control the
size of the quick help dialog used for definition links:


* !!A help font scheme has been appended to the application's
   app-defaults file.!!	 Each font scheme is a set of resources.  Sample
   font schemes are provided in the
    ``/opt/dt/dthelp/examples/fontschemes'' directory.
<\list>











