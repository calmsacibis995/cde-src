<chapter id=ChapterRespondingToHelpRequests > Responding to \ Help Requests

<!-- IFDEF ONLINE ONLY -->

<list bullet tight>
 * <xref DisplayingHelpTopicsSI>
 * <xref EnablingHelpKeyF1>
 * <xref ProvidingHelpMenu>
 * <xref SupportingItemHelpSI>
 * <xref UsingTopicAccessFunctionsSI>
<\list>

<!-- IFDEF ONLINE ONLY END -->



&headerFix;

		<idx|responding to: help requests|
		<idx|requests, responding to help|
		<idx|application-specific|


When a user requests help while using your application, it's the
application's responsibility to determine what help topic should be
displayed.  

<procedure>Context \ Sensitivity

Some help requests amount to an explicit request for specific information,
such as help on "version" (which usually displays the copyright topic).
Other help requests, however, may require some degree of "context
sensitivity."  That is, some processing might be needed to determine the
appropriate help topic based on the user's current context within the
application.

For instance, your application might test the status of certain modes or
settings to determine the appropriate help topic.  Or, it might test the
value of an input field and provide detailed help if the value is not valid,
and general help if the value is valid.


<procedure>Entry \ Points

An ++entry point++ is a specific location within a help volume -- usually
the beginning of a topic -- that can be directly accessed by requesting
help within the application.

From the author's point of view, entry points are established by assigning
IDs at the appropriate places within the help volume.  From the
programmer's point of view, entry points are created by enabling the user
to request help and using the appropriate ID when a particular request is
made.

There are three general ways for users to request help:

<list bullet tight>

 * By pressing the ++help key++ (which is F1 on most keyboards).
 * By choosing the Help button in a dialog.
 * By choosing a command from the application's Help menu.

<\list>

<procedure>Displaying \ Topics

When a help request is made, the application determines what help topic
to display.  It then creates (if necessary) and manages a help
dialog, and sets the appropriate resources to display a help topic.

Most requests display help topics that are part of the application's help
volume.  But, the &HelpSystemName; System's help dialogs are also capable of
displaying man pages, text files, and simple text strings.




<s1 id=DisplayingHelpTopicsSI>Displaying Help Topics

<!-- IFDEF ONLINE ONLY -->
<!-- &DisplayingHelpTopicsSISubtopics; -->
<list bullet tight>
* <xref ToDisplayAStandardTopicTA>
* <xref ToDisplayAStringTA>
* <xref ToDisplayATextFileTA>
* <xref ToDisplayAManPageTA>
<\list>
<!-- IFDEF ONLINE ONLY END -->


		<idx|displaying: help topic|
		<idx|topics: displaying|
		<idx|``XmNhelpType''|
		<idx|resource: ``XmNhelpType''|
		<idx|``XmNhelpVolume''|
		<idx|resource: ``XmNhelpVolume''|
		<idx|``XmNlocationId''|
		<idx|resource: ``XmNlocationId''|
		<idx|``XmNstringData''|
		<idx|resource: ``XmNstringData''|
		<idx|``XmNhelpFile''|
		<idx|resource: ``XmNhelpFile''|
		<idx|``XmNmanPage''|
		<idx|resource: ``XmNmanPage''|
		<idx|``DT_HELP_TYPE_TOPIC''|
		<idx|``DT_HELP_TYPE_STRING''|
		<idx|``DT_HELP_TYPE_DYNAMIC_STRING''|
		<idx|``DT_HELP_TYPE_FILE''|
		<idx|``DT_HELP_TYPE_MAN_PAGE''|

The &HelpSystemName; System's help dialogs are based exclusively on
Xt Intrinsics and OSF/Motif programming, so you change the values within a
help dialog just like any other widget: by setting resources.

The ``XmNhelpType'' resource determines what type of information is
displayed.  It can be set to any of these values:

<list bullet loose>

* ``DT_HELP_TYPE_TOPIC'' for displaying normal help topics that are part of
  a help volume.  The volume is specified by setting the ``XmNhelpVolume''
  resource; the topic is specified by setting the ``XmNlocationId''
  resource.

