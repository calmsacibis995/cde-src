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
# The ToolTalk shell command (ttmv, etc.) utility makefile
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ../..
THIS_DIR = bin/shell
SUBDIRS =

include $(TT_DIR)/GNUmakefile-head.mk

all::		bin_here

COMMANDS = ttmv ttrm ttcp
ALIASES = ttrmdir
COMMANDS_PLATFORM = $(COMMANDS:%=$(PDCOBJDIR)/%)
ALIASES_PLATFORM = $(ALIASES:%=$(PDCOBJDIR)/%)

# ttrcp doesn't work (yet, if ever)

bin_here::	$(COMMANDS_PLATFORM) $(ALIASES_PLATFORM)

INSTALL_COMMANDS = $(COMMANDS:%=$(INSTALL_DIR)/bin/%)
INSTALL_ALIASES =  $(ALIASES:%=$(INSTALL_DIR)/bin/%)

install_here::	$(INSTALL_COMMANDS) $(INSTALL_ALIASES)


$(INSTALL_COMMANDS): $(INSTALL_DIR)/bin/% : $(PDCOBJDIR)/%
		$(TTINSTALL) $(INSTALL_DIR)/bin $<

CONTENTS-ttmv = $(PDCOBJDIR)/ttmv.o $(PDCOBJDIR)/mover.o
CONTENTS-ttrm = $(PDCOBJDIR)/ttrm.o $(PDCOBJDIR)/remover.o
CONTENTS-ttcp = $(PDCOBJDIR)/ttcp.o $(PDCOBJDIR)/copier.o
CONTENTS-ttrcp = $(PDCOBJDIR)/ttrcp.o $(PDCOBJDIR)/rcopier.o


CPPFLAGS	+=	$(CPPFLAGS-bin) $(CPPFLAGS-version)
CCFLAGS		+=	$(CCFLAGS-bin)
CFLAGS		+=	$(CFLAGS-bin)
LDFLAGS		+=	$(LDFLAGS-bin)

ifeq ($(OS)-$(REL.major),SunOS-4)
  LDLIBS += -lX -lI18N
endif

$(PDCOBJDIR)/ttmv : $(PDCOBJDIR) $(CONTENTS-ttmv) $(LIBDEPENDENCY)
	rm -f $@
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LINK.cc) -o $@ $(CONTENTS-ttmv) \
	$(SETLINKMODE) -ltt $(RESETLINKMODE) $(LDLIBS)

$(PDCOBJDIR)/ttrm : $(PDCOBJDIR) $(CONTENTS-ttrm) $(LIBDEPENDENCY)
	rm -f $@
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LINK.cc) -o $@ $(CONTENTS-ttrm) \
	$(SETLINKMODE) -ltt $(RESETLINKMODE) $(LDLIBS)

$(PDCOBJDIR)/ttcp : $(PDCOBJDIR) $(CONTENTS-ttcp) $(LIBDEPENDENCY)
	rm -f $@
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LINK.cc) -o $@ $(CONTENTS-ttcp) \
	$(SETLINKMODE) -ltt $(RESETLINKMODE) $(LDLIBS)

$(PDCOBJDIR)/ttrcp : $(PDCOBJDIR) $(CONTENTS-ttrcp) $(LIBDEPENDENCY)
	rm -f $@
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LINK.cc) -o $@ $(CONTENTS-ttrcp) \
	$(SETLINKMODE) -ltt $(RESETLINKMODE) $(LDLIBS)

# We used to have an elaborate way of setting macros for commands and
# their aliases, but that seems silly since there is exactly one.
$(PDCOBJDIR)/ttrmdir:	$(PDCOBJDIR)/ttrm
	rm -f $(PDCOBJDIR)/ttrmdir;ln $(PDCOBJDIR)/ttrm $(PDCOBJDIR)/ttrmdir

$(INSTALL_DIR)/bin/ttrmdir:	$(INSTALL_DIR)/bin/ttrm
	(cd $(INSTALL_DIR)/bin;rm -f ttrmdir;ln ttrm ttrmdir)

include $(TT_DIR)/GNUmakefile-tail.mk
