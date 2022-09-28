<chapter id=ChapterHandlingEventsInHelpDialogs > Handling Events \ in Help Dialogs

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
 * <xref RespondingToHyperlinkEventsSI>
 * <xref DetectingWhenHelpDialogsAreDismissed>
 * <xref UsingApplicationDefinedButton>
<\list>
<!-- IFDEF ONLINE ONLY END -->



&headerFix;

		<idx|handling event in help dialog|
		<idx|event: in help dialog, handling|
		<idx|dialog: handling event in|
		<idx|application-specific|

Like most other widgets within your application, help windows have some
behavior that must be supported by the application.

<procedure>Hyperlink Events

Most standard hyperlink events are handled internally by the &ProductName;.  However, there are three types of hyperlinks that your application
is responsible for handling:

<list bullet>

 * <term "Jump-new-view hyperlink"|Jump-new-view hyperlinks| -- Your
   application must create the new
   help dialog (or get one from your cache) to honor the author's request
   for a "new view."

 * <term "Man page link"|Man page links| -- Your application must create
   a new quick help
   dialog (or get one from your cache) to display a man page.  Typically,
   the size of man page windows is different than all other help windows.

 * <term "Application-defined link"|Application-defined links| -- Your
   application must interpret the
   data associated with these links.  Application-defined links exist only
   if you and the author have collaborated to create them.

<\list>

<procedure>When Dialogs Are Dismissed

When the user closes a help dialog, your application needs to know so it
can return the dialog to its cache, or destroy it.  The general help
dialog supports a "help closed" callback.  To detect when a general
help dialog is dismissed, add a callback to its OK button.


<procedure>Quick Help Buttons

The behavior for some of the buttons in quick help dialogs
must be handled by your
application.  These buttons can be managed and unmanaged as needed.  You
add behavior just like any other push button: using an "activate
callback."


<procedure>See Also

<list bullet tight>

 * <xref CreatingHyperlinksSI> describes the types of links supported by
   the &ProductName; and explains how to create them.

<\list>







<s1 id=RespondingToHyperlinkEventsSI > Responding to Hyperlink Events

<!-- IFDEF ONLINE ONLY -->
<!-- &RespondingToHyperlinkEventsSISubtopics; -->
<list bullet tight>
* <xref ToProvideHyperlinkCallbackTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->


		<idx|responding to: hyperlink event|
		<idx|hyperlink: event, responding to|
		<idx|event: hyperlink, responding to|


Your application needs to provide support only for the types of hyperlinks
used within the help volume to be displayed.  In general, it is recommended
that you provide support for all link types. 

For your application to be notified when a hyperlink is chosen, it must add
a "hyperlink callback" to the help dialog.  You must write a callback
function that handles the hyperlink appropriately.





<s2 id=ToProvideHyperlinkCallbackTA > To provide a hyperlink callback

		<idx|providing: hyperlink callback|
		<idx|hyperlink: callback, providing|
		<idx|callback: hyperlink, providing|
		<idx|``XtAddCallback''|
		<idx|function: ``XtAddCallback()''|


<list order>

 * Add a hyperlink callback to each help dialog as shown:

<ex>
XtAddCallback (%%helpDialog%%, XmNhyperLinkCallback, 
               %%HyperlinkCB%%, (XtPointer)NULL);
<\ex>

   Where %%helpDialog%% is the widget ID of the help dialog and
   %%HyperlinkCB%% is the name of the callback function for handling
   hyperlinks.

 * Write the %%HyperlinkCB%% function to handle the hyperlink 
   events that can occur within the dialog.

<\list>

Within the hyperlink callback, you have access to the following
callback structure (which is declared in ``&<Dt/Help.h>''):

		<idx|``DtHelpDialogCallbackStruct''|
		<idx|structure: ``DtHelpDialogCallbackStruct''|

<ex>
typedef struct
  {
     int      reason;
     XEvent  *event;
     char    *locationId;
     char    *helpVolume;
     char    *specification;
     int      hyperType;
  } DtHelpDialogCallbackStruct;
<\ex>

The ``hyperType'' element indicates which type of link was executed.  Its
possible values are ``DT_HELP_LINK_JUMP_NEW'', ``DT_HELP_LINK_MAN_PAGE'', and
``DT_HELP_LINK_APP_DEFINED''. 

