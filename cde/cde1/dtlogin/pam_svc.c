/*******************************************************************************
**
**  pam_svc.c 1.17 97/06/08
**
**  Copyright 1993-1996  Sun Microsystems, Inc.  All rights reserved.
**
**  This file contains procedures specific to dtlogin's use of
**  PAM (Pluggable Authentication Module) security library.
**
*******************************************************************************/
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc. 	 	*
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*
 * Header Files
 */

#include <utmpx.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/param.h>
#include <pwd.h>
#include <dlfcn.h>
#include <security/pam_appl.h>
#include "dm.h"
#include "pam_svc.h"
#include "solaris.h"
#include "vgproto.h"
#include "vgmsg.h"

/*
 * Local function declarations
 */

static int login_conv(int num_msg, struct pam_message **msg,
    		      struct pam_response **response, void *appdata_ptr);

static int load_function(void *, char *, int (**func)());


/*
 * Local structures and variables 
 */

static struct pam_conv pam_conv = {login_conv, NULL};
static pam_handle_t *pamh = NULL;
static int pam_auth_trys = 0;
static int use_greet_pipe = FALSE;

typedef struct {

	int (*start) 
	( 
	    const char *service_name,
            const char *user,                     
            const struct pam_conv *pam_conv,     
            pam_handle_t **pamh 
	);

	int (*set_item) 
	(
	    pam_handle_t *pamh,
	    int item_type,
            const void *item
	);

	int (*authenticate) 
	(
	    pam_handle_t *pamh,
	    int flags
	);

	int (*acct_mgmt) 
	(
	    pam_handle_t *pamh,
	    int flags
	);

	int (*open_session) 
	(
	    pam_handle_t *pamh,
	    int flags
	);

	int (*close_session) 
	(
	    pam_handle_t *pamh,
	    int flags
	);

	int (*setcred) (
	    pam_handle_t *pamh,
	    int flags
	);

	int (*free_resp) (
	    int num_msg,
	    struct pam_response *resp
	);

        int (*end) (
	    pam_handle_t *pamh,
	    int status
	);

} pam_func;

static pam_func *pfunc = NULL;


/****************************************************************************
 * PamInit
 *
 * Initialize or Update PAM datastructures.
 *
 ****************************************************************************/

int 
PamInit(char* prog_name,
        char* user, 
	char* line_dev,
        char* display_name)
{
	void	*lfd;
	int status=PAM_SUCCESS;

	if (!pfunc) {
	    Debug("PamInit: try libpam dlopen\n");
	    lfd = (void *) dlopen("libpam.so", RTLD_LAZY);

	    if (lfd == NULL) {
		Debug("PamInit: libpam not found, need libauth fallback\n"); 
		return(PAM_OPEN_ERR);
	    }

	    pfunc = malloc(sizeof(pam_func));
	    if (!pfunc)
		return(PAM_BUF_ERR);

	    bzero(pfunc, sizeof(pam_func));

	    if (PAM_SUCCESS != load_function(lfd, "pam_start", 
				&pfunc->start)) return(PAM_SYMBOL_ERR);

	    if (PAM_SUCCESS != load_function(lfd, "pam_set_item", 
				&pfunc->set_item)) return(PAM_SYMBOL_ERR);

	    if (PAM_SUCCESS != load_function(lfd, "pam_authenticate", 
				&pfunc->authenticate)) return(PAM_SYMBOL_ERR);

	    if (PAM_SUCCESS != load_function(lfd, "pam_acct_mgmt", 
				&pfunc->acct_mgmt)) return(PAM_SYMBOL_ERR);

	    if (PAM_SUCCESS != load_function(lfd, "pam_open_session", 
				&pfunc->open_session)) return(PAM_SYMBOL_ERR);

	    if (PAM_SUCCESS != load_function(lfd, "pam_close_session",
				&pfunc->close_session)) return(PAM_SYMBOL_ERR);

	    if (PAM_SUCCESS != load_function(lfd, "pam_setcred",
				&pfunc->setcred)) return(PAM_SYMBOL_ERR);

	    if (PAM_SUCCESS != load_function(lfd, "__pam_free_resp",
				&pfunc->free_resp)) return(PAM_SYMBOL_ERR);

	    if (PAM_SUCCESS != load_function(lfd, "pam_end",
				&pfunc->end)) return(PAM_SYMBOL_ERR);

	}

        if (!pamh) {
	    char *loc;

	    loc = getenv("LC_MESSAGES");
	    if (loc) {
		Debug("PamInit: LC_MESSAGES=%s\n", loc);
	    } else {
		Debug("PamInit: LC_MESSAGES Not defined\n");
	    }
	    loc = getenv("LANG");
	    if (loc) {
		Debug("PamInit: LANG=%s\n", loc);
	    } else {
		Debug("PamInit: LANG Not defined\n", loc);
	    }
	    loc = getenv("NLSPATH");
	    if (loc) {
		Debug("PamInit: NLSPATH=%s\n", loc);
	    } else {
		Debug("PamInit: NLSPATH Not defined\n");
	    }

            /* Open PAM (Plugable Authentication module ) connection */
	    status = pfunc->start( DTLOGIN_SVC, user, &pam_conv, &pamh );
	    if (status != PAM_SUCCESS) {
        	Debug("PamInit: pam_start error=%d\n", status);
	        pamh = NULL;
	    } else {
        	Debug("PamInit: pam_start success\n");
	    }
	} else {
	    pfunc->set_item(pamh, PAM_SERVICE, DTLOGIN_SVC);
	    if (user) pfunc->set_item(pamh, PAM_USER, user);
	}

	if (status == PAM_SUCCESS) {
            if (line_dev) pfunc->set_item(pamh, PAM_TTY, line_dev);
	    if (display_name) pfunc->set_item(pamh, PAM_RHOST, display_name);
	}

	return(status);
}

