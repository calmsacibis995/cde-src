/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm MailRc Class.

#pragma ident "@(#)MailRc.cc	1.62 97/05/30 SMI"

// Include Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
#include <SDtMail/MailRc.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/Utility.hh>

static const char* BaseErrorMessage = "Error during mail resource file processing:";

struct cmd {
    char *c_name;   /* Name of command */
    int (*c_func)(char **, SdmMailRc *);     /* Implementor of the command */
    void (*c_write)(const char * verbatim, char ** args, FILE * outf);
    short c_argtype;     /* Type of arglist (see below) */
    short c_msgflag;     /* Required flags of messages */
    short c_msgmask;     /* Relevant flags of messages */
};

#define NOFILE	20

struct SdmMailRcVariables *SdmMailRc::_Variables[SdmMailRc_HashSize]; /* Pointer to active var list */
struct SdmMailRcGlobals *SdmMailRc::_Globals = NULL;
char *SdmMailRc::_NullField = NULL;
SdmBoolean SdmMailRc::_ClearAliases;
SdmBoolean SdmMailRc::_NoAliasWritten;
SdmBoolean SdmMailRc::_ClearAliasesWritten;

#define MAXIMUM_PATH_LENGTH        2048

/* can't initialize unions */

#define c_minargs c_msgflag             /* Minimum argcount for RAWLIST */
#define c_maxargs c_msgmask             /* Max argcount for RAWLIST */
#define HASHMASK 0x3F			// binary bit mask used to extract hash index value
#define HASHSIZE HASHMASK+1		// number of entries covered by HASHMASK bits


// defines needed by mail-parse
#define LINESIZE 10240        /* max readable line width */
#define MAXARGC  1024    /* Maximum list of raw strings */
#define equal(a, b)   (strcmp(a,b)==0)/* A nice function to string compare */
#define NONE  ((struct cmd *) 0) /* The nil pointer to command tab */

#define	CANY		0		/* Execute in send or receive mode */
#define	CRCV		1		/* Execute in receive mode only */
#define	CSEND		2		/* Execute in send mode only */
#define	CTTY		3		/* Execute if attached to a tty only */
#define	CNOTTY		4		/* Execute if not attached to a tty */

#define STRLIST  1       /* A pure string */
#define RAWLIST  2       /* Shell string list */
#define F 01000  /* Is a conditional command */
#define NOLIST   3       /* Just plain 0 */
#define CANY            0               /* Execute in send or receive mode */
#define NOSTR           ((char *) 0)    /* Null string pointer */


struct SdmMailRcVariables {
    struct  SdmMailRcVariables *v_link;            /* Forward link to next variable */
    char    *v_name;                /* The variable's name */
    char    *v_value;               /* And it's current value */
    int	    v_written;		    /* It has been written on this pass. */
};	

struct hash {
    struct hash *h_next;
    char *h_key;
    void *h_value;
    int h_written;
};

struct SdmMailRcGlobals {
    char *g_myname;
    void *g_ignore;             /* hash list of ignored fields */
    void *g_retain;             /* hash list of retained fields */
    void *g_alias;              /* hash list of alias names */
#ifdef AUTOFILING
    void *g_filter;
#endif
    void *g_alternates; /* hash list of alternate names */
    int   g_nretained;  /* the number of retained fields */
};


struct cmd cmdtab[] = {
{ "unset",      SdmMailRc::Unset, SdmMailRc::WriteUnset, RAWLIST,        1,      1000, },
{ "alias",      SdmMailRc::Group, SdmMailRc::WriteGroup, RAWLIST,	2,	1000, },
{ "group",      SdmMailRc::Group, SdmMailRc::WriteGroup, RAWLIST,	2,	1000, },
{ "set",        SdmMailRc::Set,   SdmMailRc::WriteSet, RAWLIST,	1,	1000,},
{ "ignore",     SdmMailRc::IgnoreField, SdmMailRc::WriteIgnoreField, RAWLIST,        1,      1000, },
{ "source",     SdmMailRc::Source, SdmMailRc::WriteSource, RAWLIST,        1,      1000, },
{ "if",		      SdmMailRc::IfCmd, SdmMailRc::WriteIfCmd, F|RAWLIST, 1, 1, },
{ "else",      	SdmMailRc::ElseCmd, SdmMailRc::WriteElseCmd, F|RAWLIST, 0, 0, },
{ "endif",     	SdmMailRc::EndIfCmd, SdmMailRc::WriteEndIfCmd, F|RAWLIST, 0, 0, },
{ "alternates", SdmMailRc::Alternates, SdmMailRc::WriteAlternates, RAWLIST,        1,      1000, },
{ "clearaliases", SdmMailRc::ClearAliases, SdmMailRc::WriteClearAliases, RAWLIST, 0, 0, },
{ 0, 0,		0,		0,	0,	0 }
};

SdmMailRc* SdmMailRc::_GlobalMailRc = NULL;

// constructor
SdmMailRc::SdmMailRc(SdmError &err)
{
  //char *line = NULL;

  err = Sdm_EC_Success;
  _ParseError = Sdm_EC_Success;

  _InputFile = NULL;

  InitializeGlobals();

  passwd pw;
  SdmSystemUtility::GetPasswordEntry(pw);

#ifdef AUTOFILING
  // The ~/.forward file is where we will set up mailfilter program
  _ForwardFile =  (char *)malloc(MAXIMUM_PATH_LENGTH);
  strcpy(_ForwardFile, pw.pw_dir);
  strcat(_ForwardFile, "/.forward");
#endif

  char * mail_rc = getenv("MAILRC");
  if (mail_rc) {
    _MailRcName = strdup(mail_rc);
  }
  else {
    _MailRcName = (char *)malloc(strlen(pw.pw_dir)+9);	// pw.pw_dir+/.mailrc+NULL
    strcpy(_MailRcName, pw.pw_dir);
    strcat(_MailRcName, "/.mailrc");	// the +9 above
  }

  _ExecuteCondition = CANY;

  int rval = 0;

  if ((rval = this->Load( "/etc/mail/Mail.rc")) != 0) {
    char msg[MAXIMUM_PATH_LENGTH + LINESIZE];

    if (rval == -3) {
      sprintf(msg, "%s error in opening file %s for reading\n", BaseErrorMessage,
              "/etc/mail/Mail.rc");
    }
    else if (rval == -4) {
      sprintf(msg, "%s no endif for conditional if statement while reading %s\n", BaseErrorMessage, "/etc/mail/Mail.rc");
      err.AddMinorErrorCodeAndMessage(Sdm_EC_MRC_ResourceParsingNoEndif, "/etc/mail/Mail.rc");
      _ParseError = Sdm_EC_MRC_ResourceParsingNoEndif;
    }
    else {
      sprintf(msg, "%s: unknown error processing %s\n", BaseErrorMessage, "/etc/mail/Mail.rc");
    }

    SdmUtility::LogError(Sdm_False, "%s", msg);
  }

  rval = 0;

  if ((rval = this->Load(_MailRcName)) != 0) {
    char msg[MAXIMUM_PATH_LENGTH + LINESIZE];
    if (rval == -3) {
      sprintf(msg, "%s error in opening file %s for reading \n", BaseErrorMessage,
              _MailRcName);
    }
    else if (rval == -4) {
      sprintf(msg, "%s no endif for conditional if statement while reading %s\n", BaseErrorMessage, _MailRcName);
      err.AddMinorErrorCodeAndMessage(Sdm_EC_MRC_ResourceParsingNoEndif, _MailRcName);
      _ParseError = Sdm_EC_MRC_ResourceParsingNoEndif;
    }
    else {
      sprintf(msg, "%s: unknown error processing %s\n", BaseErrorMessage, _MailRcName);
    }

    SdmUtility::LogError(Sdm_False, "%s", msg);
  }
}

SdmMailRc::~SdmMailRc(void)
{
#ifdef AUTOFILING
  if (_MailRuleName)
    free(_MailRuleName);
  if (_ForwardFile)
    free(_ForwardFile);
#endif

  free(_MailRcName);
}

// return true if in list
SdmBoolean SdmMailRc::Ignore(SdmError &err, const char *name)
{

  char *ignore_list = (char *)SdmMailRc::Hm_Test((struct hash **)_Globals->g_ignore, (char *)name);

  err = Sdm_EC_Success;

  if(ignore_list != NULL) { // we have a valid ignore list
    return Sdm_True;
  } else {
    err = Sdm_EC_RequestedDataNotFound;
    return Sdm_False;
  }
    
}

void
SdmMailRc::AddIgnore(SdmError & err, const char * name)
{
  err = Sdm_EC_Success;
  AddIgnore((char *)name);
}

