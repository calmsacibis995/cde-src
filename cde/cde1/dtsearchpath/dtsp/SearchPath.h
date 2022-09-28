/* $XConsortium: SearchPath.h /main/cde1_maint/1 1995/07/18 01:29:41 drk $ */
/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/

#ifndef _SEARCHPATH_H_
#define _SEARCHPATH_H_

#include "Environ.h"
#include "cstring.h"
#include <iostream.h>
#include <stdio.h>

class SearchPath {
 public:
  SearchPath() {}
  SearchPath (CDEEnvironment *, const char *, const char *);
  virtual ~SearchPath();

  virtual void ExportPath();
  virtual void Print();
  virtual void AddPredefinedPath();
  virtual void PrettyPrint (ostream &) const;

  friend ostream & operator<< (ostream &, const SearchPath &);

  const char *  GetEnvVar() const    { return environment_var; }
  CString       GetSearchPath() const { return final_search_path; }

 protected:
  void  AssembleString (const CString &);
  void  NormalizePath();
  void  AddToPath (const CString &);
  void  TraversePath();

  virtual void    MakePath (const CString &) {}
  virtual CString ConstructPath (const CString &, const CString *,
				 unsigned char useTT = 1);

  int     ParseState() const   { return parse_state; }
  CString Separator() const    { return separator; }

  virtual int validSearchPath (const CString &) const;

  void    setSeparator (const char * sep) { separator = sep; }

  CString          search_path;
  CString          norm_search_path;
  CString          final_search_path;
  CDEEnvironment * user;

 private:
  const char *     environment_var;
  int              parse_state;
  CString          save_host;
  CString          separator;
};


class AppSearchPath : public SearchPath {
 public:
  AppSearchPath (CDEEnvironment *, 
		 const char * = "DTAPP",
		 const char * = ",");
  virtual ~AppSearchPath() {}

  void FixUp();

 protected:
  virtual void    MakePath (const CString &);

 private:
};


class IconSearchPath : public SearchPath {
 public:
  IconSearchPath (CDEEnvironment *,
		  const char * = "XMICON",
		  const char * = ".pm",
		  const char * = ".bm",
		  const char * = ":");
  virtual ~IconSearchPath() {}

 protected:
  virtual void    MakePath (const CString &);
  virtual int     validSearchPath (const CString &) const;

 private:
  CString first;
  CString second;
};


class DatabaseSearchPath : public SearchPath {
 public:
  DatabaseSearchPath (CDEEnvironment *,
		      const char * = "DTDATABASE",
		      const char * = ",");
  virtual ~DatabaseSearchPath() {}

 protected:
  virtual void    MakePath (const CString &);
  virtual CString ConstructPath (const CString &, const CString *,
				 unsigned char useTT = 0);

 private:
};


class HelpSearchPath : public SearchPath {
 public:
  HelpSearchPath (CDEEnvironment *, 
		  const char * = "DTHELP",
		  const char * = ":");
  virtual ~HelpSearchPath() {}

 protected:
  virtual void    MakePath (const CString &);
  virtual int     validSearchPath (const CString &) const;

 private:
};


class ManSearchPath : public SearchPath {
 public:
  ManSearchPath (CDEEnvironment *, 
		 const char * = "MANPATH",
		 const char * = ":");
  virtual ~ManSearchPath() {}

  virtual void    ExportPath ();
  virtual void    Print();

  friend ostream & operator<< (ostream &, const ManSearchPath &);

 protected:
  virtual void    MakePath (const CString &);

 private:
};

#endif
