# %% 	$Revision: 1.1 $							
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
#
# @(#)GNUmakefile	1.1 93/11/09
#
# The ToolTalk tttrace utility GNUmakefile
#
# Copyright (c) 1993 by Sun Microsystems, Inc.
#

TT_DIR = ../..
THIS_DIR = bin/tttrace

include $(TT_DIR)/GNUmakefile-head.mk

all::		bin install

bin_here::	$(PDCOBJDIR)/tttrace

install_here::	$(INSTALL_DIR)/bin/tttrace

TTTRACE_CONTENTS = tttrace.o tttrace_objs.o

CPPFLAGS	+=	$(CPPFLAGS-bin) -I$(TT_DIR)/slib
CCFLAGS		+=	$(CCFLAGS-bin)
CFLAGS		+=	$(CFLAGS-bin)
LDFLAGS		+=	$(LDFLAGS-bin)

ifeq ($(OS)-$(REL.major),SunOS-4)
  LDLIBS += -lce -lX -lI18N -ldl
endif

$(PDCOBJDIR)/mp_type_comp.o : $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(CPPFLAGS-version) -o $@ $<

OBJECTS =	$(TTTRACE_CONTENTS:%=$(PDCOBJDIR)/%)

# We should probably try to generate a standard link for utilities
# Trailing -Bstatic ensures libC is linked statically -- that could
# go away in the future when libC is bundled.

$(PDCOBJDIR)/tttrace:	$(PDCOBJDIR) $(OBJECTS) \
				$(LIBDEPENDENCY) 
	rm -f $@
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LINK.cc) -o $@ $(OBJECTS) \
	$(SETLINKMODE) -ltt $(RESETLINKMODE) $(LDLIBS)

include $(TT_DIR)/GNUmakefile-tail.mk

