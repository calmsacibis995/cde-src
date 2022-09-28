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
#pragma ident " @(#)DateFieldUiItem.C	1.5 22 Feb 1995 "
#endif

#include <nl_types.h>
#include <DtMail/options_util.h>
#include <DtMail/PropUi.hh>
#include <DtMail/DateFieldUiItem.hh>
#include <DtMail/OptCmd.h>
#include <DtMail/IO.hh>
#include "RoamApp.h"
#include <SDtMail/SystemUtility.hh>
extern nl_catd DT_catd;

// DateFieldUiItem::DateFieldUiItem
// DateFieldUiItem ctor
////////////////////////////////////////////////////////////////////

DateFieldUiItem::DateFieldUiItem(Widget w, int source, char *search_key):PropUiItem(w, source, search_key)
{

  options_field_init(w, &(this->dirty_bit));

}

// DateFieldUiItem::writeFromUiToSource()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void DateFieldUiItem::writeFromUiToSource()
{
  char 	*datefield_value, *parse_error;
  Widget w = this->getWidget();

  datefield_value = options_field_get_value(w);

  if (datefield_value && *datefield_value) {
  	char time_str[15];
  	struct tm tm;

	// Convert the date to a tm struct. %x says assume the format for the
    	// current locale. Only write it out if in valid format. We dont do 
	// an error dialog here. If they turn on vacation we popup error dialog.
	// strptime returns null pointer if successful!
  	if ((parse_error = strptime (datefield_value, catgets(DT_catd, 1, 239, "%x"), &tm)) == NULL || *parse_error) {
		// Set an error flag so we can terminate start vacation
		const char *pkey = prop_source->getKey();
		OptCmd *oc = (OptCmd*)theRoamApp.mailOptions();
        	if (pkey != NULL)
			if (strcmp (pkey, "vacationstartdate") == 0) 
				oc->_startDateFormatError = Sdm_True;
			else if (strcmp (pkey, "vacationenddate") == 0) 
				oc->_endDateFormatError = Sdm_True;
		prop_source->setValue("");
 	}
	else {
		// Convert the tm struct to the C locale format for storing.
		// We always write the C locale format so we know how to convert back.
		SdmSystemUtility::SafeStrftime (time_str, 15, catgets(DT_catd, 1, 238, "%m %d %Y"), &tm);
		prop_source->setValue(time_str);
	}
  }
  else prop_source->setValue("");
  if (datefield_value) free(datefield_value);
}

// DateFieldUiItem::writeFromSourceToUi()
// Takes values in the UI and puts them into the source DB
///////////////////////////////////////////////////////////////////
void DateFieldUiItem::writeFromSourceToUi()
{
  char *value;
  Widget w = this->getWidget();
  char time_str[15];
  time_t nowtick;
  struct tm tm, *now;

  time_str[0] = NULL;
  value = (char *)prop_source->getValue();
  if (value && *value) {
  	strptime (value, catgets(DT_catd, 1, 238, "%m %d %Y"), &tm);
  	SdmSystemUtility::SafeStrftime (time_str, 15, 
		catgets(DT_catd, 1, 239, "%x"),	&tm);
  }
  else {
	// If no value stored, default to today for start vacation date.
	const char *pkey = prop_source->getKey();
        if (pkey != NULL && strcmp (pkey, "vacationstartdate") == 0) {
		nowtick = time((time_t *) 0);
        	now = localtime(&nowtick);
		SdmSystemUtility::SafeStrftime (time_str, 15, catgets(DT_catd, 1, 239, "%x"), now);
        }
  }
  if (value) free(value);
  options_field_set_value(w, time_str, this->dirty_bit);
}
