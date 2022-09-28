/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This file contains all the methods in SdmDpDataChanCclient which
//     are used to handle searching.

#pragma ident "@(#)SearchEngine.cc	1.17 96/11/04 SMI"

// Include Files
#include <PortObjs/DataChanCclient.hh>
#include <PortObjs/DynamicPredicateUtility.hh>
#include <string.h>
#include <ctype.h>
#include <Utils/CclientUtility.hh>
 
#define Sdm_BitwiseTrue 1

// 
// ** ET
// SdmDynamicPredicate defines the mapping between the dynamic predicate
// (eg. "TODAY") and the macro used to calculate the value for the 
// predicate (eg. SetToday).
//
typedef struct {
  const char *value;				/* dynamic predicate name */
  SdmErrorCode (*set_dynamic)(SdmError &err, SdmString &r_value, char *str);		/* creates real predicate */
} SdmDynamicPredicate;


//
// ** ET
// gkDynamicPredicateMap maps keywords to methods used to get the value of them.
// These are dynamic and determined in UpdateDynamicPredicates method.
// Flaky: the methods use all take a char* for argument but it is never
// used.  should it be removed?
static const SdmDynamicPredicate gkDynamicPredicateMap[] = 
{
    "\"TODAY\"",
    &SdmDynamicPredicateUtility::SetToday,

    "\"YESTERDAY\"",
    &SdmDynamicPredicateUtility::SetYesterday,

    "\"LASTWEEK\"",
    &SdmDynamicPredicateUtility::SetLastWeek,

    "\"LASTMONTH\"",
    &SdmDynamicPredicateUtility::SetLastMonth,

    "\"THISMONTH\"",
    &SdmDynamicPredicateUtility::SetThisMonth,

    "\"JANUARY\"",
    &SdmDynamicPredicateUtility::SetJanuary,

    "\"FEBRUARY\"",
    &SdmDynamicPredicateUtility::SetFebruary,

    "\"MARCH\"",
    &SdmDynamicPredicateUtility::SetMarch,

    "\"APRIL\"",
    &SdmDynamicPredicateUtility::SetApril,

    "\"MAY\"",
    &SdmDynamicPredicateUtility::SetMay,

    "\"JUNE\"",
    &SdmDynamicPredicateUtility::SetJune,

    "\"JULY\"",
    &SdmDynamicPredicateUtility::SetJuly,

    "\"AUGUST\"",
    &SdmDynamicPredicateUtility::SetAugust,

    "\"SEPTEMBER\"",
    &SdmDynamicPredicateUtility::SetSeptember,

    "\"OCTOBER\"",
    &SdmDynamicPredicateUtility::SetOctober,

    "\"NOVEMBER\"",
    &SdmDynamicPredicateUtility::SetNovember,

    "\"DECEMBER\"",
    &SdmDynamicPredicateUtility::SetDecember,

    "\"SUNDAY\"",
    &SdmDynamicPredicateUtility::SetSunday,

    "\"MONDAY\"",
    &SdmDynamicPredicateUtility::SetMonday,

    "\"TUESDAY\"",
    &SdmDynamicPredicateUtility::SetTuesday,

    "\"WEDNESDAY\"",
    &SdmDynamicPredicateUtility::SetWednesday,

    "\"THURSDAY\"",
    &SdmDynamicPredicateUtility::SetThursday,

    "\"FRIDAY\"",
    &SdmDynamicPredicateUtility::SetFriday,

    "\"SATURDAY\"",
    &SdmDynamicPredicateUtility::SetSaturday,

    NULL,
    NULL,
};

// 
// Methods for expanding dynamic predicates (eg. Today)
//

// gets dynamic predicate for given leaf and returns string with
// expanded dynamic value.
// ** ET
// keep as is.
char *
SdmDpDataChanCclient::UpdateDynamicPredicates(SdmSearchLeaf *f)
{  
  char tmp[256];
  char *field;
  SdmString dynamic_value;
  const SdmDynamicPredicate *dyn;
  int len;
  char *newPredicate;
  SdmError err;

  if (f->sl_predicate_type != Sdm_PRED_DynamicField)
    return NULL;
    
  /* Isolate the field */
  field = strchr(f->sl_predicate,(int) ' ') + 1;
  
  /* dispatch on field value */
  for (dyn = gkDynamicPredicateMap; dyn->value; ++dyn) {
    if (strcasecmp(field, dyn->value) == 0) {
     (*dyn->set_dynamic)(err, dynamic_value, field);
      break;
    }
  }
  if (dynamic_value.Length() <= 0) {
    /* Dynamic values NOT required in dynamic fields */
    // ** ET
    // for example "ON" can take a dynamic field like "TODAY"
    // for a static value like "01-dec-95".
    return NULL;
  }
  /*
   * Remake predicate with dynamic value */
  strcpy(tmp, f->sl_predicate);
  strtok(tmp, " ");			/* NULL terminate predicate */  
  len = strlen(tmp) + dynamic_value.Length() + 2; /* SPACE and NULL too */
  newPredicate = (char *)malloc(sizeof(char) * len);
  sprintf(newPredicate, "%s %s", tmp, (const char*)dynamic_value);
  return newPredicate;
}

//*************************************************
//
// Methods for creating search program from leaves.
//
//*************************************************