void
SdmMailRc::RemoveIgnore(SdmError & err, const char * name)
{
  err = Sdm_EC_Success;
  if(SdmMailRc::Hm_Test((struct hash **)_Globals->g_ignore, (char *)name)) {
    SdmMailRc::Hm_Delete((struct hash **)_Globals->g_ignore, (char *)name);
  }
}

const char *SdmMailRc::GetAlias(SdmError &err, const char * name)
{
  char *return_value = (char *)Hm_Test((struct hash **)_Globals->g_alias, (char *)name);

  err = Sdm_EC_Success;

  if(return_value == NULL)
    {
      err = Sdm_EC_RequestedDataNotFound;
      return NULL;
    } else {
      return return_value;
    }
}

void
SdmMailRc::SetAlias(SdmError & err,
			 const char * name,
			 const char * value)
{
  err = Sdm_EC_Success;

  if (Hm_Test((struct hash **)_Globals->g_alias, (char *)name)) {
    Hm_Delete((struct hash **)_Globals->g_alias, (char *)name);
  }

  AddAlias((char *)name, (char *)value);
}

void
SdmMailRc::RemoveAlias(SdmError & err,
			    const char * name)
{
  err = Sdm_EC_Success;

  if (Hm_Test((struct hash **)_Globals->g_alias, (char *)name)) {
    Hm_Delete((struct hash **)_Globals->g_alias, (char *)name);
  }
}



// if set with value return no err in err and set value
// if set with no value return no err in err and return empty string
// if not set, return err in err and leave value alone

void SdmMailRc::GetValue(SdmError &err, 
			      const char * var, char ** value)
{
  char *get_result = Mt_Value((char *)var);

  err = Sdm_EC_Success;
  *value = NULL;

  if(get_result != NULL)
    {
      *value = strdup((char *)get_result);
    } else {
      err = Sdm_EC_RequestedDataNotFound;
    }
}

void
SdmMailRc::SetValue(SdmError & err,
			 const char * var,
			 const char * value)
{
  err = Sdm_EC_Success;

  Mt_Assign((char *)var, (char *)value);
}

void
SdmMailRc::RemoveValue(SdmError & err,
			    const char * var)
{
  err = Sdm_EC_Success;

  Mt_Deassign((char *)var);
}


//
// IsValueDefined makes the same calls as GetValue
// but instead of creating a copy of the value and
// returning it, a boolean is just returned indicating
// if the variable is defined. 
// 
SdmBoolean SdmMailRc::IsValueDefined(const char* var)
{
  char *get_result = Mt_Value((char *)var);
 
  if (get_result != NULL)
    return Sdm_True;
  else 
    return Sdm_False;
}


// return alternates list
char * SdmMailRc::GetAlternates(SdmError &err)
{
  register int    i;
  register struct hash *h;
  struct hash **table = (struct hash **)_Globals->g_alternates;
  int len;

  err = Sdm_EC_Success;

  // we don't free this memory...
  _AlternateList = NULL;

  if(table == NULL)
    {
      err = Sdm_EC_RequestedDataNotFound;
      return NULL;
    }

  i = HASHSIZE;
  while (--i >= 0) {

    h = *table++;
    while (h) {
      len = strlen((const char*)h->h_key);

      if(_AlternateList == NULL)
        {
          _AlternateList = (char *)malloc(len + 1); // plus terminator
          strcpy(_AlternateList, (const char*)h->h_key);
        }
      else
        {
          len += strlen(_AlternateList);
          _AlternateList = (char *)realloc(_AlternateList, 
					   len + 2); // plus terminator
          // and space
          strcat(_AlternateList, " "); // add space
          strcat(_AlternateList, (const char *)h->h_key);
        }

      h = h->h_next;
    }
  }

  return (_AlternateList);
    
}

void
SdmMailRc::SetAlternate(SdmError & err, const char * alt)
{
  err = Sdm_EC_Success;

  AddAlternates((char *)alt);
}

void
SdmMailRc::RemoveAlternate(SdmError & err, const char * name)
{
  err = Sdm_EC_Success;

  if(SdmMailRc::Hm_Test((struct hash**)_Globals->g_alternates, (char *)name)) {
    SdmMailRc::Hm_Delete((struct hash **)_Globals->g_alternates, (char *)name);
  }
}

void
SdmMailRc::Update(SdmError & err)
{
  err = Sdm_EC_Success;
  struct stat sbuf;

  // Generate a temporary file.
  char tmp_mailrc[MAXIMUM_PATH_LENGTH];
  strcpy(tmp_mailrc, _MailRcName);
  strcat(tmp_mailrc, ".tmp");

  // Make sure that the user has write access to the mailrc file;
  // if the mailrc file exists, it must be writable to proceed

  if (access(_MailRcName, F_OK) == 0) {
    // the mailrc file exists - must be able to write to it and must be able to stat it
    if ((access(_MailRcName, W_OK) != 0)
	|| (stat(_MailRcName, &sbuf) != 0) ) {
      int lerrno = errno;
      err.SetMajorErrorCode(Sdm_EC_ErrorProcessingMailrc);
      err.AddMinorErrorCodeAndMessage(Sdm_EC_MRC_CannotOpenMailrcForUpdating, _MailRcName, (const char*)SdmUtility::ErrnoMessage(lerrno));
      return;
    }
  }
  else {
    // The mailrc file does not exist - dummy up stat structure 
    // By setting st_mode to 0, the chmod() before the rename is skipped;
    // the default permissions used by fopen as modified by the current umask
    // are used to create the .mailrc file
    sbuf.st_mode = 0;
  }

  // Create a temporary file to hold the new mailrc contents

  FILE * outf = fopen(tmp_mailrc, "w+");
  if (outf == NULL) {
    int lerrno = errno;
    err.SetMajorErrorCode(Sdm_EC_ErrorProcessingMailrc);
    err.AddMinorErrorCodeAndMessage(Sdm_EC_MRC_CannotOpenMailrcForUpdating, tmp_mailrc, (const char*)SdmUtility::ErrnoMessage(lerrno));
    return;
  }

  // Now open the mailrc for input.

  FILE * inf = fopen(_MailRcName, "r");
  if (inf != NULL) {
    // Now we will read the input file, and copy it to the output,
    // based on type and changes to the mailrc.
    //
    UpdateByLine(inf, outf);

    fclose(inf);
  }

  // Now we need to make a final scan. This will cause new values
  // to be written and the written flag to be reset for the next
  // update.
  //
  Mt_Scan(outf);

  Hm_Scan((struct hash **)_Globals->g_alias, GroupCallback, outf);

  // Alternates and groups we will add to a list, and then
  // put them out in one batch.
  //
  SdmStringL	value_list;  

  Hm_Scan((struct hash **)_Globals->g_ignore, IgnoreFieldCallback, &value_list);

  if (value_list.ElementCount()) {
    fwrite("ignore ", 1, 7, outf);
    for (int i=0; i< value_list.ElementCount(); i++) {
      const char * val = (const char *)value_list[i];
      char * newValue = GetStringWithEscapeChars((char*)val);
      fwrite(newValue, 1, strlen(newValue), outf);
      if (newValue != val) 
	delete newValue;
      fwrite(" ", 1, 1, outf);
    }
    fwrite("\n", 1, 1, outf);
  }

  // important: need to clear out value list before our next call otherwise we'll
  // get the data from the ignore list.
  value_list.ClearAllElements();
  Hm_Scan((struct hash **)_Globals->g_alternates, AlternatesCallback, &value_list);

  if (value_list.ElementCount()) {
    fwrite("alternates ", 1, 11, outf);
    for (int i=0; i< value_list.ElementCount(); i++) {
      const char * val = (const char *)value_list[i];
        
      char *newValue = GetStringWithEscapeChars((char*)val);
      fwrite(newValue, 1, strlen(newValue), outf);
      if (newValue != val) 
	delete newValue;
          
      fwrite(" ", 1, 1, outf);
    }
    fwrite("\n", 1, 1, outf);
  }

  fclose(outf);

  // If any of the f() system calls failed, do not continue with the mailrc update

  if (ferror(outf)) {
    int lerrno = errno;
    err.SetMajorErrorCode(Sdm_EC_ErrorProcessingMailrc);
    SdmSystemUtility::SafeUnlink(tmp_mailrc);
    return;
  }

  // Duplicate original mode of mailrc file

  if (sbuf.st_mode) {
    (void) chmod(tmp_mailrc, sbuf.st_mode);
  }

  // Cause the temporary file to be renamed over the original mailrc file

  if (SdmSystemUtility::SafeRename(tmp_mailrc, _MailRcName)) {
    int lerrno = errno;
    err.SetMajorErrorCode(Sdm_EC_ErrorProcessingMailrc);
    err.AddMinorErrorCodeAndMessage(Sdm_EC_MRC_CannotRenameNewMailrcFile, tmp_mailrc, _MailRcName, (const char *)SdmUtility::ErrnoMessage(lerrno));
    return;
  }

#ifdef AUTOFILING
  char tmp_mailrule[MAXIMUM_PATH_LENGTH];
  strcpy(tmp_mailrule, _MailRuleName);
  strcat(tmp_mailrule, ".tmp");

  outf = fopen(tmp_mailrule, "w+");
  if (outf == NULL) {
    int lerrno = errno;
    err.SetMajorErrorCode(Sdm_EC_ErrorProcessingMailrc);
    err.AddMinorErrorCodeAndMessage(Sdm_EC_MRC_CannotOpenMailruleForUpdating, tmp_mailrule, (const char *)SdmUtility::ErrnoMessage(lerrno));
    return;
  }

  // Now open the mailrules for input.
  inf = fopen(_MailRuleName, "r");
  if (inf != NULL) {
    // Now we will read the input file, and copy it to the output,
    // based on type and changes to the mailrc.
    //
    UpdateByLine(inf, outf);

    fclose(inf);
  }

  Hm_Scan((struct hash **)_Globals->g_filter, RuleGroupCallback, outf);

  fclose(outf);
  if (rename(tmp_mailrule, _MailRuleName)) {
    int lerrno = errno;
    err.SetMajorErrorCode(Sdm_EC_ErrorProcessingMailrc);
    err.AddMinorErrorCodeAndMessage(Sdm_EC_MRC_CannotRenameNewMailruleFile, tmp_mailrule, _MailRuleName, (const char *)SdmUtility::ErrnoMessage(lerrno));
    return;
  }
#endif
}