/****************************************************************************
 * PamAuthenticate
 *
 * Authenticate that user / password combination is legal for this system
 *
 ****************************************************************************/

int
PamAuthenticate ( char*   prog_name,
		  char*   display_name,
		  char*   user_passwd,
		  char*   passreq,
		  char*   user, 
		  char*   line )
{
	int status;
        char* user_str = user ? user : "NULL";
        char* line_str = line ? line : "NULL";
	char* line_dev = _sdt_create_devname(line_str);  
	struct passwd *pwd;

        Debug("PamAuthenticate: prog_name=%s\n", prog_name);
        Debug("PamAuthenticate: display_name=%s\n", display_name);
        Debug("PamAuthenticate: user=%s\n", user_str);
        Debug("PamAuthenticate: line=%s\n", line_str);
        Debug("PamAuthenticate: line_dev=%s\n", line_dev);

	if (user_passwd) {
	    if (strlen(user_passwd) == 0) {
	        Debug("PamAuthenticate: user passwd empty\n"); 
	    } else {
	        Debug("PamAuthenticate: user passwd present\n"); 
	    }
        }
        else {
	    Debug("PamAuthenticate: user passwd NULL\n");
	}

	status = PamInit(prog_name, user, line_dev, display_name);

#ifdef sun        
	if (status != PAM_SUCCESS) { 

	    if (line_dev) free(line_dev);

	    if (user_passwd == NULL) {
	        Debug("PamAuthenticate: get passwd for libauth fallback\n");
		return(PAM_OPEN_ERR);
	    }

	    Debug("PamAuthenticate: trying libauth fallback\n");

	    return(solaris_authenticate(prog_name, display_name,
		  			user_passwd, passreq, user, line));
	}

        /* Solaris BSM Audit trail */

	audit_login_save_host(display_name);
	audit_login_save_ttyn(line_dev);
	audit_login_save_port();
#endif

	/* User Authentication */

        status = pfunc->authenticate( pamh, 0 );

	Debug("PamAuthenticate: return status = %d\n", status);

	pam_auth_trys++;

	pwd = getpwnam(user);
	audit_login_save_pw(pwd);

	if (status == PAM_SUCCESS) {
	    int flags;

	    if ((passreq != NULL) && (!strcasecmp("YES", passreq)))
		flags = PAM_DISALLOW_NULL_AUTHTOK;
	    else
		flags = 0;

	    /*
	     * Entered password was authenticated. 
	     * Now verify its still valid for system.
	     */ 

	    status = pfunc->acct_mgmt(pamh, flags);
	} else {
	    /* Invalid account or password */

	    audit_login_bad_pw();
	    sleep(SOLARIS_LOGIN_SLEEPTIME);

            if (status == PAM_MAXTRIES) {
		pam_auth_trys = PAM_LOGIN_MAXTRIES+1;
	    }	

	    if (pam_auth_trys > PAM_LOGIN_MAXTRIES) {
		audit_login_maxtrys();
	    	sleep(PAM_LOGIN_DISABLETIME);
	    }
	}

	if (status != PAM_SUCCESS) {
            Debug("PamAuthenticate: PAM error=%d\n", status);
	    if (pamh) {
                Debug("PamAuthenticate: calling pam_end\n");
	        pfunc->end(pamh, PAM_ABORT);
		pamh=NULL;
	    }
	}  

	if (line_dev) free(line_dev);

	return(status);
}