//
// IMAP4 style search program used here:
// Turn search leafs into a program.
SdmErrorCode
SdmDpDataChanCclient::CreateSearchPgmForLeaf(SdmError err, SEARCHPGM *&top_sp, SdmSearchLeaf *result )
{
  if ( result == NULL ) {
     err = Sdm_EC_Fail;
     return err;
  }

  //
  // Convert leave to search pgm structure.
  //
  SdmSearchLeaf *pf;
  top_sp = CreateNewSearchPgm();

  // If macros are used, use the new root.
  // Otherwise, we can use the given leaf.
  if (result->sl_uses_macros) {
    // new root better not be NULL if we are using macros.
    assert(result->sl_new_root != NULL);
    pf = result->sl_new_root;
  } else {
    pf = result;
  }

  for (; pf;) {
    if (pf && pf->sl_logic_op == Sdm_BOP_Or) {
      // Create [top_sp OR pf]
      // CreateOrPgm points pf to first leaf that is NOT
      // an Sdm_BOP_Or
      SEARCHOR *or;
      
      // See if OR (<expression>)
      if (pf->sl_expression == Sdm_SLE_Start)
        or = CreatePgmForOrExpression(&pf, top_sp);
      else
        or = CreateOrPgm(&pf, top_sp, NULL);
        
      top_sp = CreateNewSearchPgm();
      top_sp->or = or;
      if (!pf) break;
      
    } 
    else {
      // See if AND <expression>
      if (pf->sl_expression == Sdm_SLE_Start) {
        top_sp = CreatePgmForAndExpression(&pf, top_sp);
      }
      else {
        // a new AND
        AddToSearchProgram(pf, top_sp);
        pf = pf->sl_next;
      }
    }
  }
  return err;
}

// Create an AND <expression> using pgm, and predicates
// of **l. 
// Here we use the De Morgan rules: 
//    A AND B == !(!A OR !B), A OR B == !(!A AND !B)
// BUT if the expression is NEGATED:
//    !(A AND B OR C) ==> !A OR !B AND !C.
// ** ET
// Very confusing.  keep method as is.
// This method is really confusing.  It uses De Morgan's rules to negate
// the expression represented by the given leaf.  It creates a demorganized
// tree for the given leaf.  If the leaf has NOT bound to the expression,
// then this tree can be directly added to "pgm".  Otherwise, the
// demorganized tree is added to the NOT field for "pgm".
// 
SEARCHPGM *
SdmDpDataChanCclient::CreatePgmForAndExpression(SdmSearchLeaf **l, SEARCHPGM *pgm)
{
  SdmSearchLeaf *pf = *l;
  SdmBoolean quit;
  SdmBoolean exp_NOT;
  SEARCHPGM *top_sp;
  // If we have NOT (<expression) then just
  // add demorganized to pgm.
  // otherwise, add it as a NOT pgm to a new
  // program.
  if (pf->sl_expression_not == Sdm_NOB_BindExpression) {
    top_sp = pgm;
    exp_NOT = Sdm_True;
  } else {
    top_sp = CreateNewSearchPgm();
    exp_NOT = Sdm_False;
  }
  // Make a DeMorgan copy of the expression:
  // pf will be updated to the leaf following 
  // the expression end
  SdmSearchLeaf *dem0, *dem = DeMorganizeExpression(&pf);
  // IF a in the passed SdmSearchLeaf quit.
  if (!dem) return pgm;
  dem0 = dem;
  // Process the demorganized tree
  for (;dem;) {
    if (dem && dem->sl_logic_op == Sdm_BOP_Or) {
      // Create [top_sp OR dem]
      SEARCHOR *or;
      // CreateOrPgm advances dem one leaf beyond
      // Sdm_BOP_Or terminating the "or".
      or = CreateOrPgm(&dem, top_sp, NULL);
      top_sp = CreateNewSearchPgm();
      top_sp->or = or;
    } else {
      // a new AND
      AddToSearchProgram(dem, top_sp);
      dem = dem->sl_next;
    }
  }
  *l = pf;
  
  // Free out demorganized SdmSearchLeaf
  SdmSearch::FreeLeaf(dem0);
  
  if (!exp_NOT) {
    // Add dem to the not list:
    SEARCHPGMLIST **not = &pgm->not;
    while ((*not)) not = &(*not)->next;
    *not = CreateNewSearchPgmList();
    (*not)->pgm = top_sp;
    return pgm;
  } else {
    // ** ET
    // Bug fix.  need to return top_sp instead of pgm.  if the expression
    // was ~(subject foo and subject bar), the second condition is
    // lost if pgm is returned.  pgm only contains the pgm structure
    // for the first condition (subject foo). you must return top_sp 
    // which contains the entire pgm list for the expression.  
    return top_sp;
  }
}
//
// Return a tree of leafs which are demorganized.
// We start with p->sl_expression == Sdm_SLE_Start, and finish with
// p->sl_expression == Sdm_SLE_End.
SdmSearchLeaf *
SdmDpDataChanCclient::DeMorganizeExpression(SdmSearchLeaf **l)
{
  SdmSearchLeaf *p = *l;
  SdmSearchLeaf *d0, *d = NULL;
  if (p->sl_expression != Sdm_SLE_Start) {
    return NULL;		// BUG! Should be Sdm_SLE_Start
  }
  
  while (p) {
    SdmSearchLeaf *tmp = (SdmSearchLeaf *)malloc(sizeof(SdmSearchLeaf));
    if (d == NULL) {
      d0 = d = tmp;
      // First leaf has no operation. It initializes
      // the program.
      d->sl_logic_op = Sdm_BOP_None;
    } else {
      d->sl_next = tmp;
      d = d->sl_next;
      // reverse the operation
      d->sl_logic_op = (p->sl_logic_op == Sdm_BOP_Or ? Sdm_BOP_And : Sdm_BOP_Or);
    }
    // Now initialize the fields.
    d->sl_predicate = strdup(p->sl_predicate);
    d->sl_leaf_macro = NULL;
    d->sl_new_root = NULL;
    d->sl_uses_macros = Sdm_False;
    // Reverse the atomic not binding
    d->sl_atomic_not = (p->sl_atomic_not == Sdm_NOB_NoBinding ? Sdm_NOB_BindAtom :
		     Sdm_NOB_NoBinding);
    d->sl_expression_not = Sdm_NOB_NoBinding; // not used
    d->sl_expression = p->sl_expression;
    d->sl_predicate_type = p->sl_predicate_type;
    d->sl_next = NULL;

    if (p->sl_expression == Sdm_SLE_End) break;
    else p = p->sl_next;
  }
  // Successor of the end
  *l = p->sl_next;
  return d0;
}
//
// Here we have: first OR (<expression>)
//  <expression> := p(1),...,p(n), n > 1, with 
//   p(1)->sl_expression == Sdm_SLE_Start, p(n)->sl_expression == Sdm_SLE_End,
//   
// Return: or->first = first, 
//         or->second = <search program of expression>
SEARCHOR *
SdmDpDataChanCclient::CreatePgmForOrExpression(SdmSearchLeaf **l, SEARCHPGM *first)
{
  SEARCHOR *top_or = CreateNewSearchOr();
  SdmBoolean quit;
  // ** ET
  // Bug fix.  fix memory leak.  delete top_or->first before
  // setting it to something else below.
  delete top_or->first;
  top_or->first = first;
  
  // Scan expression until expression end
  SdmSearchLeaf *pf = *l;
  
  // Initialize progam with pf. We will have
  // pf OP1 pf->sl_next ... OPN pf->(expression end)
  AddToSearchProgram(pf, top_or->second);
  pf = pf->sl_next;
  
  // Combine macro leafs with ordinary leafs to
  // build a search tree. 
  for (quit = Sdm_False; pf;) {
    // ** ET
    // don't need to check pf != NULL below.
    if (pf->sl_logic_op == Sdm_BOP_Or) {
      // Create [top_sp OR pf]
      // CreateOrPgm points pf to first leaf that is NOT
      // an Sdm_BOP_Or
      SEARCHOR *or;
      SdmBoolean exp_end;
      or = CreateOrPgm(&pf, top_or->second, &exp_end);
      top_or->second = CreateNewSearchPgm();
      top_or->second->or = or;
      if (!pf || exp_end) {
        break;      
      }
    } 
    else {
      // a new AND
      // ** ET
      // Why does this just set the second or?
      // Because it will just add the criteria specified in the predicate
      // to the "second" pgm.  It does not replace it.  If the 
      // predicate is a "NOT" criteria, a new pgm will be created and added
      // the the "second" pgm's not list.
      //
      AddToSearchProgram(pf, top_or->second);

      // See if this is the end of the expression
      if (pf->sl_expression == Sdm_SLE_End) quit = Sdm_True;
      
      // Always set pf to successor
      pf = pf->sl_next;
      
      // Done?
      if (quit) {
        break;
      }
    }
  }
  // See next leaf in tree
  *l = pf;
  return top_or;
}

