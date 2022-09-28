/* $XConsortium: DirIterator.h /main/cde1_maint/1 1995/07/18 01:29:54 drk $ */
/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/

#include "cstring.h"
#include <dirent.h>

class DirectoryIterator {
 public:
  DirectoryIterator(const CString &);
  ~DirectoryIterator();

  enum directoryState { good_ = 0, bad_ = 1, done_ = 2 };

  struct dirent * operator()();
  
  int bad()    { return state & bad_; }
  int good()   { return state & good_; }
  int done()   { return state & done_; }

 private:
  DirectoryIterator() {}

  DIR * theDir;
  int   state;
};
