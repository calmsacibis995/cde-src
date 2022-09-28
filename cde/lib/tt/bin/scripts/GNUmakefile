# %% 	$Revision: 1.2 $							
# %% 									
# %%  	RESTRICTED CONFIDENTIAL INFORMATION:                            
# %% 									
# %% 	The information in this document is subject to special		
# %% 	restrictions in a confidential disclosure agreement between	
# %% 	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this	
# %% 	document outside HP, IBM, Sun, USL, SCO, or Univel without	
# %% 	Sun's specific written approval.  This document and all copies	
# %% 	and derivative works thereof must be returned or destroyed at	
# %% 	Sun's request.							
# %% 									
# %% 	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.	
# %% 									
#
# @(#)GNUmakefile	1.3 93/09/07
#
# The ToolTalk shell script utility makefile
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#
# Any shell scripts shipped as part of ToolTalk are collected here.

TT_DIR = ../..
THIS_DIR = bin/tt_type_comp

include $(TT_DIR)/GNUmakefile-head.mk

ifeq ($(OS)-$(REL.major),SunOS-5)
 SCRIPTS = ttce2xdr
else
 SCRIPTS = 
endif

all::			bin install

# Scripts don't really vary by platform or variant, but it's more consistent
# to build all derived files down a level in the platform-variant directory.

$(PDCOBJDIR)/ttce2xdr: $(PDCOBJDIR) ttce2xdr.sh
	rm -f $(PDCOBJDIR)/ttce2xdr ;\
	cat ttce2xdr.sh |\
	sed "s/TT_VERSION_STRING/$(TT_VERSION)/" > $(PDCOBJDIR)/ttce2xdr ;\
	chmod +x $(PDCOBJDIR)/ttce2xdr

bin_here::	$(SCRIPTS:%=$(PDCOBJDIR)/%)

install_here::	$(SCRIPTS:%=$(INSTALL_DIR)/bin/%)

include $(TT_DIR)/GNUmakefile-tail.mk