//
// Here we have n disjuncts of Sdm_SLE_Simple ORs,
//  o(1),...,o(n), n >= 1.
// Return or->first = first, 
//        or->second = <search program of disjuncts>
// ** ET
// Creates SEARCHPGM tree for OR conditions.  The stupid thing
// is that SEARCHOR structures must have both first and second
// programs set.  Therefore, if you have more than two OR statements
// the code below goes through this gross loop to create a chain
// of pgms with OR conditions.  There's a lot of redundancy and
// unnecessary calls to set the pgms.  This should really be 
// cleaned up.
SEARCHOR *
SdmDpDataChanCclient::CreateOrPgm(SdmSearchLeaf **l, SEARCHPGM *first, SdmBoolean *exp_end)
{
  SEARCHOR *or = CreateNewSearchOr();
  SdmString saved_predicate;
  SdmBoolean saved_NOT;
  SEARCHPGM **pgm;
  SdmBoolean stop;
  
  // ** ET
  // Bug fix.  fix memory leak.  delete top_or->first before
  // setting it to something else below.
  delete or->first;
  or->first = first;
  
  // Chase leaf adding adjuncts to or
  SdmSearchLeaf *p;
  // First see if we have even or odd number of consecutive
  // Ors. We will Stop on an expression end, end of linked list,
  // Start of expression, or a non Sdm_BOP_Or
  int n_ors;
  for (stop = Sdm_False, p = *l,n_ors = 0; 
       p && p->sl_logic_op == Sdm_BOP_Or &&
       p->sl_expression != Sdm_SLE_Start && !stop; 
       p = p->sl_next, ++n_ors) 
  {
    if (p->sl_expression == Sdm_SLE_End) {
      stop = Sdm_True;
    }
  }
    
  // Tell caller if this "or" ends an expression
  if (exp_end) {
    if (stop) {
      *exp_end = Sdm_True;
    } else {
      *exp_end = Sdm_False;
    }
  }
  
  // Since or->first is taken, and we have AT LEAST one Sdm_BOP_Or,
  // and ors must be complete:
  //  n_ors odd    All ors will have first/second
  //  n_ors even   or->second will be a new program whose
  //               or program will be the first two OR_OPs! 
  // NOTE: n_ors >= 1;
  // ** ET - pgm is set to the last "or" created in both cases.
  p = *l;
  if ((n_ors & 1) == 0) {
    // Even number of ORs.  n_ors >= 2
    SEARCHOR *or1 = CreateNewSearchOr();
    or->second->or = or1;
    AddToSearchProgram(p, or1->first);
    p = p->sl_next;
    // Keep a copy of the last predicate added
    AddToSearchProgram(p, or1->second, &saved_predicate, &saved_NOT);
    p = p->sl_next;
    pgm = &or1->second;    
  } 
  else {
    // Odd number of ORs. Just set or->second.
    // Keep a copy of the last predicate added
    AddToSearchProgram(p, or->second, &saved_predicate, &saved_NOT);
    p = p->sl_next;
    pgm = &or->second;
  }
  
  
  // OK, now we have an even number of ors left or none since if:
  //  Had EVEN number    added two [2n + 2 - 2 = 2n] n >= 0
  //  Had ODD  number    added one [2n + 1 - 1 = 2n] n >= 0
  // Run through the rest of the consecutive ors, recursively
  // adding or programs to or2->second:
  //    or2->first = added saved_predicate
  //    or2->second->or->first = added p
  //    or2->second->or->second = added p->sl_next
  //    *pgm = &or2->second->or->second, etc...
  for (stop = Sdm_False; p && p->sl_logic_op == Sdm_BOP_Or && 
    p->sl_expression != Sdm_SLE_Start && !stop;) 
  {
    SEARCHOR *or2 = CreateNewSearchOr();
    SEARCHOR *tmp;
    // First, flush *pgm of the saved predicate:
    FlushSearchPgm(*pgm);
    
    // Bind this or program to the predessor
    (*pgm)->or = or2;
    
    //  or2->first <== saved predicate (just flushed)
    AddToSearchProgram((const char*)saved_predicate, saved_NOT, or2->first);
    
    // Now, add the next TWO predicates to or2->second->or
    tmp = or2->second->or = CreateNewSearchOr();
    AddToSearchProgram(p, tmp->first);
    p = p->sl_next;
    // Save our stuff for next interation
    AddToSearchProgram(p, tmp->second, &saved_predicate, &saved_NOT);
    
    // See if current is expression end
    if (p->sl_expression == Sdm_SLE_End) stop = Sdm_True;
    p = p->sl_next;
    
    pgm = &tmp->second;
    
    // ** ET
    // This is a waste.  we always blow away the second one just
    // to recreate it again in the next loop.  Is there a better
    // way to figure this out wihout doing this.
  }
    
  // Update our passed leaf pointer
  *l = p;
  return or;
}