void SdmMailRc::GetAliasList(Hm_Callback stuffing_func, void *client_data)
{
  Hm_Scan((struct hash **)_Globals->g_alias, stuffing_func, client_data);
}

SdmStringL *SdmMailRc::GetAliasList()
{	

  SdmStringL	*value_list = NULL;

  value_list = new SdmStringL;

  Hm_Scan((struct hash **)_Globals->g_alias, AliasCallback, value_list);

  return value_list;
}

SdmStringL *SdmMailRc::GetIgnoreList()
{	

  SdmStringL	*value_list = NULL;

  value_list = new SdmStringL;

  Hm_Scan((struct hash **)_Globals->g_ignore, IgnoreListCallback, value_list);

  return value_list;
}

void
SdmMailRc::UpdateByLine(FILE * inf, FILE * outf)
{
  // We are going to keep two line buffers, unlike the first
  // pass parser. One is the line, with continuations, verbatim.
  // If nothing has changed, then we put this into the output
  // unmodified. The other is the line with continuations stripped
  // so we can finish parsing and analyzing the line.
  //
  char verbatim[LINESIZE];
  char for_parser[LINESIZE];
  char linebuf[LINESIZE];
  int  at_eof = 0;
  _NoAliasWritten = Sdm_True;
  _ClearAliasesWritten = Sdm_False;

  while(!at_eof) {
    verbatim[0] = 0;
    for_parser[0] = 0;

    while(1) {
      if (SdmSystemUtility::ReadLine(inf, linebuf, LINESIZE) <= 0) {
	at_eof = 1;
	break;
      }

      int len = strlen(linebuf);
      if (len == 0 || linebuf[len - 1] != '\\') {
	break;
      }

      strcat(verbatim, linebuf);
      strcat(verbatim, "\n");

      linebuf[len - 1] = ' ';
      strcat(for_parser, linebuf);
    }
    if (at_eof) {
      break;
    }

    strcat(verbatim, linebuf);
    strcat(verbatim, "\n");
    strcat(for_parser, linebuf);
    OutputLine(verbatim, for_parser, outf);
  }
}

void
SdmMailRc::OutputLine(const char * verbatim,
			   const char * for_parser,
			   FILE * outf)
{
  char *arglist[MAXARGC];
  const char * start = for_parser;
  while(*start && isspace(*start)) {
    start++;
  }

  if (*start == '#') {
    // If we have a comment, write it out and move on.
    //
    if (*++start != '-') {
      fwrite(verbatim, 1, strlen(verbatim), outf);
      return;
    } 

    /* strip whitespace again */
    while(*++start && isspace(*start)) {
      start++;
    }
  }

  char word[LINESIZE];

  // Pull off the command word.
  //
  char * cp2 = word;
  while (*start && !strchr(" \t0123456789$^.:/-+*'\"", *start))
    *cp2++ = *start++;

  *cp2 = '\0';

  if (equal(word, "")) {
    fwrite(verbatim, 1, strlen(verbatim), outf);
    return;
  }

  struct cmd * com = (struct cmd *)Lex(word);
  if (com == NONE) {
    // We dont know the command, so just copy the line.
    //
    fwrite(verbatim, 1, strlen(verbatim), outf);
    return;
  }

  // We will simply rewrite conditionals.
  if ((com->c_argtype & F)) {
    fwrite(verbatim, 1, strlen(verbatim), outf);
    return;
  }

  int c;
  switch (com->c_argtype & ~(F)) {
  case STRLIST:
    /*
     * Just the straight string, with
     * leading blanks removed.
     */
    while (strchr(" \t", *start)) {
      start++;
    }
    if (start && *start == '#') {
      SdmUtility::LogError(Sdm_False, "%s %s requires at least 1 arg\n", BaseErrorMessage,
			   com->c_name);
      break;
    }
    com->c_write(verbatim, (char **)&start, outf);
    break;

  case RAWLIST:
    /*
     * A vector of strings, in shell style.
     */
    if ((c = GetRawList((char *)start, arglist,
			sizeof arglist / sizeof *arglist)) < 0)
      break;
    if (c < com->c_minargs) {
      SdmUtility::LogError(Sdm_False,"%s %s requires at least %d arg(s)\n", BaseErrorMessage,
			   com->c_name, com->c_minargs);
      break;
    }
    if (c > com->c_maxargs) {
      SdmUtility::LogError(Sdm_False,"%s %s takes no more than %d arg(s)\n", BaseErrorMessage,
			   com->c_name, com->c_maxargs);
      break;
    }
    com->c_write(verbatim, arglist, outf);
    FreeRawList(arglist);
    break;

  case NOLIST:
    /*
     * Just the constant zero, for exiting,
     * eg.
     */
    com->c_write(verbatim, NULL, outf);
    break;

  default:
    SdmUtility::LogError(Sdm_True,"%s Unknown argtype %#x in OutputLine (This is an internal program error)", BaseErrorMessage, com->c_argtype);
    /* 		Mt_done(1); */
  }

}

// init the global hash structure
void SdmMailRc::InitializeGlobals()
{
  _Globals = new struct SdmMailRcGlobals;

  _Globals->g_myname = NULL; 

  _Globals->g_ignore = this->Hm_Alloc(); 
  _Globals->g_retain = this->Hm_Alloc(); 
  _Globals->g_alias = this->Hm_Alloc(); 

#ifdef AUTOFILING
  _Globals->g_filter = this->Hm_Alloc();
#endif

  _Globals->g_alternates = this->Hm_Alloc(); 
  _StackTop = -1;  

  _NullField = new char[3];

  strcpy(_NullField, "");

  _AlternateList = NULL;

}

// load a "user definintion" file
// Possible return values:
//   0 - no errors
//  -3 - fopen failed
//  -4 - no endif for conditional if statement
int
SdmMailRc::Load(char *name)
{
  register FILE *in, *oldin;
  int ret=0;
    
  // return success if file does not exist.
  if (SdmSystemUtility::SafeAccess(name, F_OK) < 0) {
    return(0);
  }

  if ((in = fopen(name, "r")) == NULL) {
    return(-3);
  }
    
  oldin = _InputFile;
  _InputFile = in;
  _Sourcing = 0;
    
  if ((ret = Commands()) != 0) {
    _InputFile = oldin;
    fclose(in);
    return ret;
  }
    
  _InputFile = oldin;
  fclose(in);
  return ret;
}

/*
 * Interpret user commands one by one.  If standard input is not a tty,
 * print no prompt.
 * Possible return values:
 *    0 - no errors
 *   -4 - no endif for conditional if statement
 */

