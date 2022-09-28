/* $XConsortium: access.c /main/cde1_maint/2 1995/10/03 12:30:03 barstow $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)access.c	1.23 97/04/08 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#ifdef SunOS
#include <sys/systeminfo.h>
#include <netdir.h>
#endif
#include "access.h"
#include "appt4.h"
#include "utility.h"
#include "log.h"
#include "lutil.h"
#include "laccess.h"

#define NUM_CACHE               50      /* cache for unix user name */

extern int debug;
extern boolean_t strict_csa_model;

typedef struct uname_cache {
	int	uid;			/* unix user id */
	char	*name;			/* user name */
	struct	uname_cache *next;
} Uname_cache;

static	Uname_cache	*ucache_list = NULL;

/* structure to store access control list for calendar creation */
typedef struct __UserInfo {
	char	*user;
	char	*host;
	struct __UserInfo *next;
} _UserInfo;

typedef struct __DtCmsAccessList {
	boolean_t	set;
	_UserInfo	*acl;
} _DtCmsAccessList;

_DtCmsAccessList	_DtCmsTrusted;
_DtCmsAccessList	_DtCmsUntrusted;

#ifdef MT
pthread_mutex_t ucache_lock;
pthread_mutex_t trusted_lock;
pthread_mutex_t untrusted_lock;
#endif

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static boolean_t _InList(_UserInfo *list, _DtCmsSender *user);
static void _PrintList(_DtCmsAccessList accessinfo, boolean_t trusted);
static void _FreeAccessControlList(_UserInfo *ptr);
static _UserInfo * _GetUserInfo(char *userinfo);
static _UserInfo * _PutUserInfoInOrder(_UserInfo *head, _UserInfo *new);
static Uname_cache * in_u_cache(int uid);
static CSA_return_code _GetUserName(int uid, char **name);
static Access_Entry_4 * in_access_list(Access_Entry_4 *l, char *s);
static Access_Entry_4 * combine_access_list(Access_Entry_4 *p_list, 
				Access_Entry_4 *p_head, CSA_flags type,
				CSA_flags *p_world);
static CSA_return_code _GetV4AccessRights(_DtCmsCalendar *cal, char *target,
				_DtCmsSender *sender, CSA_flags *access);
static CSA_return_code _GetV5AccessRights(_DtCmsCalendar *cal, char *target,
				_DtCmsSender *sender, CSA_flags *access);
#ifdef SunOS
static CSA_return_code _GetHostNameFromList(struct nd_hostservlist *hostlist,
				char **hostname);
#endif

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

extern CSA_return_code
_DtCmsLoadAndCheckAccess(
	struct svc_req	*svcrq,
	char		*target,
	_DtCmsSender	**sender,
	CSA_flags	*access,
	_DtCmsCalendar	**cal)
{
	CSA_return_code	stat;

	if (target == NULL || sender == NULL || cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCmsGetClientInfo(svcrq, sender)) != CSA_SUCCESS)
		return (stat);

	if ((stat = _DtCmsGetCalendarByName(target, cal))
	    != CSA_SUCCESS)
		return (stat);

	if ((*cal)->fversion == _DtCMS_VERSION1)
		return (_GetV4AccessRights(*cal, target, *sender, access));
	else
		return (_GetV5AccessRights(*cal, target, *sender, access));
}