//
// ** ET
// This method is a nice wrapper to AddToSearchPgm(const char *pred, SdmBoolean NOT, SEARCHPGM *pgm).
// It takes a leaf, determines the NOT value for the leaf, gets the predicate from it 
// (creating dynamic predicate if necessary), and
// then call AddToSearchProgram with the predicate and NOT value.
//
void SdmDpDataChanCclient::AddToSearchProgram(SdmSearchLeaf *leaf, SEARCHPGM *pgm, 
    SdmString *savePredicate, SdmBoolean *saveNot)
{
    char* predicate = NULL;
    SdmBoolean NOT;
    SdmBoolean predicateCreated = Sdm_False;
    
    // ** ET 
    // need to check for two ways to have NOT binding to the predicate.
    // the two cases are:  
    //    ~subject "foo"    --> atomic_not is true 
    //    ~(subject "foo")  --> atomic_not is false but expression binding 
    //                             is true for a simple expression.
    //
    if ((leaf->sl_atomic_not != Sdm_NOB_NoBinding) ||   
        (leaf->sl_expression_not == Sdm_NOB_BindExpression && leaf->sl_expression == Sdm_SLE_Simple) )  
    {
      NOT = Sdm_True;
    } else  {
      NOT = Sdm_False;
    }
    
    // Check for macro expansions
    // ** ET
    // Bug fix.  memory leak fix.  set flag which indicates whether predicate 
    // needs to be deleted after we're done using it.  
    if (!(predicate = UpdateDynamicPredicates(leaf))) {
      predicate = leaf->sl_predicate;
    } else {
      predicateCreated = Sdm_True;
    }
    
    // Add predicate to search program
    AddToSearchProgram(predicate, NOT, pgm);
    
    if (savePredicate) {
      *savePredicate = predicate;
    }
    if (saveNot) {
      *saveNot = NOT;
    }
    
    if (predicateCreated) {
      delete predicate;
    }
}



