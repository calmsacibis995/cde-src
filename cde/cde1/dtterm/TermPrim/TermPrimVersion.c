#ifndef	lint
#ifdef	VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermPrimVersion.c /main/cde1_maint/2 1995/08/26 22:59:27 barstow $";
#endif	/* VERBOSE_REV_INFO */
#endif	/* lint */

/*
** (c) Copyright 1993, 1994 Hewlett-Packard Company
** (c) Copyright 1993, 1994 International Business Machines Corp.
** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
** (c) Copyright 1993, 1994 Novell, Inc.
*/
     
#include <X11/hpversion.h>

#ifndef	lint
hpversion(DtTermPrimitiveWidget:         $XConsortium: TermPrimVersion.c /main/cde1_maint/2 1995/08/26 22:59:27 barstow $)
#endif	/* lint */

char _DtTermPrimPullInTermWhatString[] = "";

/* $Log$
 * Revision 1.188  95/06/12  16:32:49  16:32:49  porsche (John Milton)
 * Defect repair:
 *      CISlx19246 - Not export XFILESEARCHPATH CMVC7922
 *      CISlx18013 - Dtterm isn't calling VuEnvControl to
 *                   set up Vue environment
 *  
 *  These two defects were both related to dtterm not doing the 'right thing'
 *  relative to environment control.  Dtterm now calls the appropriate Dt
 *  (and corresponding VUE) API's to insure that the environment is properly
 *  initialized before exec'ing child processes and to enable it to find
 *  localized help volumes in the standard locations.
 *  
 *      DtTermMain.c
 *          - added call to VuEnvControl() for 10.10
 *  
 *      TermPrim/TermPrimSubproc.c
 *          - added call to [Dt|Vu]EnvControl()
 *  
 *  John Milton
 * 
 * 
 * Revision 1.187  95/03/03  13:13:56  13:13:56  porsche (John Milton)
 * More CDE bug fixes.
 *  
 *      TermPrim.c
 *          - fixed CMVC 8146 - destroying term widgets can cause core dump
 *            The TermWidget destroy method was not unregistering the input
 *            method.
 * 
 * 
 * Revision 1.186  95/02/23  14:53:59  14:53:59  porsche (John Milton)
 * More CDE bug fixes.
 *  
 *      TermPrimRender.c
 *          - fixed CMVC 7215 - Smooth scroll problems
 *  
 *      TermHeader.h
 *          - fixed CMVC 7869 - USL porting problems
 * 
 * 
 * Revision 1.185  95/01/18  14:29:02  14:29:02  porsche (John Milton)
 * Added CDE copyrights to all files and made the following changes as well:
 *  
 *      DtTermMain.c
 *      	- restored the code that 'Added session file version id to
 *      	  session manager files' (initially added in rev 1.43)
 *  
 *      TermPrim/TermPrimOSDepI.h
 *      	- fixed CMVC 7480 ifdef macro missing on Sun architecture
 * 
 * 
 * Revision 1.184  95/01/03  15:16:56  15:16:56  daves (Dave Serisky)
 * Fixed CMVC7212:  seg violation when destroying one dtterm and then
 * unmapping the dtterm's shell widget.
 * 
 * Revision 1.181  94/12/15  12:19:01  12:19:01  tom (Tom Houser)
 * TermPrim/TermPrim.c
 *  TermPrim/TermPrimBuffer.c
 *  Term/TermParse.c
 *   - CMVC 7001
 *       fix for memory leaks in resize buffer code, mostly related to
 *       using ROWS instead of MAX_ROWS.
 * 
 * 
 * Revision 1.180  94/12/13  16:55:31  16:55:31  tom (Tom Houser)
 * Term.c
 *  TermPrimSelect.c 
 *  TermPrimSelect.h 
 *  TermPrimSelectP.h 
 *    - fix for CMVC 7288
 *        This was a problem with the introduction of new functionality in
 *        Motif for button event configuration.  The functionality just got
 *        turned on in CDE with a change in a system resource.  I added the
 *        functionality to dtterm.
 * 
 * 
 * Revision 1.179  94/12/13  14:52:26  14:52:26  porsche (John Milton)
 * Fixed CMVC 5687
 *  
 *      TermPrim/TermPrimSetPty.c
 *          - _DtTermPrimPtyInit()
 *              The IBM-specific code was calling setcsmap() before STDIN
 *              was properly initialized (setcsmap() operates on STDIN).
 *              We now do some dup'ing to point STDIN at the correct pty
 *              before calling setcsmap().
 *  	   
 *  John Milton
 *  12/13/94
 * 
 * 
 * Revision 1.178  94/12/05  12:43:32  12:43:32  porsche (John Milton)
 * Fixed DTS CISlx17970 (and its CMVC counterpart CMVC 7114)
 *  
 *      TermPrim/TermPrimBufferWc.c
 *          - DtTermPrimBufferClearLineWc() was not properly initializing
 *            the line length in all cases
 *          - added appropriate calls to _DtTermPrimSelectDisown()
 *  
 *  John Milton
 *  12/05/94
 * 
 * 
 * Revision 1.177  94/12/01  10:04:51  10:04:51  porsche (John Milton)
 * Fixed CMVC 6534 (losing enhancement info on resize), and CMVC 6661
 *  (problems with 2 column characters at EOL with auto wrap disabled)
 *  
 *      Term/TermBuffer.c
 *          - _DtTermBufferResize() now copies enhancement info for all
 *            characters on the line, and zero out the rest when we
 *            realloc a lines enhancement buffer (CMVC 6534 fix)
 *  
 *      TermPrim/TermPrimRenderMb.c
 *          - _DtTermPrimInsertTextWc() now adjusts the cursor position
 *            before trying to overwrite the last character on the line
 *            with a 2 column character (CMVC 6661 fix)
 *  
 *  John Milton
 * 
 * 
 * Revision 1.176  94/12/01  08:45:18  08:45:18  daves (Dave Serisky)
 * DtTermMain.c:
 *      - CMVC5938:  fixed ignoredKeysyms resource (was being ignored).
 *      - CMVC6617:  enabled support for -xrm *workspacList functionality. 
 *      - CMVC6342:  fixed maintaining of iconic state between sessions.
 *      - CMVC6209:  added call to _DtEnvControl to init environ for CDE.
 *      - CMVC5940:  fixed tracking of font changes across sessions.
 *  
 *  Term.c:
 *      - CMVC5302:  fixed alt-kp_n composing for IBM.
 *      - CMVC6125:  fixed bold and linedraw fonts for alternate fonts.
 *  
 *  TermParse.c:
 *      - Added USL wide char support.
 *  
 *  TermPrim.c:
 *      - CVMC5067/CMVC5684:  fixed bug with focus tracking.
 *  
 *  TermPrimAction.c:
 *      - CMVC5558:  fixed bug in processing 0x<hexvalue> in string()
 *        action.
 *  
 *  TermPrimCursor.c:
 *      - CMVC5067:  fixed disapearing cursor w/ blinkRate: 0 bug.
 *  
 *  TermPrimDebug.c:
 *      - Additional debug functions.
 *  
 *  TermPrimRender.c:
 *  TermPrimRender.h:
 *      - Added font destroy function (/declaration).
 *  
 *  TermPrim.c:
 *  TermPrimRenderFontSet.c:
 *      - CMVC5969:  fixed bug in switching back to XFontStruct from an
 *        XFontSet.
 *  
 *  TermPrimData.h:
 *  TermPrimRender.c:
 *  TermPrimRenderMb.c:
 *  TermPrimScroll.c:
 *      - CMVC6598:  fixed problem w/ equal jump scroll up/down not updating
 *        the display.
 *  
 *  TermPrimSelect.c:
 *      - Added USL wide char support.
 *  
 *  TermViewMenu.c:
 *  TermViewMenu.h:
 *      - CMVC5940:  fixed tracking of font changes across sessions.
 * 
 * 
 * Revision 1.175  94/11/02  16:07:42  16:07:42  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      - CMVC #5938:  ingoredKeysyms were always default.
 *      - CMVC #5939:  dtterm always wrote the window id to a bogus fd when
 *        the -S option was not used.
 *      - Fixed alignment and NULL ptr problems in the Remote GetValues hook.
 *  
 *  Term.c:
 *      - CMVC #4623:  changed vt220 to dtterm for CDE.
 *  
 *  TermColor.c:
 *      - Removed hpterm specific code and code not used for cde color model.
 *  
 *  TermParse.c:
 *  TermPrim.c:
 *      - CMVC #5685:  correctly pass the keyboard lock status to the status
 *        change callback.
 *  
 *  TermPrimOSDepI.h:
 *      - CMVC #5165:  added OS specific define for Shift / Caps Lock XORing
 *        of alpha chars.
 *  
 *  TermPrimRender.c:
 *      - Removed redundant out of range checking.
 *  
 *  TermPrimSelect.c:
 *      - Removed obsolete code.
 *  
 *  TermPrimSetPty.c:
 *      - Made the RS232 BREAK generation code configurable via the OSDepI.h
 *        file.
 *  
 *  TermPrimSubproc.c:
 *  TermPrimUtil.c:
 *      - Added bba specific code to update the bba database.
 *  
 *  TermView/TermViewMenu.c:
 *      - CMVC #5329:  Changed window and font size toggles to radio
 *        buttons.
 *      - CMVC #5627:  Changed window->exit to window->close.
 * 
 * 
 * Revision 1.174  94/10/20  15:04:55  15:04:55  tom (Tom Houser)
 * fix for CMVC 5682
 * TermPrim.c
 *   - removed obsolete action names for selection from Translation table.
 *   - added all translations relevant to selection in case we support
 * 
 * Revision 1.173  94/10/20  11:42:18  11:42:18  tom (Tom Houser)
 * TermPrimSelect.c
 *   - removed _DtTermPrimSelectDisownIfNecessary() as its no longer used.
 * 
 * Revision 1.172  94/10/19  07:26:10  07:26:10  daves (Dave Serisky)
 * 
 *  dtterm.msg:
 *      - CMVC #5627:  Window->Exit changed to ->Close.
 *      - Fixed bug in comment (missing space after $).
 *      - Added messages for the new CDE help menu.
 *  
 *  TermPrimBuffer.c:
 *      - CMVC #5611:  Insure we malloc a history buffer at least 1 line
 *        long so we don't get a possible NULL pointer back and get
 *        confused.
 *  
 *  TermPrimRender.c:
 *      - CMVC #4032:  In smooth scroll, compensate refreshes by the
 *        pendingScrollLines to insure that we refresh the correct data.
 *  
 *  TermPrimSubproc.c:
 *      - Set the pty's window size at pty initialization to prevent
 *        confusion with the window size.
 *  
 *  TermView.c:
 *      - Fixed online help to use correct callback for help close.
 *      - Fixed online help man page hyperlinks to correctly open the man
 *        page.
 *  
 *  TermViewHelp.h:
 *      - Added another #define for CDE help vs HelpView porting to support
 *        man page hyperlinks.  TermViewMenu.c:
 * 
 * 
 * Revision 1.171  94/10/13  14:47:40  14:47:40  tom (Tom Houser)
 * fix for CMVC 4626 and 5160.  note that previous checked in version
 *  of TermPrimSelect.c (and TermParse.c) are part of these fixes.
 *  TermPrimSelect.c
 *    -  added code to handle scroll region properly.
 * 
 * 
 * Revision 1.170  94/10/12  16:24:32  16:24:32  tom (Tom Houser)
 * CMVC 5174 fix
 *  TermParse.c
 *   - added check to adjust selection in scroll region is set.
 *  TermPrimSelect.c
 *   - added a routine to move the selection if necessary.    
 * 
 * 
 * Revision 1.169  94/09/27  16:05:17  16:05:17  daves (Dave Serisky)
 * CMVC #5137:  TermBuffer will now be initialized, and the pty montored
 * if the WM_STATE is set to IconicState.
 * 
 * Revision 1.168  94/09/27  13:32:38  13:32:38  daves (Dave Serisky)
 * 
 *  ./TermPrim/TermPrimBufferWc.c:
 *      - Merged in sun wcwidth() hack to keep trees in sync.
 *  
 *  ./TermPrim/TermPrim.c:
 *  ./TermPrim/TermPrimCursor.c:
 *  ./TermPrim/TermPrimData.h:
 *      - CISlx16839:  Added local storage for IM spotLocation and focus
 *        so we now only set them when they actually change.
 * 
 * 
 * Revision 1.167  94/09/26  11:30:39  11:30:39  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      - CMVC #4632:  changed clone mode to not try to reproduce current
 *        state.
 *      - CMVC #4704:  changed some vestigal XmN* to DtN*.
 *  
 *  Term.c:
 *  Term.h:
 *      - CMVC #5068:  removed functionKeyLast which was not used, but
 *        causing read from unitialized data.
 *  
 *  TermBuffer.h:
 *      - Fixed #define to prevent repeated includes (see last item).
 *      - CMVC #5068:  fixed NUM_ENHANCEMENT_FIELDS to prevent access past
 *        end of malloc'ed memory.
 *  
 *  TermFunction.c:
 *      - CMVC #4652:  added code to set current fg/bg storage on SGR reset.
 *  
 *  TermP.h:
 *      - Fixed #define to prevent repeated includes (see last item).
 *      - CMVC #5068:  removed functionKeyLast which was not used, but
 *        causing read from unitialized data.
 *  
 *  TermParse.c:
 *      - CMVC #4704:  changed some vestigal XmN* to DtN*.
 *  
 *  TermPrim.c:
 *      - CMVC #4743:  modified -kshMode to use Meta modifier if defined,
 *        else mod1.
 *  
 *  TermPrimAction.c:
 *      - CMVC #4928:  changed invokeAction to check for a valid
 *        transmitString before using it.
 *  
 *  TermPrimBuffer.c:
 *      - CMVC #4661, #4730:  fixed buffer code to correctly deal with a
 *        2 column character at the end of the buffer when calculating buffer
 *        length and width.
 *  
 *  TermPrimData.h:
 *      - Fixed #define to prevent repeated includes (see last item).
 *      - CMVC #4743:  added meta flag for -kshMode.
 *  
 *  TermPrimRenderLineDraw.c:
 *      - CMVC #4506:  Fixed linedraw code to support multi-byte buffers.
 *  
 *  TermPrimSelect.h:
 *      - Fixed #define to prevent repeated includes (see last item).
 *      - Added some missing function prototypes.
 *  
 *  52 other .h files:
 *      - Fixed #define to prevent repeated includes.  Several files did not
 *        have any, several files had wrong ones, and a few files had names
 *        that conflicted with other header files and prevented various
 *        source files from reading in desired header files.
 *  
 *      
 * 
 * 
 * Revision 1.166  94/09/21  14:01:49  14:01:49  tom (Tom Houser)
 * TermPrimSelect.c
 *   - fix for CMVC 5008 - fixed up the problems with the handling of text
 *     type TEXT.  One was that TEXT type was being requested in place of
 *     the correct CS_OF_LOCALE (in handleTargets()) and the other was that
 *     when TEXT is requested _DtTermPrimSelectConvert() should be sending
 *     back text of type CS_OF_LOCALE (although any type conforms to the spec.)
 * 
 * Revision 1.165  94/09/21  10:44:25  10:44:25  tom (Tom Houser)
 * TermPrimSelect.c
 *   - fix for CMVC 5010 - removed text type COMPOUND_STRING
 * 
 * Revision 1.164  94/09/20  10:31:14  10:31:14  tom (Tom Houser)
 * TermPrimSelect.c
 *   - fix for CMVC 4867 - With -sl 1l or -sl 0l options, adjustment of
 *     selection when text moved in buffer didn't work correctly.
 * 
 * Revision 1.163  94/09/15  16:00:50  16:00:50  tom (Tom Houser)
 * TermPrim.c
 *  - fix for CISlx16986.  The problem was that the input method was not being
 *    told when the font changed.  In order to fix this, I updated the
 *    second version of "ascent".  i.e. tw->term.tpd->ascent, the other being
 *    tw->term.ascent.  Some review of why there are two seems necessary.
 * 
 * Revision 1.162  94/09/09  12:02:54  12:02:54  daves (Dave Serisky)
 * 
 *  flags:
 *      - Added additional flags.
 *  
 *  TermPrimBufferWc.c:
 *      - Changed code to replace bogus wide characters with a space and to
 *        not emit an error/warning message.
 *      - Put the ...ValidateLineWc() function under a debug flag so it won't
 *        always be called (and eat up cycles).
 *  
 *  TermPrimRenderMb.c:
 *      - Added code to set the end-of-line wrap flag (copied from the single-
 *        byte file).
 *  
 *  TermPrimSetPty.c:
 *      - Modified the rs232 break code to work on IBM as well as HP.
 * 
 * 
 * Revision 1.161  94/09/06  09:25:23  09:25:23  daves (Dave Serisky)
 * ./Term/TermBuffer.c (daves), lasted locked on 09/02/94 at 14:32
 *  ./Term/TermParse.c (daves), lasted locked on 09/02/94 at 09:03
 *  ./TermPrim/TermPrim.c (daves), lasted locked on 08/31/94 at 10:27
 *  ./TermPrim/TermPrimOSDepI.h (daves), lasted locked on 08/31/94 at 10:12
 * 
 * 
 * Revision 1.160  94/08/29  17:18:47  17:18:47  daves (Dave Serisky)
 * Fixed end of line wrap bug with multi-column characters.
 * 
 * Revision 1.159  94/08/22  16:49:14  16:49:14  tom (Tom Houser)
 * TermPrimSelect.c
 *  - This is to fix CMVC 4088 which describes a problem with scrolling
 *    beyond the last line while doing scroll selection.   These changes
 *    should also fixed some other cosemetic problems.
 * 
 * Revision 1.158  94/08/22  10:04:23  10:04:23  daves (Dave Serisky)
 * 
 *  Term.c:
 *      - CMVC #3228:  added translations for osfCopy, osfCut, and osfPaste
 *        virtual keys.
 *      - CMVC #4020:  added state information for all character enhancement
 *        information.
 *  
 *  TermBuffer.h:
 *  TermBufferP.h:
 *      - CMVC #4020:  Changed #defines for protect and unprotect to
 *        correctly default to unprotected.
 *  
 *  TermData.h:
 *  TermFunction.c:
 *  TermParse.c:
 *      - CMVC #4020:  added state information for all character enhancement
 *        information.
 *  
 *  TermPrim.c:
 *      - Changed catopen per direction from CDE PDR.
 *  
 *  TermPrimSelect.c:
 *      - Fixed deselect code to not restore the cursor when it should be
 *        off.  This was dropping ghost cursors.
 * 
 * 
 * Revision 1.157  94/08/16  16:55:02  16:55:02  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      - CMVC #3677:  Changed Toggle Menubar to Toggle Menu Bar.
 *  
 *  DtTermSyntax.c:
 *      - CMVC #3677:  Changed scrollbar to scroll bar in usage message.
 *  
 *  Dtterm.ad.src:
 *      - Added alternate font list for HPVUE builds.
 *  
 *  dtterm.msg:
 *  TermViewMenu.c:
 *      - CMVC #3677:  Changes ScrollBar/MenuBar to Scroll/Menu Bar.
 *  
 *  Term.c:
 *      - CMVC #2804:  Add local echo mode.
 *      - CMVC #3428:  Added save/restore of missing modes.
 *  
 *  TermData.h:
 *      - CMVC #2804:  Add local echo mode.
 *      - CMVC #3428:  Added save/restore of missing modes.
 *  
 *  TermParse.c:
 *      - CMVC #2804:  Add local echo mode.
 *      - CMVC #3428:  Added save/restore of missing modes.
 *  
 *  TermPrim.c:  TermPrimData.h:
 *      - CMVC #2804:  Add local echo mode.
 *  
 *  TermPrimUtil.c:
 *      - CMVC #3428:  Made logging restartable.
 * 
 * 
 * Revision 1.156  94/08/15  17:02:47  17:02:47  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      - CMVC #3228:  Added code to support osfKeysyms with the exception
 *        of specific keys.
 *      - #ifdef'ed out some additonal terminal server code.
 *      - Added remote getvalues support for the scrollbar window.
 *  
 *  TermParse.c:
 *      - CMVC #3755:  Added clip support for Scroll[Up|Down] so that we
 *        don't chew up more of the window then we want to.
 *  
 *  TermPrimBuffer.c:
 *      - CMVC #3712 (probably):  Added support to clear the deleted lines
 *        before we release them.
 *  
 *  TermPrimBufferWc.c:
 *      - Fixed a typo (missing comment on #endif)
 *  
 *  TermPrimCursor.c:
 *      - CMVC #2708:  Fixed a bug where the last column was incorrectly
 *        rendered when the cursor was on it and we were about to wrap.
 *  
 *  TermViewMenu.c:
 *      - CMVC #3228:  Fixed edit pulldown accelerators so that they actuall
 *        work now.
 * 
 * 
 * Revision 1.155  94/08/08  10:31:49  10:31:49  daves (Dave Serisky)
 * 
 *  TermAction.c:
 *      - CMVC #3218:  Added help, menu, and do actions to vt-edit-key()
 *        action.
 *  
 *  TermBuffer.c:
 *      - CMVC #2710:  ECH wasn't clearing the correct number of enhancement
 *        columns.
 *  
 *  TermFunction.c:
 *      - CMVC #2390:  DL within scrolling region was messing up the lines
 *        below the scrolling region.
 *  
 *  TermParse.c:
 *      - CMVC #3294:  Tab behaviour in regards to the last column and end
 *        of line wrap was not being performed correctly.  The new behaviour
 *        works correctly for the partially implemented curses fix mode.
 *  
 *  TermSendEsc.h:
 *      - CMVC #3218:  Added help, menu, and do strings for vt-edit-key()
 *        action.
 *  
 *  TermPrimBuffer.c:
 *      - CMVC #3555:  Modified buffer code to disown the selection if
 *        necessary when the term buffer contexts are modified (instead of
 *        on cursor on/off).
 *  
 *  TermPrimBufferWc.c:
 *      - CMVC #2710:  ECH was having problems with erase of partial
 *        multi-byte characters
 *  
 *  TermPrimCursor.c:
 *      - CMVC #3555:  Turned off disowning of selection if necessary on
 *        cursor on/off.  The code is now in the Buffer code instead.
 *      - CMVC #760:  Fixed the anoying scroll bar warning when scrolling in
 *        non-jump scroll mode.
 *  
 *  TermPrimRenderMb.c:
 *      - CMVC #2710:  Fixed problem where render code was stopping one
 *        column short (on right) when there were multi-byte characters at
 *        the left edge of the render area.
 *  
 *  TermPrimSelect.c:
 *      - Added code to turn the cursor off before selecting or deselecting
 *        text and to turn it back on after.  This removed some ghost cursor
 *        problems.
 *      - CMVC #3555:  Added code to support disowning selection if necessary
 *        on buffer modify instead of cursor on/off.
 *  
 *  TermPrimSelect.h:
 *      - Added new and missing prototypes for TermPrimSelect.c functions.
 *  
 * 
 * 
 * Revision 1.154  94/08/04  14:51:38  14:51:38  tom (Tom Houser)
 * TermPrimSelect.c
 *  - This fixes CMVC 3620. add code to send DELETE to text widget
 *    after MOVE's.
 * 
 * Revision 1.153  94/08/02  11:25:23  11:25:23  daves (Dave Serisky)
 * 
 *  ./DtTermMain.c:
 *      -CMVC #3546:  dtterm -geometry 40x24 gives 36x21 lines.  Fixed this
 *       and several other geometry related quirks.
 *  
 *  ./terminfoChecklist:
 *      -CMVC #2750:  Line drawing not working from curses on AIX.  Added
 *       missing AIX curses strings.
 *      -Added missing pairs=8 for HP color curses.
 *  
 *  ./TermPrim/TermPrim.c:
 *      -CMVC #3546:  dtterm -geometry 40x24 gives 36x21 lines.  Modified
 *       geometry manager proc to correctly support -geometry, cloning, and
 *       session restart.
 *  
 *  ./TermView/TermView.c:
 *      -Fixed bogus Options->Window Size->default window size.
 *      -CMVC #3546:  dtterm -geometry 40x24 gives 36x21 lines.  Changed
 *       cloning code to get cloning to work correctly for windows with
 *       modified geometry.
 * 
 * 
 * Revision 1.152  94/07/21  10:40:28  10:40:28  porsche (John Milton)
 * Fixed vi delete character defect (no CMVC number at this time)
 *  
 *      TermPrim/TermPrimRender.c
 *      	- _DtTermPrimBufferDelete() was deleting 1 character less than requested
 *  
 *  John
 * 
 * 
 * Revision 1.151  94/07/19  16:12:58  16:12:58  tom (Tom Houser)
 * TermPrimSelect.c
 *  - removed some dead code from _DtTermPrimSelectProcessCancel()
 * 
 * Revision 1.150  94/07/19  13:53:15  13:53:15  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      - CMVC #3041:  Changed some geometry manager practices so that we
 *        correctly deal with clones of resized windows.
 *      - CMVC #3274:  Fixed to save and restore current working directory.
 *      - CMVC #394:  Restore of resized widnows now works because we now
 *        restore window size before realizing it.
 *  
 *  Term/TermData.h:
 *      - CMVC #3274:  Moved cwd from here to TermPrimP.h.
 *  
 *  Term/TermFunction.c:
 *      - CMVC #3274:  Added set/get cwd functions.
 *      - CMVC #2635:  Flush pending text on hard/soft reset.
 *  
 *  Term/TermFunction.h:
 *      - CMVC #3274:  Added declarations for set/get cwd functions.
 *  
 *  Term/TermParse.c:
 *      - CMVC #3274:  Removed memory leak and changed to point at new
 *        location.
 *      - CMVC #2706:  Removed clearing of wrapping indicator before doing
 *        backspace.
 *  
 *  TermPrim/TermPrim.c:
 *      - CMVC #3041:  Cleaned up the geometry manager functions.
 *      - CMVC #3274:  Support for save and restore of current working
 *        directory.
 *  
 *  TermPrim/TermPrimP.h:
 *      - CMVC #3274:  Added storage for cwd.
 *  
 *  TermPrim/TermPrimPendingText.c:
 *      - CMVC #2635:  Added _DtTermPrimPendingTextFlush() function.
 *  
 *  TermPrim/TermPrimPendingText.h:
 *      - CMVC #2635:  Added header for _DtTermPrimPendingTextFlush() function.
 *  
 *  TermPrim/TermPrimSubproc.c:
 *      - CMVC #3274:  Added support for restoring cwd.
 *  
 *  TermPrim/TermPrimSubproc.h:
 *      - CMVC #3274:  Header changes to allow support for restoring cwd.
 *  
 *  TermView/TermView.c:
 *      - CMVC #3041:  Cleaned up the geometry manager functions.
 *  
 *  TermView/TermViewP.h:
 *      - CMVC #3041:  Storage to clean up the geometry manager functions.
 *  
 * 
 * 
 * Revision 1.148  94/07/15  14:51:29  14:51:29  daves (Dave Serisky)
 * 
 *  TermPrim/TermPrim.c:
 *      - CMVC #1769:  Changed code to set the window size before kicking
 *        off the subprocess and not to set it again unless the size
 *        changes.  This is done to minimize (but not fix) the bug that is
 *        actually in ksh.
 * 
 * 
 * Revision 1.147  94/07/14  14:06:10  14:06:10  tom (Tom Houser)
 * TermPrimRepType.c
 *  - tried to make CvtStringToTerminalSizeList() more robust (e.g. for an
 *    incorrect parameter list)
 * 
 * Revision 1.146  94/07/14  11:08:43  11:08:43  porsche (John Milton)
 * Fixing CMVC 3216 and CMVC 3160
 *  
 *      Term/TermBuffer.c
 *      	- changed API to clearEnhancements, it now clears count
 *      	  enhancements starting with col (rather than startCol through stopCol)
 *      	  (This was part of a change to the CLEAR_ENH API in TermBuffer.h)
 *      Term/TermParse.c
 *      	- eliminated what look to be unnecessary calls to 
 *      	  _DtTermPrimRenderPadLine(w)
 *      
 *      TermPrim/TermPrimBuffer.c
 *      	- _DtTermPrimBufferDelete() 
 *      	    - now returns a potentially adjusted value for width
 *                (as opposed to using a locally adjusted value)
 *      	    - checks to make sure it doesn't delete when col > width
 *              - checks to make sure it doesn't delete past the end of
 *      	      the line
 *      	- _DtTermPrimBufferPadLine()
 *      	    - now gives the buffer a chance to clear enhancements when
 *      	      padding a line
 *  
 *      TermPrim/TermPrimBufferWc.c
 *      	- similar changes to WC versions of above functions
 *  
 *      TermPrim/TermPrimBuffer.h
 *      	- changed (*TermEnhClear) API 
 *  
 *  John
 * 
 * 
 * Revision 1.145  94/07/14  09:46:45  09:46:45  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      -Call _DtTermPrimLogFileCleanup() on signal exit.
 *  
 *  TermPrimSelect.c:
 *      -Added debug flags to turn off selection targets.
 *  
 *  TermPrimSubproc.c:
 *      -Added debug flags to force fork() failures.
 *  
 *  TermPrimUtil.c:
 *      -Added linked list of logfiles as well as
 *       _DtTermPrimLogFileCleanup() function to clean up logfiles on signal
 *       exit.
 *  
 *  TermPrimUtil.h:
 *      -Header for _DtTermPrimLogFileCleanup().
 *  
 *  TermView/TermViewGlobalDialog.c:
 *      -Save away old blink rate so that when we turn blinking off and then
 *       on we still have a valid (non 0) value.
 * 
 * 
 * Revision 1.144  94/07/13  17:09:10  17:09:10  tom (Tom Houser)
 * TermPrimRepType.c
 *  - fix for CMVC.  he problem was a core dump when there was junk
 *    in the sizeList string.  e.g dtterm -xrm "dtterm*sizeList:  10x junk"
 * 
 * Revision 1.143  94/07/08  15:17:34  15:17:34  daves (Dave Serisky)
 * 
 *  TermPrimSetPty.c:
 *      - CMVC #2396:  Fixed all ttyModes values for all 3 platforms
 *        and HP-UX 9.* and 10.0 as well.
 *  
 *  TermPrimBufferWc.c:
 *      - CMVC #3111:  Fixed the wcwidth hack function for sun.
 * 
 * 
 * Revision 1.142  94/07/08  15:13:13  15:13:13  daves (Dave Serisky)
 * 
 *  TermPrimSetPty.c:
 *      - CMVC #2396:  Fixed all ttyModes values for all 3 platforms
 *        and HP-UX 9.* and 10.0 as well.
 * 
 * 
 * Revision 1.141  94/07/06  17:02:34  17:02:34  daves (Dave Serisky)
 * 
 *  TermPrim.c:
 *      -Moved _DtTermPrimPtyGetDefaultModes() here to get them before
 *       opening up the pty master.
 *  
 *  TermPrimSubproc.c:
 *      -Removed _DtTermPrimPtyGetDefaultModes() (see above).
 *  
 *  TermParse.c:
 *      -Modified Sun escape sequences to take the window manager frame into
 *       account (providing a mwm type window manager is running).
 *      -Changed the rows and columns from ints to shorts which fixed a bug.
 * 
 * 
 * Revision 1.140  94/07/05  12:56:38  12:56:38  daves (Dave Serisky)
 * DtTermMain.c:
 *      -Added signal handlers for all catchable signals to perform cleanup
 *       upon receipt of signal.
 *  
 *  dtterm.msg:
 *      -CMVC #2666:  Changed help menu labels and mnemonics.
 *      -CMVC #2638:  Added dtterm -  to dialog titles.
 *  
 *  Term.c:
 *      -Changed translations from Meta to Mod1 modifiers.
 *      -CMVC #2870:  Removed modifiers from the definitions for the KP_
 *       keys to allow IBM's back end input method to work.
 *  
 *  TermPrim.c:
 *      -Added XtResource structures for DtNoutputLogCallback and
 *       DtNinputVerifyCallback.
 *      -CMVC #1013:  Release the pty during widget destroy.
 *  
 *  TermPrimGetPty-bsd.c:
 *      -CMVC #1013:  Added a linked list of ptys so that we can release
 *       them when we cleanup.
 *      -Set pty gid to tty and mode to 0620 for HP systems.
 *      -CMVC #2172:  Changed code to use the name pty slave name returned
 *       by ttyname(2) so that it will use the same name as everyone else.
 *  
 *  TermPrimGetPty-clone.c:
 *      -Added dummy cleanup function.
 *      -Set pty gid to tty and mode to 0620 for HP systems.
 *  
 *  TermPrimGetPty-svr4.c:
 *      -Added dummy cleanup function.
 *  
 *  TermPrimGetPty.h:
 *      -Added header for _DtTermPrimPtyCleanup().
 *  
 *  TermPrimSetUtmp.c:
 *      -Added code to maintain a linked list of modified utmp entries as well
 *       as a _DtTermPrimUtmpCleanup() function to clean them all up.
 *  
 *  TermPrimSetUtmp.h:
 *      -Added prototypes for _DtTermPrimUtmpAddEntry and
 *       _DtTermPrimUtmpCleanup().
 *  
 *  TermPrimSubproc.c:
 *      -Register the utmp entries via _DtTermPrimUtmpAddEntry() for later
 *       cleanup.
 *  
 *  TermView.c:
 *      -CMVC #2627:  Added code to raise mapped (help) dialogs when they
 *       are reinvoked.
 *  
 *  TermViewGlobalDialog.c:
 *  TermViewTerminalDialog.c:
 *      -CMVC #2638:  Added dtterm -  to dialog titles.
 *  
 *  TermViewMenu.c:
 *      -CMVC #2666:  Changed help menu labels and mnemonics.
 *      -CMVC #2636:  Added a title to the popup menu.
 *      -CMVC #2627:  Added code to raise mapped dialogs when they are
 *       reinvoked.
 * 
 * 
 * Revision 1.139  94/07/01  14:44:50  14:44:50  tom (Tom Houser)
 * TermPrimSelect.c
 *  - removed some dead code.
 * 
 * Revision 1.138  94/06/30  17:51:46  17:51:46  tom (Tom Houser)
 * TermParse.c
 *   - fixed a bug in ICH when length exceeded line length.         
 *   - replaced type short with type int to eliminate truncation to neg. numbers
 *   - did additional range checking where necessary to truncate arg. to size short
 *  TermPrimParser.c
 *   - change num. parser to keep number from going negative.
 * 
 * 
 * Revision 1.137  94/06/10  13:55:21  13:55:21  tom (Tom Houser)
 * TermPrimSelect
 *  - added newline to return char conversion in clipboard paste.
 *  - fixes CMVC 2647
 * 
 * Revision 1.136  94/06/08  17:15:43  17:15:43  porsche (John Milton)
 * Initializing the tty map info for mb processing in AIX
 *      
 *  
 *      TermPrimSetPty.c
 *      	- _DtTermPrimPtyInit() calls setcsmap() to initialize tty map
 *            info
 *  
 *      TermPrimOSDepI.h
 *      	- define USE_SETCSWIDTH for AIX
 *  
 *  John
 *  
 *  
 *  
 * 
 * 
 * Revision 1.135  94/06/08  16:05:24  16:05:24  daves (Dave Serisky)
 * 
 *  Dtterm.ad.src:
 *      -Removed the leading <n>/ from the font name strings in the
 *       terminal font array list.  This was keeping dtterm from using
 *       the font point sizes in the font size change menu.
 *  
 *  Term/Term.h:
 *      -Expanded the #include of TermPrim.h by including the source
 *       in place.
 *  
 *  TermPrim/TermPrim.c:
 *      -Added code to the map event handler to map a pending warning
 *       dialog.  This fixes the problem of the dialog being mapped before
 *       the terminal window is brought up.
 *      -Changed the callbacks to match those documented in the DtTerm(3)
 *       man page.
 *      -Fixed the stop action to match the one documented (toggle, on,
 *       off).
 *  
 *  TermPrim/TermPrim.h:
 *      -Added resources for the documented callbacks that were not yet
 *       implemented.
 *      -Removed the obsolete callback structures and #defines.
 *  
 *  TermPrim/TermPrimCursor.c:
 *      -Changed the cursor change callback to use the generic status
 *       change callback from the spec.
 *  
 *  TermPrim/TermPrimI.h:
 *      -Prototype for new status change callback function.
 *  
 *  TermPrim/TermPrimP.h:
 *      -Added code for new callbacks.
 *  
 *  TermView/TermView.c:
 *      -Added a map notify handler to set the default rows and columns to
 *       the true initial size.
 *  
 *  TermView/TermViewMenu.c:
 *      -Changed font and window size menus to toggle buttons.
 *  
 *  TermView/TermViewP.h:
 *      -Added currentFontToggleButtonIndex to TermView structure to keep
 *       track of the currently active font from the font menu.
 * 
 * 
 * Revision 1.134  94/06/07  17:29:51  17:29:51  porsche (John Milton)
 * Initializing the cswidth info for mb processing in HP-UX 10.0
 *      
 *      TermPrim.c
 *      TermPrim.h
 *      TermPrimP.h
 *      	- added csWidth resource
 *  
 *      TermPrimSetPty.c
 *      	- now initializing cswidth information for ldterm (depends
 *      	  on USE_CSWIDTH being defined in TermPrimOSDepI.h)     
 *      	- _DtTermPrimPtyInit()
 *      	    - now accepts a 3rd parameter, csWidthString
 *      	    - initializes ldterm cswidth information from the
 *      	      reference tty if possible, else from csWidthString
 *  
 *      	- parseCSWidth()
 *      	    - support routine for _DtTermPrimPtyInit()
 *   
 *      	- _DtTermPrimPtyGetDefaultModes() gets cswidth info from
 *            reference tty when possible
 *  
 *      TermPrimSetPty.h
 *      	- updated _DtTermPrimPtyInit() prototype
 *  
 *      TermPrimSubproc.c
 *      	- added value of csWidth to _DtTermPrimPtyInit() call
 *  
 *      TermPrimOSDepI.h
 *      	- define USE_CSWIDTH for HPUX 10.0
 *  
 *  John
 *  
 *  
 *  
 * 
 * 
 * Revision 1.133  94/06/07  15:48:53  15:48:53  tom (Tom Houser)
 * TermPrimSelect.c
 *  - fixed selection of page.
 * 
 * Revision 1.132  94/06/07  10:17:21  10:17:21  porsche (John Milton)
 * Fixing defects:
 *      
 *      TermPrimBuffer.c
 *      	- _DtTermPrimGetCharacterInfo() returns some reasonable
 *            defaults if the specified column falls on or past the end of
 *      	  the line (col >= WIDTH(line)).  This fixes the problem of
 *      	  trying to delete the last character of a line in wide
 *      	  character locales.
 *  
 *      TermPrimBufferWc.c
 *      	- _DtTermPrimBufferPadLineWc() was padding 1 too many characters
 *      	  (it was overwriting the existing last character on the end of
 *      	  the line), this fixes CMVC defect # 2397
 *  
 *  John
 *  
 *  
 *  
 * 
 * 
 * Revision 1.131  94/06/03  10:10:33  10:10:33  tom (Tom Houser)
 * TermPrimSelect.c
 *   - Added same fix to MB selection that was done in rev 1.40 for single byte
 *      i.e., fixed precisely the legal mouse range for word selection.
 *   - totally rewrote MB word selection to parallel the single byte logic.
 *   - simplified the single byte version a bit.
 * 
 * Revision 1.130  94/06/03  08:36:35  08:36:35  daves (Dave Serisky)
 * 
 *  TermPrimSetSubproc.c:
 *      -CMVC defect #2084:  Removed some extraneous code that was causing
 *       the pty to be initialized using the raw pty as a reference if there
 *       was no true reference to work off of.
 * 
 * 
 * Revision 1.129  94/06/02  16:17:30  16:17:30  tom (Tom Houser)
 * TermPrimSelect.c
 *  - added code for precise check to determine if mouse is in selected text.
 *    This code eliminates the half-char extension on either end of the
 *    selected text which was being included as part of the selected text.
 * 
 * Revision 1.128  94/06/02  13:44:02  13:44:02  tom (Tom Houser)
 * TermPrimSelect.c
 *   - This fixes the off by one-half character problem with selection.
 *     Selecting a word one-half char to the left or right of the word should
 *     select it.  Selecting within one-half char to the right of the word
 *     did not work.  This fix was for one char sets with only single column
 *     chars.
 * 
 * Revision 1.127  94/06/01  09:40:10  09:40:10  porsche (John Milton)
 * Fixing defects:
 *      
 *      TermPrimBuffer.c
 *      	- _DtTermPrimBufferErase() returns if startCol > stopCol (this
 *      	  was causing core dumps before).  This fixes CMVC #1946, and
 *      	  CMVC #1993
 *  
 *  John
 *  
 *  
 *  
 * 
 * 
 * Revision 1.126  94/06/01  07:45:27  07:45:27  daves (Dave Serisky)
 * 
 *  Term/TermBuffer.c:
 *      -CMVC #621:  <esc>[K (EL) leaves ghost enhancements.  Defect has
 *       been fixed.
 *  
 *  TermPrim/TermPrimBuffer.c:
 *      -Added debug flag B:1 to check the text buffer before and after line
 *       moves to check for corruption (dropped lines, duplicate lines).
 *  
 *  TermPrim/TermPrimGetPty-bsd.c:
 *  TermPrim/TermPrimGetPty-clone.c:
 *  TermPrim/TermPrimGetPty-svr4.c:
 *      -Added debug flag p:10 to force pty allocation failures.
 *  
 *  TermPrim/TermPrimParser.c:
 *      -Added debug flag p:1 and env variable dttermDebugParseChar to allow
 *       breakpointing on a specific character in the input stream.  This
 *       allows insertion of a character in the input stream and triggering
 *       a breakpoint when that character is parsed.
 * 
 * 
 * Revision 1.125  94/05/23  17:42:23  17:42:23  tom (Tom Houser)
 * Term.c
 *  TermPrim.c
 *  TermPrimSelect.c
 *  TermPrimSelect.h
 *    -  added cancel to selection.
 * 
 * 
 * Revision 1.124  94/05/20  09:14:35  09:14:35  porsche (John Milton)
 * Two changes:
 *      1) laid groundwork for working around lack of wcwidth() on
 *         various platforms
 *      2) moving cursor past last character on line doesn't automatically
 *         pad the line
 *  
 *  
 *      TermPrimBuffer.h
 *      	- now including TermPrimOSDepI.h to handle OS specific
 *      	  workarounds for wcwidth()
 *      	- added conditional prototypes for usl_wcwidth() and
 *      	  sun_wcwidth()
 *  
 *      TermPrimBufferP.h
 *      	- removed conditional defines of wcwidth()
 *  
 *      TermPrimBufferWc.c
 *      	- now including TermPrimOSDepI.h to handle OS specific
 *      	  workarounds for wcwidth()
 *      	- added conditional compilation blocks usl_wcwidth() and
 *      	  sun_wcwidth()
 *  
 *      TermPrimOSDepI.h
 *      	- added conditional defines of wcwidth()
 *  
 *      TermPrimRenderMb.c
 *      	- now including TermPrimOSDepI.h to handle OS specific
 *      	  workarounds for wcwidth()
 *  
 *  
 *      TermPrimCursor.c
 *      	- _DtTermPrimCursorMove() no longer pads lines when the cursor
 *      	  is placed past the last character on a line
 *  
 *  John
 * 
 * 
 * Revision 1.123  94/05/19  14:24:22  14:24:22  tom (Tom Houser)
 * TermPrimParser.h
 *   - added declaration of numeric stack function _DtTermPrimParserNumParmPush()
 * TermPrimParser.c
 *   - parameterized the numeric stack and added a range check.
 * 
 * Revision 1.122  94/05/19  10:00:32  10:00:32  daves (Dave Serisky)
 * 
 *  Term/Term.c:
 *  Term/TermData.h:
 *  Term/TermParse.c:
 *  Term/TermParseTable.c:
 *  Term/TermParseTable.h:
 *      -Added support for single shift font selections.
 *      -Added support for G2 and G3 font selection.
 *  
 *  TermPrim/TermPrim.c:
 *  TermPrim/TermPrimP.h:
 *      -Added an insert proc to the TermPrim class part structure for use
 *       in inserting text into the text buffer.
 *      -Added an allowOsfKeysyms flag to the TermPrim widget instance
 *       part to enable OSF keysyms when we want them to get through
 *       (such as during cut, paste, and drag operations).
 *  
 *  TermPrim/TermPrimRender.c:
 *      -Changed code to call the text insert proc from the TermPrim class
 *       part when inserting text instead of the default TermPrim insert
 *       function.
 * 
 * 
 * Revision 1.121  94/05/13  17:34:11  17:34:11  porsche (John Milton)
 * Sync'd up with CDE Developer's Environment changes
 *  
 *      TermPrim.c
 *      TermPrim.h
 *      TermPrimCursor.c
 *      TermPrimData.h
 *      TermPrimI.h
 *      TermPrimRender.c
 *      TermPrimRenderMb.c
 *      TermPrimRepType.c
 *      TermPrimSetUtmp.c
 *      TermPrimUtil.c
 *      TermPrimVersion.c
 *  
 *  John
 * 
 * 
 * Revision 1.120  94/05/09  10:39:17  10:39:17  daves (Dave Serisky)
 * 
 *  Imakefile:
 *      -dtterm.ti terminfo file.
 *      -changed code to generate executable mkfallbk from mkfallbk.sh.
 *       This takes care of the problem where revision control systems don't
 *       restore execute bits.
 *      
 *  Term/TermColor.c:
 *      -Added debug flags to force color allocation failures.
 *      -Changed the halfbright algorithm to use 75% instead of 50%
 *       intensity.
 *  
 *  TermPrim/TermPrim.c:
 *  TermPrim/TermPrimDebug.c:
 *  TermPrim/TermPrimRender.c:
 *  TermPrim/TermPrimRenderFont.c:
 *  TermPrim/TermPrimRenderFontSet.c:
 *  TermPrim/TermPrimRenderLineDraw.c:
 *  TermPrim/TermPrimRenderMb.c:
 *  TermPrim/TermPrimSubproc.c:
 *      -added #pragma_BBA_IGNOREs.
 *  
 *  TermPrim/TermPrimGetPty-svr4.c:
 *      -added debug timestamps.
 * 
 * 
 * Revision 1.119  94/05/05  11:41:13  11:41:13  porsche (John Milton)
 *     TermPrim.c
 *      	Fixed an off by one error in _mergeEnv (the strncmp wasn't including
 *      	the '=' sign in the comparison).
 *  
 *  John
 * 
 * 
 * Revision 1.118  94/05/02  12:57:59  12:57:59  porsche (John Milton)
 * Eliminated some memory leaks...
 *  
 *      TermPrim.c
 *      	- Destroy() now frees the history buffer if it exists...
 *  
 *      TermPrimBuffer.c
 *      	- _DtTermPrimBufferCreateBuffer() no longer initializes the 
 *      	  BUFFER_FREE method, we depend on the inheriting class to 
 *      	  do so
 *  
 *      TermPrimBufferWc.c
 *      	- eliminated some obsolete code to reduce the number of
 *      	  branches
 *  
 *  John
 * 
 * 
 * Revision 1.117  94/04/26  11:02:26  11:02:26  daves (Dave Serisky)
 * 
 *  TermPrimSetSubproc.c:
 *      -CMVC defect #2084:  Removed some extraneous code that was causing
 *       the pty to be initialized using the raw pty as a reference if there
 *       was no true reference to work off of.
 * 
 * 
 * Revision 1.116  94/04/13  10:39:58  10:39:58  daves (Dave Serisky)
 * 
 *  TermPrim/Imakefile:
 *      -Removed OS defines now in TermPrimOSDepI.h.
 *      -Added architecture -D defines to trigger the above.
 *  
 *  TermPrim/TermPrim.c:
 *  TermPrim/TermPrimMessageCatI.h:
 *  TermPrim/TermPrimSetPty.c:
 *      -Added #include of TermPrimOSDep.h.
 *  
 *  TermPrim/TermPrimSubproc.c:
 *      -Removed obsolete streams #defines and #includes.
 *  
 *  TermPrim/TermPrimUtil.c:
 *      -Added #include of TermHeader.h
 *      -Added #include of TermPrimOSDep.h.
 *  
 *  TermPrim/TermPrimOSDepI.h:
 *      -New header file for OS dependencies.
 *  
 * 
 * 
 * Revision 1.115  94/04/12  14:48:11  14:48:11  porsche (John Milton)
 * Fixed a core dumper...
 *  
 *      TermPrimBuffer.c
 *      	- _DtTermPrimBufferGetLineLength() returns the right value
 *      	  when the line width == 0 (it was returning bogus information
 *            in the case that the line width == 0)
 *      	- _DtTermPrimBufferClearLine() got cleaned up a bit...
 *  
 *      TermPrimRender.c
 *      	- _DtTermPrimFillScreenGap() only copies lines with length > 0
 *      	  into the history buffer
 *      	  
 *  John
 * 
 * 
 * Revision 1.114  94/04/12  13:35:33  13:35:33  tom (Tom Houser)
 * TermPrim.c
 *   -added the select-page and select-all actions which I accidently removed
 *    while eliminating dead code.
 * 
 * Revision 1.113  94/04/12  09:25:02  09:25:02  daves (Dave Serisky)
 * 
 *  TermPrimSetPty.c:
 *      - Changed code to use the reference tty terminfo state when setting
 *        the new terminfo state.
 *  
 *  TermPrimSubproc.c:
 *      - Changed code to get the reference terminfo state before calling
 *        setpgrp().
 *      - Moved setpgrp() call before the pty/stream open.
 * 
 * 
 * Revision 1.112  94/04/11  17:21:55  17:21:55  tom (Tom Houser)
 * TermPrim.c
 *   - fixed some action names
 * 
 * Revision 1.111  94/04/08  17:21:57  17:21:57  tom (Tom Houser)
 * TermPrim.c TermPrim.h TermPrimP.h
 *    - added allowSendEvents resource
 * 
 * Revision 1.110  94/04/08  10:35:24  10:35:24  tom (Tom Houser)
 * TermPrimSelect.c
 *   - changed all occurances of wint_t to wchar_t which is POSIX PG4
 * 
 * Revision 1.109  94/04/08  10:27:08  10:27:08  porsche (John Milton)
 * Latest snapshot of multi-byte functionality...
 *  
 *      TermPrimBuffer.c
 *      	- _DtTermPrimBufferInsert() calls _DtTermPrimBufferInsertWc() if in
 *      	  multibyte locales
 *      	- _DtTermPrimBufferGetLineLength() returns the right value (it
 *      	  was off by 2 in some cases)
 *  
 *      TermPrimBufferWc.c
 *      	- _primBufferInsertWc(), and _primBufferOverwriteWc() were
 *      	  passing the wrong value of maxWidth to _countWidth() which
 *      	  could cause too few characters to be inserted into the buffer
 *  
 *      TermPrimRenderMb.c
 *      	- _DtTermPrimFillScreenGap() was using the line width instead
 *      	  of the line length when copying lines from the term buffer
 *      	  into the history buffer
 *      	  
 *  John
 * 
 * 
 * Revision 1.108  94/04/07  17:22:01  17:22:01  daves (Dave Serisky)
 * 
 *  Imakefile:
 *      -Turned on streams and changed to TermPrimGetPty-svr4.c for HP-UX
 *       os's >9 (i.e., 10.0).
 *  
 *  TermPrimSubproc.c:
 *      -Removed the old pty fd manipulation that was part of the attempt to
 *       speed up life by using vfork().
 *  
 *  TermPrimGetPty-svr4.c:
 *      -Added the missing unlockpt() that I accidentally lost.
 *      -Removed the code that open'ed upu the pty after calling ptsname()
 *       since we weren't doing anything with the file descriptor before we
 *       closed it.
 *  
 *  TermPrimSetUtmp.c:
 *      -Changed the code that generates the ut_id for HP-UX to take into
 *       account the different pty/<n> name which we need to try and stuff
 *       into 4 characters.
 *  
 *  TermPrimSetPty.c:
 *      -Removed the STREAMS dependent code since this is now being done via
 *       the _DtTermPrimSetupPty() which is specific to the medium (streams
 *       or pty).
 * 
 * 
 * Revision 1.107  94/04/07  10:59:42  10:59:42  tom (Tom Houser)
 * TermPrimSelect.c
 *   - added ifdef for sun because they're not PG4 compiant
 * 
 * Revision 1.106  94/04/06  08:07:43  08:07:43  daves (Dave Serisky)
 * 
 *  Term/Term.c:
 *  TermPrim/TermPrim.c:
 *      -Cleaned up debug timestamps.
 *  
 *  Term/TermColor.c:
 *      -Fixed a bug that was causing an X error freeing an unallocated
 *       halfbright color on exit.
 *      -Changed algorithm to generate halfbright correctly.
 *  
 *  
 *  TermPrim/TermPrimGetPty-bsd.c:
 *      -Removed the code that changes ownership and mode on pty open.  This
 *       is done as part of the pty setup.
 *      -Changed _DtTermPrimSetupPty to take a file descriptor as well
 *       (needed for streams version) and to return success/failure.
 *  
 *  TermPrim/TermPrimGetPty-clone.c:
 *      -Changed _DtTermPrimSetupPty to take a file descriptor as well
 *       (needed for streams version) and to return success/failure.
 *  
 *  TermPrim/TermPrimGetPty-svr4.c:
 *      -Removed ptem.
 *      -Modified the #define sequence for the PTY_CLONE_DEVICE since
 *       it seems to be the same for everyone.
 *      -Changed the streams init code to perform the initialization after
 *       the pty is opened for actual use.  This code should work for HP,
 *       Sun and USL without OS specific code.
 *  
 *  TermPrim/TermPrimGetPty.h:
 *      -Changed prototype for _DtTermPrimSetupPty().
 *  
 *  TermPrim/TermPrimSubproc.c:
 *      -Yanked out the USL specific streams initialization code.  This is
 *       now done for all streams OS's in the _DtTermPrimSetupPty()
 *       function.
 *  
 *  TermView/TermView.c:
 *      -Cleaned up debug timestamps.
 *      -Removed some dead code.
 * 
 * 
 * Revision 1.105  94/04/06  08:06:07  08:06:07  daves (Dave Serisky)
 * 
 *  TermPrim.c:
 *      -CMVC defect #1535:  #ifdefed around setting of 80 and 24
 *       for systems such as Sun that don't fully support it.
 *  
 *  Imakefile:
 *      -CMVC defect #1535:  added -Ddefines for above functionality on
 *       platforms that support 80 and 24.
 * 
 * 
 * Revision 1.104  94/04/05  14:28:21  14:28:21  tom (Tom Houser)
 * TermPrimSelect.c
 *   -calling wrong routine was causing a corner case bug involving
 *    multiclick followed by drag when cursor starts within threshold pixels
 *    of edge of char.
 * 
 * Revision 1.103  94/04/04  13:23:53  13:23:53  tom (Tom Houser)
 * TermPrimSelect.c
 *   - sent Copied text to pty rather than only insert it in the text buffer.
 * 
 * Revision 1.102  94/04/04  09:41:19  09:41:19  daves (Dave Serisky)
 * 
 *  TermPrim.c:
 *      -CMVC defect #1535:  #ifdefed around setting of 80 and 24
 *       for systems such as Sun that don't fully support it.
 *  
 *  Imakefile:
 *      -CMVC defect #1535:  added -Ddefines for above functionality on
 *       platforms that support 80 and 24.
 * 
 * 
 * Revision 1.101  94/03/31  10:14:22  10:14:22  daves (Dave Serisky)
 * TermPrim.c:
 * 	-CMVC defect #659.  Dtterm now the buffer to the right number of
 * 	 rows and columns when it is started (via interactive placement)
 * 	 in a size other than the default.
 * 
 * Revision 1.100  94/03/29  14:40:30  14:40:30  tom (Tom Houser)
 * TermPrimSelect.c        
 *    -  added paste and copy logic
 *  TermPrimSelect.h
 *    -  added interface declarations for paste and copy
 * 
 * 
 * Revision 1.99  94/03/29  11:00:59  11:00:59  tom (Tom Houser)
 * TermPrimSelect.c        
 *    - used a different function in _DtTermPrimSelectExtendEnd() which 
 *      fixes a bug in selecting past the end of text.
 *    - corrected parm to _DtTermPrimBufferGetText()
 * 
 * 
 * Revision 1.98  94/03/28  09:41:32  09:41:32  porsche (John Milton)
 * Latest snapshot of multi-byte functionality...
 *  
 *      TermPrimBuffer.c
 *      	- some more work on _DtTermPrimBufferResizeBuffer()
 *      	  (it wasn't updating COLS if the buffer width decreased)
 *      	- _DtTermPrimGetCharacterInfo() fakes a TermCharInfoRec if the
 *      	  desired col exceeds the width of the line.  This helps deal
 *      	  with operations on the end-of-line
 *      	- _DtTermPrimBufferDelete() now deletes multiple characters
 *      	- implemented _DtTermPrimBufferErase()
 *  
 *      TermPrimBuffer.h
 *      	- new prototypes:
 *      	    _DtTermPrimBufferErase()
 *      	    _DtTermPrimBufferEraseWc()
 *      	- modified prototypes:
 *      	    _DtTermPrimBufferDelete()
 *      	    _DtTermPrimBufferDeleteWc()
 *  
 *      TermPrimBufferWc.c
 *      	- more work on multi-column/byte character management
 *      	- implemented _DtTermPrimBufferEraseWc()
 *  
 *  
 *  John
 * 
 * 
 * Revision 1.97  94/03/25  09:36:38  09:36:38  daves (Dave Serisky)
 * ./Term/TermParse.c:
 *      -Added prelim support for reverse video.  Probably needs additional
 *       support (i.e., for save/restore cursor, reset, etc).
 *      -Fixed the function calls to make the cursor visible/invisible.
 *  
 *  ./TermPrim/TermPrim.c:
 *      -Added reverse video resource.
 *      -Added setvalues support for reverse video.
 *      -Added initialization for the cursorVisible bit.
 *  
 *  ./TermPrim/TermPrim.h:
 *      -Added defines for *NreverseVideo and *CReverseVideo.
 *  
 *  ./TermPrim/TermPrimCursor.c:
 *      -Added support for cursor visible.
 *  
 *  ./TermPrim/TermPrimCursor.h:
 *      -Added function prototypes for _DtTermPrimSetCursorVisible and
 *       _DtTermPrimGetCursorVisible().
 *  
 *  ./TermPrim/TermPrimData.h:
 *      -Added cursorVisible flag.
 *  
 *  ./TermPrim/TermPrimP.h:
 *      -Added reverseVideo flag.
 *  
 *  ./TermPrim/TermPrimRender.c:
 *      -Added support for reverse video flag.
 * 
 * 
 * Revision 1.96  94/03/24  11:41:15  11:41:15  daves (Dave Serisky)
 * Term/TermParseTable.c:
 *      -Added a first pass at a pre-parse table for C0 and C1 control
 *       characters.
 *      -Fixed a few typeos in the C1 (eight-bit) control characters.
 *  
 *  TermPrim/TermPrimParser.c:
 *      -Added support for the prestate parse table.
 *      -Added support for NULL states and actions.
 *  
 *  TermPrim/TermPrimParserP.h:
 *      -Added the statePreParseEntry to the parser state structure.
 *  
 *  TermPrim/TermPrimRender.c:
 *      -Added table driven support for 8-bit control characters.  Now both
 *       the 7- and 8-bit control characters kick us into the parser.
 * 
 * 
 * Revision 1.95  94/03/23  17:21:58  17:21:58  porsche (John Milton)
 * Latest snapshot of multi-byte functionality...
 *  
 *      TermPrim/TermPrimBuffer.c
 *      	- _DtTermPrimBufferGetText() is now wchar smart, and honors
 *      	  the state of needWideChar
 *  
 *          - fixed a bug in _DtTermPrimGetCharacterInfo()
 *  
 *      	- _DtTermPrimBufferSetLineWidth() now sets LENGTH only in
 *      	  single byte locales (it is the responsibility of wchar-smart
 *      	  callers of _DtTermPrimBufferSetLineWidth() to set LENGTH
 *      	  correctly)
 *  
 *      	- _DtTermPrimBufferClearLine() is now wchar smart (it calls
 *      	  _DtTermPrimBufferClearLineWc())
 *  
 *      TermPrim/TermPrimBufferWc.c
 *      	- complete rewrite of _DtTermPrimBufferInsertWc() including
 *      	  several helper functions
 *      
 *      	- _DtTermPrimBufferClearLineWc() now does the right thing
 *  
 *      	- eliminated redundant code
 *  
 *      Term/TermBuffer.c
 *      	- started work on getting correct behavior when
 *      	  deleting/erasing in lines containing multi-column characters
 *  
 *  
 *  John
 * 
 * 
 * Revision 1.94  94/03/22  16:25:32  16:25:32  tom (Tom Houser)
 * TermPrimSelect.c
 *   - added code to covert wide char to multibyte.
 * 
 * Revision 1.93  94/03/21  17:29:34  17:29:34  tom (Tom Houser)
 * TermPrimUtil.c
 *   - added code to catch pipe closed signal (for logging to pipe)
 * TermPrim.c
 *   - changed interface to _DtTermPrimWriteLog() to since we need the
 *     widget for saving for _DtTermPrimLogPipe()
 * TermPrimUtil.h
 *   - changed declaration of _DtTermPrimWriteLog()
 * 
 * Revision 1.92  94/03/18  17:20:27  17:20:27  tom (Tom Houser)
 * TermPrim.c        
 *    - made dtterm a drop site for Move
 *    - added code for MB character selection  (can't test because of buffer 
 *        problems)
 *    - added logic for MB word selection - this works      
 *    - removed some dead code
 * 
 * 
 * Revision 1.91  94/03/18  15:42:44  15:42:44  porsche (John Milton)
 * The multi-byte implementation continues...
 *  
 *      TermPrimRenderMb.c
 *      TermPrimBuffer.c
 *      TermPrimBufferWc.c
 *      	- sync up with changes to TermCharInfoRec
 *      
 *      TermPrimBuffer.h
 *      	- TermCharInfoRec now uses a union for pointing to a character
 *      	  (reduces type casting, and make it easier to increment pointers)
 *  
 *  John
 * 
 * 
 * Revision 1.90  94/03/18  15:04:32  15:04:32  tom (Tom Houser)
 * TermPrimUtil.c and TermPrimUtil.h
 *   added some utilities for cleaning up logging.
 * 
 * Revision 1.89  94/03/17  14:05:42  14:05:42  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      - CMVC 1249:  changed the atom used for the toggle menubar client
 *        message from 0 (None) to a uniqueue atom (_DTTERM_TOGGLE_MENUBAR).
 *  
 *  dtterm.msg:
 *      - changed the string for the above from a static string with a
 *        hardcoded value to an sprintf format string.
 *  
 *  TermPrim/TermPrimSubproc.c:
 *      - changed the DtTermSubprocReap() function to not queue up a destroy
 *        of the widget if it is in the process of being destroyed.  This
 *        was causing the structure passed through the timeout to be bogus
 *        because it was free'ed (and possibly modified) before it was used
 *        again.
 * 
 * 
 * Revision 1.88  94/03/15  10:32:27  10:32:27  daves (Dave Serisky)
 * ./DtTermMain.c:
 *      -Turned fallback resources back on.
 *      -Changed TermView instance names from Dtterm to dtTermView for
 *       consistency.
 *  
 *  ./Dtterm.ad.src:
 *      -Changed to reflect new widget instance names.
 *      -Changed to reflect new Visual Designer appearance recomendations.
 *      -Changed to enable the userFontList in the app-defaults file.
 *  
 *  ./Imakefile:
 *  ./Term/Imakefile:
 *  ./TermPrim/Imakefile:
 *  ./TermView/Imakefile:
 *      -Cleanup and changes from CDE build tree.
 *  
 *  ./Term/Term.h:
 *      -Changed #include <TermPrim.h> to <Dt/TermPrim.h>
 *  
 *  ./Term/admindefines:
 *  ./TermPrim/admindefines:
 *  ./TermView/admindefines:
 *      -Added CDE defines for non Build_HpVue builds.
 *  
 *  ./TermPrim/TermPrim.c:
 *      -Turned on backgroundIsSelect for VUE, off for CDE.
 *      -SUID rework.
 *  
 *  ./TermPrim/TermPrimBufferP.h:
 *      -Added USL define for wcwidth since usl doesn't support it yet
 *       either.
 *  
 *  ./TermPrim/TermPrimGetPty-bsd.c:
 *  ./TermPrim/TermPrimGetPty-clone.c:
 *      -Added code to change pty owner/group.
 *      -Added new setuid code.
 *  
 *  ./TermPrim/TermPrimGetPty-svr4.c:
 *      -Added support for USL.
 *      -Added dummy _DtTermPrimSetupPty() and _DtTermPrimReleasePty()
 *       functions.
 *  
 *  ./TermPrim/TermPrimGetPty.h:
 *      -Added prototypes for new functions.
 *  
 *  ./TermPrim/TermPrimSetUtmp.c:
 *      -Added new setuid code.
 *  
 *  ./TermPrim/TermPrimSubproc.c:
 *      -Added USL streams hacks.
 *      -Added new setuid code.
 *  
 *  ./TermPrim/TermPrimUtil.c:
 *      -Fixed up log code to resolve -ESlit core dumper.
 *      -Added new setuid routines.
 *  
 *  ./TermPrim/TermPrimUtil.h:
 *      -Added headers for new suid routines.
 * 
 * 
 * Revision 1.87  94/03/11  17:17:41  17:17:41  tom (Tom Houser)
 * TermPrimSelect.c
 *   - added code for word selection for multibyte code.
 *   - compiled, linked and ran but can't test until MB code works.
 * 
 * Revision 1.86  94/03/11  14:24:12  14:24:12  tom (Tom Houser)
 * TermPrim.c        
 *    - added the function _DtTermPrimGetMessage() (moved from Term.c)
 *  TermPrimMessageCatI.h
 *    - changed the name of _DtTermGetMessage() to _DtTermPrimMessage()
 *  Term.c 
 *    - removed _DtTermGetMessage()
 * 
 * 
 * Revision 1.85  94/03/11  13:53:44  13:53:44  porsche (John Milton)
 * Complete rewrite of the buffer resize routines.  The term buffer now
 *  remembers its maximum size, and only allocs new memory if the either
 *  of the maximum dimensions is exceeded.
 *  
 *      TermPrimBuffer.c
 *      	- _DtTermPrimBufferCreateBuffer() now initializes MAX_ROWS and
 *             MAX_COLS
 *          - _DtTermPrimBufferResizeBuffer() only allocs new memory when
 *            necessary, and even then it doesn't make an entirely new
 *      	  term buffer, it simply allocs only the memory it needs
 *      
 *      TermPrimBuffer.h
 *      	- new function prototypes for _DtTermPrimBufferResizeBuffer()
 *  
 *      TermPrimBufferP.h
 *      	- added maxRows and maxCols to struct _TermBufferPart
 *      	- added macros for accessing maxRows and maxCols
 *  
 *      TermPrim.c
 *      	- modified InitOrResizeTermBuffer()
 *      	    - to accomodate the new DtTermPrimResizeBuffer() functionality,
 *      	    - to limit the minimum buffer size to 1 row by 1 column
 *                (prevents divide by zero errors)
 *      	    - snap screen to row 0 of the term buffer before resizing (when
 *                we have a history buffer)
 *  
 *      	- modified VerticalScrollBarCallback(), it now calls
 *            _DtTermPrimCursorOff() on entry to prevent cursor droppings
 *  
 *  John
 * 
 * 
 * Revision 1.84  94/03/11  10:45:03  10:45:03  tom (Tom Houser)
 * TermPrimVersion.c
 *   - added MB adjustment for setting the selection anchor.  Most likely
 *       incorrect since I couldn't test it.
 *   - fixed a bug for selection when selecting in non text areas.
 *   - removed some dead code and comments.
 * 
 * Revision 1.83  94/03/09  11:16:16  11:16:16  tom (Tom Houser)
 * TermPrimUtil.c
 *   - emoved code for lowering euid to user. Don't need this any longer
 *     since dtterm will be running as euid 'user' at this point anyway.
 *   - removed old comments.
 * 
 * Revision 1.82  94/03/04  16:30:41  16:30:41  tom (Tom Houser)
 * TermPrimP.h
 *   - removed catgets macro and put it in a separate file
 *     (TermPrimMessageCatI.h)
 * 
 * Revision 1.81  94/03/02  16:48:44  16:48:44  tom (Tom Houser)
 * TermPrimSelect.c
 *   - added check for eol-wrap.
 *   -fixed a corner case bug with extending the selection to the left of the
 *    window
 * 
 * Revision 1.80  94/03/01  16:12:20  16:12:20  porsche (John Milton)
 * Fixed a bug in the line wrap flag functionality.
 *  
 *      TermPrimRender.c
 *          - _DtTermPrimFillScreenGap() was moving the state of the line
 *            wrap flag the wrong way.
 *      
 *  John
 * 
 * 
 * Revision 1.79  94/03/01  15:37:46  15:37:46  daves (Dave Serisky)
 * DtTermMain.c:
 *      -Changed widget names in hierarchy to make them more consistent.
 *  
 *  Dtterm.ad.src:
 *      -Changed widget names in hierarchy to make them more consistent.
 *      -Changed visuals to match the visual designers spec.
 *  
 *  TermPrim/Imakefile:
 *      -Fixed IBM defines to allow things to build.
 *  
 *  TermPrim/TermPrim.c:
 *      -Turned backgroundIsSelect off by default.
 *  
 *  TermPrim/TermPrimSetPty.c:
 *      -Changed code to return a valid value for the parsettymodes()
 *       function to remove the bogus Invalid ttymodes... msg.
 *  
 *  TermView/TermView.c:
 *      -Changed widget names in hierarchy to make them more consistent.
 * 
 * 
 * Revision 1.78  94/03/01  10:40:17  10:40:17  porsche (John Milton)
 * Next steps toward enabling the line wrap flag functionality.  We now
 *  manage the line wrap flag.
 *  
 *      TermPrimBuffer.c
 *          - now clearing the wrapped flag when moving a locked area
 *  
 *      TermPrimRender.c
 *          - _DtTermPrimInsertText() sets wrapped flag when lines wrap
 *          - _DtTermPrimFillScreenGap() preserves state of wrapped flag
 *            moving lines
 *      
 *  John
 * 
 * 
 * Revision 1.77  94/02/28  13:02:55  13:02:55  tom (Tom Houser)
 * TermPrimSelect.c
 *  - fixed a small bug with position selection of entire line.  No end line
 *  - was appended when the selection was pasted.
 * 
 * Revision 1.76  94/02/28  12:56:35  12:56:35  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      -Fixed CMVC defect #634: added toggle menubar to cloned windows
 *       as well as the original window.
 *      -Fixed up a few variable names that got trashed during a CDE->DT
 *       rename some time ago.
 *  
 *  Term/Term.c:
 *      -Fixed CMVC defect #672: class name should be DtTerm, not Term.
 *  
 *  TermPrim/Imakefile:
 *      -Added #defines for USE_TIOCCONS, USE_SRIOCSREDIR, and USE_STREAMS.
 *  
 *  TermPrim/TermPrim.c:
 *      -Changed saveLines from 2s to 4s after discussion with Human
 *       Factors rep.
 *      -Fixed CMVC defect #490:  sun console output invokes commands on
 *       pty.  Removed code to make pty master the console device.  This is
 *       now part of the subprocess code.
 *      -Moved setuid wrapper around the fork/exec portion of the code.
 *  
 *  TermPrim/TermPrimSetPty.c:
 *      -Fixed non CMVC defect in which -C didn't work on IBM and possibly
 *       HP.
 *      -Made separate functions for each type of console ioctl support.
 *      -Made OS dependent code trigger off of -D defines instead of OS
 *       specific defines.
 *  
 *  TermPrim/TermPrimSubproc.c:
 *      -Fixed CMVC defect #490:  sun console output invokes commands on pty
 *       by invoking the ioctl against the slave intead of the master side.
 *      -Added error check for failure to open pty slave.
 *      -Added a consoleMode param to _DtTermPrimSubprocExec().
 *  
 *  TermPrim/TermPrimSubproc.h:
 *      -Added a consoleMode param to declaration of
 *       _DtTermPrimSubprocExec().
 *  
 *  TermView/TermView.c:
 *      -Fixed CMVC defect #393:  scrollbars are now on by default.
 *      -menuBarVisible is now menuBar to make it more consistent with
 *       the scrollBar resource.
 *  
 *  TermView/TermView.h:
 *  TermView/TermViewMenu.c:
 *      -menuBarVisible is now menuBar to make it more consistent with
 *       the scrollBar resource.
 *  
 * 
 * 
 * Revision 1.75  94/02/24  13:35:34  13:35:34  porsche (John Milton)
 * First steps toward enabling the line wrap flag functionality.  Moved
 *  the line wrap flag out of the DtLinePart into the TermLinePart.  Added
 *  functions to allow the line wrap flag to be set/cleared, and tested.
 *  
 *      TermPrimBuffer.c
 *          - now initializing the line wrap flag in
 *            _DtTermPrimBufferCreateBuffer()
 *          - implemented  _DtTermPrimBufferTestLineWrapFlag()
 *            and _DtTermPrimBufferSetLineWrapFlag()
 *  
 *      TermPrimBufferP.h
 *          - added wrapped flag to TermLinePart
 *  
 *      TermPrimBuffer.h
 *          - added function prototypes for _DtTermPrimBufferTestLineWrapFlag()
 *            and _DtTermPrimBufferSetLineWrapFlag()
 *  
 *  
 *  John
 * 
 * 
 * Revision 1.74  94/02/23  17:06:07  17:06:07  tom (Tom Houser)
 * TermPrimSelect.c
 *    - added scroll selection
 *  TermPrimSelectP.h
 *    - added variables for scroll selection
 * 
 * 
 * Revision 1.73  94/02/16  11:13:21  11:13:21  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      -Changed DtTermMotif12Patch() to DtTermInitialize().
 *  
 *  TermPrim/TermPrim.c:
 *      -Changed DtTermMotif12Patch() to DtTermInitialize().  Added a
 *       warning if it is called after the Core class has been initialized.
 *       Added code to call it on TermPrim class initialize.
 *      -Removed _DtTermPrimPtyWrite() and replaced it with a new public
 *       function DtTermPrimSendInput() function.
 *      -Added DtTermSubprocSend() function to send (queued) data to the
 *       subprocess.
 *      -Added DtTermDisplaySend() function to send data throught the parser
 *       to the display.
 *      -Made a few exposed private functions static.
 *  
 *  TermPrim/TermPrim.h:
 *      -Added DtTermInitialize(), DtTermSubprocSend(), and
 *       DtTermDisplaySend() function prototypes.
 *      -#defined DtTermMotif12Patch() to point to DtTermInitialize().
 *  
 *  TermPrim/TermPrimDebug.c:
 *  TermPrim/TermPrimDebug.h:
 *      -Removed the old debug() function.
 *  
 *  TermPrim/TermPrimLineDraw.c:
 *      -Moved from debug() to DebugF().
 *  
 *  TermPrim/TermPrimPendingText.c:
 *  TermPrim/TermPrimPendingText.h:
 *      -Added _DtTermPrimPendingTextIsPending() function.
 *  
 *  TermPrim/TermPrimSelect.c:
 *      -Moved from obsolete _DtTermPrimPtyWrite() to DtTermSubprocSend().
 *  
 *  TermView/TermView.c:
 *  TermView/TermViewHelp.h:
 *      -Moved DtHelp related code to the new, improved Dt'ified names.
 *  
 *  TermView/TermViewGlobalDialog.c:
 *      -Made a few exposed private functions static.
 *  
 * 
 * 
 * Revision 1.72  94/02/15  13:44:32  13:44:32  daves (Dave Serisky)
 * ./TermPrim/TermPrim.c:
 *      -Added functionality to support the bold font that I forgot to
 *       check in with all the other stuff.
 *      -CMVC defect #603, 604, 619:  don't try and free a null buffer on
 *       termination before the window is mapped and the buffer created.
 *  
 *  ./TermPrim/TermPrimCursor.c:
 *      -CMVC defect #528:  changed call from XmImVaSetFocusValues to
 *       XmImVaSetValues to minimize communication to input method.
 *  
 *  ./TermPrim/TermPrimBuffer.c:
 *      -CMVC defect #603, 604, 619:  don't try and free a null buffer on
 *       termination before the window is mapped and the buffer created.
 *  
 *  ./TermView/TermView.c:
 *      -CMVC defect #367:  added hyperlink callback to deal with the man
 *       page xrefs in the on-line help.
 *  
 *  ./TermView/TermViewTerminalDialog.c:
 *  ./TermView/TermViewGlobalDialog.c:
 *      -CMVC defect #620:  added MWM_FUNC_MOVE to the dialog window manager
 *       menus (to allow moving the dialogs via the wm).
 *  
 *  ./TermView/TermViewHelp.h:
 *      -CMVC defect #367:  Added some #defines and #includes for man page
 *       hyperlinks.
 * 
 * 
 * Revision 1.71  94/02/14  15:07:03  15:07:03  daves (Dave Serisky)
 * 
 *  Term.c:
 *      -Initialize renderFonts 2 to the bold font
 *  
 *  TermPrimBuffer.h:
 *      -Fixed bogus declaration of constants TermENH_UNDERLINE and
 *       _OVERSTRIKE.
 *  
 *  TermPrimP.h:
 *      -Added boldFont and boldFontSet declarations.
 *  
 *  TermPrimRenderFont.c:
 *  TermPrimRenderFontSet.c:
 *      -Offset second render pass for overstrike by 1 pixel in the X
 *       direction.
 *  
 *  TermView.c:
 *      -Changed default fontsize font list to reflect the new dt interface
 *       font names.
 *  
 *  TermViewMenu.c:
 *      -Changed the code that generates fontsize pulldown to calculate the
 *       font sizes on the fly from the fonts.
 * 
 * 
 * Revision 1.70  94/02/14  12:09:36  12:09:36  tom (Tom Houser)
 * TermPrim.c
 *   - added higher level of permission around utmp initialization
 * 
 * Revision 1.69  94/02/11  11:12:58  11:12:58  tom (Tom Houser)
 * syncing files which I had checked into the cde tree
 * 
 * Revision 1.26  94/02/08  10:06:59  10:06:59  tom_hp_cv
 * TermPrimSelect.c 
 *    - added _DtTermPrimSelectAll and _DtTermPrimSelectPage
 *  TermPrimSelect.h 
 *    - added declarations for above
 * 
 * 
 * Revision 1.25  94/02/03  17:54:43  17:54:43  tom_hp_cv
 * TermPrimCursor.c
 *     - fixed a deselection bug
 *  
 * 
 * Revision 1.24  94/01/31  17:03:49  17:03:49  daves_hp_cv
 * 
 *  Massive checkin of all changes made since November Developer's
 *  Conference CD.
 * 
 * 
 * Revision 1.68  94/01/31  13:39:41  13:39:41  porsche (John Milton)
 * 10.0 fix, /usr/bin/posix/sh no longer exists
 *  
 *      TermPrim/TermPrimP.h
 *      	- DEFAULT_SHELL is now /usr/bin/sh for 10.0 (the new location for
 *            posix shell)
 * 
 * 
 * Revision 1.67  94/01/31  10:46:30  10:46:30  daves (Dave Serisky)
 * ./admindefines:
 *  ./Term/admindefines:
 *  ./TermPrim/admindefines:
 *  ./TermView/admindefines:
 *      - Removed defines for CdeVersion and UnityVersion
 *  
 *  ./DtTermMain.c:
 *  ./DtTermServer.c:
 *      - #ifdef'ed out the terminal server functionality.
 *  
 *  ./Dtterm.ad.src:
 *      - Added in helpview resources (for HPVUE version only).
 *  
 *  ./TermView/Imakefile:
 *  ./TermView/TermView.c:
 *      - Added necessary stuff to compile in HPVUE and CDE environments.
 *  
 *  ./TermView/TermViewHelp.h:
 *      - Help helper file to map CDE DtHelp names to HelpVue names.
 * 
 * 
 * Revision 1.66  94/01/31  10:12:45  10:12:45  porsche (John Milton)
 * Fixed CMVC bug #398.  
 *  
 *      TermPrim/TermPrim.c
 *      	- Added a new function _mergeEnv() to create a new list of env
 *      	  strings that merges the two supplied lists.  This is a
 *      	  static function at the moment.
 *      	- modified Realize to use _mergeEnv()
 * 
 * 
 * Revision 1.65  94/01/28  15:31:14  15:31:14  tom (Tom Houser)
 * TermPrimBufferP.h
 *   - made wcwdith() a macro.  This is strictly temporary as SUN does not
 *     yet use this function.
 * 
 * Revision 1.64  94/01/28  13:41:24  13:41:24  tom (Tom Houser)
 * TermPrimUtil.c
 *   - made call to setresuid HP only.
 * 
 * Revision 1.63  94/01/28  09:51:35  09:51:35  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      - Removed fallback resources.  They are now found in a separate
 *        source file generated from Dtterm.ad.src.
 *  
 *  Dtterm.ad.src:
 *      - Source file for the app-defaults file (and the fallback
 *        resources).
 *  
 *  Imakefile:
 *      - Added new files TermViewTerminalDialog.[co], and
 *        DtTermFallBackResources.[co]
 *      - Added Imake rule for generating Dtterm.ad and
 *        DtTermFallbackResources.c
 *  
 *  TermPrim/TermPrimSubproc.c:
 *      - CMVC#479 - fixed dereference of a null pointer if
 *        DtTermSubprocessReap() is called before a terminal window
 *        subprocess is created.
 *  
 *  TermView/Imakefile:
 *      - Added TermViewTerminalDialog.[co].
 *  
 *  TermView/TermView.c:
 *      - Changed default font list from point to t-shirt sizes.
 *  
 *  TermView/TermViewGlobalDialog.c:
 *      - Removed stub _DtTermViewCreateTerminalOptionsDialog() function.
 *  
 *  TermView/TermViewGlobalDialog.h:
 *      - Removed prototype for stub
 *        _DtTermViewCreateTerminalOptionsDialog() function.
 *  
 *  TermView/TermViewMenu.c:
 *      - Added terminal options dialog functionality.
 *  
 *  TermView/TermViewTerminalDialog.c:
 *      - New terminal dialog source file.
 *  
 *  TermView/TermViewTerminalDialog.h:
 *      - New terminal dialog header file.
 *  
 *  mkfallbk.sh:
 *      - Script to generate fallback resources from the app-defaults file.
 * 
 * 
 * Revision 1.62  94/01/26  16:19:01  16:19:01  tom (Tom Houser)
 * TermPrimBuffer.c
 *    - made a change in the "insert line" function to fix a bug
 * 
 * Revision 1.61  94/01/26  13:36:44  13:36:44  tom (Tom Houser)
 * TermPrimSelect.c
 *    - remove the MOVE operation from drag/drop
 * 
 * Revision 1.60  94/01/24  15:17:47  15:17:47  tom (Tom Houser)
 * TermPrimParser.c 
 *    -  changed stack initializer to use define for stack depth
 *  TermPrimParserP.h 
 *    -  changed stack size to a define rather than a constant.
 *    -  increased the size of the parser stack (to 20) and the stringParamater
 *       length (to 256) for use by dtterm
 * 
 * 
 * Revision 1.59  94/01/21  15:58:58  15:58:58  tom (Tom Houser)
 * TermPrimSelect.c 
 *    -  allowed move type drops to dtterm (moves are disallowed FROM dtterm)
 * 
 * 
 * Revision 1.58  94/01/20  18:09:39  18:09:39  tom (Tom Houser)
 * TermPrimSelect.c 
 *    -  added drag/drop routines
 *  TermPrimSelect.h 
 *    -  added button interface for drag/drop
 *  TermPrimSelectP.h 
 *    -  added structure def for drag/drop
 * 
 * 
 * Revision 1.57  94/01/20  10:24:05  10:24:05  daves (Dave Serisky)
 * ./DtTermMain.c:
 *      -Merged in code from hpterm to toggle menubar from WM menu.
 *  
 *  ./Term/Term.c:
 *      -Added setvalues support for fontList changes.
 *  
 *  ./Term/TermFunction.c:
 *      -Fixed color enhancements.
 *  
 *  ./TermPrim/TermPrim.c:
 *      -Changed the TermPrim highlight thickness default from 0 to 2.
 *  
 *  ./TermPrim/TermPrimCursor.c:
 *      -Added in some missed functionality for input method position and
 *       focus setting.
 *  
 *  ./TermView/Imakefile:
 *      -Added TermViewGlobalDialog.c.
 *  
 *  ./TermView/TermView.c:
 *      -Added _DtTermViewMenuDefaultFonts for changing font sizes.
 *      -Added support for synthetic resource (set/get values).
 *      -Added support for the missing XmNvisualBell, XmNmarginBell, and
 *       XmNnMarginBell, and XmNjumpScroll resources.
 *      -Added support for the new XmNuserFontList, XmNsizeList, and
 *       XmNuserFontList resources.
 *  
 *  ./TermView/TermView.h:
 *      -Removed defines for Xm[NC][rR]ecolorAreas.
 *      -Added defines for Xm[NC][uU]serFontList.
 *  
 *  ./TermView/TermViewMenu.c:
 *      -Added new hpterm functionality to menu.
 *      -Modified menu to reflect feedback from the hpterm usability review.
 *  
 *  ./TermView/TermViewMenu.h:
 *      -Modified to make many static functions nonstatic.
 *  
 *  ./TermView/TermViewP.h:
 *      -Added many new variables and resources.
 * 
 * 
 * Revision 1.56  94/01/18  17:41:38  17:41:38  tom (Tom Houser)
 * TermPrimBuffer.c
 *    - initialized line line in _DtTermPrimBufferResizeBuffer().
 * 
 * Revision 1.55  94/01/18  16:22:22  16:22:22  tom (Tom Houser)
 * TermPrimBuffer.c
 *    - changed test in insertEnhancements()
 * 
 * Revision 1.54  94/01/18  14:36:42  14:36:42  tom (Tom Houser)
 * TermPrimSelect.c
 * - made _DtTermPrimSelectConvert() more robust to handle spurious
 *   calls made from sun's cmdtool
 * 
 * Revision 1.53  94/01/18  13:55:11  13:55:11  daves (Dave Serisky)
 * TermPrim.c:
 * 	-Added support for traversal highlight.
 * 
 * Revision 1.52  94/01/18  13:42:31  13:42:31  daves (Dave Serisky)
 * Imakefile:
 *      -Changed from TermPrimBufferMb.* to TermPrimBufferWc.*
 *  
 *  TermPrim.c:
 *      -Added some code to (attempt) to correctly initialize the input
 *       method.
 *  
 *  TermPrimBuffer.c:
 *      -Fixed code to correctly calculate the starting column of a
 *       multi-byte character.
 *  
 *  TermPrimCursor.c:
 *      -Added some code to turn input method focus on and off.
 *  
 *  TermPrimRender.c:
 *      -Added multi-byte render support.
 *  
 *  TermPrimRender.h:
 *      -Added prototype for _DtTermPrimInsertTextWc().
 *  
 *  TermPrimRenderFontSet.c:
 *      -Modified render debug code to correctly fill the text area (fixed
 *       the width part) before rendering.
 * 
 * 
 * Revision 1.46  94/01/10  20:43:54  20:43:54  porsche (John Milton)
 * Added widget field to TermBuffer, allowing functions that use the TermBuffer
 *  to know what widget they were called with.
 *  
 *      TermPrim.c
 *          - made proper modifications for calling create_buffer_proc
 *  
 *      TermPrimBuffer.c:
 *          - _DtTermPrimBufferCreateBuffer() now accepts an additional
 *      	  widget parameter and initializes newly created TermBuffer's
 *      	  widget field
 *      	- made necessary changes to calls to _DtTermPrimBufferCreateBuffer()
 *      	- checking debug flag m:1 to conditionally force TermBuffer
 *      	  into wide character mode
 *      	- fixed a couple of off by one errors introduced in the
 *      	  previous length to width conversion
 *      	- added implementation of _DtTermPrimGetCharacterInfo()
 *  
 *      TermPrimBuffer.h:
 *          - added widget parameter to _DtTermPrimBufferCreateBuffer()
 *      	  prototype
 *      	- added prototype for _DtTermPrimGetCharacterInfo()
 *  
 *      TermPrimBufferP.h:
 *      	- added macro to access TermBuffer's widget parameter
 *          - added widget field to struct _TermBufferPart
 *  
 *  John 
 * 
 * 
 * Revision 1.45  94/01/07  10:32:51  10:32:51  daves (Dave Serisky)
 * 
 *  TermPrim.c:
 *      -Added the debug flag m:1 to force terminal emulator to use
 *       multi-byte routines for single byte locales.
 *  
 *  TermPrimDebug.c:
 *  TermPrimDebug.h:
 *      -Added new debug syntax, functions, to make it more flexible.
 *  
 *  TermPrimRender.c:
 *      -Moved to the new debug flag functions.  Separated text rendering
 *       debug flags between t:0 and t:1.
 *      -Fixed pre-parser to treat multi-byte characters that have an
 *       mbCharLen of 0 as single byte characters (i.e., the null char is
 *       now valid).
 *  
 *  TermPrimRenderFont.c:
 *  TermPrimRenderFontSet.c:
 *  TermPrimRenderLineDraw.c:
 *      -Moved the fill before rendering debug flag to t:1.
 *  
 * 
 * 
 * Revision 1.42  94/01/03  16:55:22  16:55:22  tom (Tom Houser)
 * TermPrimSelect.c 
 *    -  fixed a couple of small problems in the design and implementation
 * 
 * Revision 1.41  93/12/22  15:09:03  15:09:03  tom (Tom Houser)
 * TermPrim.c 
 *   - added a check for keyboard lock in the input character routine
 *  TermPrim.h 
 *   - added a new type and argument for keyboard change for notifying the
 *     TermView widget so it can post the status in the message bar
 * 
 * 
 * Revision 1.40  93/12/22  10:43:34  10:43:34  porsche (John Milton)
 * Step one of adding multi-byte support to the TermPrim layer.  Lines now
 *  have a width and a length, the width is measured in single width columns,
 *  the length is measured in characters (single byte or wchar_t, depending
 *  on the locale).  We got away with using length when we really meant width
 *  as long as we were running in a single-byte locale, but that won't hack
 *  it in this brave new world of wchar_t support, where the column width of
 *  a character does not equal the byte width of the character.
 *  
 *  
 *      TermPrim.c:
 *          - references to _DtTermPrimBufferSetLineLength() have been replaced
 *            with references to _DtTermPrimBufferSetLineWidth()
 *  
 *      TermPrimBuffer.c:
 *          - references to LENGTH have been replaced with references to
 *            WIDTH (except when it was obvious that LENGTH was wanted)
 *          - _DtTermPrimBufferGetLineLength() is now 
 *            _DtTermPrimBufferGetLineWidth()
 *          - the buffer specific set length function is now a buffer specific
 *            set width function (SET_LINE_WIDTH instead of SET_LINE_LENGTH)
 *          - _DtTermPrimBufferSetLineLength() is now 
 *            _DtTermPrimBufferSetLineWidth()
 *          - _DtTermPrimBufferPadLine() now only takes 3 parameters,
 *            startCol has been eliminated (the function now pads from the
 *            current width to the desired width, which is how it has been
 *            used all along)
 *          
 *      TermPrimBuffer.h:
 *          - _DtTermPrimBufferGetLineLength() is now 
 *            _DtTermPrimBufferGetLineWidth()
 *          - _DtTermPrimBufferSetLineLength() is now 
 *            _DtTermPrimBufferSetLineWidth()
 *          - _DtTermPrimBufferPadLine() now only takes 3 parameters,
 *            startCol has been eliminated (the function now pads from the
 *            current width to the desired width, which is how it has been
 *            used all along)
 *          - added definition for struct _TermCharInfoRec which is filled 
 *            with character information for the desired character (for the
 *            moment, this is wchar_t
 * 
 * Revision 1.39  93/12/21  15:36:13  15:36:13  porsche (John Milton)
 * Made some 10.0 specific modifications to allow us (dtterm/hpterm30) to
 *  use the correct shells.
 *  
 *      TermPrim/TermPrimP.h
 *          - added defines for DEFAULT_SHELL, and DEFAULT_SHELL_ARGV0
 *  
 *      TermPrim/TermPrimSubproc.c
 *          - use DEFAULT_SHELL instead of /bin/sh
 *  
 *      TermPrim/TermPrimUtil.c
 *          - use DEFAULT_SHELL and DEFAULT_SHELL_ARGV0, instead of /bin/sh
 *            and sh respectively
 *            
 *  
 *  John Milton
 * 
 * 
 * Revision 1.38  93/12/17  12:13:32  12:13:32  tom (Tom Houser)
 * TermPrim.c
 *   - added call to notify Selection about a resize
 * 
 * Revision 1.37  93/12/16  17:19:51  17:19:51  tom (Tom Houser)
 * TermPrimCursor.c
 *   - changed the place where check for above selection is done.
 * 
 * Revision 1.36  93/12/15  16:23:47  16:23:47  daves (Dave Serisky)
 * 
 *  TermPrim.c:
 *      -Added multi-byte to pre-parser and input loop.
 *  
 *  TermPrimData.h:
 *      -Added storage for partial multi-byte character in input stream.
 *  
 *  TermPrimParser.c:
 *  TermPrimParser.h:
 *  TermPrimParserP.h:
 *      -Added multi-byte support to the parser (inputChar is now a
 *       muti-byte character).
 *  
 *  TermPrimPendingText.c:
 *  TermPrimPendingText.h:
 *      -Added a replace function to replace the buffer in a pending chunk.
 *      -Moved from malloc/free to XtMalloc/XtFree.
 *  
 *  TermPrimRender.c:
 *  TermPrimRender.h:
 *      -Changed _DtTermPrimInsertText() to only insert text.
 *      -Added _DtTermPrimParseInput() to parse and insert text.
 *      -Changed _DtTermPrimParseInput() to be multi-byte smart.
 *  
 *  
 * 
 * 
 * Revision 1.34  93/12/14  11:07:48  11:07:48  daves (Dave Serisky)
 * 
 *  [checked in on 93/12/13 21:06:35 by daves]
 *  
 *  Imakefile:
 *   	-Added necessary code to build shared and archived libraries.
 *   
 *   TermPrim.c:
 *   	-Modified the setvalues routine to XtMakeGeometryRequest the new
 *   	 rows or columns and then reset the term.rows and term.columns
 *   	 values.  When the size change request is then granted, rows and
 *   	 columns will be set appropriatly.
 *   
 *  
 * 
 * 
 * Revision 1.12  93/12/13  21:06:35  21:06:35  daves (Dave Serisky)
 * Imakefile:
 *  	-Added necessary code to build shared and archived libraries.
 *  
 *  TermPrim.c:
 *  	-Modified the setvalues routine to XtMakeGeometryRequest the new
 *  	 rows or columns and then reset the term.rows and term.columns
 *  	 values.  When the size change request is then granted, rows and
 *  	 columns will be set appropriatly.
 *  
 * 
 * 
 * Revision 1.11  93/12/07  15:47:06  15:47:06  tom (Tom Houser)
 *  TermPrim.c
 *    - added translations for selection
 * 
 * Revision 1.10  93/11/18  13:37:59  13:37:59  daves (Dave Serisky)
 * ./TermPrim/TermPrim.c:
 *      -Added support for marginBell and nMarginBell resources.
 *      -Added a key event handler and key translator to prevent OSF keysyms
 *       from being applied to TermPrim widgets.
 *      -Refresh window on SetValues of charCursorShape to prevent leaving
 *       of old cursors.
 *      -Added marginBell functionality.
 *  
 *  ./TermPrim/TermPrim.h:
 *      -Added defines for Xm[NC][mM]arginBell and Xm[NC][nN]MarginBell.
 *  
 *  ./TermPrim/TermPrimP.h:
 *      -Added storage for marginBell and nMarginBell.
 * 
 * 
 * Revision 1.9  93/11/17  10:15:30  10:15:30  porsche (John Milton)
 * Adding links to the TermBuffers, also started using const specifier where
 * appropriate.  Fixed a bug in hpterm enhancements.
 * 
 *     TermPrimBuffer.h
 *         - added const specifier to all function declarations and typedefs
 *         - added declarations for new functions
 *             _DtTermPrimBufferGetRows()
 *             _DtTermPrimBufferGetCols()
 *             _DtTermPrimBufferSetLinks()
 *         - started adding some documentation to function declarations
 * 
 *     TermPrimBufferP.h
 *         - added new elements to TermBuffer structure
 *             nextBuffer
 *             prevBuffer
 *             selectInfo
 *         - added macros to access new members of TermBuffer structure
 * 
 *     TermPrimBuffer.c
 *         - added const specifier to all functions
 *         - initializing new term buffer members to null 
 *         - insertEnhancements only calls buffer specific function when
 *           insertFlag == True, and the function exists (this fixes a bug
 *           in hpterm enhancements
 *         - added the following functions:
 *             _DtTermPrimBufferGetRows()
 *             _DtTermPrimBufferGetCols()
 *             _DtTermPrimBufferSetLinks()
 * 
 * John Milton
 * 11/17/93
 * 
 * Revision 1.8  93/11/09  14:50:55  14:50:55  daves (Dave Serisky)
 * ./TermPrim/TermPrim.c:
 *     -Added support for boldFont.
 *     -Added setvalues support for rows, columns, and userFont.
 *     -Changed the resize code not to reallocate the buffer if the rows
 *      and columns are unchanged.
 * 
 * ./TermPrim/TermPrim.h:
 *     -Added support for the XmNterminalSizeList resource.
 * 
 * ./TermPrim/TermPrimData.h:
 *     -Modified fonts and added bold font.
 * 
 * ./TermPrim/TermPrimP.h:
 *     -Modified structures for the boldFont
 * 
 * ./TermPrim/TermPrimRepType.c:
 *     -Added a converter for the XmNterminalSizeList resource.
 * 
 * ./TermPrim/TermPrimRepType.h:
 *     -Blew away old bogus declarations.
 * 
 * Revision 1.7  93/11/01  16:58:08  16:58:08  tom (Tom Houser)
 * TermPrim.c
 *  - added logging start, write and close routines for logging all option
 * TermPrimData.h
 *  - added logStream (logging FILE or pipe)
 *  - added declaration for uid, gid and shell (are these needed globally?)
 * TermPrimUtil.c
 *  - implemented logging with the 3 exported functions open, write and close.
 *    (i.e., _DtTermPrimStartLog, _DtTermPrimWriteLog and _DtTermPrimCloseLog)
 *    Issues remain which I've commented.  e.g., there was a signal handler
 *    I didn't implement as we need to parameterize it for the right widget? 
 * TermPrimUtil.h
 *  - added declarations for  _DtTermPrimStartLog, _DtTermPrimWriteLog 
 *    and _DtTermPrimCloseLog)
 * 
 * Revision 1.6  93/10/29  10:23:53  10:23:53  tom (Tom Houser)
 * TermPrim.c
 *  - implemented pointerColor and pointerColorBackground
 *  - added pointerBlank on key input
 *  - changed pointer blank entry points to conform to nameing convention
 * TermPrim.h
 *  - added fields for pointerColor and pointerColorBackground            
 * TermPrimP.h
 *  - added declarations for pointerColor and pointerColorBackground
 * TermPrimUtil.c
 *  - added utility _DtTermPrimRecolorPointer()
 * TermPrimUtil.h
 *  - changed pointer blank entry points to conform to nameing convention
 * 
 * Revision 1.5  93/10/28  16:41:51  16:41:51  tom (Tom Houser)
 * Imakefile
 *  - added TermPrimUtil.c, TermPrimUtil.o
 * TermPrim.c
 *  - added resources for pointer blanking.
 *  - added calls to pointer blanking routines in TermPrimUtil.c
 * TermPrimData.h
 *  - added private state for pointer blanking
 * 
 * Revision 1.4  93/10/28  10:52:49  10:52:49  daves (Dave Serisky)
 * TermPrim.h:
 * TermPrimP.h:
 * TermPrimData.h:
 * TermPrim.c:
 *     - Added support for the userBoldFont.
 * 
 * Revision 1.3  93/10/27  10:11:41  10:11:41  tom (Tom Houser)
 * TermPrim.c
 *  - added resource definitions for pointer blanking, ksh mode, map on output
 *    and logging.
 *  - implementation for map on output and ksh mode.
 * TermPrim.h
 *  - added resource and class names for pointer blanking, ksh mode, map on output
 * TermPrimData.h
 *  - added term data for pointer blanking, ksh mode, map on output
 * TermPrimP.h
 *  - added private term data for pointer blanking, ksh mode, map on output
 * 
 * Revision 1.2  93/10/04  10:00:03  10:00:03  porsche (John Milton)
 * Updating from the cde tree:
 * 
 *     TermPrim/TermPrimSelect.c
 *         - getString() now returns a null string when begin == end
 *         - getString() allocates sufficient memory for the string
 *           (including room for any potential new line characters)
 * 
 * John Milton
 * 10/04/93
 * 
 * Revision 1.1  93/09/27  15:37:16  15:37:16  tom (Tom Houser)
 * Initial revision
 * 
 * Revision 1.23  93/09/23  14:31:37  14:31:37  daves_hp_cv
 * Fixed the -C code for sun platforms to include the right
 *  header file and call the right ioctl.
 * 
 * 
 * Revision 1.22  93/09/22  17:42:56  17:42:56  porsche_hp_cv
 * The selection code now knows how to deal with the history buffer...
 *  
 *      TermPrim
 *          TermPrimSelect.c
 *            - reformatted some code
 *            - rowColToPos now returns XmTextPosition
 *            - modified relevant routines to accomodate the history buffer
 *              as appropriate
 *  
 *          TermPrimRender.c
 *            - modified _DtTermPrimRefreshText() to accomodate the history
 *              buffer when deciding if the text to be refreshed might be
 *              contained in the selected area
 *  
 *  John Milton
 * 
 * 
 * Revision 1.21  93/09/22  16:45:49  16:45:49  daves_hp_cv
 * TermPrim.c:
 *      -Added support for console mode.
 *      -Removed sticky[Next|Prev]Cursor (from hpterm).
 *  
 *  TermPrim.h:
 *      -Added support for console mode.
 *  
 *  TermPrimP.h:
 *      -Added support for console mode.
 *      -Removed sticky[Next|Prev]Cursor (from hpterm).
 *  
 *  TermPrimSetPty.c:
 *      -Added function _DtTermPrimPtyConsoleModeEnable() to support console
 *       mode.
 *  
 *  TermPrimSetPty.h:
 *      -Added prototype for function _DtTermPrimPtyConsoleModeEnable() to
 *       support console mode.
 *  
 *  TermPrimSetUtmp.h:
 *      -Fixed prototype for _DtTermPrimUtmpGetUtLine().
 * 
 * 
 * Revision 1.20  93/09/21  16:57:39  16:57:39  daves_hp_cv
 * TermPrim.c:
 *  TermPrimSetUtmp.c:
 *  TermPrimSetUtmp.h:
 *  TermPrimSubproc.c:
 *      -Modified to support Sun platform pty (streams) and Sun and IBM
 *       utmp entries.
 *  
 *  Imakefile:
 *      -Added LinkFile for Sun TermPrimGetPty.c.
 *  
 *  TermPrimGetPty-svr4.c:
 *      -New file for Sun platforms.
 * 
 * 
 * Revision 1.19  93/09/10  15:49:45  15:49:45  porsche_hp_cv
 * The enhancements are looking better all the time.
 *  
 *      Term
 *          TermBuffer.c
 *            - implemented _DtTermBufferResize()
 *            - _DtTermBufferCreate() now initializes the resize proc
 *  
 *      TermPrim
 *          TermPrimBuffer.h
 *            - added typedef for TermResizeProc
 *            
 *          TermPrimBufferP.h
 *            - added buffer_resize_proc to _TermBufferPart struct and a macro
 *              to access it
 *  
 *          TermPrimBuffer.c
 *            - _DtTermPrimBufferCreate() initializes the resize proc to NULL
 *            - _DtTermPrimBufferResize() calls the helper function (if it
 *              exists)
 *      
 *  
 *  John Milton
 * 
 * 
 * Revision 1.18  93/09/10  13:15:05  13:15:05  daves_hp_cv
 * TermPrim.c:
 *      -Added history buffer support.
 *      -Added TermPrimData autoLineFeed variable and support.
 *  
 *  TermPrimCursor.c:
 *      -Added history buffer support.
 *  
 *  TermPrimData.h:
 *      -Added history buffer support.
 *      -Added TermPrimData autoLineFeed variable and support.
 *  
 *  TermPrimRender.c:
 *      -Added history buffer support.
 *  
 *  TermPrimScroll.c:
 *      -Added history buffer support.
 *  
 * 
 * 
 * Revision 1.17  93/09/09  17:26:56  17:26:56  porsche_hp_cv
 * The enhancements are looking pretty good.
 *  
 *      Term
 *          TermBuffer.c
 *            - _DtTermBufferInsertEnhancement() now inserts an enhancement
 *              whenever the specified line has a non-null enhancement buffer
 *              (regardless of the state of enhDirty)
 *            - _DtTermBufferCreateEnhancement() clears the entire enhancement
 *              buffer
 *            - _DtTermGetEnhancement() treats countAll and countNew the same
 *            - _DtTermClearLine() clears all enhancements between the old
 *               end of line, and the new end of line
 *  
 *      TermPrim
 *          TermPrimBuffer.h
 *            - added prototype for _DtTermClearLine()
 *            
 *          TermPrimBuffer.c
 *            - _DtTermPrimBufferCreate() now initializes all function pointers
 *              (even if it's only a null pointer), this prevents core dumps
 *              when building an application from the TermPrimitive widget
 *            - _DtTermPrimBufferGetEnhancement() and 
 *              _DtTermPrimBufferSetEnhancement() only call the corresponding
 *              helper function if the pointer is non-null
 *            - _DtTermPrimBufferInsertLine() now calls 
 *              _DtTermPrimBufferClearLine() to clear a line instead of clearing
 *              the line itself
 *            - _DtTermPrimBufferClearLine() calls the helper function (if it
 *              exists)
 *      
 *  
 *  John Milton
 * 
 * 
 * Revision 1.16  93/09/07  16:26:39  16:26:39  daves_hp_cv
 * TermPrimBuffer.c:
 *  TermPrimBuffer.h:
 *      -Added new function to insert lines within the scrolling region.
 *  
 *  TermPrimRender.c:
 *      -Fixed up problem with end of line wrapping on the last line in the
 *       scrolling region.
 * 
 * 
 * Revision 1.15  93/09/02  14:20:53  14:20:53  daves_hp_cv
 * Fixed InitOrResizeTermBuffer to use the current window length as
 *  the minimum new buffer length (was only extending it if we passed
 *  the 75% ration change.
 * 
 * 
 * Revision 1.14  93/09/02  10:51:04  10:51:04  daves_hp_cv
 * TermPrim.c:
 *      -Added the termDeviceAllocate resource and support to take the
 *       burden of pty allocation off of the application.
 *      -Added initialization of the emulationId resource for setting
 *       $TERMINAL_EMULATOR.
 *      -Removed the fast subclass hack.
 *      -Added the DtTermMotif12Patch() function to get around the problem
 *       of managers adding some accelerators to our widget to provide
 *       Motif1.1 behaviour for us.  This breaks things like the return key.
 *  
 *  TermPrim.h:
 *      -Added declaration for the DtTermMotif12Patch() function.
 *      -Added #defines for Xm[NC][eE]mulationId and
 *       Xm[NC][tT]ermDeviceAllocate.
 *  
 *  TermPrimFunction.h:
 *      -Added "fromMenu" and "fromOther" to the FunctionSource enum.
 *  
 *  TermPrimP.h:
 *      -Added support for emulationId and ptyAllocate.
 *      -Removed support for useFontSets.
 *  
 *  TermPrimRender.c:
 *      -Fixed end of line wrap to undo the core dumper that we were seeing
 *       when we wrapped in protect mode (with end of region == last line).
 *  
 *  TermPrimSubproc.c:
 *      -Added support for setting the $WINDOWID, $DISPLAY, and
 *       $TERMINAL_EMULATOR env variables.
 * 
 * 
 * Revision 1.13  93/09/01  16:04:21  16:04:21  porsche_hp_cv
 * Still working on code.  Fixed a couple of problems:
 *      - scrolled lines weren't being properly erased
 *      - no more core dumps on resize
 *      - no more core dumps when trying to set an enhancement
 *  
 *      Term
 *          Term.c
 *            - initializing buffer_free_proc
 *  
 *          TermBuffer.c
 *            - starting to remove all remnants of hpterm enhancement behavior
 *            - modified code to reference new DtEnh elements
 *  
 *          TermBuffer.h
 *            - defining dtEnhID (moved from TermPrimBuffer.h)
 *            - setting up defines for VT enhancements
 *            - fixed core dump on set enhancement
 *  
 *          TermBufferP.h
 *            - renamed variables, structs, etcVt* to Dt*
 *            - started redefining enhancement structure
 *            - added Dt-specific buffer part
 *            - added some new macros
 *  
 *          TermEnhance.c
 *            - slight mod to _DtTermEnhProc() so it will compile, Tom
 *              will have to rework it any way to get it to work properly.
 *  
 *      TermPrim
 *          TermPrim.c 
 *            - initializing buffer_free_proc
 *  
 *          TermPrimP.h
 *            - added buffer_free_proc to _DtTermPrimitiveClass record
 *            - moved BufferCreate typedef to TermPrimBuffer.h
 *  
 *          TermPrimBufferP.c
 *            - added buffer_create_proc, and buffer_free_proc to to 
 *              TermBufferPart definition
 *  
 *          TermPrimBuffer.h
 *            - removed enhID definition, its now emulation specific (all
 *              references to enhID as a parameter type were replaced with
 *              unsigned char)
 *            - added typedefs for BufferCreateProc, and BufferFreeProc
 *            
 *          TermPrimBuffer.c
 *            - DtTermPrimBufferCreateBuffer now initializes buffer_create
 *              and buffer_free
 *            - fixed problem with not clearing scrolled lines (I'd ifdef'd out
 *              too much code)
 *            - fixed core dump on resize
 * 
 * 
 * Revision 1.12  93/08/27  15:12:01  15:12:01  daves_hp_cv
 * TermPrimSubproc.c:
 *      -Fixed subproc code to strip off the leading path of argv[0] before
 *       pre-pending the leading '-' if loginShell is set.
 * 
 * 
 * Revision 1.11  93/08/27  11:48:25  11:48:25  porsche_hp_cv
 * dtterm was not properly clearing the bottom line after scrolling
 *      TermPrimBuffer.c
 *          uncommented some code that had been commented out during
 *          first pass a removing term-specific dependencies from
 *          generic term buffer code
 *  
 *  John Milton
 * 
 * 
 * Revision 1.10  93/08/26  16:15:14  16:15:14  daves_hp_cv
 * First Dt'ified version.
 * 
 * 
 * Revision 1.9  93/08/25  15:31:31  15:31:31  daves_hp_cv
 * Term.c:
 *      -Added debug flag ('b') for scrollbar debugging.
 *  
 *  TermCursor.c:
 *      -Fixed up scrollLockBottomRow to point to 0 relative last scrollable
 *       row.
 *      -Added debug flag ('b') for scrollbar debugging.
 *  
 *  TermRender.c:
 *      -Added support for scrollLockBottomRow.
 *  
 *  TermScroll.c:
 *      -Fixed up scrollLockBottomRow support (it now works).
 * 
 * 
 * Revision 1.8  93/08/25  15:14:52  15:14:52  porsche_hp_cv
 * This is the first cut at removing emulator specific functionality from
 *  TermBuffer code, and moving it into the emulation code.
 *  
 *      Hp70096
 *          Hp70096.c
 *              - initialize the term_primitive_class with hp70096 specific info
 *  
 *          Hp70096Buffer.c
 *          Hp70096Buffer.h
 *          Hp70096BufferP.h
 *              - implementation of hp specific buffer behavior
 *      
 *          Hp70096Enhance.c
 *          Hp70096Parse.c
 *              - include Hp70096Buffer.h instead of TermPrimBuffer.h
 *          Imakefile
 *              - added Hp70096Buffer.[co] to SRCS1 and OBJS1 respectively
 *  
 *      Term
 *          Term.c
 *              - include TermPrimBufferP.h instead of TermPrimBuffer.h
 *              - initialize the term_primitive_class with generic info
 *              - call buffer_create_proc instead of _DtTermPrimBufferCreateBuffer
 *  
 *          TermBuffer.c
 *              - removing hp specific behavioral code, calling emulation
 *                specific helper functions
 *          TermPrimBuffer.h
 *              - removing hp specific information
 *              
 *          TermPrimBufferP.h
 *              - redefined TermBuffer, TermLine, and TermEnh to allow
 *                specific implementations to sub-class from them as necessary
 *  
 *          TermCursor.c
 *              - mods to accomodate new TermBuffer definition
 *  
 *          TermPrimP.h
 *              - moved Term[ENH|IS] defines to TermPrimBuffer.h
 *              - TermEnh stuff to TermPrimBuffer.h
 *              - added typedef for BufferCreateProc 
 *              - modified term_primitive_class record
 *                  added buffer_create_proc, and size information
 *                  moved term_enh_proc to TermBuffer
 *  
 *          TermRender.c
 *              - mods to accomodate new TermBuffer definition
 *  
 *  John Milton 
 *  8/25/93
 * 
 * 
 * Revision 1.7  93/08/18  16:15:08  16:15:08  daves_hp_cv
 * Term.c:
 *      -Changed XmNshadowType validator to use the public XmRepType
 *       functions.
 * 
 * 
 * Revision 1.6  93/08/18  09:39:33  09:39:33  daves_hp_cv
 * Imakefile:
 *      -Added architecture dependent naming of TermGetPty*.c file.
 *  
 *  Term.c:
 *      -Added #include of "TermHeader.h" to pull in correct version of
 *       <time.h>.
 *  
 *  TermDebug.c:
 *      -Added #include of "TermHeader.h" to pull in correct version of
 *       <time.h>.
 *      -Removed (void) typecast of timerclear to compile on AIX.
 *  
 *  TermGetPty.c:
 *      -Changed #include of <sys/termios.h> to <termios.h> to point at the
 *       POSIXly correct filename.
 *      -Removed #includes of <sys/ptyio.h> and <sys/bsdtty.h> since we
 *       are no longer trapping ioctls and since the support we were getting
 *       from <sys/bsdtty.h> has been rolled into termios.
 *  
 *  TermPrimLineFontP.h:
 *      -Changed offset declarations from "char" to "signed char" since some
 *       systems (such as AIX) default to unsigned char.
 *  
 *  TermHeader.h:
 *      -Added #include of <X11/Xos.h> to clean up some of the OS differences
 *       (such as the timeval used by select).
 *      -Added #ifdef's around MIN and MAX macro #defines to deal with the
 *       case where one of the OS header files has already defined them.
 *  
 *  TermRender.c:
 *  TermScroll.c:
 *      -Removed #include of <time.h> since that is now being pulled in via
 *       <X11/Xos.h>.
 *  
 *  TermSetPty.c:
 *      -Added an #ifdef __AIX include of <sys/tty.h> to pull in the struct
 *       winsize header file.
 *  
 *  TermSetUtmp.c:
 *      -Added an #defines for NO_pututline and an #ifdef'ed code section to
 *       define _pututline() for systems that don't already have one.
 *  
 *  TermSubproc.c:
 *      -Added the appropriate #ifdef's and #define's to deal with systems
 *       that don't have setres[gu]id.
 *      -Changed #include of <sys/fcntl.h> to the POSIXly correct <fcntl.h>
 *      -Changed from _NSIG to NSIG which also seems to by POSIXly correct.
 *  
 *  TermGetPty-clone.c:
 *      -Added a new GetPty file for pty based systems that have a clone
 *       device.
 *  
 *  util/lineToData.c:
 *      -Changed declarations (and some code) to deal with "char" declarations
 *       being "unsigned char" on some systems.
 * 
 * 
 * Revision 1.5  93/08/16  17:25:38  17:25:38  daves_hp_cv
 * Term.c:
 *      -Blew away the [forward|reverse]_wrap_procs.
 *      -Added wrap_right_after_insert to term widget class.
 *      -Added initialization of wrapRightAfterInsert.
 *  
 *  TermPrimData.h:
 *      -Added wrapRightAfterInsert.
 *      -Added wrapState that isn't being used yet but will need to be used
 *       if we are going to allow ANSI right wrap behaviour with variable
 *       right margins.
 *  
 *  TermPrimP.h:
 *      -Blew away the [forward|reverse]_wrap_procs.
 *      -Added wrap_right_after_insert to term widget class.
 *  
 *  TermRender.c:
 *      -Added (actually fixed) support for wrapAfter (HP style) or
 *       !wrapAfter (ANSI style) character insert.
 * 
 * 
 * Revision 1.4  93/08/11  14:53:28  14:53:28  daves_hp_cv
 * Term.c:
 *      -Added support for useHistoryBuffer, allowScrollBelowBuffer, and
 *       scrollLockBottomRow.
 *      -Added forward_wrap_proc and reverse_wrap_proc to DtTermPrimitiveClassPart
 *       initialization.
 *  
 *  TermCursor.c:
 *      -Added support for useHistoryBuffer, allowScrollBelowBuffer, and
 *       scrollLockBottomRow to scrollBar support code.
 *  
 *  TermPrimData.h:
 *      -Changed names from *memoryLock* to *scrollLock*.
 *      -Added support for useHistoryBuffer, allowScrollBelowBuffer, and
 *       scrollLockBottomRow.
 *  
 *  TermPrimP.h:
 *      -Added use_history_buffer, allow_scroll_below_buffer,
 *       forward_wrap_proc, and reverse_wrap_proc.
 *      -Added forward_wrap_proc and reverse_wrap_proc to DtTermPrimitiveClassPart.
 * 
 * 
 * Revision 1.3  93/08/09  13:20:44  13:20:44  daves_hp_cv
 * fixed up description.
 * 
 * 
 * Revision 1.2  93/08/09  13:18:42  13:18:42  daves_hp_cv
 * Term.c:
 *      -Added a reference to _DtTermPrimPullInTermWhatString to pull in the what
 *       string from version.c
 *      -Removed termEmulationMode, useFontSets, useLineDraw resources.
 *      -Changed terminalFontList resource to userFont.
 *      -Added and cleaned up the initialize checklist.
 *  
 *  TermPrim.h:
 *      -Added Xm[N|C][l|L]oginShell and Xm[N|C][u|U]serFont resources.
 *      -Deleted Xm[N|C][t|T]erminalFontList resource.
 *  
 *  TermPrimP.h:
 *      -Removed userLineDraw and termEmulationMode variables.
 *  
 *  TermPrimRender.h:
 *      -Removed references to termEmulationMode.
 *  
 *  version.c
 *      -New version file.
 * 
 * 
 * Revision 1.1  93/08/09  13:17:52  13:17:52  daves_hp_cv
 * Initial revision
 * 
 */
