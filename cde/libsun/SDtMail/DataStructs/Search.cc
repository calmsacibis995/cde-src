/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm SdmSearch Object Class.

#pragma ident "@(#)Search.cc	1.47 97/05/01 SMI"

// Include Files
#include <SDtMail/Search.hh>
#include <string.h>
#include <ctype.h>
#include <DataStructs/MutexEntry.hh>
#include <assert.h>
#include <PortObjs/DataChanCclient.hh>


/* defines types of terms in parsing search string */ 
enum SdmSearchParseTermType {
  Sdm_SPT_Token  = 1,
  Sdm_SPT_BooleanOp = 2
};


/* other definitions */
static const int gkMaxParseBufferLength = 256;
static const char* gkTokenStops = " \t";




// search field contains string in field and the parenthesis type for it.
typedef struct _SdmSearchField {
  SdmString field;                      /* field substring */
  SdmString key;                        /* used for fields that have key/value pair */
  SdmSearch::SdmParenthesisType parens; /* parens type */
} SdmSearchField;
 
/* search token object */
typedef struct _SdmSearchToken {
  const char *token;                    /* string itself */
  SdmBoolean field_required;            /* followed by a field if true */
  unsigned long field_attributes;       /* attributes for field */
  SdmSearch::SdmParenthesisType  parens;/* dynamic - left paren before, etc */
  SdmNotOperatorBinding atomic_not;     /* not stuff */
  SdmNotOperatorBinding expression_not;
} SdmSearchToken;

/* field  attributes */
typedef SdmBitMask SdmSearchFieldAttributeFlag;

static const SdmSearchFieldAttributeFlag Sdm_SFA_None  =  (1<<0);			/* no attributes */
static const SdmSearchFieldAttributeFlag Sdm_SFA_Atomic = (1<<1);			/* no "" used around field */
static const SdmSearchFieldAttributeFlag Sdm_SFA_Time = (1<<2);			  /* THE TIME is used */
static const SdmSearchFieldAttributeFlag Sdm_SFA_Numeric = (1<<3);		/* Must be a number */
static const SdmSearchFieldAttributeFlag Sdm_SFA_ApplySearch = (1<<4);/* Apply substring/wildcard searches */
static const SdmSearchFieldAttributeFlag Sdm_SFA_KeyValuePair = (1<<5);/* key value pair field */



/* Note: Strings are UPPERCASE:
 * "Token", Field required, field attributes, parenthesis, NOT binding*/
// ** ET
// The tokens below define all the tokens recognized in the grammar.
// There are many types of tokens.  They are:
// 1.  apply to the headers (eg. "TO", "CC", "SUBJECT"),
// 2.  apply to content of general parts of the message ("HEADER", "TEXT", "BODY")
// 3.  apply to time ("SENTON", "BEFORE")
// 4.  apply to message flags ("NEW", "DELETED")
// 5.  apply to size of message ("LARGER", "SMALLER")
//  
// search_tokens is really used in the CreateToken method where the
// it searches this table to find the matching token for a given string.
//
static const SdmSearchToken search_tokens[]= {
  SdmSearch::Sdm_STK_To,	  /* Token string */
  Sdm_True,					        /* Sdm_True if field required */
  Sdm_SFA_ApplySearch,			/* attributes of field */
  SdmSearch::Sdm_PAREN_Zero,	/* dynamic - set during parsing */
  Sdm_NOB_NoBinding,			  /* dynamic - how "not" binds token */
  Sdm_NOB_NoBinding,			  /* dynamic - how "not" bind express. */
  
  SdmSearch::Sdm_STK_From,
  Sdm_True,
  Sdm_SFA_ApplySearch,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_CC,
  Sdm_True,
  Sdm_SFA_ApplySearch,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,
  
  SdmSearch::Sdm_STK_BCC,
  Sdm_True,
  Sdm_SFA_ApplySearch,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Subject,
  Sdm_True,
  Sdm_SFA_ApplySearch,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Text,
  Sdm_True,
  Sdm_SFA_None,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Header,
  Sdm_True,
  Sdm_SFA_KeyValuePair,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Body,
  Sdm_True,
  Sdm_SFA_None,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Keyword,
  Sdm_True,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Unkeyword,
  Sdm_True,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Since,
  Sdm_True,
  Sdm_SFA_Time,				/* TIME attribute */
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Before,
  Sdm_True,
  Sdm_SFA_Time,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_On,
  Sdm_True,
  Sdm_SFA_Time,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_During,
  Sdm_True,
  Sdm_SFA_Time,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_SentSince,
  Sdm_True,
  Sdm_SFA_Time,				/* TIME attribute */
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_SentBefore,
  Sdm_True,
  Sdm_SFA_Time,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_SentOn,
  Sdm_True,
  Sdm_SFA_Time,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Larger, 
  Sdm_True,
  Sdm_SFA_Atomic|Sdm_SFA_Numeric,    
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Smaller, 
  Sdm_True,
  Sdm_SFA_Atomic|Sdm_SFA_Numeric,  
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  /* Now the flags - NO field which is "vacuously" Sdm_SFA_Atomic*/
  SdmSearch::Sdm_STK_New,
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Old, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Deleted, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Undeleted, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Seen, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Unseen, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Read, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Unread, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Recent, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Answered, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Unanswered, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Draft, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Undraft, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Flagged, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_Unflagged, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  SdmSearch::Sdm_STK_All, 
  Sdm_False,
  Sdm_SFA_Atomic,
  SdmSearch::Sdm_PAREN_Zero,
  Sdm_NOB_NoBinding,
  Sdm_NOB_NoBinding,

  NULL, 
  Sdm_False, 
  0L, 
  SdmSearch::Sdm_PAREN_Zero, 
  Sdm_NOB_NoBinding, 
  Sdm_NOB_NoBinding
};

typedef struct _SdmBooleanOperatorStructure {
  char *logic_op;
  SdmBooleanOperator bool;
} SdmBooleanOperatorStructure;


// ** ET
// gkBooleanList is list of boolean operations that apply to tokens.
static const SdmBooleanOperatorStructure gkBooleanList[] = 
{
  // THe boolean tokens
  "AND", Sdm_BOP_And,
  "OR", Sdm_BOP_Or,
  NULL, Sdm_BOP_None,
};
    



// 
// ** ET 
// macros are used to expand strings that represent a complete condition.
// these are not just keywords like those defined in gkDynamicPredicateMap above.
// Because it defines a condition, the macro is really just leaves
// that are ANDed together to define the condition.  An extra leaf is
// added to the end of each condition for some reason (I don't know why).
//