// This code adapted from MRC's imapd.c "parse_criterion()"
// Just add the "Search string" to the program, eg:
//   UNSEEN
//   From mcm-core
//
// ** ET
// This function adds the information from the predicate to the
// program.  it parses the predicate to determine which token/condition
// is being set and sets it accordingling in pgm.  If it is some string
// or date that is ANDed to the pgm, it is added to the individual list.
// If the condition is NOTed, we create a new program for the NOTed
// predicate and adds the new program to the NOT list of pgm.
// Consecutive calls to AddToSearchProgram with the same arguments will
// just result in setting the condition again.
//
// Need to replace all the constant char* with static const char* somewhere.
// fix strncmp with memcmp passing in size of static const char*.
//
void SdmDpDataChanCclient::AddToSearchProgram(const char *pred, SdmBoolean NOT, SEARCHPGM *pgm)
{
  SEARCHSET **set;    // ** ET - set used to store message number range.
  
  // ** ET
  // need to get number of messages from somewhere..
  long maxmsg = _dcStream->nmsgs; // ** ET get num of msgs from mail stream.  
    
  const char *tail, *s, *v;
  char *tok1 = NULL, *tok2 = NULL;
  int i,j;
  char c;
  
  tail = strpbrk(pred, " ");    // ** ET - tail will eventually 
                                // point to the field in the predicate.
  s = pred;                     // ** ET - s will point to the predicate.
  /*
   * Check NOT first */
  // ** ET
  // Create new search program for the predicate and add it to the not
  // list of this program.
  if (NOT) {
    SEARCHPGMLIST **not = &pgm->not;
    while (*not) not = &(*not)->next;
    *not = CreateNewSearchPgmList ();
    AddToSearchProgram (pred, Sdm_False, (*not)->pgm);
    return;
  }
  /* The break char */
  if (tail) c = *tail;
  else c = NULL;
  switch (*s) {		/* dispatch based on character */
  case '*':		/* sequence */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      if (*(set = &pgm->msgno)){/* already a sequence? */
          // ** ET
          // This code is executed only if message number sequence
          // was already specified earlier for this program.
          //
          /* silly, but not as silly as the client! */
          // ** ET
          // This is totally stupid!  Because there is no AND syntax
          // supported by the c-client, to add the list of numbers,
          // we create a new program on the not list and add to it's not
          // list the numbers we want to search for.  Thus the double
          // negation cancels out.  
          SEARCHPGMLIST **not = &pgm->not;
          while (*not) not = &(*not)->next;
          *not = CreateNewSearchPgmList ();
          set = &((*not)->pgm->not = CreateNewSearchPgmList ())->pgm->msgno;
      }
      StringToSearchSet (set,&s,maxmsg);
      break;
    case 'A':			/* possible ALL, ANSWERED */
      if (!strncmp (s, SdmSearch::Sdm_STK_Answered,strlen(SdmSearch::Sdm_STK_Answered))) pgm->answered = Sdm_BitwiseTrue;
      else if (!strncmp (s, SdmSearch::Sdm_STK_All,strlen(SdmSearch::Sdm_STK_All))) pgm->only_all = Sdm_BitwiseTrue;
      break;
    case 'B':			/* possible BCC, BEFORE, BODY */
      if (!strncmp (s, SdmSearch::Sdm_STK_BCC,strlen(SdmSearch::Sdm_STK_BCC)) && c == ' ' && *++tail)
          AddStringToList (&pgm->bcc,&tail);
      else if (!strncmp (s, SdmSearch::Sdm_STK_Before,strlen(SdmSearch::Sdm_STK_Before)) && c == ' ' && *++tail)
          StringToDate (&pgm->before,&tail);
      if (!strncmp (s, SdmSearch::Sdm_STK_Body ,strlen(SdmSearch::Sdm_STK_Body )) && c == ' ' && *++tail)
          AddStringToList (&pgm->body,&tail);
      break;
    case 'C':			/* possible CC */
      if (!strncmp (s, SdmSearch::Sdm_STK_CC,strlen(SdmSearch::Sdm_STK_CC)) && c == ' ' && *++tail)
          AddStringToList (&pgm->cc,&tail);
      break;
    case 'D':			/* possible DELETED */
      if (!strncmp (s, SdmSearch::Sdm_STK_Deleted,strlen(SdmSearch::Sdm_STK_Deleted))) pgm->deleted = Sdm_BitwiseTrue;
      if (!strncmp (s, SdmSearch::Sdm_STK_Draft,strlen(SdmSearch::Sdm_STK_Draft))) pgm->draft = Sdm_BitwiseTrue;
      if (!strncmp (s, SdmSearch::Sdm_STK_During,strlen(SdmSearch::Sdm_STK_During)) && c == ' ' && *++tail)
        StringToDate (&pgm->on,&tail);
      break;
    case 'F':			/* possible FLAGGED, FROM */
      if (!strncmp (s, SdmSearch::Sdm_STK_Flagged,strlen(SdmSearch::Sdm_STK_Flagged))) pgm->flagged = Sdm_BitwiseTrue;
      else if (!strncmp (s, SdmSearch::Sdm_STK_From,strlen(SdmSearch::Sdm_STK_From)) && c == ' ' && *++tail)
          AddStringToList (&pgm->from,&tail);
      break;
    case 'H':			/* possible HEADER Field String */
      // ** ET
      // Bug fix.  change 6 to 5 in strncmp call below.
      if (!strncmp (s, SdmSearch::Sdm_STK_Header,strlen(SdmSearch::Sdm_STK_Header)) && c == ' ' && *(v = tail + 1) &&
        (tok1 = CreateNonQuotedToken (&v,&i)) && i && c == ' ' &&
        (tok2 = CreateNonQuotedToken (&v,&i)) && i) 
      {
          SEARCHHEADER **hdr = &pgm->header;
          while (*hdr) hdr = &(*hdr)->next;
          *hdr = CreateNewSearchHeader (tok1);
          (*hdr)->text = tok2;
          tok2 = NULL;
      }
      // ** ET
      // Bug fix.  fix memory leak.  need to delete tok1 returned
      // from CreateNonQuotedToken.  tok2 is already used in search header.
      free (tok1);
      tok1 = NULL;
      
      break;
    case 'K':			/* possible KEYWORD */
      if (!strncmp (s, SdmSearch::Sdm_STK_Keyword,strlen(SdmSearch::Sdm_STK_Keyword)) && c == ' ' && *++tail)
        AddStringToList (&pgm->keyword,&tail);
      break;
    case 'L':
      if (!strncmp (s, SdmSearch::Sdm_STK_Larger,strlen(SdmSearch::Sdm_STK_Larger)) && c == ' ' && *++tail)
        StringToNumber (&pgm->larger,&tail);
      break;
    case 'N':			/* possible NEW */
      if (!strncmp (s, SdmSearch::Sdm_STK_New,strlen(SdmSearch::Sdm_STK_New))) pgm->recent = pgm->unseen = Sdm_BitwiseTrue;
      break;
    case 'O':			/* possible OLD, ON */
      if (!strncmp (s, SdmSearch::Sdm_STK_Old,strlen(SdmSearch::Sdm_STK_Old))) pgm->old = Sdm_BitwiseTrue;
      else if (!strncmp (s, SdmSearch::Sdm_STK_On,strlen(SdmSearch::Sdm_STK_On)) && c == ' ' && *++tail)
        StringToDate (&pgm->on,&tail);
      break;
    case 'R':			/* possible RECENT */
      if (!strncmp (s, SdmSearch::Sdm_STK_Recent,strlen(SdmSearch::Sdm_STK_Recent))) pgm->recent = Sdm_BitwiseTrue;
      break;
    case 'S':			/* possible SEEN, SINCE, SUBJECT */
      if (!strncmp (s, SdmSearch::Sdm_STK_Seen,strlen(SdmSearch::Sdm_STK_Seen))) pgm->seen = Sdm_BitwiseTrue;
      else if (!strncmp (s, SdmSearch::Sdm_STK_SentBefore,strlen(SdmSearch::Sdm_STK_SentBefore)) && c == ' ' && *++tail)
        StringToDate (&pgm->sentbefore,&tail);
      else if (!strncmp (s, SdmSearch::Sdm_STK_SentOn,strlen(SdmSearch::Sdm_STK_SentOn)) && c == ' ' && *++tail)
        StringToDate (&pgm->senton,&tail);
      else if (!strncmp (s, SdmSearch::Sdm_STK_SentSince,strlen(SdmSearch::Sdm_STK_SentSince)) && c == ' ' && *++tail)
        StringToDate (&pgm->sentsince,&tail);
      else if (!strncmp (s, SdmSearch::Sdm_STK_Since,strlen(SdmSearch::Sdm_STK_Since)) && c == ' ' && *++tail)
        StringToDate (&pgm->since,&tail);
      else if (!strncmp (s, SdmSearch::Sdm_STK_Smaller,strlen(SdmSearch::Sdm_STK_Smaller)) && c == ' ' && *++tail)
        StringToNumber (&pgm->smaller,&tail);
      else if (!strncmp (s, SdmSearch::Sdm_STK_Subject,strlen(SdmSearch::Sdm_STK_Subject)) && c == ' ' && *++tail)
        AddStringToList (&pgm->subject,&tail);
      break;
    case 'T':			/* possible TEXT, TO */
      if (!strncmp (s, SdmSearch::Sdm_STK_Text,strlen(SdmSearch::Sdm_STK_Text)) && c == ' ' && *++tail)
        AddStringToList (&pgm->text,&tail);
      else if (!strncmp (s, SdmSearch::Sdm_STK_To,strlen(SdmSearch::Sdm_STK_To)) && c == ' ' && *++tail)
        AddStringToList (&pgm->to,&tail);
      break;
    case 'U':			/* possible UN* */
      if (!strncmp (s, SdmSearch::Sdm_STK_Unanswered,strlen(SdmSearch::Sdm_STK_Unanswered))) pgm->unanswered = Sdm_BitwiseTrue;
      else if (!strncmp (s, SdmSearch::Sdm_STK_Undeleted,strlen(SdmSearch::Sdm_STK_Undeleted))) pgm->undeleted = Sdm_BitwiseTrue;
      else if (!strncmp (s, SdmSearch::Sdm_STK_Undraft,strlen(SdmSearch::Sdm_STK_Undraft))) pgm->undraft = Sdm_BitwiseTrue;
      else if (!strncmp (s, SdmSearch::Sdm_STK_Unflagged,strlen(SdmSearch::Sdm_STK_Unflagged))) pgm->unflagged = Sdm_BitwiseTrue;
      else if (!strncmp (s, SdmSearch::Sdm_STK_Unkeyword,strlen(SdmSearch::Sdm_STK_Unkeyword)) && c == ' ' && *++tail)
        AddStringToList (&pgm->unkeyword,&tail);
      else if (!strncmp (s, SdmSearch::Sdm_STK_Unseen,strlen(SdmSearch::Sdm_STK_Unseen))) pgm->unseen = Sdm_BitwiseTrue;
      break;
    default:		
      break;
    }
}

