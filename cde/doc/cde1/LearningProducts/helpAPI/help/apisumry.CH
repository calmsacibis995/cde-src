<chapter id=ChapterAPISummary> Summary of Application\Programmers Interface


<!-- IFDEF ONLINE ONLY -->
<!-- don't add a list of links here, the chapter intro uses xrefs so
     the links to the function summaries are already provided -->
<!-- IFDEF ONLINE ONLY END -->


&headerFix;
		<idx|API summary|

The &HelpSystemName; System's Application Programmers Interface (API) includes the
following functions:

<list bullet>
 * Functions for creating and working with help dialogs:

<list plain tight>
 * <xref DtCreateHelpDialog>
 * <xref DtCreateQuickHelpDialog>
 * <xref DtHelpQuickDialogGetChild>
<\list>

 * Function for implementing item help mode:

<list plain tight>
 * <xref DtHelpReturnSelectedWidgetId>
<\list>

 * Functions for working directly with help text:

<list plain tight>
 * <xref DtHelpGetTopicData>
 * <xref DtHelpProcessLinkData>
 * <xref DtHelpFreeTopicData>
<\list>

 * Function for specifying the message catalog for the DtHelp library:
			<idx|message catalog|

<list plain tight>
 * <xref DtHelpSetCatalogName>
<\list>
<\list>


<s1 id=DtCreateHelpDialog>``DtCreateHelpDialog()''

		<idx|``DtCreateHelpDialog()''|
		<idx|function: ``DtCreateHelpDialog()''|


<ex>
#include &<Dt/Help.h>
#include &<Dt/HelpDialog.h>

Widget DtCreateHelpDialog (
     Widget    %%parent%%,
     String    %%name%%,
     ArgList   %%arglist%%,
     Cardinal  %%argcount%% );
<\ex>


Where:

<lablist>
\%%parent%%\  Specifies the parent widget ID.

\%%name%%\  Specifies the name of the new help dialog.

\%%arglist%%\  Specifies the argument list.

\%%argcount%%\  Specifies the number of attribute-value
pairs in the argument list (%%arglist%%).
<\lablist>


This function creates a new instance of a general help dialog and returns
its ID.  The widget ID returned is a Dialog Shell widget which serves as
the top level child in the created help dialog.  Refer to the OSF/Motif
documentation for more information on the Dialog Shell widget and
applicable resources.

<procedure> Resources

The following resources are specific to the help dialog widget.

``XmNcloseCallback''
		<idx|``XmNcloseCallback''|
		<idx|resource: ``XmNcloseCallback''|

<p indent> Specifies the list of callback functions executed when the user
chooses Close from the dialog's File menu.  (See <xref
DetectingWhenHelpDialogsAreDismissed>.)

``XmNcolumns''
		<idx|``XmNcolumns''|
		<idx|resource: ``XmNcolumns''|

<p indent> Specifies the desired width of the help display area.  The
``XmNcolumns'' resource expects a number (type ``Dimension'') value that
represents the number of average-width characters (based on the current
font).  The default is ``40''.

``XmNhelpFile''
		<idx|``XmNhelpFile''|
		<idx|resource: ``XmNhelpFile''|

<p indent> Specifies the name of a text file to be displayed in the
help dialog. This resource is used only when the ``XmNhelpType'' resource
is set to ``DT_HELP_TYPE_FILE''.

<p indent>The ``XmNhelpFile'' resource expects a string (``char *'')
value.  Its default is the NULL string.


``XmNhelpOnHelpVolume''
		<idx|``XmNhelpOnHelpVolume''|
		<idx|resource: ``XmNhelpOnHelpVolume''|

<p indent> Specifies the help volume used to display "help on help."  This
volume is accessed if the user requests help while using the help dialog.

<p indent>The ``XmNhelpOnHelpVolume'' resource expects a string (``char *'')
value.  Its default is the NULL string.

``XmNhelpPrint''
		<idx|``XmNhelpPrint''|
		<idx|resource: ``XmNhelpPrint''|

<p indent> Specifies the command used to print help topics.  The
``XmNhelpPrint'' resource expects a string (``char *'') value.  Its default
is ``/opt/dt/bin/dthelpprint''.

``XmNhelpType''
		<idx|``XmNhelpType''|
		<idx|resource: ``XmNhelpType''|

<p indent> Specifies the type of information to be displayed in the help
dialog.  Valid values, which are defined in ``&<Dt/Help.h>'', include
the following:

