# %% 	$Revision: 1.3 $							
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
# The ToolTalk ttsession server makefile
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ../..
THIS_DIR = bin/ttsession
SUBDIRS=

include $(TT_DIR)/GNUmakefile-head.mk

all::		bin install

bin_here::	$(PDCOBJDIR)/ttsession

install_here::	$(INSTALL_DIR)/bin/ttsession

bb_here::	bin_here install_here

TTSESSION_CONTENTS =	mp_server.o

CPPFLAGS	+=	$(CPPFLAGS-bin) -I$(TT_DIR)/slib
CCFLAGS		+=	$(CCFLAGS-bin)
CFLAGS		+=	$(CFLAGS-bin)
LDFLAGS		+=	$(LDFLAGS-bin) -L$(TT_DIR)/slib/$(PDCOBJDIR)

ifeq ($(OS)-$(REL.major),SunOS-4)
  LDLIBS += -lce -lX -lI18N -ldl
endif
ifeq ($(OS)-$(REL.major),SunOS-5)
  LDLIBS += -lw
endif

$(PDCOBJDIR)/mp_server.o : $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(CPPFLAGS-version) -o $@ $<

OBJECTS =	$(TTSESSION_CONTENTS:%=$(PDCOBJDIR)/%)

# We don't try to generate a general rule for linking ttsession, since
# it links differently from any other binary in the system anyway.

$(PDCOBJDIR)/ttsession:	$(PDCOBJDIR) $(OBJECTS) \
			$(SLIBDEPENDENCY) $(LIBDEPENDENCY)
	LD_RUN_PATH=$(LD_RUN_PATH) \
	rm -f $@
	$(LINK.cc) -o $@ $(OBJECTS) \
	$(SETLINKMODE) -lstt -ltt $(RESETLINKMODE) $(LDLIBS)

include $(TT_DIR)/GNUmakefile-tail.mk
