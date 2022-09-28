<chapter id=ChapterNativeLanguageSupport > Native Language Support

<!-- IFDEF ONLINE ONLY -->
<list bullet tight>
 * <xref HelpForInternationalAudiencesSI>
 * <xref UnderstandingFontSchemesSI>
<\list>
<!-- IFDEF ONLINE ONLY END -->



&headerFix;

		<idx|native language support|
		<idx|language: native support|
		<idx|support for: native language|
		<idx|localization|
		<idx|foreign language|
		<idx|language: foreign|


<!--
<note>
This &TagAppName; software snapshot does not support multi-byte character
sets.  A future release, &TagAppName; Version 1.0, will support localized
help volumes.
<\note>
-->

If your product is intended for an international audience, then providing
online help in the users' native language is important.  The &ProductName;
supports the authoring and displaying of online help in virtually any
language.  Several factors, which are explained below, contribute to
providing online help in the user's native language.

<procedure>Character Sets and \ Multibyte Characters

A <term nogloss|character set| determines how a computer's internal
character codes (numbers) are mapped to recognizable characters.  In most
languages, single-byte characters are sufficient for representing an entire
character set.  However, there are some languages that use thousands of
characters.  Some of these languages require two to four bytes to represent
each character uniquely.

The &ViewerAppName; application supports multi-byte character sets.  


<procedure>The &TagAppName; Software

When you process a help volume to create run-time help files, the &TagAppName;
software must be told what character set you used to author your files.
The character set information is used to determine the proper fonts for
displaying help topics.  If you do not specify a character set, &TagAppName;
assumes the default, which is ISO 8859-1.

<note>
When writing &TagAppName; files, you may use multi-byte characters for any help
text.  However, the &TagAppName; markup itself (tag names, entity names, IDs,
and so on), must be entered using eight-bit characters.
<\note>

<procedure>The DtHelp Message Catalog

The menus, buttons, and labels that appear in help dialogs should also be
displayed in the user's native language.  To enable this, Help dialogs
read such strings from a <term nogloss|message catalog| named ``DtHelp.cat''.

		<idx|``gencat'' command|
		<idx|command: ``gencat''|

The ``DtHelp.cat'' file has been translated into several languages and
these translations are included with the &KitName;.  Look in the
``/opt/dt/dthelp/nls/'' directory.  If the language you need is not
supplied, you'll have to translate the message catalog
(``/opt/dt/dthelp/nls/DtHelp.msg'') and then use the ``gencat'' command
to create the needed run-time message catalog file.


<procedure>Font Schemes

One of the primary functions of the &TagAppName; software is to convert your
marked-up files into run-time formats that the &ProductName; understands.
Text is formatted by specifying particular attributes such as type family,
size, slant, and weight.  Font schemes are used to "map" combinations of
attributes to actual font specifications.

		<idx|formatting table|
		<idx|``fmt_tbl.cat''|
		<idx|Japanese|
		<idx|Japanese EUC|

Formatting of some languages also requires a !!formatting table!!.  This
table specifies rules for word wrap and other processing.  If you are
preparing a Japanese help volume, be sure the sample formatting table in
``/opt/dt/dthelp/nls/japanese/fmt_tbl.cat'' is installed in the 
``/usr/lib/nls/japanese/'' directory.  (If you are using the Japanese EUC
character set, use the ``euc/'' subdirectory.)


<procedure>The LANG \ Environment Variable

The user's LANG environment variable is important for these two reasons:

<list bullet>

* The value of LANG is used to locate the correct help volume.

* When a help topic is displayed, the correct fonts and formatting
  rules are chosen based on the user's LANG variable.  This is
  especially important for Asian languages that have word-wrap rules that are
  more sophisticated than European and American languages.

<\list>



<!-- seealsos/Hlaintr.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref HowHelpFilesAreFoundSI>
 * Also refer to the NLS documentation for your computer's operating system
   or programmer's kit.