int
SdmMailRc::Commands()
{
  register int n;
  char linebuf[LINESIZE];
  char line[LINESIZE];

  for (;;) {
    /*
     * Read a line of commands from the current input
     * and handle end of file specially.
     */

    n = 0;
    linebuf[0] = '\0';
    for (;;) {
      if (SdmSystemUtility::ReadLine(_InputFile, line, LINESIZE) <= 0) {
	if (n != 0) {
	  break;
	}
	if (_Sourcing) {
	  Unstack();
	  goto more;
	}
	// Conditional if statement with no corresponding endif 
	if (_ExecuteCondition != CANY) {
	  return(-4);	// no endif for conditional if statement
	}
	return 0;
      }

      if ((n = strlen(line)) == 0) {
	break;
      }  
      n--;
      if (line[n] != '\\') {
	break;
      }  
      line[n++] = ' ';
      if (n > LINESIZE - strlen(linebuf) - 1) {
	break;
      }
      strcat(linebuf, line);
    } /* end internal for */

    n = LINESIZE - strlen(linebuf) - 1;

    if (strlen(line) > n) {
      SdmUtility::LogError(Sdm_False, 
			   "%s Line plus continuation line too long:\n\t%s\n\nplus\n\t%s\n", BaseErrorMessage,
			   linebuf, line);

      if (_Sourcing) {
	Unstack();
	goto more;
      }

      // Conditional if statement with no corresponding endif 
      if (_ExecuteCondition != CANY) {
	return(-4);	// no endif for conditional if statement
      }
      return 0;
    }
    strncat(linebuf, line, n);
    Execute(linebuf);

  more:	/* do nothing */	;

  } /* end external for */

  return 0;
}

/*
 * Execute a single command.  If the command executed
 * is "quit," then return non-zero so that the caller
 * will know to return back to main, if he cares.
 * Contxt is non-zero if called while composing mail.
 */

int SdmMailRc::Execute(char linebuf[])
{
  char word[LINESIZE];
  char *arglist[MAXARGC];
  struct cmd *com;
  char *cp, *cp2;
  int c;
  //	int muvec[2];
  int e;
  int newcmd = 0;

  /*
   * Strip the white space away from the beginning
   * of the command, then scan out a word, which
   * consists of anything except digits and white space.
   *
   * Handle ! escapes differently to get the correct
   * Lexical conventions.
   */

  cp = linebuf;
  while (*cp && strchr(" \t", *cp))
    cp++;
  /*
   * Throw away comment lines here.
   */
  if (*cp == '#') {
    if (*++cp != '-') {
      return(0);
    }
    /* the special newcmd header -- "#-" */
    newcmd = 1;

    /* strip whitespace again */
    while (*++cp && strchr(" \t", *cp));
  }

  cp2 = word;
  while (*cp && !strchr(" \t0123456789$^.:/-+*'\"", *cp))
    *cp2++ = *cp++;
  *cp2 = '\0';

  /*
   * Look up the command; if not found, complain.
   * We ignore blank lines to eliminate confusion.
   */

  if (equal(word, ""))
    return(0);

  com = (struct cmd *)Lex(word);
  if (com == NONE) {
    if (newcmd) {
      /* this command is OK not to be found; that way
       * we can extend the .mailrc file with new
       * commands and not kill old mail and mailtool
       * programs.
       */
      return(0);
    }
    SdmUtility::LogError(Sdm_False,"%s Unknown command: \"%s\"\n", BaseErrorMessage, word);
    if (_Sourcing)
      Unstack();
    return(1);
  }

  /*
   * See if we should execute the command -- if a conditional
   * we always execute it, otherwise, check the state of _ExecuteCondition.  
   */

  if ((com->c_argtype & F) == 0) {
    if (_ExecuteCondition == CSEND || _ExecuteCondition == CTTY )
      {
	return(0);
      }

#ifdef undef
    if (_ExecuteCondition == CRCV && !rcvmode || _ExecuteCondition == CSEND && rcvmode ||
        _ExecuteCondition == CTTY && !intty || _ExecuteCondition == CNOTTY && intty)
      {
	return(0);
      }
#endif 
  }

  /*
   * Process the arguments to the command, depending
   * on the type he expects.  Default to an err.
   * If we are _Sourcing an interactive command, it's
   * an err.
   */

  e = 1;
  switch (com->c_argtype & ~(F)) {
  case STRLIST:
    /*
     * Just the straight string, with
     * leading blanks removed.
     */
    while (strchr(" \t", *cp)) {
      cp++;
    }
    if (cp && *cp == '#') {
      SdmUtility::LogError(Sdm_False,"%s %s requires at least 1 arg\n", BaseErrorMessage,
			   com->c_name);
      break;
    }
    e = (*com->c_func)(&cp, this);
    break;

  case RAWLIST:
    /*
     * A vector of strings, in shell style.
     */
    if ((c = GetRawList(cp, arglist,
			sizeof arglist / sizeof *arglist)) < 0)
      break;
    if (c < com->c_minargs) {
      SdmUtility::LogError(Sdm_False,"%s %s requires at least %d arg(s)\n", BaseErrorMessage,
			   com->c_name, com->c_minargs);
      break;
    }
    if (c > com->c_maxargs) {
      SdmUtility::LogError(Sdm_False,"%s %s takes no more than %d arg(s)\n", BaseErrorMessage,
			   com->c_name, com->c_maxargs);
      break;
    }
    e = (*com->c_func)(arglist, this);
    FreeRawList(arglist);
    break;

  case NOLIST:
    /*
     * Just the constant zero, for exiting,
     * eg.
     */
    e = (*com->c_func)(0, this);
    break;

  default:
    SdmUtility::LogError(Sdm_True,"%s Unknown argtype %#x in Execute (This is an internal program error)", BaseErrorMessage, com->c_argtype);
    /* 		Mt_done(1); */
  }

  /*
   * Exit the current source file on
   * err.
   */

  if (e && _Sourcing)
    Unstack();

  /* ZZZ:katin: should we return an err here? */
  return(e);
}



/*
 * Pop the current input back to the previous level.
 * Update the "_Sourcing" flag as appropriate.
 */

void SdmMailRc::Unstack()
{
  if (_StackTop < 0) {
    SdmUtility::LogError(Sdm_False,"%s \"Source\" stack over-pop.\n", BaseErrorMessage);
    _Sourcing = 0;
    return;
  }

  fclose(_InputFile);
  if (_ExecuteCondition != CANY)
    SdmUtility::LogError(Sdm_False, "%s Unmatched \"if\"\n", BaseErrorMessage);

  _ExecuteCondition = sstack[_StackTop].s_condition;
  _InputFile = sstack[_StackTop--].s_file;

  if (_StackTop < 0)
    _Sourcing = 0;
}
/*
 * Find the correct command in the command table corresponding
 * to the passed command "word"
 */

void *SdmMailRc::Lex(char word[])
{
  register struct cmd *cp;

  for (cp = &cmdtab[0]; cp->c_name != NOSTR; cp++)
    if (SdmMailRc::IsPrefix(word, cp->c_name))
      return(cp);
  return(NONE);
}

/*
 * Determine if as1 is a valid prefix of as2.
 * Return true if yep.
 */

int SdmMailRc::IsPrefix(char *as1, char *as2)
{
  register char *s1, *s2;

  s1 = as1;
  s2 = as2;

  while (*s1++ == *s2)
    if (*s2++ == '\0')
      return(1);

  return(*--s1 == '\0');
}

/*
 * Scan out the list of string arguments, shell style
 * for a RAWLIST.
 */

int SdmMailRc::GetRawList(char line[], char **argv, int argc)
{
  register char **ap, *cp, *cp2;
  char linebuf[LINESIZE], quotec;
  register char **last;
  char *next;
  char realChar;

  ap = argv;
  cp = line;
  last = argv + argc - 1;
  while (*cp != '\0') {
    while (*cp && strchr(" \t", *cp))
      cp++;

    // skip the rest of the line if we hit a comment.        
    if (*cp == '#') 
      break;
      
    cp2 = linebuf;
    quotec = 0;
    while (*cp != '\0') {
      // if we encounter a backslash, take the next character as is if
      // it's a backslash or a single quote.
      if (*cp == '\\') {
        next = cp+1;
        if (*next != '\0' && (*next == '\'' || *next == '\\' || *next == '\"' ||
                              *next == '@' || *next == '%')) {
          switch (*next) {
            case '\'':
            case '%':
              realChar = '\'';
              break;
            case '\"':
            case '@':
              realChar = '\"';
              break;
            default:
              realChar = '\\';
              break;
          }
          cp += 2;  // skip backslash and character.
          *cp2++ = realChar; // write character after backslash.
        } else {
          *cp2++ = *cp++;
        }
      } else {
        if (quotec) {
          if (*cp == quotec) {
	    quotec=0;
	    cp++;
          } else
	    *cp2++ = *cp++;
        } else {
          if (*cp && strchr(" \t", *cp))
	    break;
          if (*cp && strchr("'\"", *cp)) 
	    quotec = *cp++;
          else 
	    *cp2++ = *cp++;
        }
      }
    }
    *cp2 = '\0';
    if (cp2 == linebuf)
      break;
    if (ap >= last) {
      SdmUtility::LogError(Sdm_False, 
			   "%s Too many elements in the list; excess discarded\n", BaseErrorMessage);
      break;
    }
    *ap++ = strdup((char*)linebuf);
  }
  *ap = NOSTR;
  return(ap-argv);
}

