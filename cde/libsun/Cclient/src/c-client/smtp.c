/*
 * @(#)smtp.c	1.6 97/05/19
 *
 * Program:	Simple Mail Transfer Protocol (SMTP) routines
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	27 July 1988
 *
 * Sponsorship:	The original version of this work was developed in the
 *		Symbolic Systems Resources Group of the Knowledge Systems
 *		Laboratory at Stanford University in 1987-88, and was funded
 *		by the Biomedical Research Technology Program of the National
 *		Institutes of Health under grant number RR-00785.
 *
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  This software is made available "as is", and
 * THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
 * DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
 * INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
 * WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Edit Log:
 * [10-04-96] ctime_r
 * [10-21-96] Pass a mailstream in make_solaris_from. 
 * [10-25-96] Pass a mailstream in smtp_mail 
 * [10-26-96] Pass a mailstream in smtp_soutr
 * [10-26-96] Pass a mailstream in smtp_file_soutr
 * [10-26-96] Pass a mailstream in rfc822_output
 * [10-26-96] Pass a mailstream in sun_att_output
 * [11-08-96] Pass a mailstream in smtp_open, tcp_open, mm_log 
 *	      and smtp_log_outgoing_data
 * [01-24-96] Found by DTmail: Pass Mailstream to mm_log in 
 *	      smtp_log_outgoing_data_end.
 */

#include "UnixDefs.h"
#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include "mail.h"
#include "os_sv5.h"
#include "smtp.h"
#include "rfc822.h"
#include "sunatt.h"
#include "misc.h"

long smtp_file_soutr (void *stream, char *s, MAILSTREAM *mstream );
void smtp_log_outgoing_data_end(void *stream, MAILSTREAM *mstream);

/* Mailer parameters */

static long smtp_port = 0;		/* default port override */

/*
 * Mail Transfer Protocol open connection
 * Accepts: service host list
 *	    SMTP open options
 * Returns: SEND stream on success, NIL on failure
 */
SENDSTREAM *
smtp_open(char ** hostlist, long options, MAILSTREAM * mstream)
{
  SENDSTREAM 	*	stream = NULL;
  long 			reply;
  char 		*	s;
  char			tmp[MAILTMPLEN];
  void 		*	tcpstream;

  if (!(hostlist && *hostlist)) {
	mm_log("Missing SMTP service host", ERROR, mstream);
  } else do {			/* try to open connection */
    if (smtp_port) {
      sprintf(s = tmp, "%s:%ld", *hostlist, smtp_port);
    } else {
      s = *hostlist;		/* get server name */
    }
    if (tcpstream = tcp_open(s, "smtp", SMTPTCPPORT, mstream)) {
      stream = (SENDSTREAM *) fs_get(sizeof(SENDSTREAM));

      /* initialize stream */
      memset((void *) stream, 0, sizeof(SENDSTREAM));
      stream->tcpstream = tcpstream;
      stream->debug = (options & SOP_DEBUG) ? _B_TRUE : _B_FALSE;

      /* get name of local host to use */
      s =  strcmp("localhost", lcase(strcpy(tmp, *hostlist))) ?
	tcp_localhost(tcpstream) : "localhost";

      do reply = smtp_reply(stream);
      while (stream->reply[3] == '-');

      if (reply == SMTPGREET) {	/* get SMTP greeting */
	if ((options & SOP_ESMTP) && (smtp_ehlo (stream,s) == SMTPOK)) {
	  stream->local.esmtp.ok_ehlo = _B_TRUE;
	  return(stream);
	}

	/* try ordinary SMTP then */
	if (smtp_send(stream, "HELO", s) == SMTPOK) {
	  return(stream);
	}
      }
      smtp_close(stream);	/* otherwise punt stream */
    }
  } while (*++hostlist);	/* try next server */
  return(NULL);
}


/*
 *Mail Transfer Protocol close connection
 * Accepts: SEND stream
 * Returns: NIL always
 */
SENDSTREAM *
smtp_close(SENDSTREAM * stream)
{
  if (stream) {			/* send "QUIT" */
    smtp_send(stream, "QUIT", NIL);

    /* close TCP connection */
    tcp_close(stream->tcpstream);
    if (stream->reply) {
      fs_give((void **) &stream->reply);
    }
    fs_give((void **) &stream);/* flush the stream */
  }
  return(NULL);
}

