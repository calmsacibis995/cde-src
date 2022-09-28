/*
 *+SNOTICE
 *
 *	$Revision: 1.0 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

////////////////////////////////////////////////////////////
// Rulecmd.h: Display the Rule Editor dialog
////////////////////////////////////////////////////////////
#ifndef RULECMD_H
#define RULECMD_H

#ifndef I_HAVE_NO_IDENT
#pragma ident "%W %G"
#endif

#include "WarnNoUndoCmd.h"
#include "Rule_edit_ui.h"
#include <filter.h>

class ExprStack {
  private:
    ExprStack *next;
    expr *e;

  public:
    ExprStack(ExprStack *s, expr *e1) : next(s), e(e1) {;}
    ~ExprStack() {;}	// Not allocate any space
    expr *get_expr() {return e;}
    ExprStack *get_next() {return next;}
};

class RuleCmd: public WarnNoUndoCmd {
  protected:
    DtbRuleEditDialogInfo _rule_dlog;
    virtual void doit();      // Call exit

  public:
    RuleCmd ( char *, char *, int, Widget parent);
    ~RuleCmd();	
    void create_dlog();
    void unmanage();
    void manage_dialog();	  //  Manage the Rule Editor dialog 
    virtual const char *const className () { return "RuleCmd"; }
    virtual void execute(); 

    void handle_browse_cb(char *);
    void updateUi(filter *);
    filter *get_filter();	  // Format a filter structure from the UI
    void clear_dialog();  
    void update_fields();
    void loadFilters();

  private:
    char *_mailrule_name;
    struct filter *_filter;
    void update_text_field(char *, char*);
    void parse_expr(expr *, int*);
    void parse_action(act *, int*);
    void format_grammar(struct filter*);
    char *build_expr(expr *);
    void push(char*, char *);
    expr *pop();
    ExprStack *_stack;
};
#endif
