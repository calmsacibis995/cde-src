
/*
 *      $XConsortium: objxm_configP.h /main/cde1_maint/2 1995/10/16 10:41:47 rcs $
 *
 * @(#)objxm_configP.h	1.10 01/20/96      cde_app_builder/src/libABobjXm
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

/*
 * abxm_config.h - implements the configuring of an object
 * 		   into the appropriate Motif obj hierarchy
 *
 * An XmConfigured Object consists of a tree of objects which
 * matches the required Motif widget hierarchy for that object
 * type.  For Example, an XmConfigured TextField will look like:
 *
 *			obj (RootObj)
 *		     (XmRowColumn)
 *			 |
 *		 ------------------
 *		|		   |
 *	      obj (SubObj)       obj (SubObj)
 *	 (XmLabel)		(XmTextField)
 *
 *
 */
#ifndef _ABXM_CONFIG_H_
#define _ABXM_CONFIG_H_

#include <X11/Intrinsic.h>

typedef struct _XM_CONFIG_INFO_REC    XmConfigInfoRec;
typedef struct _XM_CONFIG_INFO_REC    *XmConfigInfo;

/*
 * define types for Xm-Configure object "methods"
 */
typedef int    	(*XmConfigProc) (
    ABObj 	obj
);

typedef BOOL    (*XmConfigIsaFunc) (
    ABObj	obj
);

typedef ABObj	(*XmConfigGetObjProc) (
    ABObj	obj,
    AB_CFG_OBJ_TYPE type
);

/*
 * XmConfigInfo Structure - one for each type of object supported
 */
typedef struct _XM_CONFIG_INFO_REC {
    /* Methods...*/
    XmConfigProc  	xmconfig;      /* configures obj hierarchy */
    XmConfigGetObjProc  get_config_obj;/* returns config obj       */ 
    XmConfigIsaFunc     is_a_test;     /* "is a" test              */
} XM_CONFIG_INFO_REC;



/* Function to initialize XmConfigInfo for all AB object types 
 */
extern int	abxm_init_obj_config_info(
		);

/* Function to register XmConfigInfo for a single object type 
 */
extern void    	abxm_register_config_info(
            	    XmConfigInfo cfginfo
        	);

/* Function which returns XmConfigInfo ptr for an object 
 */
extern XmConfigInfo 
		abxm_get_config_info(
            	    ABObj        obj
        	);

/* Function which XmConfigures an object 
 */
extern int    	abxm_configure_obj(
            	    ABObj        obj
        	);

/* Function which XmConfigures an entire tree 
 */
extern int	abxm_configure_tree(
		    ABObj	root
		);

/* Function which re-configures attachments after 
 * objects have been xmconfigured 
 */ 
extern int      abxm_configure_attachments( 
                    ABObj       obj 
                ); 

/* Function which returns the correct Composite SubObj corresponding
 * to 'type'.
 */
extern ABObj	abxm_get_config_obj(
		    ABObj	obj,
		    AB_CFG_OBJ_TYPE type
		);

#endif /* _ABXM_CONFIG_H_ */