/*
 * Mail Transfer Protocol deliver mail
 * Accepts: SEND stream
 *	    delivery option (MAIL, SEND, SAML, SOML)
 *	    message envelope
 *	    message body
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long
smtp_mail(SENDSTREAM 	* stream,
	  char 		* type,
	  ENVELOPE 	* env,
	  BODY 		* body,
	  char 		* outgoing_log,
	  int 		  body_format,
	  int 		  return_receipt, 
	  HEADERLIST 	* header_list,
	  MAILSTREAM 	* mstream)
{
  char		tmp[8*MAILTMPLEN];
  boolean_t	error = _B_FALSE;
  int 		retval;

  if (!(env->to || env->cc || env->bcc)) {
    /* no recipients in request */
    smtp_fake(stream, SMTPHARDERROR, "No recipients specified");
    return(_B_FALSE);
  }

  /* make sure stream is in good shape */
  smtp_send(stream, "RSET", NIL);
  strcpy(tmp, "FROM:<");	/* compose "MAIL FROM:<return-path>" */
  rfc822_address(tmp, env->return_path);
  strcat(tmp,">");

  /* BODY 8bit MIME only if MIME format */
  if (body_format != SUNATTBF && stream->ok_8bitmime) {
    strcat(tmp," BODY=8BITMIME");
  }

  /* send "MAIL FROM" command */
  if (!(smtp_send(stream, type, tmp) == SMTPOK)) {
    return(_B_FALSE);
  }
				/* negotiate the recipients */
  if (env->to) {
    smtp_rcpt(stream, env->to, &error);
  }
  if (env->cc) {
    smtp_rcpt(stream, env->cc, &error);
  }
  if (env->bcc) {
    smtp_rcpt(stream, env->bcc, &error);
  }
  if (error) {			/* any recipients failed? */
     				
    /* reset the stream */
    smtp_send (stream, "RSET", NIL);
    smtp_fake(stream, SMTPHARDERROR, "One or more recipients failed");
    return(_B_FALSE);
  }

  /* negotiate data command */
  if (!(smtp_send(stream, "DATA", NIL) == SMTPREADY)) {
    return NIL;
  }

  /* set up error in case failure */
  smtp_fake(stream, SMTPSOFTFATAL, "SMTP connection went away!");

  /* encode body as necessary */
  /* IF MIME outgoing ... */
  if (body_format != SUNATTBF) {
    if (stream->ok_8bitmime) {
      rfc822_encode_body_8bit(env, body);
    } else {
      rfc822_encode_body_7bit (env, body);
    }
  }

  /* Clear the io_bytecount sending */
  clear_tcp_iocounter(stream->tcpstream);

  /*
   * Stash pointer to outgoing mail log file. NIL indicates NOT TO
   * LOG this message locally
   */
  set_outgoing_log(stream->tcpstream, outgoing_log);

  /* output data, return success status */
  /* Dispatch on body format */
  switch (body_format) {

  default:
    /*FALLTHRU*/
  case MIMEBF:
    retval = rfc822_output(tmp, env, body, (soutr_t)smtp_soutr,
			   stream->tcpstream, return_receipt,
			   header_list, mstream) &&
      (smtp_send (stream,".",NIL) == SMTPOK);
    break;

  case SUNATTBF:
    retval = sun_att_output(tmp, env, body, (soutr_t)smtp_soutr,
			    stream->tcpstream, return_receipt,
			    header_list, mstream) &&
      (smtp_send (stream, ".", NIL) == SMTPOK);
    break;

  case SUNATTBF_TOFILE:
    retval = sun_att_output(tmp, env, body, (soutr_t)smtp_file_soutr,
			    stream->tcpstream, return_receipt,
			    header_list, mstream) &&
      (smtp_send (stream, ".", NIL) == SMTPOK);
    break;
  }

  if (retval) {
    smtp_log_outgoing_data_end((void *)stream->tcpstream, mstream);
  }
  return(retval);
}

/* Internal routines */

/*
 * Simple Mail Transfer Protocol send recipient
 * Accepts: SMTP stream
 *	    address list
 *	    pointer to error flag
 */
void
smtp_rcpt(SENDSTREAM * stream, ADDRESS * adr, boolean_t * error)
{
  char 		tmp[MAILTMPLEN];

  while (adr) {
    /* clear any former error */
    if (adr->error) {
      fs_give((void **) &adr->error);
    }
    if (adr->host) {		/* ignore group syntax */
      strcpy(tmp,"TO:<");	/* compose "RCPT TO:<return-path>" */
      rfc822_address(tmp, adr);
      strcat(tmp,">");

      /* send "RCPT TO" command */
      if (!(smtp_send(stream, "RCPT", tmp) == SMTPOK)) {
	*error = _B_TRUE;	/* note that an error occurred */
	adr->error = cpystr(stream->reply);
      }
    }
    adr = adr->next;		/* do any subsequent recipients */
  }
  return;
}

/*
 * Simple Mail Transfer Protocol send command
 * Accepts: SMTP stream
 *	    text
 * Returns: reply code
 */
