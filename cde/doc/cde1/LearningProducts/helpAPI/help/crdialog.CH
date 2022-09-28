<chapter id=ChapterCreatingAndManagingHelpDialogs > Creating and Managing Help Dialogs

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
 * <xref QuickHelpDialogDthelpquickdialogSI>
 * <xref GeneralHelpDialogDthelpdialogSI>
 * <xref CreatingDialogCacheSI>
<\list>
<!-- IFDEF ONLINE ONLY END -->



&headerFix;

		<idx|creating: help dialog|
		<idx|managing help dialogs|
		<idx|dialog: creating and managing|
		<idx|widget resources and classes|
		<idx|resources, widget|
		<idx|classes, widget|
		<idx|dialog: quick help|
		<idx|quick help dialog|
		<idx|dialog: general help|
		<idx|general help dialog|


For application programmers, the &ProductName; provides a programming
library that adds help dialogs to any OSF/Motif application.  The library
provides two types of help dialogs:

<list bullet>

 * <term nogloss|General help dialogs| have a menu bar, a Topic Hierarchy
   (that tells you where you are), and a help topic display area. 

 * <term nogloss|Quick help dialogs| have just a topic display area and one
   or more dialog buttons at the bottom.

<\list>


<newpage>

<procedure>The Quick Help Dialog

The quick help dialog is designed to help you meet the first objective of
online help: !!"Get the user back on task as quickly and successfully as
possible."!!  

The quick help dialog, which never has more than five buttons, has a simple
user interface, which helps keep the user focused on the information.
Hopefully the information is useful enough that the user dismisses the
dialog after reading it and continues working.


<procedure>The General Help Dialog


The general help dialog has a few user interface features beyond the
features of the Quick Help Dialog.  Most notably, the Topic Hierarchy
list, which appears just above the help topic display area, provides
a table of contents of all the topics and subtopics in the help volume.
The home topic is always the first title in the table of contents.  An
arrow ( -> ) marks the current topic's title and indicates the location
of the current topic within the hierarchy.


<procedure>Standard Xt Paradigm

In terms of programming, you interact with the help dialogs the same as
you do with any other OSF/Motif widgets in your applications.  The two
types of help dialogs are
defined as two new widget classes:  DtQuickHelpDialogWidgetClass and
DtHelpDialogWidgetClass.

Nearly every attribute of the help windows -- including the volume name and
topic ID -- are manipulated as widget resources.  For instance, to display
a new topic, you just execute an ``XtSetValues()'' call to set the
``XmNhelpVolume'', ``XmNlocationId'', and ``XmNhelpType'' resources.
For more information, refer to <xref DisplayingHelpTopicsSI>.


<note><head>Prerequisite Knowledge

  Integrating the &ProductName; into an application requires a working
  knowledge of the the C programming language, the OSF/Motif programmer's
  toolkit, and the Xt Intrinsics toolkit.

<\note>



<s1 id=QuickHelpDialogDthelpquickdialogSI > The Quick Help Dialog

<!-- IFDEF ONLINE ONLY -->
<!-- &QuickHelpDialogDthelpquickdialogSISubtopics; -->
<list bullet tight>
* <xref ToCreateQuickHelpDialogTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->
		<idx|quick help dialog|
		<idx|dialog: quick help|
		<idx|``DtHelpQuickDialog()''|

<figure nonumber entity=HPHelpKit-QuickHelpDialog>
<\figure>

Within a quick help dialog, users can get to more help using hyperlinks
within the displayed topic.

One of the dialog's buttons is application-defined, so
this button can be used for anything.  However,
its intended purpose is to provide a path to more help in one of these two
ways: 

<!-- a version of this text is also in "enabling the app-def. button." -->


<list bullet>

 * Let the user ask for more detailed information.  In this
   case, the default button label ("More") is appropriate.
   This is called "progressive disclosure."

 * !!Or,!! let the user open a general help dialog for general browsing of
   the application's help volume.  In this case, "Browse..." is the most
   appropriate button label.