* ``DT_HELP_TYPE_STRING'' for displaying a string supplied by the
  application.	Automatic word wrap is disabled, so line breaks are
  observed as specified in the string.	The string is specified by setting
  the ``XmNstringData'' resource.

* ``DT_HELP_TYPE_DYNAMIC_STRING'' for displaying a string supplied by the
  application, using word wrap to format the text.  Line breaks within the
  string are used to separate paragraphs.  The string is specified by
  setting the ``XmNstringData'' resource.

* ``DT_HELP_TYPE_FILE'' for displaying a text file.  The name of the file
  to be displayed is specified by setting the ``XmNhelpFile'' resource.

* ``DT_HELP_TYPE_MAN_PAGE'' for displaying a manual reference page ("man page")
  in a help dialog.  The man page to be displayed is specified by setting
  the ``XmNmanPage'' resource.

These values are defined in the ``Help.h'' file.

<\list>



<!-- seealsos/Hdisp.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ChapterCreatingAndManagingHelpDialogs> includes information on
   creating new dialogs and maintaining a dialog cache.	 
<\list>


<need begin>


<s2 id=ToDisplayAStandardTopicTA>To display a help topic

		<idx|displaying: help topic|
		<idx|topic: displaying|
		<idx|``XmNhelpType''|
		<idx|``XmNhelpVolume''|
		<idx|``XmNlocationId''|
		<idx|resource: ``XmNhelpType''|
		<idx|resource: ``XmNhelpVolume''|
		<idx|resource: ``XmNlocationId''|
		<idx|``DT_HELP_TYPE_TOPIC''|

<list order>

 * Create a help dialog or retrieve one from your dialog cache.

 * Set the following resources for the help dialog:

   <lablist width=1.5in loose>

    \ ``XmNhelpType'' \  Set to ``DT_HELP_TYPE_TOPIC''.

    \ ``XmNhelpVolume'' \ Set to the volume name for your application.

    \ ``XmNlocationId'' \ Set to the topic ID that you want to display.

   <\lablist>

   You can also set other values for the dialog, such as its size and
   title.

 * Manage the dialog using ``XtManageChild()''.

<\list>

<procedure>Example

This program segment displays a topic with the ID ``getting-started'' in
the volume ``MyVolume''.

<ex smaller>
ac = 0;
XtSetArg (al[ac], XmNhelpType,   DT_HELP_TYPE_TOPIC); ac+&empty;+;
XtSetArg (al[ac], XmNhelpVolume, "MyVolume");         ac+&empty;+;
XtSetArg (al[ac], XmNlocationId, "getting-started");  ac+&empty;+;
XtSetArg (al[ac], XmNcolumns,    40);                 ac+&empty;+;
XtSetArg (al[ac], XmNrows,       12);                 ac+&empty;+;
XtSetValues (helpDialog, al, ac);
XtManageChild (helpDialog);
<\ex>

If the help volume ``MyVolume'' is not registered, then a complete path
to the ``MyVolume.sdl'' file is required for the value of ``XmNhelpVolume''. 

<!-- seealsos/HdispTo.SA -->
<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ChapterCreatingAndManagingHelpDialogs> describes how to create
help dialogs and maintain a dialog cache.
 * <xref ChapterRegisteringHelpVolumes> describes how and why to register
your help volume.
 * <xref ToDisplayAStringTA>
 * <xref ToDisplayATextFileTA>
 * <xref ToDisplayAManPageTA>
 * <xref ToAddHelpCallbackTA>
<\list>

<need end>



<need begin>


<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToDisplayAStringTA>To display a string of text

		<idx|displaying: text string|
		<idx|text: string, displaying|
		<idx|string of text: displaying|
		<idx|``DT_HELP_TYPE_DYNAMIC_STRING''|
		<idx|``DT_HELP_TYPE_STRING''|

