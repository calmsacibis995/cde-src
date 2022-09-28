#pragma ident "@(#)submit.c	1.6 96/05/06 Sun Microsystems, Inc."

/*******************************************************************************
**
**  submit.c
**
**  $XConsortium: submit.c /main/cde1_maint/3 1995/10/04 08:44:16 barstow $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


static char **
arpaPhrase(const char * name)
{
    register char c;
    register const char *cp;
    char * cp2;
    int gotlt, lastsp, didq;
    int nesting;
    const char * last_comma = name;
    int biggest = 0;
    int n_addrs = 1;
    int cur_addr;
    int distance;
    char ** addrs;
    const char * comma;
    const char * start;
    
    if (name == (char *) 0) {
	return(NULL);
    }

    /* We need to figure out what is the biggest possible address.
       This will be the maximum distance between commas.
       */
    for (comma = name; *comma; comma++) {
	if (*comma == ',') {
	    n_addrs += 1;
	    distance = comma - last_comma;
	    biggest = biggest < distance ? distance : biggest;
	    last_comma = comma;
	}
    }
    distance = comma - last_comma;
    biggest = biggest < distance ? distance : biggest;
    biggest += 2; /* Just in case. */

    cur_addr = 0;
    addrs = (char **)malloc((n_addrs + 1) * sizeof(char *));

    cp2 = (char *)malloc(biggest);
    addrs[cur_addr++] = cp2;

    gotlt = 0;
    lastsp = 0;
    start = name;
    for (cp = name; (c = *cp++) != 0;) {
	switch (c) {
	  case '(':
	    /*
	      Start of a comment, ignore it.
	      */
	    nesting = 1;
	    while ((c = *cp) != 0) {
		cp++;
		switch(c) {
		  case '\\':
		    if (*cp == 0) goto outcm;
		    cp++;
		    break;
		  case '(':
		    nesting++;
		    break;
		  case ')':
		    --nesting;
		    break;
		}
		if (nesting <= 0) break;
	    }
	  outcm:
	    lastsp = 0;
	    break;
	    
	  case '"':
	    /*
	      Start a quoted string.
	      Copy it in its entirety.
	      */
	    didq = 0;
	    while ((c = *cp) != 0) {
		cp++;
		switch (c) {
		  case '\\':
		    if ((c = *cp) == 0) goto outqs;
		    cp++;
		    break;
		  case '"':
		    goto outqs;
		}
		if (gotlt == 0 || gotlt == '<') {
		    if (lastsp) {
			lastsp = 0;
			*cp2++ = ' ';
		    }
		    if (!didq) {
			*cp2++ = '"';
			didq++;
		    }
		    *cp2++ = c;
		}
	    }
	  outqs:
	    if (didq)
		*cp2++ = '"';
	    lastsp = 0;
	    break;
	    
	  case ' ':
	  case '\t':
	  case '\n':
	      done:
	    *cp2 = 0;

	    cp2 = (char *)malloc(biggest);
	    addrs[cur_addr++] = cp2;

	    lastsp = 1;
	    break;
	    
	  case ',':
	    *cp2++ = c;
	    if (gotlt != '<') {
		gotlt = 0;
		goto done;
	    }
	    break;
	    
	  case '<':
	    cp2 = addrs[cur_addr - 1];
	    gotlt = c;
	    lastsp = 0;
	    break;
	    
	  case '>':
	    if (gotlt == '<') {
		gotlt = c;
		break;
	    }
	    
	    /* FALLTHROUGH . . . */
	    
	  default:
	    if (gotlt == 0 || gotlt == '<') {
		if (lastsp) {
		    lastsp = 0;
		    *cp2++ = ' ';
		}
		*cp2++ = c;
	    }
	    break;
	}
    }
    *cp2 = 0;
    addrs[cur_addr] = NULL;
    return(addrs);
}

static char *
formatMessage(char ** addrs, const char * subject, const char * body)
{
    char * 	msg;
    int		size = 0;
    int		line;
    char **	to;

    /* Figure out how big we need the buffer to be. */
    for (to = addrs; *to; to++) {
	size += strlen(*to);
	size += 2; /* Leave room for the , */
    }

    size += strlen(subject);
    size += strlen(body);

    /* We will need space for the header names, a blank line, and
       other general formatting things. We could be exact, but
       1024 is more than enough and give us some spare.
       */
    size += 1024;

    msg = (char *)malloc(size);

    strcpy(msg, "To: ");
    line = 4;

    for (to = addrs; *to; to++) {
	strcat(msg, *to);
	if (*(to + 1) != NULL) {
	    strcat(msg, ", ");
	    line += strlen(*to);
	    if (line > 72) {
		strcat(msg, "\n    ");
		line = 0;
	    }
	}
    }
    strcat(msg, "\nSubject: ");

    strcat(msg, subject);
    if (msg[strlen(msg) - 1] == '\n') {
	msg[strlen(msg) - 1] = 0;
    }

    strcat(msg, "\nMime-Version: 1.0\n");
    strcat(msg, "Content-Type: text/plain;charset=us-ascii\n\n");
    strcat(msg, body);

    return(msg);
}

static int
deliver(char ** addrs, char * msg)
{
    char ** 	argv;
    int 	fd[2];
    int 	c_pid;
    int 	status;
    int		n_addrs;
    int		cp;

    for (n_addrs = 0; addrs[n_addrs]; n_addrs++) {
	continue;
    }

    argv = (char **)malloc((n_addrs + 2) * sizeof(char *));
    argv[0] = "/usr/lib/sendmail";

    for (cp = 0; addrs[cp]; cp++) {
	argv[cp + 1] = addrs[cp];
    }
    argv[cp + 1] = NULL;

    pipe(fd);

    c_pid = fork();
    if (c_pid < 0) {
	return(c_pid);
    }

    if (c_pid == 0) { /* The child. */
	dup2(fd[0], STDIN_FILENO);

#if defined(_AIX) /* Wrong prototype in AIX for execvp. */
	execvp("/usr/lib/sendmail", (const char **)argv);
#else
	execvp("/usr/lib/sendmail", (char *const *)argv);
#endif
	_exit(1); /* This had better never happen! */
    }
    else { /* The parent. */
	write(fd[1], msg, strlen(msg));
	close(fd[0]);
	close(fd[1]);

	waitpid(c_pid, &status, 0);
    }

    return(status);
}

int
submit_mail(const char * to,
	    const char * subject,
	    const char * body)
{
    char ** 	addrs;
    char **	ad;
    char *	msg;
    int		status;

    /* 
      Parse the address list so we can form a reasonable one
      for the user to see in the message.
      */
    addrs = arpaPhrase(to);

    msg = formatMessage(addrs, subject, body);

    status = deliver(addrs, msg);

    for (ad = addrs; *ad; ad++) {
	free(*ad);
    }
    free(addrs);

    free(msg);

    return(status);
}