long
smtp_send(SENDSTREAM * stream, char * command, char * args)
{
  char		tmp[MAILTMPLEN];
  long 		reply;

  /* build the complete command */
  if (args) {
    sprintf(tmp,"%s %s", command, args);
  } else {
    strcpy(tmp, command);
  }
  if (stream->debug) {
    mm_dlog(NIL, tmp);
  }
  strcat(tmp, "\015\012");

  /* send the command */
  if (!tcp_soutr(stream->tcpstream, tmp)) {
    return(smtp_fake(stream,
		     SMTPSOFTFATAL,
		     "SMTP connection broken (command)"));
  }
  do reply = smtp_reply(stream);
  while (stream->reply[3] == '-');
  return(reply);
}


/* 
 * Simple Mail Transfer Protocol get reply
 * Accepts: SMTP stream
 * Returns: reply code
 */
long
smtp_reply(SENDSTREAM * stream)
{
  /* flush old reply */
  if (stream->reply) {
    fs_give((void **) &stream->reply);
  }

  /* get reply */
  if (!(stream->reply = tcp_getline (stream->tcpstream))) {
    return(smtp_fake(stream, SMTPSOFTFATAL, "SMTP connection went away!"));
  }
  if (stream->debug) {
    mm_dlog(NIL, stream->reply);
  }
				/* return response code */
  return((long) atoi(stream->reply));
}

/*
 * Simple Mail Transfer Protocol send EHLO
 * Accepts: SMTP stream
 *	    host name
 * Returns: reply code
 */