<list order>

 * Create a quick help dialog or retrieve one from your dialog cache.

   You can use a general help dialog to display string data, but this
   isn't recommended because most of its features do not apply to string
   data. 

 * Set the following resources for the help dialog:

   <lablist width=1.0in>

    \ ``XmNhelpType'' \  Set to ``DT_HELP_TYPE_DYNAMIC_STRING'' (if you
			 want wordwrap enabled) or ``DT_HELP_TYPE_STRING''
			 (if you want the line breaks within the string to
			 be maintained).

    \ ``XmNstringData'' \ Set to the string you want to display.  A copy
			  of the string is kept internally, so you need
			  not maintain your copy of it.

   <\lablist>

   You can also set other values for the dialog, such as its size and
   title.

 * Manage the dialog using ``XtManageChild()''.

<\list>

<procedure>Example

This program segment displays a string stored in the variable
``descriptionString''.

<ex smaller>
ac = 0;
XtSetArg (al[ac], XmNhelpType,   DT_HELP_TYPE_DYNAMIC_STRING); ac+&empty;+;
XtSetArg (al[ac], XmNstringData, (char *)descriptionString);   ac+&empty;+;
XtSetValues (quickHelpDialog, al, ac);
XtManageChild (quickHelpDialog);
<\ex>

If the string is no longer needed within the application, the memory can be
freed, because the help dialog makes its own copy of the data.

<ex smaller>
XtFree (descriptionString);
<\ex>


<!-- seealsos/HdispSt.SA -->
<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ChapterCreatingAndManagingHelpDialogs> describes how to create
help dialogs and maintain a dialog cache.
 * <xref ToDisplayAStandardTopicTA>
 * <xref ToDisplayATextFileTA>
 * <xref ToDisplayAManPageTA>
<\list>

<need end>



<need begin>


<!-- IFDEF ONLINE ONLY END -->


<s2 id=ToDisplayATextFileTA>To display a text file

		<idx|text: file, displaying|
		<idx|displaying: text file|
		<idx|file: displaying text|
		<idx|``DT_HELP_TYPE_FILE''|

<list order>

 * Create a quick help dialog or retrieve one from your dialog cache.

   You can use a general help dialog to display a text file, but this isn't
   recommended because most of its features are useful only for standard
   help topics.

 * Set the following resources for the help dialog:

   <lablist>

    \ ``XmNhelpType'' \  Set to ``DT_HELP_TYPE_FILE''.

    \ ``XmNhelpFile'' \  Set to the file name you want to display.  If the
			 file is not in the application's current working
			 directory, provide a path to the file.

   <\lablist>

   You can also set other values for the dialog, such as its size and
   title.  In particular, you might want to set the width to 80 columns,
   which is the standard width for text files.

 * Manage the dialog using ``XtManageChild()''.

<\list>




<procedure>Example

The following program segment displays a file named ``/tmp/printer.list''.
It also sets the size of the dialog to better suit a text file.

<ex smaller>
ac = 0;
XtSetArg (al[ac], XmNhelpType, DT_HELP_TYPE_FILE);    ac+&empty;+;
XtSetArg (al[ac], XmNhelpFile, "/tmp/printer.list");  ac+&empty;+;
XtSetArg (al[ac], XmNcolumns,  80);                   ac+&empty;+;
XtSetArg (al[ac], XmNrows,     20);                   ac+&empty;+;
XtSetValues (quickHelpDialog, al, ac);
XtManageChild (quickHelpDialog);
<\ex>
<!-- seealsos/HdispTx.SA -->
<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ChapterCreatingAndManagingHelpDialogs> describes how to create
help dialogs and maintain a dialog cache.
 * <xref ToDisplayAStandardTopicTA>
 * <xref ToDisplayAStringTA>
 * <xref ToDisplayAManPageTA>
<\list>

<need end>



<need begin>


<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToDisplayAManPageTA>To display a man page

		<idx|displaying: man page|
		<idx|man page: displaying|
		<idx|page, man: displaying|
		<idx|``DT_HELP_TYPE_MAN_PAGE''|

