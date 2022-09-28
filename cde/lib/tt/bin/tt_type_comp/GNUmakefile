# %% 	$Revision: 1.4 $							
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
# @(#)GNUmakefile	1.3 93/09/07
#
# The ToolTalk tt_type_comp utility makefile
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ../..
THIS_DIR = bin/tt_type_comp

include $(TT_DIR)/GNUmakefile-head.mk

all::		bin install

bin_here::	$(PDCOBJDIR)/tt_type_comp

install_here::	$(INSTALL_DIR)/bin/tt_type_comp

TTTYPECOMP_CONTENTS = \
	mp_type_comp.o \
	mp_types_table.o \
	mp_types_gram.o \
	mp_types_lex.o


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


%.C : frozen.%.C
	rm -f $@
	cp $< $@

%.h : frozen.%.h
	rm -f $@
	cp $< $@

# The dependency on mp_type_gram.h is buried deep enough
# I don't trust make depend to find it.

$(PDCOBJDIR)/mp_types_lex.o: mp_types_gram.h

OBJECTS =	$(TTTYPECOMP_CONTENTS:%=$(PDCOBJDIR)/%)

# We should probably try to generate a standard link for utilities
# Trailing -Bstatic ensures libC is linked statically -- that could
# go away in the future when libC is bundled.

$(PDCOBJDIR)/tt_type_comp:	$(PDCOBJDIR) $(OBJECTS) \
				$(LIBDEPENDENCY) $(SLIBDEPENDENCY)
	rm -f $@
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LINK.cc) -o $@ $(OBJECTS) \
	$(SETLINKMODE) -lstt -ltt $(RESETLINKMODE) $(LDLIBS)

include $(TT_DIR)/GNUmakefile-tail.mk

