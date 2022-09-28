/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the SdmTime class
// It is provided as a simple timestamp object.  It's resolution
// is in seconds.  Operators to compare timestamps are provided.

#ifndef _SDM_TIME_H
#define _SDM_TIME_H

#pragma ident "@(#)Time.hh	1.7 96/05/13 SMI"

#include <sys/types.h>
#include <time.h>
#include <SDtMail/Sdtmail.hh>

class SdmTime {

public:
  SdmTime();
  SdmTime (const SdmTime& t);
  ~SdmTime();

  // equal operator
  SdmTime operator= (const SdmTime &t);

  // comparison operators
  SdmBoolean operator== (const SdmTime &t) const;
  SdmBoolean operator< (const SdmTime &t) const;
  SdmBoolean operator> (const SdmTime &t) const;

  // accessor
  time_t GetTime (void);

private:
  time_t the_time;		// time in seconds since dawn of time
  
};

#endif

