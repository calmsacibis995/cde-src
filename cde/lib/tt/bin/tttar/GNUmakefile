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
#
# @(#)GNUmakefile	1.3 93/09/07
#
# The ToolTalk tttar utility makefile
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ../..
THIS_DIR = bin/tttar
SUBDIRS=

include $(TT_DIR)/GNUmakefile-head.mk

all::		bin install

bin_here::	$(PDCOBJDIR)/tttar

install_here::	$(INSTALL_DIR)/bin/tttar

TTTAR_CONTENTS = \
	tttar.o \
	archiver.o \
	tttar_utils.o \
	tttar_api.o \
	tttar_spec.o \
	tttar_file_utils.o \
	tttar_string_map.o

CPPFLAGS	+=	$(CPPFLAGS-bin)
CCFLAGS		+=	$(CCFLAGS-bin)
CFLAGS		+=	$(CFLAGS-bin)
LDFLAGS		+=	$(LDFLAGS-bin)

ifeq ($(OS)-$(REL.major),SunOS-4)
  LDLIBS += -lX -lI18N
endif

$(PDCOBJDIR)/archiver.o : $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(CPPFLAGS-version) -o $@ $<

OBJECTS =	$(TTTAR_CONTENTS:%=$(PDCOBJDIR)/%)

$(PDCOBJDIR)/tttar:	$(PDCOBJDIR) $(OBJECTS) $(LIBDEPENDENCY)
	LD_RUN_PATH=$(LD_RUN_PATH) \
	rm -f $@
	$(LINK.cc) -o $@ $(OBJECTS) \
	$(SETLINKMODE) -ltt $(RESETLINKMODE) $(LDLIBS)

include $(TT_DIR)/GNUmakefile-tail.mk