<\list>



<s1 id=HelpForInternationalAudiencesSI >Preparing Online Help for \ International Audiences
		<idx|writing: for international audiences|
		<idx|international audiences, writing for|
		<idx|audience: writing for international|
		<idx|NLS checklist for authors and translators|
		<idx|checklist: NLS|
		<idx|``helplang.ent''|

The following checklist summarizes the questions you should answer when
providing online help for international audiences:

<list check>

 * Are help topics written with an international audience in mind?

 * Did you copy the ``/opt/dt/dthelp/dthelptag/helplang.ent'' file and localize
   the string entities it contains?  Using the entities in this file,
   you can override the English strings built into the &TagAppName; software.

 * Was the &TagAppName; software run using the correct character set option?
   Here are the most common character set names:

<ex>
iso8859
Japanese-EUC
Korean-EUC
Chinese-EUC (Traditional)
Chinese-EUC (Simplified)
<\ex>

   If you author in another character set, you may have to translate the
   ``DtHelp.msg'' message catalog file and provide a font scheme that supports
   the new character set.

 * Within your &TagAppName; markup, are all tag names, entity names, and IDs
   entered using an eight-bit character set, even if the help text uses 
   multi-byte characters?

 * When the user's LANG environment variable is set to the correct
   language, are the help files installed so they are found and displayed
   appropriately?

 * If you have integrated the &ProductName; into an application, have you
   properly set the "locale" using the ``setlocale()'' function?

<\list>



<!-- seealsos/Hlaiaud.SA -->
<!-- SEE ALSO -->

<procedure>See Also

<list bullet tight>
 * <xref HowHelpFilesAreFoundSI> describes how search paths (which use the
   value of LANG) are used to locate help files.
 * The man page for the ``setlocale()'' function describes when and how to
   use the ``setlocale()'' function.
 * The man page for the ``gencat'' command describes how to create a message
   catalog file.  You'll use this command if you translate the ``DtHelp.msg''
   file. 
<\list>

<!-- IFDEF ONLINE ONLY -->
<!-- SEE ALSO ONLINE -->

<procedure>See Also

<list bullet tight>
 * <xref HowHelpFilesAreFoundSI> describes how search paths (which use the
   value of LANG) are used to locate help files.

 * The man page for the <link hyperlink="setlocale"
   type=Man>``setlocale()''<\link> function describes when and how to use
   the ``setlocale()'' function. 

 * The man page for the <link hyperlink="gencat"
   type=Man>``gencat''<\link> command describes how to create a message
   catalog file.  You'll use this command if you translate the ``DtHelp.msg''
   file. 

<\list>

<!-- IFDEF ONLINE ONLY END -->


<s1 id=UnderstandingFontSchemesSI >Understanding Font Schemes

<!-- IFDEF ONLINE ONLY -->
<!-- &UnderstandingFontSchemesSISubtopics; -->
<list bullet tight>
* <xref ToChooseFontSchemeForDisplayTA>
<\list>

<!-- IFDEF ONLINE ONLY END -->

		<idx|font: scheme, determining actual appearance|
		<idx|appearance, determining font scheme|

When you write a help volume using the &TagAppName; markup language, you don't
specify the fonts and sizes of the text.  When you run the &TagAppName;
software, the structural information you've entered is formatted into
run-time help files which include text attributes.

A <term nogloss|font scheme| maps text attributes to actual font
specifications.  For example, if a help topic has some text formatted as
"sans serif, bold, italic," the font scheme dictates which X font is
actually used to display the text.

One of the primary uses of font schemes is to provide a choice of font
sizes.  The &TagAppName; software formats the body of most topics as 10-point
text.  However, because the actual display font is determined by the font
scheme being used, all 10-point text could be specified to use a 14-point
font.

<procedure>Font Resources

