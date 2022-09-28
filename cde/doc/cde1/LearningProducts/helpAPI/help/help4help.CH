<chapter id=ChapterProvidingHelpOnHelp > Providing Help On Help

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
 * <xref AccessingHelp4HelpSI>
 * <xref WritingYourOwnHelp4HelpSI
<\list>
<!-- IFDEF ONLINE ONLY END -->



&headerFix;

		<idx|providing: help on help|
		<idx|help on help: providing|
		<idx|``Help4Help''|

"Help on help" tells users how to use the &ProductName;.  Specifically,
it describes such tasks as using hyperlinks, using the keyword index,
and printing help topics.  Normally, help on help is supplied as an
individual help volume named "Help4Help."

<procedure>For Application Help

If you are writing application-specific help, there are two ways to ensure
that your application has help on help for its own help dialogs:

<list bullet>

 * !!Rely on the &DesktopName; help on help volume.!!	 For example, on
   workstations running the &DesktopName; the standard Help4Help volume
   is installed.

 * !!Or, supply your own help on help volume.!!	 The &TagAppName; source files
   for the Help4Help volume are provided in the
   ``/opt/dt/dthelp/help/C/Help4Help/'' directory.  A ``build/''
   subdirectory contains &TagAppName; processing files.  You run
   &TagAppName; in this directory to create the run-time help files.
   Graphics files used in the help on help volume are stored in the
    ``build/graphics/'' subdirectory. 

<\list>

<procedure>For Stand-Alone Help

If you are writing stand-alone help, you are probably relying on the
&ViewerAppName; program already being installed and ready to use.
If this is the case, you don't have to worry about help on help
because &ViewerAppName; accesses the standard Help4Help volume by default.

<procedure>How Help on \ Help is Found

Each application that uses the &ProductName; (including &ViewerAppName;) has a
``helpOnHelpVolume'' resource that identifies a help volume to be accessed
for help on help topics.  For &ViewerAppName;, this resource is set as
follows:

<ex smaller>
&ViewerAppName;*helpOnHelpVolume:  Help4Help
<\ex>

If you provide your own help on help volume, be sure to give it a unique
name so it doesn't conflict with another help on help volume that may be
installed on the system.



<s1 id=AccessingHelp4HelpSI>Accessing Help on Help in an Application

<!-- IFDEF ONLINE ONLY -->
<!-- &AccessingHelp4HelpSISubtopics; -->
<list bullet tight>
* <xref ToSetHelpOnHelpResourceTA>
* <xref ToProvideUsingHelpCommandTA>
* <xref ToAddHelpOnHelpForQuickHelpTA>
* <xref ToDisplayHelpOnHelpTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->

		<idx|help on help|
		<idx|accessing: help on help|
		<idx|``Help4Help''|

Your application should do the following to support help on help:

<list bullet>

 * Set the ``helpOnHelpVolume'' resource to identify the help volume you
   want to access.

 * Add a "Using Help" command to the application's Help menu.

 * Add support for getting help on quick help dialogs.

<\list>


