#include <porttest.hh>
#include <SDtMail/LinkedList.hh>
#include <ptcmdscan.hh>
#include <ptctoken.hh>
#include <SDtMail/Sdtmail.hh>

/*
** EXTERNALS
*/

/*
** GLOBALS
*/
char *Htoken =
"Description: manipulate token objects\n"
"General usage: token command [ arguments ]\n"
"Specific usage:\n"
"\ttoken check token name parameter [value]\n"
"\ttoken clear [token-name(s) | *]\n"
"\ttoken compare token1 token2\n"
"\ttoken create name\n"
"\ttoken delete [token(s) | *]\n"
"\ttoken fetch token name [parameter]\n"
"\ttoken set token name parameter [value]\n"
"\ttoken status [token(s) | *]\n"
"\ttoken unset token name [parameter [value]]\n"
"";

SdmLinkedList		*TOKEN_LIST;

static int subcmd_token_check(int argc, char **argv);
static int subcmd_token_clear(int argc, char **argv);
static int subcmd_token_compare(int argc, char **argv);
static int subcmd_token_create(int argc, char **argv);
static int subcmd_token_delete(int argc, char **argv);
static int subcmd_token_fetch(int argc, char **argv);
static int subcmd_token_set(int argc, char **argv);
static int subcmd_token_status(int argc, char **argv);
static int subcmd_token_unset(int argc, char **argv);

static SUBCMD tokencmdtab[] = {
  // cmdname,	handler,	argsrequired
  {"check",	subcmd_token_check,	0},	// check to see if value is set
  {"clear",	subcmd_token_clear,	0},	// clear a token of all values
  {"compare",	subcmd_token_compare,	2},	// compare two tokens
  {"create",	subcmd_token_create,	0},	// create a new token
  {"delete",	subcmd_token_delete,	0},	// delete an existing token
  {"fetch",	subcmd_token_fetch,	0},	// fetch values in existing token
  {"set",	subcmd_token_set,	0},	// set token value
  {"status",	subcmd_token_status,	0},	// status of tokens
  {"unset",	subcmd_token_unset,	0},	// unset token value
  {NULL,	0,		0}	// must be last entry in table
};

/*
** FORWARD DECLARATIONS
*/

/*************************************************************************
 *
 * Function:	Ctoken - token command handler
 *
 * Input:	int argc - number of arguments passed to command
 *		char **argv - array of pargc arguments passed
 *
 * Output:	TRUE - help given
 *
 ************************************************************************/

int Ctoken(int argc, char **argv)
{
  return (argc == 1 ? TRUE : oneSubCommand(argv[0], argv[1], tokencmdtab, argc-2, argv+2));
}

static PtkToken *validTokenName(char *tokenName)
{
  SdmLinkedListIterator lit(TOKEN_LIST);
  PtkToken *ptk;

  while (ptk = (PtkToken *)(lit.NextListElement())) {
    assert(ptk->ptk_name);
    if (strcasecmp((const char *)*ptk->ptk_name, tokenName)==0)
      return(ptk);
  }
  return((PtkToken *)0);
}

const char *expandTokenName(PtkToken *ptk)
{
  assert(ptk->ptk_name);
  return ((const char *)*ptk->ptk_name);
}

static void deleteToken(PtkToken *ptk, int printMessage)
{
  assert(ptk);
  SdmToken *tk = ptk->ptk_token;
  assert(tk);
  printf("token %s: deleted\n", expandTokenName(ptk));
  TOKEN_LIST->RemoveElementFromList(ptk);
  delete ptk;
}

static void clearToken(PtkToken *ptk, int printMessage)
{
  assert(ptk);
  SdmToken *tk = ptk->ptk_token;
  assert(tk);
  tk->ClearTokenList();
  printf("token %s: cleared\n", expandTokenName(ptk));
}

static void statusToken(PtkToken *ptk, int printMessage)
{
  assert(ptk);
  SdmToken *tk = ptk->ptk_token;
  assert(tk);
  printf("token %s: exists\n", expandTokenName(ptk));

  SdmTokenIterator tit(tk);
  SdmString tkcName, tkcParameter, tkcValue;
  while (tit.NextToken(tkcName, tkcParameter, tkcValue) == Sdm_True)
    printf("\t%-10s %-10s %-10s\n", (const char *)tkcName, (const char *)tkcParameter, (const char *)tkcValue);
}