/****************************************************************************
 * PamAccounting
 *
 * Work related to open and close of user sessions
 ****************************************************************************/

int
PamAccounting( char*   prog_name,
	       char*   display_name,
	       char*   entry_id,
	       char*   user, 
	       char*   line, 
	       pid_t   pid,
	       int     entry_type,
	       int     exitcode )
{
	int session_type, status;
        char *user_str = user ? user : "NULL";
        char *line_str = line ? line : "NULL";
	char *line_dev = _sdt_create_devname(line_str);  

        Debug("PamAccounting: prog_name=%s\n", prog_name);
        Debug("PamAccounting: display_name=%s\n", display_name);
        Debug("PamAccounting: entry_id=%c %c %c %c\n", entry_id[0],
				   entry_id[1], entry_id[2], entry_id[3]);
        Debug("PamAccounting: user=%s\n", user_str);
        Debug("PamAccounting: line=%s\n", line_str);
        Debug("PamAccounting: line_dev=%s\n", line_dev);
        Debug("PamAccounting: pid=%d\n", pid);
        Debug("PamAccounting: entry_type=%d\n", entry_type);
        Debug("PamAccounting: exitcode=%d\n", exitcode);

        /* Update line device's access time and initialize PAM */

	_sdt_dev_touch(line_dev);

	status = PamInit(prog_name, user, line_dev, display_name);

	if (status != PAM_SUCCESS) {
	    if (line_dev) free(line_dev);

	    return(solaris_accounting(prog_name, display_name, entry_id,
				      user, line, pid, entry_type, exitcode));
	}


	/* Session accounting */

	if (status == PAM_SUCCESS) switch(entry_type) {
	    case USER_PROCESS:
		Debug("PamAccounting: entry_type is USER_PROCESS\n");

		/* New user session, open session accounting logs */
		status = pfunc->open_session(pamh, 0);
		if (status != PAM_SUCCESS) {
                    Debug("PamAccounting: USER_PROCESS open_session error=%d\n",
								 status); 
		}
#ifdef sun
	        if (status == PAM_SUCCESS) audit_login_success();
#endif
		session_type = SOLARIS_LOGIN;
		status = solaris_setutmp_mgmt(user, line_str, display_name, 
					session_type, entry_type, entry_id); 
	        if (status != SOLARIS_SUCCESS) {
                    Debug("PamAccounting: USER_PRCESS set_utmp error=%d\n", 
								status);
 		}
		break;


	    case ACCOUNTING:
		Debug("PamAccounting: entry_type is ACCOUNTING\n");
		/* 
		 * User session has terminated, mark it DEAD 
		 */

		entry_type = DEAD_PROCESS;
		session_type = SOLARIS_NOLOG;
		status = solaris_setutmp_mgmt(user, line_str, display_name, 
					 session_type, entry_type, entry_id); 

	        if (status != SOLARIS_SUCCESS) {
                    Debug("PamAccounting: ACCOUNTING set_utmp error=%d\n", 
								status);
 		}
		/* Intentional fall thru */


	    case DEAD_PROCESS:
		Debug("PamAccounting: entry_type is DEAD_PROCESS\n");

		/* Cleanup account files for dead processes */
		status = pfunc->close_session(pamh, 0);
	        if (status != PAM_SUCCESS) {
                    Debug("PamAccounting: DEAD_PROCES close_session error=%d\n",
								 status); 
	        }

		status = solaris_reset_utmp_mgmt(pid, exitcode, entry_id);

		if (status != SOLARIS_SUCCESS) {
                    Debug("PamAccounting: DEAD_PROCESS reset_utmp error=%d\n",
								      status);
		}

		break;


	    case LOGIN_PROCESS:
	    default:	
		Debug("PamAccounting: entry_type is LOGIN_PROCESS\n");

		session_type = 0;
		status = solaris_setutmp_mgmt(user, line_str, display_name, 
					 session_type, entry_type, entry_id); 
	        if (status != SOLARIS_SUCCESS) {
                    Debug("PamAccounting: LOGIN_PRCESS set_utmp error=%d\n", 
								status);
 		}
		break;
	}

	if (line_dev) free(line_dev);

	return(status);
}



