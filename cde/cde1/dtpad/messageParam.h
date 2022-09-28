/**********************************<+>*************************************
***************************************************************************
**
**  File:        messageParam.h
**
**  Project:     DT dtpad, a memo maker type editor based on the motif
**               1.1 widget.
**
**  Description: Contains the codes used in communication between the
**		 "client" and "server" dtpad.  These are used in place
**		 of strings to reduce the computational overhead of
**		 parsing the parameters on the receiving (server) end.
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1992.  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
********************************************************************
**  (c) Copyright 1993, 1994 Hewlett-Packard Company
**  (c) Copyright 1993, 1994 International Business Machines Corp.
**  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
**  (c) Copyright 1993, 1994 Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include <Tt/tt_c.h>

/*
 * The codes are consecutive byte values to give the compiler the best
 * shot at optimizing the resulting "switch" statement.
 */

#define NETFILE			(char)'?'
#define IDSTRING		(char)'@'

/* non Text Editor specific options */
#define GEOMETRY		(char)'A'

/* basic options */
#define STATUSLINE		(char)'B'
#define WINDOWWORDWRAP		(char)'C'
#define OVERSTRIKE		(char)'D'
#define SAVEONCLOSE		(char)'E'
#define MISSINGFILEWARNING	(char)'F'
#define NOREADONLYWARNING	(char)'G'
#define NONAMECHANGE		(char)'H'
#define VIEWONLY		(char)'I'
#define WORKSPACELIST		(char)'J'
#define SESSION			(char)'K'

/* client/server control options */
#define BLOCKING		(char)'L'

/* application server support options */
#define	DIRECTORY		(char)'M'
#define SAVEASDIR		(char)'N'
#define NONAMEINTITLE		(char)'O'
#define MAINTITLE		(char)'P'
#define EXITLABEL		(char)'Q'
#define EXITMNEMONIC		(char)'R'
#define HELPVOL			(char)'S'
#define	NOMENUBAR		(char)'T'
#define CONFIRMATIONSTRING	(char)'U'
#define NOEXIT			(char)'V'
#define CLOSEIMMEDIATE		(char)'W'

/* field nos. for DTPAD_OPEN_FILE_MSG and DTPAD_RUN_SESSION_MSG fields
 * (DT_MSG_DATA_? are defined in lib1/DtSvc/DtUtil2/Message.h) */
#define FILENAME_MSG_FIELD	DT_MSG_DATA_1
#define NETFILE_ID_MSG_FIELD	DT_MSG_DATA_3
#define PROCESS_ID_MSG_FIELD	DT_MSG_DATA_5

/* field nos. for DTPAD_DONE message fields */
#define CHANNEL_MSG_FIELD	DT_MSG_DATA_1
#define RETURN_STATUS_MSG_FIELD	DT_MSG_DATA_2
