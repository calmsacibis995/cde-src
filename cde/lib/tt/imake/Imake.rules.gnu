/*
 * Imake.rules.gnu 1.1	2/23/93
 */

#define ValFromShell(variable,cmd)	variable = $(shell cmd)
#define WhenAssign(target,assignment)	assignment
#define AppendVal(variable,value)	variable := $(variable) value
