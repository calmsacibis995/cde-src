/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/
/*******************************************************************
 *
 *  File:              dtappgather.h
 *
 *  Purpose:           Class definitions for the gathering process
 *
 *  Product:           @(#)Common Desktop Environment 1.0          
 *
 *  Revision:          $XConsortium: dtappgather.h /main/cde1_maint/1 1995/07/14 22:59:28 drk $
 *
 ********************************************************************/

#include "Environ.h"

class AppManagerDirectory {
 public:
  AppManagerDirectory() {}
  AppManagerDirectory(CDEEnvironment *, const CString &);
  ~AppManagerDirectory() { delete user_; }

  void GatherAppsFromASearchElement (const CString & path);
  void TraversePath();

  char * operator()() const { return dirname_.data(); }

 private:
  int goodFile (const CString &, const CString &) const;

  CString          dirname_;
  CDEEnvironment * user_;
  CString          appsp_;
  int              langVersionFound;
};


