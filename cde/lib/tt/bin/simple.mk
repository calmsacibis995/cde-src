# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.3 93/07/30

SUBDIRS = ttsession tt_type_comp ttdbserverd shell tttar dbck scripts 


all clean install: FRC
	@for d in $(SUBDIRS);\
	do	cd $$d ;\
		echo Down to `pwd`;\
		$(MAKE) -f simple.mk $@;\
		cd ..;\
		echo Up to `pwd`;\
	done

FRC:
