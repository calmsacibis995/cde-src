/*
 *+SNOTICE
 *
 *  $Revision: 1.1 $
 *
 *  RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *  The information in this document is subject to special
 *  restrictions in a confidential disclosure agreement between
 *  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *  document outside HP, IBM, Sun, USL, SCO, or Univel without
 *  Sun's specific written approval.  This document and all copies
 *  and derivative works thereof must be returned or destroyed at
 *  Sun's request.
 *
 *  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef MAILMSG_H
#define MAILMSG_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MailMsg.h	1.10 09/23/94"
#else
static char *sccs__MAILMSG_H = "@(#)MailMsg.h	1.10 09/23/94";
#endif

#include <nl_types.h>

/*
 * DtMail comment tag is NL_COMMENT (NL = National Language).
 * genmsg will extract comment blocks containing NL_COMMENT.
 */

extern nl_catd DT_catd;   /* Catgets file descriptor */

#define DTMAIL_CAT        "DtMail"
#define NL_SET            1
#define BUTTON_SET        1
#define TITLE_SET         1
#define LABEL_SET         1
#define DIALOG_SET        3
#define MSG_SET           3
#define ERR_SET           2

#ifdef XGETTEXT
#define MAILMSG(msgid, str)   dgettext(NL_SET, msgid, str)
#else
#define MAILMSG(msgid, str)   catgets(DT_catd, NL_SET, msgid, str)
#endif


/*  MailBox.C             msgid  100 - 199
 *  MBOX_*
 */

/*  MsgScrollingList.C    msgid  200 - 299
 *  MSGLIST_*
 */

/*  RoamCmds.C            msgid  300 - 399
 *  ROCMD_*
 */

/*  RoamMenuWindow.C      msgid  400 - 499
 *  ROMENU_*
 */

/*  SendMsgDialog.C       msgid  500 - 599
 *  SEND_*
 */

/*  Undelete.C            msgid  600 - 699
 *  UNDEL_*
 */

#endif  // MAILMSG_H
