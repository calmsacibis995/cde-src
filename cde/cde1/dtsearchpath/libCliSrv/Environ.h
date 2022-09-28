/* $XConsortium: Environ.h /main/cde1_maint/4 1995/08/31 11:47:00 barstow $ */
/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/

#ifndef _ENVIRON_H_
#define _ENVIRON_H_

#include "cstring.h"
#include <sys/stat.h>
#include <stdio.h>


class Shell  {
 public:
   // class constructor, destructor
   Shell()  {}
   ~Shell() {}

   // pure virtual function
   virtual void putToEnv( const CString &, const char * ) = 0;
};

class KShell : public Shell {
 public:
   KShell() {}
   ~KShell() {}

   void putToEnv ( const CString & envvar,
                   const char * value )
   {
	CString toPut(envvar);
	toPut += "=";
	toPut += value;

#ifdef IOSTREAMSWORKS
	cout << toPut << ";" << endl;
	cout << "export " << envvar << ";" << endl;
#else
	printf("%s;\n", toPut.data());
	printf("export %s;\n", envvar.data());
#endif
   }
};

class CShell : public Shell {
 public:
   CShell() {}
   ~CShell() {}

   void putToEnv ( const CString & envvar,
                   const char * value )
   {
	CString toPut("setenv ");

	toPut += envvar;
	toPut += " ";
	toPut += value;

#ifdef IOSTREAMSWORKS
	cout << toPut << ";" << endl;
#else
	printf("%s;\n", toPut.data());
#endif
   }
};

class OSEnvironment {
 public:
  OSEnvironment();
  virtual ~OSEnvironment() {}

  virtual int     FileExists (const CString &) const = 0;
  virtual void    MakeDirectory (const CString &, mode_t) = 0;
  virtual CString getEnvironmentVariable (const char *) = 0;
  virtual int     isDirectory (const CString &) = 0;
  virtual int     isFile (const CString &) = 0;
  virtual int     isLink (const CString &) = 0;
  virtual void    changePermissions (const CString &, mode_t) = 0;
  virtual void    changeOwnerGroup (const CString &, const char *, 
				    const char * = 0) = 0;
  virtual void    removeDirectory (const CString &) = 0;
  virtual void    removeFiles (const CString &, const CString &) = 0;
  virtual void    removeFile (const CString &) = 0;
  virtual void    removeDeadLinks (const CString &) = 0;
  virtual void    symbolicLink (const CString &, const CString &) = 0;

  virtual void    setUserId (const char * = 0) = 0;

  CString   MountPoint() const { return dtMountPoint; }
  CString   LocalHost() const  { return localHost; }
  CString   LANG() const       { return lang; }
  CString   MANPATH() const    { return manpath; }
  int       NonDefaultLang()   { return lang != "C"; }
  Shell	    *shell () { return shell_ ; }

 protected:
  CString   dtMountPoint;
  CString   localHost;
  CString   lang;
  CString   manpath;
  Shell     *shell_;
};


class UnixEnvironment : public OSEnvironment {
 public:
  UnixEnvironment();
  virtual ~UnixEnvironment();

  virtual int     FileExists (const CString &) const;
  virtual void    MakeDirectory (const CString &, mode_t);
  virtual CString getEnvironmentVariable (const char *);
  virtual int     isDirectory (const CString &);
  virtual int     isFile (const CString &);
  virtual int     isLink (const CString &);
  virtual void    changePermissions (const CString &, mode_t);
  virtual void    changeOwnerGroup (const CString &, const char *, 
				    const char * = 0);
  virtual void    removeDirectory (const CString &);
  virtual void    removeFiles (const CString &, const CString &);
  virtual void    removeFile (const CString &);
  virtual void    removeDeadLinks (const CString &);
  virtual void    symbolicLink (const CString &, const CString &);
  virtual void    setUserId (const char * = 0);

  CString ksh() {
#if defined(hpV4) || defined(sun)
	return CString("/usr/bin/ksh");
#else
	return CString("/bin/ksh");
#endif
  }

  CString csh() {
#if defined(hpV4) || defined(sun)
	return CString("/usr/bin/csh");
#else
	return CString("/bin/csh");
#endif
  }

 private:
  uid_t  uid_;
  gid_t  gid_;
};

class CDEEnvironment {
 public:
  CDEEnvironment () {}
  CDEEnvironment (CString *, OSEnvironment *);
  ~CDEEnvironment();

  void CreateHomeAppconfigDir();

  CString * DTAPPSP()      const  { return dtspSysApp;   }
  CString * DTUSERAPPSP()  const  { return dtspUserApp;  }
  CString * DTICONSP()     const  { return dtspSysIcon;  }
  CString * DTUSERICONSP() const  { return dtspUserIcon; }
  CString * DTHELPSP()     const  { return dtspSysHelp;  }
  CString * DTUSERHELPSP() const  { return dtspUserHelp; }
  CString * DTDBSP()       const  { return dtspSysDB;    }
  CString * DTUSERDBSP()   const  { return dtspUserDB;   }
  CString * DTMANPATH()    const  { return dtManPath;    }

  void      setDTAPPSP (const CString & sp);

  CString HOME()              const { return theHome;           }
  CString SysAdmConfig()      const { return sysAdmConfig;      }
  CString FactoryInstall()    const { return factoryInstall;    }
  CString FactoryManPath()    const { return factoryManPath;    }
  CString DefaultSearchPath() const { return defaultSearchPath; }
  CString UserHostDir()       const { return userHostDir;       }

  OSEnvironment * OS() const { return os; }

 private:
  CString theHome;
  CString sysAdmConfig;
  CString factoryInstall;
  CString factoryManPath;
  CString defaultSearchPath;
  CString userHostDir;

  CString * dtspSysApp;
  CString * dtspUserApp;
  CString * dtspSysIcon;
  CString * dtspUserIcon;
  CString * dtspSysHelp;
  CString * dtspUserHelp;
  CString * dtspSysDB;
  CString * dtspUserDB;
  CString * dtManPath;

  OSEnvironment * os;
};

#endif
