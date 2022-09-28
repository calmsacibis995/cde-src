/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm SdmSearch Object Class.

#pragma ident "@(#)DynamicPredicateUtility.cc	1.8 97/06/16 SMI"

#include <PortObjs/DynamicPredicateUtility.hh>
#include <time.h>


static const int gkSecondsPerDay = (60*60*24);


static char *gTheMonths[12]= {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static int gDaysPerMonth[]= {/*dec - dec */
  31, 30, 28, 31, 30, 31, 30, 31, 31, 31, 31, 30, 31
};

static int gLeapYearDaysPerMonth[]= {/*dec - dec */
  31, 30, 29, 31, 30, 31, 30, 31, 31, 31, 31, 30, 31
};

// ************************************************
//
// class SdmDynamicPredicateUtility contains methods
// used to expand dynamic predicates.  All methods
// are static so they can be called directly.
//
// ************************************************


// ** ET
// Below are all the static methods used to define the dynamic predicates
// in the leaves.  They are called in update_dynamic_predicate.
//
SdmErrorCode 
SdmDynamicPredicateUtility::SetToday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  
  time_t currentTime;
  struct tm *localTime;
  struct timeval tp;
  struct timezone tzp;
  char theDate[32];

  memset(&theDate, 0, 32);
  gettimeofday(&tp, &tzp);
  currentTime =  (time_t)tp.tv_sec;
  localTime = localtime(&currentTime);
  sprintf(theDate, "\"%02d-%s-%4d\"", localTime->tm_mday, 
	  gTheMonths[localTime->tm_mon], localTime->tm_year+1900);
  r_value = theDate;
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetYesterday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  
  time_t currentTime;
  struct tm *localTime;
  struct timeval tp;
  struct timezone tzp;
  char theDate[32];

  memset(&theDate, 0, 32);
  gettimeofday(&tp, &tzp);
  currentTime =  (time_t)tp.tv_sec;
  currentTime -= gkSecondsPerDay;
  localTime = localtime(&currentTime);
  sprintf(theDate, "\"%02d-%s-%4d\"", localTime->tm_mday, 
	  gTheMonths[localTime->tm_mon], localTime->tm_year+1900);
  r_value = theDate;
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetLastWeek(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  
  time_t currentTime;
  struct tm *localTime;
  struct timeval tp;
  struct timezone tzp;
  char theDate[32];

  memset(&theDate, 0, 32);
  gettimeofday(&tp, &tzp);
  currentTime =  (time_t)tp.tv_sec;
  localTime = localtime(&currentTime);
  /* Take us to a week ago Sunday */
  currentTime = currentTime - (localTime->tm_wday * gkSecondsPerDay)
    - (7 * gkSecondsPerDay);
  localTime = localtime(&currentTime);
  sprintf(theDate, "\"%02d-%s-%4d\"", localTime->tm_mday, 
	  gTheMonths[localTime->tm_mon], localTime->tm_year+1900);
  r_value = theDate;
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetLastMonth(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  
  time_t currentTime;
  struct tm *localTime;
  struct timeval tp;
  struct timezone tzp;
  char theDate[32];
  int days_last_month;

  memset(&theDate, 0, 32);
  gettimeofday(&tp, &tzp);
  currentTime =  (time_t)tp.tv_sec;
  localTime = localtime(&currentTime);
  /* Take us to the debut of last month */
					/* THe debut of this month */
  currentTime = currentTime - ((localTime->tm_mday - 1)* gkSecondsPerDay); 
  /* Our month tables begin with dec, and tm_mon is 0 - 11, ie, tm_mon + 1
   * indexes THIS MONTH in our tables, alors tm_mon indexes LAST MONTH: */
  days_last_month = ((localTime->tm_year % 4) == 0) ? 
    gLeapYearDaysPerMonth[localTime->tm_mon] : gDaysPerMonth[localTime->tm_mon];
  currentTime -= days_last_month * gkSecondsPerDay;
  localTime = localtime(&currentTime);
  sprintf(theDate, "\"%02d-%s-%4d\"", localTime->tm_mday, 
	  gTheMonths[localTime->tm_mon], localTime->tm_year+1900);
  r_value = theDate;
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetThisMonth(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  
  time_t currentTime;
  struct tm *localTime;
  struct timeval tp;
  struct timezone tzp;
  char theDate[32];
  int days_last_month;

  memset(&theDate, 0, 32);
  gettimeofday(&tp, &tzp);
  currentTime =  (time_t)tp.tv_sec;
  localTime = localtime(&currentTime);
  sprintf(theDate, "\"1-%s-%4d\"", gTheMonths[localTime->tm_mon], localTime->tm_year+1900);
  r_value = theDate;
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetMonth(SdmError &err, SdmString &r_value, char *month_str)
{
  err = Sdm_EC_Success;
  
  time_t currentTime;
  struct tm *localTime;
  struct timeval tp;
  struct timezone tzp;
  char theDate[32];
  int year;

  memset(&theDate, 0, 32);
  gettimeofday(&tp, &tzp);
  currentTime =  (time_t)tp.tv_sec;
  localTime = localtime(&currentTime);
  year = localTime->tm_year;

  sprintf(theDate, "\"1-%s-%4d\"", month_str, year+1900);
  r_value = theDate;
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetJanuary(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "JAN");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetFebruary(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "FEB");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetMarch(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "MAR");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetApril(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "APR");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetMay(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "MAY");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetJune(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "JUN");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetJuly(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "JUL");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetAugust(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "AUG");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetSeptember(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "SEP");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetOctober(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "OCT");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetNovember(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "NOV");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetDecember(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetMonth(err, r_value, "DEC");
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetWeekday(SdmError &err, SdmString &r_value, int dayOfTheWeek)
{
  err = Sdm_EC_Success;
  
  time_t currentTime;
  struct tm *localTime;
  struct timeval tp;
  struct timezone tzp;
  char theDate[32];
  memset(&theDate, 0, 32);

  gettimeofday(&tp, &tzp);
  currentTime =  (time_t)tp.tv_sec;
  localTime = localtime(&currentTime);
  currentTime -= (localTime->tm_wday - dayOfTheWeek) * gkSecondsPerDay;
  localTime = localtime(&currentTime);

  sprintf(theDate, "\"%02d-%s-%4d\"", localTime->tm_mday, gTheMonths[localTime->tm_mon],
	  localTime->tm_year+1900);
  r_value = theDate;
  
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetSunday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetWeekday(err, r_value, 0);
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetMonday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetWeekday(err, r_value, 1);
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetTuesday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetWeekday(err, r_value, 2);
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetWednesday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetWeekday(err, r_value, 3);
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetThursday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetWeekday(err, r_value, 4);
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetFriday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetWeekday(err, r_value, 5);
  return err;
}

SdmErrorCode
SdmDynamicPredicateUtility::SetSaturday(SdmError &err, SdmString &r_value, char *str)
{
  err = Sdm_EC_Success;
  SetWeekday(err, r_value, 6);
  return err;
}



