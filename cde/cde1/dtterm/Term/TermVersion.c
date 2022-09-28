#ifndef	lint
#ifdef	VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermVersion.c /main/cde1_maint/2 1995/08/26 22:59:52 barstow $";
#endif	/* VERBOSE_REV_INFO */
/* DKS
static char alpha_string[] = "\
@(#)\n\
@(#)\n\
@(#)\n\
@(#)\n\
@(#)";
DKS */
#endif	/* lint */

/*
** (c) Copyright 1993, 1994 Hewlett-Packard Company
** (c) Copyright 1993, 1994 International Business Machines Corp.
** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
** (c) Copyright 1993, 1994 Novell, Inc.
*/

#include <X11/hpversion.h>

#ifndef	lint
hpversion(dtterm1.0:         $XConsortium: TermVersion.c /main/cde1_maint/2 1995/08/26 22:59:52 barstow $)
#endif	/* lint */

char _DtTermPullInTermWhatString[] = "";

/* $Log$
 *  
 *      DtTermMain.c
 *      	- restored the code that 'Added session file version id to
 *      	  session manager files' (initially added in rev 1.43)
 *  
 *      TermPrim/TermPrimOSDepI.h
 *      	- fixed CMVC 7480 ifdef macro missing on Sun architecture
 * 
 * 
 * Revision 1.169  95/01/03  15:19:08  15:19:08  daves (Dave Serisky)
 * -Fixed CMVC7169:  dtterm -session <bogus_session_file> was coredumping
 *  or just not working right.
 *  -Fixed CMVC7216:  dtterm session restores of multiple iconified instances
 *  was not all coming up iconified.
 * 
 * 
 * Revision 1.168  95/01/03  11:33:39  11:33:39  tom (Tom Houser)
 * Term.c
 *   CMVC 7001 - free the line draw font when the Term widget is destroyed.
 * 
 * Revision 1.167  94/12/06  14:57:24  14:57:24  daves (Dave Serisky)
 * DtTermMain.c:
 *      - Fixed a session manager bug that was causing session restores to
 *        come up in the wrong position with the wrong size.
 * 
 * 
 * Revision 1.166  94/12/01  10:05:23  10:05:23  porsche (John Milton)
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
 * Revision 1.165  94/12/01  08:48:02  08:48:02  daves (Dave Serisky)
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
 * Revision 1.164  94/11/18  14:29:50  14:29:50  tom (Tom Houser)
 * Term.c
 *   -continuation of fix for CMVC 6202
 *      added a couple more osf virtual keysyms.
 *      moved default (to osf virtual keysyms) up in table.
 * 
 * Revision 1.163  94/11/16  16:09:59  16:09:59  tom (Tom Houser)
 * Fix for bug CMVC 5988
 *  TermParse.c
 *   - removed call to selection adjustment.
 *  TermPrimScroll.c
 *   - added call to selection adjustment with slight modification from when
 *     it was in TermParse.c.
 *  TermFunction.c
 *    - remove dead code.  not related to above except that this was part of
 *      the old deselection design.
 * 
 * 
 * Revision 1.162  94/11/15  10:54:35  10:54:35  tom (Tom Houser)
 * Term.c
 *   fix for CMVC 6202 - added some key translations for defaults to
 *      the osf virtual keysyms.
 * 
 * Revision 1.161  94/11/14  11:50:23  11:50:23  tom (Tom Houser)
 * TermParse.c
 *   fix for CMVC 5930
 *     removed references to wrap state which is not needed by dtterm.
 * 
 * Revision 1.160  94/11/02  16:06:42  16:06:42  daves (Dave Serisky)
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
 * Revision 1.159  94/11/01  11:24:12  11:24:12  tom (Tom Houser)
 * Term.c
 *   - added osfActivate key to translation table.
 * 
 * Revision 1.158  94/10/19  07:27:00  07:27:00  daves (Dave Serisky)
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
 * Revision 1.157  94/10/12  16:24:58  16:24:58  tom (Tom Houser)
 * CMVC 5174 fix
 *  TermParse.c
 *   - added check to adjust selection in scroll region is set.
 *  TermPrimSelect.c
 *   - added a routine to move the selection if necessary.    
 * 
 * 
 * Revision 1.156  94/09/28  13:47:25  13:47:25  tom (Tom Houser)
 * CMVC 5174 fix
 *  TermData.h
 *   - removed obsolete variables from the *TermData structure.
 *  Term.c
 *   - removed initialization of said variables.
 * 
 * 
 * Revision 1.155  94/09/28  13:19:57  13:19:57  tom (Tom Houser)
 * TermAction.c
 *   - cleaned up some of the argument handling.
 *  TermAction.h
 *   - removed some obsolete interfaces
 * 
 * 
 * Revision 1.154  94/09/27  16:09:08  16:09:08  tom (Tom Houser)
 * TermData.h
 *  TermFunctionKey.c
 *  TermFunctionKey.h
 *  TermFunctionKeyP.h
 *  TermP.h
 *  TermParse.c
 *  DtTermMain.c
 *  - Fix for CMVC 5139 Term.c - fixed a reference to a structure in TermParse.c that 
 *       should have been a reference to a string in the structure.  
 *  - general cleanup of TermFunctionKey.h header which contained lots of references
 *       to old hpterm stuff.
 *  - removed ALL references to TermFunctionKeyP.h 
 * 
 * 
 * Revision 1.153  94/09/26  11:30:06  11:30:06  daves (Dave Serisky)
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
 * Revision 1.152  94/09/12  13:25:28  13:25:28  tom (Tom Houser)
 *    fixes for CMVC 4685  
 *  Term.c
 *   - added initialization for char sets, which is current, if single char
 *     shift is pending (False), and fg/bg color when reset (soft and hard) 
 *     is performed.
 *  TermFunction.c
 *   - added initialization for fg and bg color to the save cursor structure
 *     on power up.
 * 
 * 
 * Revision 1.151  94/09/09  16:33:13  16:33:13  tom (Tom Houser)
 * TermParse.c
 *   - fix for CMVC 4676 and CMVC 4678  These were 2 cases where the cursor
 *     could escape the scroll region.  One was after a DEC Align escape
 *     sequence and the other was after a "Cursor Restore" escape sequence.
 * 
 * Revision 1.150  94/09/06  12:35:21  12:35:21  tom (Tom Houser)
 * TermParse.c
 *   - fix for CMVC 4524.  CUP didn't always work in origin mode.
 * 
 * Revision 1.149  94/09/06  09:24:00  09:24:00  daves (Dave Serisky)
 * ./Term/TermBuffer.c (daves), lasted locked on 09/02/94 at 14:32
 *  ./Term/TermParse.c (daves), lasted locked on 09/02/94 at 09:03
 *  ./TermPrim/TermPrim.c (daves), lasted locked on 08/31/94 at 10:27
 *  ./TermPrim/TermPrimOSDepI.h (daves), lasted locked on 08/31/94 at 10:12
 * 
 * 
 * Revision 1.148  94/08/30  15:11:48  15:11:48  tom (Tom Houser)
 * TermParse.c
 *  - fix for CMVC 4468 - changed "Cursor back" (Esc[pD)  and carriage
 *    return to keep the cursor confined to top and bottom margins when
 *    in origin mode.
 * 
 * Revision 1.147  94/08/22  10:03:50  10:03:50  daves (Dave Serisky)
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
 * Revision 1.146  94/08/16  16:54:26  16:54:26  daves (Dave Serisky)
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
 * Revision 1.145  94/08/15  17:02:11  17:02:11  daves (Dave Serisky)
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
 * Revision 1.144  94/08/08  10:32:42  10:32:42  daves (Dave Serisky)
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
 * Revision 1.143  94/08/02  11:26:40  11:26:40  daves (Dave Serisky)
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
 * Revision 1.142  94/07/25  10:34:04  10:34:04  tom (Tom Houser)
 * TermParse.c
 *  - fixed CMVC 3384.  This was a problem of SU and SD routines not
 *    checking if they were outside of scrolling region.
 *    Also noticed another problem when cursor is at the bottom of the
 *    screen and margins are set and a CUD happens.  This incorrectly
 *    scrolled the scrolling region.
 * 
 * Revision 1.141  94/07/22  15:04:45  15:04:45  tom (Tom Houser)
 * TermFunction.c
 *   - fix for CMVC 3382.  Check is now made to prevent calls to scrolling
 *     routines when top and bottom margins are set.
 * Term.c
 *   - added scroll up 1 line and scroll down 1 line actions to shifted
 *     cursor up and down resp.
 * 
 * Revision 1.140  94/07/19  13:54:26  13:54:26  daves (Dave Serisky)
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
 * Revision 1.139  94/07/14  11:08:19  11:08:19  porsche (John Milton)
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
 * Revision 1.138  94/07/14  09:48:04  09:48:04  daves (Dave Serisky)
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
 * Revision 1.137  94/07/07  15:29:55  15:29:55  daves (Dave Serisky)
 * 
 *  DtTermMain.c:
 *      - CMVC #2996:  Changed session restart to include the same args that
 *        are used for non session restarts.
 *  
 * 
 * 
 * Revision 1.136  94/07/06  17:02:57  17:02:57  daves (Dave Serisky)
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
 * Revision 1.135  94/07/05  15:51:36  15:51:36  tom (Tom Houser)
 * TermSendEsc.h
 *  - change DT_KP_Enter from newline to carriage return
 * 
 * Revision 1.134  94/07/05  14:56:24  14:56:24  tom (Tom Houser)
 * TermAction.c
 *  - added a newline to keyboard enter (for newline mode)
 * 
 * Revision 1.133  94/07/05  12:55:32  12:55:32  daves (Dave Serisky)
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
 * Revision 1.132  94/06/30  17:50:13  17:50:13  tom (Tom Houser)
 * TermParse.c
 *   - fixed a bug in ICH when length exceeded line length.         
 *   - replaced type short with type int to eliminate truncation to neg. numbers
 *   - did additional range checking where necessary to truncate arg. to size short
 *  TermPrimParser.c
 *   - change num. parser to keep number from going negative.
 * 
 * 
 * Revision 1.131  94/06/28  17:42:36  17:42:36  tom (Tom Houser)
 * TermParse.c
 *  - Change DECSET 80-132 mode logic so the internal state of row and cols
 *    gets set so that the menu logic is in sync.  However, there is still a
 *    bug having to do with changing the size of the terminal and the row
 *    and cols not getting updated.  I made this change mostly because it
 *    simplfied the code.  However, it does work around the bug also.
 * 
 * Revision 1.130  94/06/28  14:18:29  14:18:29  tom (Tom Houser)
 * TermAction.c
 *  - changed keypad action so it does the right thing in c2 mode.
 * 
 * Revision 1.129  94/06/24  17:38:43  17:38:43  tom (Tom Houser)
 * TermFunction.c
 *  - Implementation of deleting lines when bottom margin is above last used line.
 *    This should fix CMVC 558.
 * 
 * Revision 1.128  94/06/23  13:00:45  13:00:45  tom (Tom Houser)
 * Term.c
 *  TermData.h
 *  TermParse.c
 *   - added save and restore of DECSET for reverse video and margin bell.
 * 
 * 
 * Revision 1.127  94/06/23  11:57:59  11:57:59  daves (Dave Serisky)
 * TermParse.c:
 * 	-Added LS2 and LS3 functionality.
 * 
 * Revision 1.126  94/06/22  16:28:09  16:28:09  tom (Tom Houser)
 * TermFunction.c
 *  -changed soft reset to turn off reverse wrap mode (not consistant with xterm)
 * 
 * Revision 1.125  94/06/22  16:19:52  16:19:52  tom (Tom Houser)
 * TermParse.c
 *  - tightened up the parsing for DECSCL and fixed some bugs regarding setting
 *    the 7/8-bit mode while doing the DECSCL.
 * 
 * Revision 1.124  94/06/22  15:30:01  15:30:01  tom (Tom Houser)
 * TermParse.c
 *  TermParseTable.c
 *  TermParseTable.h
 *  TermParseTableP.h
 *   - added a new parser state Esc[?.  Had tried to lump this state with Esc[,
 *     but that's not possible without accepting illegal grammars.
 * 
 * 
 * Revision 1.123  94/06/22  12:49:26  12:49:26  tom (Tom Houser)
 * Term.c
 *  -  changed button translation so the drag operation will never be MOVE
 * 
 * Revision 1.122  94/06/20  13:26:12  13:26:12  tom (Tom Houser)
 * TermParse.c
 *  - moved cursor to top margin and 0 when origin mode is turned on and off
 *    respectively.
 * 
 * Revision 1.121  94/06/17  15:34:08  15:34:08  tom (Tom Houser)
 * TermParse.c
 *  - added an abstraction for sending the 8-bit value for escape.
 *  - cleaned up the reporting code.
 * 
 * Revision 1.120  94/06/16  14:32:02  14:32:02  tom (Tom Houser)
 * TermParse.c
 *  - removed an extra write in DA.
 * 
 * Revision 1.119  94/06/15  15:09:28  15:09:28  tom (Tom Houser)
 * TermFunction.c
 *  - fixed soft reset so it will set the cursor visible state to true as
 *    does the vt220
 * 
 * Revision 1.118  94/06/15  14:58:40  14:58:40  tom (Tom Houser)
 * TermParse.c
 *  - removed some dead code and commented out empty branches.
 * 
 * Revision 1.117  94/06/15  14:37:03  14:37:03  tom (Tom Houser)
 * TermFunction.c
 *  - fixed CMVC 2743 which was a scrolling bug.  Rewrote scrolling
 *    range checking and made it much simpler.
 * 
 * Revision 1.116  94/06/14  12:51:57  12:51:57  tom (Tom Houser)
 * TermParseTable.c
 *  - add states to fix CMVC report 2713
 *    which is a bug in handling the lock parameter if no user strings
 *    are given
 * 
 * Revision 1.115  94/06/10  13:35:48  13:35:48  tom (Tom Houser)
 * TermFunction.c
 *   - reset the parser stack on soft and hard reset - fixes CMVC 2634
 * 
 * Revision 1.114  94/06/08  16:04:29  16:04:29  daves (Dave Serisky)
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
 * Revision 1.113  94/06/07  16:12:36  16:12:36  tom (Tom Houser)
 * TermParse.c 
 *  TermParseTable.c 
 *   - added device status reports (DSR) for printer, UDK locked, and keyboard lang.
 * 
 * 
 * Revision 1.112  94/06/07  13:29:29  13:29:29  tom (Tom Houser)
 * TermAction.c
 *   - changed page scrolling so that last/first line of the prev/next
 *     respectively is left on the screen for styleguide compliance.
 * 
 * Revision 1.111  94/06/06  16:08:48  16:08:48  tom (Tom Houser)
 * Term.c
 *   - fixed a bugged with using strcmp() incorrectly.
 * 
 * Revision 1.110  94/06/03  13:52:45  13:52:45  tom (Tom Houser)
 * TermParse.c
 *  - fixed SU which broke when I changed the function it was calling
 *    _DtTermFuncScroll() to disallow scrolling beyond the last line
 *    SU is a "special case" of scrolling and I handled it that way.
 * 
 * Revision 1.109  94/06/03  08:37:30  08:37:30  daves (Dave Serisky)
 * 
 *  DtTermSyntax.c:
 *      -CMVC #875, #1038:  Cleaned up usage message.
 *  
 *  Dtterm.ad.src:
 *      -CMVC #2080:  Reduced borders in response to Steve Anderson and May
 *       Tech Review feedback.
 *  
 *  TermPrim/TermPrim.h:
 *  TermPrim/TermPrimRepType.c:
 *      -CMVC #2412:  Changed rep types to match Dt standards.
 *  
 *  TermPrim/TermPrimData.h:
 *      -CMVC #2395:  Added a flag to allow delaying mapping of warning
 *       dialogs until the window has been mapped.  This allows the warning
 *       dialog on pty allocation failure to act as a subwindow of the
 *       terminal window (before it was mapped before the terminal window
 *       and was independent).
 *  
 *  TermPrim/TermPrimSelectP.h:
 *      -Made an external declaration external.  This fixed a possible bug
 *       in the two button bindings code (part of CMVC #524).
 *  
 *  TermPrim/TermPrimUtil.c:
 *      -Fixed a few bogus #elif cpp lines.
 *  
 *  TermView/TermView.c:
 *      -CMVC #2412:  Changed rep types to match Dt standards.
 *      -Changed some help related calls to match the latest CDE help
 *       changes.
 *      -CMVC #294:  TermView code now uses the -geometry string to get the
 *       default rows and columns so things now work when we are run with
 *       -geometry 100x40.
 *  
 *  TermView/TermViewGlobalDialog.c:
 *  TermView/TermViewTerminalDialog.c:
 *      -Added the wm pulldown menu to the wm decorations.  This is part of
 *       a usablity fix.
 *  
 *  TermView/TermViewHelp.h:
 *      -Changed some help related #defines to match the latest CDE help
 *       changes.
 * 
 * 
 * Revision 1.108  94/06/01  07:46:00  07:46:00  daves (Dave Serisky)
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
 * Revision 1.107  94/05/25  18:11:57  18:11:57  tom (Tom Houser)
 * TermParse.c
 * TermData.h
 *   - added parsing of dave's extension of xterm text esc seq to define
 *     current working directory for session start up
 * 
 * Revision 1.106  94/05/25  14:30:27  14:30:27  bobm (Bob May)
 * 
 *  Added support for Reset buttons in the Terminal Options and Global
 *  Options dialogs.
 *  
 * 
 * 
 * Revision 1.105  94/05/23  17:42:56  17:42:56  tom (Tom Houser)
 * Term.c
 *  TermPrim.c
 *  TermPrimSelect.c
 *  TermPrimSelect.h
 *    -  added cancel to selection.
 * 
 * 
 * Revision 1.104  94/05/19  15:56:26  15:56:26  tom (Tom Houser)
 * Term.c
 *  - fixed a bug with new selection model initialization.
 *    need to initialize boolean passed to XtVaGetValues() since, if
 *    it doesn't exit the boolean won't get set to False.
 * 
 * Revision 1.103  94/05/19  14:28:19  14:28:19  tom (Tom Houser)
 * TermParse.c
 *   - changed the number parser to use the new routine,
 *          _DtTermPrimParserNumParmPush()
 * 
 * Revision 1.102  94/05/19  10:01:50  10:01:50  daves (Dave Serisky)
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
 * Revision 1.100  94/05/18  14:17:36  14:17:36  daves (Dave Serisky)
 * 
 *  Term/Term.c:
 *      -Added support for vt100 style font selection.
 *  
 *  Term/TermBuffer.c:
 *      -Fixed CMVC 1670: erase had an off by one error that was causing
 *       it to fail in some situations.
 *      -Added support for vt100 style font selection.
 *  
 *  Term/TermBuffer.h:
 *      -Added BOLD enhancement.
 *      -Removed FONT_USER font.
 *  
 *  Term/TermBufferP.h:
 *      -Changed video enhancements from 5 to 6 bits.
 *      -Changed font enhancements from 2 to 1 bit.
 *  
 *  Term/TermData.h:
 *      -Added support for vt100 style font selection.
 *  
 *  Term/TermEnhance.c:
 *      -Added support for BOLD enhancement.
 *      -Added support for linedraw font.
 *  
 *  Term/TermFunction.c:
 *      -Modified SGR enhancements to treat BOLD as an enhancement and not a
 *       font.
 *      -Added support for vt100 style font selection.
 *  
 *  Term/TermFunction.h:
 *      -Added missing declarations.
 *  
 *  Term/TermLineData.data:
 *      -Changed encoding from HP linedraw font to DEC special graphics
 *       linedraw characters.
 *  
 *  Term/TermParse.c:
 *      -Added vt100 style fonts to cursor save and restore.
 *      -Added parser functions for LS0 and LS1.
 *      -Implemented SCS font selection functions.
 *  
 *  Term/TermParseTable.c:
 *      -Added functions for LS0, LS1 and SCS font functions.
 *  
 *  Term/TermParseTable.h:
 *      -Added declarations for SCS functions.
 *  
 *  TermView/TermView.c:
 *      -Fixed CMVC #2191: dtterm with *menuBar: false no longer coredumps.
 *      -Added menuPopup resource to go along with the menuBar resource.
 *  
 *  TermView/TermView.h:
 *      -Added Dt[NC][mM]enuPopup #defines.
 *  
 *  TermView/TermViewMenu.c:
 *  TermView/TermViewMenu.h:
 *      -Fixed CMVC #2191: dtterm with *menuBar: false no longer coredumps.
 *  
 *  TermView/TermViewP.h:
 *      -Added storage for menuPopupVisible.
 * 
 * 
 * Revision 1.99  94/05/16  14:50:54  14:50:54  tom (Tom Houser)
 * TermParse.c 
 *  TermParseTable.c 
 *  TermParseTable.h 
 *  TermParseTableP.h 
 *    - added declarations, parsing and implementation for one more sun 
 *      escape sequences
 * 
 * 
 * Revision 1.98  94/05/13  17:35:23  17:35:23  porsche (John Milton)
 * Sync'd up with CDE Developer's Environment changes
 *  
 *      Term.c
 *      Term.h
 *      TermFunction.c
 *      TermParse.c
 *      TermVersion.c
 *  
 *  John
 * 
 * 
 * Revision 1.97  94/05/12  17:15:06  17:15:06  tom (Tom Houser)
 * TermParse.c 
 *  TermParseTable.c 
 *  TermParseTable.h 
 *  TermParseTableP.h 
 *    - added declarations, parsing and implementation for sun escape sequences
 * 
 * 
 * Revision 1.96  94/05/09  10:25:28  10:25:28  daves (Dave Serisky)
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
 * Revision 1.95  94/05/04  16:02:35  16:02:35  tom (Tom Houser)
 * Term.c
 *  TermAction.c
 *  TermFunction.c
 *  TermAction.h
 *  TermFunction.h
 *    - fixed an off-by-one error in doScroll() in TermFunction.c
 *    - replaced _DtTermActionPreviousPage() and _DtTermActionNextPage() with
 *      _DtTermActionScroll() 
 *    - removed _DtTermActionPreviousPage() and _DtTermActionNextPage()
 *    - removed _DtTermFuncNextPage() and _DtTermFuncPreviousPage() which
 *      are no longer needed.
 *    - removed some needless logic from _DtTermActionScroll().  
 * 
 * 
 * Revision 1.94  94/05/04  10:44:06  10:44:06  tom (Tom Houser)
 * TermParse.c
 *   - fixed DECREQTPARM by checking to make sure parameter was 0 or 1
 * 
 * Revision 1.93  94/05/03  16:54:54  16:54:54  tom (Tom Houser)
 * TermFunction.c
 *   - fixed a bug with not turning on the cursor after scrolling
 *   - fixed a bug with scrolling past the active buffer
 * 
 * Revision 1.92  94/04/29  14:05:02  14:05:02  porsche (John Milton)
 *     TermBuffer.c
 *      	- fixed some memory corruption/leak problems identified by Sentinel
 *  
 *  John
 * 
 * 
 * Revision 1.91  94/04/28  15:51:38  15:51:38  tom (Tom Houser)
 * Term.c 
 *     - initialized compatLevel and terminalId  (new state)
 *  TermData.h   
 *     - added terminalId to the TermData structure.
 *  TermParse.c
 *     - fixed in bug in DA report
 *     - fixed setting of compatible level for 8-bit or 7-bit transmission 
 * 
 * 
 * Revision 1.90  94/04/28  10:35:41  10:35:41  tom (Tom Houser)
 * TermAction.c
 *   - fixed parameters in calls to "Func" routines in TermActionScroll()
 * 
 * Revision 1.89  94/04/22  16:44:19  16:44:19  tom (Tom Houser)
 * Term.c
 *   - added translation for home key to beginning-of-buffer
 *   - added translation for shift home key to end-of-buffer
 * TermAction.c
 *   - fixed the function _DtTermActionEndOfBuffer()
 * 
 * Revision 1.88  94/04/22  15:41:27  15:41:27  tom (Tom Houser)
 * TermFunction.c
 *    - removed some dead code.
 *    - fixed Next Page so it stops appropriately
 *    - implemented beginning-of-buffer and end-of-buffer
 * 
 * Revision 1.87  94/04/21  11:08:11  11:08:11  tom (Tom Houser)
 * TermAction.c  TermFunctionKey.c
 * - removed dead code and unused parameters from _DtTermWriteEscSeq()
 * 
 * Revision 1.86  94/04/11  17:25:39  17:25:39  tom (Tom Houser)
 * Term.c 
 *  TermAction.c 
 *  TermAction.h 
 *  TermFunction.c 
 *  TermFunction.h 
 *    -  removed dead code from these files
 * 
 * 
 * Revision 1.85  94/04/08  17:02:17  17:02:17  tom (Tom Houser)
 * Term.c Term.h TermP.h
 *   removed allowSendEvent because it belongs at the TermPrim layer
 * 
 * Revision 1.84  94/04/08  14:21:28  14:21:28  tom (Tom Houser)
 * Term.c
 *  Term.h
 *  TermP.h
 *    -  added allowSendEvents resource
 * 
 * 
 * Revision 1.83  94/04/08  11:30:28  11:30:28  tom (Tom Houser)
 * TermParse.c 
 *   - remove unnecessary Xon/Xoff logic
 *  TermParseTable.c
 *   - remove unnecessary Xon/Xoff logic
 *  TermParseTable.h 
 *   - remove unnecessary Xon/Xoff declarations.
 * 
 * 
 * Revision 1.82  94/04/06  17:16:40  17:16:40  tom (Tom Houser)
 * TermFunction.c
 *   - added "cursorOn" to next page
 * 
 * Revision 1.81  94/04/06  12:41:46  12:41:46  tom (Tom Houser)
 * Term.c
 *   - removed a duplicate action and refined a translation mapping.
 * TermPrimSelect.c
 *   - type changes to avoid compile warnings
 * TermPrimSelect.h
 *   - type changes to avoid compile warnings
 * 
 * Revision 1.80  94/04/06  08:08:21  08:08:21  daves (Dave Serisky)
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
 * Revision 1.79  94/04/05  10:51:52  10:51:52  porsche (John Milton)
 * Check point latest multi-byte work:
 *  
 *      TermBuffer.c
 *      	- more work on _DtTermBufferResize()
 *      	    - fixed core dumper when copying enhancments (this time
 *      	- added _DtTermDeleteEnhancement() now delete multiple enhancements
 *  
 *  John
 * 
 * 
 * Revision 1.78  94/03/29  16:54:52  16:54:52  tom (Tom Houser)
 * TermParseTable.c
 *   - added pre_parse_table entries to appropriate states
 * 
 * Revision 1.77  94/03/29  14:38:41  14:38:41  tom (Tom Houser)
 * Term.c
 *    - added actions copy-clipboard() and paste-clipboard()
 * 
 * 
 * Revision 1.76  94/03/28  10:48:58  10:48:58  tom (Tom Houser)
 * TermFunction.c
 *    -  reset cursor visible on hard reset.    
 *    -  reset not reverse video on hard reset.    
 * 
 * 
 * Revision 1.75  94/03/28  10:18:27  10:18:27  porsche (John Milton)
 *     TermBuffer.c
 *      	- a litte work on _DtTermBufferResize()
 *      	    - fixed core dumper when copying enhancments
 *      	    - copy the correct number of bytes when moving characters
 *                between line buffers
 *      	    - better fall back if mallocs fail
 *      	- added _DtTermBufferDelete()
 *      	- replaced eraseCharacters() with _DtTermPrimBufferErase()
 *  
 *      TermBuffer.h
 *      	- added prototype for _DtTermBufferDelete()
 *  
 *      TermFunction.c
 *      	- got rid of 'Possible nested comment' warnings
 *      	- modified calls to _DtTermPrimRefreshText() to accomodate the
 *            possibility of multi-column characters
 *      	- DtTermFuncClear() and _DtTermFuncClearToEndOfLine() are now
 *      	  column width independent
 *      	- got rid of deleteChar() it's now _DtTermBufferDelete()
 *  
 *  John
 * 
 * 
 * Revision 1.74  94/03/25  09:44:21  09:44:21  daves (Dave Serisky)
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
 * Revision 1.73  94/03/24  18:02:58  18:02:58  tom (Tom Houser)
 * Term.c
 *    -  initialized the term type from termId.
 *  TermParse.c
 *    -  added MB to xterm Text Parameters esc seq
 *    -  added DA reports for vt100 and vt101.
 *  TermParseTable.c
 *    -  made 8 bit ctrl kill escape sequence in progress as per the vt220
 * 
 * 
 * Revision 1.72  94/03/24  11:41:56  11:41:56  daves (Dave Serisky)
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
 * Revision 1.71  94/03/23  17:20:38  17:20:38  porsche (John Milton)
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
 * Revision 1.70  94/03/18  15:37:26  15:37:26  porsche (John Milton)
 *     TermBuffer.c
 *      	- Sync up with changes to TermCharInfoRec in TermPrim/TermPrimBuffer.h
 *  
 *  John
 * 
 * 
 * Revision 1.69  94/03/18  13:49:35  13:49:35  tom (Tom Houser)
 * Term.c
 *   - Changed the name of paste action from insert-selection() to copy-to()
 *     to match documentation and to motifize it.
 * 
 * Revision 1.68  94/03/18  12:58:26  12:58:26  tom (Tom Houser)
 * Term.c
 *    - added more actions for potentially up to 35 function keys (for sun)
 *  TermAction.c
 *    - changed vt-function-key-execute to take function/UDK args instead of
 *      shift/unshift.
 *  TermFunction.c
 *    - added _DtTermSetUserKeyLock() for Terminal dialog interface
 *  TermFunction.h
 *    - added declaration of _DtTermSetUserKeyLock() for Terminal dialog interface
 *  TermFunctionKey.c
 *    - added defines for number of vt and sun function keys and added logic to
 *         range check those values
 *    - fixed a bug in static declaration of sun keys.
 * 
 * 
 * Revision 1.67  94/03/17  14:06:45  14:06:45  daves (Dave Serisky)
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
 * Revision 1.66  94/03/16  11:15:08  11:15:08  porsche (John Milton)
 * Fixed multi-byte rendering problems.  _DtTermGetEnhancement() was returning
 *  the wrong value (it was looking at the line length, instead of the line
 *  width).
 *  
 *      TermBuffer.c
 *          - fixed _DtTermGetEnhancement()
 *  
 *  John
 * 
 * 
 * Revision 1.65  94/03/15  17:44:55  17:44:55  tom (Tom Houser)
 * Term.c 
 *    -  added initialization for userKeysLocked and needToLockUserKeys in the
 *       termdata structure.
 *  TermData.h 
 *    -  added definition of the afore mentioned two variables
 *  TermParse.c 
 *    -  added a new routine _DtTermParseUserKeyLoadLast() which does the 
 *       used defined key locking if necessary.
 *    -  added checks in user key routines to check whether keys are locked
 *  TermParseTable.c 
 *    -  changed parsing of User defined keys so locking can be done.
 *    -  also fixed a small bug that caused semantic to vary from vt220
 *  TermParseTable.h 
 *    -  added declaration of _DtTermParseUserKeyLoadLast().
 * 
 * 
 * Revision 1.64  94/03/15  10:33:52  10:33:52  daves (Dave Serisky)
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
 * Revision 1.63  94/03/11  09:26:43  09:26:43  porsche (John Milton)
 * Brought the DtTermBufferResize() in line with the new
 *  DtTermPrimBufferResizeBuffer() functionality.  Memory will be
 *  allocated if the buffer grows, but not if it shrinks.  This should
 *  help reduce fragmentation and improve performance.
 *  
 *      TermBufferP.h
 *          - redefined some macros
 *  
 *      TermBuffer.c
 *          - new implementation for DtTermBufferResize()
 *  
 *  John
 * 
 * 
 * Revision 1.62  94/03/08  10:38:26  10:38:26  tom (Tom Houser)
 * TermFunction.c
 *   - added setting top row to hard reset.
 *   - added check for selection in cleared memory.
 * 
 * Revision 1.61  94/03/04  16:45:22  16:45:22  tom (Tom Houser)
 * Imakefile
 *   - added MESSAGE_CAT define
 * Term.c
 *   -  added new function _DtTermGetMessage() for message cataloging
 * 
 * Revision 1.60  94/03/01  15:41:47  15:41:47  daves (Dave Serisky)
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
 * Revision 1.59  94/02/28  12:57:20  12:57:20  daves (Dave Serisky)
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
 * Revision 1.58  94/02/24  17:53:00  17:53:00  tom (Tom Houser)
 * TermData.h
 *    -  added state variables for save cursor
 *  TermFunction.c
 *    -  added state changes to soft/hard reset.
 *  TermParse.c
 *    -  added state changes to save/restore cursor escape sequence.
 * 
 * 
 * Revision 1.57  94/02/24  13:06:39  13:06:39  porsche (John Milton)
 * Moved the line wrap flag out of the DtLinePart into the TermLinePart.
 *  
 *      TermBuffer.c
 *          - no longer initializing the line wrap flag in
 *            _DtTermBufferCreateBuffer()
 *  
 *      TermBufferP.h
 *          - removed wrapped
 *  
 *  John
 * 
 * 
 * Revision 1.56  94/02/22  12:29:56  12:29:56  tom (Tom Houser)
 * TermParse.c
 *   - reworked code for 132/80 column mode yet one more time.
 * 
 * Revision 1.55  94/02/22  09:06:16  09:06:16  daves (Dave Serisky)
 * Term/Term.h:
 *      -Changed double include #define protection from Vt to Term.
 *      -Added Dt[NCR] strings, #ifdef'ed Xm[NCR] strings back to them (for
 *       the transition period).
 *      -Added #include <TermPrim.h>.
 *  
 *  Term/TermAction.c:
 *  Term/TermFunction.c:
 *  Term/TermFunctionKey.c:
 *  Term/TermParse.c:
 *  TermPrim/TermPrim.c:
 *  TermPrim/TermPrimAction.c:
 *  TermPrim/TermPrimRender.c:
 *  TermPrim/TermPrimScroll.c:
 *      -Added include of TermPrimI.h.
 *  
 *  TermPrim/TermPrim.h:
 *      -Changed double include #define protection from Term to TermPrim.
 *      -Added #include <sys/wait.h> to define pid_t.
 *      -Added callback structures for the final API.
 *      -Moved internal function prototypes to TermPrimI.h.
 *      -Added public subproc prototypes.
 *      -Added Dt[NCR] strings, #ifdef'ed Xm[NCR] strings back to them (for
 *       the transition period).
 *  
 *  TermPrim/TermPrimI.h
 *      -New internal function header file.
 *  
 *  TermPrim/TermPrimSubproc.c:
 *      -Added include of TermPrimI.h.
 *      -Changed DtTermPrimitiveSubprocReapSubproc() to DtTermSubprocReap().
 *  
 *  TermPrim/TermPrimSubproc.h:
 *      -Removed prototype of DtTermPrimitiveSubprocReapSubproc().  The
 *       prototype for the new API function is now in TermPrim.h.
 * 
 * 
 * Revision 1.54  94/02/18  12:36:35  12:36:35  tom (Tom Houser)
 * Term.c
 *  -changed a couple action names for selection to conform with the text
 *    widget.
 * 
 * Revision 1.53  94/02/18  10:08:06  10:08:06  daves (Dave Serisky)
 * TermView.c:
 *      -Fixed CMVC defect #641.  Menubar now resizes correctly when
 *       changing from 132<->80 column mode.
 *      -Fixed new problem where going from 132 to 80 column mode was ending
 *       up with 79 columns.  Could have also been a problem when turning
 *       the menubar on and off.
 * 
 * 
 * Revision 1.52  94/02/17  12:09:09  12:09:09  tom (Tom Houser)
 * Term.c
 *    - added parse entry in resource table for c132.
 *  Term.h
 *    - defined strings necessary for the parse entry for c132.
 *  TermData.h
 *    - removed allow80_132ColMode which gets replaced by c132.
 *  TermP.h
 *    - defined c132 in the TermRec structure.
 *  TermParse.c
 *    - replaced references to allow80_132ColMode by c132.
 *    - fixed a bug in code order in changing 80/132 mode.
 * 
 * 
 * Revision 1.51  94/02/16  11:15:00  11:15:00  daves (Dave Serisky)
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
 * Revision 1.50  94/02/15  13:45:26  13:45:26  daves (Dave Serisky)
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
 * Revision 1.49  94/02/14  15:07:54  15:07:54  daves (Dave Serisky)
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
 * Revision 1.48  94/02/11  11:38:01  11:38:01  tom (Tom Houser)
 * sync hp tree with files changed and checked into the cde tree.
 * These files were synced:
 * Term.c, TermAction.c, TermEnhance.c, TermFunction.c TermParse.c,
 * TermVersion.c, Imakefile
 * 
 * Revision 1.34  94/02/08  10:03:43  10:03:43  tom_hp_cv
 * Term.c 
 *   - added action table entries for select-page and select-all
 *  TermData.h 
 *   - added state variable for font state
 *  TermEnhance.c 
 *   - added code for setting font or OVERSTRIKE
 *  TermFunction.c 
 *   - added new enhancements
 *  TermParse.c 
 *   - added marginBell escape sequence
 * 
 * 
 * Revision 1.33  94/01/31  16:58:27  16:58:27  daves_hp_cv
 * 
 *  Massive checkin of all changes made since November Developer's
 *  Conference CD.
 * 
 * 
 * Revision 1.47  94/01/31  10:44:46  10:44:46  daves (Dave Serisky)
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
 * Revision 1.46  94/01/28  09:52:19  09:52:19  daves (Dave Serisky)
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
 * Revision 1.45  94/01/27  18:00:56  18:00:56  tom (Tom Houser)
 * TermParse.c
 *   - fixed cursor position after Erase in Display
 * 
 * Revision 1.44  94/01/27  17:17:41  17:17:41  tom (Tom Houser)
 * TermParse.c
 *   - fix insert and delete lines outside of scrolling region
 *  TermFunction.c
 *   - fixed delete routine by not calling _DtTermPrimDeleteLines() since
 *     it does not handle corner cases like the first or last line in the buffer.
 * 
 * 
 * Revision 1.43  94/01/27  15:59:25  15:59:25  porsche (John Milton)
 * Fixed a problem erasing characters, we now clear the associated
 *  enhancements as well.  This fixes CMVC bug #418.
 *  
 *      Term/TermBuffer.c:
 *      	- removed code that had been ifdef'd out
 *      	- added helper function clearEnhancements() to clear
 *      	  enhancements between two specified locations
 *      	- added _DtTermBufferErase() to erase characters and their
 *            associated enhancements
 *  
 *      Term/TermBuffer.h:
 *      	- added typedef for enumerated type DtEraseMode that specifies
 *            how characters are to be erased
 *      	- added prototype for _DtTermBufferErase
 *  
 *      Term/TermFunction.c:
 *      	- added helper function termFuncErase() to help when erasing 
 *      	  characters
 *      	- added _DtTermFuncEraseInDisplay(), _DtTermFuncEraseInLine(),
 *      	  and _DtTermFuncEraseCharacter()
 *  
 *      Term/TermFunction.h:
 *      	- added prototypes for above mentioned _DtTermFunc* functions
 *  
 *      Term/TermParse.c
 *      	- _DtTermEraseChars(), _DtTermEraseDisplay(), and
 *      	  _DtTermEraseInLine() now call there respective _DtTermFunc*
 *      	  counterparts
 *  
 *  John
 * 
 * 
 * Revision 1.42  94/01/21  17:19:06  17:19:06  tom (Tom Houser)
 * TermParse.c
 *   - added logging toggle on and off              
 *   - fixed a bug in DECSTBM  (don't believe this one was logged)
 * 
 * 
 * Revision 1.41  94/01/21  14:45:18  14:45:18  porsche (John Milton)
 * Fixed a problem with Cursor Horizontal Absolute escape sequence, it was using 1
 *  based counting instead of 0 based counting.  This fixed 2 bugs that were file
 *  against dtterm 434, and 433 (in CMVC)
 *  
 *  
 *      Term/TermParse.c:
 *          _DtTermCursorToCol() now subtracts BASE before setting the cursor column
 *  
 *  John
 * 
 * 
 * Revision 1.40  94/01/21  09:37:56  09:37:56  tom (Tom Houser)
 * Term.c
 *   - added action and translation for drag begin
 * 
 * 
 * Revision 1.39  94/01/20  10:25:07  10:25:07  daves (Dave Serisky)
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
 * Revision 1.38  94/01/19  16:29:51  16:29:51  tom (Tom Houser)
 * Term.c
 *  - added translations for select extension
 * 
 * Revision 1.37  94/01/13  14:03:42  14:03:42  tom (Tom Houser)
 * TermBuffer.c and TermBuffer.h
 *   -fixed buffer create interface to match the change made in the TermPrim
 *    version
 * 
 * Revision 1.36  93/12/15  16:27:02  16:27:02  daves (Dave Serisky)
 * TermParse.c:
 *      -Added initial support for multibyte parser input.
 * 
 * 
 * Revision 1.35  93/12/14  11:01:34  11:01:34  daves (Dave Serisky)
 * 
 *  [checked in on 93/12/13 21:06:52 by daves]
 *  
 *  DtTermMain.c:
 *   	-Removed duplicate #include of TermPrimBufferP.h.
 *   
 *   Imakefile:
 *   Term/Imakefile:
 *   TermView/Imakefile:
 *   	-Added required stuff to build archived and shared libraries.
 *   
 *   
 *  
 * 
 * 
 * Revision 1.4  93/12/13  21:06:52  21:06:52  daves (Dave Serisky)
 * DtTermMain.c:
 *  	-Removed duplicate #include of TermPrimBufferP.h.
 *  
 *  Imakefile:
 *  Term/Imakefile:
 *  TermView/Imakefile:
 *  	-Added required stuff to build archived and shared libraries.
 *  
 *  
 * 
 * 
 * Revision 1.3  93/11/04  12:54:18  12:54:18  tom (Tom Houser)
 * TermParse.c
 *   - fixed a problem with selective erase.
 *   - changed the semantics of ECH (erase char) to match vt220
 * 
 * Revision 1.2  93/10/08  15:34:07  15:34:07  tom (Tom Houser)
 * Term.c
 *   - add initialization of the saveDECMode structure
 *   - added initialization of the vt100/200 compatibility flag.
 *   - added initialization of the selective erase mode.
 *   - added an action for shifted Prior and Next
 * TermAction.c
 *   - fixed the previous and next page action.
 * TermData.h
 *   - added 80/132 column mode, and compatibility level mode.
 *   - removed allowReverseWrapMode flag which isn't needed.
 * TermFunction.c
 *   - tweaked the doScroll function.
 * TermParse.c
 *   - added mode changing for 80/132 column mode, and compatibility level mode
 *   - added actually esc seq to the comments for each routine.
 *   - implemented selective erase mode setting and selective erase functions.
 *   - fixed DA (Device Attributes) to report for vt100
 *   - implemented SU and SD (scroll up and scroll down)
 * TermParseTable.c
 *   - fixed parsing for selective erase.
 * TermParseTable.h
 *   - added declaration for _DtTermSetCompatLevel(Widget w)
 * 
 * Revision 1.1  93/09/27  15:34:47  15:34:47  tom (Tom Houser)
 * Initial revision
 * 
 * Revision 1.32  93/09/23  16:08:57  16:08:57  tom_hp_cv
 * Term.c
 *    - removed some hp specific things wrt function keys
 *    - added init and clear for user function keys
 *  TermData.h
 *    - added userKeyString array to widget data structure.
 *  TermFunctionKey.c
 *    - removed all the HP function key functions
 *    - added functions for Vt user defined keys
 *  TermParse.c
 *    - implemented CPL (Cursor Previous Line)
 *    - implemented DECUDK  (Set user defined keys)
 *  TermParseTable.c
 *    - expanded parse routines to handle all the parsing for DECUDK
 *  TermParseTable.h
 *    - added declarations to do DECUDK (user defined keys)
 *  TermParseTableP.h
 *    - added declarations to do DECUDK (user defined keys)
 * 
 * 
 * Revision 1.31  93/09/22  16:54:00  16:54:00  daves_hp_cv
 * *** empty log message ***
 * 
 * Revision 1.30  93/09/21  16:11:51  16:11:51  tom_hp_cv
 * TermSendEsc.h
 *     - changed the edit keys BACK to the right thing (got confused by
 *     - incorrect documentation)  e.g. "Prev Screen" should be ESC[5~
 *             not ESC[5
 * 
 * 
 * Revision 1.29  93/09/20  16:17:07  16:17:07  tom_hp_cv
 * Term.c
 *   - removed the init of some old state variables and added some for SaveCursor.
 *  TermData.h
 *   - removed some old state variables and added some for SaveCursor.
 *   - added a new (sub)substructure to hold DECSET/DECRST save values.
 *  TermFunction.c
 *   - fixed a bug in video enhancements (I wasn't saving the state)
 *   - changed SaveCursor to use the new (sub)structure.
 *  TermParse.c
 *   - changed SaveCursor to use the new (sub)structure.
 * 
 * 
 * Revision 1.28  93/09/16  17:45:02  17:45:02  tom_hp_cv
 * TermColor.c
 *    -  added one more color pair for ISO color selection
 *  TermData.h
 *    -  increase the color pair array by one for ISO color selection
 *  TermFunction.c
 *    -  fixed off-by-one bug in setting colors and a bug in clear line
 *  TermParse.c
 *    -  fixed a bug in Cursor Back (CUB) involving reverse wrap
 * 
 * 
 * Revision 1.27  93/09/10  16:19:37  16:19:37  tom_hp_cv
 * TermFunction.c
 *    - added "return to defaults" for color enhancement  (SGR 100)
 * 
 * 
 * Revision 1.26  93/09/10  15:50:20  15:50:20  porsche_hp_cv
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
 * Revision 1.25  93/09/10  15:09:17  15:09:17  daves_hp_cv
 * Term.c:
 *      -Turned on historybuffer in class.
 *      -Removed TermData's autoLineFeed (in TermPrimData).
 *  
 *  TermData.h:
 *      -Removed TermData's autoLineFeed (in TermPrimData).
 *  
 *  TermFunction.c:
 *      -Changed source of autoLineFeed.
 *  
 *  TermParse.c:
 *      -Changed source of autoLineFeed.
 *  
 *  TermViewMenu.c:
 *      -Changed help volume and topics to reflect the dtterm names.
 * 
 * 
 * Revision 1.24  93/09/10  14:39:20  14:39:20  tom_hp_cv
 * Term.c
 *    - added editkey actions (Find, Insert, Select, Prior, Next)
 *  TermAction.c
 *    - added the action for edit keys (Find, Insert, Select, Prior, Next)
 *  TermAction.h
 *    - declarations for edit key execute function
 *  TermColor.c
 *    - tickled the hp color initialization for vt
 *  TermEnhance.c
 *    - added check for inititialization for bg color
 *  TermFunction.c
 *    - added code for color enhancement
 *  TermSendEsc.h
 *    - changed xterm esc seq (sent) to vt220 esc seq for edit keys.
 * 
 * 
 * Revision 1.23  93/09/09  17:24:43  17:24:43  porsche_hp_cv
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
 *            - added 'Boolean' to TermEnhInsert prototype
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
 * Revision 1.22  93/09/08  18:29:33  18:29:33  tom_hp_cv
 * Term.c
 *      - added app mode keypad events
 *  TermAction.c
 *      - added semantic routine for normal/app mode keypad keys
 *  TermAction.h
 *      - declarations for app mode keypads keys
 *  TermParse.c
 *      fixed bug in xterm change text escape seq.
 *  TermSendEsc.h
 *      - added esc seqs for app mode keypad keys.
 * 
 * 
 * Revision 1.21  93/09/07  17:59:33  17:59:33  tom_hp_cv
 * TermParse.c
 *     fixed stack handling logic that I corrupted during last fixes. e.g. SGR  
 *     fixed handling of xterm Text Parameter escape seq string parsing.
 *  TermParseTable.c
 *     Fixed parsing for xterm Text Parameter escape seq.
 * 
 * 
 * Revision 1.20  93/09/07  16:28:56  16:28:56  daves_hp_cv
 * TermFunction.c:
 *      -Modified _DtTermFuncInsertLine to correctly insert lines within the
 *       scrolling region.
 * 
 * 
 * Revision 1.19  93/09/03  16:28:44  16:28:44  tom_hp_cv
 * TermFunction.c
 *   - removed some prints
 *  TermParse.c
 *   - replaced the parm count preamble in many functions with STORELASTARG
 *   - filled out the Save and Restore Cursor esc. seq.
 *   - added the implementation for DECALN
 *   - added the implementation for CHA (Cursor Horizontal Absolute)
 *  TermParseTable.c
 *   - changed the parsing of the icon and title string to include all
 *     7 bit printable characters.
 * 
 * 
 * Revision 1.18  93/09/02  17:48:44  17:48:44  tom_hp_cv
 * Term.c 
 *    - added resources sunFunctionKeys, autoWrap, and reverseWrap.
 *  Term.h 
 *    - added declarations fo sunFunctionKeys, autoWrap, and reverseWrap.
 *  TermFunctionKey.c 
 *    - added sunFunctionKeys
 *  TermP.h 
 *    - added declarations to DtTermPart for sunFunctionKeys, autoWrap, and 
 *      reverseWrap.
 *  TermSendEsc.h 
 *    - added defines for sunFunctionKeys
 * 
 * 
 * Revision 1.17  93/09/02  10:44:31  10:44:31  daves_hp_cv
 * Term.c:
 *      -Added emulationId resource for setting $TERMINAL_EMULATOR.
 *      -Changed widget class name from Vt to Term.
 *      -Removed the fast subclass hack.
 *  
 *  TermView.c:
 *      -Added emulationId resource for setting $TERMINAL_EMULATOR.
 *      -Added the termDeviceAllocate resource and support to take the
 *       burden of pty allocation off of the application.
 *      -Removed the allocation of the pty (now done by the DtTermPrim
 *       widget).
 *  
 *  TermViewMenu.c:
 *      -Removed jump scroll toggle.
 *      -Enabled hard and soft resets.
 *      -Disabled all but the helpOnHelp help buttons.
 *  
 *  TermViewP.h:
 *      -Added emulationId resource for setting $TERMINAL_EMULATOR.
 *      -Added the termDeviceAllocate resource and support to take the
 *       burden of pty allocation off of the application.
 *  
 *  DtTermMain.c:
 *      -Removed hpterm specific command line options.
 *      -Added all the obsolete command line options.
 *      -Fixed instance counting in non-server mode so that we don't
 *       terminate prematurly.
 *      -Added invocation of DtTermMotif12Patch() to take care of the
 *       ugly 1.1 hack that breaks us.
 *      -Added support for the -S option.
 * 
 * 
 * Revision 1.16  93/09/01  18:11:17  18:11:17  tom_hp_cv
 * TermParse.c
 *    - Changed the way the parser work to accomodate missing numbers
 *      e.g. ESC [ ;2 H
 *      This also streamlined the semantic handler which now does not need to
 *      check the number of parameters
 *    - Added support for Erase Line from beginning of line.
 *    - Added support for Erase Display from beginning of display.
 *    - Added implementation for changing window title and icon name
 *    - Added support for reverseWrap in CUB and Backspace.
 *  TermParseTable.c
 *    - Added parsing for Backspace, DECUDK (Softkeys), and fixed parsing
 *      for down-line-loading fonts.
 *  TermParseTable.h
 *    - added declarations for the above changes.
 *  TermParseTableP.h
 *    - added declarations for the above changes.
 * 
 * 
 * Revision 1.15  93/09/01  16:05:52  16:05:52  porsche_hp_cv
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
 * Revision 1.14  93/08/27  15:17:22  15:17:22  daves_hp_cv
 * DtTermLogit.c:
 *      -Turned off usage logging.
 *  
 *  DtTermMain.c:
 *      -Turned off usage logging.
 *      -Added new resources and parameters for the new improved server
 *       mode.
 *  
 *  DtTermServer.c
 *      -Turned off usage logging.
 *      -Changed server mode to the new server definition.  It works!
 *  
 *  DtTermServer.h
 *      -Changed function prototypes to include the new parameters for the
 *       new improved server mode.
 * 
 * 
 * Revision 1.13  93/08/26  16:16:18  16:16:18  daves_hp_cv
 * First Dt'ified version.
 * 
 * 
 * Revision 1.12  93/08/25  16:48:21  16:48:21  porsche_hp_cv
 * Changes accomodate extraction of emulator specific functionality from
 *  TermBuffer code (see the rlog in Term/version.c for more info).
 *  
 *      ANSIterm
 *          Vt.c
 *              - initialize the term_primitive_class with Vt specific info
 *  
 *          VtBuffer.c
 *          TermBuffer.h
 *          TermBufferP.h
 *              - implementation of VT specific buffer behavior
 *      
 *          VtEnhance.c
 *              - include TermBuffer.h instead of TermPrimBuffer.h
 *  
 *          VtFunction.c
 *              - now including TermBuffer.h
 *  
 *          Imakefile
 *              - added VtBuffer.[co] to SRCS1 and OBJS1 respectively
 *  John Milton 
 *  
 *  8/25/93
 * 
 * 
 * Revision 1.11  93/08/20  16:56:17  16:56:17  tom_hp_cv
 * VtParse.c
 *    fixed origin mode bug in cursor pos (CUP)
 *    added support for preventing linefeed going below bottom margin
 *  VtParseTable.c
 *    added parsing routines for linefeed
 *  TermParseTable.h
 *    extern declaration for linefeed
 * 
 * 
 * Revision 1.10  93/08/19  17:29:52  17:29:52  tom_hp_cv
 * Vt.c
 *     added initialization of IsCursorStateSaved
 *  TermData.h
 *     added vt220 and xterm states to DtTermData
 *  VtFunction.c
 *    added soft reset
 *    saved state of video enhancements  (still doesn't work till Term does)
 *  VtFunctionKey.c
 *    added vt220 style function keys (no user defined yet)
 *  VtParse.c
 *    reworked cursor routines and added softreset interface
 *  VtParseTable.c
 *    fixed parsing for softreset
 *  TermParseTable.h
 *    declarations for changes to VtParseTable.c
 *  TermParseTableP.h
 *    declarations for changes to VtParseTable.c
 * 
 * 
 * Revision 1.9  93/08/19  10:35:03  10:35:03  daves_hp_cv
 * Vt.c:
 *      -Cleaned up default translations.
 * 
 * 
 * Revision 1.8  93/08/18  18:10:18  18:10:18  tom_hp_cv
 * Vt.c
 *  removed some function key actions
 *  VtAction.c
 *    Added support for 8bit C1 transmission
 *    Renamed invokeAction to _DtTermWriteEscSeq and made it global
 *  VtFunction.c
 *    Reworked some the of the hpterm functions for ANSI.
 *  VtParse.c
 *    Reworked some of the routines to handle top and bottom margin constraints.
 *    Toggled off wrap for some routines by moving cursor from lastcol+1 to lastcol
 * 
 * 
 * Revision 1.7  93/08/18  16:22:35  16:22:35  daves_hp_cv
 * HpTerm.c:
 *      -Changed from internal to public XmRepType functions and headers.
 *  
 *  HpTerm.c,
 *  TermView.h,
 *  HpTermFunctionKey.c,
 *  HpTermFunctionKey.h,
 *  HpTermFunctionKeyP.h,
 *  HpTermLogit.c,
 *  DtTermLogit.h,
 *  HpTermMain.c,
 *  DtTermMain.h,
 *  HpTermMenu.c,
 *  TermViewMenu.h,
 *  TermViewP.h,
 *  HpTermServer.c,
 *  DtTermServer.h,
 *  HpTermSyntax.c,
 *  DtTermSyntax.h:
 *      -Removed function key area and support code.
 *      -Moved from coeterm and hpterm to dtterm.
 *  
 * 
 * 
 * Revision 1.6  93/08/16  17:27:51  17:27:51  daves_hp_cv
 * ANSITerm.c:
 *      -Blew away the [forward|reverse]_wrap_procs.
 *      -Added wrap_right_after_insert to term widget class.
 * 
 * 
 * Revision 1.5  93/08/13  18:39:07  18:39:07  tom_hp_cv
 * Vt.c
 *     - added initialization for origin and c1 modes
 *  TermData.h
 *     - added origin and c1 modes in DtTermDataRec
 *  VtParse.c
 *     - added parsing for some vt220 only escape seqs (Double wide chars etc)
 *     - reworked many parsing routines for handling too many parameters,
 *       and some that didn't handle all their parameters.
 *     - got insert char ICH working
 *     - implemented the setting of top and bottow scrolling margins.
 *  VtParseTable.c
 *     - added table entries for some vt200 specific escape sequences
 *  TermParseTable.h
 *     - added declarations for some vt220 specific escape sequences
 *  TermParseTableP.h
 *     - added declarations for some vt220 specific escape sequences
 * 
 * 
 * Revision 1.4  93/08/11  18:29:55  18:29:55  tom_hp_cv
 * VtAction.c
 *    changed some of the hp escape seq. to Vt versions.
 *  VtParse.c
 *    added implementation for RI, NEL, IND and DECREQTPARM
 *    fixed the default case (too many parameters) for some escape sequences
 *    added some doc on the handling of numeric parameters
 *  VtParseTable.c
 *    added recognition of the 9 C1 control chars
 * 
 * 
 * Revision 1.3  93/08/11  14:54:47  14:54:47  daves_hp_cv
 * Vt.c:
 *      -Added Term widget class variables use_history_buffer and
 *       allow_scroll_below_buffer.
 *      -Added forward_wrap_proc and reverse_wrap_proc to DtTermPrimitiveClassPart
 *       initialization.
 * 
 * 
 * Revision 1.2  93/08/09  13:16:23  13:16:23  daves_hp_cv
 * HpTerm.c:
 *      -Changed from subprocessLoginShell to loginShell resource.
 *      -Changed from terminalFontList to userFont.
 *      -Cleaned up the initialize checklist.
 *  
 *  HpTermMain.c:
 *      -Changed widget names, shell class names, etc from hpterm based to
 *       coeterm based.
 *      -Added support for the loginShell variable.
 *  
 *  TermViewP.h:
 *      -Removed statusVisible and recolorAreas variables.
 *  
 *  HpTermServer.c:
 *      -Added support for loginShell.
 *      -Changed names from hpterm based to coeterm based.
 *  
 *  DtTermServer.h:
 *      -Added loginShell to function prototype.
 * 
 * 
 * Revision 1.1  93/07/01  09:40:58  09:40:58  tom
 * Initial revision
 * 
 * Revision 1.77  93/06/11  10:57:48  10:57:48  porsche (John Milton)
 * Added support for the ttyModes resource...
 * 
 *     Term/TermSetPty.c
 *         - now parsing dsusp again (but its ignored on HP-UX)
 * 
 * jrm 06/10/93
 * 
 * Revision 1.76  93/06/10  16:16:16  16:16:16  porsche (John Milton)
 * Added support for the ttyModes resource...
 * 
 *     Term/TermSetPty.c
 *         - added implementation of parseTtyModes()
 *         - _DtTermPrimPtyInit() now accepts a ttyModes string, parses the string
 *           (via parseTtyModes()), and sets any options specified by the
 *           string
 * 
 *     Term/TermPrimSetPty.h
 *         - declarations for ttyMode structure and related defines
 *         - updated _DtTermPrimPtyInit() prototype
 * 
 *     Term/TermPrimSubproc.h
 *         - modified call to _DtTermPrimPtyInit() to provide ttyModes string
 * 
 * jrm 06/10/93
 * 
 * Revision 1.75  93/06/10  13:50:52  13:50:52  daves (Dave Serisky)
 * Hp70096.c:
 *     -Added support for termId and termName resources.
 * 
 * HpTermMain.c:
 *     -Added +block and +standAlone command line options.
 * 
 * HpTermSyntax.c:
 *     -Removed commas from help and syntax messages.
 *     -Removed [*] from -tm, -tn, and, -ti options.
 * 
 * Term.c:
 *     -Added support for termId and termName resources.
 * 
 * TermPrim.h:
 *     -Added support for termId and termName resources.
 * 
 * TermPrimP.h:
 *     -Added support for termId and termName resources.
 * 
 * Revision 1.74  93/06/09  16:17:59  16:17:59  daves (Dave Serisky)
 * TermSetUtmp.c:
 *     -Fixed utmp code to rewind the file before searching for an entry
 *      (so we continue to work).  Also, close the file when we are done
 *      with it.
 * 
 * Revision 1.73  93/06/09  14:48:20  14:48:20  daves (Dave Serisky)
 * Added alpha what string.
 * Changed translation to use insert() instead of key-input().
 * 
 * Revision 1.72  93/06/09  13:54:47  13:54:47  porsche (John Milton)
 * Some hpterm 2.0 compatability...
 * 
 *     HpTerm/HpTermMain.c
 *         - now parsing the '-n' option to set the icon name (this is for
 *           backwards compatability with hpterm 2.0)
 * 
 * jrm 06/09/93
 * 
 * Revision 1.71  93/06/09  12:47:12  12:47:12  porsche (John Milton)
 * A little more work on Cut/Paste (for alpha) and some incidental cleanup...
 * 
 *     Hp70096/Hp70096.c
 *         - now freeing up BlockModeInfo in the Destroy() proc (this was
 *           a memory leak found by Sentinel)
 * 
 *     Hp70096/Hp70096BlockMode.c
 *         - casting pointer in calls to _DtTermPrimPtyWrite() to (unsigned char *)
 *           to prevent warnings
 *         - correctly locking/unlocking the keyboards on pending transfers
 * 
 *     Term/Term.c
 *         - eliminated declaration of bytesWritten in _DtTermPrimPtyWrite()
 *           (lint was complaining)
 * 
 *     Term/TermPrim.h
 *         - added prototype for _DtTermPrimPtyWrite() (lint was complaining)
 * 
 *     Term/TermBuffer.c
 *         - _DtTermPrimBufferInsert() now returns 0 on exit (lint was complaining)
 * 
 *     Term/TermSelect.c        
 *         - now including Xm/AtomMgr.h (for XmInternAtom() prototype)
 *         - removed unused variables in various functions (lint was complaining)
 *         - setSelection() no longer calls _DtTermPrimRenderRefreshTextLinear()
 *           with unintialized values
 *         - setSelection() no properly handles the case where the user
 *           starts a new selection in the middle of an existing selection
 *         - _DtTermPrimSelectIsInSelection() now uses the local copys of 
 *           selectInfo->begin and selectInfo->end
 *         - _DtTermPrimSelectConvert() now returns True on success (until this
 *           change, a fully optimized hpterm 3.0 would not paste any data,
 *           (a debuggable one would))
 * 
 * jrm 06/09/93
 * 
 * Revision 1.70  93/06/03  08:33:01  08:33:01  porsche (John Milton)
 * Cut/Paste now limping along (well enough for alpha)...
 * 
 *     Hp70096/Hp70096.c
 *         - grab-focus, select-extend, extend-end, and insert-selection
 *           actions no longer work with shift, or control
 * 
 *     Term/Term.c
 *         - _DtTermPrimPtyWrite simply appends all text to the pending text buffer
 *           then calls _DtTermPrimStartOrStopPtyOutput() to turn on the write select
 *           (this forces all write pacing to go through the dispatch loop)
 *         - _DtTermPrimStartOrStopPtyOutput() only adds a write select if one
 *           doesn't already exist
 * 
 *     Term/TermBuffer.c
 *     Term/TermPrimBuffer.h
 *         - added _DtTermPrimBufferGetText(), to retrieve the requested number of
 *           bytes from the termbuffer
 * 
 *     Term/TermPendingText.c
 *         - added some debugging code to walk the pending text queue
 *         - _DtTermPrimPendingTextRemoveChunk() was checking for head and tail
 *           rather than head or tail
 * 
 *     Term/TermRender.c
 *         - moved isInSelection() to TermSelect.c and renamed it to
 *           _DtTermPrimSelectIsInSelection()
 * 
 *     Term/TermSelect.c        
 *     Term/TermPrimSelect.h
 *     Term/TermPrimSelectP.h        
 *         - major changes to implement the cut/paste functionality
 * 
 * jrm 06/03/93
 * 
 * Revision 1.69  93/06/01  14:50:47  14:50:47  daves (Dave Serisky)
 * Fixed HpTerm Initialize() to fix some incorrectly nested if
 * statements (i.e. missing {}s) that were causing a zero derefrence
 * core dump.
 * 
 * Revision 1.68  93/05/26  13:20:49  13:20:49  daves (Dave Serisky)
 * HpTerm.c:
 *     -Did some codecenter cleanup.
 * 
 * TermView.h:
 *     -Fixed the declaration of TermHptermCloneCallback();
 * 
 * HpTermFunctionKey.c:
 *     -Did some codecenter cleanup.
 * 
 * HpTermMain.c:
 *     -Fixed a malloc error.
 *     -Added invocation logging.
 * 
 * HpTermServer.c:
 *     -Added code to terminate 15 minutes after last session ends.
 *     -Added cleanup code.
 *     -Added signal handler for normal user kill signals
 *     -Added invocation logging.
 * 
 * HpTermSyntax.c:
 *     -Did some codecenter cleanup.
 * 
 * Imakefile:
 *     -Added HpTermLogit.[co].
 * Term.c:
 *     -Removed duplicate resource of XmNstatusChangeCallback which was
 *      causing a double free on widget destroy.
 *     -Some codecenter cleanback.
 *     -Removed some dead code.
 * 
 * TermBuffer.c:
 *     -Pulled some static declarations into file from TermPrimBufferP.h.
 * 
 * TermPrimBufferP.h:
 *     -Yanked some static declarations and put into TermBuffer.c.
 * 
 * TermCursor.c:
 *     -Some codecenter cleanback.
 *     -Removed some dead code.
 * 
 * TermSetUtmp.c:
 *     -Added missing #include of <pwd.h>.
 * 
 * TermSubproc.c:
 *     -Fixed code to correctly pass back the stat_loc from the wait()
 *      call.
 * 
 * Revision 1.67  93/05/22  08:18:41  08:18:41  daves (Dave Serisky)
 * HpTerm.c:
 *     -Added command line arg and resource to turn on functionKeys
 *      initially.  Default is initially off.
 *     -Changed scrollbar initial state to default off.
 *     -Fixed up support of -e command.
 *     -Removed lots of obsolete code.
 * 
 * TermView.h
 *     -Removed obsolete code.
 *     -Changed names from "softkey" to "functionkey".
 * 
 * HpTermFunctionKey.c
 *     -Changed to not create the function keys managed.
 *     -Changed to always update the position labels.  This way we will
 *      have the correct position when the function keys are mapped.  After
 *      not being mapped.
 * 
 * HpTermMain.c
 *     -Added server support.
 *     -Added support for lots of other options including -help, -e, -?,
 *      -fk, -sb.
 *     -Tried to add via DEBUG_INCLUDES all the private include files, but
 *      I still can't access some structures.
 *     -Added 5 minute pings to the X server to see if this will prevent
 *      hung sessions after the X server dies.
 *     -Removed obsolete code (including the font, geometry -multi dialog
 *      box.
 * 
 * HpTermMenu.c
 *     -Removed old status and function key toggle menu items.  Renamed
 *      from menu* to HpTermMenu*.
 * 
 * TermViewP.h
 *     -Renamed include file from functionKey.h to HpTermFunctionKey.h.
 *     -Changed references from softKey to functionKey.
 * 
 * Imakefile
 *     -Added HpTermSyntax.[co], HpTermServer.[ch].
 *     -Switched from functionKey* and menu* to HpTermFunctionKey* and
 *      HpTermMenu*.
 *     -Added libCoe.a to library list.
 * Term.c:
 *     -Kluged in support for subprocessCmd and subprocessArgv to get the
 *      -e option to actually work.
 * 
 * TermLineDraw.c:
 *     -Fixed bogus initialization of tw bug (was being initialized to tw,
 *      should have been w).
 * 
 * TermRender.c:
 *     -Added support to reverse video the selection area.
 * 
 * TermSetPty.c:
 *     -Added a function to grab a set of initial default terminal modes so
 *      the server can grab them before doing a setpgid().
 * 
 * TermPrimSetPty.h:
 *     -Header declarations for above.
 * 
 * TermSubproc.c:
 *     -Removed vfork (for now) and uses fork for everything.
 *     -Added calls to reset alarms and 
 * 
 * Revision 1.66  93/05/18  10:44:28  10:44:28  daves (Dave Serisky)
 * Hp70096.c:
 *     -Moved function key post and toggle functions from Hp70096Action.c
 *      to Hp70096FunctionKey.c.
 * 
 * Hp70096Action.c:
 *     -Changed to call _hp70096FunctionKeyExecute() with keys relative to
 *      F1==1.
 *     -Removed the function key post and toggle functions.
 * 
 * Hp70096FunctionKey.c:
 *     -Extended the FunctionKeyUpdate() function to post and toggle the
 *      function keys.
 *     -Added functions to post and toggle the function keys.
 * 
 * Hp70096FunctionKey.h:
 *     -Added typedefs and callback struct fields to support posting and
 *      toggling of function keys.
 * HpTerm.c:
 *     -Added XmNfunctionKeyLast resource and support for it.
 * 
 * functionKey.c:
 *     -Linked user keys to the Hp70096 widgets user keys.
 *     -Fixed code so that it works (including 16 or 24 function keys).
 *     -Changed code to use a numKeys of 16 or 24 instead of 8 or 12 to be
 *      consistant with the Hp70096FunctionKey.c code.
 *     -Changed function names from _term.*() to _hpterm.*().
 * 
 * functionKey.h:
 *     -Changed function names from _term.*() to _hpterm.*().
 * 
 * functionKeyP.h:
 *     -Removed extraneous info from the user key information.
 *     -Removed our locak FunctionKeyClientData structure.
 * 
 * main.c:
 *     -Blew away all the use* resources.
 * 
 * menu.c:
 *     -Changed things to support the new help volume (which includes a
 *      version window).
 * 
 * 
 * Revision 1.65  93/05/12  15:47:17  15:47:17  daves (Dave Serisky)
 * HpTerm.c:
 * functionKey.c:
 * functionKey.h:
 * functionKeyP.h:
 * main.c:
 *     -Modified appearance.
 *     -Merged status bar into the function keys.
 *     -Removed function keys.
 *     -Fixed up some geometry bugs.
 * 
 * Revision 1.64  93/05/12  14:12:10  14:12:10  porsche (John Milton)
 * Paving the way for cut/paste...
 * 
 *     Hp70096/Hp70096.c
 *         - now includes TermPrimSelect.h
 *         - setup actions for grab-focus, select-extend, extend-end, and
 *           insert-selection
 *     
 *     Term/Imakefile
 *         - added TermSelect.[co] to SRCS/OBJS
 * 
 *     Term/Term.c
 *         - now includes TermPrimSelect.h
 *         - Initialize() calls _DtTermPrimSelectCreate()
 *         - Destroy() calls _DtTermPrimSelectDestroy()
 *         - grab-focus maps to _DtTermPrimSelectGrabFocus()
 *         - removed _termActionGrabFocus() definition
 * 
 *     Term/TermPrimData.h
 *         - now includes TermPrimSelect.h
 *         - added selectInfo to DtTermPrimDataRec
 * 
 *     Term/TermSelect.c        
 *     Term/TermPrimSelect.h
 *     Term/TermPrimSelectP.h        
 *         - definition files for cut/paste functionality
 * 
 * jrm 05/12/93
 * 
 * Revision 1.63  93/05/07  09:44:28  09:44:28  porsche (John Milton)
 * Cursor parsing now uses the parser context sign...
 * 
 *     cursor.c
 *         - [SG]etMoveSign() becomes [SG]etSign()
 * 
 *     cursor.h
 *         - removed [SG]etMoveSign() definitions
 *         - removed moveSign enumerated type
 * 
 * jrm 05/07/93
 * 
 * Revision 1.62  93/05/06  16:44:35  16:44:35  porsche (John Milton)
 * It just gets better all the time...
 * 
 *     Hp70096Action.c
 *     Hp70096Action.h
 *         - removed _hp70096ActionDeleteCharWrap(), _hp70096ActionDeleteChar()
 *           handles it all
 * 
 *     Hp70096Parse.c
 *         - implemented _hp70096ParseHomeDown()
 * 
 * jrm 05/06/93
 * 
 * Revision 1.61  93/05/06  15:16:48  15:16:48  daves (Dave Serisky)
 * Hp70096Parse.c:
 *     -Modified to use the new Term parser support for signs.
 * 
 * Hp70096ParseTable.c:
 *     -Modified to use the new Term parser support for signs.
 *     
 * Hp70096FunctionKey.c:
 *     -Modified to clip key labels to 16 chars and strings to 80 chars.
 * 
 * main.c:
 *     -Added yet another private include file.
 * 
 * TermParser.c:
 *     -Added new parser support for signs.
 * 
 * TermPrimParserP.h:
 *     -Added declarations for new parser support for signs.
 * 
 * Revision 1.60  93/05/06  14:23:18  14:23:18  porsche (John Milton)
 * Made the delete character functionality more modular.
 * 
 *     Hp70096Action.c
 *     Hp70096Action.h
 *         - added _hp70096ActionDeleteCharWrap()
 * 
 *     Hp70096Function.c
 *     Hp70096Function.h
 *         - added deleteChar() to delete a character with or without wrap
 *         - added _hp70096FunctionDeleteCharWrap()
 *         
 *     Hp70096ParseTable.c
 *     Hp70096ParseTable.h
 *         - added _hp70096ParseDeleteCharWrap()
 * 
 * jrm 05/06/93
 * 
 * Revision 1.59  93/05/06  13:37:02  13:37:02  daves (Dave Serisky)
 * Hp70096.c:
 *     -Changed the _hp70096FunctionKeyCreate() and
 *      _hp70096FunctionKeyDestroy() functions to create and destoy opaque
 *      structure instead of taking an hp70092 widget as the argument.
 * 
 * Hp70096Parse.c:
 *     -Fixed default values for user key label and string (were reversed).
 *     -Changed _hp70096FunctionKeyUserKeySet() to return a boolean that is
 *      True if we don't need to parse a string or label (such as when an
 *      invalid key number was seen).
 *     -Fixed the string/label parsing code to finish up at the right
 *      point.
 * 
 * Hp70096FunctionKey.c:
 *     -Changed the args and return values for _hp70096FunctionKeyCreate()
 *      and _hp70096FunctionKeyDestroy().
 *     -Added support for the window title and icon name escape sequences.
 * 
 * Hp70096FunctionKey.h:
 *     -Added declarations for _hp70096FunctionKeyCreate() and
 *      _hp70096FunctionKeyDestroy().
 *     -Changed return for _hp70096FunctionKeyUserKeySet().
 * 
 * Hp70096FunctionKeyP.h:
 *     -Added variables to support the widow title and icon name strings.
 * 
 * Revision 1.58  93/05/06  13:09:27  13:09:27  porsche (John Milton)
 * Made the block mode information an opaque type.  Files affected are:
 * 
 *     Hp70096/Hp70096.c
 *         - now calls _Hp70096BlockModeCreate() to create and initialize the
 *           block mode specific information
 * 
 *     Hp70096/Hp70096Data.h
 *         - blockModeInfo is now of type BlockModeInfo
 * 
 *     Hp70096/Hp70096BlockMode.h
 *         - eliminated _Hp70096BlockModeInit() prototype (its now blockModeInit(), a
 *           static function)
 *         - added _Hp70096BlockModeCreate() prototype
 *         - added _Hp70096BlockModeDestroy() prototype
 *         - added typedef for BlockModeInfo
 * 
 *     Hp70096/Hp70096BlockMode.c
 *         - eliminated _Hp70096BlockModeInit() (its now blockModeInit(), a static
 *           function)
 *         - added _Hp70096BlockModeCreate() to allocate and initialize block mode info
 *         - added _Hp70096BlockModeDestroy() to free block mode info
 * 
 *     Hp70096/Hp70096BlockModeP.h
 *         - added prototype for blockmodeInit()
 * 
 * jrm 05/06/93
 * 
 * Revision 1.57  93/05/06  10:32:56  10:32:56  porsche (John Milton)
 * 
 *     Term/TermBuffer.c
 *         - Removed _DtTermPrimBufferInsertCharacter()
 *         - Did some major rework on _DtTermPrimBufferInsert() (mostly made it work),
 *           it also returns the new line length (instead void).
 *           
 *     Term/TermPrimBuffer.h
 *         - Removed _DtTermPrimBufferInsertCharacter() prototype
 *         - updated the _DtTermPrimBufferInsert() prototype
 * 
 * jrm 05/06/93
 * 
 * Revision 1.56  93/05/06  09:22:43  09:22:43  daves (Dave Serisky)
 * Hp70096/Hp70096.c:
 *     -Added resources for the functionKeyExecuteProc,
 *      functionKeyUpdateProc, and functionKeyLast.
 *     -Changed initial setting of remoteMode to True since we don't yet
 *      have NV memory support.
 *     -Added calls to initialize and destroy the user keys.
 * 
 * Hp70096/Hp70096.h:
 *     -Added #defines for the functionKeyExecuteProc,
 *      functionKeyUpdateProc, and functionKeyLast resources.
 * 
 * Hp70096/Hp70096Action.c:
 *     -Changed actions for KeyMenuSelect from _hp70096 to _hpterm.
 *     -Changed function key execute to pass keynumber with first key == 0
 *      instead of == 1.
 * 
 * Hp70096/Hp70096BlockMode.c:
 *     -Filled in code to support user key transmit.
 * 
 * Hp70096/Hp70096Data.h:
 *     -Added a UserKeyInfo struct pointer to the hp70096Data structure.
 * 
 * Hp70096/Hp70096P.h:
 *     -Added variables for FunctionKeyExecuteProc, FunctionKeyUpdateProc,
 *      and functionKeyLast to the Hp70096Part.
 * 
 * Hp70096/Hp70096Parse.c:
 *     -Added support for user keys.
 * 
 * Hp70096/Hp70096ParseTable.c:
 *     -Added table for user keys.
 * 
 * Hp70096/Hp70096ParseTable.h:
 *     -Removed private state table declarations (moved to
 *      Hp70096ParseTableP.h).
 *     -Added entries for function key parser functions.
 * 
 * Hp70096/Imakefile:
 *     -Added Hp70096FunctionKey.[ch].
 * 
 * Hp70096/cursor.c:
 *     -Included new TermPrimParserP.h private header file.
 * 
 * Added new files:
 *     +Hp70096FunctionKey.c:	usr function key functions.
 *     +Hp70096FunctionKey.h:	public header file for above
 *     +Hp70096FunctionKeyP.h:	private header file for above
 *     +Hp70096ParseTableP.h:      private header table for the parser
 * 				table.
 * 
 * Revision 1.55  93/05/06  09:22:28  09:22:28  daves (Dave Serisky)
 * Term.c:
 *     -Changes for new DtTermPrimData typedef and  pendingText changes.
 *     -Changed pendingRead to type pendingText.
 *     -Added _DtTermPrimLoopBackData function.
 * 
 * TermPrim.h:
 *     -Added declaration for _DtTermPrimLoopBackData().
 * 
 * TermPrimData.h:
 *     -Changed from #include of TermParser and TermPendingText private
 *      header files to the public header files.  Since everyone includes
 *      TermPrimData.h, this makes all the other code less dependent on changes
 *      to the private header files.
 *     -Changed declarations for pending read stuff.
 *     -Changed from using pendingTextRec's to pendingText's.
 * 
 * TermPrimP.h:
 *     -Changed the parser_start_state from a StateTable to a StateTable
 *      pointer so we can remove our dependency on the private parser
 *      include files.
 * 
 * TermParseTable.c:
 *     -Made initial state table _termStateStart static, and added a global
 *      pointer to it.
 * 
 * TermPrimParseTable.h:
 *     -Changed declaration for the initial state table.
 * 
 * TermParser.c:
 *     -Changed parser to set the next state before executing the function.
 *      This allows the function key parser functions to bailout at the
 *      appropriate point.
 *     -Added support for workingNumIsDefault Boolean.
 *     -Changed initialization code to dereference the initial state.
 * 
 * TermPrimParser.h:
 *     -Moved #defines for accessing private data to the private header
 *      file.
 * 
 * TermPrimParserP.h:
 *     -Moved in the #defines mentioned above.
 *     -Added workingNumIsDefault Boolean.
 * 
 * TermPendingText.c:
 *     -Changed to support pending read as well as write.
 *     -Changed to a doubly linked list so we can actually remove things
 *      from the end.
 *     -Changed head and tail to be pointers to full head and tail
 *      structures.  This maked life much easier, but costs a few bytes.
 * 
 * TermPrimPendingText.h:
 *     -Changed pendingText structure declarations as well as function
 *      declarations.
 * 
 * TermPrimPendingTextP.h:
 *     -Modified pendingTextChunkRec to have additional info and not use
 *      unsigned ints (we keep having problems with them).  TermRender.c:
 * 
 * Revision 1.54  93/05/06  09:22:23  09:22:23  daves (Dave Serisky)
 * HpTerm/functionKey.c:
 *     -Changed function names from _hp70096* to _hpterm*.
 *     -Changed UserKeyInfo and FunctionKeyClientData names to prevent
 *      collision with those used by _hp70096FunctionKey*.[ch].
 * 
 * HpTerm/functionKeyP.h:
 *     -Same as for functionKey.c.
 * 
 * HpTerm/main.c:
 *     -Added yet another private header file for "debug purposes only!!!!"
 * 
 * Revision 1.53  93/05/04  13:51:20  13:51:20  porsche (John Milton)
 * Moved some of the 70096 specific data out of Term/TermPrimData.h into
 * Hp70096/Hp70096Data.h, then modified all affected files.  This involved
 * changes too numerous to mention.  The files affected by this change are
 * listed below:
 * 
 *     Hp70096/Hp70096.c
 *     Hp70096/Hp70096Action.c
 *     Hp70096/Hp70096BlockMode.c
 *     Hp70096/Hp70096BlockMode.h
 *     Hp70096/Hp70096BlockModeP.h
 *     Hp70096/Hp70096Data.h
 *     Hp70096/Hp70096Parse.c
 *     Hp70096/Hp70096ParseTable.c
 *     Hp70096/Hp70096ParseTable.h
 *     Term/Term.c
 *     Term/TermPrimData.h
 * 
 * jrm 05/04/93
 * 
 * Revision 1.52  93/04/30  17:34:10  17:34:10  porsche (John Milton)
 * Created an opaque data type for blockModeRec in the termdata structure...
 * 
 *     Hp70096/Hp70096BlockMode.c
 *         - modified all relevant functions to access the blockModeRec in
 *           the termdata structure
 *         - modified all relevant functions to use the enumClearPending
 *           data type
 *         - added _blockModeSetCursorSense() to access private blockModeRec
 *           structure
 *         - added _blockModeSetBlockTerminator()
 *         - added _blockModeSetFieldSeparator()
 *         - added _blockModeInit() to initialize the blockModeRec
 *         - _blockModeInitiateXfer() now takes a Widget instead of a DtTermPrimitiveWidget
 *         - cleaned up blockModeInitiateXfer() to make the handshaking more
 *           clear
 * 
 *     Hp70096/Hp70096BlockMode.h
 *         - added prototypes for _blockModeSetCursorSense(), 
 *           _blockModeSetBlockTerminator(), _blockModeSetFieldSeparator(), and
 *           _blockModeInit()
 *         - modified _blockModeInitiateXfer() prototype
 * 
 *     Hp70096/Hp70096BlockModeP.h
 *         - added enumerated type 'enumClearPending'
 *         - added declaration for blockModeRec structure
 *         
 *     Hp70096/Hp70096Parse.c
 *         - _DtTermPrimParseRelCursorPosition(), and _DtTermPrimParseAbsCursorPosition()
 *           now call _blockModeSetCursorSense() to record state of cursor
 *           sense request (relative or absolute)
 * 
 *     Term/Term.c
 *         - eliminated initializeBlockMode()
 *         - now calling _blockModeInit() instead of initializeBlockMode()
 * 
 *     Term/TermPrimData.h
 *         - the blockmode specific stuff is now contained in blockModeInfo,
 *           a blockModeRec
 * 
 * 
 * jrm 4/30/93
 * 
 * Revision 1.51  93/04/30  17:06:56  17:06:56  porsche (John Milton)
 * Progress marches on...
 * 
 *     Hp70096/Hp70096Action.c
 *         - _hp70096ActionDeleteChar() now handles delete with wrap
 * 
 *     Hp70096/Hp70096Function.c
 *         - _hp70096FuncDeleteChar() now handles delete with wrap
 * 
 *     Term/TermBuffer.c
 *         - _DtTermPrimBufferInsert() rearranged some parameters, only pad
 *           up to col - 1 when necessary
 *         - _DtTermPrimBufferDelete() returns the character it just deleted
 *           (this made implementing delete-with-wrap real easy)
 *  
 *     Term/TermPrimBuffer.h
 *         - updated _DtTermPrimBufferInsert(), and _DtTermPrimBufferDelete() prototypes
 * 
 * jrm 4/29/93
 * 
 * Revision 1.50  93/04/30  15:51:00  15:51:00  daves (Dave Serisky)
 * <This is for the Hp70096 directory only>
 * 
 * Hp70096.c:
 *     -Moved to new emulation specific render and enhancement code.
 * 
 * Hp70096.h:
 *     -Added prototype for _Hp70096GetRenderFontIndex().
 * 
 * Hp70096Color.c:
 *     -Moved color code from _term to emulation specific methods in
 *      _hp70096.
 * 
 * Hp70096Color.h:
 *     -Changed function declarations from _term*() functions to _hp70096()
 *      functions.
 * 
 * Hp70096Data.h:
 *     -First functional hp70096 data file.
 * 
 * Hp70096LineData.data:
 *     -Changed the #included file.
 * 
 * Hp70096P.h:
 *     -Added the Hp70096Data structure to the widget's data.
 * 
 * Hp70096Parse.c:
 *     -Changed the font enhancement parser functions from _term*()
 *      functions to the new emulation specific _hp70096() functions.
 * 
 * Imakefile:
 *     -Added Hp70096Enhance.c and HP70096LineData.c.
 *     -Added support to generate the Hp70096LineData.c file from the
 *      Hp70096LineData.data file.
 * 
 * Added the following files:
 *     +Hp70096Enhance.c:		hp70096 specific enhancement code.
 *     +Hp70096Enhance.h:		headers for above.
 *     +Hp70096LineData.h:		declarations for HP line draw glpyh info.
 * 
 * Revision 1.49  93/04/30  15:38:46  15:38:46  daves (Dave Serisky)
 * <this is for the Term directory only>
 * 
 * Imakefile:
 *     -Added TermRenderFont.c TermRenderFontSet.c TermRenderLineDraw.c.
 * 
 * TermCursor.c:
 *     -Modified the cursor code to use the TermClass enhancement proc to
 *      obtain the fg and bg.
 * 
 * TermPrimData.h:
 *     -Removed the font and color info from termData.  Added a
 *      defaultRenderFont.
 * 
 * TermLineDraw.c:
 *     -Modified the linedraw code to support multiple glyph descriptions.
 *      This way we can have an "HP" and "ANSI" linedraw font loaded and in
 *      use within a single application.
 * 
 * TermPrimLineDraw.h:
 *     -Changed the linedraw function declarations to use the new improved
 *      LineDrawFontRec structure.
 * 
 * TermPrimLineFont.h:
 * TermPrimLineFontP.h:
 *     -Pulled stuff out of TermPrimLineFont.h and into the new
 *      TermPrimLineFontP.h.
 * 
 * TermPrimP.h:
 *     -Added some enhancement declarations and prototypes that really
 *      should be in the term buffer code.
 *     -Modified the DtTermPrimitiveClassPart data fields.
 * 
 * TermPrimParser.h:
 * TermPrimParserP.h:
 *     -Made declaration for the StateTable structure pointer public.
 * 
 * TermRender.c:
 *     -Fixed bug that was preventing the render code from clearing the end
 *      of the line if it only had 1 character to clear.
 *     -Yanked out the emulation specific enhancement code to use the
 *      TermClass enhancement proc instead.
 * 
 * TermPrimRender.h:
 *     -Added a front end render function for all the various font types.
 * 
 * Added the following new files:
 *     +TermPrimLineDrawP.h:		line draw private declarations.
 *     +TermPrimLineFontP.h		line font private declarations.
 *     +TermRenderFont.c		XFontStruct render, create, destroy routines.
 *     +TermPrimRenderFont.h		headers for above.
 *     +TermRenderFontSet.c	FontSet render, create, destroy routines.
 *     +TermPrimRenderFontSet.h	headers for above.
 *     +TermRenderLineDraw.c	linedraw render, create, destroy routines.
 *     +TermPrimRenderLineDraw.h	headers for above.
 *     +TermPrimRenderP.h		rendering private header file.
 * 
 * 
 * Revision 1.47  93/04/28  16:12:11  16:12:11  daves (Dave Serisky)
 * -Created Hp70096 widget.
 * -Split up code functionality between Term and Hp70096 widgets.
 * 
 * Revision 1.46  93/04/23  14:38:13  14:38:13  daves (Dave Serisky)
 * HpTerm.c:
 *     -Changed from SoftKeys to FunctionKeys.
 * 
 * TermViewP.h:
 *     -Changed from SoftKeys to FunctionKeys.
 * 
 * Imakefile:
 *     -Added functionKey.[ch].
 * 
 * Term.c:
 *     -Added translations for the user/system/menu keys.
 *     -Blew away the osf* keysyms.
 *     -Changed the code that reads the child pipe to keep from eating up
 *      CPU time.
 * 
 * TermAction.c:
 *     -Added _termActionPostFunctionKeys() function.
 * 
 * TermPrimAction.h:
 *     -Added _termActionPostFunctionKeys() declaration.
 * 
 * main.c:
 *     -Changed from SoftKeys to FunctionKeys.
 *     -Blew away the osf* keysyms.
 * 
 * menu.c:
 *     -Blew away the osf* keysyms.  Menus are now posted via Ctrl<Key>F10
 *      and Ctrl Shift<Key>F10.
 * 
 * Revision 1.45  93/04/22  17:50:45  17:50:45  porsche (John Milton)
 * Work progresses on block mode...
 * 
 *     Term.c
 *         - added initializeBlockMode() to initialize block mode specific
 *           parameters in term data structure
 * 
 *     blockMode.c
 *         - continued progress towards core block mode functionality
 *         - cleaned up completePendingXfer()
 *         - added shortBlockTerminator() to add the correct terminator to
 *           short block transfers
 *         - _termAppendCRLF() returns either CR, or CR/LF depending on
 *           autoLinefeed mode
 *         - finished implementation of handshakeRequired()
 *         - implemented transmitDC2()
 *         - finished implementation of transmitCursorSense()
 *         - implemented pendingXferID() - returns Xfer ID of highest priority
 *           pending transfer request
 *         - implemented _blockModeRecordDC1Rcvd() to record the fact that
 *           a DC1 had been received, and to kick off any pending transfer
 * 
 *     blockMode.h
 *         - cleaned up _blockXferMask, and _blockXferID enumerated types
 *         - added prototypes for _blockModeRecordDC1Rcvd() and 
 *           _blockModeAppendCRLF()
 *         
 *     blockModeP.h
 *         - added enumerated types for _transferType and _handshakeType
 *         - set up handshakeTable for looking up handshake types
 *         - added prototypes for handshakeRequired(), completePendingXfer(),
 *           and shortBlockTerminator()
 * 
 *     line.c
 *         - autoWrapRight, pageMode, inhibitHandshake, inhibitDC2, and
 *           sendEscapeToPrinter weren't being set properly in 
 *           termParseSetModes()
 * 
 *     parser.h
 *         - removed RecordDC1Rcvd() prototype (its now 
 *           _blockModeRecordDC1Rcvd())
 * 
 *     table.c
 *         - added _blockModeRecordDC1Rcvd to parse table
 *         - redid amp_k[] table to elminate need for amp_k_num[]
 * 
 *     termData.h
 *         - added blockTerminator and fieldSeparator definitions to
 *           the termData structure
 * 
 * jrm 4/22/93
 * 
 * Revision 1.44  93/04/22  10:41:13  10:41:13  daves (Dave Serisky)
 * Term.c:
 *     -Changed utmp child to parent communication to be via a pipe.
 * 
 * setUtmp.c:
 *     -Turned on and fixed UT_HOST and UT_ADDR code.
 * 
 * subproc.c:
 *     -Added code to setres[gu]id to the real [gu]id.
 * 
 * termData.h:
 *     -Added pipe and XtInputIds to be used by the child to notify the
 *      parent of /etc/utmp changes.
 * 
 * Revision 1.43  93/04/20  16:58:03  16:58:03  daves (Dave Serisky)
 * Imakefile:
 *     -Added setUtmp.[ch].
 * 
 * Term.c:
 *     -Added an event handler for propertyChangeNotify events and processing
 *      for the utmpId property.
 *     -Added "keymap" and "string" translations.
 *     -Added support for /etc/utmp.
 *     -Added support for SIGWINCH, et.al.
 * 
 * TermAction.c:
 *     -Added "keymap" and "string" actions.
 * 
 * TermPrimAction.h:
 *     -Added headers for the "keymap" and "string" actions.
 * 
 * setPty.c:
 *     -Added a _DtTermPrimPtySetWindowSize() function.
 * 
 * setPty.h:
 *     -Added a proto header for _DtTermPrimPtySetWindowSize() function.
 * 
 * subproc.c:
 *     -Added support for /etc/utmp routines.
 *     -Changed a few functions to pass in a widget to make them more
 *      consitent.
 * 
 * subproc.h:
 *     -Changed a few proto headers to pass in a widget to make them more
 *      consitent.
 * 
 * termData.h:
 *     -Added some /etc/utmp variables.
 * 
 * Revision 1.42  93/04/20  16:57:27  16:57:27  porsche (John Milton)
 * Work progresses on block mode...
 * 
 *     termData.h
 *         - pageBlockMode becomes pageMode
 *         - added escdXfer field (true if transfer was initiated by esc d)
 * 
 *     Term.c
 *         - pageBlockMode becomes pageMode
 * 
 *     line.c
 *         - pageBlockMode becomes pageMode
 *         - cleaned up _DtTermPrimParseSetMode
 *         - removed debug priint statements from _DtTermPrimParseRelCursorPosition
 *           and _DtTermPrimParseAbsCursorPosition
 *     
 * jrm 4/20/93
 * 
 * Revision 1.41  93/04/19  15:35:57  15:35:57  daves (Dave Serisky)
 * Term.c:
 *     -Added translations for the shifted and unshifted Tab and KP_Tab
 *      (numeric tab) keys.
 * 
 * TermAction.c:
 *     -Fixed the long break to make it 2 sec instead of 20 seconds.
 *      Actually, on hp-ux, the duration is a no-op.
 *     -Added actions for the various flavours of tab key.
 * 
 * TermBuffer.c:
 *     -Added code to initialize and resize the tab buffer.
 *     -Added functions to access and set the tab buffer.
 * 
 * TermPrimBuffer.h:
 *     -Added prototypes for the _TermBuffer*Tab*() functions.
 * 
 * TermFunction.c:
 *     -Added the _DtTermPrimFuncTab() and _DtTermPrimFuncTabBack() functions.
 *     -Added the _DtTermPrimFuncTabSet(), _DtTermPrimFuncTabClear(), and
 *      _DtTermPrimFuncTabClearAll() functions.
 *     -Added the _termFuncMargin*() functions (taken from line.c).
 * 
 * TermPrimFunction.h:
 *     -Added prototypes for the _termFunc*Tab*() and _termFuncMargin*()
 *      functions.
 * 
 * line.c:
 *     -Yanked out the tab and margin code.  Replaced them with wrappers to
 *      the associated _termFunc*() functions.
 * 
 * Revision 1.40  93/04/19  11:19:31  11:19:31  daves (Dave Serisky)
 * Term.c:
 *     -Added sticky prev/next cursor resources.
 * 
 * TermPrim.h:
 *     -Added XmNstickyNextCursor, XmNstickyPrevCursor, and XmCStickyCursor
 *      defines.
 * 
 * TermAction.c:
 *     -Turned on _DtTermPrimActionRedrawDisplay().
 * 
 * TermFunction.c:
 *     -Added support for sticky next/prev cursor.
 *     -Added _termFuncBeginningOfLine() and
 *      _termFuncBeginningOfBufferIgnoreXmit() functions.
 *     -Added _DtTermPrimFuncRedrawDisplay().
 * 
 * TermPrimFunction.h:
 *     -Added declarations for _termFuncBeginningOfLine() and
 *      _termFuncBeginningOfBufferIgnoreXmit().
 * 
 * TermPrimP.h:
 *     -Added Boolean variables for sticky next/prev cursor.
 * 
 * line.c:
 *     -Merged shift in/out into a single function.
 *     -Added _DtTermPrimParseFontShift() function.
 *     -Added code to call _DtTermPrimFillScreenGap() before setting
 *      enhancements.
 *     -Changed strap setting code to only set straps for numeric args of 0
 *      or 1.
 *     -Added _DtTermPrimParseCursorLeftMargin() and _DtTermPrimParseHomeUpIgnoreXmit()
 *      functions.
 * 
 * parser.h:
 *     -Modified function prototypes.
 * 
 * parserP.h:
 *     -Added declaration for rparen_num[] table.
 * 
 * table.c:
 *     -Added tables for "<esc>)".
 *     -Merged font shiftin/shiftout into a single function call.
 * 
 * Revision 1.39  93/04/18  17:21:46  17:21:46  porsche (John Milton)
 * Started work on implementing the core block mode transfer functionality:
 * 
 *     blockMode.c
 *     blockMode.h
 *     blockModeP.h
 *         - initial implementations for dealing with block mode transfers
 * 
 *     Imakefile
 *         - added blockMode.c and blockMode.o
 * 
 *     Term.c
 *         - added initializeStraps, initializeModes, and initializeKeyboard,
 *           to provide the correct initiailizations for their corresponding
 *           termData fields, (NOTE: this initialize functions aren't complete
 *           in particular, they don't have any notion of non-volatile RAM 
 *           containing default values.  Work will have to be done to do
 *           the right thing regarding power on reset, soft reset, and hard
 *           reset)
 * 
 *     TermAction.c
 *         - modified invokeAction, and _termActionBreak to accomodate the
 *           new definition for keyboardLocked
 * 
 *     TermFunction.c
 *         - modified _termFuncHardReset, and _termFuncSoftReset to accomodate
 *           the new definition for keyboardLocked
 * 
 *     line.c
 *         - added _DtTermPrimParseSetMode to properly set terminal modes
 *         - modified _DtTermPrimParseEnableKeyboard and _DtTermPrimParseDisableKeyboard
 *           to accomodate the new definition for keyboardLocked
 *         - began implementing _DtTermPrimParseRelCursorPosition and 
 *           _DtTermPrimParseAbsCursorPosition for sensing cursor position (as a test
 *           case for some of the block mode transfer functionality)
 * 
 *     parser.h
 *         - added prototype for _DtTermPrimParseSetMode
 * 
 *     parserP.h
 *     table.c
 *         - simplified existing handling of ^[&k escape sequences
 * 
 *     termData.h
 *         - now including blockMode.h
 *         - macro definition of KEYBOARD_LOCKED tests if keyboard is locked
 *         - modified definition of keyboarLocked, it is now a struct of 
 *           Booleans indicating the possible reasons that the keyboard may
 *           be locked
 *         - added fields for terminal modes
 *         - added fields for dealing with block mode handshaking
 * 
 * jrm
 * 
 * Revision 1.38  93/04/16  15:41:32  15:41:32  daves (Dave Serisky)
 * TermFunction.c
 *     -Fixed clearToEndOfMemory to only refresh the number of lines on the
 *      display (not one more then the number of lines on the display).
 * 
 * TermRender.c
 *     -Changed the code to clip the end columns/rows before setting /
 *      clearing the refresh flags.
 *     -Fixed _DtTermPrimExposeText to correctly calculate the end row / column.
 *      Before, it would overshoot 1 pixel forcing an update of an extra
 *      row/column.
 * 
 * TermScroll.c
 *     -Fixed a sign error in _DtTermPrimScrollText().
 * 
 * cursor.c
 *     -Fixed _hpGetRowOrCol to default to the memory row number instead of
 *      the screen row number.
 * 
 * Revision 1.37  93/04/14  15:49:39  15:49:39  daves (Dave Serisky)
 * Term.c:
 *     -Changed to new Debug functions.
 *     -Added malloc/realloc of scrollRefreshRows when display memory is
 *      created or resized.
 *     -Added a free of scrollRefreshRows to the Destroy code.  Cleaned up
 *      the code to prevent it from freeing stuff in the termData structure
 *      if termData itself has been freed.
 * 
 * TermAction.c:
 *     -Changed to new Debug functions.
 *     -Fixed up the if against the counts for _termActionScroll() (>0
 *      instead of >1).
 *     -Turned on TermActionEndOfBuffer().
 *     -Fixed the transmit functions string for _termActionEndOfBuffer().
 * 
 * TermCursor.c:
 *     -Changed to new Debug functions.
 *     -Changed cursor code to track the current position color and video
 *      enhancements.
 * 
 * TermFunction.c:
 *     -Changed to new Debug functions.
 *     -Reworked function code to support jump scrolling for function
 *      scrolls.
 * 
 * TermRender.c:
 *     -Changed to new Debug functions.
 *     -Reworked render code to support jump scrolling for function
 *      scrolls.
 *     --Fixed up insert/delete line to fix a few edge cases.
 * 
 * TermScroll.c:
 *     -Reworked scrolling code to support jump scrolling for function
 *      scrolls.
 * 
 * TermPrimScroll.h:
 *     -Added/modified function prototypes.
 * 
 * debug.c:
 *     -Added new inline macro debug functions.
 * 
 * debug.h:
 *     -Added new inline macro debug functions.
 * 
 * termData.h:
 *     -Changed a few things from int to short.
 *     -Added scrollRefreshRows.
 *     -Changed the name of some scroll variables to make them more
 *      understandable.
 * 
 * Revision 1.36  93/04/13  11:54:05  11:54:05  porsche (John Milton)
 *     Term.c
 *         - _DtTermPrimStartOrStopPtyOutput() calls XtRemoveInput() only
 *           if tpd->ptyOutputId is non-zero
 * 
 * jrm
 * 
 * Revision 1.35  93/04/12  16:45:13  16:45:13  daves (Dave Serisky)
 * Term.c:
 *     -Added support for a warning dialog that is (currently) used by
 *      memory protect mode.
 * 
 * TermPrim.h:
 *     -Added declaration for the _DtTermPrimWarningDialog() function.
 * 
 * TermAction.c:
 *     -Enabled InsertLine action.
 * 
 * TermFunction.c:
 *     -Modified several functions to turn the cursor off before they
 *      scroll or update the display.
 *     -Added the guts to _termFuncInsertLine().
 * 
 * TermRender.c:
 *     -Added memory protect support to _DtTermPrimFillScreenGap().
 * 
 * termData.h:
 *     -Added variables to support the warning dialog.
 * 
 * Revision 1.34  93/04/08  17:03:07  17:03:07  daves (Dave Serisky)
 * HpTerm.c:
 *     -Added an insert char indicator change function.
 * 
 * TermViewP.h:
 *     -Added an insert char state variable.
 * 
 * Term.c:
 *     -Added a _DtTermPrimInsertCharUpdate() function.
 * 
 * TermPrim.h:
 *     -Added an enum type for TermInsertCharMode.
 *     -Added a variable for insertCharMode to the TermTermCallbackStruct{}.
 *     -Added a prototype for a _DtTermPrimInsertCharUpdate() function.
 * 
 * TermAction.c:
 *     -Moved #include Term[P].h before #include termData.h to clean up
 *      dependencies.
 *     -Changed the count for many actions from 0 to 1.
 *     -Added deleteLine action.
 *     -Added insertChar action.
 * 
 * TermBuffer.c:
 *     -Added _DtTermPrimBufferDeleteLine() function.
 * 
 * TermPrimBuffer.h:
 *     -Added prototype for _DtTermPrimBufferDeleteLine().
 *     -Added missing extern to a few function prototype.
 * 
 * TermCursor.c:
 *     -Moved #include Term[P].h before #include termData.h to clean up
 *      dependencies.
 * 
 * TermFunction.c:
 *     -Moved #include Term[P].h before #include termData.h to clean up
 *      dependencies.
 *     -Filled out _termFuncDeleteLine().
 *     -Changed variable from memoryLockOn Boolean to a memoryLockMode
 *      enum.
 *     -Changed insert char functions to update the flag and status line
 *      indicator.
 * 
 * TermPrimFunction.h:
 *     -Added missing delete/insert line/char function prototype.
 * 
 * TermRender.c:
 *     -Moved #include Term[P].h before #include termData.h to clean up
 *      dependencies.
 * 
 * TermScroll.c:
 *     -Moved #include Term[P].h before #include termData.h to clean up
 *      dependencies.
 *     -Changed from memoryLockOn Boolean to memoryLockMode enum.
 *     -Added _DtTermPrimScrollTextArea() function to do dump scrolls for
 *      insert/delete line.
 * 
 * TermPrimScroll.h:
 *     -Added prototype for _DtTermPrimScrollTextArea().
 * 
 * line.c:
 *     -Moved #include Term[P].h before #include termData.h to clean up
 *      dependencies.
 *     -Changed from memoryLockOn Boolean to memoryLockMode enum.
 * 
 * parser.h:
 *     -Fixed incorrect function prototype.
 * 
 * table.c:
 *     -Fixed incorrect function name.
 * 
 * termData.h:
 *     -Added insertCharMode enum.
 *     -Changed from memoryLockOn Boolean to memoryLockMode enum.
 * 
 * Revision 1.33  93/04/08  11:12:43  11:12:43  daves (Dave Serisky)
 * Term.c, TermCursor.c, TermFunction.c, TermRender.c:
 *     -Changed definition of lastUsedLine to be the number of lines,
 *      rather than the number of the last used line.  This means that if
 *      we have 10 lines, lastUsedLine is 10 (the count) instead of 9 (the
 *      number of the last line).
 * 
 * TermBuffer.c:
 *     -Changed code to not turn on enhancement validation when DEBUG is
 *      defined (which it may (probably?)  be in the final product).
 * 
 * Revision 1.32  93/04/08  08:23:02  08:23:02  daves (Dave Serisky)
 * Imakefile:
 *     -Added TermFunction.[co] to Imakefile.
 * 
 * TermAction.c:
 *     -Added additional actions.
 *     -Changed actions to call _TermFunc*() functions.
 *     -Added support for keyboard lock flag.
 * 
 * line.c:
 *     -Added additional parser functions.
 *     -Changed parser functions to call _TermFunc*() functions.
 * 
 * parser.h:
 *     -Added additional parser function declarations.
 * 
 * table.c:
 *     -Added additional escape sequence tables.
 * 
 * termData.h:
 *     -Added keyboard locked variable.
 * 
 * 
 * Revision 1.30  93/04/06  16:16:44  16:16:44  daves (Dave Serisky)
 * Modified _DtTermPrimInsertText() to only render the text if there was
 * something worth rendering.  Previously, it was rendering the current
 * position for every character in an escape sequence.
 * 
 * Revision 1.29  93/04/06  15:04:49  15:04:49  daves (Dave Serisky)
 * Changed all unsigned short references to short to prevent math errors
 * in coding.
 * 
 * Changed enhancement buffer length from unsigned char to short to allow
 * longer lines and to prevent unsigned math errors in coding.
 * 
 * Fixed calls to _DtTermPrimRefreshText() to use endrow and endcol instead
 * of width and height.
 * 
 * Revision 1.28  93/04/06  08:51:01  08:51:01  daves (Dave Serisky)
 * Term.c:
 *     -Mucked with scrollbar settings to support memory lock.
 * 
 * TermAction.c:
 *     -Modified next/prev page scrolling to only scroll the unlocked
 *      portion of the screen.
 *     -Modified scrolling up to not try to scroll off the memory locked
 *      portion of the screen.
 * 
 * TermBuffer.c:
 *     -Fixed _DtTermPrimBufferInsertLine to cache off of the possibly malloced
 *      holdLines instead of the static (and possibly too small) lineCache.
 *     -Fixed _DtTermPrimBufferMoveLockArea to correctly function.
 * 
 * TermCursor.c:
 *     -Pulled the cursor position and scroll bar update code into a
 *      separate function so that they can be updated by events other than
 *      just turning on the cursor.
 *     -Mucked with scrollbar settings to support memory lock.
 * 
 * TermPrimCursor.h:
 *     -Added public declaration for new _DtTermPrimCursorUpdate() function.
 * 
 * TermPrimP.h:
 *     -Added variables for caching the verticalScrollBar siderSize and
 *      pageIncrement.
 * 
 * TermRender.c:
 *     -Made _DtTermPrimFillScreenGap() an exported public function since
 *      _DtTermPrimParseLF() needs it.
 *     -Removed unnecessary bogus memory lock support from
 *      _DtTermPrimFillScreenGap().
 *     -Fixed right margin non-wrap mode.  It was bouncing back one column
 *      each time.
 * 
 * TermPrimRender.h:
 *     -Added public declaration for newly public _DtTermPrimFillScreenGap().
 * 
 * TermScroll.c:
 *     -Modified scrolling code to support memory lock.
 *     -Added call to _DtTermPrimCursorUpdate() after each scroll to update the
 *      scrollbar and cursor status line.
 * 
 * line.c:
 *     -Modified _DtTermPrimParseLF() and _DtTermPrimParseMemLockOn() to call
 *      _DtTermPrimFillScreenGap() since these functions fill memory through the
 *      current line.
 * 
 * 
 * Revision 1.27  93/04/05  11:56:28  11:56:28  porsche (John Milton)
 * Did some work on the termBuffer:
 * 
 *     TermBuffer.c
 *         - allocating tabs buffer
 *         - added clearEnhancement (a private function that resets all
 *           enhancement information)
 *         - MAJOR work on enhancement specific code for setting, getting,
 *           and propagating (I may even have gotten it right.).
 *         - all termBuffer* functions now validate row, column, or length
 *           parameters on entry
 *         - termBufferInsertCharacter now adjusts line->length (eliminates
 *           need to call termBufferSetLineLength after inserting a character)
 *         - termBufferClearLine parameter renamed col -> newLength
 * 
 *     TermPrimBuffer.h
 *         - termBufferClearLine parameter renamed col -> newLength
 * 
 *     TermPrimBufferP.h
 *         - declaration for clearEnhancement
 *         - added 'wrapped' field to termLineRec to indicate if the line
 *           is wrapped to the next line
 *         - added 'wrapped' field to termLineRec to indicate the column
 *           where user input started (should prove useful for supporting
 *           line modify mode)
 *         - added tabs buffer pointer to TermBufferRec
 * 
 *     TermRender.c
 *         - eliminated redundant calls to _termSetLineLength
 * 
 * jrm
 * 
 * Revision 1.26  93/04/02  15:54:14  15:54:14  daves (Dave Serisky)
 * Term.c:
 *     -Added setting of left/right margins on buffer allocation/resizing.
 *     -Added a _DtTermPrimSendInput() function to support transmit function
 *      mode.
 * 
 * TermPrim.h:
 *     -Add header for _DtTermPrimSendInput().
 * 
 * TermAction.c:
 *     -Added guts to _termActionClearLine() and
 *      _termActionClearToEndOfLine().
 *     -Added transmitFunctions support to _termAction*() functions.
 * 
 * TermRender.c:
 *     -Made renderText public as _DtTermPrimRefreshText().
 * 
 * TermPrimRender.h:
 *     -Added declaration for newly public function _DtTermPrimRefreshText().
 * 
 * line.c:
 *     -Changed parser functions to _DtTermPrimParse*().
 *     -Added parser functions for cursor motion (up/down/left/right).
 * 
 * parser.h:
 *     -Changed parser function declarations to _DtTermPrimParse*().
 *     -Cleaned up function declarations.
 * 
 * table.c:
 *     -Changed parser function declarations to _DtTermPrimParse*().
 *     -Turned on cursor motion (up/down/left/right).
 * 
 * termData.h:
 *     -Added left/right margins.
 *     -Added straps (transmitFunctions, SPOW, pageBlockMode,
 *      inhibitHandshake, inhibitDC2, sendEscapeToPrinter).
 * 
 * Revision 1.25  93/04/02  09:09:01  09:09:01  daves (Dave Serisky)
 * Fixed up translation table to work with those darn OSF keysyms.
 * 
 * Revision 1.24  93/04/01  14:55:19  14:55:19  daves (Dave Serisky)
 * Imakefile:
 *     -Added TermAction.[co].
 * 
 * Term.c:
 *     -Expanded actions and translations.
 * 
 * TermRender.c:
 *     -Added renderReverse GC to be usedin rendering the background of
 *      linedrawing characters.
 * 
 * line.c:
 *     -Modified the video, font, and attribute enhancement setting
 *      functions to pay attention to the returned length and re-render the
 *      text that has had its enhancement(s) changed.
 *     -Added _termSetStrap to implement the end of line wrap inhibit so
 *      that we could run the fill program from the old hpterm test suite.
 * 
 * lineDraw.c:
 *     -Modified the lineDraw render code to fill the area with the
 *      background color instead of simply clearing it.  This was necessary
 *      for rendering text with a different than the default background
 *      color.
 * 
 * lineDraw.h:
 *     -Added the clearGC to the declaration for
 *      _DtTermPrimLineDrawImageString().
 * 
 * parser.h:
 *     -Cleaned up declarations for the <esc>&s table.
 * 
 * parserP.h:
 *     -Cleaned up declarations for the <esc>&s table.
 * 
 * setPty.c:
 *     -Added function to send OS dependent break to pty.  This is what's
 *      called making things modular.
 * 
 * setPty.h:
 *     -Added declaration for pty break function.
 *     -Fixed multiple include protecting #ifdef.  It was the same as that
 *      used by getPty.h
 * 
 * table.c:
 *     -Cleaned up table for <esc>&s.
 * 
 * termData.h:
 *     -Added reverseRenderGC to the termData structure.
 * 
 * Revision 1.23  93/03/31  14:04:27  14:04:27  daves (Dave Serisky)
 * HpTerm.c:
 *     -Added support for marginWidth, marginHeight, and shadowThickness,
 *      et.al.
 *     -#ifdef'ed out support to recolor softkey area.
 *     -Turned recomputeSize off on the status and softkey labels and
 *      buttons.  This was causing everyone's geometry handler to be
 *      invoked each time the cursor was moved and the position in the
 *      softkey area was updated.
 * 
 * Term.c:
 *     -Added support for the verticalScrollBar.
 *     -Set background color to select color and added support for the
 *      "backgroundIsSelect" resource.
 *     -Changed algorithm used to compute the character cell width of
 *      fontSets.  Was using the max_logical_extent width.  Now, the
 *      max_bounds width of the fontSet's font that contains the characters
 *      'A'..'Z' is used.
 *     -Changed the name of what is now cursorRow, cursorColumn,
 *      bufferRows, and bufferRowRatio to make them more descriptive.
 * 
 * TermPrim.h:
 *     -Added support for "backgroundIsSelect".
 * 
 * TermBuffer.c:
 *     -Modified buffer line movement / scroll functions.
 *     -Added support for dangling enhancement.
 *     -Major rework to set / get enhancement.
 * 
 * TermPrimBuffer.h:
 *     -Modified buffer line movement / scroll functions.
 * 
 * TermPrimBufferP.h:
 *     -Added support for dangling enhancements.
 * 
 * TermCursor.c:
 *     -Changed the name of what is now cursorRow, cursorColumn,
 *      bufferRows, and bufferRowRatio to make them more descriptive.
 *     -Implemented support for the verticalScrollBar.
 *     -Fixed cursorMotionCallback to reflect the absolute position instead
 *      of the screen relative position.
 *     -Added support for 700/92 screen memory buffer.
 *     -Added support for caching of the current scrollbar maximum and
 *      value.
 * 
 * TermPrimP.h:
 *     -Added variables to support caching of the current scrollbar maximum
 *      and value.
 *     -Added support for "backgroundIsSelect".
 * 
 * TermRender.c:
 *     -Added support to scroll below the lastUsedRow (as defined by the
 *      700/92).
 *     -Added fillScreenGap() to move around screen memory to fill any
 *      space between the cursor and the lastUsedRow.
 *     -M
 * 
 * Revision 1.22  93/03/23  12:42:37  12:42:37  daves (Dave Serisky)
 * TermRender.c:
 *     -Added support for the "secure" portion of the video support.
 *     -Added support for the "underline" portion of the video support.
 *     -Fixed/cleaned up the remaining video enhancement support.
 *     -Fixed color enhancement support.
 *     -Changed the shortSleep in the debug code to flash the render area
 *      for .1sec.
 * 
 * Revision 1.21  93/03/23  12:37:36  12:37:36  daves (Dave Serisky)
 * Imakefile:
 *     -Added TermColor.[co].
 * 
 * Term.c:
 *     -Pulled out color initialization and stuck it into TermColor.c.
 *     -Added _termColorDestroy to Term destroy function.
 * 
 * debug.c:
 *     -Fixed shortSleep() to set MICROseconds.  It was treating values as
 *      milliseconds.
 * 
 * line.c:
 *     -Added video and color enhancement functions.
 * 
 * parser.h:
 *     -Added video and color enhancement function declarations.
 * 
 * table.c:
 *     -Added (and collapsed hpterm2.0 version) of video enhancement table.
 *     -Added abreviated color enhancement table.
 * 
 * termData.h:
 *     -Pulled out declaration of ColorPair typedef so that it could be
 *      accessed outside the framework of the termData structure.
 * 
 * Revision 1.20  93/03/19  16:28:44  16:28:44  daves (Dave Serisky)
 * HpTerm.c:
 *     -Added initialization for the hpterm.capsLock and hpterm.stop fields
 *      of the hpterm structure.
 * 
 * TermRender.c:
 *     -Changed renderText to take unsigned shorts as X and Y values.
 *      Probably need to change the variables that are passed in to it in
 *      all the function calls as well.
 * 
 * TermScroll.c:
 *     -Added code to clear the lines (text and enhancements) that are
 *      scrolled in.
 * 
 * Revision 1.19  93/03/19  11:02:41  11:02:41  daves (Dave Serisky)
 * Term.c:
 *     -Added code to set the baseFontIndex and altFontIndex after the
 *      render fonts are registered.
 * 
 * TermBuffer.c
 *     -Changed _DtTermPrimBufferGetEnhancement so it is now passed a buffer in
 *      which to return the enhancements.
 *     -Fixed _DtTermPrimBufferGetEnhancement so it no longer returns an
 *      enhancement length of one greater than the actual length and it
 *      will no longer scan past the end of the enhancement buffer length.
 * 
 * TermPrimBuffer.h
 *     -Changed declaration for _DtTermPrimBufferGetEnhancement().
 * 
 * TermRender.c
 *     -Added _DtTermPrimBufferGetEnhancement().
 *     -Modified renderText loop to get enhancement chunks and to use them.
 * 
 * TermPrimRender.h
 *     -Added declaration for _termGetRenderFontIndex().
 * 
 * line.c
 *     -Fix _termShiftIn and _termShiftOut to set the correct font index
 *      (they were reversed).
 * 
 * Revision 1.18  93/03/19  09:04:44  09:04:44  daves (Dave Serisky)
 * termData.h:
 *     - Modified color pair structure to include all the info needed to
 *       allocate colors, change them, and tell if they have been
 *       initialized.
 *     - Added a TermGC structure for all GC's that contains the current
 *       fg, bg, and font (and the GC).
 * 
 * Term.c:
 *     - Changed termBuf* references to TermBuffer*.
 *     - Modified color pair code to initialize all the color pairs.
 *       However, when a new colorpair is used, it is not yet created (only
 *       the base colorpair is), so we will need to fix this before we
 *       start parsing the color escape sequences.
 *     - Modified to work with the new TermGC structure, and to cache the
 *       fg, bg, and font.
 * 
 * TermCursor.c:
 *     - Changed termBuf* references to TermBuffer*.
 *     - Modified to work with the new TermGC structure, and to cache the
 *       fg, bg, and font.
 * 
 * TermRender.c:
 *     - Changed termBuf* references to TermBuffer*.
 *     - Modified to work with the new TermGC structure, and to cache the
 *       fg, bg, and font.
 *     - Modified code to work with the new color pair structures.
 * 
 * TermScroll.c:
 *     - Changed termBuf* references to TermBuffer*.
 *     - Modified to work with the new TermGC structure, and to cache the
 *       fg, bg, and font.
 * 
 * parser.c:
 *     - Changed termBuf* references to TermBuffer*.
 * 
 * Revision 1.17  93/03/18  17:51:18  17:51:18  porsche (John Milton)
 * Did some work on the termBuffer:
 * 
 *     termBuf.c
 *     termBuf.h
 *     termBufP.h
 *         - renamed all files to their TermBuffer equivalents
 *         - revamped the video enhancement bits
 *         - changed some enhancement field names (fewer characters,
 *           more relevant)
 *         - cleaned/modified up _DtTermPrimBufferSetEnhancement()
 *         - added implementations for _DtTermPrimBufferGetEnhancement() and
 *           _DtTermPrimBufferClearLine()
 * 
 *     Imakefile
 *         - renamed termBuf*.* to their TermBuffer equivalents
 * 
 * jrm
 * 
 * Revision 1.16  93/03/17  16:33:21  16:33:21  daves (Dave Serisky)
 * - Added support for "esc@".
 * - Modified Term font support to support fonts that can be any of
 *   fontSets, XFontStruct, or line drawing fonts.  The areas hit are the
 *   registering and initialization of fonts and the rendering of fonts as
 *   well as the switch in the rendering code.
 * - Added support for shiftOut and shiftIn.  The enhancements are getting
 *   set, and we have support to switch between multiple fonts.  Now all we
 *   need are functions to get the enhancements out of the enhancement
 *   buffer.  As part of this, if we are rendering XFontStructs, the fontId
 *   is cached in termData and the GC's font is set anytime the cached
 *   value changes
 * 
 * Revision 1.15  93/03/17  14:58:36  14:58:36  porsche (John Milton)
 * Laid some ground work for writing to the pty:
 * 
 *     pendingTextP.h
 *     pendingText.h
 *     pendingText.c
 *         - these files implement the pending text queue, data to be
 *           written is queued here in a linked list
 * 
 *     Imakefile
 *         - added pendingText.c and pendingText.o
 * 
 *     Term.c
 *         - initializing pendingWrite.head and pendingWrite.tail
 *         - pendingLength becomes pendingReadLength
 *         - tpd->pending becomes tpd->pendingRead
 *         - added implementations of _DtTermPrimPtyWrite(), writePty(), and 
 *           _DtTermPrimStartOrStopPtyOutput()
 *         
 *     TermPrim.h
 *         - added declaration for _DtTermPrimStartOrStopPtyOutput()
 * 
 *     termData.h
 *         - now includes pendingTextP.h
 *         - 'int pendingLength' becomes 'int pendingReadLength'
 *         - 'unsigned char *pending' becomes 'unsigned char *pendingRead'
 *         - added pendingWrite (a linked list of to termData, 
 * 
 * jrm
 * 
 * Revision 1.14  93/03/11  10:36:15  10:36:15  daves (Dave Serisky)
 * - Changed the caps lock and cursor motion callbacks to both go through
 *   a single callback "statusChangeCallback".
 * - Added the stop indicator to the statusChangeCallback in HpTerm.c
 * - Added stop functionality to Term.c
 * - Fixed capslock modifier lookup code in Term.c to correctly malloc
 *   the required data.
 * 
 * Revision 1.13  93/03/11  10:22:37  10:22:37  porsche (John Milton)
 * Created _DtTermPrimBufferPadLine:
 * 
 *     termBuf.h
 *         - prototype for _DtTermPrimBufferPadLine
 *         - externalized some of the existing functions
 * 
 *     termBuf.c
 *         - _DtTermPrimBufferPadLine pads from the end of the line up to (and
 *           including) the current cursor column (if the current cursor 
 *           column is past the current end of line
 * 
 *         - _DtTermPrimBufferCreateBuffer no longer fills newly created lines
 *           with spaces
 * 
 *     TermRender.c
 *         - _DtTermPrimRenderPadLine now pads up to (and including) the current
 *           cursor position
 * 
 * jrm
 * 
 * Revision 1.12  93/03/09  14:17:11  14:17:11  daves (Dave Serisky)
 * - Fixed up the geometry management code in HpTerm.c to correctly resize
 *   the Term Widget (and the others as well).
 * - Term.c now supports a supplied scrollbar.  It will be re-configured
 *   when the Term widget is resized, and the slider moved when the cursor
 *   is moved.
 * - Fixed some problems in the buffer resize code in Term.c.
 * 
 * Revision 1.11  93/03/09  10:30:38  10:30:38  porsche (John Milton)
 * Created _DtTermPrimRenderPadLine:
 * 
 *     TermPrimRender.h
 *     TermRender.c
 *         - _DtTermPrimRenderPadLine pads from the end of the line up to but not
 *           including the current cursor column (if the current cursor column
 *           is past the current end of line
 * 
 *     line.c
 *         - _termTab calls _DtTermPrimRenderPadLine
 * 
 *     TermCursor.c
 * 	- _DtTermPrimCursorMove calls _DtTermPrimRenderPadLine
 * jrm
 * 
 * Revision 1.10  93/03/09  10:05:34  10:05:34  daves (Dave Serisky)
 * - Changed default softkey labels to "f<n>" instead of "F<n>".
 * - Added resources to tie the following to the menubar color
 *   individually:
 *     + softkey buttons
 *     + full softkey area
 *     + status area
 * - Pulled some widget settings out of the XtSetArg's and put them into
 *   the fallback resources so that they can be altered to play with the
 *   appearance of the application.
 * - The caps lock indicator now tracks the actual caps lock setting.
 * - Modified the Term Initialize function to use a local termData pointer
 *   (tpd) instead of going through the widget.  This makes it consistent
 *   with the rest of the code.
 * - Moved a few widget variables from TermPrimP.h to termData.h.  This was
 *   done to support the separation of Widget specific functionality
 *   (get/set resources, etc) in TermPrimP.h and Terminal behavior in
 *   termData.h.
 * - Added support for KBD_LANG as taken from the XmText widget.  This is
 *   only supported when #ifdef __hpux.
 * - Added additional variables to support off screen lines.
 * - Added support for "-sl" command line option, "*saveLines:"  resource,
 *   allocation of offscreen lines, and preserving the ratio of offscreen
 *   to on screen lines upon resizing.
 * - Added callback reasons to all the widget callbacks.
 * - Changed all instances of "caddr_t" to "XtPointer".
 * 
 * Revision 1.9  93/03/05  10:18:24  10:18:24  porsche (John Milton)
 * Some additional work on cursor positioning:
 * 
 *     TermCursor.c
 *         - termMoveCursor pads out lines properly if the cursor is positioned
 *           past the end of the line
 *     cursor.c
 *         - functions which implement HP-specific terminal behavior now have
 *           an _hp prefix (i.e. _termCursUp becomes _hpCursUp)
 *         - eliminated some obsolete functions
 *     cursor.h
 *         - elmintated some obsolete macros
 *         - cleaned up the enumerated types
 *     line.c
 *     parser.c
 *     parser.h
 *     table.c
 *         - functions which implement HP-specific terminal behavior now have
 *           an _hp prefix (i.e. _termInsertLine becomes _hpInsertLine)
 *     termBuf.c
 *         - _DtTermPrimBufferCreateBuffer now fills all newly create lines with
 *           spaces
 * 
 * jrm
 * 
 * Revision 1.8  93/03/05  09:55:50  09:55:50  daves (Dave Serisky)
 * - Modifed Caps lock indicator to update on enter/leave.  Still need to
 *   add a caps lock callback to Term.c to allow it to tell HpTerm.c that
 *   the caps lock has changed when a change is noticed in the key/pointer
 *   modifier.
 * - Checked in the changes that were made to Term.c to support the
 *   scrolling fixes that didn't get checked in the last time.
 * 
 * Revision 1.7  93/03/04  16:23:42  16:23:42  porsche (John Milton)
 * Some debugging of the ^[&a parsing, and the parser in general.
 * 
 *     Term.c
 *         - Added initializer for tw->term.tpd->parserNotInStartState
 *     
 *     TermRender.c
 *         - Applied a little boolean algebra:
 *             !(!tw->term.jumpScroll && tpd->scroll.nojump.scrollPending)
 *           becomes
 *             (tw->term.jumpScroll || !tpd->scroll.nojump.scrollPending)
 * 
 *         - Checking tw->term.tpd->parserNotInStartState before 
 *           calling parse().
 * 
 *         - Setting tw->term.tpd->parserNotInStartState to the return value
 *           of parse().
 * 
 *     cursor.c
 *         - row and col are no longer required in _termGetRowOrColValue
 * 
 *         - resetting WorkingNum and MoveSign rather than calling
 *           _defaultA() (we don't want RowMoveValid and ColMoveValid reset)
 * 
 *     parser.h
 *     parser.c
 *         - parse now returns True or False depending on if the parser
 *           is in the start state
 *         
 *     table.c
 *         - cleaned up the parse tables some more
 * 
 *     termData.h
 *         - added 'parserNotInStartState' to term data structure
 * 
 *     jrm
 * 
 * Revision 1.6  93/03/03  16:33:36  16:33:36  daves (Dave Serisky)
 * Fixed bogus fix to repeated wrap character in non-jump scroll.
 * 
 * Revision 1.5  93/03/03  16:08:41  16:08:41  daves (Dave Serisky)
 * - Added a managed flag in the HpTerm widget for all the widgets that we
 *   need to track the managed state.  This should help to make some of the
 *   code a little easier to weave through.
 * - Added the status line to the HpTerm widget.
 * - Cleaned up the HpTerm widget's geometry manager and changed managed
 *   functions.  Added support for the status line here as well.
 * - Modified the scrolling code to support a "cleanup" mode.  This allows
 *   completion of any partial or pending scroll requests so that we can do
 *   stuff that may break the partial changes.
 * - Fixed non-jump scrolling so that the character that forces a wrap
 *   doesn't get processed twice (a bug in the algorithm).
 * - _DtTermPrimScrollText is no longer a boolean.  It now returns a void since
 *   no one was using the value it returned.
 * - _termCompleteScroll is now _DtTermPrimScrollComplete to follow the naming
 *   style being used.
 * - Added toggles to the menu for the status bar and jump scroll.
 * - Added function to HpTerm.c to query the various widgets in the HpTerm
 *   widget.
 * 
 * Revision 1.4  93/03/03  11:18:30  11:18:30  porsche (John Milton)
 * Beginning to implement cursor positioning escape sequences:
 * Imakefile:
 *     Added target for cursor.c
 * Term.c
 *     Added initializer for tpd->topRow
 * TermCursor.c
 *     Now including termBufP.h
 *     Added _termMoveCursor
 * TermPrimCursor.h
 *     Declaration for _termMoveCursor
 * line.c
 *     renamed some functions i.e. linefeed becomes _termLF
 * parser.c
 *     some functions renamed
 *     accomodated change of parm1, parm2... to an array
 *     accomodated change of parm4 and parm4 (string parameters) to 
 *     an array of string parameters
 *     got rid of 'enterMinNum'
 *     eliminated some obsolete test code
 * parser.h
 *     accomodated new function names
 *     added macros for accessing parser information
 *     eliminated some obsolete states for parsing ^[&a escape sequences
 *     made parm[1234] and array
 *     made parm[56] an array of stringParameter
 * table.c
 *     accomodated new function names
 *     modified parsing of ^[&a escape sequences (added 1 state,
 *         eliminated several others)
 * termData.h
 *     renderFontsNum is now unsigned char
 *     insertX is now unsigned short
 *     insertY is now unsigned short
 *     offsetX is now unsigned short
 *     offsetY is now unsigned short
 *     cellWidth is now unsigned short
 *     cellHeight is now unsigned short
 *     added definition for topRow
 * 
 * Revision 1.3  93/03/01  10:15:23  10:15:23  daves (Dave Serisky)
 * Fixed coredumper in HpTerm widget subprocess termination callback.
 * 
 * Revision 1.2  93/02/26  13:55:26  13:55:26  daves (Dave Serisky)
 * - Added version.c file.
 * - Modified the call_data for the SubprocTermination callback to pass a
 *   structure which includes the pid and the exit status.
 * - Added x/y cursor position to HpTerm widget's softkey label area.
 * - Added cursor motion callback to Term widget.
 * - Added window manager destroy notify callback to main.c.
 * - Added visual bell, backspace, and tab to Term widget render and its
 *   parser.
 * - Added +/-vb command line options.
 * - Changed declaration of debugLevel to make it a "_term*" private
 *   variable.
 * - 
 * 
 * Revision 1.1  93/02/26  13:54:10  13:54:10  daves (Dave Serisky)
 * Initial revision
 * 
 */