// ******************************************************
//
// Utility methods.  probably can all be static.
//
// ******************************************************

// Extract next " " token, and return it as an allocated
// string. "<string>" is stripped nude.
// ** ET
// This rips out the double quotes around the string.
// returns a new heap allocated string and sets the
// given length to the length of the string.
char *SdmDpDataChanCclient::CreateNonQuotedToken(const char **str, int *len)
{
  const char *token = *str, *end;
  char *ret, *buf;
  int i;
  // Remove any quotes
  // Well formed string should be: "Lilly Steve Carl"
  //  The c-client will find the SPACE and requote it.
  if (*token == '\"') {
    end = ++token;
    while (*end != '\"' && *end) ++end;
  } else {
    end = token;
    while (*end != ' ' && *end) ++end;
  }
  *len = end - token;
  *str = (end ? end + 1 : end);
  ret = (char *)malloc(*len + 1);
  for (i = 0, buf = ret; i < *len; ++i)
    *buf++ = *token++;
  *buf = '\0';
  return ret;
}


// Get string from *arg, and place in in the string list.
// ** ET
// adds arg to string list.  before adding to list, calls
// CreateNonQuotedToken to strip out any double quotes around the
// string.
SdmBoolean 
SdmDpDataChanCclient::AddStringToList (STRINGLIST **string, const char **arg)
{
  int i;
  char *s = CreateNonQuotedToken (arg,&i);
  if (!s) {
    return Sdm_False;
  }
  
  /* find tail of list */
  while (*string) {
    string = &(*string)->next;
  }
  *string = InitializeStringList (s);
  
  // ** ET
  // Bug fix.  fix memory leak.  need to delete string returned from
  // CreateNonQuotedToken.
  free (s);
  
  return Sdm_True;
}