<\list>

The &HelpSystemName; programmer's toolkit includes a convenience function for
determining the widget ID for any of the quick help dialog buttons.


<newpage>




<s2 id=ToCreateQuickHelpDialogTA > To create a quick help dialog

		<idx|creating: quick help dialog|
		<idx|quick help dialog: creating|
		<idx|dialog: creating quick help|
		<idx|``DtCreateQuickHelpDialog()''|
		<idx|creating: help dialog|
		<idx|help on help: quick help dialogs|
		<idx|hyperlink: callback|
		<idx|callback: hyperlink|
		<idx|``MoreHelpCB()''|
		<idx|``HyperlinkCB()''|
		<idx|``CloseHelpCB()''|

<list order>

 * Include the appropriate header files:

<ex>
#include Help.h
#include HelpQuickD.h
<\ex>

 * Create an instance of the quick help dialog widget:

   <list bullet tight>
    * Use the ``DtCreateQuickHelpDialog()'' convenience function.
    * !!Or,!! use the ``XtCreateManagedWidget()'' function.
   <\list>

 * Add a callback for handling hyperlink events that occur within the
   dialog.  (This is described in more detail in <xref
   RespondingToHyperlinkEventsSI>.) 

 * Add a ++close callback++ for handling the OK button.  

 * Configure the dialog buttons that you want to use:

   <list bullet tight>
     * If you intend to use the application-defined button, manage it and
       add an activate callback.
     * If you want to disallow printing, unmanage the Print button.
     * Manage the Help button and add a ++help callback++ to the
       dialog to allow the user to get ++help on help++.
   <\list>

<\list>


<need begin>

<procedure>Example

The following code segment creates a quick help dialog (as a child of
%%parent%%) using the convenience function.  The dialog is left unmanaged;
presumably it is managed elsewhere in the application when a help request
is made.  In this example, the application-defined button is enabled and
used to request "more" help.

		<idx|help on help|

<ex smaller>
Widget   quickHelpDialog, moreButton, helpButton;

ac = 0;
XtSetArg (al[ac], XmNtitle, "My Application - Help");  ac+&empty;+;
quickHelpDialog = 
  DtCreateQuickHelpDialog (%%parent%%, "quickHelpDialog", al, ac);
<\ex>

The following two calls add the hyperlink and close callbacks to the dialog.
Presumably, the functions ``HyperlinkCB()'' and ``CloseHelpCB()'' are
declared elsewhere in the application.

<ex smaller>
XtAddCallback (quickHelpDialog, XmNhyperLinkCallback, 
               HyperlinkCB, (XtPointer)NULL);
XtAddCallback (quickHelpDialog, XmNcloseCallback, 
               CloseHelpCB, (XtPointer)NULL);
<\ex>

Here, the application-defined button is managed and assigned an activate
callback that invokes the application's ``MoreHelpCB()'' function.

<ex smaller>
moreButton = DtHelpQuickDialogGetChild (quickHelpDialog,
                                     DT_HELP_QUICK_MORE_BUTTON);
XtManageChild (moreButton);
XtAddCallback (moreButton, XmNactivateCallback, 
               MoreHelpCB, (XtPointer)NULL);
<\ex>

To provide "help on help," the dialog's Help button is managed and a help
callback is added to the dialog. 

<ex smaller>
helpButton = DtHelpQuickDialogGetChild (quickHelpDialog, 
                                     DT_HELP_QUICK_HELP_BUTTON);
XtManageChild (helpButton);
XtAddCallback (quickHelpDialog, XmNhelpCallback, 
               HelpRequestCB, USING_HELP);
<\ex>

Like other OSF/Motif dialogs, when you add a help callback to a quick
help dialog, it is used by both the F1 key and the Help button.

<need end>

<procedure>See Also

<list bullet tight>
 * <xref ToEnableApplicationDefinedButtonTA>
 * <xref ToAddHelpOnHelpForQuickHelpTA>
