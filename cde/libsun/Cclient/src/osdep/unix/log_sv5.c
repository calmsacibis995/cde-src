/*
 * Program:	SVR4 server login
 *
 * Author:	Mark Crispin
 *		Networks and Distributed Computing
 *		Computing & Communications
 *		University of Washington
 *		Administration Building, AG-44
 *		Seattle, WA  98195
 *		Internet: MRC@CAC.Washington.EDU
 *
 * Date:	11 May 1989
 * Last Edited:	10-23-96
 *
 * Copyright 1995 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appears in all copies and that both the
 * above copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  This software is made
 * available "as is", and
 * THE UNIVERSITY OF WASHINGTON DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN
 * NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Edit log:
 * [10-03-96 clin] getpwnam_r and getspnam_r implemented to ensure MT-safe.
 * [10-05-96 clin] can't free pw buffer in this routine yet. 
 * [10-23-96 clin] passing stream in mail_parameters.
 *
 */
#include "os_sv5.h"

/* Server log in
 * Accepts: user name string
 *	    password string
 *	    optional place to return home directory
 * Returns: T if password validated, NIL otherwise
 */
#include <grp.h>
long server_login (char *user,char *pass,int argc,char *argv[], MAILSTREAM *stream)
{
  char *pwd,tmp[MAILTMPLEN];
  long ret; 
  struct spwd *sp = NIL, res_sp;
  gid_t saved_egid;
  char *buf_pwd = (char *)fs_get(NSS_BUFLEN_PASSWD + 1);
  char *buf_sp  = (char *)fs_get(NSS_BUFLEN_SHADOW + 1);
  struct passwd res_pwd;
  struct passwd *pw;

#if (_POSIX_C_SOURCE - 0 >= 199506L)
  getpwnam_r (user, &res_pwd, buf_pwd, NSS_BUFLEN_PASSWD, &pw );
#else
  pw = getpwnam_r (user, &res_pwd, buf_pwd, NSS_BUFLEN_PASSWD);
#endif

  mail_stream_setNIL(stream);

  /* allow case-independent match */
  /* if pw equals null, why not reuse buf_pwd? */
  if (!pw) 
#if (_POSIX_C_SOURCE - 0 >= 199506L)
    getpwnam_r(lcase(strcpy(tmp,user)),&res_pwd,buf_pwd,NSS_BUFLEN_PASSWD,&pw);
#else
    pw =getpwnam_r(lcase(strcpy(tmp,user)),&res_pwd,buf_pwd,NSS_BUFLEN_PASSWD);
#endif
				/* no entry for this user or root */
  if (!(pw && pw->pw_uid)) {
        fs_give( (void **) &buf_pwd ); /* Need to flush it here . */
        fs_give( (void **) &buf_sp  ); /* Need to flush it here . */
	return NIL;
	}
	
  if ((((*(pwd = pw->pw_passwd) == 'x') && (!pwd[1])) ||
       ((pwd[0] == '#') && (pwd[1] == '#'))) && 
	(sp = getspnam_r(pw->pw_name, &res_sp, buf_sp, NSS_BUFLEN_SHADOW + 1 )))
    	pwd = sp->sp_pwdp;	/* get shadow password if necessary */
				/* validate password and password age */
  if (strcmp (pwd,(char *) crypt (pass,pwd)) ||
      (sp && (sp->sp_lstchg > 0) && (sp->sp_max > 0) &&
       ((sp->sp_lstchg + sp->sp_max) < (time (0) / (60*60*24))))) {
        fs_give( (void **) &buf_pwd ); /* or here . */
        fs_give( (void **) &buf_sp  ); 
    	return NIL;
	}
  saved_egid = getegid();       /* Get the effective gid and save it */
  mail_parameters(stream,SET_EGID,(void *)saved_egid);
  mail_stream_unsetNIL(stream);
  initgroups (pw->pw_name,pw->pw_gid);/* initialize groups */
  /* NOW, set real gid, effective gid and saved gid to user */
  setgid (pw->pw_gid);	        /* all OK, login in as that user */
  setegid(saved_egid);          /* Set the effective gid again */
  setuid (pw->pw_uid);          /* relinquish root */

  /* buf_pwd can NOT be freed here since it will be used again in env_init.*/ 
	ret = env_init (pw->pw_name,pw->pw_dir,stream);
        fs_give( (void **) &buf_sp  ); /*Free them after getting ret. */
	fs_give( (void **) &buf_pwd );
  return ret;
}
