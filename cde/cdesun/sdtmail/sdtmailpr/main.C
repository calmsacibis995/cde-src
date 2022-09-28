/*
 *+SNOTICE
 *
 *	$:$
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1994 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)main.C	1.19 05/13/97 20:24:49 SMI"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include <lcl/lcl.h>

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Connection.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/MessageEnvelope.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/DataTypeUtility.hh>
#include <SDtMail/Utility.hh>

#define	MAXIGNOREHEADERS	1024


/*
 * globals
 */

gid_t       _originalEgid;    // startup effective gid
gid_t       _originalRgid;    // startup real gid

void	display_msg(SdmMessage *, int, int *, LCLd *);
SdmBoolean handleError (SdmError &, char *);

static void    enableGroupPrivileges() { (void) setgid(_originalEgid); }
static void    disableGroupPrivileges() { (void) setgid(_originalRgid); }

SdmStringL	*ignoreList;	// List of headers to ignore

// Private function that allows the mid end library to request enabling
// or disabling of the process group privileges

static void GroupPrivilegeActionCallback(void*, SdmBoolean enableFlag)
{
  if (enableFlag) {
    enableGroupPrivileges();
  }
  else {
    disableGroupPrivileges();
  }
}

extern char *optarg;
extern int optind;

int
main (int argc, char **argv)
{
	// parse command-line options
	int	i;
	int	c;
	LCLd	lcld;
	char	*locale;
	char	*ffile = NULL;
	int	aflag = 0, pgflag = 0;
	int	errflag = 0;

	// we have to be set-gid to group "mail" when opening and storing
	// folders.  But we don't want to do everything as group mail.
	// here we record our original gid, and set the effective gid
	// back the the real gid.  We'll set it back when we're dealing
	// with folders...
	//
	_originalEgid = getegid();	// remember effective group ID
	_originalRgid = getgid();	// remember real group ID
	disableGroupPrivileges();	// disable group privileges from here on

	setlocale(LC_ALL, "");

	if ( (locale = setlocale(LC_CTYPE, NULL)) == NULL ) {
	  SdmUtility::LogError(Sdm_True, "sdtmailpr: localization failure - locale is not set correctly.\n");
	  exit(1);
	}

	lcld = lcl_open(locale);

	if ( !lcld ) {
	  SdmUtility::LogError(Sdm_True, "sdtmailpr: localization failure - lcl_open failed.\n");
	  exit(1);
	}

	while ((c = getopt(argc, argv, "f:aph?")) != EOF)
		switch (c)
		{
			case 'p':
				pgflag++;
				//printf ("Print each message on its own page\n");
				break;
			case 'a':
				aflag++;
				//printf ("Strip attachments\n");
				break;
			case 'f':
				ffile = optarg;
				//printf ("Input file is: %s\n", ffile);
				break;
			case '?':
			case 'h':
				errflag++;
				break;
			default:	
				errflag++;
		}

	if (errflag)
	{
		fprintf(stderr, "\n\n\tusage: %s [-p] [-a] [-f <filename>] \n", argv[0]);
		exit (1);
	}

	//for ( ; optind < argc; optind++)
	//	(void)printf("%s\n", argv[optind]);


	// create mail session
	SdmError		mail_error;
	SdmSession		*session;
	SdmMailRc		*mailrc;
	char			*value = NULL;

	// Connect to the back-end
	SdmConnection *mcon = new SdmConnection("sdtmailpr");
	if ( handleError(mail_error, "new connection") == Sdm_True )
		exit(1);

	// Got a connection with the mid end library established.
	mcon->StartUp(mail_error);

	if ( handleError(mail_error, "startup connection") == Sdm_True )
		exit(1);

	// Hook in our private function to allow the mid end library to request enabling
	// or disabling of the process group privileges

	(void) mcon->RegisterGroupPrivilegeActionCallback(mail_error,
							  GroupPrivilegeActionCallback,
							  (void*)0);

	if ( handleError(mail_error, "registering service function") == Sdm_True )
	  exit(1);

	// Open up outgoing session
	mcon->SdmSessionFactory(mail_error, session, Sdm_ST_InSession);

	if ( handleError(mail_error, "new session") == Sdm_True )
		exit(1);

	session->StartUp(mail_error);

	if (handleError (mail_error, "startup session") == Sdm_True)
		exit (1);

	int r_pipe;
	void *r_calldata;

	SdmAttachSession(mail_error, r_pipe, r_calldata, *session);

	if (handleError (mail_error, "attach session") == Sdm_True)
		exit (1);

	mcon->SdmMailRcFactory(mail_error, mailrc);

	if (handleError (mail_error, "mailrc") == Sdm_True)
		exit(1);

	ignoreList = mailrc->GetIgnoreList();

	// temporary hack, until I'm sure that buffer objects are working
	char	buf[BUFSIZ];
	int	n = 0;
	char 	*name;
	FILE	*msgFile;

	if (ffile == NULL)
	{
		name = tempnam ("/tmp", "sdtmpr");

		if ((msgFile = fopen (name, "w+")) == NULL)
		{
			perror ("tmpfile");
			exit (1);
		}

		while ( (n = read (fileno (stdin), buf, BUFSIZ)) > 0)
		{
			if (write (fileno (msgFile), buf, n) != n)
			{
				perror ("write");
				exit (1);
			}
		}

		if (n < 0)
		{
			perror ("read");
		}

	        fclose (msgFile);

	} else {
		name = ffile;
	}

	SdmMessageStore	*mailbox = NULL;
	SdmMessage	*msg;
	SdmToken	token;
	int		att = 0;	// Attachment number

	session->SdmMessageStoreFactory(mail_error, mailbox);

	if ( handleError(mail_error, "new mailbox") == Sdm_True )
		exit(1);

	mailbox->StartUp(mail_error);

	if ( handleError(mail_error, "mailbox startup") == Sdm_True )
		exit(1);

	token.SetValue("servicechannel", "cclient");
	token.SetValue("serviceclass", "messagestore");
	token.SetValue("serviceoption", "messagestorename", name);
	token.SetValue("servicetype", "local");
	token.SetValue("serviceoption", "readonly");
	token.SetValue("serviceoption", "ignoresessionlock", "true");
	token.SetValue("serviceoption", "nosessionlocking", "true");

	SdmBoolean readOnly;
        SdmMessageNumber nmsgs = 0;
	mailbox->Open(mail_error, nmsgs, readOnly, token);

	if ( handleError(mail_error, "mailbox open") == Sdm_True )
		exit(1);

	// Print the messages
	for (i = 1; i <= nmsgs; i++)
	{
		mailbox->SdmMessageFactory(mail_error, msg, i);
		display_msg(msg, 0, &att, &lcld);
		att = 0; // reset the counter
		if ( i < nmsgs )
		{
			if ( pgflag )
				printf ("\n");
			else
				printf ("\n\n");
		}
	}

	mailbox->Close(mail_error);

	if ( handleError(mail_error, "mailbox close") == Sdm_True )
		exit(1);

	mailbox->ShutDown(mail_error);

	if ( handleError(mail_error, "mailbox shutdown") == Sdm_True )
		exit(1);

	return 0;
}

