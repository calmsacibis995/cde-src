/* $XConsortium: select.h /main/cde1_maint/1 1995/07/17 16:39:47 drk $ */
/* @(#)select.h	1.11  @(#)select.h	1.11 01/20/96 15:41:36 */
/*
 * COMPONENT_NAME: (PIITOOLS) AIX PII Tools
 *
 * FUNCTIONS: Standard header file for select
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
typedef struct {
  int eov,                      /* Exited option video mode  */
      ewv,                      /* Exited window video mode  */
      cov,                      /* Current option video mode */
      cwv;                      /* Current window video mode */
} VID_MODES ;