<\list>






<s1 id=GeneralHelpDialogDthelpdialogSI> The General Help Dialog

<!-- IFDEF ONLINE ONLY -->
<!-- &GeneralHelpDialogDthelpdialogSISubtopics; -->
<list bullet tight>
* <xref ToCreateGeneralHelpDialogTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->


		<idx|``DtHelpDialog()''|
		<idx|general help dialog|
		<idx|dialog: general help|

<figure nonumber entity=HPHelpKit-GeneralHelpDialog>
<\figure>

The menu bar contains several commands, including commands that display
these additional dialogs:

<list bullet>

 * Choose Print from the File menu to display the <term nogloss|Print|
   dialog.  This dialog lets the user choose which topics are to be printed:
   "All," "Current," and "Current and Down."  (Current and Down prints the
   current topic and all of its subtopics.)

 * Choose Keyword from the Search menu to display the <term nogloss|Keyword
   Index| dialog.  This dialog lists all the words and phrases that the
   author has marked as index entries.	Selecting a keyword, then one of
   the topics where the keyword occurs, displays that topic in the general help
   dialog.

 * Choose History from the Search menu to display the
   <term nogloss|History| dialog.  This
   dialog lists the topic titles for each topic the user has visited.  To
   return to any topic in the list, select its title.

 * Choose Using Help from the Help menu to display the <term nogloss|Help
   On Help| dialog.  This help information describes how to use the help
   system itself.

<\list>


<need begin>




<s2 id=ToCreateGeneralHelpDialogTA > To create a general help dialog

		<idx|creating: general help dialog|
		<idx|general help dialog: creating|
		<idx|dialog: creating general help|

<list order>

 * Include the appropriate header files:

<ex>
#include Help.h
#include HelpDialog.h
<\ex>


 * Create an instance of the general help dialog widget:

   <list bullet tight>
    * Use the ``DtCreateHelpDialog()'' convenience function.
    * !!Or,!! use the ``XtCreateManagedWidget()'' function.
   <\list>

 * Add a callback for handling hyperlink events that occur within the
   dialog.  (This is described in more detail in <xref
   RespondingToHyperlinkEventsSI>.) 

 * Add a ++close callback++ for handling the Close command.  

<\list>

<need end>

<procedure>Example

The following code segment creates a general help dialog (as a child of
%%parent%%) using the convenience function.  The dialog is left unmanaged
-- presumably it is managed elsewhere in the application when a help
request is made.

<ex smaller>
Widget   mainHelpDialog, moreButton, helpButton;

ac = 0;
XtSetArg (al[ac], XmNtitle, "My Application - Help");  ac+&empty;+;
mainHelpDialog = 
  DtCreateHelpDialog (%%parent%%, "mainHelpDialog", al, ac);
<\ex>

The following two calls add the hyperlink and close callbacks to the dialog.
Presumably, the functions ``HyperlinkCB()'' and ``CloseHelpCB()'' are
declared elsewhere in the application.

<ex smaller>
XtAddCallback (mainHelpDialog, XmNhyperLinkCallback, 
               HyperlinkCB, (XtPointer)NULL);
XtAddCallback (mainHelpDialog, XmNcloseCallback, 
               CloseHelpCB, (XtPointer)NULL);
<\ex>



<procedure>See Also

<list bullet tight>
 * <xref ChapterProvidingHelpOnHelp>
 * <xref ToEnableApplicationDefinedButtonTA>
<\list>




<s1 id=CreatingDialogCacheSI > Creating a Dialog Cache

<!-- IFDEF ONLINE ONLY -->
<!-- &CreatingDialogCacheSISubtopics; -->
<list bullet tight>
* <xref ToCreateADialogCacheTA>
* <xref ToRetrieveADialogFromYourCacheTA>
* <xref ToReturnADialogToYourCacheTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->

		<idx|creating: dialog cache|
		<idx|dialog cache: creating|
		<idx|cache, dialog: creating|