<s2 id=ToSetHelpOnHelpResourceTA>To set the `helpOnHelpVolume' resource

		<idx|setting ``helpOnHelpVolume'' resource|
		<idx|``helpOnHelpVolume'' resource|
		<idx|resource: ``helpOnHelpVolume''|
		<idx|app-defaults file|

<list bullet>

 * Add a line to your application's app-defaults file like this:

<ex>%%App-class%%*helpOnHelpVolume:  %%volume%%<\ex>

   Where %%App-class%% is the application's class name and %%volume%% is
   the name of the help on help volume you want to access.

 * !!Or,!! within your application, set the ``helpOnHelpVolume'' resource
   for each general help dialog you create.  (Quick help dialogs do not
   support this resource.)

<\list>

<procedure>Examples

Here's the line from ``dthelpview's'' app-defaults file (``DtHelpview'')
that specifies the help on help volume:

<ex smaller>
DtHelpview*helpOnHelpVolume:  Help4Help
<\ex>

To specify the help on help volume when creating a help dialog, add it to
the argument list passed to the create function as shown here:

<ex smaller>
ac = 0;
XtSetArg (al[ac], XmNtitle, "My Application - Help"); ac+&empty;+;
XtSetArg (al[ac], XmNhelpOnHelpVolume, "Help4Help");  ac+&empty;+;
helpDialog = DtCreateHelpDialog (%%parent%%, "helpDialog", al, ac);
<\ex>


<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hh4res.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToProvideUsingHelpCommandTA>
<\list>

<newpage>



<!-- IFDEF ONLINE ONLY END -->


<s2 id=ToProvideUsingHelpCommandTA>To provide a Using Help command

		<idx|providing: Using Help command|
		<idx|Using Help command, providing|
		<idx|command, Using Help|
		<idx|``HelpRequestCB()''|
		<idx|function: ``HelpRequestCB()''|

<list order>

 * Add to your Help menu a button labeled "Using Help".  Also add the
   necessary "activate" callback to call your ``HelpRequestCB()'' function.

 * Add support to your ``HelpRequestCB()'' function to display help
   on help.  Specifically:

   <list bullet tight>
    * Create a quick help dialog (or retrieve one from your cache).
    * Set the dialog's title to "Help On Help."
    * Display the home topic of the help on help volume.
    * Manage the quick help dialog.
   <\list>

<\list>

<procedure>Example

The following lines create a menu button labeled "Using Help..." that calls
the ``HelpRequestCB()'' function.  

<ex smaller>
/* %%Create the `Using Help ...' button.%% */
labelStr = XmStringCreateLtoR ("Using Help ...", XmSTRING_DEFAULT_CHARSET);
ac = 0;
XtSetArg (al[ac], XmNlabelString, labelStr);     ac+&empty;+;
button = XmCreatePushButtonGadget (%%parent%%, "usingHelpButton", al, ac);
XtManageChild (button);
XmStringFree (labelStr);

/* %%Add a callback to the button.%% */
XtAddCallback (button, XmNactivateCallback, HelpRequestCB, USING_HELP);
<\ex>

``USING_HELP'' is the client data passed to the ``HelpRequestCB()''
function when the menu button is chosen by the user.  Presumably it has
been defined somewhere in the application (perhaps in a ``Help.h'' file)
as a unique integer:

<ex smaller>
#define USING_HELP  47
<\ex>

To see how the ``HelpRequestCB()'' function handles the ``USING_HELP''
case, see the example in <xref ToDisplayHelpOnHelpTA>.

<newpage>





<s2 id=ToAddHelpOnHelpForQuickHelpTA>To provide help on help for a quick help dialog

		<idx|providing: help on help|
		<idx|help on help: providing|
		<idx|``HelpRequestCB()''|
		<idx|function: ``HelpRequestCB()''|

<list order>

 * After creating the quick help dialog, do the following:

   <list bullet tight>
    * Manage the dialog's Help button.
    * Add a help callback to the dialog.
   <\list>

 * Add support in your ``HelpRequestCB()'' function to handle the case when
   a user requests help in a quick help dialog.  Specifically:

   <list bullet tight>
    * Create a quick help dialog (or retrieve one from your cache).
    * Set the dialog's title to "Help On Help."
    * Display the home topic of the help on help volume.
    * Manage the quick help dialog.
   <\list>

<\list>

<procedure>Example

The following program segment creates a quick help dialog, manages its
Help button, and adds a help callback to the dialog:

<ex smaller>
/* %%Create a quick help dialog.%% */
ac = 0;
XtSetArg (al[ac], XmNtitle, "My Application - Help"); ac+&empty;+;
helpDialog = DtCreateHelpDialog (%%parent%%, "helpDialog", al, ac);

/* %%Manage the dialog's Help button.%% */
XtManageChild (DtHelpQuickDialogGetChild (quickHelpDialog, 
                                       DT_HELP_QUICK_HELP_BUTTON));

