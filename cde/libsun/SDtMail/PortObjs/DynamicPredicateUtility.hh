/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Dynamic Predicate Utility Class.

#ifndef _SDM_DYNAMICPREDICATEUTILITY_H
#define _SDM_DYNAMICPREDICATEUTILITY_H

#pragma ident "@(#)DynamicPredicateUtility.hh	1.5 96/05/13 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>

// SdmDynamicPredicateUtility contains the methods used to expand 
// dynamic predicates in search string (eg. Today, January, Monday, etc.)
//
class SdmDynamicPredicateUtility
{
public:  
  static SdmErrorCode SetToday(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetYesterday(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetLastWeek(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetLastMonth(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetThisMonth(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetJanuary(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetFebruary(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetMarch(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetApril(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetMay(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetJune(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetJuly(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetAugust(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetSeptember(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetOctober(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetNovember(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetDecember(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetSunday(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetMonday(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetTuesday(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetWednesday(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetThursday(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetFriday(SdmError &err, SdmString &r_value, char* field);
  static SdmErrorCode SetSaturday(SdmError &err, SdmString &r_value, char* field);

private: 
  SdmDynamicPredicateUtility() {}
  ~SdmDynamicPredicateUtility() {}

  static SdmErrorCode SetMonth(SdmError &err, SdmString &r_value, char* month_str);
  static SdmErrorCode SetWeekday(SdmError &err, SdmString &r_value, int dayOfTheWeek);

};

#endif
