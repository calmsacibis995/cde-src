/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the SdmTime class.
 
#pragma ident "@(#)Time.cc	1.6 96/05/13 SMI"

#include <SDtMail/Time.hh>

SdmTime::SdmTime ()
{
  time_t tptr;

  // get the current time
  the_time = time (&tptr);

  // if there's an error getting the time, assert for now
  if (the_time == ((time_t) -1))
    assert (0);
}

SdmTime::~SdmTime ()
{
}

// Copy constructor
SdmTime::SdmTime (const SdmTime& t)
{
  the_time = t.the_time;
}
 
// Assignment operator
SdmTime SdmTime::operator=(const SdmTime& t)
{
  // check for assigment to self
  if (this == &t) return *this;

  the_time = t.the_time; 
  return *this;
}
 
// Compare times - equal
SdmBoolean SdmTime::operator== (const SdmTime &t) const
{
  if (the_time == t.the_time)
    return Sdm_True;

  return Sdm_False; 
}
 
// Compare times - equal
SdmBoolean SdmTime::operator< (const SdmTime &t) const
{
  if (the_time < t.the_time)
    return Sdm_True;

  return Sdm_False; 
}
 
// Compare times - equal
SdmBoolean SdmTime::operator> (const SdmTime &t) const
{
  if (the_time > t.the_time)
    return Sdm_True;

  return Sdm_False; 
}

// Return the time
time_t SdmTime::GetTime (void)
{
  return (the_time);
}