/* %%Add a help callback to enable the F1 key and the Help button.%% */
XtAddCallback (quickHelpDialog, XmNhelpCallback, 
               HelpRequestCB, USING_HELP);
<\ex>


To see how the ``HelpRequestCB()'' function handles the ``USING_HELP''
case, see the example in <xref ToDisplayHelpOnHelpTA>.


<need begin>




<s2 id=ToDisplayHelpOnHelpTA>To display help on help

		<idx|help on help: displaying|
		<idx|displaying: help on help|
		<idx|``HelpRequestCB()''|
		<idx|function: ``HelpRequestCB()''|

<list order>
 * Create a quick help dialog (or retrieve one from your cache).
 * Display in the dialog the home topic of your help on help volume.
<\list>

Help on help can be displayed in a general help window.  However, 
a quick help dialog is recommended because its user interface is
simpler, which is less intimidating to new users who commonly need
help on help.

<need end>

<procedure>Example

The following program segment is part of a ``HelpRequestCB()'' function.
Presumably, the ``USING_HELP'' constant is passed to the function
because the user chose Using Help from the application's Help menu or chose
the Help button in a quick help dialog.

<need begin>
This example assumes that the application never creates more than one "Help
On Help" dialog and maintains its widget ID in a variable called
``onHelpDialog''.

<ex smaller>
case USING_HELP:
  if (onHelpDialog == (Widget)NULL)
    {
       /* %%Get a quick help dialog for use as the `help on help' dialog.%% */
       onHelpDialog = FetchHelpDialog (True);

       if (onHelpDialog == (Widget)NULL)
         /* %%We didn't get a dialog!  Add your error handling code here.%% */
    }

  /* %%Set the proper volume and ID to display the home topic of%%
     %%the help on help volume.  Also, set the dialog's title.%%   */
  ac = 0;
  XtSetArg (al[ac], XmNtitle,      "Help On Help");     ac+&empty;+;
  XtSetArg (al[ac], XmNhelpType,   DT_HELP_TYPE_TOPIC); ac+&empty;+;
  XtSetArg (al[ac], XmNhelpVolume, "Help4Help");        ac+&empty;+;
  XtSetArg (al[ac], XmNlocationId, "_hometopic");       ac+&empty;+;
  XtSetValues (onHelpDialog, al, ac);

  /*  %%If the `help on help' dialog is already managed, it might%%
      %%be in another workspace, so unmanage it.%%  */
  if (XtIsManaged (onHelpDialog))
    XtUnmanageChild (onHelpDialog);

  /* %%Manage the `help on help' dialog.%% */
  XtManageChild (onHelpDialog);

  break;
<\ex>

To see how the rest of the ``HelpRequestCB()'' function might be
structured, refer to the example in <xref ToAddHelpCallbackTA>.
<need end>



<!-- seealsos/Hh4hdis.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateQuickHelpDialogTA>
 * <xref ToRetrieveADialogFromYourCacheTA> (includes a sample
   ``FetchHelpDialog()'' function)
 * <xref ToDisplayAStandardTopicTA>
<\list>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToProvideUsingHelpCommandTA>
 * <xref ToAddHelpOnHelpForQuickHelpTA>
 * <xref ToRetrieveADialogFromYourCacheTA>
 * <xref ToSetHelpOnHelpResourceTA>
<\list>



<!-- IFDEF ONLINE ONLY END -->



<s1 id=WritingYourOwnHelp4HelpSI>Writing Your Own Help on Help Volume

<!-- IFDEF ONLINE ONLY -->
<!-- &WritingYourOwnHelp4HelpSISubtopics; -->
<list bullet tight>
* <xref ToCopyTheH4HSourceFilesTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->
		<idx|writing: help on help volume|
		<idx|help on help: writing volume|

If you need to provide your own help on help volume, you should start with
the existing Help4Help volume, then make the necessary changes.  All the
source files used to write the Help4Help volume are provided in the
``/opt/dt/dthelp/help/C/Help4Help/'' directory.

