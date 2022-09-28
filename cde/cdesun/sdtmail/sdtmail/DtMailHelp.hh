/*
 *+SNOTICE
 *
 *      $Revision: 1.1 $
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */
 
#ifndef DTMAILHELP_HH
#define DTMAILHELP_HH
 
#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DtMailHelp.hh	1.8 08 Dec 1994"
#endif

// Help Ids for dtmail.

#define DTMAILWINDOWID  "_HOMETOPIC"

// RoamMenuWindow
#define DTMAILWINDOWMAILBOXMENU "DTMAILVIEWMAINWINDOWMENUBARFILE"
#define DTMAILWINDOWMESSAGEMENU "DTMAILVIEWMAINWINDOWMENUBARMESSAGE"
#define DTMAILWINDOWEDITMENU "DTMAILVIEWMAINWINDOWMENUBAREDIT"
#define DTMAILWINDOWATTACHMENTSMENU "DTMAILVIEWMAINWINDOWMENUBARATTACH"
#define DTMAILWINDOWVIEWMENU "DTMAILVIEWMAINWINDOWMENUBARVIEW"
#define DTMAILWINDOWCOMPOSEMENU "DTMAILVIEWMAINWINDOWMENUBARCOMPOSE"
#define DTMAILWINDOWMOVEMENU "DTMAILVIEWMAINWINDOWMENUBARMOVE" 
#define DTMAILWINDOWROWOFLABELSID "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWOFLABELS" 
#define DTMAILMSGLISTID "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2MESSAGELISTSWMESSAGELIST"
#define DTMAILNEXTBTNID "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWCOLUMNNEXT"
#define DTMAILPREVBTNID "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWCOLUMNPREVIOUS"
#define DTMAILDELBTNID "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWCOLUMNDELETE"
#define DTMAILPRINTBTNID "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWCOLUMNPRINT"
#define DTMAILREPLYBTNID "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWCOLUMNREPLY"
#define APP_MENU_ID "onApplicationMenu"
#define VER_MENU_ID "_copyright"

// SendMsgDialog
#define DTMAILCOMPOSEWINDOW "DTMAILCOMPOSEWINDOW"
#define DTMAILCOMPOSEFILEMENU "DTMAILCOMPOSEWINDOWMENUBARFILE"
#define DTMAILCOMPOSEEDITMENU "DTMAILCOMPOSEWINDOWMENUBAREDIT"
#define DTMAILCOMPOSEATTACHMENU "DTMAILCOMPOSEWINDOWMENUBARATTACH"
#define DTMAILCOMPOSEFORMATMENU "DTMAILCOMPOSEWINDOWMENUBARFORMAT"

// Find Message Dialog
#define DTMAILFINDDIALOG "DTMAILVIEWMAINWINDOWMESSAGEFIND"
#define DTMAILHELPSAVEASVIEW "DTMAILHELPSAVEASVIEW"

// Views Dialog
#define DTMAILHELPVIEWS "DTMAILHELPVIEWS"

// Other Mailboxes Dialog
#define DTMAILOTHERMAILBOXESDIALOG "DTMAILVIEWMAINWINDOWMOVEMAILBOX"
#define DTMAILOTHERMAILBOXESCOPYBUTTON "DTMAILVIEWMAINWINDOWWORK-AREAPANEDWFORM2ROWCOLUMNMOVECOPY"

// Undelete Messages From List Dialog
#define DTMAILUNDELETEFROMLISTDIALOG "DTMAILVIEWMAINWINDOWMESSAGEUNDELETELIST"

// Rename Attachment Dialog
#define DTMAILRENAMEATTACHMENTDIALOG "dtmailViewmainWindowAttachRename"

// Rename Mailbox Dialog
#define DTMAILRENAMEMAILBOXDIALOG "RenameAMailbox"