void SdmMailRc::FreeRawList(char **argv)
{

  while(*argv) {
    free((char*)*argv);
    argv++;
  }
}

/*
 * Get the value of a variable and return it.
 * Look in the errironment if its not available locally.
 */

char *SdmMailRc::Mt_Value(char name[])
{
  register struct SdmMailRcVariables *vp;
  register char *cp;
  //        extern char *geterr();

  if ((vp = Lookup(name, (struct SdmMailRcVariables **)this->_Variables)) == (struct SdmMailRcVariables *)NULL)
    cp = getenv(name);
  else
    cp = vp->v_value;
  return (cp);
}

/*
 * Locate a variable and return its variable
 * node.
 */
struct SdmMailRcVariables *SdmMailRc::Lookup(char *name, struct SdmMailRcVariables **hasharray)
{
  register struct SdmMailRcVariables *vp;
  register int h;

  h = Hash(name);
  for (vp = hasharray[h]; vp != (struct SdmMailRcVariables *)NULL; vp = vp->v_link)
    if (strcmp(vp->v_name, name) == 0)
      return (vp);

  return ((struct SdmMailRcVariables *)NULL);
}


/*
 * Creates a string with escape characters inserted before quotes and back slashes.
 */
char* SdmMailRc::GetStringWithEscapeChars(char* value)
{
  char *newstring = NULL;

  if (value != NULL) {    
    char *cp = value;
    int special = 0;

    // loop thru string and determine how many special characters we need to escape.
    while (cp && *cp != '\0') {
      if (*cp == '\\' || *cp == '\'' || *cp == '\"') {
        special++;
      }
      cp++;
    }
    
    // if special characters found, create new string with escape characters inserted.
    // otherwise, just duplicate the string.
    if (special) {
      newstring = (char*) malloc (strlen(value)+special+1);
      char *cp2 = newstring;
      cp = value;
      while (cp && *cp != '\0') {
        if (*cp == '\\' || *cp == '\'' || *cp == '\"') {
          *cp2++ = '\\';
          switch (*cp) {
            case '\'':
              *cp2++ = '%';
              break;
            case '\"':
              *cp2++ = '@';
              break;
            default:
              *cp2++ = '\\';
              break;
          }
          cp++;
        } else {
          *cp2++ = *cp++;
        }
      }
      *cp2 = '\0';
    } else {
      newstring = value;
    }
  }
    
  return newstring;
}
/*
 * Put add users to a group.
 */
int SdmMailRc::Group(char **argv, SdmMailRc *)
{
  int size;
  char *buf;
  char *s;
  char **ap;

  /*
   * Insert names from the command list into the group.
   * Who cares if there are duplicates?  They get tossed
   * later anyway.
   */

  /* compute the size of the buffer */
  size = 0;
  for (ap = argv+1; *ap != NOSTR; ap++) {
    size += strlen(*ap) +1;
  }
  buf = (char *)malloc(size);
  s = buf;
  for (ap = argv+1; *ap != NOSTR; ap++) {
    strcpy(s, *ap);
    s += strlen(s);
    *s++ = ' ';
  }
  *--s = '\0';

  SdmMailRc::AddAlias((char *)argv[0], (char *)buf);
  free(buf);
  return(0);
}

void
SdmMailRc::WriteGroup(const char * verbatim, char ** argv, FILE * outf)
{
  int size = 0;
  char * buf;
  char * s;
  char ** ap;

  // if there is a clearaliases in the .mailrc and we haven't written
  // it out yet, write out the alias verbatim and return.
  // DO NOT mark the alias as written.
  // This code will preserve any aliases defined before the "clearaliases"
  // command.
  if (_ClearAliases && !_ClearAliasesWritten) {
    fwrite(verbatim, 1, strlen(verbatim), outf);
    return;
  }  


  for (ap = argv+1; *ap != NOSTR; ap++) {
    size += strlen(*ap) +1;
  }
  buf = (char *)malloc(size);
  s = buf;
  for (ap = argv+1; *ap != NOSTR; ap++) {
    strcpy(s, *ap);
    s += strlen(s);
    *s++ = ' ';
  }
  *--s = '\0';

  // get the current value of the alias.
  char * cur = (char *)Hm_Test((struct hash **)_Globals->g_alias, argv[0]);

  // If the alias has been removed or written, dont write it to the file.
  if (!cur || Hm_IsMarked((struct hash **)_Globals->g_alias, argv[0])) 
    {
      free(buf);
      return;
    }
  
  // write out the clearaliases before the first alias if the
  // clear aliases command is not defined in the .mailrc file.
  // this is to maintain portability with mailtool who always 
  // has clearaliases before the first alias it displays.
  if (!_ClearAliases && _NoAliasWritten) {
    fwrite("#-clearaliases\n", 1, 15, outf);
    _NoAliasWritten = Sdm_False;
    _ClearAliases = Sdm_True;
    _ClearAliasesWritten = Sdm_True;
  }

  if (strcmp(cur, buf) == 0) {
    // It hasnt changed. Write the original to preserve spacing.
    //
    fwrite(verbatim, 1, strlen(verbatim), outf);
  }
  else {
    // It has changed. Create a new alias.
    //
    fwrite("alias ", 1, 6, outf);
    fwrite(argv[0], 1, strlen(argv[0]), outf);
    fwrite(" ", 1, 1, outf);
    
    char * newValue = GetStringWithEscapeChars(cur);
    fwrite(newValue, 1, strlen(newValue), outf);
    if (newValue != cur) 
      delete newValue;
      
    fwrite("\n", 1, 1, outf);
  }
  free(buf);
  Hm_Mark((struct hash **)_Globals->g_alias, argv[0]);
}

void
SdmMailRc::GroupCallback(char * key, void * data, void * client_data)
{
  char * value = (char *)data;
  FILE * outf = (FILE *)client_data;

  if (!_ClearAliases && _NoAliasWritten) {
    fwrite("#-clearaliases\n", 1, 15, outf);
    _NoAliasWritten = Sdm_False;
    _ClearAliases = Sdm_True;
    _ClearAliasesWritten = Sdm_True;
  }

  fwrite("alias ", 1, 6, outf);
  char * newValue = GetStringWithEscapeChars(key);
  fwrite(newValue, 1, strlen(newValue), outf);
  if (newValue != key) 
    delete newValue;

  fwrite(" ", 1, 1, outf);
  newValue = GetStringWithEscapeChars(value);
  fwrite(newValue, 1, strlen(newValue), outf);
  if (newValue != value) 
    delete newValue;

  fwrite("\n", 1, 1, outf);
}

#ifdef AUTOFILING
void
SdmMailRc::RuleGroupCallback(char * key, void * data, void * client_data)
{
  char * value = (char *)data;
  FILE * outf = (FILE *)client_data;

  fwrite("rule \"", 1, 7, outf);
  fwrite(key, 1, strlen(key), outf);
  fwrite("\" ", 1, 2, outf);
  fwrite(value, 1, strlen(value), outf);
  fwrite("\n", 1, 1, outf);
}
#endif

void
SdmMailRc::AliasCallback(char * key, void * data, void * client_data)
{
  SdmStringL *value_list = (SdmStringL *)client_data;
  char *new_alias = NULL;
  char *white_space = NULL;
  int m_size = 0;
  int  i, num_spaces = 0;
  int key_len = strlen(key);
  // figure out whitespace for formatting
  // assume 13 for normal sized alias name

  if(key_len < 13)  // need to add spaces
    {
      num_spaces = 13 - key_len;

      white_space = (char *)malloc(num_spaces + 1);
      white_space[0] = '\0';

      for(i = 0; i < num_spaces; i++)
	white_space[i] = ' ';

      white_space[num_spaces] = '\0';

      //	  strcat(white_space, " ");

      /* make an alias string */
      m_size = key_len + strlen((char *)white_space)
	+ strlen((char *)data) + strlen(" = ") + 1;
      new_alias = (char *)malloc(m_size);

      sprintf(new_alias, "%s%s = %s",key, white_space, data);

    }
  else
    {
      /* make an alias string */
      m_size = key_len + strlen((char *)data) + strlen(" = ") + 1;
      new_alias = (char *)malloc(m_size);

      sprintf(new_alias, "%s = %s",key, data);

    }

  (*value_list)(-1) = new_alias;
  delete new_alias;
}



