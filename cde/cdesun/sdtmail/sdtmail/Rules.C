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
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Rules.C	1.7 02/28/96"
#endif

#include "MailMsg.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rules.h"
#define MAXACTION 10    // How many actions are we allowed in a rule

RulesStringPair::RulesStringPair(char *key, char *data)
  :ignore(1), action_num(0), rule_state(ON), skip(SKIPREMAINING)
{
    if (key)
	label = strdup(key);
    if (data)
	value = strdup(data);
    set_state();    //We only need to display state and label
    actions = NULL;

    int i;
    for (i=0; i< 5; i++)
	fields[i] = NULL;

    shownewmail = NULL;
    parseRulesString();
}

RulesStringPair::RulesStringPair(const RulesStringPair & other)
  :ignore(other.ignore), action_num(other.action_num), 
   rule_state(other.rule_state), skip(other.skip)
{
    label = NULL;
    value = NULL;

    if (other.label) {
	label = strdup(other.label);
    }

    if (other.value) {
	value = strdup(other.value);
    }
    rule_state = other.rule_state;

    int i;
    for (i=0; i<5; i++) {
	if (other.fields[i])
	    fields[i] = strdup(other.fields[i]);
	else 
	    fields[i] = NULL;
    }

    actions = new char*[action_num];
    for (i=0; i<action_num; i++) {
	if (other.actions[i])
	    actions[i] = strdup(other.actions[i]);
	else
	    actions[i] = NULL;
    }

    if (other.shownewmail)
	shownewmail = strdup(other.shownewmail);
    else 
	shownewmail = NULL;
}

RulesStringPair::~RulesStringPair(void)
{
    if (label)
	free(label);
    if (value)
	free(value);

    int i;
    for (i=0; i<5; i++) {
	if (fields[i])
	    free(fields[i]);
    }
    
    for (i=0; i<action_num; i++) {
	if (actions[i])
	    delete []actions[i];
    }
    actions = NULL;

    if (shownewmail)
	free(shownewmail);
}

void 
RulesStringPair::parse_header(char *str)
{
    char *string = NULL;
    char *p, *end;

    // The string after '=' is the header value
    if (p = strchr(str, '=')) {
	p++;
	// Take out leading spaces and double quote
	while(p && (isspace(*p) || (*p == '\"'))) 
	    p++;

	// Take out ending 'and'
	if (end = strstr(str, "and")) {
	    int len = end - p + 1;
	    string = new char[len];
	    strncpy(string, p, len-1);
	    string[len-1] = '\0';
	}
	else 
	    string = strdup(p);

	// Take out ending spaces and double quote
	int len = strlen(string);
	while ((len > 0) && (isspace(string[len-1]) || (string[len-1] == '\"'))) {
	    string[len-1] = '\0';
	    len--;
	}

	if (strstr(str, "TO/CC")) 
	    fields[TO_OR_CC] = strdup(string);
	else if (strstr(str, "TO"))
	    fields[TO] = strdup(string);
	else if (strstr(str, "CC")) 
	    fields[CC] = strdup(string);
	else if (strstr(str, "FROM")) 
	    fields[FROM] = strdup(string);
	else if (strstr(str, "SUBJECT")) 
	    fields[SUBJECT] = strdup(string);
    
	if (string)
	    delete []string;
    }
}