// 
// ** ET - creates a SEARCHSET using the given arg.  The arg
// is a pointer to a string that represent the message numbers that should
// be searched.  this list looks like "1,3,8,10" to specify a list of
// specific numbers and/or "1:10" to specify ranges of numbers.
// Both syntax may be combined together "1,3,8:26,7,30:40".  
// It appears that the numbers don't need to be ascending order.
// Note that the last value only needs to be set if it's a range.
// This function calls itself to build the entire SEARCHSET list.
// Note: no error is returned even if arg is in the incorrect format
// or contains invalid characters.  It's assumed that arg will be in
// the correct format when passed in.  arg is also modified to point
// to after the last character that is processed.  this is not relevant
// for the caller (AddToSearchProgram) because arg is not used again
// after calling StringToSearchSet.  It can be relevant to future callers if they
// want to know what was the last unparsed character.
//
void SdmDpDataChanCclient::StringToSearchSet (SEARCHSET **set, const char **arg, long maxima)
{
  unsigned long i;
  *set = CreateNewSearchSet ();	/* instantiate a new search set */
  if (**arg == '*') {		/* maxnum? */
    (*arg)++;			/* skip past that number */
    (*set)->first = maxima;
  }
  else if (StringToNumber (&i,arg) && i) {
    (*set)->first = i;
  }
  else {
    return;			/* bogon */
  }
  
  switch (**arg) {		/* decide based on delimiter */
    case ':':			/* sequence range */
      if (*++(*arg) == '*') {	/* maxnum? */
        (*arg)++;			/* skip past that number */
        (*set)->last -= maxima;  // ** ET - BUG : why using -= here? 
                                 // should be using = .
      }
      else if (StringToNumber (&i,arg) && i) {
        if (i < (*set)->first) {	/* backwards range */
          (*set)->last = (*set)->first;
          (*set)->first = i;
        }
        else (*set)->last = i;	/* set last number */
      }
      else {
        return;		// ** ET - this should really return an error to
                    // indicate that the args given are not numeric.
      }

      if (**arg != ',') break;	/* drop into comma case if comma seen */
    case ',':
      (*arg)++;			/* skip past delimiter */
      StringToSearchSet (&(*set)->next,arg,maxima);
    default:
      break;
  }
  return;			
}

// ** ET 
// converts given string to a number and returns this value in
// the "number" argument.  numeric_field is similar.  Maybe
// these should be combined?
//
SdmBoolean
SdmDpDataChanCclient::StringToNumber (unsigned long *number, const char **arg)
{
  if (!isdigit (**arg)) return Sdm_False;
  *number = 0;
  while (isdigit (**arg)) {	/* found a digit? */
    *number *= 10;		/* add a decade */
    *number += *(*arg)++ - '0';	/* add number */
  }
  return Sdm_True;
}


SdmBoolean 
SdmDpDataChanCclient::IsDateField (const char *arg)
{
  if (arg == NULL) {
    return Sdm_False;
  } else {
    unsigned short i;
    const char *tmp = arg;  // need to use tmp pointer because
                            // StringToDate will change the value of 
                            // the pointer.
    
    // first call StringToDate to see if string is in "DD-MMM-YYYY" format.
    // tmp pointer will point to character after the date string.
    if (StringToDate(&i, &tmp) && tmp != NULL && *tmp == '\0') {
      return Sdm_True;

    // if not in proper format, it could be a dynamic predicate (eg. Today).
    // check the list of dynamic predicates.      
    } else {
      const SdmDynamicPredicate *dyn;  
      
      // the dynamic predicates are surrounded by quotes so we'll
      // need to add quotes around the arg to properly do the comparison.
      SdmString tmpStr;
      if (*arg == '\"') {
        tmpStr = arg;
      } else {
        tmpStr = "\"";
        tmpStr += arg;
        tmpStr += "\"";
      }
       
      for (dyn = gkDynamicPredicateMap; dyn->value; ++dyn) {
        if (strcasecmp((const char*)tmpStr, dyn->value) == 0) {
          return Sdm_True;
        }
      }
    } 
  }
    
  return Sdm_False;
}


