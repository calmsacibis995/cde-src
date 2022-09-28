
 /*
 *+SNOTICE
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)RuleCmd.C	1.21 08/02/96"
#endif

///////////////////////////////////////////////////////////
// RuleCmd.C : creates the Rule Editor dialog
//////////////////////////////////////////////////////////
#include <Xm/Text.h>
#include <DtMail/RuleCmd.h>
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/SystemUtility.hh>
#include "RoamApp.h"
#include "MailMsg.h"
#include "ExistingButtonInterface.h"
#include "SelectFileCmd.h"
#include "parse.h"

#define MAXCHAR 512

extern void rule_browse_cb(void *, char *);

RuleCmd::RuleCmd ( char *name, char *label, int active, Widget parent) : 
                 WarnNoUndoCmd ( name, label, active ) 
{
    _dialogParentWidget = parent;
    _rule_dlog = NULL;
    _filter = NULL;
}

void RuleCmd::create_dlog()
{
    // Create the Rule Editor dialog
    _rule_dlog = (DtbRuleEditDialogInfo)malloc(sizeof(
	DtbRuleEditDialogInfoRec));
    dtb_rule_edit_dialog_initialize(&(dtb_rule_edit_dialog),
	dtb_get_toplevel_widget());
    _rule_dlog = &dtb_rule_edit_dialog;

    XtVaSetValues(_rule_dlog->editor_to_tf,
	XmNuserData, this,
	NULL);
    XtVaSetValues(_rule_dlog->editor_sub_tf,
	XmNuserData, this,
	NULL);
    XtVaSetValues(_rule_dlog->editor_from_tf,
	XmNuserData, this,
	NULL);
    XtVaSetValues(_rule_dlog->editor_cc_tf,
	XmNuserData, this,
	NULL);
    XtVaSetValues(_rule_dlog->editor_to_or_cc_tf,
	XmNuserData, this,
	NULL);
    XtVaSetValues(_rule_dlog->store_msg_tf,
	XmNuserData, this,
	NULL);	
    XtVaSetValues(_rule_dlog->add_button,
	XmNuserData, this,
	NULL);
    XtVaSetValues(_rule_dlog->change_button,
	XmNuserData, this,
	NULL);
    XtVaSetValues(_rule_dlog->reset_button,
	XmNuserData, this,
	NULL);

  // Add connection to Select buttons
  // to bring up a FSB.
    char *label = catgets(DT_catd, 1, 122, "Include...");
    char *title = catgets(DT_catd, 1, 123, "Mailer - Include");
    char *ok_label = catgets(DT_catd, 1, 124, "Include");

    SelectFileCmd *browse;
    browse = new SelectFileCmd("Include...", 
	label,
	title,
	ok_label,
	TRUE,
	rule_browse_cb,
	_rule_dlog->store_msg_tf,
	_rule_dlog->dialog);
    new ExistingButtonInterface(_rule_dlog->browse_but, browse);
}


void
RuleCmd::loadFilters()
{
    passwd pw;

    SdmSystemUtility::GetPasswordEntry(pw);

    // 512 should be big enough for now
    _mailrule_name = (char *)malloc(512);

    strcpy(_mailrule_name, pw.pw_dir);
    strcat(_mailrule_name, "/");
    strcat(_mailrule_name, RULEFILE);


    // Parse all rules
    parse_rules(_mailrule_name);

    // We can't use yacc to store the grammar, so we have to do it here 
    // in order to write it out later.
    struct filter *f = reorder_filter(basefilter, NULL);
    FILE *inf = fopen(_mailrule_name, "r");
#define LINESIZE 1024
    char linebuf[LINESIZE];

    while (f) {
	while (SdmSystemUtility::ReadLine(inf, linebuf, LINESIZE) > 0) {
	    char *token;
	    char *tmpbuf = strdup(linebuf);
	    if (linebuf[0] != '#')  
		if ((token = strtok(tmpbuf, "\"")) && (token = strtok(NULL, "\""))) {
		    if (strcmp(token, f->f_name) == 0)
			f->f_grammar = strdup(linebuf);
		    free(tmpbuf);
		    break;
		}
	    free(tmpbuf);
	}

	f= f->f_next;
    }

    fclose(inf);
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
RuleCmd::~RuleCmd()
{
  free(_rule_dlog);
}

//////////////////////////////////////////////////////////////////////////////
void RuleCmd::doit()
{
}

//////////////////////////////////////////////////////////////////////////////
void RuleCmd::unmanage()
{
  XtPopdown(_rule_dlog->dialog);
}

//////////////////////////////////////////////////////////////////////////////
void
RuleCmd::execute()
{
    if (_rule_dlog == NULL) {
	this->create_dlog();
    }
}

//////////////////////////////////////////////////////////////////////////////
void
RuleCmd::handle_browse_cb(char *selection)
{
    XtVaSetValues(_rule_dlog->store_msg_tf, XmNvalue, selection, NULL);
}

//////////////////////////////////////////////////////////////////////////////
void 
RuleCmd::manage_dialog()
{
    XtManageChild(_rule_dlog->dialog_shellform);
    XRaiseWindow(XtDisplay(_rule_dlog->dialog), XtWindow(_rule_dlog->dialog));
}

//////////////////////////////////////////////////////////////////////////////
void 
RuleCmd::update_fields()
{
    clear_dialog();

    if ((_filter == NULL) || (*_filter->f_name == NULL))
	return;

    // The Rule name field
    XmTextSetString(_rule_dlog->rule_name_tf, _filter->f_name);

    // The ON/OFF check box
    if (_filter->f_state) {
	    XtVaSetValues(_rule_dlog->rule_on_off_checkbox_items.On_item,
			  XmNset, True, NULL);
	    XtVaSetValues(_rule_dlog->rule_on_off_checkbox_items.Off_item,
			  XmNset, False, NULL);
	}
	else {
	    XtVaSetValues(_rule_dlog->rule_on_off_checkbox_items.On_item,
			  XmNset, False, NULL);
	    XtVaSetValues(_rule_dlog->rule_on_off_checkbox_items.Off_item,
			  XmNset, True, NULL); 
	}

    // The Skip check box
    if (_filter->f_consume)
	XtVaSetValues(_rule_dlog->checkbox_items.Skip_remaining_rules_if_message_matches_this_rule_item, XmNset, True, NULL);
    else
	XtVaSetValues(_rule_dlog->checkbox_items.Skip_remaining_rules_if_message_matches_this_rule_item, XmNset, False, NULL);

    // Update the criteria and action fields, if there is any expression
    // or action that is not supported by the UI, popup a warning. This
    // UI only accepts a small subset from the rule spec
    int err = 0;
    parse_expr(_filter->f_expr, &err);
    parse_action(_filter->f_act, &err);
    if (err > 0) {
	DtMailGenDialog *genDialog = theRoamApp.genDialog();
	genDialog->setToErrorDialog(catgets(DT_catd, 3, 48, "Mailer"),
				    catgets(DT_catd, 4, 7, "Mailer cannot display some of the settings that you use\nto create this rule.  Clicking Add or Change in the Rule\nEditor dialog box may cause deletion of these settings."));
	genDialog->post_and_return(catgets(DT_catd, 3, 72, "OK"),
				   DTMAILHELPUNSUPPORTRULE);
    }
}

void 
RuleCmd::updateUi(filter *f)
{
    _filter = f;
    if (XtIsManaged(_rule_dlog->dialog_shellform))
	update_fields();
}

// Clear all fields in the Editor dialog 
void
RuleCmd::clear_dialog()
{
    XmTextSetString(_rule_dlog->rule_name_tf, NULL);
    XmTextSetString(_rule_dlog->editor_to_tf, NULL);
    XmTextSetString(_rule_dlog->editor_cc_tf, NULL);
    XmTextSetString(_rule_dlog->editor_to_or_cc_tf, NULL);
    XmTextSetString(_rule_dlog->editor_sub_tf, NULL);
    XmTextSetString(_rule_dlog->editor_from_tf, NULL);
    XmToggleButtonSetState(_rule_dlog->rule_on_off_checkbox_items.On_item, True, True);
    XtVaSetValues(_rule_dlog->checkbox_items.Skip_remaining_rules_if_message_matches_this_rule_item, XmNset, True, NULL);

    XmTextSetString(_rule_dlog->store_msg_tf, NULL);
}

filter *
RuleCmd::get_filter()
{
    // We must have a rule name and some rules
    char *rulename = XmTextGetString(_rule_dlog->rule_name_tf);

    DtMailGenDialog *genDialog = theRoamApp.genDialog();
    if (!rulename || !*rulename) {
	genDialog->setToErrorDialog(
	    catgets(DT_catd, 3, 48, "Mailer"),
	    catgets(DT_catd, 4, 4, "You did not specify a rule name."));
	genDialog->post_and_return(catgets(DT_catd, 3, 72, "OK"),
	                           DTMAILHELPNORULENAME);


	XmProcessTraversal(_rule_dlog->rule_name_tf, XmTRAVERSE_CURRENT);
	return NULL;
    }
    
    // Get all the text in the criteria text fields
    char *to_text = XmTextGetString(_rule_dlog->editor_to_tf);
    char *cc_text = XmTextGetString(_rule_dlog->editor_cc_tf);
    char *to_or_cc_text = XmTextGetString(_rule_dlog->editor_to_or_cc_tf);
    char *from_text = XmTextGetString(_rule_dlog->editor_from_tf);
    char *sub_text = XmTextGetString(_rule_dlog->editor_sub_tf);

    if ((*to_text == NULL) && (*cc_text == NULL) && 
	(*to_or_cc_text == NULL) && (*from_text == NULL) && 
	(*sub_text == NULL)) {
	genDialog->setToErrorDialog(
	    catgets(DT_catd, 3, 48, "Mailer"),
	    catgets(DT_catd, 4, 5, "You did not specify the criteria for this rule."));
	genDialog->post_and_return(catgets(DT_catd, 3, 72, "OK"),
	                           DTMAILHELPNORULES);
	XmProcessTraversal(_rule_dlog->editor_to_tf, XmTRAVERSE_CURRENT);
	return NULL;
    }

    // Now we scan through the actions widgets to write out the action rule
    char *str = XmTextGetString(_rule_dlog->store_msg_tf);

    if ((str == NULL) || (*str == NULL)) {
	genDialog->setToErrorDialog(
	    catgets(DT_catd, 3, 48, "Mailer"),
	    catgets(DT_catd, 4, 6, "You did not specify the mailbox path name for this rule."));
	genDialog->post_and_return(catgets(DT_catd, 3, 72, "OK"),
	                           DTMAILHELPNOMAILFILE);
	XmProcessTraversal(_rule_dlog->store_msg_tf, XmTRAVERSE_CURRENT);
	return NULL;
    }


    struct filter *f = alloc_filter();
    f->f_name = rulename;
    f->f_state = XmToggleButtonGetState(_rule_dlog->rule_on_off_checkbox_items.On_item);
    f->f_consume = XmToggleButtonGetState(_rule_dlog->checkbox_items.Skip_remaining_rules_if_message_matches_this_rule_item);

    // Construct the action list
    act *a = alloc_act();
    a->a_next = NULL;
    a->a_type = SAVE;
    a->a_string = strdup(str);
    f->f_act = a;
    XtFree(str);

    // Contruct the expression string
    _stack = NULL;
    push("to", to_text);
    push("cc", cc_text);
    push("to_or_cc", to_or_cc_text);
    push("from", from_text);
    push("subject", sub_text);

    expr *prev_e = pop();
    if (prev_e) {
	expr *exp = NULL;
	while (exp = pop()) {
	    expr *tree = alloc_expr();
	    tree->e_type = AND;
	    tree->e_left = exp;
	    tree->e_right = prev_e;
	    prev_e = tree;
	}
    }
	
    f->f_expr = prev_e;
    format_grammar(f);
    return f;
}

void
RuleCmd::update_text_field(char *header, char *value)
{
    if (strcasecmp(header, "to") == 0)
	XmTextSetString(_rule_dlog->editor_to_tf, value);
    else if (strcasecmp(header, "cc") == 0)
	XmTextSetString(_rule_dlog->editor_cc_tf, value);
    else if (strcasecmp(header, "to_or_cc") == 0)
	XmTextSetString(_rule_dlog->editor_to_or_cc_tf, value);
    else if (strcasecmp(header, "subject") == 0)
	XmTextSetString(_rule_dlog->editor_sub_tf, value);
    else if (strcasecmp(header, "from") == 0)
	XmTextSetString(_rule_dlog->editor_from_tf, value);
}

void
RuleCmd::parse_expr(expr *e, int *err)
{
    if (!e) return;

    switch(e->e_type) {
      case HEADER:
	update_text_field(e->e_string1, e->e_string2);
	break;
      case AND:
	parse_expr(e->e_left, err);
	parse_expr(e->e_right, err);
	break;
      default:
	(*err)++;
	if (e->e_left) 
	    parse_expr(e->e_left, err);
	if (e->e_right) 
	    parse_expr(e->e_right, err);
	break;
    }
}

void
RuleCmd::parse_action(act *a, int *err)
{
    if (!a)
	return;

    if (a->a_next)
	parse_action(a->a_next, err);

    switch (a->a_type) {
      case SAVE:
	XmTextSetString(_rule_dlog->store_msg_tf, a->a_string);
	break;
      default:
	(*err)++;
	break;
    }
    return;
}

void 
RuleCmd::push(char *header, char *value)
{
    if (!value || !*value)
	return;

    expr *exp = alloc_expr();
    exp->e_type = HEADER;
    exp->e_string1 = strdup(header);
    exp->e_string2 = strdup(value);
    
    ExprStack *s = new ExprStack(_stack, exp);
    _stack = s;
}

expr *
RuleCmd::pop()
{
    if (_stack) {
	expr *exp = _stack->get_expr();
	_stack = _stack->get_next();
	return exp;
    }
    else
	return NULL;
}


void
RuleCmd::format_grammar(struct filter *f)
{
    char *grammar = new char[MAXCHAR];
    char *action_str = new char[MAXCHAR];
    char *state = NULL;
    char *consume = NULL;
    char *ignore = NULL;

    // Construct the action fields
    act *a = f->f_act;
    strcpy(action_str, "");
    while(a) {
	char *action;
	switch (a->a_type) {
	  case RUN:
	    action = strdup(RUN_V);
	    break;
	  case SAVE:
	    action = strdup(SAVE_V);
	    break;
	  default:
	    printf("unsupported action\n");
	    break;
	}
	sprintf(action_str, "%s %s \"%s\"", action_str, action, a->a_string);
	
	free (action);
	a = a->a_next;
    }

    // Construct the exprssion field
    char *expr_str = build_expr(f->f_expr);
	
    if (f->f_state)
	state = strdup("ON");
    else
	state = strdup("OFF");

    if (f->f_consume)
	consume = strdup(SKIP_V);
    else
	consume = strdup("");

    if (f->f_ignorecase)
	ignore = strdup(IGNORE_V);
    else
	ignore = strdup("");

    // Format the final grammar
    sprintf(grammar, "%s \"%s\" %s %s %s (%s) %s %s %s", 
	           RULE_V, f->f_name, state, ignore, CRITERIA_V, expr_str, action_str, consume, END_V);

    f->f_grammar = strdup(grammar);

    delete []expr_str;
    delete []grammar;
    delete []action_str;
    free (state);
    free (consume);
    free (ignore);
}

// Build the rule criteria string
char *
RuleCmd::build_expr(expr *e) 
{
    char *new_str = new char[MAXCHAR];
    char type_str[15];
    
    if (e->e_left == NULL) {
	char *left_str = new char[512];

	switch (e->e_type) {
	  case HEADER:
	    // grammar:
	    //       HEADER STRING EQUALS STRING
	    //  or
	    //       HEADER STRING
	    
	    if (e->e_string2)
		sprintf(new_str, "%s \"%s\" = \"%s\"", HEADER_V, e->e_string1, e->e_string2);
	    else 
		sprintf(new_str, "%s \"%s\"", HEADER_V, e->e_string1);

	    delete []left_str;
	    return (new_str);
	    
	  case BODY:
	    // grammar:
	    //      BODY EQUALS STRING
	    
	    strcpy(type_str, "body =");
	    left_str = strdup(e->e_string1);
	    break;
	    
	  case AFTER:
	    // grammar:
	    //      AFTER STRING
	    
	    strcpy(type_str, AFTER_V);
	    left_str = strdup(e->e_string1);
	    break;
	    
	  case BEFORE:
	    // grammar:
	    //      BEFORE STRING
	    
	    strcpy(type_str, BEFORE_V);
	    left_str = strdup(e->e_string1);
	    break;
	    
	  case ATTACHMENT:
	    // grammar:
	    //      ATTACHMENT STRING
	    // or   ATTACHMENT
	    
	    strcpy(type_str, ATTACH_V);
	    if (e->e_left) 
		left_str = strdup(e->e_string1);
	    else 
		strcpy(left_str, "");
	    break;

	  default:
	    printf("error exprssion\n");
	    delete []left_str;
	    strcpy(new_str, "");
	    return (new_str);
	}

	sprintf(new_str, "%s \"%s\"", type_str, left_str);
	delete []left_str;
    }
    else {
	switch (e->e_type) {
	  case AND:
	    // grammar:
	    //      expression AND expression
	    strcpy(type_str, AND_V);
	    break;

	  case OR:
	    // grammar:
	    //      expression OR expression
	    strcpy(type_str, OR_V);
	    break;

	  default:
	    delete []type_str;
	    printf("error exprssion type\n");
	    return ("");	   
	}

	char *s1 = build_expr(e->e_left);
	char *s2 = build_expr(e->e_right);
	sprintf(new_str, "%s %s %s", s1, type_str, s2);

	delete []s1;
	delete []s2;
    }

    return (new_str);
}