PtkToken *lookupToken(char *tokenName, int printMessage)
{
  PtkToken *ptk;

  assert(tokenName);

  // If the token list is empty, nothing to do here
  if (!TOKEN_LIST || !TOKEN_LIST->ElementCount()) {
    if (printMessage)
      printf("?No tokens defined\n");
    return((PtkToken *)0);
  }

  // lookup the token by name - if the name is registered, return that token 
  ptk = validTokenName(tokenName);
  if (ptk)
    return(ptk);

  // token not registered - say so and return nothing
  if (printMessage)
    printf("?token %s does not exist\n", tokenName);
  return((PtkToken *)0);
}

static int subcmd_token_clear(int argc, char **argv)
{
  if (!argc) {
    printf("?The clear subcommand takes one or more arguments which specify\n?the tokens to be cleared; to clear all tokens, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentToken = *argv++;
    if (strcmp(currentToken,"*")==0) {
      SdmLinkedListIterator lit(TOKEN_LIST);
      PtkToken *ptk;

      while (ptk = (PtkToken *)(lit.NextListElement())) {
	clearToken(ptk, TRUE);
      }
    }
    else {
      PtkToken *ptk = lookupToken(currentToken, TRUE);
      if (ptk)
	clearToken(ptk, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_token_compare(int argc, char **argv)
{
  assert(argc>=2);
  char *tk1name = *argv++;
  char *tk2name = *argv++;
  PtkToken *ptk1 = lookupToken(tk1name, TRUE);
  if (!ptk1)
    return(TRUE);
  SdmToken *tk1 = ptk1->ptk_token;
  assert(tk1);
  PtkToken *ptk2 = lookupToken(tk2name, TRUE);
  if (!ptk2)
    return(TRUE);
  SdmToken *tk2 = ptk2->ptk_token;
  assert(tk2);
  if (tk1 == tk2) 
    printf("The two token names specified resolve to the same token\n");
  printf("comparison exact: %s(%s) = %s %s(%s) = %s\n", 
	 tk1name, tk2name, tk1->CompareExact(*tk2) ? "YES":"no", 
	 tk2name, tk1name, tk2->CompareExact(*tk1) ? "YES":"no");
  printf("comparison loose: %s(%s) = %s %s(%s) = %s\n", 
	 tk1name, tk2name, tk1->CompareLoose(*tk2) ? "YES":"no", 
	 tk2name, tk1name, tk2->CompareLoose(*tk1) ? "YES":"no");
  return(TRUE);
}

static int subcmd_token_create(int argc, char **argv)
{
  if (argc != 1) {
    printf("?The create subcommand requires a name argument: token create name \n");
    return(TRUE);
  }

  // See if the token name already exists
  if (lookupToken(*argv, FALSE) != (PtkToken *)0) {
    printf("?Token %s already exists\n", *argv);
    return(TRUE);
  }

  // Name is unique - create token with it
  SdmString *name = new SdmString(*argv);
  assert(name);
  PtkToken *ptk = new PtkToken();
  assert(ptk);
  SdmToken *tk = new SdmToken();
  assert(tk);
  if (!TOKEN_LIST)
    TOKEN_LIST = new SdmLinkedList();
  ptk->ptk_token = tk;
  ptk->ptk_name = name;
  TOKEN_LIST->AddElementToList(ptk);
  printf("token %s: defined\n", expandTokenName(ptk));
  return(TRUE);
}

static int subcmd_token_delete(int argc, char **argv)
{
  if (!argc) {
    printf("?The delete subcommand takes one or more arguments which specify\n?the tokens to be deleted; to delete all tokens, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentToken = *argv++;
    if (strcmp(currentToken,"*")==0) {
      SdmLinkedListIterator lit(TOKEN_LIST);
      PtkToken *ptk;

      while (ptk = (PtkToken *)(lit.NextListElement())) {
	deleteToken(ptk, TRUE);
      }
    }
    else {
      PtkToken *ptk = lookupToken(currentToken, TRUE);
      if (ptk)
	deleteToken(ptk, TRUE);
    }
  }
  return(TRUE);
}

static int subcmd_token_fetch(int argc, char **argv)
{
  // args can be either: name parameter <or> name 
  if (argc < 2 || argc > 3) {
    printf("?too %s arguments to token command\n", argc < 2 ? "few" : "many");
    printf("?token fetch has two forms: token name parameter <or> token name\n");
    return(TRUE);
  }

  PtkToken *ptk;
  SdmToken *tk;
  char *tokenName = *argv++;
  ptk = lookupToken(tokenName, TRUE);
  if (!ptk)
    return(TRUE);
  tk = ptk->ptk_token;
  assert(tk);

  SdmString name = (const char *)*argv++;
  SdmString parameter;
  SdmString value;

  if (argc == 2) {
    SdmBoolean success = tk->FetchValue(parameter, name);
    if (success == Sdm_True)
      printf("Value fetched: %s\n", (const char *)parameter);
    else
      printf("No value fetched\n");
  }
  else {
    parameter = (const char *)*argv;
    SdmBoolean success = tk->FetchValue(value, name, parameter);
    if (success == Sdm_True)
      printf("Value fetched: %s\n", (const char *)value);
    else
      printf("No value fetched\n");
  }

  return(TRUE);
}

static int subcmd_token_set(int argc, char **argv)
{
  // args can be either: name parameter value <or> name parameter
  if (argc < 3 || argc > 4) {
    printf("?too %s arguments to token command\n", argc < 3 ? "few" : "many");
    printf("?token set has two forms: token name parameter <or> token name parameter value\n");
    return(TRUE);
  }

  PtkToken *ptk;
  SdmToken *tk;
  char *tokenName = *argv++;
  ptk = lookupToken(tokenName, TRUE);
  if (!ptk)
    return(TRUE);
  tk = ptk->ptk_token;
  assert(tk);

  SdmString name = (const char *)*argv++;
  SdmString parameter = (const char *)*argv++;

  if (argc == 3)
    tk->SetValue(name, parameter);
  else {
    SdmString value = (const char *)*argv;
    tk->SetValue(name, parameter, value);
  }
  printf("token %s: set\n", expandTokenName(ptk));
  return(TRUE);
}

static int subcmd_token_unset(int argc, char **argv)
{
  // args can be either: name parameter <or> name
  if (argc < 2 || argc > 4) {
    printf("?too %s arguments to token command\n", argc < 2 ? "few" : "many");
    printf("?token set has three forms: token name <or> token name parameter <or> token name parameter value\n");
    return(TRUE);
  }

  PtkToken *ptk;
  SdmToken *tk;
  char *tokenName = *argv++;
  ptk = lookupToken(tokenName, TRUE);
  if (!ptk)
    return(TRUE);
  tk = ptk->ptk_token;
  assert(tk);

  SdmString name = (const char *)*argv++;

  if (argc == 2)
    tk->ClearValue(name);
  else if (argc == 3) {
    SdmString parameter = (const char *)*argv++;
    tk->ClearValue(name, parameter);
  }
  else {
    SdmString parameter = (const char *)*argv++;
    SdmString value = (const char *)*argv++;
    tk->ClearValue(name, parameter, value);
  }
  printf("token %s: unset\n", expandTokenName(ptk));
  return(TRUE);
}

static int subcmd_token_check(int argc, char **argv)
{
  // args can be either: name parameter <or> name
  if (argc < 3 || argc > 4) {
    printf("?too %s arguments to token command\n", argc < 2 ? "few" : "many");
    printf("?token check has two forms: token name parameter <or> token name parameter value\n");
    return(TRUE);
  }

  PtkToken *ptk;
  SdmToken *tk;
  SdmBoolean result;

  char *tokenName = *argv++;
  ptk = lookupToken(tokenName, TRUE);
  if (!ptk)
    return(TRUE);
  tk = ptk->ptk_token;
  assert(tk);

  SdmString name = (const char *)*argv++;
  SdmString parameter = (const char *)*argv++;
  if (argc == 3)
    result = tk->CheckValue(name, parameter);
  else {
    SdmString value = (const char *)*argv++;
    result = tk->CheckValue(name, parameter, value);
  }
  printf("token %s check: %s\n", expandTokenName(ptk), result ? "set" : "not set");
  return(TRUE);
}

static int subcmd_token_status(int argc, char **argv)
{
  if (!argc) {
    printf("?The status subcommand takes one or more arguments which specify\n?the tokens to report status on; to report status of all tokens, use a single * character.\n");
    return(TRUE);
  }
  while (argc--) {
    char *currentToken = *argv++;
    if (strcmp(currentToken,"*")==0) {
      SdmLinkedListIterator lit(TOKEN_LIST);
      PtkToken *ptk;

      while (ptk = (PtkToken *)(lit.NextListElement())) {
	statusToken(ptk, TRUE);
      }
    }
    else {
      PtkToken *ptk = lookupToken(currentToken, TRUE);
      if (ptk)
	statusToken(ptk, TRUE);
    }
  }
  return(TRUE);
}