/****************************************************************************
 * PamSetCred
 *
 * Set Users login credentials: uid, gid, and group lists 
 ****************************************************************************/

int
PamSetCred(char* prog_name, char* user, struct verify_info* verify) 
{
	int cred_type, status, ngroups;
        char* user_str = user ? user : "NULL";
	gid_t groups[NGROUPS_UMAX];

        Debug("PamSetCred: prog_name=%s\n", prog_name);
        Debug("PamSetCred: user=%s\n", user_str);
        Debug("PamSetCred: uid=%d\n", verify->uid);
        Debug("PamSetCred: gid=%d\n", verify->gid);

	status = PamInit(prog_name, user, NULL, NULL);
	if (status != PAM_SUCCESS) {
	    return( solaris_setcred(prog_name, user, 
					       verify->uid, verify->gid) );
        }

	/* Set Group credentials */

	if ( setgid(verify->gid) == -1 ) 
	    status = SOLARIS_BAD_GID; 

	Debug("PamSetCred: ngroups before = %d\n", getgroups( 0, NULL ));

        if (status == PAM_SUCCESS &&
            ( !user || (initgroups(user, verify->gid) == -1) )) {
            status = SOLARIS_INITGROUP_FAIL;
	    ngroups = 0;
	} else { 
	    ngroups = getgroups( NGROUPS_UMAX, groups); 
	}

	Debug("PamSetCred: ngroups after = %d\n", getgroups( 0, NULL ));

	/*
	 * Send credential information to the Xserver.
         */

        solaris_xserver_cred( verify, ngroups, groups);


        /*
         * Do PAM credendial setup and final uid set 
  	 */

	if (status == PAM_SUCCESS)
	    status = pfunc->setcred(pamh, PAM_ESTABLISH_CRED);

	if (status == PAM_SUCCESS && (setuid(verify->uid) == -1))
	    status = SOLARIS_BAD_UID;

	if (status != PAM_SUCCESS) {
            Debug("PamSetCred: user=%s, err=%d)\n", user, status);
	}

	return(status);
}


/****************************************************************************
 * PamUseGreetPipe
 *
 * Sets flag to use or not use the DtGreet communication pipe 
 ****************************************************************************/

void
PamUseGreetPipe(int val) {
	use_greet_pipe = val;
}


/****************************************************************************
 * PamClose
 *
 * Closes Pam handle  
 ****************************************************************************/

int
PamClose(int val) {
	int status=PAM_SUCCESS;

	if (pamh) {
            Debug("PamClose: closing pam handle\n");
	    status = pfunc->end(pamh, val);
	    pamh=NULL;
	} else {
            Debug("PamClose: no pam handle found\n");
	}

	return(status);
}


/*****************************************************************************
 * login_conv():
 *
 * This is a conv (conversation) function called from the PAM 
 * authentication scheme.  It returns the user's password when requested by
 * internal PAM authentication modules and also logs any internal PAM error
 * messages.
 *****************************************************************************/

static int
login_conv(int num_msg, struct pam_message **msg,
           struct pam_response **response, void *appdata_ptr)
{
	struct pam_message	*m;
	struct pam_response	*r;
	char 			*temp;
	int			k;
 	unsigned char 		g_request[REQUEST_LIM_MAXLEN];
 	unsigned char 		g_responce[REQUEST_LIM_MAXLEN];

	RequestMessage   *g_msg = (RequestMessage *) g_request;
	RequestChallenge *c_msg = (RequestChallenge *) g_request;
	ResponseChallenge *r_msg = (ResponseChallenge *) g_responce;

#ifdef lint
	conv_id = conv_id;
#endif
	if (num_msg <= 0)
		return (PAM_CONV_ERR);

	*response = (struct pam_response*) 
				calloc(num_msg, sizeof (struct pam_response));
	if (*response == NULL)
		return (PAM_CONV_ERR);

	(void) memset(*response, 0, sizeof (struct pam_response));

