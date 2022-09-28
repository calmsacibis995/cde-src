$ $XConsortium: msgfac.msg /main/cde1_maint/2 1995/08/18 18:48:45 drk $
$ @(#)76        1.14  com/cmd/msg/msgfac.msg, bos, bos320 5/29/91 10:24:36
 
$ 
$ @OSF_COPYRIGHT@
$ (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$  
$ 
$  
$ HISTORY
$ Motif Release 1.2.5
$ 

$ COMPONENT_NAME: CMDMSG
$
$ FUNCTION: msgfac.msg
$ 
$ ORIGINS: 27
$ 
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (Copyright statements and/or associated legends of other
$ companies whose code appears in any part of this module must
$ be copied here.)




$quote "         define quote character.

$set MS_DSPCAT        Define initial set#

M_CAT_NO_OPEN	"dspcat: 1312-001 Cannot open specified catalog: %s\n"
M_NO_ID	"dspcat: 1312-002 Cannot specify message number with the -g flag."
M_USAGE	"Usage:  dspcat [-g] CatalogName [SetNumber [MessageNumber]]"

$set MS_GENCAT        Redefine set#

M_MSG_0	"Usage: gencat CatalogFile [SourceFile...]"
M_MSG_1	"gencat: 1312-005 Cannot open target file %s.\n"
M_MSG_2	"gencat: 1312-006 The realloc system call failed."
M_MSG_3	"gencat: 1312-007 The following message text is longer than the $len value:\n\t%s\n"
M_MSG_5	"gencat: 1312-008 The set number in the following line is not valid:\n\t%s\n"
M_MSG_7	"gencat: 1312-009 The length of the hex number in the following line is not valid.\n\
It must be either two or four digits.\n\t%s\n"
M_MSG_8	"gencat: 1312-010 Reached end of line before the defined closing quote.\n\t%s\n"
M_MSG_9	"gencat: 1312-011 The following message string is longer than NL_TEXTMAX:\n%s\n"
M_MSG_10	"gencat: 1312-012 Reached end of string before expected.\n\t%s\n"
M_MSG_11	"gencat: 1312-013 Internal error: The file pointer offset is not correct."
M_MSG_12	"gencat: 1312-014 Internal error: The file pointer offset is not correct."
M_MSG_13	"gencat: 1312-015 Cannot load the catalog file %s.\n"
M_MSG_14	"gencat: 1312-016 Cannot read the existing catalog file %s.\n"
M_MSG_15	"gencat: 1312-017 There is not enough memory available now."
M_MSG_18	"gencat: 1312-020 The following set uses a symbolic identifier:\n\t%s\n"
M_MSG_19	"gencat: 1312-021 The following message uses a symbolic identifier:\n\t%s\n"
M_NOMSG	"gencat: 1312-019 There is no message defined in a source file."
M_ORDER	"gencat: 1312-022 Set or message numbers are not in ascending sequence after:\n\
\tMessage:  %d,  Set: %d\n\
\t%s"

$set MS_MKCATDEFS        Redefine set#

MKCATUSAGE	"Usage: %s SymbolName SourceFile[...SourceFile] [-h]\n"
MNAMTOOLONG	"mkcatdefs: 1312-025 The message catalog name is too long.\n"
MKCATOPN	"mkcatdefs: 1312-026 Cannot open %s.\n"
INVTAG	"mkcatdefs: 1312-027 The symbolic message identifier is not valid:\n\
\t%s\n"
INVLDSET	"mkcatdefs: 1312-028 A set number is not in ascending sequence.\n"
ZEROINV	"mkcatdefs: 1312-029 %s is not a valid set identifier.\n"
MULTOPN	"mkcatdefs: 1312-030 The message identifier %s is used more than once.\n"
MULTNR	"mkcatdefs: 1312-031 The message id %s has already been assigned to identifier.\n"
NOTASC	"mkcatdefs: 1312-032 The message numbers are not in ascending sequence.\n"
READERRS	"mkcatdefs: 1312-033 There were read errors in file %s.\n"
WRITERRS	"mkcatdefs: 1312-034 There were write errors in file %s.\n"
ERRFND	"mkcatdefs: 1312-035 Errors found: The %s header file is not created.\n"
HCREAT	"mkcatdefs: %s is created.\n"
NOSYMB	"mkcatdefs: 1312-036 No symbolic identifiers; %s is not created.\n"
NOHDR	"mkcatdefs: The %s header file is not created.\n"
IMBCHD	"mkcatdefs: 1312-037 Catname contains invalid International Language character.\n"
IMBCTX	"mkcatdefs: 1312-038 SourceFile contains invalid International Language character in the\
 following text:\n\t%s\n"

$set MS_DSPMSG        Define initial set#

M_DSPMSG	"Usage: dspmsg Catalogname [-s SetNumber] MessageNumber ['default' Arguments[Agrument...]]"
M_REORDER	"\nNone or all arguments must use %n$ format"
M_INVRE		"\n% missing from %n$ format"
M_REINDEX	"\nInvalid argument index"

$set MS_RUNCAT        Define initial set#

M_RUNCAT	"Usage: runcat CatalogName SourceFile [CatalogFile]"