Each font scheme is actually a set of X resources.  These resources are
read by the application displaying the help.  For example, if you use
&ViewerAppName; to display a help volume, the font scheme is included in
&ViewerAppName;'s app-defaults file ( /opt/dt/app-defaults/C/Dthelpview)
For application help, the resources may be added to the application's
app-defaults file.

Each resource within a font scheme has this general form:

<ex>
*%%pitch%%.%%size%%.%%slant%%.%%weight%%.%%style%%.%%char-set%%:  %%font%%
<\ex>

Where ...

<lablist width=1.5cm>

 \ %%pitch%% \	Specifies the horizontal spacing of characters.  This field
		should be either ``p'' (proportional) or ``m'' (monospace).

 \ %%size%% \	Specifies the height of the desired font.  For help files
		formatted with &TagAppName;, this value should be ``6'', ``8'', ``10'',
		``12'', or ``14''.

 \ %%slant%% \	Specifies the slant of the desired font.  Usually this
		field is either ``roman'' for upright letters or ``italic''
		for slanted letters.

 \ %%weight%% \	Specifies the weight of the desired font.  Usually this
		field is either ``medium'' or ``bold''.

 \ %%style%% \	Specifies the general style of the desired font.  For help
		files formatted with &TagAppName;, this value should be either
		``serif'' or ``sans_serif''.

 \ %%char-set%% \	Specifies the character set used to author the help
			text.  This value must match the character set that was
			specified when &TagAppName; was run.	 The default is
			``iso8859-1''.	Some special characters are
			displayed using a ``symbol'' character set.

<\lablist>

An asterisk (``*'') can be used in a field to specify a font that has
any value of that particular attribute.  For instance, the symbol set
(for special characters and special symbols) distinguishes a unique
font based only on size and character set.  Its font resources appear like
this within a font scheme:

<ex smaller>
*.8.*.*.*.symbol:   -adobe-symbol-medium-r-normal--8-*-*-*-p-*-adobe-fontspecific
*.10.*.*.*.symbol:  -adobe-symbol-medium-r-normal--10-*-*-*-p-*-adobe-fontspecific
*.12.*.*.*.symbol:  -adobe-symbol-medium-r-normal--12-*-*-*-p-*-adobe-fontspecific
*.14.*.*.*.symbol:  -adobe-symbol-medium-r-normal--14-*-*-*-p-*-adobe-fontspecific
<\ex>

The ``/opt/dt/dthelp/fontschemes/'' directory contains some sample
font schemes.  The naming convention of the sample files uses the actual
pixel height of the font used to display text formatted as "10-point" text.
For example, the file ``help014.fns'' contains a font scheme where all
10-point text is displayed using a 14-pixel font.

<newpage>




<s2 id=ToChooseFontSchemeForDisplayTA > To choose a font scheme
		<idx|choosing font scheme for display|
		<idx|font: scheme, for display|
		<idx|display, font scheme for|

<list bullet>
 * Edit the app-defaults file for the application that displays the online
   help.  Replace the current font resources (if any) with the new scheme.
<\list>

If you are making this change just for yourself, copy the app-defaults file
into your home directory before editing it.  

<procedure>Example: Fonts for\ &ViewerAppName;

To use a larger size font in your &ViewerAppName; windows, first change to
your home directory:

<ex smaller>
cd
<\ex>

Then copy the &ViewerAppName; app-defaults file and make it writable:

<ex smaller>
cp /opt/dt/app-defaults/C/Dthelpview .
chmod u+w Dthelpview
<\ex>


Edit the ``Dthelpview;'' file to replace the existing font scheme with the
largest scheme (``help017.fns'').  Search for the block of comments titled
"Font Scheme."  Delete !!all!! the lines that follow those comments.
Then, insert the contents of this file:

<ex smaller>
/opt/dt/dthelp/fontschemes/help017.fns
<\ex>

Save your new ``Dthelpview'' file.  Any new instance of &ViewerAppName;
that you start will now use the new font scheme.  





 

