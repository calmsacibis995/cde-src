/*
 *+SNOTICE
 *
 * RESTRICTED CONFIDENTIAL INFORMATION:
 * 
 * The information in this document is subject to special
 * restrictions in a confidential disclosure agreement between
 * HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 * document outside HP, IBM, Sun, USL, SCO, or Univel without
 * Sun's specific written approval.  This document and all copies
 * and derivative works thereof must be returned or destroyed at
 * Sun's request.
 *
 * Copyright 1993, 1994, 1995, 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Rules.h	1.3 02/12/96"
#endif

#ifndef	RULES_H
#define RULES_H

// The variables in rule grammar
#define SAVE_V	 	"save"
#define SOUND_V	 	"sound"
#define ALERT_V 	"alert"
#define BEEP_V	 	"beep"
#define IGNORE_V	"ignorecaes"
#define RUN_V		"run"
#define CRITERIA_V	"criteria"
#define SKIP_V		"skipremaining"
#define NEXTRULE_V	"nextrule"
#define END_V 		"endrule"
#define HEADER_V	"header"
#define BODY_V		"body"
#define SIZE_V		"size"
#define BEFORE_V	"before"
#define AFTER_V		"after"

typedef enum {OFF, ON} State;
typedef enum {TO, CC, TO_OR_CC, FROM, SUBJECT} Header;
typedef enum {CONTINUE, SKIPREMAINING, NEXTRULE} SkipRule;

class RulesStringPair {
  public:
    char *	label;	     // Rule name
    char *	value;	     // The rest of the strings in the rule grammar
    char *	shownewmail; // Mailbox that is displayed in 'Show mailboxes with new mail' menu
    int		ignore;      // ignorecase variable, this field can be empty or "ingorecase"
    State	rule_state;  // The state of the rule, it can be "ON" or "OFF"
    char *	fields[5];   // The header fields that are supported by this UI
    char **	actions;     // The action commands 
    int 	action_num;  // How many action commands
    SkipRule	skip;	     // skip var, it can be "skipremaining", "nextrule" or empty

    RulesStringPair(char *, char *);
    RulesStringPair(const RulesStringPair &);
    ~RulesStringPair(void);
    void parseRulesString();
    char *formatRulesPair(); 

  private:
    void set_state();
    void parse_header(char *);
    void parse_actions(char *);
};
#endif
