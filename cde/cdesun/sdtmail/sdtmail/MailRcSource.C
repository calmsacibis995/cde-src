/*
 *+SNOTICE
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident " @(#)MailRcSource.C	1.40 11/15/96 "
#endif

#include <string.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/MailRc.hh>
#include <DtMail/PropSource.hh>

static PropDefaults default_props[] ={
{ "retrieveinterval",		"300" },
{ "bell",			"1" },
{ "flash",			"0" },
{ "headerlines",		"15" },
{ "showto",			"f"  },
{ "showmsgnum",			"f"  },
{ "popuplines",			"24" },
{ "toolcols",			"80" },
{ "indentprefix",		">" },
{ "hideattachments",		"f" },
{ "folder",			"$HOME/Mail" },
{ "imapfolder",			"$HOME/Mail" },
{ "imapfolderserver",		"" },
{ "imapinboxserver",		"" },
{ "cachedfilemenusize",		"10" },
{ "dontdisplaycachedfiles",	"f" },
{ "record",			"sent.mail" },
{ "dontlogmessages",		"f" },
{ "keepdeleted",		"f"  },
{ "quietdelete",		"f"  },
{ "expert",			"f"   },
{ "cdenotooltalklock",		"f"  },
{ "keep",                       "f" },
{ "allnet",			"f"  },
{ "metoo",			"f"  },
{ "usealternates",              "f"  },
{ "deaddir",                	"$HOME/DeadLetters"  },
{ "saveinterval",             	"30" },
{ "dontautosave",		"f" },
{ "dontincludesignature",	"f" },
{ "realsound",             	"" },
{ "msgsizelimit",               "1000" },
{ "displaymsgsizewarning",	"f" },
{ "composewincols",             "80" },
{ "inactivityinterval",         "30" },
{ "signature",                  "$HOME/.signature" },
{ "toolbarcommands",		"delete next previous reply_to_sender print" },
{ "toolbarusetext",		"f" },
{ "vacationreplyinterval",      "7" },
{ "vacationstartdate",          "" },
{ "vacationenddate",            "" },
{ "templatedir",                "$HOME/MailTemplates" },
{ "draftdir",                   "$HOME/MailDrafts" },
{ "dontsavemimemessageformat",	"f" },
{ "dontsendmimeformat",		"f" },
{ "smtpmailserver",		"" },

{ NULL,				NULL }
};

const char *
PropSource::getDefaultValue(void)
{
	for (PropDefaults * df = default_props; df->key; df++) {
		if (strcasecmp(key, df->key) == 0) {
			return(df->value);
		}
	}

	return(" ");
}

// MailRcSource::getValue
// gets the value from the mailrc hash table and returns it
/////////////////////////////////////////////////////////////////
const char *MailRcSource::getValue()
{
	char *value = NULL;
	SdmError error;

	error.Reset();

	mail_rc->GetValue(error, (const char *) key, &value);

	if ( value == NULL || (error != Sdm_EC_Success)) {
		value = strdup(getDefaultValue());
	}

	return value;
}

// MailRcSource::setValue
// Sets the passed value into the mailrc hash tables
/////////////////////////////////////////////////////////////////
void MailRcSource::setValue(const char *value)
{
  SdmError error;

  error.Reset();

  const char * d_value = getDefaultValue();

  if (strcmp(d_value, value) != 0) 
      mail_rc->SetValue(error, key, value);
  else 
      mail_rc->RemoveValue(error, key);
}

#ifdef DEAD_WOOD
void MailRcSource::updateStore(void)
{
    DtMailEnv error;
    mail_rc->update(error);
}
#endif /* DEAD_WOOD */
