/* $XConsortium: spell.h /main/cde1_maint/1 1995/07/17 16:40:29 drk $ */
/* @(#)spell.h	1.11  @(#)spell.h	1.11 01/20/96 15:41:40 */
/*
 * COMPONENT_NAME: (PIITOOLS) AIX PII Tools
 *
 * FUNCTIONS: Standard header file for spell.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
char *spell_check();
char *all_syn();
#define SYNLEN 256

typedef struct {
  char bwd[30];
  char *syn[30];
} SPLT;