void
RulesStringPair::parseRulesString()
{
    if (!value && !*value)
	return;
    
    // If ignorecase is in the rule, we set the ignorecase 
    if (strstr(value, IGNORE_V) == NULL)
	ignore = 0;
    else
	ignore = 1;

    char *p, *end_p;
    if (!(p = strstr(value, CRITERIA_V)) || !(end_p = strstr(value, SAVE_V))) {
	printf("No criteria\n");
	return;
    }

    // We take the string within keyword "criteria" and "run" for parsing, strip out
    // the outmost bracket before parsing
    char *start, *end;
    start = strchr(p, '('); 
    start ++;
	
    end = end_p - 1;
    while (end > start) {
	if (*end == ')')
	    break;
	end--;
    }

    // We take the string after keyword "criteria" to "run" for parsing
    int len = end - start;
    char *str = new char[len + 1];
    strncpy(str, start, len);
    str[len] = '\0';

    p = strstr(str, HEADER_V);
    while(p) {
	char *s;
	if (end = strstr(p+4, HEADER_V)) {
	    int length = end - (p+7) - 1;
	    s = new char[length+1];
	    strncpy(s, p+7, length);
	    s[length] = '\0';
	    parse_header(s);
	}
	else {
	    s = strdup(p+7);
	    parse_header(s);
	}
	p = end;
	free(s);
    }
    free(str);

    // Check the skip area, if "skipremaining", we turn on skip check box, if
    // "nextrule", we go to the next rule for filtering. If this field is empty,
    // we turn off skip check box. 
    char *end_rule;
    if (end_rule = strstr(end_p, SKIP_V))
	skip = SKIPREMAINING;
    else if (end_rule = strstr(end_p, END_V))
	skip = CONTINUE;
    else {
	printf("illegal rule command, it should end with \"endrule\"");
	return;
    }

    len = end_rule - end_p;
    char *command = new char[len];
    strncpy(command, end_p, len-1);
    command[len-1] = '\0';

    // parse the actions command
    if (p = strstr(command, SAVE_V)) {
	action_num = 1;
	actions = new char*[action_num];
	p = p + 4;
	// Take out leading spaces and double quotes
	while(p && (isspace(*p) || (*p == '\"'))) 
	    p++;

	actions[0] = strdup(p);
	// Take out ending spaces and double quotes
	int length = strlen(actions[0]);
	while ((length > 0) &&  
	      (isspace(actions[0][length-1]) || (actions[0][length-1] == '\"'))) {
	    actions[0][length-1] = '\0';
	    length--;
	}
	shownewmail = strdup(p);
    }

    else if (p = strstr(command, RUN_V)) {
	action_num = 0;
	actions = new char*[MAXACTION];

    while (p) {
	p = p + 4;
	// Take out leading spaces and double quotes
	while(p && (isspace(*p) || (*p == '\"'))) 
	    p++;

        if (end = strstr(p, RUN_V)) {
            int length = end - p - 1;
            actions[action_num] = new char[length+1];
            strncpy(actions[action_num], p, length);
            actions[action_num][length] = '\0';
        }
        else 
            actions[action_num] = strdup(p);

	// Take out ending spaces and double quotes
	int length = strlen(actions[action_num]);
	while ((length > 0) &&  
	      (isspace(actions[action_num][length-1]) || (actions[action_num][length-1] == '\"'))) {
	    actions[action_num][length-1] = '\0';
	    length--;
	}

	action_num++;
	p = end;
    }

    for (int i = 0; i<action_num; i++) {
	if ((p = strstr(actions[i], SAVE_V)) && (*(p+5) != NULL)) {
	    shownewmail = strdup(p+5);
	    break;
	}
    }
    }

    delete []command;
}

void
RulesStringPair::set_state()
{
    if (strstr(value, "ON"))
	rule_state = ON;
    else if (strstr(value, "OFF"))
	rule_state = OFF;
    else {
	printf("illegal grammar, must have the rule state ON or OFF\n");
	rule_state = ON;
    }
}

// The formatted string that is displayed in the scrolling list. We only 
// care about the rule name and the ON/OFF state
char *
RulesStringPair::formatRulesPair()
{
    // The string of the list is formatted with the "ON" "OFF" sign followed 
    // by the rule name.
    char *str;
    if (rule_state) 
	str = strdup(catgets(DT_catd, 4, 6, "ON"));
    else 
	str = strdup(catgets(DT_catd, 4, 7, "OFF"));
    
    char *formatted_str = NULL;
    int i, num_spaces = 0;
    int key_len = strlen(str);

    if (key_len < 13)
	num_spaces = 13 - key_len;
    formatted_str = (char*)malloc(key_len + strlen((char *)label) + num_spaces + 5);
    strcpy(formatted_str, str);
    free(str);

    for (i=key_len; i < (key_len+num_spaces); i++)
	formatted_str[i] = ' ';	
    formatted_str[i] = '\0';	
    strcat(formatted_str, (char*)label);
    return formatted_str;

    return strdup(label);
}