<!-- This issue is still unresolved (7/29/92) -->
<memo>Mike:  The ES isn't clear about which elements are valid for various
types of links.  The ES seems to indicate that the `locationId' and
`helpVolume' elements are for the topic currently being displayed -- is
that true, or are they the destination for a jump-new-view link?  Also, what are the possible values for `reason'?<\memo>


<procedure>Example

The following function, ``HyperlinkCB()'', illustrates the general
structure needed to handle hyperlink callbacks.  

<ex smaller>
XtCallbackProc
HyperlinkCB (widget, clientData, callData)
     Widget     widget;
     XtPointer  clientData;
     XtPointer  callData;
  {
     DtHelpDialogCallbackStruct *hyperData = 
        (DtHelpDialogCallbackStruct *) callData;

     switch ((int)hyperData->hyperType)
        {
           case DT_HELP_LINK_JUMP_NEW:
             /* %%Handles "jump new view" hyperlinks.%% */
             break;

           case DT_HELP_LINK_MAN_PAGE:
             /* %%Handles "man page" hyperlinks.%% */
             break;

           case DT_HELP_LINK_APP_DEFINED:
             /* %%Handles "application-defined" hyperlinks.%% */
             break;

           default:
             break;
   }
<\ex>





<s1 id=DetectingWhenHelpDialogsAreDismissed > Detecting When Help Dialogs are Dismissed
		<idx|detecting when help dialog dismissed|
		<idx|dialog: detecting when dismissed|
		<idx|``XtAddCallback''|
		<idx|function: ``XtAddCallback()''|

To detect when a general help dialog is closed, add the following callback to the
dialog:

<ex>
XtAddCallback (%%helpDialog%%, XmNcloseCallback, 
               %%HelpCloseCB%%, (XtPointer)NULL);
<\ex>

Where %%helpDialog%% is the widget ID for the help dialog and
%%HelpCloseCB%% is the name of the callback procedure you've written to
handle closing dialogs.

To detect when a quick help dialog is closed, add the following callback to the
dialog's OK button:

<ex>
XtAddCallback (DtHelpQuickDialogGetChild (%%helpDialog%%, 
                                       DT_HELP_QUICK_OK_BUTTON),
               XmNactivateCallback, %%HelpCloseCB%%, (XtPointer)NULL);
<\ex>

Where %%helpDialog%% is the widget ID for the help dialog and
%%HelpCloseCB%% is the name of the callback procedure you've written to
handle closing dialogs.

A sample %%HelpCloseCB%% function is included in <xref
ToReturnADialogToYourCacheTA>.





<s1 id=UsingApplicationDefinedButton > Using the Application-Defined Button

<!-- IFDEF ONLINE ONLY -->
<!-- &UsingApplicationDefinedButtonSubtopics; -->
<list bullet tight>
* <xref ToEnableApplicationDefinedButtonTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->

		<idx|using: application-defined button|
		<idx|application-defined: button|
		<idx|button, application-defined: using|

The quick help dialog's application-defined button lets you add custom
behavior to any quick help dialog.  This button can be used for anything
you want, but its intended purpose is to provide a path to more help in one
of these two ways:

<list bullet>

 * Lets the user progressively ask for more information.  This is sometimes
   called "progressive disclosure."  In this
   case, the default button label ("More") is appropriate.

 * !!Or,!! lets the user open a general help dialog for general browsing of
   the application's help volume.  In this case, "Browse..." is the most
   appropriate button label.

<\list>

<!-- a version of this text is also in "creating help dialogs." -->





<s2 id=ToEnableApplicationDefinedButtonTA > To enable the application-defined button
		<idx|enabling: application-defined button|
		<idx|application-defined: button, enabling|
		<idx|button, application-defined: enabling|

<list order>

 * Get the button's ID.

 * Add an activate callback to the button.

 * Manage the button.

<\list>

<procedure>Example

The following code segment gets the button's ID, assigns a callback,
and manages the button.  It assumes that ``quickHelpDialog'' was just created.

<ex smaller>
Widget  moreButton;

moreButton = DtHelpQuickDialogGetChild (quickHelpDialog,
                                     DT_HELP_QUICK_MORE_BUTTON);
XtAddCallback (moreButton, XmNactivateCallback,
               MoreHelpCB, NULL);

XtManageChild (moreButton);
<\ex>



<procedure>See Also

<list bullet tight>
 * <xref ToCreateQuickHelpDialogTA>
<\list>

