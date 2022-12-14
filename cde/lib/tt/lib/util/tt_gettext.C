//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * tt_gettext.cc
 *
 * Copyright (c) 1988, 1990 by Sun Microsystems, Inc.
 *
 */

#include <stdlib.h>
#include "util/tt_string.h"
#include "tt_options.h"

#if defined(OPT_DGETTEXT)
#	if defined(OPT_BUG_SUNOS_4)
		extern "C" {
			char *bindtextdomain(const char *, const char *);
			char *dgettext(const char *, const char *);
		}
#	else
#		include <libintl.h>
#	endif
#elif defined(OPT_CATGETS)
#	include <nl_types.h>
#	if !defined(NL_CAT_LOCALE)
#		define NL_CAT_LOCALE 0
#	endif
#endif

static const char tt_err_domain[] = "SUNW_TOOLTALK";

#if defined(OPT_DGETTEXT)
/*
 * This routine wraps around the code to set the domain and call
 * dgettext, so we don't have to replicate this logic all over
 * the place.
 */
char *
_tt_gettext(
	const char *msgid
)
{
	static int bindtextdomain_done = 0;

	if (!bindtextdomain_done) {
		char *envvar;
		_Tt_string path;

		// For development purposes, find
		// the catalogs under $TOOLTALKHOME/locale.
		// When running normally, as part of Open Windows,
		// find the catalogs under $OPENWINHOME/lib/locale.
		// Otherwise try /usr/openwin/lib/locale, the standard
		// location, as a last resort.

		if(envvar = getenv("TOOLTALKHOME")) {
			path = envvar;
			path = path.cat("/locale");
		} else if (envvar = getenv("OPENWINHOME")) {
			path = envvar;
			path = path.cat("/lib/locale");
		} else {
			path = "/usr/openwin/lib/locale";
		}
		bindtextdomain(tt_err_domain, path);
		bindtextdomain_done = 1;
	}
	return dgettext(tt_err_domain,msgid);
}
#endif // OPT_DGETTEXT

#if defined(OPT_CATGETS)
static char *
_tt__catgets(
	int		set_num,
	int		msg_num,
	const char     *default_string
)
{
	static nl_catd catalog = 0;
	if (catalog == 0) {
		catalog = catopen(
#if defined(OPT_BUG_AIX)
			(char *)
#endif
				  tt_err_domain, NL_CAT_LOCALE );
	}
	return catgets( catalog, set_num, msg_num,
#if defined(OPT_BUG_SUNOS_5) || defined(OPT_BUG_AIX)
				(char *)
#endif
			default_string );
}
#endif // OPT_CATGETS

char *
_tt_catgets(
	int		set_num,
	int		msg_num,
	const char     *default_string
)
{
#if defined(OPT_DGETTEXT)
	return _tt_gettext( default_string );
#elif defined(OPT_CATGETS)
	return _tt__catgets( set_num, msg_num, default_string );
#else
	return default_string;
#endif
}