<p indent> ``DT_HELP_TYPE_TOPIC'' indicates that the information is a
formatted help topic.  The displayed topic is located using the ID
specified by the
``XmNlocationId'' resource in the volume specified by ``XmNhelpVolume''.
(``DT_HELP_TYPE_TOPIC'' is the default value for the ``XmNhelpType''
resource.)

<p indent> ``DT_HELP_TYPE_STRING'' indicates that the information is a text
string provided in the ``XmNstringData'' resource.  Newline characters
within the string are used to determine line breaks when formatting the
string.

<p indent> ``DT_HELP_TYPE_DYNAMIC_STRING'' indicates that the information
is a text string provided in the ``XmNstringData'' resource.  Newline
characters within the string are used to separate paragraphs.  Text in the
string is automatically wrapped to fit the current size of the window.

<p indent> ``DT_HELP_TYPE_MAN_PAGE'' indicates that the information is a
man page.  The text to be displayed is retrieved internally by executing
the ``man'' command using the value of the ``XmNmanPage'' resource.

<p indent> ``DT_HELP_TYPE_FILE'' indicates that the information is in a text
file.  The file name is identified using the value of the ``XmNhelpFile''
resource. 

``XmNhelpVolume''
		<idx|``XmNhelpVolume''|
		<idx|resource: ``XmNhelpVolume''|

<p indent> Specifies the help volume to use.  This resource is used in
conjunction with the ``XmNlocationId'' resource, which specifies an ID
within the volume.  The ``XmNhelpType'' resource must be set to
``DT_HELP_TYPE_TOPIC''.

<p indent>The ``XmNhelpVolume'' resource expects a string (``char *'')
value.  Its default is the NULL string.


``XmNhyperLinkCallback''
		<idx|``XmNhyperLinkCallback''|
		<idx|resource: ``XmNhyperLinkCallback''|

<p indent> Specifies the list of callback functions executed when a
hyperlink event occurs within the help dialog.  Callbacks should be added
to the dialog using the standard ``XtAddCallback()'' function.
(See <xref RespondingToHyperlinkEventsSI>.)

``XmNlocationId''
		<idx|``XmNlocationId''|
		<idx|resource: ``XmNlocationId''|

<p indent> Specifies the ID string for a help topic.  The ``XmNhelpVolume''
resource must be set to specify the help volume in which the corresponding
ID resides, and ``XmNhelpType'' must be set to ``DT_HELP_TYPE_TOPIC''.

<p indent>The ``XmNlocationId'' resource expects a string (``char *'')
value.  Its default is the NULL string.

``XmNmanPage''
		<idx|``XmNmanPage''|
		<idx|resource: ``XmNmanPage''|

<p indent> Specifies a man page to display.  This resource is used when the
``XmNhelpType'' resource is set to ``DT_HELP_TYPE_MAN_PAGE''.

<p indent>The ``XmNmanPage'' resource expects a string (``char *'')
value.  Its default is the NULL string.  The string is passed directly to
the system ``man'' command to find and display the man page.


``XmNprinter''
		<idx|``XmNprinter''|
		<idx|resource: ``XmNprinter''|

<p indent> Specifies the printer device name to be used for printing help
topics. 

<p indent>The ``XmNprinter'' resource expects a string (``char *'')
value.  Its default is the NULL string, which causes printed help topics to
be directed to the system's default printer.

<need begin>

``XmNrows''
		<idx|``XmNrows''|
		<idx|resource: ``XmNrows''|

<p indent> Specifies the desired height of the help display area in
terms of the number of rows of text.  The
height of each row is determined by the current font scheme in use.  The
``XmNrows'' resource expects a number (type ``Dimension'') value.  The
default is ``15''.

<need end>

``XmNscrollBarDisplayPolicy''
	<idx|``XmNscrollBarDisplayPolicy''|
	<idx|resource: ``XmNscrollBarDisplayPolicy''|

<p indent> Controls the automatic placement of scroll bars around the help
topic display area.  If set to ``DT_HELP_AS_NEEDED_SCROLLBARS'', scroll bars are
displayed only if the help text doesn't completely fit within the display area.
(``DT_HELP_AS_NEEDED_SCROLLBARS'' is the default value for this resource.)

<p indent> If ``XmNscrollBarDisplayPolicy'' is set to
``DT_HELP_STATIC_SCROLLBARS'', the scroll bars are always managed, regardless of
the size of the current help topic.