<list order>

 * Create a quick help dialog or retrieve one from your dialog cache.

   You can use a general help dialog to display a man page, but this
   isn't recommended because most of its features are useful only with
   standard help topics.

 * Set the following resources for the help dialog:

   <lablist>

    \ ``XmNhelpType'' \  Set to ``DT_HELP_TYPE_MAN_PAGE''.
 
    \ ``XmNmanPage'' \   Set to the name of the man page.  The value of
			 this resource is passed directly to the system ``man''
			 command.  So, to specify a particular section of
                         a man page, precede the man page name by a
			 section number, just as you would if you were
			 typing the ``man'' command conventionally.

   <\lablist>

   You can also set other values for the dialog, such as its size and
   title.

 * Manage the dialog using ``XtManageChild()''.

<\list>
<need end>

<procedure>Example

The following program segment displays the man page for the ``grep'' command.
It also sets the size of the dialog to better suit a man page.

<ex smaller>
ac = 0;
XtSetArg (al[ac], XmNhelpType, DT_HELP_TYPE_MAN_PAGE);  ac+&empty;+;
XtSetArg (al[ac], XmNmanPage,  "grep");            ac+&empty;+;
XtSetArg (al[ac], XmNcolumns,  80);                ac+&empty;+;
XtSetArg (al[ac], XmNrows,     20);                ac+&empty;+;
XtSetValues (quickHelpDialog, al, ac);
XtManageChild (quickHelpDialog);
<\ex>

<!-- seealsos/HdispMp.SA -->
<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ChapterCreatingAndManagingHelpDialogs> describes how to create
help dialogs and maintain a dialog cache.
 * <xref ToDisplayAStandardTopicTA>
 * <xref ToDisplayAStringTA>
 * <xref ToDisplayATextFileTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->



<s1 id=EnablingHelpKeyF1 > Enabling the Help Key (F1)

<!-- IFDEF ONLINE ONLY -->
<!-- &EnablingHelpKeyF1Subtopics; -->
<list bullet tight>
* <xref ToAddHelpCallbackTA>
<\list>


<!-- IFDEF ONLINE ONLY END -->

		<idx|enabling: help key (F1)|
		<idx|F1 (help key)|
		<idx|key, enabling help (F1)|


The <term nogloss|help key| mechanism is a feature built into all OSF/Motif
manager widgets and primitive widgets.  The help key is enabled by
adding a "help callback" to the widget where you want the help key active.

Within your application, you should add a help callback to each widget
where you want a unique entry point into help.  The help callback mechanism
automatically "walks" up the widget hierarchy (up to the shell widget) until
it finds a widget with a help callback, then invokes that callback.

If you add a help callback to a manager widget, when the help key is
pressed for any of its children, the manager's help callback is invoked
(unless the child widget has a help callback of its own).




<s2 id=ToAddHelpCallbackTA > To add a help callback

		<idx|adding: help callback|
		<idx|callback: adding help|
		<idx|``XtAddCallback()''|
		<idx|function: ``XtAddCallback()''|


<list bullet>

 * Use the ``XtAddCallback()'' function as follows:

<ex>
XtAddCallback (
     Widget          %%widget%%, 
     String          XmNhelpCallback, 
     XtCallbackProc  %%HelpRequestCB%%, 
     XtPointer       %%clientData%% );
<\ex>

Where:

<lablist width=1.25in>
\%%widget%%\		The widget where you want to activate 
			the help key.
\%%HelpRequestCB%%()\	The function in your application that
			handles the help request when the user presses the
			help key.
\%%clientData%%\	The data you want passed to the 
			%%HelpRequestCB%%() function.  Typically, 
			this data identifies the topic to
			be displayed.
<\lablist>

<\list>

When the user presses the help key, the help callback is invoked for the
widget with the current keyboard focus.  If that widget does not have a
help callback, the help callback for its nearest ancestor that does have a
help callback is invoked.

If no help callbacks are found, nothing happens.  Therefore, it is
recommended that you add a help callback to each shell in your application.
This ensures that no user requests for help are lost.

