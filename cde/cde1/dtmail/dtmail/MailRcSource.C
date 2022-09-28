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
#pragma ident " @(#)MailRcSource.C	1.28 05/07/96 "
#endif

#include <EUSCompat.h>
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
{ "cachedfilemenusize",		"10" },
{ "dontdisplaycachedfiles",	"f" },
{ "record",			"$HOME/Mail/sent.mail" },
{ "dontlogmessages",		"f" },
{ "keepdeleted",		"f"  },
{ "quietdelete",		"f"  },
{ "expert",			"f"   },
{ "strictmime",                 "f"  },
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
  DtMailEnv error;

  mail_rc->getValue(error, key , &value);

  if (value == NULL || error.isSet()) {
      value = strdup(getDefaultValue());
  }
  
  return value;
}

// MailRcSource::setValue
// Sets the passed value into the mailrc hash tables
/////////////////////////////////////////////////////////////////
void MailRcSource::setValue(char *value)
{
  DtMailEnv error;

  const char * d_value = getDefaultValue();

  if (strcmp(d_value, value)) {
      mail_rc->setValue(error, key , value);
  }
  else {
      mail_rc->removeValue(error, key);
  }

}

#ifdef DEAD_WOOD
void MailRcSource::updateStore(void)
{
    DtMailEnv error;
    mail_rc->update(error);
}
#endif /* DEAD_WOOD */