// ** ET
// converts string that represents date to numeric value
// for date.  The actual translation occurs in HandleStringToDate
// but this method checks for quotes around the date before
// call HandleStringToDate.
SdmBoolean 
SdmDpDataChanCclient::StringToDate (unsigned short *date, const char **arg)
{
  /* handle quoted form */
  if (**arg != '"') {
    return HandleStringToDate (date,arg);
  }
  (*arg)++;			/* skip past opening quote */
  if (!(HandleStringToDate (date,arg) && (**arg == '"'))) {
    return Sdm_False;
  }
  (*arg)++;			/* skip closing quote */
  return Sdm_True;
}


// ** ET
// This method does the actual conversion from a string to a date.
SdmBoolean 
SdmDpDataChanCclient::HandleStringToDate(unsigned short *date, const char **arg)
{
  int d,m,y;
  if (isdigit (**arg)) {	/* day */
    d = *(*arg)++ - '0';	/* first digit */
    if (isdigit (**arg)) {	/* if a second digit */
      d *= 10;			    /* slide over first digit */
      d += *(*arg)++ - '0';	/* second digit */
    }
    if ((**arg == '-') && (y = *++(*arg))) {
      m = (y >= 'a' ? y - 'a' : y - 'A') * 1024;
      if ((y = *++(*arg))) {
        m += (y >= 'a' ? y - 'a' : y - 'A') * 32;
        if ((y = *++(*arg))) {
          m += (y >= 'a' ? y - 'a' : y - 'A');
          switch (m) {		/* determine the month */
            case (('J'-'A') * 1024) + (('A'-'A') * 32) + ('N'-'A'): m = 1; break;
            case (('F'-'A') * 1024) + (('E'-'A') * 32) + ('B'-'A'): m = 2; break;
            case (('M'-'A') * 1024) + (('A'-'A') * 32) + ('R'-'A'): m = 3; break;
            case (('A'-'A') * 1024) + (('P'-'A') * 32) + ('R'-'A'): m = 4; break;
            case (('M'-'A') * 1024) + (('A'-'A') * 32) + ('Y'-'A'): m = 5; break;
            case (('J'-'A') * 1024) + (('U'-'A') * 32) + ('N'-'A'): m = 6; break;
            case (('J'-'A') * 1024) + (('U'-'A') * 32) + ('L'-'A'): m = 7; break;
            case (('A'-'A') * 1024) + (('U'-'A') * 32) + ('G'-'A'): m = 8; break;
            case (('S'-'A') * 1024) + (('E'-'A') * 32) + ('P'-'A'): m = 9; break;
            case (('O'-'A') * 1024) + (('C'-'A') * 32) + ('T'-'A'): m = 10;break;
            case (('N'-'A') * 1024) + (('O'-'A') * 32) + ('V'-'A'): m = 11;break;
            case (('D'-'A') * 1024) + (('E'-'A') * 32) + ('C'-'A'): m = 12;break;
            default: return Sdm_False;
          }
          if ((*++(*arg) == '-') && isdigit (*++(*arg))) {
            y = 0;		/* init year */
            do {
              y *= 10;		/* add this number */
              y += *(*arg)++ - '0';
            }
            while (isdigit (**arg));
            
            /* minimal validity check of date */
            if (d < 1 || d > 31 || m < 1 || m > 12 || y < 0) return Sdm_False; 
            /* Tenex/ARPAnet began in 1969 */
            if (y < 100) y += (y >= (BASEYEAR - 1900)) ? 1900 : 2000;
            /* return value */
            *date = ((y - BASEYEAR) << 9) + (m << 5) + d;
            return Sdm_True;		/* success */
          }
        }
      }
    }
  }
  return Sdm_False;			/* else error */
}



// ***************************************************
//
// Methods used to create cclient search structures
//
// ***************************************************
// ** ET
// Check to sett how these structures are being initialized.
// Find out which pointers fields are allocated memory.
//
SEARCHPGM *
SdmDpDataChanCclient::CreateNewSearchPgm(void)
{
  return mail_newsearchpgm();
}
void
SdmDpDataChanCclient::FlushSearchPgm(SEARCHPGM *pgm)
{
  if (pgm) mail_flush_searchpgm(pgm);
}
 
SEARCHOR *
SdmDpDataChanCclient::CreateNewSearchOr(void)
{
  return mail_newsearchor();
}
SEARCHPGMLIST *
SdmDpDataChanCclient::CreateNewSearchPgmList(void)
{
  return mail_newsearchpgmlist();
}
SEARCHHEADER *
SdmDpDataChanCclient::CreateNewSearchHeader(const char *line)
{
  return mail_newsearchheader((char*)line);
}
SEARCHSET *
SdmDpDataChanCclient::CreateNewSearchSet(void)
{
  return mail_newsearchset();
}

//
// IMAP4 string list manipulation methods
STRINGLIST *
SdmDpDataChanCclient::InitializeStringList(const char *str)
{
  STRINGLIST *slist = mail_newstringlist();
  slist->text = strdup(str);
  slist->size = strlen(str);
  slist->next = NIL;
  return slist;
}

void
SdmDpDataChanCclient::FreeStringList(STRINGLIST *slist)
{
  mail_free_stringlist(&slist);
}