Because authors can create hyperlinks that request a new window, your
application must be able to create an arbitrary number of help windows.
But, creating and destroying widgets as they are needed can cause your
application to run slower.
So, to optimize performance and make efficient use of resources, caching help
dialogs is recommended.

A <term nogloss|dialog cache| is a list of
the help dialogs that your application has already created.  When
the user dismisses a dialog, the widget stays in the cache instead of
being destroyed.

The next time the user requests help that would otherwise require a new
widget, your application can scan the cache
list looking for a dialog that isn't currently being used.

<!--
So the user's first request for help is quick, you may want to create one
or two dialogs during application startup time.
-->



<s2 id=ToCreateADialogCacheTA > To create a dialog cache

		<idx|creating: dialog cache|
		<idx|dialog cache: creating|
		<idx|cache, dialog: creating|


<list order>

  * Declare a structure that you can use to create a linked list
    of dialogs.  Typically, a help dialog cache structure has
    these attributes:

    <list bullet tight>
      * Keeps track of the Widget ID for each help dialog.
      * Maintains a flag that indicates which type of help dialog
        it is (quick help or general help).
      * Maintains an "in use" flag.
    <\list>

  * When you create a help dialog, be sure to add it to your cache.

  * When the user closes a help dialog, return the dialog to your
    cache by clearing its "in use" flag, then unmanage it.

<\list>

<need begin>

<procedure>Example

The following type definition demonstrates a simple structure
that you could use to build a dialog cache.   Instances of this
structure would be connected (via the ``next'' element) to form
a linked list.

<ex smaller>
typedef struct _HelpDialogCacheStruct {
     Widget   dialog;                         /* %%The dialog's handle.%% */
     Boolean  inUse;                          /* %%The `in use' flag.%% */
     Boolean  isQuickHelpDialog;              /* %%The dialog type flag.%% */
     struct   _HelpDialogCacheStruct  *next;  /* %%Next in the list.%% */
}  HelpDialogCacheStruct;
<\ex>

<need end>

<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hdicrcac.SA -->

<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToRetrieveADialogFromYourCacheTA>
 * <xref ToReturnADialogToYourCacheTA>
<\list>



<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToRetrieveADialogFromYourCacheTA > To retrieve a dialog from your cache

		<idx|retrieving: dialog from cache|
		<idx|dialog cache: retrieving dialog|
		<idx|cache, dialog: retrieving dialog|


<list bullet>
 * Scan the cache looking for a dialog of the correct type whose
   "in use" flag is false.

   If there are no dialogs available in the cache, create a new one, add
   it to the cache (marked "in use") and use it.
<\list>

<procedure>Example

The following ``FetchHelpDialog()'' function scans a dialog cache for a
help dialog of the specified type and returns a Widget ID.  If an unused
dialog is not found, one is created, added to the cache and returned.  (If
you are viewing this example online, you can copy and paste this example
directly into your source code to save the trouble of typing it.)