Adding a help callback to a dialog shell automatically enables
the Help button on the dialog to invoke the help callback.

<procedure>The Importance of Client Data

Specifying a unique value for %%clientData%% in each help callback you
add saves you the trouble of writing a separate function to
process each help callback.
Your application can have a single callback procedure to process all help
requests (referred to above as %%HelpRequestCB%%()).  Within that
procedure, use the %%clientData%% to identify where the user requested help.
That is, each time you add a help callback, you should provide a unique
value for %%clientData%%.


<procedure>Example

The following example demonstrates one way to associate IDs with entry points.
A ``Help.h'' file is used to define a unique integer for each 
%%clientData%% value for each help callback.  Also defined are two ID
strings for each widget:  one for normal F1 help, the other for "item
help mode" (where the user picks a widget to get a description).

For this example, we'll assume that the application's user interface is
just a main window with three input fields: Name, Address, and
Telephone Number.  Here's what the ``Help.h'' file would contain:

<ex smaller>
#define HELP_volumeName           "MyVolume"

#define HELP_MainWindow           100
#define HELP_MainWindow_ID        "basic-tasks"
#define HELP_MainWindow_ITEM_ID   "main-window-desc"

#define HELP_NameField            101
#define HELP_NameField_ID         "specifying-a-name"
#define HELP_NameField_ITEM_ID    "name-field-desc"

#define HELP_AddressField         102
#define HELP_AddressField_ID      "specifying-an-address"
#define HELP_AddressField_ITEM_ID "address-field-desc"

#define HELP_PhoneField           103
#define HELP_PhoneField_ID        "specifying-a-phone-no"
#define HELP_PhoneField_ITEM_ID   "phone-field-desc"
<\ex>

Within the part of the application that initially creates the widgets, a
help callback is added to each widget as follows:

<ex smaller>
XtAddCallback (mainWindow, XmNhelpCallback, 
               HelpRequestCB, HELP_MainWindow);
XtAddCallback (nameField, XmNhelpCallback, 
               HelpRequestCB, HELP_NameField);
XtAddCallback (addressField, XmNhelpCallback, 
               HelpRequestCB, HELP_AddressField);
XtAddCallback (phoneField, XmNhelpCallback, 
               HelpRequestCB, HELP_PhoneField);
<\ex>

Within the ``HelpRequestCB()'' function, the %%clientData%% parameter is
used to dispatch the help requests (via a ``switch()'' statement).  Within each
case, the value of a global flag ``itemHelp'' is tested to see if the
help callback was invoked by the F1 key (the flag is "false") or
by the user picking the widget in item help mode (the flag is "true").

<ex smaller>
XtCallbackProc HelpRequestCB (
     Widget     w,
     XtPointer  clientData,
     XtPointer  callData )
{
  char    *topicToDisplay;
  Boolean  useQuickHelpDialog;

  /* %%Determine the topic ID for the given `clientData.'%% */
  switch ((int)clientData)
    { 
      case HELP_MainWindow:
        useQuickHelpDialog = False;
        if (itemHelpFlag)
          topicToDisplay = HELP_MainWindow_ITEM_ID;
        else
          topicToDisplay = HELP_MainWindow_ID;
        break;

      case HELP_NameField:
        useQuickHelpDialog = True;
        if (itemHelpFlag)
          topicToDisplay = HELP_NameField_ITEM_ID;
        else
          topicToDisplay = HELP_NameField_ID;
        break;

      case HELP_AddressField:
        useQuickHelpDialog = True;
        if (itemHelpFlag)
          topicToDisplay = HELP_AddressField_ITEM_ID;
        else
          topicToDisplay = HELP_AddressField_ID;
        break;

      case HELP_PhoneField:
        useQuickHelpDialog = True;
        if (itemHelpFlag)
          topicToDisplay = HELP_PhoneField_ITEM_ID;
        else
          topicToDisplay = HELP_PhoneField_ID;
        break;

      default:
        /* %%An unknown clientData was received.%% */
        /* %%Put your error handling code here.%% */
        return;
        break;
    }

  /* %%Display the topic.%% */
  ac = 0;
  XtSetArg (al[ac], XmNhelpType,   DT_HELP_TYPE_TOPIC); ac+&empty;+;
  XtSetArg (al[ac], XmNhelpVolume, HELP_volumeName);    ac+&empty;+;
  XtSetArg (al[ac], XmNhelpType,   topicToDisplay);     ac+&empty;+;
  if (useQuickHelpDialog)
    {
       XtSetValues (mainQuickHelpDialog, al, ac);
       XtManageChild (mainQuickHelpDialog);
    }
  else
    {
       XtSetValues (mainHelpDialog, al, ac);
       XtManageChild (mainHelpDialog);
    }

  /* %%Clear the `item help' flag.%% */
  itemHelpFlag = False;
}
<\ex>