It is important that you name your help on help volume something other than
Help4Help, to prevent installation conflicts.  Consider picking a name that
is specific to your product.  For example, if your application's help
volume is "Newapp," perhaps your help for help volume could be "NewappH4H."

<procedure>Required \ Entry Points

To ensure that context-sensitive help within a help dialog operates
correctly, you must provide the following entry points (IDs) within your
help on help volume. (These are already included in the Help4Help source
files.)

<lablist width=7pc>
<labheads>\ ID \    Topic Description

		<idx|``_hometopic'' ID|

 \ ``_hometopic'' \ Displays an introduction to using the help system.
		    This topic is displayed when you choose Using Help from
		    the general help dialog's Help menu, or when you press
                    F1 in a quick help dialog.  (The ID ``_hometopic''
                    is created automatically by the ``&<hometopic>''
                    element.)

		<idx|``_copyright'' ID|

 \ ``_copyright'' \ Displays the copyright and version information for the
		    help on help volume.  This topic is displayed when you
		    choose Version from the general help dialog's Help
		    menu.  (The ID ``_copyright'' is created automatically
		    by the ``&<copyright>'' element.)

		<idx|``history'' ID|

 \ ``history'' \    Displays a topic that describes how to use the History
		    dialog.  This topic is displayed when you choose Help
		    or press F1 within the History dialog.

		<idx|``printing'' ID|

 \ ``printing'' \   Displays a topic describing how to use the Print
		    dialog.  This topic is displayed when you choose Help
		    or press F1 within the Print dialog.

		<idx|``keyword-index'' ID|

 \ ``keyword-index'' \  Displays a topic describing how to use the Keyword
			Search dialog.	This topic is displayed when you
			choose Help or press F1 within the Keyword Search
			dialog.
<\lablist>





<s2 id=ToCopyTheH4HSourceFilesTA > To copy the Help4Help source files

		<idx|copying Help4Help source files|
		<idx|``Help4Help'': copying source files|

<list order>

 * Copy the entire ``/opt/dt/dthelp/help/C/Help4Help/'' directory to a new
   working directory (%%new-dir%%) using a command like this:

<ex>
cp -r /opt/dt/dthelp/help/C/Help4Help %%new-dir%%   
<\ex>

   This creates %%new-dir%% and copies all the files and directories
   into it.

 * To permit editing the files (which are copied as "read only"),
   change the permissions using a command like this:

<ex>
chmod -R u+w %%new-dir%%
<\ex>

<\list>

The source for the Help4Help volume includes these files:

<ex smaller>
MetaInfo       TopicMap        Printing
HomeTopic      KeywordIndex    Config
Hyperlinks     History         Commands   
<\ex>

Also included is a ``build/'' directory, where you run &TagAppName; to create
the run-time help files.  Graphics are stored in the ``build/graphics/''
subdirectory.

Be sure to rename the ``Help4Help.htg'' file before running &TagAppName;.  Your
help on help volume should have a unique name to prevent conflicts with
other help on help volumes.

<procedure>Example

The following commands create a copy of the help on help volume and make its
files writable.  (Presumably the ``projects/'' subdirectory already exists.)

<ex smaller>
cp -r /opt/dt/dthelp/help/C/Help4Help /users/dex/projects/NewHelp4Help
chmod -R u+w /users/dex/projects/NewHelp4Help
<\ex>

To build a new version of the run-time help files, first ensure that the
directory ``/opt/dt/bin/'' is in your search path.  Then, change to the new
directory, rename the ``Help4Help.htg'' file, and run &TagAppName;:

<ex smaller>
cd /users/dex/projects/NewHelp4Help
mv Help4Help.htg NewH4H.htg
dthelptag NewH4H
<\ex>

When the &TagAppName; software is done, you can display the new help on help
volume using this command:

<ex smaller>
dthelpview -helpVolume NewH4H &&
<\ex>
