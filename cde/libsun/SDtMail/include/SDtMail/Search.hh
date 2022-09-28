/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Search Object Class.

#ifndef _SDM_SEARCH_H
#define _SDM_SEARCH_H

#pragma ident "@(#)Search.hh	1.46 96/07/03 SMI"

// Include Files
#include <thread.h>
#include <SDtMail/Sdtmail.hh>

enum SdmBooleanOperator {
  Sdm_BOP_None = 1,
  Sdm_BOP_And = 2,
  Sdm_BOP_Or = 3
} ;


/* expression types for leafs of our tree */
enum SdmSearchLeafExpressionType {
  Sdm_SLE_Simple = 1,         /* NO parenthetial influences */
  Sdm_SLE_Start = 2,          /* left paren influence */
  Sdm_SLE_Intermediate = 3,   /* start < exp < end */
  Sdm_SLE_End = 4             /* right paren influence */
} ;
 
/* How '~" binds with the leaf
 *   o bind atom means this "Sdm_SLE_Simple" expression
 *   o bind expression means this expression
 *       if SdmSearchLeafExpressionType is "Sdm_SLE_Simple" 
 *             then same as Sdm_NOB_BindAtom.
 *       if SdmSearchLeafExpressionType is "Sdm_SLE_Start" 
 *              then upto the next end leaf.
 */
enum SdmNotOperatorBinding {
  Sdm_NOB_NoBinding = 1,
  Sdm_NOB_BindAtom = 2,
  Sdm_NOB_BindExpression = 3
} ;


/* This defines the type of predicate in the leaf */
enum SdmSearchPredicateType {
  Sdm_PRED_Simple = 1,        /* predicate does not contain a field */
  Sdm_PRED_Field = 2,         /* predicate contains a extern  field */
  Sdm_PRED_DynamicField = 3   /* predicate contains a dynamic field */
} ;

// Forward declarations for leaf macro is needed because it's used in SdmLeaf.
struct _SdmLeafMacro;
#define SdmSearchLeafMacro struct _SdmLeafMacro


/* 
 * leaves of search parse tree:
 * paren_type = Sdm_PAREN_Left if the leaf is preceded by a left parenthesis,
 *            = Sdm_PAREN_Right if the leaf is followed by a right guy.
 */
typedef struct _SdmLeaf {
  char* sl_predicate;     /* search predicate which contains the token and
                            * field (if applicable) */
 
  /* The following 3 fields are used in expanding leaf macros */
  const SdmSearchLeafMacro      *sl_leaf_macro;  /* macro equivalent of predicate */
  SdmBoolean                    sl_uses_macros;  /* Set in root node if macro used */
  struct _SdmLeaf               *sl_new_root;    /* Expanded new tree a root node */
 
  SdmBooleanOperator            sl_logic_op;      /* predeceding operator */
  SdmSearchLeafExpressionType   sl_expression;    /* Sdm_SLE_Simple, Sdm_SLE_Start, or Sdm_SLE_End */
  SdmNotOperatorBinding         sl_atomic_not;    /* How ~ binds with this expression */
  SdmNotOperatorBinding         sl_expression_not;/*  How binds with ~(<expression>)*/
  SdmSearchPredicateType        sl_predicate_type;/* simple_pred or field_pred */
  struct _SdmLeaf               *sl_next;         /* link to next leaf */

} SdmSearchLeaf;


// Actual definition for the leaf macro.
struct _SdmLeafMacro {
  char *slm_macro_name;         /* text name of this macro */
  SdmSearchLeaf *slm_leaves;    /* resulted search string */
  int  slm_leaf_count;          /* no. of petals in the leaf */
};
 

// forward declarations for internal structures.
struct _SdmSearchToken;
struct _SdmSearchField;


class SdmSearch {

public:
  SdmSearch();
  virtual ~SdmSearch();

  SdmErrorCode GetSearchString(SdmError &err, SdmString &r_string) const;
  SdmErrorCode SetSearchString(SdmError &err, const SdmString &string);

  // Parses search string.  Returns list of leaves in r_leaf representing 
  // the search if createLeafList is true.  Otherwise, ParseSearch only
  // parses the search and returns error code indicating whether the
  // parse was successful.
  //
  SdmErrorCode ParseSearch(SdmError &err, SdmSearchLeaf *&r_leaf, 
                  const SdmBoolean createLeafList = Sdm_True);
  
  static void FreeLeaf(SdmSearchLeaf *);
  static void PrintLeaf(SdmSearchLeaf *);

  //
  // search tokens
  //
  
  // tokens that require text field
  //
  static  const char* Sdm_STK_To;
  static  const char* Sdm_STK_From;
  static  const char* Sdm_STK_CC;
  static  const char* Sdm_STK_BCC;
  static  const char* Sdm_STK_Subject;
  static  const char* Sdm_STK_Text;
  static  const char* Sdm_STK_Header;    // Header token requires 2 text fields.
  static  const char* Sdm_STK_Body;
  
  // tokens that require date field
  //
  static  const char* Sdm_STK_Before;
  static  const char* Sdm_STK_Since;
  static  const char* Sdm_STK_On;
  static  const char* Sdm_STK_During;
  static  const char* Sdm_STK_SentSince;
  static  const char* Sdm_STK_SentBefore;
  static  const char* Sdm_STK_SentOn;
  