extern CSA_return_code
_DtCmsGetClientInfo(struct svc_req *svcrq, _DtCmsSender **source)
{
	CSA_return_code	stat, res;
	char		*name;
	_DtCmsSender	*user;
	struct authunix_parms *ucred;
	int		len;
	char		*hostname;
#ifdef SunOS
	struct netconfig *nconf;
	struct nd_hostservlist *hostlist;
#endif

	if (source == NULL)
		return (CSA_E_INVALID_PARAMETER);

	switch (svcrq->rq_cred.oa_flavor) {
	case AUTH_UNIX:
		if (svcrq->rq_clntcred == NULL)
			return (CSA_E_NO_AUTHORITY);
		else
			break;

	case AUTH_NULL:
	default:
		svcerr_weakauth(svcrq->rq_xprt);
		return (CSA_E_NO_AUTHORITY);
	}

	ucred = (struct authunix_parms *) svcrq->rq_clntcred;

	stat = _GetUserName(ucred->aup_uid, &name);
	if (stat != CSA_SUCCESS && stat != CSA_E_USER_NOT_FOUND)
		return (stat);

	/* get host name of sending machine */
	hostname = NULL;
#ifdef SunOS
	if (nconf = getnetconfigent(svcrq->rq_xprt->xp_netid)) {
		if (netdir_getbyaddr(nconf, &hostlist,
		    svc_getrpccaller(svcrq->rq_xprt)) == 0) {
			res = _GetHostNameFromList(hostlist, &hostname);
			netdir_free(hostlist, ND_HOSTSERVLIST);
			if (res != CSA_SUCCESS) {
				free(name);
				freenetconfigent(nconf);
				return (res);
			}
		}
		freenetconfigent(nconf);
	}
#endif

	if (debug) {
		fprintf(stderr, "sending host: %s (%s)\n",
			(hostname?hostname:"NULL"), ucred->aup_machname);
	}

	if ((user = (_DtCmsSender *)calloc(1, sizeof(_DtCmsSender))) == NULL) {
		free(name);
		if (hostname) free(hostname);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	len = strlen(name)+strlen(hostname?hostname:ucred->aup_machname)+2;
	if ((user->name = malloc(len)) == NULL) {
		free(user);
		free(name);
		if (hostname) free(hostname);
		return (CSA_E_INSUFFICIENT_MEMORY);
	} else {
		sprintf(user->name, "%s@%s", name,
			(hostname ? hostname: ucred->aup_machname));

		if (stat == CSA_SUCCESS)
			user->knownuser = _B_TRUE;
		else
			user->knownuser = _B_FALSE;

		/* restrict access of root from remote machines */
		if (ucred->aup_uid == 0 &&
		    strcmp(ucred->aup_machname, _DtCmGetLocalHost()) == 0)
			user->localroot = _B_TRUE;
		else
			user->localroot = _B_FALSE;

		*source = user;

		free(name);
		if (hostname) free(hostname);
		return (CSA_SUCCESS);
	}
}

/*
 * good format of owner and user assumed:
 * owner: user@host[.domain]
 * user: user@host[.domain]
 * target: name@host[.domain]
 */
extern boolean_t
_DtCmsIsFileOwner(char *owner, char *user, char *target)
{
	char *ptr1, *ptr2, *ptr3;
	boolean_t checkhost = _B_FALSE;

	if (debug) {
		fprintf(stderr, "rpc.cmsd: %s, %s = %s, %s = %s, %s = %s\n",
			"check file owner",
			"owner", ((owner == NULL) ? "NULL" : owner),
			"user", ((user == NULL) ? "NULL" : user),
			"target", ((target == NULL) ? "NULL" : target));
	}

	if (owner == NULL || user == NULL || target == NULL)
		return (_B_FALSE);

	ptr1 = _DtCmGetPrefix(owner, '@');
	ptr2 = _DtCmGetPrefix(user, '@');
	if (strcmp(ptr1, ptr2)) {
		free(ptr1);
		free(ptr2);
		return(_B_FALSE);
	}
	if (strcmp(ptr1, _DTCMS_SUPER_USER) == 0)
		checkhost = _B_TRUE;
	free(ptr1);
	free(ptr2);

	if (checkhost == _B_TRUE) {
		/* if owner is the super-user, need to check the
		 * host name as well, since super-users on different
		 * hosts are treated as different users
		 */
		char	*host1, *host2;

		ptr1 = strchr(owner, '@');
		if (ptr1 == NULL)
			host1 = _DtCmGetLocalHost();
		else
			host1 = _DtCmGetPrefix(++ptr1, '.');

		ptr2 = strchr(user, '@');
		host2 = _DtCmGetPrefix(++ptr2, '.');

		if (strcmp(host1, host2)) {
			if (ptr1) free(host1);
			free(host2);
			return (_B_FALSE);
		}
		if (ptr1) free(host1);
		free(host2);
	}

	/* check domain if domain info is available */

	ptr1 = strchr(user, '@');
	ptr1 = strchr(ptr1, '.');
	ptr2 = strchr(target, '@');
	if (ptr2)
		ptr3 = strchr(ptr2, '.');
	else
		ptr3 = NULL;

	if (ptr1 == NULL || ptr3 == NULL)
		/* assume that user is in the local domain */
		return _B_TRUE;
	else
		return(_DtCmIsSamePath(++ptr1, ++ptr3));
}

extern void
_DtCmsShowAccessList(Access_Entry_4 *l)
{
	while (l!=NULL) {
		fprintf(stderr, "Access: %s(%c%c%c)\n", l->who,
			l->access_type & access_read_4   ? 'r' : '_',
			l->access_type & access_write_4  ? 'w' : '_',
			l->access_type & access_delete_4 ? 'd' : '_');
		l = l->next;
	}
}

extern Access_Entry_4 *
_DtCmsCalendarAccessList(_DtCmsCalendar *cal)
{
	CSA_flags	world = access_none_4;
	Access_Entry_4	*a;
	Access_Entry_4	*l = NULL;

	l = combine_access_list(cal->r_access, l, access_read_4, &world);
	l = combine_access_list(cal->w_access, l, access_write_4, &world);
	l = combine_access_list(cal->d_access, l, access_delete_4, &world);
	l = combine_access_list(cal->x_access, l, access_exec_4, &world);

	/* WORLD exists in one of the lists, add her to the combined list. */
	if (world != access_none_4)
	{
		a = _DtCm_make_access_entry4(WORLD, world);
		a->next = l;
		l = a;
	}
	return (l);
}

/*
 * the user can view the entry if it has OWNER rights,
 * the appropriate VIEW rights or he is the organizer of
 * the entry and has ORGANIZER rights.
 *
 * if we are to simulate OpenWindows behavior (i.e. strict_csa_model is false)
 * allow users who have only CSA_VIEW_PUBLIC_ENTRIES access
 * allow users who have no view access to CSA_CLASS_CONFIDENTIAL entries
 * to see the time of the CSA_CLASS_CONFIDENTIAL entries (actually we only
 * change the content of the summary attribute to "Appointment" for events,
 * "To Do" for todos, "Memo" for memos and "Calendar Entry" for
 * other entry types.
 */
extern CSA_return_code
_DtCmsCheckViewAccess(
	char		*user,
	CSA_flags	access,
	cms_entry	*eptr,
	boolean_t	*timeonly)
{
	CSA_flags	need;
	cms_attribute_value *oval, *sval;

	*timeonly = _B_FALSE;

	need = (_DtCmsClassToViewAccess(eptr)) | CSA_OWNER_RIGHTS;
	if (access & need) {
		return (CSA_SUCCESS);
	} else {
		oval = eptr->attrs[CSA_ENTRY_ATTR_ORGANIZER_I].value;
		sval = eptr->attrs[CSA_ENTRY_ATTR_SPONSOR_I].value;
		if (((access & CSA_ORGANIZER_RIGHTS) &&
		    _DtCmIsSameUser(user, oval->item.calendar_user_value)) ||
		    ((access & CSA_SPONSOR_RIGHTS) && sval &&
		    _DtCmIsSameUser(user, sval->item.calendar_user_value))) {
			return (CSA_SUCCESS);
		} else if (strict_csa_model == _B_FALSE &&
		    (need & CSA_VIEW_CONFIDENTIAL_ENTRIES) &&
		    (access & CSA_VIEW_PUBLIC_ENTRIES)) {
			*timeonly = _B_TRUE;
			return (CSA_SUCCESS);
		} else {
			return (CSA_E_NO_AUTHORITY);
		}
	}
}

extern CSA_return_code
_DtCmsCheckChangeAccess(char *user, CSA_flags access, cms_entry *eptr)
{
	CSA_flags	need;
	cms_attribute_value *oval, *sval;

	need = (_DtCmsClassToChangeAccess(eptr)) | CSA_OWNER_RIGHTS;

	if (access & need) {
		return (CSA_SUCCESS);
	} else {
		oval = eptr->attrs[CSA_ENTRY_ATTR_ORGANIZER_I].value;
		sval = eptr->attrs[CSA_ENTRY_ATTR_SPONSOR_I].value;
		if (((access & CSA_ORGANIZER_RIGHTS) &&
		    _DtCmIsSameUser(user, oval->item.calendar_user_value)) ||
		    ((access & CSA_SPONSOR_RIGHTS) && sval &&
		    _DtCmIsSameUser(user, sval->item.calendar_user_value)))
			return (CSA_SUCCESS);
		else
			return (CSA_E_NO_AUTHORITY);
	}
}

extern CSA_flags
_DtCmsClassToViewAccess(cms_entry *entry)
{
	switch (entry->attrs[CSA_ENTRY_ATTR_CLASSIFICATION_I].value->\
		item.uint32_value)
	{
	case CSA_CLASS_PUBLIC:
		return (CSA_VIEW_PUBLIC_ENTRIES);
	case CSA_CLASS_PRIVATE:
		return (CSA_VIEW_PRIVATE_ENTRIES);
	case CSA_CLASS_CONFIDENTIAL:
		return (CSA_VIEW_CONFIDENTIAL_ENTRIES);
	}
}

extern CSA_flags
_DtCmsClassToInsertAccess(cms_entry *entry)
{
	switch (entry->attrs[CSA_ENTRY_ATTR_CLASSIFICATION_I].value->\
		item.uint32_value)
	{
	case CSA_CLASS_PUBLIC:
		return (CSA_INSERT_PUBLIC_ENTRIES);
	case CSA_CLASS_PRIVATE:
		return (CSA_INSERT_PRIVATE_ENTRIES);
	case CSA_CLASS_CONFIDENTIAL:
		return (CSA_INSERT_CONFIDENTIAL_ENTRIES);
	}
}

extern CSA_flags
_DtCmsClassToChangeAccess(cms_entry *entry)
{
	switch (entry->attrs[CSA_ENTRY_ATTR_CLASSIFICATION_I].value->\
		item.uint32_value)
	{
	case CSA_CLASS_PUBLIC:
		return (CSA_CHANGE_PUBLIC_ENTRIES);
	case CSA_CLASS_PRIVATE:
		return (CSA_CHANGE_PRIVATE_ENTRIES);
	case CSA_CLASS_CONFIDENTIAL:
		return (CSA_CHANGE_CONFIDENTIAL_ENTRIES);
	}
}

extern CSA_return_code
_DtCmsSetAccessControlList(char *value, boolean_t setTrusted)
{
	_UserInfo	*uptr, *head = NULL;
	char		*myvalue, *nextone, *ptr;

	ptr = myvalue = strdup(value);
	if (debug)
		fprintf(stderr, "config value = '%s'\n",
			(value ? value : "NULL"));

	while (nextone = _DtCmsGetNextWord(&ptr)) {
		if (debug)
			fprintf(stderr, "next word = %s\n", nextone);

		if (uptr = _GetUserInfo(nextone))
			head = _PutUserInfoInOrder(head, uptr);
		else {
			_FreeAccessControlList(head);
			free(myvalue);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
	}

	if (setTrusted) {
#ifdef MT
	        pthread_mutex_lock(&trusted_lock);
#endif
                _PrintList(_DtCmsTrusted, setTrusted);
		_DtCmsTrusted.set = _B_TRUE;
		_FreeAccessControlList(_DtCmsTrusted.acl);
		_DtCmsTrusted.acl = head;
		_PrintList(_DtCmsTrusted, setTrusted);
#ifdef MT
	        pthread_mutex_unlock(&trusted_lock);
#endif
	} else {
#ifdef MT
	        pthread_mutex_lock(&trusted_lock);
#endif
		_PrintList(_DtCmsUntrusted, setTrusted);
		_DtCmsUntrusted.set = _B_TRUE;
		_FreeAccessControlList(_DtCmsUntrusted.acl);
		_DtCmsUntrusted.acl = head;
		_PrintList(_DtCmsUntrusted, setTrusted);
#ifdef MT
		pthread_mutex_unlock(&trusted_lock);
#endif
	}

	free(myvalue);
	return (CSA_SUCCESS);
}

/*
 * By default, anybody can create calendars on the local host (i.e.,
 * no trusted or untrusted options specified in /etc/cmsd.conf).
 * Otherwise, the user has to be in the trusted list and also not
 * in the untrusted list.
 * The only exception is that the local super user always has full
 * access.
 */
extern CSA_return_code
_DtCmsCheckCreateAccess(_DtCmsSender *user)
{
	if (user->knownuser == _B_FALSE)
		return (CSA_E_NO_AUTHORITY);

#ifdef MT
	pthread_mutex_lock(&trusted_lock);
	pthread_mutex_lock(&untrusted_lock);
#endif
	if (user->localroot == _B_TRUE ||
	    (_DtCmsTrusted.set == _B_FALSE && _DtCmsUntrusted.set == _B_FALSE)) {
#ifdef MT
	        pthread_mutex_unlock(&trusted_lock);
	        pthread_mutex_unlock(&untrusted_lock);
#endif
		return (CSA_SUCCESS);
	}

	if (_InList(_DtCmsTrusted.acl, user) &&
	    !_InList(_DtCmsUntrusted.acl, user)) {
#ifdef MT
	        pthread_mutex_unlock(&trusted_lock);
	        pthread_mutex_unlock(&untrusted_lock);
#endif
		return (CSA_SUCCESS);
	}
	else {
#ifdef MT
	        pthread_mutex_unlock(&trusted_lock);
	        pthread_mutex_unlock(&untrusted_lock);
#endif
		return (CSA_E_NO_AUTHORITY);
	}
}

extern void
_DtCmsUnsetAccessControlList()
{
#ifdef MT
	pthread_mutex_lock(&untrusted_lock);
#endif
	_FreeAccessControlList(_DtCmsUntrusted.acl);
	_DtCmsUntrusted.set = _B_FALSE;
	_DtCmsUntrusted.acl = NULL;
#ifdef MT
	pthread_mutex_unlock(&untrusted_lock);

	pthread_mutex_lock(&trusted_lock);
#endif
	_FreeAccessControlList(_DtCmsTrusted.acl);
	_DtCmsTrusted.set = _B_FALSE;
	_DtCmsTrusted.acl = NULL;
#ifdef MT
	pthread_mutex_unlock(&trusted_lock);
#endif
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

static boolean_t
_InList(_UserInfo *list, _DtCmsSender *user)
{
	_UserInfo	*ptr;
	char		*uname, *hname, *hptr;
	int		res;
	boolean_t	found = _B_FALSE, checkhost = _B_FALSE;

	uname = user->name;
	hname = strchr(uname, '@');
	*hname = NULL;
	hname++;

	for (ptr = list; ptr != NULL; ptr = ptr->next) {
		if (ptr->user == NULL) {
			/* any user from this host */
			checkhost = TRUE;
		} else if ((res = strcmp(ptr->user, uname)) == 0) {
			/* same user name */
			if (ptr->host == NULL) {
				found = _B_TRUE;
				break;
			} else
				checkhost == _B_TRUE;
		} else if (res > 0)
			break;

		if (checkhost == _B_TRUE) {
			if (hptr = strchr(hname, '.'))
				found = _DtCmIsSamePath(hname, ptr->host);
			else
				found = _DtCmIsSamePath(ptr->host, hname);
			if (found == _B_TRUE)
				break;
		}
		checkhost = _B_FALSE;
	}

	*(--hname) = '@';
	return (found);
}

static void
_PrintList(_DtCmsAccessList accessinfo, boolean_t trusted)
{
	_UserInfo	*ptr;

	if (!debug)
		return;

	fprintf(stderr, "info about %strusted list:\n", (trusted ? "" : "un"));
	fprintf(stderr, "The list is %s set.\n", (accessinfo.set ? "" : "not"));

	if (!accessinfo.set)
		return;

	if (accessinfo.acl == NULL) {
		fprintf(stderr, "The list is empty.\n");
		return;
	}

	for (ptr = accessinfo.acl; ptr != NULL; ptr = ptr->next) {
		fprintf(stderr, "user = %s,\thost = %s\n",
			(ptr->user ? ptr->user : "NULL"),
			(ptr->host ? ptr->host : "NULL"));
	}
}

static void
_FreeAccessControlList(_UserInfo *ptr)
{
	_UserInfo *next;

	while (ptr != NULL) {
		next = ptr->next;

		if (ptr->user)
			free(ptr->user);
		if (ptr->host)
			free(ptr->host);
		free(ptr);
		ptr = next;
	}
}

static _UserInfo *
_GetUserInfo(char *userinfo)
{
	_UserInfo	*uptr;
	char		*ptr;

	if ((uptr = (_UserInfo *)calloc(1, sizeof(_UserInfo))) == NULL)
		return (NULL);

	if (ptr = strchr(userinfo, '@'))
		*ptr = NULL;

	/* get user name */
	if (ptr == NULL || (ptr && ptr != userinfo)) {
		if ((uptr->user = strdup(userinfo)) == NULL) {
			if (ptr) *ptr = '@';
			free(uptr);
			return (NULL);
		}
	}

	if (ptr) {
		*ptr = '@';
		if (*(++ptr) != NULL) {
			if ((uptr->host = strdup(ptr)) == NULL) {
				if (uptr->user) free(uptr->user);
				free(uptr);
				return (NULL);
			}
		}
	}

	return (uptr);
}

/*
 * The list is ordered alphabetically by users first and then by hosts
 */
static _UserInfo *
_PutUserInfoInOrder(_UserInfo *head, _UserInfo *new)
{
	_UserInfo	*ptr, *pptr;
	int		res1, res2;

	for (pptr = ptr = head; ptr != NULL; pptr = ptr, ptr = ptr->next) {
		res1 = 0;
		if ((new->user == NULL && ptr->user == NULL) ||
		    (ptr->user && new->user &&
		     ((res1 = strcmp(ptr->user, new->user)) == 0))) {
			if (ptr->host == NULL) {
				if (new->host == NULL)
					return head;
				/* else continue */
			} else if (new->host == NULL) {
				new->next = ptr;
				if (pptr == head)
					head = new;
				else
					pptr->next = new;
				return head;
			} else if ((res2 = strcmp(ptr->host, new->host)) == 0) {
				return head;
			} else if (res2 > 0)
				break;
		} else if ((ptr->user && new->user == NULL) || res1 > 0)
			break;
	}

	if (ptr == NULL) {
		if (head == NULL)
			head = new;
		else
			pptr->next = new;
	} else {
		new->next = ptr;
		if (ptr == head)
			head = new;
		else
			ptr->next = new;
	}

	return head;
}

static Uname_cache *
in_u_cache(int uid)
{
	int	cache = NUM_CACHE;
	Uname_cache *p_prev;
	Uname_cache *p_cache;

	p_prev = NULL;
	p_cache = ucache_list;
	while (p_cache != NULL)
	{
		if (p_cache->uid == uid)
			return (p_cache);
		if (--cache < 0)
		{
			/* Assume that the cache size is at least 1 */
			p_prev->next = p_cache->next;
			free (p_cache->name);
			free (p_cache);
			p_cache = p_prev->next;
		}
		else
		{
			p_prev = p_cache;
			p_cache = p_cache->next;
		}
	}
	return (NULL);
}

static CSA_return_code
_GetUserName(int uid, char **name)
{
	char		buff[16];
	Uname_cache	*ucache;
	struct passwd	*pw;
#ifdef SunOS
	struct passwd	pwstruct;
	char		buf[BUFSIZ];
#endif

	/* Put a lock around the ucache list. */
#ifdef MT
	pthread_mutex_lock(&ucache_lock);
#endif

	if ((ucache = in_u_cache(uid)) == NULL) {
#ifdef SunOS
	  	int	status;
		if ((status=getpwuid_r(uid, &pwstruct, buf, BUFSIZ-1, &pw))>0)
#else
		if ((pw = getpwuid(uid)) == NULL)
#endif
		{
			/* Can't map uid to name.  Don't cache the uid. */
			sprintf (buff, "%d", uid);
			if ((*name = strdup(buff)) == NULL) {
#ifdef MT
			  	pthread_mutex_unlock(&ucache_lock);
#endif
				return (CSA_E_INSUFFICIENT_MEMORY);
			}
			else {
#ifdef MT
			        pthread_mutex_unlock(&ucache_lock);
#endif
				return (CSA_E_USER_NOT_FOUND);
			}
		}

		if ((ucache = (Uname_cache *)malloc(sizeof(Uname_cache)))
		    == NULL) {
#ifdef MT
		        pthread_mutex_unlock(&ucache_lock);
#endif
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		if ((ucache->name = strdup(pw->pw_name)) == NULL) {
			free(ucache);
#ifdef MT
			pthread_mutex_unlock(&ucache_lock);
#endif
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		ucache->uid = uid;
		ucache->next = ucache_list;
		ucache_list = ucache;
	}

	if ((*name = strdup(ucache->name)) == NULL) {
#ifdef MT
	        pthread_mutex_unlock(&ucache_lock);
#endif
		return (CSA_E_INSUFFICIENT_MEMORY);
	}
	else {
#ifdef MT
	        pthread_mutex_unlock(&ucache_lock);
#endif
		return (CSA_SUCCESS);
	}
}

static Access_Entry_4 *
in_access_list(Access_Entry_4 *l, char *s)
{
	if (l==NULL || s==NULL) return(NULL);
	while(l != NULL) {
		/* only for combining lists, not for authentication */
		if (strcmp(l->who, s) == 0)
			break;
		l = l->next;
	}
	return(l);
}

static Access_Entry_4 *
combine_access_list(
	Access_Entry_4 *p_list, 
	Access_Entry_4 *p_head, 
	CSA_flags type, 
	CSA_flags *p_world)
{
	Access_Entry_4	*a;
	Access_Entry_4	*h = p_head;

	while (p_list != NULL)
	{
		/* Delay to put the WORLD into the combined list because 
		 * in_access_list() may return wrong result.
		 */
		if (strcmp (p_list->who, WORLD) == 0)
			*p_world |= type;
		else
		{
			/* The user is not in the combined list, add to list. */
			if ((a = in_access_list (h, p_list->who)) == NULL)
			{
				a = _DtCm_make_access_entry4(p_list->who, type);
				a->next = p_head;
				p_head = a;
			}
			a->access_type |= type;
		}
		p_list = p_list->next;
	}
	return (p_head);
}

static CSA_return_code
_GetV4AccessRights(
	_DtCmsCalendar	*cal,
	char		*target,
	_DtCmsSender	*sender,
	CSA_flags	*access)
{
	CSA_flags	userAtAccess = 0, userAccess = 0, worldAccess = 0;
	boolean_t	foundUser = _B_FALSE, foundUserAt = _B_FALSE;
	char		*username, *ownername, *ptr;
	Access_Entry_4	*alist;

	/* check for local root and give root user full access */
	if (sender->localroot == _B_TRUE) {
		*access = CSA_OWNER_RIGHTS;
		return (CSA_SUCCESS);
	}

	/* first check to see if the user is the owner of the calendar */
	if (sender->knownuser == _B_TRUE &&
	    strcmp(cal->calendar, _DTCMS_SUPER_USER) &&
	    cal->checkowner == _B_FALSE &&
	    (_DtCmsIsFileOwner(cal->owner, sender->name, target) == _B_TRUE)) {
		*access = CSA_OWNER_RIGHTS;
		return (CSA_SUCCESS);
	}

	/* go through the whole list in case there's more than one
	 * entry for the user
	 */
	for (alist = cal->alist; alist != NULL; alist = alist->next) {
		if (strcmp(alist->who, WORLD) == 0)
			worldAccess = alist->access_type;
		else if (_DtCmIsSameUser(sender->name, alist->who) == _B_TRUE) {
			foundUser = _B_TRUE;

			if (ptr = strchr(alist->who, '@')) {
				foundUserAt = _B_TRUE;
				userAtAccess = alist->access_type;
			} else
				userAccess = alist->access_type;
		}
	}

	if (foundUser == _B_FALSE)
		*access = worldAccess;
	else if (foundUserAt == _B_TRUE)
		*access = userAtAccess;
	else
		*access = userAccess;

	/* if user is the same as the owner user name and he/she
	 * has all the three access rights, return CSA_OWNER_RIGHTS
	 */
	username = _DtCmGetPrefix(sender->name, '@');
	ownername = _DtCmGetPrefix(cal->owner, '@');
	if ((strcmp(username, ownername) == 0) &&
	    ((*access) & access_read_4) &&
	    ((*access) & access_write_4) &&
	    ((*access) & access_delete_4)) {
		*access = CSA_OWNER_RIGHTS;
	}
	free(ownername);
	free(username);

	return (CSA_SUCCESS);
}

static CSA_return_code
_GetV5AccessRights(
	_DtCmsCalendar	*cal,
	char		*target,
	_DtCmsSender	*sender,
	CSA_flags	*access)
{
	CSA_flags		userAtAccess = 0, userAccess = 0,
				worldAccess = 0;
	boolean_t		foundUser = _B_FALSE, foundUserAt = _B_FALSE;
	cms_access_entry	*alist;
	char			*ptr;

	/* check for root and give root user full access */
	if (sender->localroot == _B_TRUE) {
		*access = CSA_OWNER_RIGHTS;
		return (CSA_SUCCESS);
	}

	/* first check to see if the user is the owner of the calendar */
	if (sender->knownuser == _B_TRUE &&
	    strcmp(cal->calendar, _DTCMS_SUPER_USER) &&
	    cal->checkowner == _B_FALSE &&
	    (_DtCmsIsFileOwner(cal->owner, sender->name, target) == _B_TRUE)) {
		*access = CSA_OWNER_RIGHTS;
		return (CSA_SUCCESS);
	}

	/* go through the whole list in case there's more than one
	 * entry for the user
	 */
	alist = cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value->\
			item.access_list_value;
	for (; alist != NULL; alist = alist->next) {
		if (strcmp(alist->user, WORLD) == 0)
			worldAccess = alist->rights;
		else if (_DtCmIsSameUser(sender->name, alist->user)) {
			foundUser = _B_TRUE;

			if (ptr = strchr(alist->user, '@')) {
				foundUserAt = _B_TRUE;
				userAtAccess = alist->rights;
			} else
				userAccess = alist->rights;
		}
	}

	if (foundUser == _B_FALSE)
		*access = worldAccess;
	else if (foundUserAt == _B_TRUE)
		*access = userAtAccess;
	else
		*access = userAccess;

	return (CSA_SUCCESS);
}

#ifdef SunOS
static CSA_return_code
_GetHostNameFromList(struct nd_hostservlist *hostlist, char **hostname)
{
        if (hostlist->h_cnt > 0) {
		/* get the first one */
		if ((*hostname) = strdup(hostlist->h_hostservs[0].h_host))
			return (CSA_SUCCESS);
		else
			return (CSA_E_INSUFFICIENT_MEMORY);
	} else {
		*hostname = NULL;
		return (CSA_SUCCESS);
	}
}
#endif