The above function assumes that the application uses two help dialogs for
all help requests (``mainHelpDialog'' and ``mainQuickHelpDialog''), and
that those dialogs have already been created.  It also assumes that ``al''
and ``ac'' (used in assembling Xt argument lists) are declared elsewhere.


<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hrqcalbk.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToAddSupportForItemHelpTA>
<\list>



<!-- IFDEF ONLINE ONLY END -->



<s1 id=ProvidingHelpMenu > Providing a Help Menu

		<idx|providing: help menu|
		<idx|menu: providing help menu|

The <book|OSF/Motif Style Guide| recommends that each menu bar include a
Help menu.  The Help menu may contain a variety of commands that let the
user access different types of online help for your application.

The most important commands include:

<list bullet>

 * <term nogloss|Introduction| displays the home topic of your
   application's help, allowing the user to use hyperlinks to navigate to
   any desired information.

 * <term nogloss|Using Help| displays "help on help."  This is
   information that tells the user how to use the help system.

 * <term nogloss|Version| displays your application's version and copyright
   information.	 The copyright topic (created using the ``&<copyright>''
   element), has the ID ``_copyright''.

<\list>

Additional commands may display help on special keyboard usage, application
tasks, reference, or tutorials.  You should design your help menu to best
suit your application, while staying within the guidelines and
recommendations of the <book|OSF/Motif Style Guide|.


<!-- seealsos/Hrqmenu.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateHomeTopicTA> describes how authors create the home topic for a
   help volume.
 * <xref ToCreateMetaInformationSectionTA> describes how authors create a
   copyright topic.
 * <xref ChapterProvidingHelpOnHelp> describes how "help on help" is found
   and how to add it to your application.
<\list>






<s1 id=SupportingItemHelpSI>Supporting Item Help Mode

<!-- &SupportingItemHelpSISubtopics; -->
<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
* <xref ToAddSupportForItemHelpTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->


		<idx|supporting item help mode|
		<idx|item help: mode|
		<idx|mode, item help|
		<idx|``DtHelpReturnSelectedWidgetId()''|
		<idx|function: ``DtHelpReturnSelectedWidgetId()''|

Some applications provide an "On Item" or "Help Mode" command in their Help
menu.  This command temporarily redefines the mouse pointer as a question
mark (``?'') to prompt the user to pick an item on the screen.  When an
item is picked, the application is expected to display a description of the
item.

The &HelpSystemName; System provides a convenience function,
``DtHelpReturnSelectedWidgetId()'', that changes the pointer to a question
mark and waits for the user to pick a widget.  The ID of the selected
widget is returned.  This function is similar to the ``XmTrackingLocate()''
function except that ``DtHelpReturnSelectedWidgetId()'' returns NULL if the
user presses Esc to cancel the operation.

To display help on the selected item, your application can simply invoke the
help callback for the returned widget.  This is equivalent to the
user pressing F1 while using that widget.  

If you want the application to differentiate between item help and F1 help,
you can set a flag before calling the widget's help callback.  The help
callback procedure can then use that flag to determine that the callback
was invoked as a result of item help and alter its response accordingly.