void
SdmMailRc::IgnoreListCallback(char * key, void * data, void * client_data)
{
  data = data;
  SdmStringL *value_list = (SdmStringL *)client_data;
  char *new_ignore = NULL;

  new_ignore = (char *)malloc(strlen((char *)key) + 2);

  sprintf(new_ignore, "%s", key);

  (*value_list)(-1) = new_ignore;
  delete new_ignore;
}


/*
 * Set or display a variable value.  Syntax is similar to that
 * of csh.
 */

int SdmMailRc::Set(char **arglist, SdmMailRc *)
{
  register char *cp, *cp2;
  char varbuf[LINESIZE], **ap;
  int errs;

  errs = 0;
  for (ap = arglist; *ap != NOSTR; ap++) {
    cp = *ap;
    cp2 = varbuf;
    while (*cp != '=' && *cp != '\0')
      *cp2++ = *cp++;
    *cp2 = '\0';
    if (*cp == '\0')
      cp = "";
    else
      cp++;
    if (equal(varbuf, "")) {
      SdmUtility::LogError(Sdm_False,"%s Non-null variable name required\n", BaseErrorMessage);
      errs++;
      continue;
    }
    SdmMailRc::Mt_Assign(varbuf, cp);
  }
  return(errs);
}

void
SdmMailRc::WriteSet(const char * verbatim,
		     char ** arglist,
		     FILE * outf)
{
  char varbuf[LINESIZE];
  char *cp, *cp2, **ap;

  for (ap = arglist; *ap != NOSTR; ap++) {
    cp = *ap;
    cp2 = varbuf;
    while (*cp != '=' && *cp != '\0')
      *cp2++ = *cp++;
    *cp2 = '\0';
    if (*cp == '\0')
      cp = "";
    else
      cp++;
    if (equal(varbuf, "")) {
      continue;
    }

    // Lookup the current value, and see if we should rewrite this
    // variable.
    //
    struct SdmMailRcVariables * vp = Lookup(varbuf, (struct SdmMailRcVariables **)_Variables);
    if (vp == NULL || vp->v_written) {
      // If the original input line was set novar then just write
      // it out again. We can not easily track duplicates here.
      //
      if (varbuf[0] == 'n' && varbuf[1] == 'o') {
	fwrite(verbatim, 1, strlen(verbatim), outf);
      }

      // This variable has been unassigned or previously written.
      // Remove it from the file, by not writing it.
      //
      continue;
    }
    
    char *newValue = GetStringWithEscapeChars(vp->v_value);

    // Compare the values. If equal, then write the line verbatim to
    // preserve the users original spacing and quoting.
    //
    if (strcmp(cp, newValue) == 0) {
      fwrite(verbatim, 1, strlen(verbatim), outf);
      vp->v_written = 1;
    }
    else {
      // Write the variable, with a new value.
      //
      fwrite("set ", 1, 4, outf);
      fwrite(varbuf, 1, strlen(varbuf), outf);
      if (strlen(newValue) > 0) {
	fwrite("='", 1, 2, outf);
	fwrite(newValue, 1, strlen(newValue), outf);
	fwrite("'", 1, 1, outf);
      }
      fwrite("\n", 1, 1, outf);
      vp->v_written = 1;
    }
    
    if (newValue != vp->v_value) 
      delete newValue;
  }
}

/*
 * Unset a bunch of variable values.
 */

int SdmMailRc::Unset(char **arglist, SdmMailRc *)
{
  register char **ap;

  for (ap = arglist; *ap != NOSTR; ap++)
    (void) SdmMailRc::Mt_Deassign(*ap);

  return(0);
}

void
SdmMailRc::WriteUnset(const char * verbatim,
		     char ** arglist,
		     FILE * outf)
{
  arglist = arglist;
  if(verbatim != NULL && outf != NULL)
    fwrite(verbatim, 1, strlen(verbatim), outf);
}

/*
 * Hash the passed string and return an index into
 * the variable or group hash table.
 */
int SdmMailRc::Hash(char *name)
{
  register unsigned h;
  register char *cp;

  for (cp = name, h = 0; *cp; h = (h << 1) + *cp++)	;

  return (h & SdmMailRc_HashMask);
}

/* manage the alias list */
void SdmMailRc::AddAlias(char *name, char *value)
{
  char *old;
  char *new_ptr;

  /* aliases to the same name get appended to the list */
  old = (char*)SdmMailRc::Hm_Test((struct hash **)_Globals->g_alias, name);
  if (old) {

    int size;

    size = strlen(value) + strlen(old) + 2;
    new_ptr = (char *)malloc(size);
    sprintf(new_ptr, "%s %s", value, old);

    /* delete any old bindings for the name */
    SdmMailRc::Hm_Delete((struct hash**)_Globals->g_alias, name);

    /* add the new binding */
    SdmMailRc::Hm_Add((struct hash**)_Globals->g_alias, name, new_ptr, size);

    /* free up the temporary space */
    free(new_ptr);

  } else {
    /* add the new binding */
    SdmMailRc::Hm_Add((struct hash**)_Globals->g_alias, name, value, strlen(value) +1);
  }
}

/*
 * Assign a value to a mail variable.
 */
void SdmMailRc::Mt_Assign(char *name,char * val)
{

  if (name[0]=='-') {
    (void) Mt_Deassign(name+1);
  }
  else if (name[0]=='n' && name[1]=='o') {
    (void) Mt_Deassign(name+2);
  }
  else if ((val[0] == 'n' || val[0] == 'N') &&
	   (val[1] == 'o' || val[1] == 'O') && val[2] == '\0') 
    {
      (void) Mt_Deassign(name);
    } 
  else 
    { 
      Mt_PutHash(name, CopyValue(val), SdmMailRc::_Variables);
    }
}

int SdmMailRc::Mt_Deassign(char *s)
{
  register struct SdmMailRcVariables *vp, *vp2;
  register int h;

  if ((vp2 = Lookup(s, SdmMailRc::_Variables)) == (struct SdmMailRcVariables *)NULL) {
    return (1);
  }
  h = Hash(s);
  if (vp2 == SdmMailRc::_Variables[h]) {
    SdmMailRc::_Variables[h] = SdmMailRc::_Variables[h]->v_link;
    FreeValue(vp2->v_name);
    FreeValue(vp2->v_value);
    free((char *)vp2);
    return (0);
  }

  for (vp = SdmMailRc::_Variables[h]; vp->v_link != vp2; vp = vp->v_link);

  vp->v_link = vp2->v_link;
  FreeValue(vp2->v_name);
  FreeValue(vp2->v_value);
  free((char *)vp2);
  return (0);
}

/*
 * associate val with name in hasharray
 */
void SdmMailRc::Mt_PutHash(char *name, char *val, struct SdmMailRcVariables **hasharray)
{
  register struct SdmMailRcVariables *vp;
  register int h;

  vp = Lookup(name, hasharray);
  if (vp == (struct SdmMailRcVariables *)NULL) {
    h = Hash(name);
    vp = (struct SdmMailRcVariables *) (calloc(sizeof *vp, 1));
    vp->v_name = CopyValue(name);
    vp->v_link = hasharray[h];
    vp->v_written = 0;
    hasharray[h] = vp;
  } else {
    FreeValue(vp->v_value);
  }
  vp->v_value = val;
}

void
SdmMailRc::Mt_Scan(FILE * outf)
{
  for (int slot = 0; slot < SdmMailRc_HashSize; slot++) {
    for (SdmMailRcVariables * vp = SdmMailRc::_Variables[slot]; vp != (struct SdmMailRcVariables *)NULL; vp = vp->v_link) {
      if (!vp->v_written) {
        fwrite("set ", 1, 4, outf);
        fwrite(vp->v_name, 1, strlen(vp->v_name), outf);
        if (strlen(vp->v_value)) {
	  fwrite("='", 1, 2, outf);
	  char *newValue = GetStringWithEscapeChars(vp->v_value);
	  fwrite(newValue, 1, strlen(newValue), outf);
	  if (newValue != vp->v_value) 
	    delete newValue;
	  fwrite("'", 1, 1, outf);
        }
        fwrite("\n", 1, 1, outf);
      }
      vp->v_written = 0;
    }
  }
}

/*
 * Copy a variable value into permanent space.
 * Do not bother to alloc space for "".
 */
char *SdmMailRc::CopyValue(char *str)
{

  if (strcmp(str, "") == 0)
    return ("");
  return (strdup(str));
}

/*
 * Free up a variable string.  We do not bother to allocate
 * strings whose value is "" since they are expected to be frequent.
 * Thus, we cannot free same!
 */
void SdmMailRc::FreeValue(char *cp)
{
    
  if (strcmp(cp, "") != 0)
    free(cp);
}