<p indent> If ``XmNscrollBarDisplayPolicy'' is set to
``DT_HELP_NO_SCROLLBARS'', the scroll bars are never managed, even if the
current help topic is too big to competely fit within the display area.

``XmNstringData''
		<idx|``XmNstringData''|
		<idx|resource: ``XmNstringData''|

<p indent> Specifies a string of characters to display in the help dialog.
This resource is used when the ``XmNhelpType'' resource is set to
``DT_HELP_TYPE_STRING'' or ``DT_HELP_TYPE_DYNAMIC_STRING''.

<p indent>If ``XmNhelpType'' is set to ``DT_HELP_TYPE_STRING'', newline
characters in the string are used to determine the line breaks when
formatting the text.  If ``XmNhelpType'' is set to
``DT_HELP_TYPE_DYNAMIC_STRING'', newline characters are interpreted as
paragraph separators and the string is automatically wrapped to fit the
current display area.

<p indent>The ``XmNstringData'' resource expects a string (``char *'')
value.  Its default is the NULL string.

``XmNtopicTitle''
		<idx|``XmNtopicTitle''|
		<idx|resource: ``XmNtopicTitle''|

<p indent> Specifies the topic title to be used in conjunction with either
the ``XmNstringData'' or ``XmNhelpFile'' resource.  The topic title is
used to represent the topic in the History list.   This resource is used
only when the ``XmNhelpType'' is set to ``DT_HELP_TYPE_STRING'',
``DT_HELP_TYPE_DYNAMIC_STRING'', or ``DT_HELP_TYPE_FILE''.

<p indent>The ``XmNtopicTitle'' resource expects a string (``char *'')
value.  Its default is the NULL string.

<need begin>

``XmNvisiblePathCount''
		<idx|``XmNvisiblePathCount''|
		<idx|resource: ``XmNvisiblePathCount''|

