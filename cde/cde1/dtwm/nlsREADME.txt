
#############################################################################
#
# Component:  dtwm
#
############################################################################

 This is presented in a example format.  All examples
 are for the japanese LANG.
 
 Look in the dt system administration manual for more general
 information.
 


_DtMessage catalog source:

   File name:   dtwm.msg
   
   Target:      /usr/dt/nls/%L/dtwm.cat
      
   Instructions:
    
	Go to /usr/dt/nls
	mkdir japanese
	place dtwm.msg in japanese
	translate messages as per instructions in dtwm.msg
	set LANG to japanese
	run gencat dtwm.cat dtwm.msg


App-defaults file:
      
    File name:   Dtwm
	 
    Target:      /usr/dt/app-defaults/%L/Dtwm
	    
    Instructions:
    
	Go to /usr/dt/app-defaults
	  a) If there is not a "japanese" directory, 
	     mkdir japanese
	
	  b) cp /usr/dt/app-defaults/Dtwm  /usr/dt/app-defaults/japanese
	     Edit /usr/dt/app-defaults/japanese/Dtwm

	     Change Dtwm*Date*format:       %b %.1d%n%a
 	     to a language specific date format.  This should be the
	     same format string that was used in Dt 2.01.
             Use the same syntax as that described in the STRFTIME(3) man page.

	     The Dt 2.01 date format for japanese was:
             Dtwm*dtwmdate*format: %b %.1d“ú%n(%a)
                                            ^^
                       (octal character 223 ||)
					     |
                       (octal character 372  |)
		       
	     so the Dt 3.0 date format for japanese will be;
             Dtwm*Date*format: %b %.1d“ú%n(%a)
                                       ^^
                  (octal character 223 ||)
                                        |
                  (octal character 372  |)
		       


	     Change these
		!# Dtwm*FrontPanel*highResFontList: system17
		!# Dtwm*FrontPanel*mediumResFontList: system16
		!# Dtwm*FrontPanel*lowResFontList: system13
	     to a language specific font and uncomment them
	     by removing the leading "!# " from each line.

	     Change these

		Dtwm*VersionDialog_popup*rows:		20
		Dtwm*VersionDialog_popup*columns:	40

	     to set the size (number of character rows
	     and columns) for the Workspace Manager Version
	     dialog. (This dialog pops up when you press the
	     HP Logo on the front panel.) Set the size so 
	     that all the information is available with no 
	     scroll bar.

    

System default configuration files (subpanels):

    File name:   fp.clock
		 fp.date
		 fp.help
		 fp.home
		 fp.load
		 fp.mail
		 fp.printer
		 fp.style
		 fp.terminal
		 fp.tool
		 fp.toolbox
		 fp.trash
	 
    Target:      /usr/dt/config/panels/%L/fp.clock
		 /usr/dt/config/panels/%L/fp.date
		 /usr/dt/config/panels/%L/fp.help
		 /usr/dt/config/panels/%L/fp.home
		 /usr/dt/config/panels/%L/fp.load
		 /usr/dt/config/panels/%L/fp.mail
		 /usr/dt/config/panels/%L/fp.printer
		 /usr/dt/config/panels/%L/fp.style
		 /usr/dt/config/panels/%L/fp.terminal
		 /usr/dt/config/panels/%L/fp.tool
		 /usr/dt/config/panels/%L/fp.toolbox
		 /usr/dt/config/panels/%L/fp.trash

    Instructions:
    
	Go to /usr/dt/config/
	  a) mkdir japanese
	  b) cp /usr/dt/config/fp.* japanese
	  c) cd japanese
	  d) Edit each fp.* file (fp.clock, fp.date, etc.).
	     Look for any element that has a type of TITLE, LABEL, or
	     HELP_STRING.  These can all be localized.



System default configuration file:

    File name:   sys.dtwmrc
	 
    Target:      /usr/dt/config/%L/sys.dtwmrc
	    
    Instructions:
    
	Go to /usr/dt/config/
	  a) mkdir japanese
	  b) cp /usr/dt/config/sys.dtwmrc japanese
	  c) cd japanese
	  d) Edit sys.dtwmrc.  
	  e) look for any element that has a type of TITLE, LABEL,
	     or HELP_STRING. These can all be localized
	  f) look for the lines in the file that begin "INCLUDE", as
	     in 
	     
	     INCLUDE
	     {
	         /usr/dt/config/panels/fp.printer
	     }
	     
	     These include lines must be updated to reference the 
	     L10N config directory, as 

	     INCLUDE
	     {
	         /usr/dt/config/panels/japanese/fp.printer
	     }

	  g) look for the "Busy" and "LiteBusy" controls. These controls 
	     use the "exit" and "exit02" images. These images contain 
	     small pictures of the DT login screen and busy light. The
	     picture of the DT login screen in these images contain
	     the English word "EXIT". Replace these two lines with
	     the following:

	      IMAGE            exitl
	      ALTERNATE_IMAGE  exitl02

	     NOTE: A lowercase 'L' is used in the above names, not the
	     number '1'.

	     The "exitl" and "exitl02" images do not have the word
	     "EXIT" in the DT login screen picture.

	     Remove the comment from the "LABEL" line. Localize the 
	     "Exit" text in the LABEL line.


	To localize the default root menu for dtwm:
     	Edit sys.dtwmrc to change the DtRootMenu.
	     The root menu has no mnemonics or accelerators so only the
  	     labels should be localized.


     	The default window menu is localized in dtwm.msg.  You may want to
	localize the window menu definition here so users can use it as a
	starting point for customizing their window menu
	Here are the comments from dtwm.msg that explain what should
	and should not be localized for DtWindowMenu. 
	

$ Messages for the built in menu.  The built in menu will 
$ appear unless a user if a user (through resources or app-defaults) 
$ specified a window menu .

$ Messages 48-58 are the definitions for the default built in
$ window menu.  These new messges allow the localizer to change the
$ accelerators if they interfere with the local keyboard.  The only item
$ that has an accelerator is Close _C Alt<Key>F4 f.kill.

$ The underbar is part of the syntax for mnemonics and should not 
$ be changed. The letter after the underbar should match a 
$ letter (case sensitive) in the item title and should not conflict
$ with another mnemonic.
$        (e.g.   Normalize  _N  )

$ The name of the function (e.g. f.normalize) should not change

$    This item will "close" the window.  This usually results in the
$    window being withdrawn.
48 Close _C Alt<Key>F4 f.kill

$    This item normalizes a window
49 Restore _R  f.normalize

$    This item moves a window
50 Move _M  f.move

$    This item resizes a window
51 Size _S  f.resize

$    This item changes a window into an icon
52 Minimize _n  f.minimize

$    This item makes a window as large as allowed (determined by a number
$         of factors)
53 Maximize _x  f.maximize

$    This item lowers a window below other windows
54 Lower _L  f.lower

$    This item brings up the occupy workspace dialog.  The character
$    sequence  "\\.\\.\\."  will appear as "..." in the menu . DO NOT CHANGE 
$    THE  "\\.\\.\\."  CHARACTERS
55 Occupy\\ Workspace\\.\\.\\. _O  f.workspace_presence

$    This item puts a window into all workspaces.  The character sequence
$    "\\" provides a space between "Occupy" and "all"  Use this sequence
$    to provide spaces in item titles.
56 Occupy\\ All\\ Workspaces _A  f.occupy_all

$    This item removes a window from a workspace.  This item will appear in
$    in the menu as  "Unoccupy Workspace"  with the first e in the title underlined
$    to indicate this is a mnemonic for this item.
57 Unoccupy\\ Workspace _U  f.remove

