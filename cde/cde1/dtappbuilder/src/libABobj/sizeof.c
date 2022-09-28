
/*
 *	$XConsortium: sizeof.c /main/cde1_maint/2 1995/10/16 16:02:10 rcs $
 *
 *	@(#)sizeof.c	3.5 11 Feb 1994	
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */


/*
 * sizeof.c
 */

#include <stdio.h>
#include <ab_private/obj.h>

main ()
{
	{
	AB_OBJ			obj;
	AB_OBJ_EXTRA_INFO	*info= &(obj.info);
	printf("sizeof:\n");
	printf("	ABObj:%d (%d + %d)\n", 
			sizeof(obj), 
			sizeof(obj)-sizeof(obj.info),
			sizeof(obj.info));
	printf("	info:%d\n", sizeof(obj.info));
	printf("	action:%d\n", sizeof(info->action));
	printf("	action_list:%d\n", sizeof(info->action_list));
	printf("	button:%d\n", sizeof(info->button));
	printf("	choice:%d\n", sizeof(info->choice));
        printf("        file_chooser:%d\n", sizeof(info->file_chooser));
/*        printf("        message_box:%d\n", sizeof(info->msg_box));     */
	printf("	container:%d\n", sizeof(info->container));
	printf("	drawing_area:%d\n", sizeof(info->drawing_area));
/*	printf("	drop_target:%d\n", sizeof(info->drop_target)); */
	/*printf("	glyph:%d\n", sizeof(info->glyph)); */
	printf("	item:%d\n", sizeof(info->item));
	printf("	label:%d\n", sizeof(info->label));
	printf("	list:%d\n", sizeof(info->list));
	printf("	menu:%d\n", sizeof(info->menu));
/*	printf("	menu_ref:%d\n", sizeof(info->menu_ref)); */
	printf("	module:%d\n", sizeof(info->module));
	printf("	project:%d\n", sizeof(info->project));
	printf("	slider:%d\n", sizeof(info->scale));
	printf("	term:%d\n", sizeof(info->term));
	printf("	text:%d\n", sizeof(info->text));
	printf("	window:%d\n", sizeof(info->window));
	/* printf("	:%d\n", sizeof()); */
	}
}