<p indent> Specifies the height of the Topic Hierarchy area
(which is just below the help
dialog's menu bar).  The ``XmNvisiblePathCount'' resource expects an
``int'' (integer) value that specifies the number of visible items.  The
default value is ``4''.  This resource may only be set at creation time.

<need end>

<procedure> Callback Information

A pointer to the following structure is passed to each callback.

		<idx|``DtHelpDialogCallbackStruct''|
<ex>
typedef struct {
     int      reason;
     XEvent  *event;
     char    *locationId;
     char    *helpVolume;
     char    *specification;
     int      hyperType;
} DtHelpDialogCallbackStruct;
<\ex>

Where:

<lablist width=1.5in>

\``reason''\  This element indicates why the callback was invoked.

\``event''\  This element points to the event that triggered the callback.

\``locationId''\  This element points to the current topic ID.
                  If the dialog is not displaying a topic, this value is NULL.

\``helpVolume''\  This element points to the current help volume.
                  If the dialog is not
		  displaying a formatted help topic, this value is NULL.

\``specification''\   This element points to author-supplied data that is
		      used in application-defined hyperlinks.  This value
		      is NULL if the event that invoked the callback was
		      not an application-defined hyperlink.

\``hyperType''\   This element indicates the type of hyperlink that invoked
		  this callback.  The value is one of:
		  ``DT_HELP_LINK_JUMP_NEW'', ``DT_HELP_LINK_MAN_PAGE'', or
		  ``DT_HELP_LINK_APP_DEFINE''.

<\lablist>



<procedure>Usage Tips

<list bullet>
		<idx|``XtDestroyWidget()''|
		<idx|function: ``XtDestroyWidget()''|
* To destroy an instance of a help dialog, use ``XtDestroyWidget()''.


		<idx|``XtManageChild()''|
		<idx|function: ``XtManageChild()''|
* To display a help dialog, use ``XtManageChild()''.


		<idx|``XtUnmanageChild()''|
		<idx|function: ``XtUnmanageChild()''|
* To hide a help dialog, use ``XtUnmanageChild()''.

<\list>


<procedure>See Also

<list bullet tight>
 * <xref DisplayingHelpTopicsSI>
 * <xref RespondingToHyperlinkEventsSI>
 * <xref DetectingWhenHelpDialogsAreDismissed>
 * <xref ToCreateGeneralHelpDialogTA>
 * <xref CreatingDialogCacheSI>
<\list>







<s1 id=DtCreateQuickHelpDialog>``DtCreateQuickHelpDialog()''

		<idx|``DtCreateQuickHelpDialog()''|
		<idx|function: ``DtCreateQuickHelpDialog()''|

<ex>
#include &<Dt/Help.h>
#include &<Dt/HelpQuickD.h>

Widget DtCreateQuickHelpDialog (
     Widget    %%parent%%,
     String    %%name%%,
     ArgList   %%arglist%%,
     Cardinal  %%argcount%% );
<\ex>


Where:

<lablist>
\%%parent%%\  Specifies the parent widget ID.

\%%name%%\  Specifies the name of the new help dialog.

\%%arglist%%\  Specifies the argument list.

\%%argcount%%\  Specifies the number of attribute-value
pairs in the argument list (%%arglist%%).
<\lablist>

This function creates a new instance of a general help dialog and returns
its ID.  The widget ID returned is a Dialog Shell widget which serves as
the top level child in the created help dialog.  Refer to the OSF/Motif
documentation for more information on the Dialog Shell widget and
applicable resources.

<procedure> Resources

The following resources are specific to the quick help dialog widget.

``XmNbackLabelString''
		<idx|``XmNbackLabelString''|
		<idx|resource: ``XmNbackLabelString''|

<p indent> Specifies the string label for the Back button. The
``XmNbackLabelString'' resource expects a compound string (type
``XmString'') value.  The default string is "Backtrack." 



``XmNcolumns''
		<idx|``XmNcolumns''|
		<idx|resource: ``XmNcolumns''|

<p indent> Specifies the desired width of the help display area.  The
``XmNcolumns'' resource expects a number (type ``Dimension'') value that
represents the number of average-width characters (based on the current
font).  The default is ``40''.

``XmNhelpFile''
		<idx|``XmNhelpFile''|
		<idx|resource: ``XmNhelpFile''|

<p indent> Specifies the name of a text file to be displayed in the
help dialog. This resource is used only when the ``XmNhelpType'' resource
is set to ``DT_HELP_TYPE_FILE''.

<p indent>The ``XmNhelpFile'' resource expects a string (``char *'')
value.  Its default is the NULL string.


``XmNhelpPrint''
		<idx|``XmNhelpPrint''|
		<idx|resource: ``XmNhelpPrint''|

<p indent> Specifies the command used to print help topics.  The
``XmNhelpPrint'' resource expects a string (``char *'') value.  Its default
is ``/opt/dt/bin/dthelpprint''.

``XmNhelpType''
		<idx|``XmNhelpType''|
		<idx|resource: ``XmNhelpType''|

<p indent> Specifies the type of information to be displayed in the quick help
dialog.  Valid values, which are defined in ``&<Dt/Help.h>'', include the
following:

<p indent> ``DT_HELP_TYPE_TOPIC'' indicates that the information is a
formatted help topic.  The topic is located using the ID specified by the
``XmNlocationId'' resource in the volume specified by ``XmNhelpVolume''.
(``DT_HELP_TYPE_TOPIC'' is the default value for the ``XmNhelpType''
resource.)

<p indent> ``DT_HELP_TYPE_STRING'' indicates that the information is the text
string in the ``XmNstringData'' resource.  Newline characters
within the string are used to determine line breaks when formatting the
string.

<p indent> ``DT_HELP_TYPE_DYNAMIC_STRING'' indicates that the information
is the text string in the ``XmNstringData'' resource.  Newline
characters within the string are used to separate paragraphs.  Text in the
string is automatically wrapped to fit the current size of the window.

<p indent> ``DT_HELP_TYPE_MAN_PAGE'' indicates that the information is a
man page.  The text to be displayed is retrieved internally by executing
the ``man'' command using the value of the ``XmNmanPage'' resource.

<p indent> ``DT_HELP_TYPE_FILE'' indicates that the information is in a text
file.  The file name is identified using the value of the ``XmNhelpFile''
resource. 

``XmNhelpVolume''
		<idx|``XmNhelpVolume''|
		<idx|resource: ``XmNhelpVolume''|

<p indent> Specifies the help volume to use.  This resource is used in
conjunction with the ``XmNlocationId'' resource, which specifies an ID
within the volume.  The ``XmNhelpType'' resource must be set to
``DT_HELP_TYPE_TOPIC''.

<p indent>The ``XmNhelpVolume'' resource expects a string (``char *'')
value.  Its default is the NULL string.


``XmNhyperLinkCallback''
		<idx|``XmNhyperLinkCallback''|
		<idx|resource: ``XmNhyperLinkCallback''|

<p indent> Specifies the list of callback functions executed when a
hyperlink event occurs within the help dialog.  Callbacks should be added
to the dialog using the standard ``XtAddCallback()'' function.
(See <xref RespondingToHyperlinkEventsSI>.)

``XmNlocationId''
		<idx|``XmNlocationId''|
		<idx|resource: ``XmNlocationId''|

<p indent> Specifies the ID string for a help topic.  The ``XmNhelpVolume''
resource must be set to specify the help volume in which the corresponding
ID resides, and ``XmNhelpType'' must be set to ``DT_HELP_TYPE_TOPIC''.

<p indent>The ``XmNlocationId'' resource expects a string (``char *'')
value.  Its default is the NULL string.

``XmNmanPage''
		<idx|``XmNmanPage''|
		<idx|resource: ``XmNmanPage''|

<p indent> Specifies a man page to display.  This resource is used when the
``XmNhelpType'' resource is set to ``DT_HELP_TYPE_MAN_PAGE''.

<p indent>The ``XmNmanPage'' resource expects a string (``char *'')
value.  Its default is the NULL string.  The string is passed directly to
the system ``man'' command to find and display the man page.


``XmNminimizeButtons''

<p indent> Specifies whether the dialog's buttons should be resized so they are
all the same width as the widest button and the same height as the tallest
button.  The ``XmNminimizeButtons'' resource expects a Boolean value
(``True'' or ``False'').  The default is ``True'', which makes the buttons
all the same size.  If this resource is ``False'', the button sizes
are not altered from their default sizes.

``XmNmoreLabelString''
		<idx|``XmNmoreLabelString''|
		<idx|resource: ``XmNmoreLabelString''|

<p indent> Specifies the string label for the More button.  The
``XmNprintLabelString'' resource expects a compound string (type
``XmString'') value.  The default string is "More."  If this button is
used to display a general help dialog, the recommended label is
"Browse..."

<need begin>

``XmNokCallback''
		<idx|``XmNokCallback''|
		<idx|resource: ``XmNokCallback''|

<p indent> Specifies the list of callback functions executed when the user
chooses the OK button. The callback reason is ``DT_HELP_CR_OK''.  (See <xref
DetectingWhenHelpDialogsAreDismissed>.)

<need end>


``XmNokLabelString''
		<idx|``XmNokLabelString''|
		<idx|resource: ``XmNokLabelString''|

<p indent> Specifies the string label for the OK button.   The
``XmNokLabelString'' resource expects a compound string (type ``XmString'')
value.  The default string is "OK."

``XmNprinter''
		<idx|``XmNprinter''|
		<idx|resource: ``XmNprinter''|

<p indent> Specifies the printer device name to be used for printing help
topics. 

<p indent>The ``XmNprinter'' resource expects a string (``char *'')
value.  Its default is the NULL string, which causes printed help topics to
be directed to the system's default printer.

``XmNrows''
		<idx|``XmNrows''|
		<idx|resource: ``XmNrows''|

<p indent> Specifies the desired height of the help display area in
terms of the number of rows of text.  The
height of each row is determined by the currently used font scheme.  The
``XmNrows'' resource expects a number (type ``Dimension'') value.  The
default is ``15''.

``XmNscrollBarDisplayPolicy''
	<idx|``XmNscrollBarDisplayPolicy''|
	<idx|resource: ``XmNscrollBarDisplayPolicy''|

<p indent> Controls the automatic placement of scroll bars around the help
topic display area.  If set to ``DT_HELP_AS_NEEDED_SCROLLBARS'', scroll bars are
displayed only if the help text doesn't completely fit within the display area.
(``DT_HELP_AS_NEEDED_SCROLLBARS'' is the default value for this resource.)

<p indent> If ``XmNscrollBarDisplayPolicy'' is set to
``DT_HELP_STATIC_SCROLLBARS'', the scroll bars are always managed, regardless of
the size of the current help topic.

<p indent> If ``XmNscrollBarDisplayPolicy'' is set to
``DT_HELP_NO_SCROLLBARS'', the scroll bars are never managed, even if the
current help topic is too big to competely fit within the display area.

``XmNstringData''
		<idx|``XmNstringData''|
		<idx|resource: ``XmNstringData''|

<p indent> Specifies a string of characters to display in the help dialog.
This resource is used when the ``XmNhelpType'' resource is set to
``DT_HELP_TYPE_STRING'' or ``DT_HELP_TYPE_DYNAMIC_STRING''.

<p indent>If ``XmNhelpType'' is set to ``DT_HELP_TYPE_STRING'', newline
characters in the string are used to determine the line breaks when
formatting the text.  If ``XmNhelpType'' is set to
``DT_HELP_TYPE_DYNAMIC_STRING'', newline characters are interpreted as
paragraph separators and the string is automatically wrapped to fit the
current window size.

<p indent>The ``XmNstringData'' resource expects a string (``char *'')
value.  Its default is the NULL string.

``XmNhelpLabelString''
		<idx|``XmNhelpLabelString''|
		<idx|resource: ``XmNhelpLabelString''|

<p indent> Specifies the string label for the Help button.  The
``XmNhelpLabelString'' resource expects a compound string (type ``XmString'')
value.  The default string is "Help."

``XmNprintLabelString''
		<idx|``XmNprintLabelString''|
		<idx|resource: ``XmNprintLabelString''|

<p indent> Specifies the string label for the Print button. The
``XmNprintLabelString'' resource expects a compound string (type ``XmString'')
value.  The default string is "Print..."


<procedure> Callback Information

A pointer to the following structure is passed 
to each callback.

		<idx|``DtHelpDialogCallbackStruct''|

<ex>
typedef struct {
     int      reason;
     XEvent  *event;
     char    *locationId;
     char    *helpVolume;
     char    *specification;
     int      hyperType;
} DtHelpDialogCallbackStruct;
<\ex>


<lablist width=1.5in>

\``reason''\  This element indicates why the callback was invoked.

\``event''\  This element points to the event that triggered the callback.

\``locationId''\  This element points to the current topic ID.  If the
		  dialog is not displaying a topic, this value is NULL.

\``helpVolume''\  This element points to the current help volume.  If the
		  dialog is not displaying a formatted help topic, this
		  value is NULL.

\``specification''\   This element points to author-supplied data that is
		      used in application-defined hyperlinks.  This value
		      is NULL if the event that invoked the callback was
		      not an application-defined hyperlink.

\``hyperType''\   This element indicates the type of hyperlink that invoked
		  this callback.  The value is one of:
		  ``DT_HELP_LINK_JUMP_NEW'', ``DT_LINK_MAN_PAGE'', or
		  ``DT_LINK_APP_DEFINE''.

<\lablist>

<procedure>Usage Tips

<list bullet>
		<idx|``XtDestroyWidget()''|
		<idx|function: ``XtDestroyWidget()''|
* To destroy an instance of a quick help dialog, use ``XtDestroyWidget()''.


		<idx|``XtManageChild()''|
		<idx|function: ``XtManageChild()''|
* To display a quick help dialog, use ``XtManageChild()''.


		<idx|``XtUnmanageChild()''|
		<idx|function: ``XtUnmanageChild()''|
* To hide a quick help dialog, use ``XtUnmanageChild()''.

<\list>

<procedure>See Also

<list bullet tight>
 * <xref DisplayingHelpTopicsSI>
 * <xref RespondingToHyperlinkEventsSI>
 * <xref DetectingWhenHelpDialogsAreDismissed>
 * <xref UsingApplicationDefinedButton>
 * <xref ToCreateQuickHelpDialogTA>
 * <xref CreatingDialogCacheSI>
<\list>







<s1 id=DtHelpQuickDialogGetChild>``DtHelpQuickDialogGetChild()''

		<idx|``DtHelpQuickDialogGetChild()''|
		<idx|function: ``DtHelpQuickDialogGetChild()''|

<ex>
#include &<Dt/HelpQuickD.h>

Widget DtHelpQuickDialogGetChild (
     Widget         %%widget%%,
     unsigned char  %%child%% );
<\ex>



<procedure> Description

DtHelpQuickDialogGetChild is used to access a 
component within a Quick Help Dialog. The parameters 
given to the function are the Quick Help 
Dialog widget and a value indicating which child to 
access.


<lablist>

\%%widget%%\  Specifies the widget ID of the quick help 
dialog.

\%%child%%\  Specifies a component within the Quick Help 
Dialog. The following are 
legal values for this parameter:

<list plain tight>
* ``DT_HELP_QUICK_OK_BUTTON''
* ``DT_HELP_QUICK_PRINT_BUTTON''
* ``DT_HELP_QUICK_HELP_BUTTON''
* ``DT_HELP_QUICK_SEPARATOR''
* ``DT_HELP_QUICK_MORE_BUTTON''
* ``DT_HELP_QUICK_BACK_BUTTON''
<\list>

<\lablist>


<procedure> Return Value

Returns the widget ID of the specified Quick 
Help Dialog child. An application should not assume 
that the returned widget will be of any particular 
class.


<procedure> Usage

DtHelpQuickDialogGetChild() allows developers to create 
and display Quick Help Dialogs with different
button configurations.






<s1 id=DtHelpReturnSelectedWidgetId>``DtHelpReturnSelectedWidgetId()''

		<idx|``DtHelpReturnSelectedWidgetId()''|
		<idx|function: ``DtHelpReturnSelectedWidgetId()''|

<ex>
#include &<Dt/HelpUtil.h>

int DtHelpReturnSelectedWidgetId (
     Widget   %%parent%%,
     Cursor   %%cursor%%,
     Widget  *%%widget%% );
<\ex>

Where:

<lablist>

\%%parent%%\  Specifies the widget ID to use as the basis of the
	      interaction.  This can be any valid widget within the
	      application's widget hierarchy. Usually it is a top-level or
	      application shell widget.

\%%cursor%%\  Specifies the shape to be used for the pointer during the
	      interaction.  If %%cursor%% is NULL, the function uses a
	      default pointer shape.

\%%widget%%\  The ID of the widget that the user selects.  If the value
	      returned is NULL, the function was canceled with the Esc key
	      or an error occurred.

<\lablist>



<procedure> Description

This function temporarily grabs the pointer so the user can select any
widget on the screen.  The function completes when the user selects a
widget or presses Esc to cancel the function.  

If a successful selection has been made, the %%widget%% parameter contains
the widget ID of the selected widget.

The function always returns one of the following exit status values:

<lablist width=1.75in>
		<idx|``DT_HELP_SELECT_VALID''|
		<idx|return status: ``DT_HELP_SELECT_VALID''|
\``DT_HELP_SELECT_VALID''\  Indicates that the selection was successful.  The
		       %%widget%% parameter should contain the ID of the
		       selected widget.

		<idx|``DT_HELP_SELECT_INVALID''|
		<idx|return status: ``DT_HELP_SELECT_INVALID''|
\``DT_HELP_SELECT_INVALID''\  Indicates that the user selected an invalid widget
			 outside the scope of the current application's
			 widget hierarchy.

		<idx|``DT_HELP_SELECT_ABORT''|
		<idx|return status: ``DT_HELP_SELECT_ABORT''|
\``DT_HELP_SELECT_ABORT''\  Indicates that the user canceled the function by
		       pressing the Esc key.

		<idx|``DT_HELP_SELECT_ERROR''|
		<idx|return status: ``DT_HELP_SELECT_ERROR''|
\``DT_HELP_SELECT_ERROR''\  Indicates that the function terminated due to an
		       error.
<\lablist>

<procedure>See Also

<list bullet tight>
 * <xref SupportingItemHelpSI> explains how the
   ``DtHelpReturnSelectedWidgetId()'' function is used to implement "item help
   mode."
<\list>



<s1 id=DtHelpGetTopicData>``DtHelpGetTopicData()''

		<idx|``DtHelpGetTopicData()''|
		<idx|function: ``DtHelpGetTopicData()''|

<ex>
#include &<Dt/HelpTerm.h>

int DtHelpGetTopicData (
     char            *%%helpVolume%%,
     char            *%%locationId%%,
     int              %%maxColumns%%,
     char          ***%%helpList%%, 
     DtHelpHyperLines  **%%hyperList%% );
<\ex>

Where:

<lablist>

\%%helpVolume%%\  Specifies the help volume file.  This must be a fully
		  qualified path to a help volume file (``%%volume%%.sdl'').

\%%locationId%%\  Specifies the topic ID to search for within the
		  %%helpVolume%%.

\%%maxColumns%%\  Specifies the maximum number of characters per line
                  for formatting the help text.

\%%helpList%%\    A returned value containing a pointer to a list of
		  NULL-terminated strings.  Each string is a line of help
		  text.	 Blank lines are allocated with zero-length line.
		  The caller is responsible for freeing the %%helpList%%
		  information using the ``DtHelpFreeTopicData()'' function.

\%%hyperList%%\   A returned value containing a pointer to a list of
		  NULL-terminated hyperlink specifications.  See
		  below for more information.  The caller is
		  responsible for freeing the %%hyperList%% information
		  using the ``DtHelpFreeTopicData()'' function.

<\lablist>


The ``DtHelpGetTopicData()'' function provides a mechanism for retrieving help
text from a help volume.  All graphics and special characters are stripped
from the data.

Hyperlinks found within the topic are assembled in a list (%%hyperList%%)
and a pointer to that list is returned.  It is up to the application
to display the list of
possible links and provide a user interface for choosing a link.  When a
link is chosen, the ``DtHelpProcessLinkData()'' function is used to follow the
link to display the related topic.

If any problems occurred while processing the information, a value of ``-1''
is returned and the appropriate error message is generated.  If the
requested information is found and processed with no errors, then this
function returns zero (``0'').

The hypertext specifications returned by the ``DtHelpGetTopicData()'' function
are of type ``DtHelpHyperLines'', which is a structure defined as follows:

		<idx|``DtHelpHyperLines''|
		<idx|structure: ``DtHelpHyperLines''|
<ex>
typedef struct {
     char  *title;
     char  *linkData;
     int    hyperType;
} DtHelpHyperLines;
<\ex>

Where:

<lablist>

\``title''\  This element specifies the title to the hypertext topic that
	     is pointed to by ``linkData''.

\``linkData''\  This element specifies a pointer to a string that is the
		hypertext link information. Depending on the type of
		hypertext link, this could be a file name, an ID string, or
		a command to execute.

\``hyperType''\     This element specifies the hyperlink type, which is one
		    of these values: ``DT_HELP_LINK_JUMP_REUSE'',
		    ``DT_HELP_LINK_JUMP_NEW'', ``DT_HELP_LINK_DEFINITION'',
		    ``DT_HELP_LINK_EXECUTE'', or ``DT_HELP_LINK_APP_DEFINED''.

<\lablist>


This function does not provide information to determine which words or phrases
in %%helpList%% are associated with the members in %%hyperList%%.


<s1 id=DtHelpProcessLinkData>``DtHelpProcessLinkData()''

		<idx|``DtHelpProcessLinkData()''|
		<idx|function: ``DtHelpProcessLinkData()''|

<ex>
#include &<Dt/HelpTerm.h>

int  DtHelpProcessLinkData (
     char               *%%ref_volume%%
     DtHelpHyperLines   *%%hyperList%%,
     char           **%%helpVolume%%,
     char           **%%locationId%% );
<\ex>

Where:

<lablist>

\%%ref_volume%%\ Specifies the original volume that %%hyperList%% was
                 generated from.

\%%hyperList%%\  Specifies an individual structure in the
		 %%hyperList%% value returned by the ``DtHelpGetTopicData()''
		 function.  The caller is responsible for freeing the
		 %%hyperList%% information using the ``DtHelpFreeTopicData()''
		 function.

\%%helpVolume%%\  Returns the fully qualified path of the help volume file.

\%%locationId%%\  Returns the topic ID to locate within the
		  %%helpVolume%%.

<\lablist>

The ``DtHelpProcessLinkData()'' function provides a mechanism for traversing
hyperlinks that occur within a topic retrieved with the
``DtHelpGetTopicData()'' function.

If the requested information is found and formatted with no errors, a
status of zero (``0'') is returned. If any problems occurred, a value of
``-1'' is returned.





<s1 id=DtHelpFreeTopicData>``DtHelpFreeTopicData()''

		<idx|``DtHelpFreeTopicData()''|
		<idx|function: ``DtHelpFreeTopicData()''|

<ex>
#include &<Dt/HelpTerm.h>

void DtHelpFreeTopicData (
     char           **%%helpList%%;
     DtHelpHyperLines   *%%hyperList%%; );
<\ex>

Where:

<lablist>

\%%helpList%%\  Specifies the list of strings to be freed.

\%%HyperList%%\  Specifies the list of hyperlink specifications to be
		 freed.

<\lablist>


The ``DtHelpFreeTopicData()'' function frees the memory allocated for
structures returned by the ``DtHelpGetTopicData()'' function. 






<s1 id=DtHelpSetCatalogName>``DtHelpSetCatalogName()''

		<idx|``DtHelpSetCatalogName()''|
		<idx|function: ``DtHelpSetCatalogName()''|

<ex>
#include &<Dt/Help.h>

void DtHelpSetCatalogName (
     char           *%%catFile%%; );
<\ex>

Where:

<lablist>

\%%catFile%%\  The name of the message catalog file.

<\lablist>

The ``DtHelpSetCatalogName()'' function sets the name of the message catalog
used to supply the labels and error messages for help dialogs.

Users of the &KitName; who ship a translated version
of their product must uniquely name their translated version of the
``DtHelp.cat'' message catalog file (for example ``%%appName%%_DtHelp.cat'').
This ensures that other products will not overwrite the message catalog
file for your application's help dialogs        .