SdmBoolean
inIgnoreHeaders(const char *hdr)
{
	for (int i = 0; i < ignoreList->ElementCount(); i++) {
		if ( strcasecmp((const char *)(*ignoreList)[i], hdr) == 0 )
			return Sdm_True;
	}
return Sdm_False;
}

void
print_segment(LclCharsetSegmentSet *segs)
{
	char	*buf;

	for (int i = 0; i < segs->num; i++) {

		if ( (buf = (char *)malloc(segs->seg[i].size + 1)) == NULL ) {
		  SdmUtility::LogError(Sdm_True, "sdtmailpr: Not enough memory - needed %d bytes.\n", segs->seg[i].size+1);
		  exit(1);
		}

		strncpy(buf, segs->seg[i].segment, segs->seg[i].size);

		buf[segs->seg[i].size] = (char)0;

		fprintf(stdout, "%s\n", buf);

		free(buf);
	}
}

void
display_msg(SdmMessage *msg, int level, int *att, LCLd *lcld)
{
    SdmMessage		*nestedMsg;
    SdmMessageBody	*mbody;
    SdmMessageEnvelope	*env;
    SdmMessageBodyStructure bodyStruct;
    SdmStrStrL		headers;
    int			bodyCount;
    SdmError		error;
    SdmContentBuffer	contents_buf;
    SdmString		contents;
    SdmString		first, second;
    SdmString		type;
    char		*header, *hdrPtr;
    LCTd		lclt;
    LclError		*ret = NULL;
    LclCharsetSegmentSet *segs;
    int			hdrCount;

    error.Reset();

    msg->SdmMessageEnvelopeFactory(error, env);
    if ( !error ) {
 	env->GetHeaders(error, headers);

	// Calculate the size of a buffer to accept the headers
	hdrCount = 0;
	for (int i = 0; i < headers.ElementCount(); i++) {
	    first = headers[i].GetFirstString();
	    if ( !inIgnoreHeaders((const char *)first) ) {
		hdrCount += headers[i].GetFirstString().Length() +
			    headers[i].GetSecondString().Length() + 3;
	    }
	}

	hdrCount++;

	if ( (header = (char *)calloc(1, hdrCount)) == NULL ) {
	  SdmUtility::LogError(Sdm_True, "sdtmailpr: Not enough memory - needed %d bytes.\n", hdrCount);
	  exit(1);
	}

	hdrPtr = header;
	for (i = 0; i < headers.ElementCount(); i++) {
	    first = headers[i].GetFirstString();
	    if ( !inIgnoreHeaders((const char *)first) ) {
		second = headers[i].GetSecondString();
		sprintf(hdrPtr, "%s: %s\n", (const char *)first, (const char *)second);
		hdrPtr += headers[i].GetFirstString().Length() +
			  headers[i].GetSecondString().Length() + 3;
	    }
	}

	lclt = lct_create(*lcld,
		      LctNSourceType, LctNMsgText, header, LctNNone,
		      LctNSourceForm, LctNInComingStreamForm,
		      LctNKeepReference, LctNKeepByReference,
		      NULL);

	if ( !lclt ) {
	  SdmUtility::LogError(Sdm_True, "sdtmailpr: localization failure - lcl_create failed.\n");
	  exit(1);
	}

	ret = lct_getvalues(lclt,
			LctNPrintForm,
			LctNHeaderSegment, &segs,
			NULL);

	if ( ret ) {
	  SdmUtility::LogError(Sdm_True, "sdtmailpr: localization failure - lct_getvalues failed.\n");
	  lcl_destroy_error(ret);
	  exit(1);
	}

	print_segment(segs);

	lct_destroy(lclt);

	free(header);

	header = NULL;
    }

    msg->GetBodyCount(error, bodyCount);

    for (int i = 1; i <= bodyCount; i++) {
	msg->SdmMessageBodyFactory(error, mbody, i);
	mbody->GetStructure(error, bodyStruct);
	if ( bodyStruct.mbs_type == Sdm_MSTYPE_multipart ) {
	    mbody->SdmMessageFactory(error, nestedMsg);
	    display_msg(nestedMsg, (level + 1), att, lcld);
	}
	else {

	    mbody->GetContents(error, contents_buf);
	    contents_buf.GetContents(error, contents);

	    // Print if it's the first body part.  Otherwise, it's an attachment.
	    if ( level <= 1 && i == 1 &&
		 (bodyStruct.mbs_type == Sdm_MSTYPE_none ||
		  bodyStruct.mbs_type == Sdm_MSTYPE_text ||
		  bodyStruct.mbs_type == Sdm_MSTYPE_message) ) {

		lclt = lct_create(*lcld,
		      LctNSourceType, LctNMsgText, LctNNone, (const char *)contents,
		      LctNSourceForm, LctNInComingStreamForm,
		      LctNKeepReference, LctNKeepByReference,
		      NULL);

		if ( !lclt ) {
		  SdmUtility::LogError(Sdm_True, "sdtmailpr: localization failure - lcl_create failed.\n");
		  exit(1);
		}

		ret = NULL;
		ret = lct_getvalues(lclt,
			LctNPrintForm,
			LctNBodySegment, &segs,
			NULL);

		if ( ret ) {
		  SdmUtility::LogError(Sdm_True, "sdtmailpr: localization failure - lct_getvalues failed.\n");
		  lcl_destroy_error(ret);
		  exit(1);
		}

		print_segment(segs);

		lct_destroy(lclt);

		///printf("%s", (const char *)contents);
	    }
	    else {

		(*att)++; // Update the attachment counter

		SdmDataTypeUtility::DetermineCdeDataTypeForMessage(error, type, *mbody, &bodyStruct);

		// Print a short description of the attachment
		printf("\n\n[%d] \"%s\" %s, %d bytes\n\n",
			*att, (const char *)bodyStruct.mbs_attachment_name,
			(const char *)type, contents.Length());
	    }
	}
    }
}

SdmBoolean
handleError (SdmError &dterror, char *msg)
{
  if (dterror) {
    SdmString errorMessage;
    char msgbuf[4096];

    sprintf(msgbuf, "%s\n", dterror.ErrorMessage());
    errorMessage += msgbuf;
    int i = dterror.MinorCount();
    while (i--) {
      sprintf(msgbuf, "%s\n", dterror.ErrorMessage(i));
      errorMessage += msgbuf;
    }
    SdmUtility::LogError(Sdm_True, "sdtmailpr: (%s) %s\n", msg, (const char *)errorMessage);
    dterror.Reset();
    return (Sdm_True);
  }
 
  dterror.Reset();
  return Sdm_False;
}
 