<s2 id=ToAddSupportForItemHelpTA > To add support for "item help"

		<idx|adding: support for item help|
		<idx|support for: item help|
		<idx|item help: adding support for|
		<idx|item help: invoking|
		<idx|``DtHelpReturnSelectedWidgetId()''|
		<idx|function: ``DtHelpReturnSelectedWidgetId()''|
		<idx|``HelpMode()''|
		<idx|function: ``HelpMode()''|

<list order>

 * Write a function that uses the ``DtHelpReturnSelectedWidgetId()'' function
   and lets the user pick a widget.  Within that function, invoke the help
   callback for the selected widget.  In the following steps, this function
   is called ``HelpMode()'', but you can name it whatever you want.

 * Add to your Help menu a command button labeled "On Item."
   Add an "activate" callback that invokes your ``HelpMode()'' function.

 * Add a help callback to each widget in your application where you want
   item help to be available. 

<\list>

If the selected widget does not have a help callback, the application should
try its
parent widget.  Similarly, if the parent does not have a help callback, the
application should continue to walk up the widget hierarchy until it finds a help
callback.

<!-- Richard: this memo may remain unresolved for this release -->
<memo>Team:  It seems that using XtCallActionProc() might be better than
XtCallCallbacks().  The "Help" action built into Motif should do the
hierarchy walks for us - right?  Could someone please look into this? <\memo>


<procedure>Example

The following procedure is a sample ``HelpModeCB()'' function that would be
invoked by choosing On Item from the Help menu.  

<need begin>

<ex smaller>
Boolean  HelpModeCB()
  {
     /* %%Declare a variable for the selected widget.%% */
     Widget  selectedWidget = (Widget)NULL;

     /* %%Let the user select a widget.%% */
     selectedWidget = DtHelpReturnSelectedWidgetId (%%topLevelShell%%);

     while (selectedWidget != (Widget)NULL)
        {
           /* %%If the selected widget has a help callback, invoke it and return `success.'%% */
           if ((XtHasCallbacks (selectedWidget, XmNhelpCallback) 
                               == XtCallbackHasSome))
              {
                 itemHelpFlag = True;
                 XtCallCallbacks (selectedWidget, XmNhelpCallback,
                                 (XtPointer)NULL);
                 return (True);
              }
           /* %%Otherwise, try the widget's parent.%% */
           else
              selectedWidget = XtParent (selectedWidget);
        }
     /* %%No help callback was found, return `failure.'%% */
     return (False);
  }
<\ex>

<need end>






<s1 id=UsingTopicAccessFunctionsSI > Using the Topic Access Functions

		<idx|using: topic access function|
		<idx|topic: access function, using|
		<idx|access function, topic|
		<idx|``DtHelpGetTopicData()''|
		<idx|``DtHelpProcessLinkData()''|
		<idx|``DtHelpFreeTopicData()''|
		<idx|function: ``DtHelpGetTopicData()''|
		<idx|function: ``DtHelpProcessLinkData()''|
		<idx|function: ``DtHelpFreeTopicData()''|


The &HelpSystemName; System programming toolkit provides three functions for
retrieving the text within help topics.
These functions are intended for use in character-based
applications running on a terminal or in a terminal emulator window.  It's
up to the application to control the user interface to the help
information.

Here are descriptions of the functions:

<list bullet>

 * ``DtHelpGetTopicData()'' retrieves help text from a help volume, based on the
   volume file location and an ID.  The topic's text is returned in a list
   of lines, formatted to a given column width.  Information about the
   hyperlinks that occur within the topic is also returned.

 * ``DtHelpProcessLinkData()'' processes a particular hyperlink, returning the
   destination volume and ID (which can, in turn, be processed by
   ``DtHelpGetTopicData()'').

 * ``DtHelpFreeTopicData()'' frees the data returned by ``DtHelpGetTopicData()''.

<\list>

Graphics and special characters are ignored by the topic access functions.

<!-- Richard:  this memo may remain unresolved for this release -->
<memo>We need an example here!<\memo>


<!-- seealsos/Htafunc.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref ChapterAPISummary> includes a more detailed description of each
   function. 
<\list>