// Mailer Options Dialogs
#define DTMAILOPTIONSHEADERLISTDIALOG "DTMAILMAILBOXOPTIONSMESSAGEHEADERLIST"
#define DTMAILOPTIONSVIEWDIALOG "DTMAILMAILBOXOPTIONSMESSAGEVIEW"
#define DTMAILOPTIONSCOMPOSEDIALOG "DTMAILMAILBOXOPTIONSCOMPOSEWINDOW"
#define DTMAILOPTIONSMESSAGEFILINGDIALOG "DTMAILMAILBOXOPTIONSMESSAGEFILING"
#define DTMAILOPTIONSVACATIONDIALOG "DTMAILMAILBOXOPTIONSVACATION"
#define DTMAILOPTIONSTEMPLATESDIALOG "DTMAILMAILBOXOPTIONSTEMPLATES"
#define DTMAILOPTIONSALIASDIALOG "DTMAILMAILBOXOPTIONSALIASES"
#define DTMAILOPTIONSADVANCEDDIALOG "DTMAILMAILBOXOPTIONSADVANCED" 

// These helpIds are for the help button in dialogs.  
#define DTMAILHELPCANTINITTOOLTALK "DTMAILHELPCANTINITTOOLTALK"
#define DTMAILHELPTAKELOCK "DTMAILHELPTAKELOCK"
#define DTMAILHELPBADGROUPID "DTMAILHELPBADGROUPID"
#define DTMAILHELPUNKNOWNSTATE "DTMAILHELPUNKNOWNSTATE"
#define DTMAILHELPFATALERROR "DTMAILHELPFATALERROR"
#define DTMAILHELPERROR "DTMAILHELPERROR"
#define DTMAILDISMISSOPTIONS "DTMAILDISMISSOPTIONS"
#define DTMAILHELPCLOSECOMPOSEWINDOW "DTMAILHELPCLOSECOMPOSEWINDOW"
#define DTMAILHELPPENDINGACTIONS "DTMAILHELPSENDLOSEATTACH"
#define DTMAILHELPSELECTONEATTACH "DTMAILHELPSELECTONEATTACH"
#define DTMAILHELPBADADDRESS "DTMAILHELPUNKNOWNUSER"
#define DTMAILHELPNOMEMORY "DTMAILHELPNOMEMORY"
#define DTMAILHELPTRANSPORTFAILED "DTMAILHELPNOSEND"
#define DTMAILHELPNOWRITEVACATION "DTMAILHELPNOWRITEVACATION"
#define DTMAILHELPEXISTSVACATION "DTMAILHELPEXISTSVACATION"
#define DTMAILHELPNOCOMPOSE "DTMAILHELPNOCOMPOSE"
#define DTMAILHELPNOTEMPLATE "DTMAILHELPNOTEMPLATE"
#define DTMAILHELPCORRUPTTEMPLATE "DTMAILHELPCORRUPTTEMPLATE"
#define DTMAILHELPNOMEMTEMPLATE "DTMAILHELPNOMEMTEMPLATE"
#define DTMAILHELPNOVIEW "DTMAILHELPNOVIEW"
#define DTMAILHELPDIRECTORYONLY "DTMAILHELPDIRECTORYONLY"
#define DTMAILHELPSELECTATTACH "DTMAILHELPSELECTATTACH"
#define DTMAILHELPDESTROYMARKMSG "DTMAILHELPDESTROYMARKMSG"
#define DTMAILHELPNOOPEN "DTMAILHELPNOOPEN"
#define DTMAILHELPNOALLOCMEM "DTMAILHELPNOALLOCMEM"
#define DTMAILHELPALREADYEXISTS "DTMAILHELPALREADYEXISTS"
#define DTMAILHELPMSGALREADYEXISTS "DTMAILHELPMSGALREADYEXISTS"
#define DTMAILHELPNOREPLACE "DTMAILHELPNOREPLACE"
#define DTMAILHELPNOCREATE "DTMAILHELPNOCREATE"
#define DTMAILHELPNOOVERWRITE "DTMAILHELPNOOVERWRITE"
#define DTMAILHELPNEWMBOXOVERWRITE "DTMAILHELPNEWMBOXOVERWRITE"
#define DTMAILHELPNOWRITE "DTMAILHELPNOWRITE"
#define DTMAILHELPNOLOADVACATION "DTMAILHELPNOLOADVACATION"
#define DTMAILHELPCORRUPTVACATION "DTMAILHELPCORRUPTVACATION"
#define DTMAILHELPNEEDADDRESSEE	  "DTMAILHELPNEEDADDRESSEE"
#define DTMAILHELPEXECUTEOK	"DTMAILHELPEXECUTEOK"
#define DTMAILHELPOPENREADONLY "DTMAILHELPOPENREADONLY"
#define DTMAILHELPOPENREADWRITEOVERRIDE "DTMAILHELPOPENREADWRITEOVERRIDE"
#define DTMAILHELPREADONLYMAILBOX "DTMAILHELPREADONLYMAILBOX"
#define DTMAILHELPMAILBOXCONNECTIONREADONLY "DTMAILHELPMAILBOXCONNECTIONREADONLY"
#define DTMAILHELPMSGSIZELIMIT "DTMAILHELPMSGSIZELIMIT"
#define DTMAILHELPNOFILENAME "DTMAILHELPNOFILENAME" 
#define DTMAILHELPLOGDEFAULT "DTMAILHELPLOGDEFAULT"
#define DTMAILHELPTEMPLATEEMPTY "DTMAILHELPTEMPLATEEMPTY"
#define DTMAILHELPTEMPLATEDELETE "DTMAILHELPTEMPLATEDELETE"
#define DTMAILHELPTEMPLATEERRORRENAME "DTMAILHELPTEMPLATEERRORRENAME"
#define DTMAILHELPTEMPLATEERRORREMOVE "DTMAILHELPTEMPLATEERRORREMOVE"
#define DTMAILHELPTEMPLATEDIALOGHELPBUTTON "DTMAILHELPTEMPLATEDIALOGHELPBUTTON"
#define DTMAILHELPDRAFTDIALOGHELPBUTTON "DTMAILHELPDRAFTDIALOGHELPBUTTON"
#define DTMAILHELPTEMPLATENOTEXT "DTMAILHELPTEMPLATENOTEXT"
#define DTMAILHELPRULES "DTMAILHELPRULES"
#define DTMAILHELPNORULENAME "DTMAILHELPNORULENAME"
#define DTMAILHELPNORULES "DTMAILHELPNORULES"
#define DTMAILHELPUNSUPPORTRULE "DTMAILHELPUNSUPPORTRULE"
#define DTMAILHELPNOMAILFILE "DTMAILHELPNOMAILFILE"
#define DTMAILHELPNODRAFT "DTMAILHELPNODRAFT"
#define DTMAILHELPCORRUPTDRAFT "DTMAILHELPCORRUPTDRAFT"
#define DTMAILHELPNOMEMDRAFT "DTMAILHELPNOMEMDRAFT"
#define DTMAILHELPDRAFTEMPTY "DTMAILHELPDRAFTEMPTY"
#define DTMAILHELPNOFOLDER "DTMAILHELPNOFOLDER"
#define DTMAILHELPSENDDELETEDRAFT "DTMAILHELPSENDDELETEDRAFT"
#define DTMAILHELPDRAFTREMOVE "DTMAILHELPDRAFTREMOVE"
#define DTMAILHELPTEMPLATERENAMEFAILED "DTMAILHELPTEMPLATERENAMEFAILED"
#define DTMAILHELPTEMPLATERENAME "DTMAILHELPTEMPLATERENAME"
#define DTMAILHELPVACATIONENDDATEBEFORESTART "DTMAILHELPVACATIONENDDATEBEFORESTART"
#define DTMAILHELPVACATIONINVALIDSTARTDATE "DTMAILHELPVACATIONINVALIDSTARTDATE"
#define DTMAILHELPVACATIONSTARTDATEPASSED "DTMAILHELPVACATIONSTARTDATEPASSED"
#define DTMAILHELPVACATIONINVALIDENDDATE "DTMAILHELPVACATIONINVALIDENDDATE"
#define DTMAILHELPVACATIONSETUPERROR "DTMAILHELPVACATIONSETUPERROR"
#define DTMAILHELPVACATIONTURNONBUTTON "DTMAILHELPVACATIONTURNONBUTTON"
#define DTMAILHELPVACATIONNOSTARTDATE "DTMAILHELPVACATIONNOSTARTDATE"
#define DTMAILHELPVACATIONHOSTCHANGED "DTMAILHELPVACATIONHOSTCHANGED"
#define DTMAILHELPPASSWD "DTMAILHELPPASSWD"
#define DTMAILHELPAUTOFILING "DTMAILHELPAUTOFILING"
#define DTMAILHELPAUTOFILINGCHECK "DTMAILHELPAUTOFILINGCHECK"
#define DTMAILHELPAUTOFILINGOPEN "DTMAILHELPAUTOFILINGOPEN"
#define DTMAILHELPSTARTUPMAILBOX "DTMAILHELPSTARTUPMAILBOX"
#define DTMAILHELPLOGIN "DTMAILHELPLOGIN"
#define DTMAILHELPCUSTOMHEADERSNOSPACES "DTMAILHELPCUSTOMHEADERSNOSPACES"
#define DTMAILHELPALIASESNOSPACES "DTMAILHELPALIASESNOSPACES"
#define DTMAILHELPIGNORELISTNOSPACES "DTMAILHELPIGNORELISTNOSPACES"
#define DTMAILHELPALIASESDUPLICATE "DTMAILHELPALIASESDUPLICATE"
#define DTMAILHELPALIASESINVALIDADDRESS "DTMAILHELPALIASESINVALIDADDRESS"
#define DTMAILHELPALIASESINVALIDALIAS "DTMAILHELPALIASESINVALIDALIAS"
#define DTMAILHELPALIASESINVALIDALIST "DTMAILHELPALIASESINVALIDALIST"
#define DTMAILHELPIMAPERROR "DTMAILHELPIMAPERROR"
#define DTMAILHELPIMAPSERVER "DTMAILHELPIMAPSERVER"
#define DTMAILHELPSIGNATUREFILE "DTMAILHELPSIGNATUREFILE"
#define DTMAILHELPMOVEMENUNAME "DTMAILHELPMOVEMENUNAME"
#define DTMAILHELPRESTARTMAILER "DTMAILHELPRESTARTMAILER"
#define DTMAILHELPDESTROYDELMSGS "DTMAILHELPDESTROYDELMSGS"
#define DTMAILHELPSAVEMESSAGESMIME "DTMAILHELPSAVEMESSAGESMIME"
#define DTMAILHELPCANNOTDISPLAYMSG "DTMAILHELPCANNOTDISPLAYMSG"
#define DTMAILHELPCANNOTDELETEMSGS "DTMAILHELPCANNOTDELETEMSGS"
#define DTMAILHELPCANNOTUNDELETEMSGS "DTMAILHELPCANNOTUNDELETEMSGS"
#define DTMAILHELPCANNOTUNDELETELASTDELMSG "DTMAILHELPCANNOTUNDELETELASTDELMSG"
#define DTMAILHELPCANNOTDISPLAYDELMSGS "DTMAILHELPCANNOTDISPLAYDELMSGS"
#define DTMAILHELPCANNOTUPDATEMSGHDRLIST "DTMAILHELPCANNOTUPDATEMSGHDRLIST"
#define DTMAILHELPCANNOTDESTROYDELMSGS "DTMAILHELPCANNOTDESTROYDELMSGS"
#define DTMAILHELPCANNOTSAVEMAILBOX "DTMAILHELPCANNOTSAVEMAILBOX"
#define DTMAILHELPCANNOTUPDATEDELMSGHDRLIST "DTMAILHELPCANNOTUPDATEDELMSGHDRLIST"
#define DTMAILHELPCANNOTACCESSMESSAGE "DTMAILHELPCANNOTACCESSMESSAGE"
#define DTMAILHELPCANNOTCHECKNEWMAIL "DTMAILHELPCANNOTCHECKNEWMAIL"
#define DTMAILHELPLOSTEXCLUSIVEACCESSTOMAILBOX "DTMAILHELPLOSTEXCLUSIVEACCESSTOMAILBOX"
#define DTMAILHELPLOSTIMAPSERVERCONNECTION "DTMAILHELPLOSTIMAPSERVERCONNECTION"
#define DTMAILHELPCANNOTUPDATEVIEWSTATS "DTMAILHELPCANNOTUPDATEVIEWSTATS"
#define DTMAILHELPINVALIDVIEW "DTMAILHELPINVALIDVIEW"
#define DTMAILHELPCANNOTOPENMAILBOX "DTMAILHELPCANNOTOPENMAILBOX"
#define DTMAILHELPLOSTCCLIENTLOCKONMAILBOX "DTMAILHELPLOSTCCLIENTLOCKONMAILBOX"
#define DTMAILHELPCHECKNEWMAILFAILSFROMNULLS "DTMAILHELPCHECKNEWMAILFAILSFROMNULLS"
#define DTMAILHELPOPENFAILSFROMNULLS "DTMAILHELPOPENFAILSFROMNULLS"
#define DTMAILHELPFILENOTEXIST "DTMAILHELPFILENOTEXIST"
#define DTMAILHELPCANNOTUPDATEMAILRC "DTMAILHELPCANNOTUPDATEMAILRC"
#define DTMAILHELPNOSAVEATT "DTMAILNOSAVEATTACHMENT"
#define DTMAILHELPMBOXRENAME "DTMAILHELPMBOXRENAME"
#define DTMAILHELPRENAMEEXISTS "DTMAILHELPRENAMEEXISTS"
#define DTMAILHELPRENAMEINBOX "DTMAILHELPRENAMEINBOX"
#define DTMAILHELPRENAMEERROR "DTMAILHELPRENAMEERROR"
#define DTMAILHELPMBOXDELETE "DTMAILHELPMBOXDELETE"
#define DTMAILHELPINBOXDELETE "DTMAILHELPINBOXDELETE"
#define DTMAILHELPMBOXDELETEERROR "DTMAILHELPMBOXDELETEERROR"