  // tokens that require numeric field.
  //
  static  const char* Sdm_STK_Larger;
  static  const char* Sdm_STK_Smaller;
  
  // atomic tokens that do not require field.
  //
  static  const char* Sdm_STK_New;
  static  const char* Sdm_STK_Old;
  static  const char* Sdm_STK_Deleted;
  static  const char* Sdm_STK_Undeleted;
  static  const char* Sdm_STK_Read;
  static  const char* Sdm_STK_Unread;
  static  const char* Sdm_STK_Recent;
  static  const char* Sdm_STK_Flagged;
  static  const char* Sdm_STK_Unflagged;
  static  const char* Sdm_STK_Answered;
  static  const char* Sdm_STK_Unanswered;
  static  const char* Sdm_STK_Seen;
  static  const char* Sdm_STK_Unseen;
  static  const char* Sdm_STK_Keyword;
  static  const char* Sdm_STK_Unkeyword;
  static  const char* Sdm_STK_Draft;
  static  const char* Sdm_STK_Undraft;
  static  const char* Sdm_STK_All;

  // dynamic date fields.  these can be used with the date tokens
  // in place of actual dates.
  // 
  static  const char* Sdm_DDF_January;
  static  const char* Sdm_DDF_February;
  static  const char* Sdm_DDF_March;
  static  const char* Sdm_DDF_April;
  static  const char* Sdm_DDF_May;
  static  const char* Sdm_DDF_June;
  static  const char* Sdm_DDF_July;
  static  const char* Sdm_DDF_August;
  static  const char* Sdm_DDF_September;
  static  const char* Sdm_DDF_October;
  static  const char* Sdm_DDF_November;
  static  const char* Sdm_DDF_December;
  static  const char* Sdm_DDF_ThisMonth;
  static  const char* Sdm_DDF_LastMonth;
  static  const char* Sdm_DDF_LastWeek;
  static  const char* Sdm_DDF_Today;
  static  const char* Sdm_DDF_Yesterday;
  static  const char* Sdm_DDF_Sunday;
  static  const char* Sdm_DDF_Monday;
  static  const char* Sdm_DDF_Tuesday;
  static  const char* Sdm_DDF_Wednesday;
  static  const char* Sdm_DDF_Thursday;
  static  const char* Sdm_DDF_Friday;
  static  const char* Sdm_DDF_Saturday;


  /* leaf states */
  enum SdmParenthesisType {
    Sdm_PAREN_Zero = 0,
    Sdm_PAREN_Left = 1,
    Sdm_PAREN_Right = 2,
    Sdm_PAREN_Both = 3
  };
  
protected:
  
  // all the parsing subroutines    
  void AddLeafToTree(SdmSearchLeaf **, struct _SdmSearchToken *, 
                struct _SdmSearchField *, SdmBooleanOperator op);
                
  char* MakePredicate(const SdmString &, const SdmString&, SdmBoolean, unsigned long, const char *token);
  char *AdvanceFilter(char *, int *, int);
  SdmErrorCode CreateToken(SdmError &err, struct _SdmSearchToken *&r_token, char *token_str);
  SdmBooleanOperator FindBoolean( char *token );
  SdmErrorCode InternalParseSearch(SdmError &err, SdmSearchLeaf *&r_leaf, 
                  const SdmBoolean createLeafList);
  SdmErrorCode SelectField(SdmError &err, struct _SdmSearchField *, char *, int *);
  SdmErrorCode SetFormEnd(SdmError &err, char *&r_field_start, char *, int *);
  
  // methods for getting leaf macro for leaves.
  void SetMacroForLeaf(SdmSearchLeaf *f);
  const SdmSearchLeafMacro* GetMacroForLeaf( SdmSearchLeaf *f );
  void CombineSearchTree(SdmSearchLeaf *f);
  SdmSearchLeaf** CopyLeaf(SdmSearchLeaf *src, SdmSearchLeaf **dst, SdmSearchLeaf *f, 
          SdmBoolean dup_f);

  static  SdmSearchLeafExpressionType GetLeafState(SdmParenthesisType, SdmParenthesisType);
  static  void MakeUpperSearchString( char * );
  static  char *GetLastCharacter(char *);
  static  SdmBoolean IsNumericField(SdmError &err, const char *);
  static  SdmBoolean IsValidKey(SdmError &err, struct _SdmSearchField &field);
  static  SdmParenthesisType GetParenthesisType(char *str);
  static  void StripNotOperators ( char **);
  static  void StripParenthesis(SdmString&, SdmParenthesisType);
  static  void StripParenthesis(char **, SdmParenthesisType);
  static  void StripQuotes(SdmString&);
  static  void FixNotOperators(char *);
  static  int FixWhiteSpaces( char *);
  static  SdmBoolean IsParenthesisBalanced(char *);

public:

private:
  SdmString _CurrentSearchString;        // the string to parse  
  
  // lock for exclusive access to search string.
  mutex_t   _SearchStringLock;   
};

#endif
