# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.3 93/07/30

TT_DIR	= ../..

include $(TT_DIR)/simple-macros.mk

# ttce2xdr is a migration tool.  It is not needed on any platform but
# Solaris.

#all:	ttce2xdr
all:	

ttce2xdr:	ttce2xdr.sh
	$(RM) ttce2xdr; \
	cat ttce2xdr.sh | sed "s/TT_VERSION_STRING/$(TT_VERSION)/" >ttce2xdr;\
	chmod ugo+x ttce2xdr

clean:
	$(RM) ttce2xdr

install: 