<ex smaller>
Widget
FetchHelpDialog(Boolean  lookingForQuickHelpDialog)
  {
     /* %%Declare a local pointer for walking the cache.%% */
     HelpDialogCacheStruct *pCacheStruct;

     /* %%Declare another, in case we need to add a new item to the cache.%% */
     HelpDialogCacheStruct *pNewCacheStruct;

     /* %%Set the local pointer to the first item in the cache list.%% */
     pCacheStruct = pFirstHelpDialogCacheStruct;

     /* %%Scan the cache for an unused help dialog.%% */
     while (pCacheStruct != (_HelpDialogCacheStruct)NULL)
       {
          /* %%Is this dialog available?%% */
          if (  (pCacheStruct->inUse == False)
                  /* %%And, is it the correct type?%% */
              &&&&  (  (   lookingForQuickHelpDialog
                      &&&& pCacheStruct->isQuickHelpDialog)
                  || (   !lookingForQuickHelpDialog
                      &&&& !pCacheStruct->isQuickHelpDialog)))
            {
               /* %%Yes!  This is a match.%% */
               pCacheStruct->inUse = True;
               return ((Widget)pCacheStruct->dialog);
            }
          else
            {
               /* %%Nope.  Go on to the next item.%% */
               pCacheStruct = pCacheStruct->next;
            }
       }

     /* %%Searching the cache was unsuccessful.%%*/

     /* %%Create a new item in the cache.%% */
     pNewCacheStruct = (HelpDialogCacheStruct *)
          XtMalloc((sizeof(HelpDialogCacheStruct)));

     /* %%Fill in the new structure.%% */
     pNewCacheStruct->inUse = True;
     pNewCacheStruct->isQuickHelpDialog = 
          lookingForQuickHelpDialog;
     pNewCacheStruct->next = 
          pFirstHelpDialogCacheStruct->next;
     pFirstHelpDialogCacheStruct = pNewCacheStruct;

     /* %%Create the new help dialog widget.%% */
     ac = 0;
     XtSetArg (al[ac], XmNtitle, "My Application - Help"); ac+&empty;+;
     if (lookingForQuickHelpDialog)
       {
          pNewCacheStruct->dialog = DtCreateQuickHelpDialog 
               (topLevel, "quickHelpDialog", al, ac);
       }
     else
       {
          pNewCacheStruct->dialog = DtCreateHelpDialog 
               (topLevel, "helpDialog", al, ac);
       }

     /* %%Done.  Return the new dialog.%% */
     return ((Widget)pNewCacheStruct->dialog);
  }
<\ex>

<need begin>

The example above assumes the following:

<list bullet tight>
 * That the pointer to the head of the cache (``pFirstHelpDialogCacheStruct'')
   is initialized to NULL during application start up. 
 * That the calling routine tests the return value to determine if a valid
   widget ID is returned.
 * That the parent for all help dialogs is a widget named ``topLevel''.
 * That the variables ``al'' and ``ac'' (used in Xt argument lists)
   are declared external to this function.
 * The help volume name is "MyApplication."
<\list>

<need end>

<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hdigetca.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateADialogCacheTA>
 * <xref ToReturnADialogToYourCacheTA>
<\list>




<!-- IFDEF ONLINE ONLY END -->



<s2 id=ToReturnADialogToYourCacheTA > To return a dialog to your cache

<need begin>

		<idx|returning, help dialog to cache|
		<idx|dialog cache: returning dialog|
		<idx|cache, dialog: returning dialog|


<list bullet>

 * Unmanage the dialog and clear the dialog's "in use" flag.

<\list>

<procedure>Example

The following function is called when a help dialog is closed (via the close
callback):

<ex smaller>
Boolean
HelpCloseCB (
     Widget     closedDialog,
     XtPointer  clientData,
     XtPointer  callData )
  {
    /* %%Declare a local pointer for walking the cache.%% */
    HelpDialogCacheStruct *pCacheStruct;

    /* %%Search the cache list for dialog.%% */
    pCacheStruct = pFirstHelpDialogCacheStruct;

    while (pCacheStruct != (HelpDialogCacheStruct *)NULL)
      {
        if (pCacheStruct->dialog == closedDialog)
          break;
        pCacheStruct = pCacheStruct->next;
      }

    /* %%Unmanage the dialog.%% */
    XtUnmanageChild(closedDialog);

    /* %%If the dialog wasn't found, the cache is corrupt.  Return failure.%% */
    if (pCacheStruct == (HelpDialogCacheStruct *)NULL)
      return (False);

    /* %%Mark the dialog unused, then return success.%% */
    pCacheStruct->inUse = False;
    return (True);
  }
<\ex>

<need end>


<!-- IFDEF ONLINE ONLY -->
<!-- seealsos/Hdiretca.SA -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref ToCreateADialogCacheTA>
 * <xref ToRetrieveADialogFromYourCacheTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->




