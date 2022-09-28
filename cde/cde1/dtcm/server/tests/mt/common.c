/*SNOTICE*****************************************************************
**************************************************************************
*
*                   Common Desktop Environment
* 
* (c) Copyright 1993, 1994 Hewlett-Packard Company 
* (c) Copyright 1993, 1994 International Business Machines Corp.             
* (c) Copyright 1993, 1994 Sun Microsystems, Inc.
* (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
*                                                                    
* 
*                     RESTRICTED RIGHTS LEGEND                              
* 
* Use, duplication, or disclosure by the U.S. Government is subject to
* restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
* Technical Data and Computer Software clause in DFARS 252.227-7013.  Rights
* for non-DOD U.S. Government Departments and Agencies are as set forth in
* FAR 52.227-19(c)(1,2).
*
* Hewlett-Packard Company, 3000 Hanover Street, Palo Alto, CA 94304 U.S.A.
* International Business Machines Corp., Route 100, Somers, NY 10589 U.S.A. 
* Sun Microsystems, Inc., 2550 Garcia Avenue, Mountain View, CA 94043 U.S.A.
* Unix System Labs, Inc., 190 River Road, Summit, NJ 07901 U.S.A.
*
*****************************************************************ENOTICE*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/utsname.h>

#include "cm.h"
#include "rtable4.h"
#include "csa.h"

#include "mtcommon.h"
#include "common.h"

/* Define TRUE and FALSE values. */
#ifndef TRUE
#define TRUE    1
#endif
 
#ifndef FALSE
#define FALSE   0
#endif

void 
MtTest_Init_Server()
{
  _DtCm_init_hash();
}

void
Clean_Up_Calendar(char * name, void * rqstp)
{
  cms_remove_args          argument;
  CSA_return_code          result;

  argument.pid = getpid();
  argument.cal = name;
  
  cms_remove_calendar_5_svc(&argument, &result, rqstp);
}

void
Create_Calendar(char * name, void * rqstp)
{
  cms_create_args          argument;
  CSA_return_code          result;

  argument.cal = name;
  argument.char_set = NULL;
  argument.pid = getpid();
  argument.num_attrs = 0;
  argument.attrs = NULL;
  
  cms_create_calendar_5_svc(&argument, &result, rqstp);

  if (result != 0) {
    printf("ERROR: Create_Calendar failed with return code: %d\n",
	   result);
    printf("Create_Calendar: TEST FAILED\n\n");
    exit(0);
  }
}

cms_attribute *
Make_Attrs(int entry_id)
{
  cms_attribute *       attrs = (cms_attribute *)calloc(1, sizeof(cms_attribute) * 6);
  cms_attribute_value * val = (cms_attribute_value *)calloc(1, sizeof(cms_attribute_value) * 6);
  CSA_reminder *	audio = (CSA_reminder *)calloc(1, sizeof(CSA_reminder));
  CSA_reminder *	popup = (CSA_reminder *)calloc(1, sizeof(CSA_reminder));

  attrs[0].name.num = 0;
  attrs[0].name.name = CSA_ENTRY_ATTR_START_DATE;
  attrs[0].value = &val[0];
  val[0].type = CSA_VALUE_DATE_TIME;
  if (entry_id == 1)
    val[0].item.date_time_value = APPOINTMENT_START_TIME1;
  else
    val[0].item.date_time_value = APPOINTMENT_START_TIME2;

  attrs[1].name.num = 0;
  attrs[1].name.name = CSA_ENTRY_ATTR_END_DATE;
  attrs[1].value = &val[1];
  val[1].type = CSA_VALUE_DATE_TIME;
  if (entry_id == 1)
    val[1].item.date_time_value = APPOINTMENT_END_TIME1;
  else 
    val[1].item.date_time_value = APPOINTMENT_END_TIME2;
	
  attrs[2].name.num = 0;
  attrs[2].name.name = CSA_ENTRY_ATTR_SUMMARY;
  attrs[2].value = &val[2];
  val[2].type = CSA_VALUE_STRING;
  val[2].item.string_value = "test appointment";

  attrs[3].name.num = 0;
  attrs[3].name.name = CSA_ENTRY_ATTR_TYPE;
  attrs[3].value = &val[3];
  val[3].type = CSA_VALUE_UINT32;
  val[3].item.sint32_value = CSA_TYPE_EVENT;

  attrs[4].name.num = 0;
  attrs[4].name.name = CSA_ENTRY_ATTR_AUDIO_REMINDER;
  attrs[4].value = &val[4];
  val[4].type = CSA_VALUE_REMINDER;
  val[4].item.reminder_value = audio;
  audio->lead_time = "+PT300S";
  audio->snooze_time = 0;
  audio->repeat_count = 0;
  audio->reminder_data.size = 0;
  audio->reminder_data.data = NULL;

  attrs[5].name.num = 0;
  attrs[5].name.name = CSA_ENTRY_ATTR_POPUP_REMINDER;
  attrs[5].value = &val[5];
  val[5].type = CSA_VALUE_REMINDER;
  val[5].item.reminder_value = popup;
  popup->lead_time = "+PT300S";
  popup->snooze_time = 0;
  popup->repeat_count = 0;
  popup->reminder_data.size = 0;
  popup->reminder_data.data = NULL;

  return(attrs);
}

void
Add_Entry(char * name, void * rqstp, int entry_id)
{
  cms_insert_args       argument;
  cms_entry_res         result;

  argument.cal = name;
  argument.pid = getpid();
  argument.num_attrs = 6;
  argument.attrs = Make_Attrs(entry_id);

  cms_insert_entry_5_svc(&argument, &result, rqstp);

  if (result.stat != 0) {
    printf("ERROR: Add_Entry failed with return code: %d\n",
	   result.stat);
    printf("Add_Entry: TEST FAILED\n\n");
    exit(0);
  }
}

void	
Init_Entry_Attr(int id, cms_attribute *attrs, int type)
{
	int	size;

	/* Get the attribute name. */
	attrs->name.num = 0;
	attrs->name.name = CSA_ENTRY_ATTRIBUTE_NAMES[id];

	/* Allocate memory for the attribute value. */
	size = sizeof(cms_attribute_value);
	attrs->value = (cms_attribute_value *)calloc(1, size);

	/* Initialize the attribute value to null. */
	memset(attrs -> value, NULL, size);

	/* Assign a value type. */
	attrs->value->type = type;
}

void	
Set_Up_Time_Criteria(time_t stime, CSA_enum op1, time_t etime, CSA_enum op2, cms_attribute ** attrs, CSA_enum ** new_ops)
{
	cms_attribute	*match_attr;
	CSA_enum	*ops;
	char		timebuf[BUFSIZ];

	/* set up matching criteria */
	match_attr = (cms_attribute *)calloc(1, sizeof(cms_attribute) * 2);
	ops = (CSA_enum *)calloc(1, sizeof(CSA_enum) * 2);

	_csa_tick_to_iso8601(stime, timebuf);
	Init_Entry_Attr(CSA_ENTRY_ATTR_START_DATE_I, &match_attr[0], CSA_VALUE_DATE_TIME);
	match_attr[0].value->item.date_time_value = strdup(timebuf);
	ops[0] = op1;

	_csa_tick_to_iso8601(etime, timebuf);
	Init_Entry_Attr(CSA_ENTRY_ATTR_START_DATE_I, &match_attr[1], CSA_VALUE_DATE_TIME);
	match_attr[1].value->item.date_time_value = strdup(timebuf);
	ops[1] = op2;

	*attrs = match_attr;
	*new_ops = ops;
}
