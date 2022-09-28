
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
#pragma ident "@(#)main.C	1.9 10/11/95 01:16:21 SMI"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include "dmx.hh"


/*
 * globals
 */

gid_t       _originalEgid;    // startup effective gid
gid_t       _originalRgid;    // startup real gid

void    enableGroupPrivileges(void *client_data) { (void) setgid(_originalEgid); }
void    disableGroupPrivileges(void *client_data) { (void) setgid(_originalRgid); }

int
main (int argc, char **argv)
{
	// parse command-line options
	int	c;
	extern char *optarg;
	extern int optind;
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
	disableGroupPrivileges((void *)0);	// disable group privileges from here on

	setlocale(LC_ALL, "");

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
		fprintf(stderr, "usage: %s [-p] [-a] [-f <filename>] \n", argv[0]);
		exit (1);
	}

	//for ( ; optind < argc; optind++)
		//(void)printf("%s\n", argv[optind]);
	

	// create DtMail session

	DtMailEnv		dmxenv;
	DtMail::Session		*session;

	DmxMailbox		mbox;

	session = new DtMail::Session (dmxenv, "dtmailpr");


	if (handleError (dmxenv, "new session") == B_TRUE)
		exit (1);

	if (session == NULL)
	{
		fprintf (stderr, "Error opening session...exiting.\n");
		exit (1);
	}

	// Register all callbacks the backend may have to deal with
	session->registerDisableGroupPrivilegesCallback(disableGroupPrivileges, (void *)0);
	session->registerEnableGroupPrivilegesCallback(enableGroupPrivileges, (void *)0);
	
	// initialize typing system (will go away eventually)
	DtDtsLoadDataTypes ();

	// temporary hack, until I'm sure that buffer objects are working
	char buf [BUFSIZ];
	int n = 0;
	char 	*name;
	FILE	*msgFile;

	if (ffile == NULL)
	{
		name = tempnam ("/tmp", "dtmpr");


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
	DtMail::MailBox		*mailbox = NULL;

	// try to construct mbox
	mailbox = session->mailBoxConstruct (
				dmxenv,
				DtMailFileObject,
				name,
				NULL,
				NULL,
				NULL);

	if (handleError (dmxenv, "new DtMail::MailBox") == B_TRUE)
		exit (1);

	mbox.name = new char [strlen (name) +1];
	strcpy (mbox.name, name);

	mbox.mbox = mailbox;
	mbox.loadMessages ();

	int m = 0;

	for (m = 1; m <= mbox.messageCount; m++)
	{
		mbox.msg[m].getFlags ();
		mbox.msg[m].display ();
		if (m < mbox.messageCount) {
			if (pgflag) {
				printf ("");
			} else {
				printf ("\n\n");
			}
		}
	}

	return 0;
}