void *SdmMailRc::Hm_Alloc(void)
{
  struct hash **table;

  table = (struct hash**)malloc(sizeof (struct hash) * HASHSIZE);
  memset(table, '\0', sizeof (struct hash) * HASHSIZE);
  return (table);
}
void SdmMailRc::Hm_Add(struct hash **table, 
			    const char *key, 
			    void *value, 
			    int size)
{
  int index;
  register struct hash *h;

  if (!table)
    return;

  index = HashIndex(key);
  h = (struct hash *)malloc(sizeof (struct hash));
  h->h_next = table[index];
  h->h_written = 0;
  table[index] = h;
  h->h_key = strdup(key);
  if (size && value != NULL) {
    h->h_value = malloc(size);
    memcpy(h->h_value, value, size);
  } else {
    h->h_value = _NullField;
  }
}


void SdmMailRc::Hm_Delete(struct hash **table, const char *key)
{
  register int index;
  register struct hash *h;
  register struct hash *old;

  if (!table)
    return;

  index = HashIndex(key);
  old = NULL;
  h = table[index];
  while (h) {
    if (strcasecmp(h->h_key, key) == 0) {
      /* found the match */
      if (old == NULL)
        table[index] = h->h_next;
      else
        old->h_next = h->h_next;

      FreeHash(h);
      break;
    }

    old = h;
    h = h->h_next;
  }
}


void *SdmMailRc::Hm_Test(struct hash **table, const char *key)
{
  register struct hash *h;

  if (!table)
    return (NULL);

  h = table[HashIndex(key)];

  while (h) {
    if (strcasecmp(h->h_key, key) == 0) {
      /* found a match */
      return (h->h_value);
    }

    h = h->h_next;
  }

  return (NULL);
}

void SdmMailRc::Hm_Mark(struct hash **table, const char *key)
{
  register struct hash *h;

  if (!table)
    return;

  h = table[HashIndex(key)];

  while (h) {
    if (strcasecmp(h->h_key, key) == 0) {
      h->h_written = 1;
      return;
    }

    h = h->h_next;
  }
}

int SdmMailRc::Hm_IsMarked(struct hash **table, const char *key)
{
  register struct hash *h;

  if (!table)
    return (NULL);

  h = table[HashIndex(key)];

  while (h) {
    if (strcasecmp(h->h_key, key) == 0) {
      return(h->h_written);
    }

    h = h->h_next;
  }

  return(0);
}

void
SdmMailRc::Hm_Scan(struct hash **table, Hm_Callback callback, void * client_data)
{
  for (int slot = 0; slot < HASHSIZE; slot++) {
    for (struct hash * h = table[slot]; h; h = h->h_next) {
      if (!h->h_written) {
        callback(h->h_key, h->h_value, client_data);
      }
      h->h_written = 0;
    }
  }
}

int SdmMailRc::HashIndex(const char *key)
{
  register unsigned h;
  register const char *s;
  register c;

  s = key;
  h = 0;
  while (*s) {
    c = *s++;
    if (isupper(c)) {
      c = tolower(c);
    }
    h = (h << 1) + c;
  }

  return (h & HASHMASK);
}

void SdmMailRc::FreeHash(struct hash *h)
{
  free(h->h_key);
  if (h->h_value != _NullField) {
    free(h->h_value);
  }
  free(h);
}

/*
 * Add the given header fields to the ignored list.
 * If no arguments, print the current list of ignored fields.
 */
int SdmMailRc::IgnoreField(char *list[], SdmMailRc *)
{
  char **ap;

  for (ap = list; *ap != 0; ap++) {
    //                DP(("igfield: adding %s\n", *ap));
    SdmMailRc::AddIgnore(*ap);
  }
  return(0);
}

void
SdmMailRc::WriteIgnoreField(const char * verbatim, char ** list, FILE * outf)
{
  char ** ap;
  SdmBoolean ignore_written = Sdm_False;

  // We need to see that every name in this line still exists. If not,
  // then we will make a second pass, rewriting the line with only the
  // valid ignores.
  //
  int good = 1;
  for (ap = list; *ap && good; ap++) {
    if (!Hm_Test((struct hash **)_Globals->g_ignore, *ap)) {
      good = 0;
      break;
    }
  }

  if (good) {
    fwrite(verbatim, 1, strlen(verbatim), outf);
    for (ap = list; *ap; ap++) {
      Hm_Mark((struct hash **)_Globals->g_ignore, *ap);
    }
    return;
  }

  // Create a new ignore line, leaving out the ignores that have
  // been removed. Also, dont write any ignores that have been
  // previously written.
  //
  for (ap = list; *ap; ap++) {
    if (Hm_Test((struct hash **)_Globals->g_ignore, *ap) &&
        !Hm_IsMarked((struct hash **)_Globals->g_ignore, *ap)) 
      {
        // Only write 'ignore' if there is something in the list
        if (!ignore_written) {
          fwrite("ignore ", 1, 7, outf);
          ignore_written = Sdm_True;
        }
        char *newValue = GetStringWithEscapeChars(*ap);
        fwrite(newValue, 1, strlen(newValue), outf);
        if (newValue != *ap) 
          delete newValue;
        fwrite(" ", 1, 1, outf);
        Hm_Mark((struct hash **)_Globals->g_ignore, *ap);
      }
  }
  if (ignore_written)
    fwrite("\n", 1, 1, outf);
}

void
SdmMailRc::IgnoreFieldCallback(char * key, void *, void * client_data)
{
  SdmStringL *value_list = (SdmStringL *)client_data;

  // do not delete key.  It's part of the hash table.
  (*value_list)(-1) = key;
}

/* manage the retain/ignore list */
void SdmMailRc::AddIgnore(char *name)
{
  if(! SdmMailRc::Hm_Test((struct hash **)_Globals->g_ignore, name)) {
    /* name is not already there... */
    SdmMailRc::Hm_Add((struct hash **)_Globals->g_ignore, name, NULL, 0);
  }
}


/*
 * Set the list of alternate names.
 */
int SdmMailRc::Alternates(char **namelist, SdmMailRc *)
{
  while (*namelist != NULL)
    SdmMailRc::AddAlternates(*namelist++);
  return(0);
}

void
SdmMailRc::WriteAlternates(const char * verbatim, char ** list, FILE * outf)
{
  // This is like ignores. We need to make sure all of the alternates
  // on this command are still in the database.
  //
  char ** ap;
  int good = 1;
  for (ap = list; *ap && good; ap++) {
    if (!Hm_Test((struct hash **)_Globals->g_alternates, *ap)) {
      good = 0;
      break;
    }
  }

  if (good) {
    fwrite(verbatim, 1, strlen(verbatim), outf);
    for (ap = list; *ap; ap++) {
      Hm_Mark((struct hash **)_Globals->g_alternates, *ap);
    }
    return;
  }

  // Write out the alternates that still exist and have not been
  // previously written.
  //
  SdmBoolean written = Sdm_False;
  for (ap = list; *ap; ap++) {
    if (Hm_Test((struct hash **)_Globals->g_alternates, *ap) &&
        !Hm_IsMarked((struct hash **)_Globals->g_alternates, *ap))
      {
        if (!written) {
          fwrite("alternates ", 1, 11, outf);
          written=Sdm_True;
        }
        char *newValue = GetStringWithEscapeChars(*ap);
        fwrite(newValue, 1, strlen(newValue), outf);
        if (newValue != *ap) 
          delete newValue;
        fwrite(" ", 1, 1, outf);
        Hm_Mark((struct hash **)_Globals->g_alternates, *ap);
      }
  }

  if (written)
    fwrite("\n", 1, 1, outf);
}

void
SdmMailRc::AlternatesCallback(char * key, void *, void * client_data)
{
  SdmStringL *value_list = (SdmStringL *)client_data;

  // do not delete key.  It's part of the hash table.
  (*value_list)(-1) = key;
}

/* manage the alternate name list */
void SdmMailRc::AddAlternates(char *name)
{
  if(! SdmMailRc::Hm_Test((struct hash**)_Globals->g_alternates, name)) {
    /* name is not already there... */
    SdmMailRc::Hm_Add((struct hash **)_Globals->g_alternates, name, NULL, 0);
  }
}

/*
 * Determine the current folder directory name.
 */
int
SdmMailRc::GetFolderDirectory(char *name)
{
  char *folder;

  if ((folder = Mt_Value("folder")) == NOSTR)
    return(-1);
  if (*folder == '/')
    strcpy(name, folder);
  else
    sprintf(name, "%s/Mail/%s", Mt_Value("HOME"), folder);
  return(0);
}

/*
 * Take a file name, possibly with shell meta characters
 * in it and expand it by using "sh -c echo filename"
 * Return the file name as a dynamic string.
 */