// Option defines for menubar help access
#define HELP_MAILER_TASKS "Tasks"
#define HELP_MAILER_REFERENCE "Reference"
#define HELP_MAILER_MESSAGES "Messages"
 
char *getHelpId(Widget);

// Help id for all the dialogs and menus related to DtEditor widget,
// like Compose window Edit->Find/Change, Format->Settings..., etc.
// These id were defined in dtpad. 
 
/************************************************************************
 * Help location ids for the dialog posted by [Format] menu [Settings...]
 * (this dialog is controlled by the DtEditor widget)
 ************************************************************************/
#define FORMAT_SETTINGS_HELP                    "formatDialog"

/* -----> fields/buttons within the Format Settings dialog */
#define FORMAT_LEFT_MARGIN_HELP                 "format-leftmargin"
#define FORMAT_RIGHT_MARGIN_HELP                "format-rightmargin"
#define FORMAT_ALIGNMENT_HELP                   "format-alignment"

/************************************************************************
 * Help location ids for the dialog posted by [Edit] menu [Find/Change...]
 * (this dialog is controlled by the DtEditor widget)
 ************************************************************************/
#define FINDCHANGE_HELP                         "findchangeDialog"

/* -----> fields/buttons within the Find/Change dialog */
#define FINDCHANGE_FIND_HELP                    "fc-find"
#define FINDCHANGE_CHANGETO_HELP                "fc-changeto"

/************************************************************************
 * Help location ids for the dialog posted by [Edit] menu [Find/Change...]
 * (this dialog is controlled by the DtEditor widget)
 ************************************************************************/
#define SPELL_HELP                              "spellDialog"

/* -----> fields/buttons within the Check Spelling dialog */
#define SPELL_MISSPELLED_WORDS_HELP             "sp-spelllist"
#define SPELL_CHANGETO_HELP                     "sp-changeto"

void HelpCB(Widget, XtPointer, XtPointer);
extern void DisplayMain(Widget, char *, char *);
extern void DisplayVersion(Widget, char *, char *);
static void CloseMainCB(Widget, XtPointer, XtPointer);
 
#endif