	k = num_msg;
	m = *msg;
	r = *response;
	while (k--) {

	    switch (m->msg_style) {

	    case PAM_PROMPT_ECHO_ON:
	    case PAM_PROMPT_ECHO_OFF:
		if (m->msg != NULL) { 
		    char *user_response;

	    	    Debug ("login_conv PROMPT: '%s'\n", m->msg); 

		    if (use_greet_pipe) {
			char *mg=m->msg;

			if (strcmp(mg, "Password: ") == 0) {
			   /* 
			    * Some locales don't yet have "Password: " 
			    * translated.  Use DtLogin catalog's translation. 
			    */ 
			   Debug("nl_fd = %d\n", nl_fd);
			   mg=(char*) ReadCatalog(MC_LABEL_SET,MC_PASSWD_LABEL,
				          	  MC_DEF_PASSWD_LABEL);
			}

			c_msg->hdr.opcode = REQUEST_OP_CHALLENGE;
			c_msg->hdr.reserved = 0;
		        if (m->msg_style == PAM_PROMPT_ECHO_ON) {
			    c_msg->bEcho = TRUE;
			} else {
			    c_msg->bEcho = FALSE;
			}
			c_msg->idMC = 0;
			c_msg->offChallenge = sizeof(*c_msg);
        		c_msg->offUserNameSeed = 0;
        		c_msg->enableClearBtn = False;
			strncpy(((char *)c_msg) + c_msg->offChallenge, mg, 
				REQUEST_LIM_MAXLEN - c_msg->offChallenge );
			c_msg->hdr.length = sizeof(*c_msg) + strlen(mg) + 1;

			if (c_msg->hdr.length > REQUEST_LIM_MAXLEN) {
			    c_msg->hdr.length = REQUEST_LIM_MAXLEN;
		        }

			TellGreeter((RequestHeader *)c_msg);
			AskGreeter(NULL, (char*)g_responce, REQUEST_LIM_MAXLEN);
			user_response=(char *)g_responce + r_msg->offResponse;

			Debug("Got user prompt response\n");

                        if (user_response != NULL) {
			    r->resp = (char *)malloc(strlen(user_response)+1);

		            if (r->resp != NULL) {
			        (void) strcpy(r->resp, user_response);
                                r->resp_retcode=0;
			    }
			} 

			if (user_response == NULL || r->resp == NULL) {
			    pfunc->free_resp(num_msg, *response);
                            *response = NULL;
                            return (PAM_CONV_ERR);
			}
                    } 
		}

		m++;
		r++;
		break;


	    case PAM_ERROR_MSG:
	    case PAM_TEXT_INFO:
		if (m->msg != NULL) { 
	    	    Debug ("login_conv ERROR / INFO: %s\n", m->msg); 

		    if (use_greet_pipe) {
			g_msg->hdr.opcode = REQUEST_OP_MESSAGE;
			g_msg->hdr.reserved = 0;
			g_msg->idMC = 0;
			g_msg->offMessage = sizeof(*g_msg);
			strncpy(((char *)g_msg) + g_msg->offMessage, m->msg, 
				REQUEST_LIM_MAXLEN - g_msg->offMessage );
			g_msg->hdr.length = sizeof(*g_msg) + strlen(m->msg) + 1;

			if (g_msg->hdr.length > REQUEST_LIM_MAXLEN) {
			    g_msg->hdr.length = REQUEST_LIM_MAXLEN;
		        }

			TellGreeter((RequestHeader *)g_msg);
			AskGreeter(NULL, (char*)g_responce, REQUEST_LIM_MAXLEN);
		    }
		}

		m++;
		r++;
		break;

	    default:
	   	Debug ("login_conv: Unexpected case %d\n", m->msg_style);
		break;
	    }
	}
	return (PAM_SUCCESS);
}

/*****************************************************************************
 * load_function():
 *
 * Loads functions symbol from a dlopen'd library
 *
 *****************************************************************************/
static int
load_function(void *lfd, char *name, int (**func)())
{
        char *errmsg = NULL;

        *func = (int (*)())dlsym(lfd, name);

        if (*func == NULL) {
	    errmsg = (char *) dlerror();
	    Debug("pam_src, load_function: dlsym failed %s: error %s", 
		   name, errmsg != NULL ? errmsg : "");
            return (PAM_SYMBOL_ERR);
        }

	return(PAM_SUCCESS);
}