long
smtp_ehlo(SENDSTREAM * stream,char * host)
{
  unsigned long 	i;
  unsigned long		j;
  char 			tmp[MAILTMPLEN];

  sprintf(tmp, "EHLO %s", host);	/* build the complete command */
  if (stream->debug) {
    mm_dlog(NIL,tmp);
  }
  strcat(tmp, "\015\012");

  /* send the command */
  if (!tcp_soutr(stream->tcpstream,tmp)) {
    return(smtp_fake(stream, SMTPSOFTFATAL, "SMTP connection broken (EHLO)"));
  }

  /* got an OK reply? */
  do {
    if ((i = smtp_reply (stream)) == SMTPOK) {
      ucase (strcpy (tmp,stream->reply+4));
      /* command name */
      j = (((long) tmp[0]) << 24) + (((long) tmp[1]) << 16) +
	(((long) tmp[2]) << 8) + tmp[3];

      /* defined by SMTP 8bit-MIMEtransport */
      if (j == (('8'<<24) + ('B'<<16) + ('I'<<8) + 'T')
	  && tmp[4]=='M' && tmp[5]=='I' && tmp[6]=='M' && tmp[7]=='E'
	  && !tmp[8]) {
	stream->ok_8bitmime = _B_TRUE;

      } else { /* defined by SMTP Size Declaration */
	
	if (j == (('S' << 24) + ('I' << 16) + ('Z' << 8) + 'E') &&
	    (!tmp[4] || tmp[4] == ' ')) {
	  if (tmp[4]) {
	    stream->size = atoi (tmp+5);
	  }
	  stream->local.esmtp.ok_size = _B_TRUE;
	  
	} else { /* defined by SMTP Service Extensions */

	  if (j == (('S'<<24) + ('E'<<16) + ('N'<<8) + 'D')
	      && !tmp[4]) {
	    stream->local.esmtp.ok_send = _B_TRUE;

	  } else {
	    if (j == (('S'<<24) + ('O'<<16) + ('M'<<8) + 'L')
		&& !tmp[4]) {
	      stream->local.esmtp.ok_soml = _B_TRUE;

	    } else {
	      if (j == (('S'<<24) + ('A'<<16) + ('M'<<8) + 'L')
		  && !tmp[4]) {
		stream->local.esmtp.ok_saml = _B_TRUE;

	      } else {
		if (j == (('E'<<24) + ('X'<<16) + ('P'<<8) + 'N')
		    && !tmp[4]) {
		  stream->local.esmtp.ok_expn = _B_TRUE;

		} else {
		  if (j == (('H'<<24) + ('E'<<16) + ('L'<<8) + 'P')
		      && !tmp[4]) {
		    stream->local.esmtp.ok_help = _B_TRUE;

		  } else {
		    if (j == (('T'<<24) + ('U'<<16) + ('R'<<8) + 'N')
			&& !tmp[4]) {
		      stream->local.esmtp.ok_turn = _B_TRUE;
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  } while (stream->reply[3] == '-');
  return(i);			/* return the response code */
}

/*
 *  Simple Mail Transfer Protocol set fake error
 * Accepts: SMTP stream
 *	    SMTP error code
 *	    error text
 * Returns: error code
 */
long
smtp_fake(SENDSTREAM * stream, long code, char * text)
{
  /* flush any old reply */
  if (stream->reply ) {
    fs_give((void **) &stream->reply);
  }

  /* set up pseudo-reply string */
  stream->reply = (char *) fs_get(20+strlen (text));
  sprintf(stream->reply, "%ld %s", code, text);
  return(code);			/* return error code */
}

/*
 * Log outgoing mail. Note that if we ever use a non-tcp
 * stream for smtp connections, then the stream will need
 * to be marked with some kind of status flag so we can
 * correctly dispatch off of its structure to get the file name.
 */
void
smtp_log_outgoing_data(void * stream, char * data, MAILSTREAM * mstream)
{
  FILE 		* ofile;
  char 		* fname= get_outgoing_log((TCPSTREAM *)stream);
  char 		  c;
  char 		  tmp[MAILTMPLEN];

  if (!fname) {
    return;		/* nothing to log */
  }

  /* Open the file for create/write at eof */
  ofile = fopen(fname, "a+");

  if (!ofile) {
    sprintf(tmp, "Open failed for outgoing mail logfile %s", fname);
    mm_log(tmp, ERROR, mstream);
    return;
  }

  /*
   * write the data char by char turning CRLF to LF */
  while (c = *data++) {
    if (c == '\015' && *data == '\012') {
      continue;			/* skip the CR */
    } else {
      fputc((int)c, ofile);
    }
  }

  /* Do all one can to get the data to disk now */
  fflush(ofile);
  fsync(fileno(ofile));
  fclose(ofile);

  return;
}

void
smtp_log_outgoing_data_end(void * stream, MAILSTREAM * mstream)
{
  FILE 		* ofile;
  char 		* fname= get_outgoing_log((TCPSTREAM *)stream);
  char 		  c;
  char 		  tmp[MAILTMPLEN];

  if (!fname) {
    return;		/* nothing to log */
  }

  /* Open the file for create/write at eof */
  ofile = fopen(fname, "a+");

  if (!ofile) {
    sprintf(tmp, "Open failed for outgoing mail logfile %s", fname);
    mm_log(tmp, ERROR, mstream);
    return;
  }
  /* write LF inter-message gap */
  fputc((int)'\n', ofile);

  /* Do all one can to get the data to disk now */
  fflush(ofile);
  fsync(fileno(ofile));
  fclose(ofile);

  return;
}

/*
 * NOTE: All outgoing mail files are in solaris/bezerk mailbox
 * format
 */
#define DATELEN 64
char *
make_solaris_from(MAILSTREAM * stream)
{
  char 		  tmp[MAILTMPLEN];
  char 		* username= myusername(stream);
  char 		* from;
  time_t 	  tics = time(0);
  const int 	  buf_len = 26;
  char 		  buf_time[26];

  sprintf(tmp, "From %s@%s %s", username, local_host_name(stream),
#if (_POSIX_C_SOURCE - 0 >= 199506L)
	  ctime_r(&tics, buf_time)
#else
	  ctime_r(&tics, buf_time, buf_len)
#endif
	  );
  from = strdup(tmp);
  return(from);
}

/*
 * Simple Mail Transfer Protocol filter mail
 * Accepts: stream
 *	    string
 * Returns: _B_TRUE on success, _B_FALSE on failure
 */
long
smtp_soutr(void * stream, char * s, MAILSTREAM * mstream)
{
		/* Note: two different streams.*/

  char 		  c;
  char		* t;
  
  if (!outgoing_from_done(stream)) { /* test and set, then write */
    char 	* from= make_solaris_from(mstream);

    smtp_log_outgoing_data(stream, from, mstream);
    fs_give((void **)&from);
  }

  /* Log outgoing data if appropriate */
  smtp_log_outgoing_data(stream, s, mstream);

  /* "." on first line */
  if (s[0] == '.') {
    tcp_soutr(stream, ".");
  }

  /* find lines beginning with a "." */
  while (t = strstr (s, "\015\012.")) {
    c = *(t += 3);		/* remember next character after "." */
    *t = '\0';			/* tie off string */
				/* output prefix */
    if (!tcp_soutr(stream, s)) {
      return _B_FALSE;
    }
    *t = c;			/* restore delimiter */
    s = t - 1;			/* push pointer up to the "." */
  }
				/* output remainder of text */
  return(*s ? tcp_soutr(stream, s) : _B_TRUE);
}

/*
 * Output message to the tmp file for debugging:
 *   File: /tmp/smtp_output 
 */
long
smtp_file_soutr(void * stream, char * s, MAILSTREAM * mstream)
{
  FILE 	*	f;

  f = fopen("/tmp/smtp_output", "a");
  if (!f) {
    mm_log("Could not open /tmp/smtp_output for output", ERROR, mstream);
    return(_B_FALSE);
  }
  fputs(s, f);
  fflush(f);
  fsync(fileno(f));
  fclose(f);

  return(_B_TRUE);
}