char *
SdmMailRc::Expand(char *name)
{
  char xname[LINESIZE];
  char cmdbuf[LINESIZE];
  register int pid, l;
  register char *cp, *Shell;
  int pivec[2];
  struct stat sbuf;

  if (name[0] == '+' && GetFolderDirectory(cmdbuf) >= 0) {
    sprintf(xname, "%s/%s", cmdbuf, name + 1);
    return(Expand(xname));
  }
  if (!strpbrk(name, "~{[*?$`'\"\\")) {
    return(strdup(name));
  }
  if (pipe(pivec) < 0) {
    int lerrno = errno;
    SdmUtility::LogError(Sdm_False, "%s cannot create pipe: %s\n", BaseErrorMessage, (const char *)SdmUtility::ErrnoMessage(lerrno));
    return(strdup(name));
  }
  sprintf(cmdbuf, "echo %s", name);
  if ((pid = fork1()) == 0) {
    Shell = Mt_Value("SHELL");
    if (Shell == NOSTR || *Shell=='\0')
      Shell = "/bin/sh";
    close(pivec[0]);
    close(1);
    dup(pivec[1]);
    close(pivec[1]);
    close(2);
    execlp(Shell, Shell, "-c", cmdbuf, (char *)0);
    _exit(1);
  }
  if (pid == -1) {
    int lerrno = errno;
    SdmUtility::LogError(Sdm_False, "%s cannot create fork: %s\n", BaseErrorMessage, (const char *)SdmUtility::ErrnoMessage(lerrno));
    close(pivec[0]);
    close(pivec[1]);
    return(NOSTR);
  }
  close(pivec[1]);
  l = read(pivec[0], xname, LINESIZE);
  int readErrno = errno;
  close(pivec[0]);
#if 0
  // This code was commented out because we had to set SIGCLD to SIG_IGN so
  // that zombied children caused by forking off copies of audio_play to play
  // arrived mail sounds would not be created. This has the side effect of 
  // never allowing us to wait for forked children status... yetch!
  int s;
  while (wait(&s) != pid);
  ;
  s &= 0377;
  if (s != 0 && s != SIGPIPE) {
    SdmUtility::LogError(Sdm_False, "%s Echo failed\n", BaseErrorMessage);
    return(NOSTR);
  }
#endif
  if (l < 0) {
    SdmUtility::LogError(Sdm_False, "%s cannot read from subprocess: %s\n", BaseErrorMessage, (const char *)SdmUtility::ErrnoMessage(readErrno));
    return(NOSTR);
  }
  if (l == 0) {
    SdmUtility::LogError(Sdm_False, "%s %s: No match\n", BaseErrorMessage, name);
    return(NOSTR);
  }
  if (l == LINESIZE) {
    SdmUtility::LogError(Sdm_False, "%s Buffer overflow expanding %s\n", BaseErrorMessage, name);
    return(NOSTR);
  }
  xname[l] = 0;
  for (cp = &xname[l-1]; *cp == '\n' && cp > xname; cp--)
    ;
  *++cp = '\0';
  if (strchr(xname, ' ') && stat(xname, &sbuf) < 0) {
    SdmUtility::LogError(Sdm_False, "%s %s: Ambiguous\n", BaseErrorMessage, name);
    return(NOSTR);
  }

  return(strdup(xname));
}

int
SdmMailRc::Source(char **arglist, SdmMailRc *self)
{
  char *fname = arglist[0];
  FILE *fi;
  char *cp;

  /* if any of the three if test failed, return 0,
   * since we have not updated the input and stack pointer yet
   */
  if ((cp = self->Expand(fname)) == NOSTR)
    return(0);

  if ((fi = fopen(cp, "r")) == NULL) {
    free(cp);
    return(0);
  }
  free(cp);

  if (self->_StackTop >= SdmMailRc_NoFile - 2) {
    fclose(fi);
    return(0);
  }
  self->sstack[++(self->_StackTop)].s_file = self->_InputFile;
  self->sstack[self->_StackTop].s_condition = self->_ExecuteCondition;
  self->_ExecuteCondition = CANY;
  self->_InputFile = fi;
  self->_Sourcing++;

  return(0);
}

void
SdmMailRc::WriteSource(const char * verbatim, char ** arglist, FILE * outf)
{
  arglist = arglist;
  if(verbatim != NULL && outf != NULL)
    fwrite(verbatim, 1, strlen(verbatim), outf);
}

int
SdmMailRc::IfCmd(char **arglist, SdmMailRc *self)
{
  register char *cp;

  if (self->_ExecuteCondition != CANY) {
    SdmUtility::LogError(Sdm_False,"%s Illegal nested \"if\"\n", BaseErrorMessage);
    return(1);
  }
  self->_ExecuteCondition = CANY;
  cp = arglist[0];
  switch (*cp) {
  case 'r': case 'R':
    self->_ExecuteCondition = CRCV;
    break;

  case 's': case 'S':
    self->_ExecuteCondition = CSEND;
    break;

  case 't': case 'T':
    self->_ExecuteCondition = CTTY;
    break;

  default:
    SdmUtility::LogError(Sdm_False,"%s Unrecognized if-keyword: \"%s\"\n", BaseErrorMessage,
			 cp);
    return(1);
  }
  return(0);
}

void
SdmMailRc::WriteIfCmd(const char * verbatim, char ** arglist, FILE * outf)
{
  arglist = arglist;
  if(verbatim != NULL && outf != NULL)
    fwrite(verbatim, 1, strlen(verbatim), outf);
}


int
SdmMailRc::ElseCmd(char **, SdmMailRc *self)
{

  switch (self->_ExecuteCondition) {
  case CANY:
    SdmUtility::LogError(Sdm_False, "%s \"Else\" without matching \"if\"\n", BaseErrorMessage);
    return(1);

  case CSEND:
    self->_ExecuteCondition = CRCV;
    break;

  case CRCV:
    self->_ExecuteCondition = CSEND;
    break;

  case CTTY:
    self->_ExecuteCondition = CNOTTY;
    break;

  case CNOTTY:
    self->_ExecuteCondition = CTTY;
    break;

  default:
    SdmUtility::LogError(Sdm_False,"%s invalid condition encountered\n", BaseErrorMessage);
    self->_ExecuteCondition = CANY;
    break;
  }
  return(0);
}

void
SdmMailRc::WriteElseCmd(const char * verbatim, char ** arglist, FILE * outf)
{
  arglist = arglist;
  if(verbatim != NULL && outf != NULL)
    fwrite(verbatim, 1, strlen(verbatim), outf);
}


int
SdmMailRc::EndIfCmd(char **, SdmMailRc *self)
{

  if (self->_ExecuteCondition == CANY) {
    SdmUtility::LogError(Sdm_False,"%s \"Endif\" without matching \"if\"\n", BaseErrorMessage);
    return(1);
  }
  self->_ExecuteCondition = CANY;
  return(0);
}

void
SdmMailRc::WriteEndIfCmd(const char * verbatim, char ** arglist, FILE * outf)
{
  arglist = arglist;
  if(verbatim != NULL && outf != NULL)
    fwrite(verbatim, 1, strlen(verbatim), outf);
}

int
SdmMailRc::ClearAliases(char **, SdmMailRc *)
{
  SdmStringL value_list;

  Hm_Scan((struct hash **)_Globals->g_alias, AliasCallback, &value_list);

  for (int i=0; i < value_list.ElementCount(); i++) {
    char *buf;
    const char *val = (const char*)((value_list)[i]);
		
    if ((buf = strchr(val, ' ')) != NULL)
      *buf = '\0';
			
    if (Hm_Test((struct hash **)_Globals->g_alias, val))
      Hm_Delete((struct hash **)_Globals->g_alias, val);
        
    // Do we need to delete val??  No, it's part
    // of a SdmString stored in value_list.  this will
    // get clean up when the value_list is destroyed.
  }
  _ClearAliases = Sdm_True;
  return 0;
}
void
SdmMailRc::WriteClearAliases(const char *verbatim, char **arglist, FILE *outf)
{
  arglist = arglist;
  
  // write out "#-clearaliases" line to file.
  if(verbatim != NULL && outf != NULL) {
    fwrite(verbatim, 1, strlen(verbatim), outf);
    _ClearAliasesWritten = Sdm_True;

    struct hash **table = (struct hash **)_Globals->g_alias;

    // mark all aliases as unwritten.
    for (int slot = 0; slot < HASHSIZE; slot++) {
      for (struct hash * h = table[slot]; h; h = h->h_next) {
        h->h_written = 0;
      }
    }
  }
}


SdmMailRc*
SdmMailRc::GetMailRc()
{
  SdmError err;
  if (_GlobalMailRc == NULL) {
    _GlobalMailRc = new SdmMailRc(err);
    assert (_GlobalMailRc != NULL);
  }
  return _GlobalMailRc;
}
