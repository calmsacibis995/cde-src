/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm MailRc Class.

#ifndef _SDM_MAILRC_H
#define _SDM_MAILRC_H

#pragma ident "@(#)MailRc.hh	1.45 97/03/13 SMI"

// Include Files
#include <stdio.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/Vector.hh>

#define SdmMailRc_HashMask	0x1F
#define SdmMailRc_HashSize    	(SdmMailRc_HashMask+1)
#define SdmMailRc_NoFile      20 


class SdmMailRc {

public:

  static SdmMailRc*  GetMailRc();

  SdmBoolean IsValueDefined(const char* var);
  void GetValue(SdmError &, const char * var, char ** value);
  void SetValue(SdmError &, const char * var, const char * value);
  void RemoveValue(SdmError &, const char * var);

  typedef void (*Hm_Callback)(char * key, void * value, void * client_data);

  const char * GetAlias(SdmError &, const char * name);
  void GetAliasList(Hm_Callback stuffing_func, void *client_data);
  void SetAlias(SdmError &, const char * name, const char * value);
  void RemoveAlias(SdmError &, const char * name);
  SdmStringL *GetAliasList();

  SdmBoolean Ignore(SdmError &, const char *name);
  void AddIgnore(SdmError &, const char * name);
  void RemoveIgnore(SdmError &, const char * name);
  SdmStringL *GetIgnoreList();

  char* GetAlternates(SdmError &);
  void SetAlternate(SdmError &, const char * alt);
  void RemoveAlternate(SdmError &, const char * alt);

  void Update(SdmError &);
  
  SdmErrorCode GetParseError()  { return _ParseError; }

  static int Group(char **argv, SdmMailRc *);
  static void WriteGroup(const char *, char **, FILE *);
  static int Unset(char **arglist, SdmMailRc *);
  static void WriteUnset(const char * verbatim, char ** arglist, FILE * outf);
  static int Set(char **arglist, SdmMailRc *);
  static void WriteSet(const char *, char **, FILE *);
  static int Source(char **arglist, SdmMailRc *);
  static void WriteSource(const char *, char **, FILE *);
  static int IfCmd(char **arglist, SdmMailRc *);
  static void WriteIfCmd(const char *, char **, FILE *);
  static int ElseCmd(char **arglist, SdmMailRc *);
  static void WriteElseCmd(const char *, char **, FILE *);
  static int EndIfCmd(char **arglist, SdmMailRc *);
  static void WriteEndIfCmd(const char *, char **, FILE *);
  static int IgnoreField(char **list, SdmMailRc *);
  static void WriteIgnoreField(const char *, char **, FILE *);
  static int ClearAliases(char **list, SdmMailRc *);
  static void WriteClearAliases(const char *, char **, FILE *);
  static int Alternates(char **namelist, SdmMailRc *);
  static void WriteAlternates(const char *, char **, FILE *);

protected:
  virtual ~SdmMailRc();
  SdmMailRc(SdmError& error);

  void UpdateByLine(FILE * in, FILE * out);
  void OutputLine(const char * verbatim, const char * parseable, FILE * out);
  int Commands();
  int Execute(char linebuf[]);
  void Unstack();
  int IsPrefix(char *as1, char *as2);
  void *Lex(char word[]);
  int GetRawList(char line[], char ** argv, int argc);
  void FreeRawList(char **argv);
  char *Mt_Value(char name[]);
  char *Expand(char *);
  int GetFolderDirectory(char *);

  static int Hash(char *name);
  static void FreeValue(char *cp);
  static char *CopyValue(char *str);

private:
  // Methods below this line are not part of the public interface.
  // They must be declared public due to implementation restrictions.
  // DO NOT CALL THESE AS A CLIENT OF THIS CLASS.
  //
  int Load(char *filename);
  void InitializeGlobals();

  static void AddAlias(char *name, char *value);
  static void AddIgnore(char *name);
  static void AddAlternates(char *name);

  static void Mt_Assign(char *name,char * val);
  static int Mt_Deassign(char *s);
  static void Mt_PutHash(char *name, char * val, struct SdmMailRcVariables **hasharray);
  static void Mt_Scan(FILE * outf);

  // internal hash related callbacks
  static void AliasCallback(char * key, void * data, void * client_data);
  static void AlternatesCallback(char * key, void * value, void * client_data);
  static void GroupCallback(char * key, void * value, void * client_data);

#ifdef AUTOFILING
  static void RuleGroupCallback(char * key, void * value, void * client_data);
#endif

  static void IgnoreListCallback(char *key, void *data, void *client_data);
  static void IgnoreFieldCallback(char * key, void * value, void * client_data);

  // hash related methods.
  static void *Hm_Alloc();
  static void *Hm_Test(struct hash **table, const char *key);
  static void Hm_Delete(struct hash **table, const char *key);
  static void Hm_Add(struct hash **table,  const char *key, void *value, int size);
  static void Hm_Mark(struct hash **table, const char * key);
  static int Hm_IsMarked(struct hash **table, const char * key);
  static void Hm_Scan(struct hash **table, Hm_Callback, void * client_data);
  static void FreeHash(struct hash *h);
  static int HashIndex(const char *key);

  static char * GetStringWithEscapeChars(char* value);

  static char *_NullField;
  static SdmBoolean _ClearAliases;
  static SdmBoolean _NoAliasWritten;
  static SdmBoolean _ClearAliasesWritten;

  static struct SdmMailRcGlobals *_Globals;

  static struct SdmMailRcVariables *_Variables[SdmMailRc_HashSize]; /* Pointer to active var list */

private:

  static SdmMailRc*  _GlobalMailRc;
  
  SdmErrorCode			_ParseError;
  FILE *_InputFile;
  int _Sourcing;
  int _ExecuteCondition;
  int  _StackTop;               /* Top of file stack */
  char *_AlternateList;
  char *_ForwardFile;
  char *_MailRcName;
  char *_MailRuleName;

  static struct SdmMailRcVariables *Lookup(char *name, SdmMailRcVariables **hasharray);
  
  struct sstack {
    FILE    *s_file;                /* File we were in. */
    int      s_condition;           /* Saved state of conditionals */
  };

  struct sstack sstack[SdmMailRc_NoFile];

};

#endif