static SdmSearchLeaf unread_macro[] = {
  "UNSEEN", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf read_macro[] = {
  "SEEN", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf lastweek_macro[] = {
  "SINCE \"LASTWEEK\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"SUNDAY\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf lastmonth_macro[] = {
  "SINCE \"LASTMONTH\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"THISMONTH\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf thismonth_macro[] = {
  "SINCE \"THISMONTH\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf january_macro[] = {
  "SINCE \"January\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"February\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf february_macro[] = {
  "SINCE \"February\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"March\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf march_macro[] = {
  "SINCE \"March\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"April\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf april_macro[] = {
  "SINCE \"April\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"May\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf may_macro[] = {
  "SINCE \"May\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"June\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf june_macro[] = {
  "SINCE \"June\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"July\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf july_macro[] = {
  "SINCE \"July\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"August\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf august_macro[] = {
  "SINCE \"August\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"September\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf september_macro[] = {
  "SINCE \"September\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"October\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf october_macro[] = {
  "SINCE \"October\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"November\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf november_macro[] = {
  "SINCE \"November\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  "BEFORE \"December\"", NULL, NULL, NULL, Sdm_BOP_And, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };

static SdmSearchLeaf december_macro[] = {
  "SINCE \"December\"", NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_DynamicField, NULL,
  NULL, NULL, NULL, NULL, Sdm_BOP_None, Sdm_SLE_Simple, Sdm_NOB_NoBinding, Sdm_NOB_NoBinding, Sdm_PRED_Simple, NULL
  };


// ** ET
// gMacroElementTable below maps the string to the macro used to define it.
// the macro is really just a list of leaves that are "AND" together to
// define the condition.
//
// Each element in gkMacroElementTable contains:
//    first item:  string
//    second item: name of macro that expands the string.
//    third item: number of leaves used in macro.
//
static const SdmSearchLeafMacro gkMacroElementTable[] = 
{
  "UNREAD",
  unread_macro,
  1,

  "READ",
  read_macro,
  1,

  "DURING \"LASTWEEK\"",
  lastweek_macro,
  2,

  "DURING \"LASTMONTH\"",
  lastmonth_macro,
  2,

  "DURING \"THISMONTH\"",
  thismonth_macro,
  1,

  "DURING \"JANUARY\"",
  january_macro,
  2,
    
  "DURING \"FEBRUARY\"",
  february_macro,
  2,

  "DURING \"MARCH\"",
  march_macro,
  2,

  "DURING \"APRIL\"",
  april_macro,
  2,

  "DURING \"MAY\"",
  may_macro,
  2,
  
  "DURING \"JUNE\"",
  june_macro,
  2,

  "DURING \"JULY\"",
  july_macro,
  2,

  "DURING \"AUGUST\"",
  august_macro,
  2,

  "DURING \"SEPTEMBER\"",
  september_macro,
  2,
  
  "DURING \"OCTOBER\"",
  october_macro,
  2,

  "DURING \"NOVEMBER\"",
  november_macro,
  2,

  "DURING \"DECEMBER\"",
  december_macro,
  1,
    
  NULL,
  NULL,
  0,
};
    

//
// Constants defined in SdmSearch for tokens.
//
const char* SdmSearch::Sdm_STK_To = "TO";
const char* SdmSearch::Sdm_STK_From = "FROM";
const char* SdmSearch::Sdm_STK_CC = "CC";
const char* SdmSearch::Sdm_STK_BCC = "BCC";
const char* SdmSearch::Sdm_STK_Subject = "SUBJECT";
const char* SdmSearch::Sdm_STK_Text = "TEXT";
const char* SdmSearch::Sdm_STK_Header = "HEADER";
const char* SdmSearch::Sdm_STK_Body = "BODY";
const char* SdmSearch::Sdm_STK_Keyword = "KEYWORD";
const char* SdmSearch::Sdm_STK_Unkeyword = "UNKEYWORD";
const char* SdmSearch::Sdm_STK_Before = "BEFORE";
const char* SdmSearch::Sdm_STK_Since = "SINCE";
const char* SdmSearch::Sdm_STK_On = "ON";
const char* SdmSearch::Sdm_STK_During = "DURING";
const char* SdmSearch::Sdm_STK_SentSince = "SENTSINCE";
const char* SdmSearch::Sdm_STK_SentBefore = "SENTBEFORE";
const char* SdmSearch::Sdm_STK_SentOn = "SENTON";
const char* SdmSearch::Sdm_STK_Larger = "LARGER";
const char* SdmSearch::Sdm_STK_Smaller = "SMALLER";
const char* SdmSearch::Sdm_STK_New = "NEW";
const char* SdmSearch::Sdm_STK_Old = "OLD";
const char* SdmSearch::Sdm_STK_Deleted = "DELETED";
const char* SdmSearch::Sdm_STK_Undeleted = "UNDELETED";
const char* SdmSearch::Sdm_STK_Read = "READ";
const char* SdmSearch::Sdm_STK_Unread = "UNREAD";
const char* SdmSearch::Sdm_STK_Seen = "SEEN";
const char* SdmSearch::Sdm_STK_Unseen = "UNSEEN";
const char* SdmSearch::Sdm_STK_Recent = "RECENT";
const char* SdmSearch::Sdm_STK_Flagged = "FLAGGED";
const char* SdmSearch::Sdm_STK_Unflagged = "UNFLAGGED";
const char* SdmSearch::Sdm_STK_Answered= "ANSWERED";
const char* SdmSearch::Sdm_STK_Unanswered= "UNANSWERED";
const char* SdmSearch::Sdm_STK_Draft = "DRAFT";
const char* SdmSearch::Sdm_STK_Undraft = "UNDRAFT";
const char* SdmSearch::Sdm_STK_All = "ALL";

const char* SdmSearch::Sdm_DDF_January = "JANUARY";
const char* SdmSearch::Sdm_DDF_February = "FEBRUARY";
const char* SdmSearch::Sdm_DDF_March = "MARCH";
const char* SdmSearch::Sdm_DDF_April = "APRIL";
const char* SdmSearch::Sdm_DDF_May = "MAY";
const char* SdmSearch::Sdm_DDF_June = "JUNE";
const char* SdmSearch::Sdm_DDF_July = "JULY";
const char* SdmSearch::Sdm_DDF_August = "AUGUST";
const char* SdmSearch::Sdm_DDF_September = "SEPTEMBER";
const char* SdmSearch::Sdm_DDF_October = "OCTOBER";
const char* SdmSearch::Sdm_DDF_November = "NOVEMBER";
const char* SdmSearch::Sdm_DDF_December = "DECEMBER";
const char* SdmSearch::Sdm_DDF_ThisMonth = "THISMONTH";
const char* SdmSearch::Sdm_DDF_LastMonth = "LASTMONTH";
const char* SdmSearch::Sdm_DDF_LastWeek = "LASTWEEK";
const char* SdmSearch::Sdm_DDF_Today = "TODAY";
const char* SdmSearch::Sdm_DDF_Yesterday = "YESTERDAY";
const char* SdmSearch::Sdm_DDF_Sunday = "SUNDAY";
const char* SdmSearch::Sdm_DDF_Monday = "MONDAY";
const char* SdmSearch::Sdm_DDF_Tuesday = "TUESDAY";
const char* SdmSearch::Sdm_DDF_Wednesday = "WEDNESDAY";
const char* SdmSearch::Sdm_DDF_Thursday = "THURSDAY";
const char* SdmSearch::Sdm_DDF_Friday = "FRIDAY";
const char* SdmSearch::Sdm_DDF_Saturday = "SATURDAY";
    
//
// Methods for SdmSearch
//

SdmSearch::SdmSearch()
{
  int rc;
  rc = mutex_init(&_SearchStringLock, USYNC_THREAD, NULL);
  assert (rc == 0);
}

SdmSearch::~SdmSearch()
{
  mutex_destroy(&_SearchStringLock);
}

// Set current SdmSearch string
// ** ET
// A copy of the string is kept internally in this object.
// It is protected using a mutex for multithread access.
SdmErrorCode 
SdmSearch::SetSearchString(SdmError &err, const SdmString &string) 
{ 
  err = Sdm_EC_Success;
  SdmMutexEntry entry(_SearchStringLock);
  _CurrentSearchString = string; 
  return err;
}


SdmErrorCode
SdmSearch::GetSearchString(SdmError &err, SdmString &r_string) const
{ 
  err = Sdm_EC_Success;
  SdmSearch *me = (SdmSearch*)this;
  SdmMutexEntry entry(me->_SearchStringLock);
  r_string = _CurrentSearchString;
  return err;
}

void
SdmSearch::FreeLeaf(SdmSearchLeaf *leaf)
{
  if (!leaf) return;
  
  SdmSearchLeaf *ptr = leaf;
  SdmSearchLeaf *previous_ptr;
  SdmSearchLeaf *r = leaf->sl_new_root;
  // Free and the  macro parallel tree
  // if it exists.
  while (r) {
    free(r->sl_predicate);
    SdmSearchLeaf *rp = r->sl_next;
    free(r);
    r = rp;
  }
  while (ptr) {
    free(ptr->sl_predicate);
    previous_ptr = ptr;
    ptr = ptr->sl_next;
    free(previous_ptr);
  }
}

void
SdmSearch::PrintLeaf(SdmSearchLeaf *leaf)
{
  if (!leaf) {
    cout << "++++ Info: Leaf is NULL. ++++\n";
    return;
  }

  cout << "\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
  
  char buffer[gkMaxParseBufferLength];
  memset(&buffer, 0, gkMaxParseBufferLength);
  
  cout << "predicate: " << (leaf->sl_predicate ? leaf->sl_predicate : "NULL") << "\n";
  cout << "uses macro: " << (leaf->sl_uses_macros ? "true" : "false") << "\n";
  
  switch (leaf->sl_logic_op) {
    case Sdm_BOP_None:  strcpy(buffer, "none");  break; 
    case Sdm_BOP_And:   strcpy(buffer, "AND"); break; 
    case Sdm_BOP_Or:    strcpy(buffer, "OR"); break; 
    default:            strcpy(buffer, "unknown"); break; 
  }
  cout << "boolean op: " << buffer << "\n";

  switch (leaf->sl_expression) {
    case Sdm_SLE_Simple:       strcpy(buffer, "Simple");  break; 
    case Sdm_SLE_Start:        strcpy(buffer, "Start"); break; 
    case Sdm_SLE_Intermediate: strcpy(buffer, "Intermediate"); break; 
    case Sdm_SLE_End:          strcpy(buffer, "End"); break; 
    default:                   strcpy(buffer, "unknown"); break; 
  }
  cout << "expression type: " << buffer << "\n";

  switch (leaf->sl_atomic_not) {
    case Sdm_NOB_NoBinding:      strcpy(buffer, "none");  break; 
    case Sdm_NOB_BindAtom:       strcpy(buffer, "BindAtom"); break; 
    case Sdm_NOB_BindExpression: strcpy(buffer, "BindExpression"); break; 
    default:                     strcpy(buffer, "unknown"); break; 
  }
  cout << "atomic not: " << buffer << "\n";

  switch (leaf->sl_expression_not) {
    case Sdm_NOB_NoBinding:      strcpy(buffer, "none");  break; 
    case Sdm_NOB_BindAtom:       strcpy(buffer, "BindAtom"); break; 
    case Sdm_NOB_BindExpression: strcpy(buffer, "BindExpression"); break; 
    default:                     strcpy(buffer, "unknown"); break; 
  }
  cout << "expression not: " << buffer << "\n";

  switch (leaf->sl_predicate_type) {
    case Sdm_PRED_Simple:       strcpy(buffer, "Simple");  break; 
    case Sdm_PRED_Field:        strcpy(buffer, "Field"); break; 
    case Sdm_PRED_DynamicField: strcpy(buffer, "Dynamic"); break; 
    default:                    strcpy(buffer, "unknown"); break; 
  }
  cout << "predicate type: " << buffer << "\n";
  
  if (leaf->sl_next) {
    PrintLeaf(leaf->sl_next);
  } else {
    cout << "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
  }
}

// ParseSearch - public interface to the search parser (InternalParseSearch)
// Handles stuffing returned error codes into minor codes
//
SdmErrorCode    
SdmSearch::ParseSearch(SdmError &err, SdmSearchLeaf *&r_leaf, 
  const SdmBoolean createLeafList)
{
  SdmErrorCode ec;

  err = Sdm_EC_Success;
  ec = InternalParseSearch(err, r_leaf, createLeafList);
  if (ec != Sdm_EC_Success) {
    err.AddMajorErrorCode(Sdm_EC_SearchStringParseFailure);
  }
  return(ec);
}

// ** ET
// ParseSearch parses the SdmSearch string and the SdmSearchLeaf list
// representing the search string.   _error is set to an error code if
// there is an error found in parsing the string.  If parsing is
// successfull and createLeafList is TRUE, the resulting leaf is 
// returned to the caller in r_leaf.  Caller is responsible for the 
// deletion of r_leaf.  If createLeafList is FALSE, the string is parsed
// and the resulting error code indicates whether the parse was successful.
//
SdmErrorCode    
SdmSearch::InternalParseSearch(SdmError &err, SdmSearchLeaf *&r_leaf, 
  const SdmBoolean createLeafList)
{
  err = Sdm_EC_Success;  
  char *filter = NULL;

  // set the search criteria to ALL if no search string is specified.
  if ((const char*)_CurrentSearchString == NULL || _CurrentSearchString.Length() == 0) {
    filter = strdup(SdmSearch::Sdm_STK_All);
  } else {
    // ** ET 
    // make a copy of the current search string.  
    // need to get lock to access the string.
    //    
    SdmMutexEntry entry(_SearchStringLock);
    filter = strdup((const char*)_CurrentSearchString);  // make a copy
  }
  
  char *filter_ptr = filter;
  SdmBoolean looping = Sdm_True;         // keeps it going
  int parsing;                           // count terms
  int next_term = Sdm_SPT_Token;         // initially a Sdm_SPT_Token
  SdmSearchToken *token_ptr = NULL;      // ptr to a token block
  int other_len;
  SdmSearchField field;                  // parsed field
  int field_len;
  SdmSearchLeaf *new_leaf = NULL;
  SdmBooleanOperator current_operator = Sdm_BOP_None;
  char *current_token = NULL;

  MakeUpperSearchString(filter);            // change to upper case
  FixNotOperators(filter);                  // clean up ~s
  int n_parens = FixWhiteSpaces(filter);    // clear unnescessary whitespace
  

  if (n_parens > 0 && !IsParenthesisBalanced(filter)) { // check parens
    err = Sdm_EC_PRS_ParenthesisBad;
    free(filter_ptr);
    return err;					       
  }

  int filter_len = strlen(filter);
  for (parsing = 0; looping; ++parsing) {
    // find the first token 
    current_token = strtok(filter, gkTokenStops); 
    if (!current_token) {
      if (parsing == 0) {
        err = Sdm_EC_PRS_EmptyFilter;
      } else if (next_term == Sdm_SPT_BooleanOp) {
        // last term in the predicate
        err = Sdm_EC_Success;
      } else
        err = Sdm_EC_PRS_MissingPredicate;        // No predicate after AND/OR
      break;
    }

    // cases for token or SdmBoolean operators
    switch (next_term) {
      case Sdm_SPT_Token:
        // select a token
        // ** ET 
        // Note: Token returned from CreateToken is a copy of the
        // entry in the static global search_tokens table.  Therefore
        // we need to delete it when we are done.
        if (CreateToken(err, token_ptr, current_token)) {
          looping = Sdm_False;
          continue;
        }
        else {
          other_len = strlen(current_token);
          StripNotOperators(&current_token);        
          StripParenthesis(&current_token, token_ptr->parens);
          StripNotOperators(&current_token);
          next_term = Sdm_SPT_BooleanOp;
        }
        filter = AdvanceFilter(filter, &filter_len, other_len);
        break;

      case Sdm_SPT_BooleanOp:
        // find the operator that binds the next predicate with
        // its predecessors   (And/Or)
        current_operator = FindBoolean(current_token);
        if (current_operator == Sdm_BOP_None) {
          err = Sdm_EC_PRS_ExpectedBoolean;
          looping = Sdm_False;
          continue;
        }
        else {
          other_len = strlen(current_token);
          next_term = Sdm_SPT_Token;
        }
        filter = AdvanceFilter(filter, &filter_len, other_len);
        continue;
    }


    // check the field following the token
    if (token_ptr->field_required) {
    
      if (token_ptr->field_attributes & Sdm_SFA_KeyValuePair)
      {
        // get the key.
        if (SelectField(err, &field, filter, &field_len)) {
          // change error code to reflect error in parsing key field.
          if (err == Sdm_EC_PRS_IllegalLeftParenthesisField) {
            err = Sdm_EC_PRS_IllegalLeftParenthesisKeyField;
          }
          looping = Sdm_False;
          continue;
        }
        
        // unfortunately, SelectField puts the string in
        // "field" so we need to copy it to "key".
        field.key = field.field;

        // check for valid key field syntax.
        if (!IsValidKey(err, field)) {
          looping = Sdm_False;
          continue;
          
        } else {
          StripParenthesis(field.field, field.parens);
          
          // remove double quotes around key if they exist.  
          StripQuotes(field.key);

          filter_len -= field_len;      // skip the field in the filter
          filter += field_len;
          *filter = '\0';
          if (filter_len > 0) {
            ++filter;
            -- filter_len;
          }
        }
      } else {
        field.key = "";  // set key to empty string.  it's not used for this token.
      }

      // call to SelectField below sets up the field structure.
      if (SelectField(err, &field, filter, &field_len)) {
        looping = Sdm_False;
        continue;
      }
      
      // ** ET - the following removes parenthesis from field.
      StripParenthesis(field.field, field.parens);

      if (token_ptr->field_attributes & Sdm_SFA_Numeric &&
          !IsNumericField(err, (const char*)field.field)) 
      {
          looping = Sdm_False;
          // ** ET
          // why isn't _error set here?  it is set in numberic_field call.  
          // why not return??  setting looping to false will
          // break out of loop and we will return after loop 
          // because of error.
          continue;
      }

      if (token_ptr->field_attributes & Sdm_SFA_Time &&
          !SdmDpDataChanCclient::IsDateField((const char*)field.field)) 
      {
          looping = Sdm_False;
          err = Sdm_EC_PRS_IllegalDateField;
          continue;
      }

      filter_len -= field_len;      // skip the field
      filter += field_len;
      *filter = '\0';
      if (filter_len > 0) {
        ++filter;
        -- filter_len;
      }
    }
    else {
      field.parens = SdmSearch::Sdm_PAREN_Zero;
      field.field = "";
    }
    
    /* place in our parsing tree */
    // ** ET
    // if new_leaf is NULL, then a new leaf will be created and new_leaf
    // will be set to point to it.  if new_leaf is not NULL,
    // a new leaf will be created and placed at the end
    // of the parsing tree.
    if (createLeafList) {
      AddLeafToTree(&new_leaf, token_ptr, &field, 
                    current_operator);
    }
    
    // ** ET
    // if a separate token is returned to us from CreateToken, then
    // we need to delete it here.
    delete token_ptr;
    token_ptr = NULL;
  }

  if (err == Sdm_EC_Success) {
    if (createLeafList) {
      r_leaf = new_leaf;
    
      // Do any necessary macro expansions, save them in the result.
      // ** ET - SetMacroForLeaf goes through the leaves
      // and sets the leaf_macro field if the leave is dynamic
      // and uses a macro to determine is value (eg. "DURING February").
      SetMacroForLeaf(r_leaf);

      // Do any necessary leaf expansions.  Goes through leaves and
      // expands the leaf macros to the individual leaves.  (eg. changes 
      // the "DURING February" leaf to two leaves that represent
      // "SINCE February" and "BEFORE March".  The new tree is placed
      // in the sl_new_root field.  If no macros are defined, then
      // r_leaf is unchanged.
      CombineSearchTree(r_leaf);
    }
    
  } else if (new_leaf) {			
    // ** ET - fixed memory leak in new_leaf below.
    FreeLeaf(new_leaf);
  }
  
  free(filter_ptr);
  // ** ET clean up token_ptr if it was set earlier.
  if (token_ptr) {
    delete token_ptr;
  }
  return err;
}



// ***************************************************
// 
// Methods used for parsing.
//
// ***************************************************



// convert a string to uppercase 
// ** ET **
// Keep this function as is.
void
SdmSearch::MakeUpperSearchString(char *str)
{
  char c;

  while (c = *str) 
    *str++ = toupper(c);
}

// ** ET - skips first char in string only if it is the not character.
// Keep method as is.
void
SdmSearch::StripNotOperators(char **str)
{
  /* must be the first char, so we just skip it */
  if (**str == '~')
    ++*str;
}

/* remove parens from a string */
// ** ET - Keep method as is.
void
SdmSearch::StripParenthesis(
     SdmString &str,
     SdmParenthesisType ptype)
{
  char *cp;
  SdmStringLength length;
  length = str.Length();

  switch (ptype) {
  case SdmSearch::Sdm_PAREN_Zero:
    break;
  case SdmSearch::Sdm_PAREN_Left:
    str = str(1, length-1);    /* skip this char */
    break;
  case SdmSearch::Sdm_PAREN_Right:	
    str = str(0, length-1);   /* null out last char */
    break;
  default:				            /* left and right */
    str = str(1, length-2);  	/* skip both parens */
    break;
  }
}

/* remove double quotes from a string */
// ** ET - new.  quotes should match before this method is called.
void
SdmSearch::StripQuotes(SdmString &str)
{
  SdmStringLength length = str.Length();
  if (length > 1 && str[0] == '\"' && str[length-1] == '\"') {
    str = str(1, length-2);   /* skip first and last character */
  }
}

// ** ET - new. remove parens from a SdmString object.
void
SdmSearch::StripParenthesis(
     char **str,
     SdmParenthesisType ptype)
{
  char *cp;
 
  switch (ptype) {
    case SdmSearch::Sdm_PAREN_Zero:
      break;
    case SdmSearch::Sdm_PAREN_Left:
      ++*str;                             /* skip this char */
      break;
    case SdmSearch::Sdm_PAREN_Right:
      cp = GetLastCharacter(*str);
      *cp = '\0';                         /* null it out */
      break;
    default:                              /* left and right */
      ++*str;                             /* skip both parens */
      cp = GetLastCharacter(*str);
      *cp = '\0';
      break;
  }
}
 


/*
 * FixNotOperators:
 *  o An even number of '~' --> ' '
 *  o An odd number --> '~' 
 *  o Remove trailing spaces after a ~ */
// ** ET 
// keep function as is.
void
SdmSearch::FixNotOperators(char *str)
{
  char *src, *dst, c;
  int n_nots= 0;

  src = dst = str;
  while (c = *src++) {
    if (c == '"') {
      *dst++ = c;
      while (c = *src++) {			  /* copy quoted strings */
        *dst++ = c;
        if (c == '"')
          break;
      }
      if (c == '\0') 
        break;			  /* all done  */
      else
        continue;
    }
    if (c == '~') {
      /* have a ~ */
      n_nots = 1;
      while (c = *src) {
        if (c == '~') {
          ++n_nots;
          ++src;
        } else {
          break;
        }
      }
      /* if there is an even number we simply save the current char, sinon ... */
      if ((n_nots & 1) != 0) {			  /* odd number of NOTs */
        *dst++ = '~';				  /* keep one NOT */
        if (c == ' ') {				  /* remove spaces following ~ */
          while (*src && *src == ' ') {
            ++src;				  /* skip "~ " */
          }
        } else {
          *dst++ = c;				  /* NOT a space  */
          ++src;				  /* TO next src char after c */
        }
        continue;				  /* c == ' ' so discard it. */
      }
    }						  /* end have a ~ */
    *dst++ = c;
  }
  *dst = '\0';
}

/* FixWhiteSpaces:
 *   Consectutive whitespace chars made into a ' ',
 *   and "( " becomes "(" and " )" becomes ")" */
// ** ET **
// Keep this function as is.
int
SdmSearch::FixWhiteSpaces(char *str)
{
  char *src, *dst;
  char c;
  int ws= 1;				/* to clear leading whitespace */
  SdmBoolean gp;				/* gauche (left) parenthesis */
  int parens= 0;
  int cnt;

  src = dst = str;
  /* first compact all whitespace */
  while (c = *src++) {
    if (c == '\n')
      continue;				/* skip RETURN */
      
    if (c == ' ' || c == '\t') {
      ws += 1;				/* count whitespace */
      if (ws > 1) continue;		/* skip it */
      c = ' ';				/* use SPACE */
    } else {
      ws = 0;
    }
    
    *dst++ = c;
    if (c == ')' || c == '(')
      ++parens;
  }  
  *dst = '\0';
  
  if (parens == 0)
    return 0;
    
  /* Now check for "( " or " )" */
  src = dst = str;
  gp = Sdm_False;
  cnt = 0;
  while (c = *src++) {
    ++cnt;
    switch (c) {

    case '"':
      *dst++ = c;
      while (c = *src++) {
        *dst++ = c;			/* cueiller these chars. */
        if (c == ')' || c == '(') {
          --parens;			/* does not count within ".." */
        }
        else if (c == '"') {		/* skipped "<expression>" */
          break;
        }
      }
      if (!c) --str;			/* '"' not found, terminate the loop */
      break;
 
    case '(': 
      gp = Sdm_True; *dst++ = c;
      break;

    case ')': 
      if (cnt >= 2 && *(src - 2) == ' ') { /* then " )" */
        --dst;				/* write over the SPACE */
      } 	
      *dst++ = c;			/* keep the guy */
      break;
    default:
      if (gp) {			/* previous "(" */
        gp = Sdm_False;
        if (c == ' ')			/* Found "( " */
          continue;			/* skip the SPACE */
      }
      *dst++ = c;			/* keep the CHAR */
      break;
    }
   }
  *dst = '\0';
  return parens;
}



/* Here is a dumb paranthesis check that makes sure we have:
   o balanced right and left parenthesis
   o () is illegal
   o Only a depth of 1 is allowed, ie (..( .. ) ..) is illegal.
   o skip "<expression>"  */
// ** ET **
// Returns boolean indicating whether the filter contains balanced
// parenthesis.  The filter is not changed in this function.
// Note that the current search only supports one level of nested
// parenthesis.
// Keep this function as is.  
SdmBoolean
SdmSearch::IsParenthesisBalanced(char *filter)
{
  int pcnt= 0;
  char c;
  int n_not= 0;

  while (c = *filter++) {
    switch (c) {
      case '"':				/* scan until '"' */
        ++n_not;
        while (c = *filter++) {
          ++n_not;			/* not a parenthesis */
          if (c == '"') {			/* skipped "<expression>" */
            break;
          }
        }
        if (!c) --filter;			/* '"' not found, terminate the loop */
        break;

      case '(':
        ++pcnt;
        if (pcnt > 1) {
          return Sdm_False;			/* too deep */
        }
        n_not = 0;
        break;

      case ')':
        --pcnt;
        if (pcnt < 0) {
          return Sdm_False;			/* too many rights */
        }
        else if (n_not == 0) {     
          return Sdm_False;			/* () NULLE is illegal */
        }
        n_not = 0;
        break;

      default:
        ++n_not;				/* not one of the above */
        break;
    }
  }
  if (pcnt != 0)
    return Sdm_False;			/* not balanced */
  else
    return Sdm_True;
}


// ** ET
// returns boolean indicating whether field represents a numeric
// (positive integer) value.  field is not changed and the actual
// numeric value is NOT returned.
SdmBoolean
SdmSearch::IsNumericField(SdmError &err, const char *field)
{
  unsigned long i;
  err = Sdm_EC_Success;
  const char *tmp = field;
  
  if (!SdmDpDataChanCclient::StringToNumber(&i, &tmp) ||
      tmp == NULL || *tmp != '\0')
  {
    err = Sdm_EC_PRS_IllegalNumericField;
    return Sdm_False;
  }  
  
  return Sdm_True;
}



// ** ET
// new method added.  checks if given string is a valid key.
// A valid key must not have any spaces in it and the field
// should not have any parenthesis around it.
//
SdmBoolean
SdmSearch::IsValidKey(SdmError &err, struct _SdmSearchField &field)
{
  // first check parenthesis.
  if (field.parens == SdmSearch::Sdm_PAREN_Left) {
    err = Sdm_EC_PRS_IllegalLeftParenthesisKeyField;
    return Sdm_False;
  } else if (field.parens == SdmSearch::Sdm_PAREN_Right) {
    err = Sdm_EC_PRS_IllegalRightParenthesisKeyField;
    return Sdm_False;
  }
  
  // next check that there are no spaces in the field.
  SdmString key = field.key;  
  SdmStringLength len = key.Length();
  for (SdmStringLength i=0; i<len; i++) {
    if (key(i) == ' ' || key(i) == '\t') {
      err = Sdm_EC_PRS_IllegalKeyField;
      return Sdm_False;
    }
  }
  return Sdm_True;
}

//
// ** ET **
// GetParenthesisType checks the given string and returns
// whether it has a single left paren, a single right
// paren, or both parenthesis.
//
SdmParenthesisType 
SdmSearch::GetParenthesisType(char *str) 
{
  SdmParenthesisType ptype;
  char *cp;

  ptype = SdmSearch::Sdm_PAREN_Zero;
  if (*str == '(') {
    ptype = SdmSearch::Sdm_PAREN_Left;
  }
  if (*(cp = GetLastCharacter(str)) == ')') 
    if (ptype == SdmSearch::Sdm_PAREN_Left)
      ptype = SdmSearch::Sdm_PAREN_Both;
    else
      ptype = SdmSearch::Sdm_PAREN_Right;
  return ptype;
}


// ** ET
// CreateToken determines if given string stores a token.  It loops
// through search_tokens to determine if it is a token.  If found, it
// creates a copy of the token, sets up the NOT binding for it, and
// returns the copy.  The caller is responsible for deleting the result.
//
SdmErrorCode
SdmSearch::CreateToken(SdmError &err, SdmSearchToken *&r_token, char* token_str)
{
  err = Sdm_EC_Success;
  
  int i;
  SdmParenthesisType ptype;
  char *cp;
  char str[gkMaxParseBufferLength], *strptr= str;
  SdmBoolean not;            // ** ET - indicates whether not is found around token.
  SdmBoolean before, after;  // ** ET - indicates whether the not found is 
                             //          before and/or after the token.

  const SdmSearchToken *tlist = search_tokens; // ** ET - tlist points a list of all 
                                               //          the valid tokens.

  // ET - check length of token.  this should not exceed gkMaxParseBufferLength.
  if (strlen(token_str) > gkMaxParseBufferLength-1) {
    err = Sdm_EC_PRS_IllegalSearchToken;
    return err;
  }
  
  strcpy(str, token_str);	        /* local copy */

  if (*str == '~') {			/* ~(TOKEN --> TOKEN/~TOKEN -->TOKEN*/
    before = Sdm_True;
    not = Sdm_True;
    ++strptr;
  } else {
    not = Sdm_False;
    before = Sdm_False;
  }
  
  ptype = GetParenthesisType(strptr);		/* now parens check */
  
  // ** ET - strip out any parenthesis around the token.
  StripParenthesis(&strptr, ptype);     
  
  if (*strptr == '~') {			/* ~TOKEN --> TOKEN */
    after = not = Sdm_True;
    ++strptr;				/* skip the ~ */
  } else {
    after = Sdm_False;
  }

  // ** ET
  // This loop goes through token list to try to find a match for the given token.
  // When found, it checks that the token is syntactically correct and sets up
  // the NOT binding for the token (eg. not bound to token or expression or none.
  //
  for (i = 0; tlist[i].token; ++i) {
    if (strcmp(strptr, tlist[i].token) == 0) {        // ** ET : token match found.

      if (tlist[i].field_required == Sdm_True && ptype == SdmSearch::Sdm_PAREN_Right) {
         err =  Sdm_EC_PRS_IllegalRightParenthesisToken;
         return err;			/* <token>) <field> illegal */
      }
      
      // ** ET - create a copy of the token entry which gets return
      // to the caller.
      SdmSearchToken *copy_token = new SdmSearchToken;
      *copy_token = tlist[i];

      // ** ET - set the paren type for the token.
      copy_token->parens = ptype;
      
      // ** ET - set the NOT binding for the token.
      /* fix our not binding for this token */
      if (not) {
        if (before) { /* ~exp OR ~(..) */
          if (ptype == SdmSearch::Sdm_PAREN_Left) {
            copy_token->expression_not = Sdm_NOB_BindExpression; /* ~( */
          } else {
            copy_token->expression_not = Sdm_NOB_NoBinding; /* ~exp */
            copy_token->atomic_not = Sdm_NOB_BindAtom;
          }
        } else {			/* NOT before */
            copy_token->expression_not = Sdm_NOB_NoBinding; /* (<exp> */
        }

        if (after) {			/* ~(~ OR (~ */
          copy_token->atomic_not = Sdm_NOB_BindAtom;
        }

        /* The weird case of ~(~ATOMIC_EXPRESSION) == ATOMIC_EXPRESSION */
        if (ptype == SdmSearch::Sdm_PAREN_Both && before && after) {
          copy_token->atomic_not = Sdm_NOB_NoBinding;
          copy_token->expression_not = Sdm_NOB_NoBinding;  // ** ET - added.  redundant but more clear.
        }
        
      } else {    // ** ET - "not" not found in token.  set both to Sdm_NOB_NoBinding.
          copy_token->atomic_not = copy_token->expression_not = Sdm_NOB_NoBinding;
      }
          
      r_token = copy_token;
      return err;       // ** ET - we're done; return.
      
    } /* end if */
  }  /* end for */
    
  err =  Sdm_EC_PRS_IllegalSearchToken;
  return err;
}

//
// ** ET
// FindBoolean goes through the static list of boolean operators
// (gkBooleanList) and tries to match token with an operator
// in this list.  If found, it returns the boolean operator to the
// caller.  Otherwise, Sdm_BOP_None is returned.  Nothing is allocated 
// on the heap.  No reference to the static list is returned.  No
// change is made to the static list gkBooleanList.
//
SdmBooleanOperator
SdmSearch::FindBoolean(char *token)
{
  const SdmBooleanOperatorStructure *blist = gkBooleanList;
  int i;
  for (i=0; blist[i].logic_op; ++i) {
    if (strcmp(token, blist[i].logic_op) == 0) {
      return blist[i].bool;
    }
  }
  return Sdm_BOP_None;
}

/* select the field following a token. 
   o (<field>) and <field>) are legal.
   o (<field>  is not legal.
   If parenthesis are included in a field, then the field must be of
   the form "<expression>". 
*/
//
// ** ET  - keep this function as is.
// Sets fptr below to point to field if field is legal.  If field is
// surrounded by parens or double quotes, they are included in the field.
// Sets field_length to be the length of the field.
// Returns error code indicating whether field is legal.
//
SdmErrorCode
SdmSearch::SelectField(SdmError &err, 
  struct _SdmSearchField *fptr,         // updated: the field structure.
  char *field,                // input: the field string.
  int *complete_field_len)    // output: the length of the field 
                              // including double quotes.
{
  err = Sdm_EC_Success;
  char c;
  int loc_len= 0;
  char *loc_str = NULL;
  char *str= field;
  SdmParenthesisType ptype;
  char *cp;
  
  /* Anything left? */
  if (!*field) {
    err =  Sdm_EC_PRS_MissingField;
    return err;
  }

  /* place null at end of string and get beginning of field.
   * loc_str may be adjusted past a leading "(" or leading ^C.
   * loc_len will contain the number of white spaces and parens
   * skipped over in the string.
   */
  if (SetFormEnd(err, loc_str, str, &loc_len)) {
    // ** ET - what's the error in this case??
    // It was already set in SetFormEnd.
    return err;
  }
  
  /* check the parenthesis type. */    
  ptype = GetParenthesisType(str);		/* must include all parens */
  switch (ptype) {
    case SdmSearch::Sdm_PAREN_Zero:
      fptr->parens = SdmSearch::Sdm_PAREN_Zero;
      break;
    case SdmSearch::Sdm_PAREN_Left:
      // ** ET - illegal to have (<field>
      err =  Sdm_EC_PRS_IllegalLeftParenthesisField;
      return err;
    case SdmSearch::Sdm_PAREN_Right:
      fptr->parens = SdmSearch::Sdm_PAREN_Right;		/* <token> <field>) terminates */
      break;
    case SdmSearch::Sdm_PAREN_Both:			/* left and right */
      fptr->parens = SdmSearch::Sdm_PAREN_Both;		/* nullifly one another */
      break;
  }

  /* Is it a \". Note that any surrounding parenthesis have
     already been counted in SetFormEnd(). */
  if (*loc_str == '"') {			/* quoted field */
    ++loc_str;
    loc_len += 1;
    while (c = *loc_str++) {		/* search for '"' termination */
      loc_len += 1;
      if (c == '"') {
        *complete_field_len = loc_len;    // ** ET - then length of the
                                  // field does include the double quotes
        fptr->field = str;
        return err;
      }
    }
    err =  Sdm_EC_PRS_MissingDoubleQuote;
    return err;
  } else if (*loc_str == '~') {
    err =  Sdm_EC_PRS_IllegalNotBeforeField;
    return err;
  }
      
  /* Take next variable, ie, break on white-space or end-of-string */
  while (c = *loc_str++) {
    if (c == ' ' || c == '\t' || c == ')') 
      break;
    else 
      loc_len += 1;
  }
  *complete_field_len = loc_len;
  fptr->field = str;
  return err;
}


// 
// Methods for creating SdmLeaf tree.
//
void
SdmSearch:: AddLeafToTree(
     SdmSearchLeaf **new_leaf,        // output: new leaf
     struct _SdmSearchToken *token_ptr,// input: token.
     struct _SdmSearchField *fptr,     // input: structure.
     SdmBooleanOperator bool_op)    // input: operator for leaf.
{
  SdmSearchLeaf *leaf= (SdmSearchLeaf *)malloc(sizeof(SdmSearchLeaf));    
  SdmSearchLeaf *vert;
  SdmBoolean need_end;

  leaf->sl_predicate = MakePredicate(fptr->field, fptr->key, 
				   token_ptr->field_required,
				   token_ptr->field_attributes,
				   token_ptr->token);

  leaf->sl_leaf_macro = NULL;
  leaf->sl_uses_macros = Sdm_False;
  leaf->sl_new_root = NULL;

  leaf->sl_logic_op = bool_op;
  leaf->sl_expression = GetLeafState(token_ptr->parens, fptr->parens);
  leaf->sl_atomic_not = token_ptr->atomic_not;
  leaf->sl_expression_not = token_ptr->expression_not;

  // Set the predicate type to Sdm_SLE_Simple (no field required) OR field.
  leaf->sl_predicate_type = (token_ptr->field_required ? Sdm_PRED_Field : Sdm_PRED_Simple);

  // some predicate fields are dynamic at search time.
  // ** ET - dynamic fields are those associated with dates. 
  if (leaf->sl_predicate_type == Sdm_PRED_Field &&
      token_ptr->field_attributes & Sdm_SFA_Time)   
  {
    leaf->sl_predicate_type = Sdm_PRED_DynamicField;
  }

  // Adjoin the leaf to the tree 
  if (!*new_leaf) {
    *new_leaf = leaf;			// the head 
  } else {				// find last leaf 
    vert = *new_leaf;
    if (vert->sl_expression == Sdm_SLE_Start)	// current expression type 
      need_end = Sdm_True;
    else
      need_end = Sdm_False;

    // ** ET 
    // the folloiwng loop goes through each of the leaves and
    // determines whether we have a start that needs a matching end.      
    while (vert->sl_next) {
      vert = vert->sl_next;
      switch (vert->sl_expression) {
        case Sdm_SLE_Simple:
        case Sdm_SLE_Intermediate:
            break;
        case Sdm_SLE_Start:
            need_end = Sdm_True;
            break;
        case Sdm_SLE_End:
            need_end = Sdm_False;
            break;
      }
    }
    //to add this leaf 
    vert->sl_next = leaf;	
    
    // if leaf is in a chain with a start and no end,
    // then leaf is Sdm_SLE_Intermediate 
    if (need_end && leaf->sl_expression != Sdm_SLE_End)
      leaf->sl_expression = Sdm_SLE_Intermediate;
  }
  // to terminate the tree 
  leaf->sl_next = NULL;		
}

/* make <token> or <token> "<field>" as a string */
// ** ET
// MakePredicate combines the token and the field into one string
// and returns it.  The predicate string is allocated on the heap
// and returned to the caller.  Caller owns this string and is 
// responsible for it's destruction.
//
char*
SdmSearch::MakePredicate(
     const SdmString &field,     // input: field string
     const SdmString &key,       // input: key field for key/value pairs
     SdmBoolean needs_field,     // input: boolean indicating if token
                                 // requires field.
     unsigned long attributes,   // input: field attributes
     const char *token)          // input: token
{
  int len;
  SdmBoolean quoted;
  char* predicate;

  if (!needs_field) {
    predicate = strdup(token);
    return predicate;
  }

  if (field(0) == '"')
    quoted = Sdm_True;
  else 
    quoted = Sdm_False;
  
  // need to create key with space after it to separate it from field in predicate.
  SdmString newKey;
  if (key.Length() > 0) { 
    newKey = key; 
    newKey += " ";
  }

  /* need space for NULL SPACE ^C and possibly "" */
  len = strlen(token) + field.Length() + 8 + newKey.Length();
  predicate = (char*) malloc(len);
  
  // ** ET - if the field already has quotes, we don't need to add
  // them.  we only add them if the field does not have quotes and
  // it is not an atomic field.    
  if (quoted || attributes & Sdm_SFA_Atomic) {
      sprintf(predicate,"%s %s%s", token, (const char*)newKey, (const char*)field);
  } else {
      sprintf(predicate,"%s %s\"%s\"", token, (const char*)newKey, (const char*)field);
  }
  return predicate;
}


/*
 * set leaf state given parenthesis type of token and field
    <token>            -->                 SIMPLE_EXPRESSION
   (<token>)           -->                 SIMPLE_EXPRESSION
   (<token>) (<field>) --> <token> <field> SIMPLE_EXPRESSON 
   (<token>   <field>) --> <token> <field> SIMPLE_EXPRESSON 
   <token>   (<field>) --> <token> <field> SIMPLE_EXPRESSON 

   (<token>   <field>  -->                 START_EXPRESSION
    <token>   <field>) -->                 END_EXPRESSION 

   *** Others illegal and illiminated in parse ***
 */
// ** ET 
// keep function as is.
// Rename function to remove french name.  change to
// GetLeafState.
SdmSearchLeafExpressionType
SdmSearch::GetLeafState(
  SdmParenthesisType token_ptype,
  SdmParenthesisType field_ptype)
{
  switch (token_ptype) {
    case SdmSearch::Sdm_PAREN_Both:			/* (<expression>) */
    case SdmSearch::Sdm_PAREN_Zero:			/*  <expression> */
      break;				/* field determines result */
    case SdmSearch::Sdm_PAREN_Left:
      if (field_ptype == SdmSearch::Sdm_PAREN_Right)
        return Sdm_SLE_Simple;
      else
        return Sdm_SLE_Start;
    case SdmSearch::Sdm_PAREN_Right:			/* <token>) with no field */
      return Sdm_SLE_End;
  }
  /* "case SdmSearch::Sdm_PAREN_Left:" is impossible */
  switch (field_ptype) {
    case SdmSearch::Sdm_PAREN_Both:			/* (<expression>) */
    case SdmSearch::Sdm_PAREN_Zero:			/*  <expression> */
      return Sdm_SLE_Simple;
    case SdmSearch::Sdm_PAREN_Right:
      return Sdm_SLE_End;
  }
}


// ** ET
// AdvanceFilter moves filter pointer up the given "len" number
// of characters.  returns the new pointer to the filter.  filter
// pointer itself is not changed because it is passed in by value.
//
char *
SdmSearch::AdvanceFilter(char *filter, int *filter_len, int len)
{
  *filter_len -= len;			/* decrement by string length  */
  if (*filter_len > 1) {			/* strtok NULLS out one char */
    filter += len + 1;			/* next list item */
    *filter_len -= 1;			/* advance one char more */
  } else
    filter += len;			/* points to NULL termination */
  return filter;
} 

/* return pointer to last char in a string */
char *
SdmSearch::GetLastCharacter(char *str)
{
  int len = strlen(str);

  return &str[len - 1];
}



/* 
   place null at end of field, and return pointer to beginnig
   of the field which may skip a '('.
   n_excess will count surrounding parenthesis and any whitespace
   we step on to terminate the field.
*/
//
// ** ET - keep function as is.
// field is unchanged.  n_excess is modified to contain the count of
// the parenthesis and whitespaces that was skipped to terminate the field.
SdmErrorCode
SdmSearch::SetFormEnd(SdmError &err, char *& r_field_start, char *field, int *n_excess)
{
  err = Sdm_EC_Success;
  char c, *cp= field, *cp0= field;
  r_field_start = NULL;

  if (*cp == '(') {
    ++cp;				/* skip parens */
    ++cp0;
    *n_excess = 1;
  } else
    *n_excess = 0;

  if (*cp == '"') {			/* search for '"' */
    ++cp;				/* skip initial '"' */
    while (c = *cp++) {
      if (c == '"') {
        if (*cp == ')') {		/* "..") is legal */
          ++cp;
          ++*n_excess;			/* parenthesis are excess */
        }
        *cp = '\0';			/* tie it off; puts null after double quote or ')' */
        r_field_start = cp0;
        return err;			/* found our matching " */
      }
    }
    // return error if resulting double quote is missing.
    err =  Sdm_EC_PRS_MissingDoubleQuote;
    return err;			/* This should never happen */
  }
  /* OK, search for whitespace or a terminating ')' */
  while (c = *cp) {
    if (c == ' ' || c == '\t' || c == ')') {
      if (c == ')') {
        ++cp;				/* don't step on it. */
        ++*n_excess;
        if (*cp != ' ' && *cp != '\t' && *cp != '\0') {
          /* MUST have ') ' or end of string */
          err =  Sdm_EC_PRS_IllegalRightParenthesisTermination;
          return err;
        }
      }
      if (*cp != '\0') {		/* end of field here */
        *cp = '\0';			/* tie it off */
        ++n_excess;			/* stepped on white-space */
      }
      break;
    }
    ++cp;
  }
  r_field_start = cp0;
  return err;
}

//*****************************************
//
// Methods for expanding macros in leaves.
//
//*****************************************

// Run through each petal. If no macro expansion is there, then
// return.
//
// ** ET - SetMacroForLeaf goes through the leaves
// and sets the leaf_macro field if the leave is dynamic
// and uses a macro to determine its value (eg. "BEFORE Today").
//
void
SdmSearch::SetMacroForLeaf(SdmSearchLeaf *f)
{
  SdmSearchLeaf *petal = f;
  const SdmSearchLeafMacro *lm;
  int mexp = 0;

  while (petal) {
    if (!petal->sl_leaf_macro && (lm = GetMacroForLeaf(petal))){
      // initialize the macro leaf
      petal->sl_leaf_macro = lm;
      mexp += 1;
    }
    petal = petal->sl_next;
  }
  /* See if either we expanded, OR a canned leaf_macro is there */
  if (mexp > 0 || f->sl_leaf_macro)
    f->sl_uses_macros = Sdm_True;
  else
    f->sl_uses_macros = Sdm_False;
}

// ** ET
// GetMacroForLeaf searches for a macro used to expand the given leaf.  
// goes through global static table "gkMacroElementTable" and searches for
// the macro that matches the leaf.  If found, the leaf macro
// is returned.  It is ok to return these macros because they are
// never modified by the caller.  Therefore, there is no problem
// here for multi-thread access.  
//
const SdmSearchLeafMacro * 
SdmSearch::GetMacroForLeaf( SdmSearchLeaf *f )
{
  const SdmSearchLeafMacro *lm = gkMacroElementTable;

  for (int i = 0; lm->slm_macro_name; i++, lm++) {
    if (strcasecmp(lm->slm_macro_name, f->sl_predicate) == 0) {
      return lm;
    }
  }

  return NULL;
}



//
// Combine ordinary leafs and macros into one
// search tree.  
//
// ** ET 
// Checks if macro is used to expand any leaves in the tree.
// If yes, then creates combined tree in "new_root" field for the leaf f.
// The combined tree has all the leaves that is in the original tree with
// additional leaves added as a result of the macro expansion for
// the leaf macros.
//
void
SdmSearch::CombineSearchTree(SdmSearchLeaf *f)
{
  // do nothing if there are no macros in this tree.
  if (!f->sl_uses_macros) return;

  // OK, make a new tree if NOT there at the root
  if (f->sl_new_root) {
    return;
  }
  else {
    SdmSearchLeaf *tmp, *f0 = f;
    SdmSearchLeaf *new_root, **leaf_ptr = &new_root;    // ** ET - new_root is pointer to new root.
                               // leaf_ptr is used to traverse down the tree beginning at 
                               // new root and set up the new leaves.
    while (f) {
      if (f->sl_leaf_macro) {
        const SdmSearchLeafMacro *lm = f->sl_leaf_macro;
        // Duplicate and adjoin each macro
        int i;
        SdmSearchLeaf *lf = lm->slm_leaves;
        
        // ** ET 
        // go through the leaves of the macros.  create copy of each
        // leave and add it to the new tree at root new_root.
        for (i = 0; i < lm->slm_leaf_count; ++i) {
          // ** ET - tmp and leaf_ptr points to new leaf here.
          tmp = *leaf_ptr = (SdmSearchLeaf *)malloc(sizeof (SdmSearchLeaf));  
          
          // ** ET - after duplicate call, leaf_ptr is set to point to the
          // next leaf after the new leaf we just created 
          // (leaf_ptr = (&tmp->sl_next)).  this is how the leaves get added
          // to the current leaf f.
          leaf_ptr = CopyLeaf(lf, leaf_ptr, f, (i == 0) ? Sdm_True : Sdm_False);
          
          // Determine expression type here.
          SdmBoolean exp_end = (i+1 == lm->slm_leaf_count && f->sl_expression == Sdm_SLE_End ?
                     Sdm_True : Sdm_False);
          SdmBoolean exp_start = (i == 0 && f->sl_expression == Sdm_SLE_Start?
                       Sdm_True : Sdm_False);
                       
          if (exp_end) {
            tmp->sl_expression = Sdm_SLE_End;
          } else if (exp_start) {
            tmp->sl_expression = Sdm_SLE_Start;
          } else if (f->sl_expression == Sdm_SLE_Intermediate) {
            tmp->sl_expression = Sdm_SLE_Intermediate;
          } else {
            tmp->sl_expression = Sdm_SLE_Simple;
          }
          lf++;
        }
      } 
      else {
        // ** ET 
        // if this leaf is not a macro, just copy it completely and
        // add it to the new root.
        tmp = *leaf_ptr = (SdmSearchLeaf *)malloc(sizeof (SdmSearchLeaf));
        leaf_ptr = CopyLeaf (f, leaf_ptr, NULL, Sdm_False);
        tmp->sl_expression = f->sl_expression;
      }
      f = f->sl_next;
    }
    f0->sl_new_root = new_root;
    return;
  }
}

//
// duplicate src in *dst, and return ptr to 
// (*dst)->sl_next
//
SdmSearchLeaf **
SdmSearch::CopyLeaf(SdmSearchLeaf *src, SdmSearchLeaf **dst, SdmSearchLeaf *f, SdmBoolean dup_f)
{
  SdmSearchLeaf *dst0 = *dst;
  // Duplicate values
  dst0->sl_predicate = strdup(src->sl_predicate);
  dst0->sl_leaf_macro = NULL;
  dst0->sl_uses_macros = Sdm_False;
  dst0->sl_new_root = NULL;
  if (f && dup_f) {
    dst0->sl_logic_op = f->sl_logic_op;
    dst0->sl_atomic_not = f->sl_atomic_not;
    dst0->sl_expression_not = f->sl_expression_not;
  } else {
    dst0->sl_logic_op = src->sl_logic_op;
    dst0->sl_atomic_not = src->sl_atomic_not;
    dst0->sl_expression_not = src->sl_expression_not;
  }
  dst0->sl_predicate_type = src->sl_predicate_type;
  dst0->sl_next = NULL;
  // Return **link
  return &dst0->sl_next;
}
