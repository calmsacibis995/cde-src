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
# The ToolTalk ttdbck utility makefile
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ../..
THIS_DIR = bin/dbck

include $(TT_DIR)/GNUmakefile-head.mk

all::	bin install

bin_here::	$(PDCOBJDIR)/ttdbck

install_here::	$(INSTALL_DIR)/bin/ttdbck

TTDBCK_CONTENTS = \
	ttdbck.o \
	binkey.o \
	binkey_utils.o \
	common.o \
	options.o \
	options_tt.o \
	prop.o \
	prop_utils.o \
	spec.o \
	spec_repair.o \
	spec_utils.o

ifeq ($(OS),AIX)
TTDBCK_CONTENTS += \
	nonsharelibC.o
endif	


CPPFLAGS	+=	$(CPPFLAGS-bin) -I$(TT_DIR)/slib $(CPPFLAGS-isam)
CCFLAGS		+=	$(CCFLAGS-bin)
CFLAGS		+=	$(CFLAGS-bin)
LDFLAGS		+=	$(LDFLAGS-bin)


ifeq ($(OS)-$(REL.major),SunOS-4)
  LDLIBS	+=	-lce -lX -lI18N -ldl
endif


$(PDCOBJDIR)/options.o : $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(CPPFLAGS-version) -o $@ $<


OBJECTS =	$(TTDBCK_CONTENTS:%=$(PDCOBJDIR)/%)

$(PDCOBJDIR)/ttdbck: $(PDCOBJDIR) $(OBJECTS) $(LIBDEPENDENCY) $(SLIBDEPENDENCY)
	rm -f $@
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LINK.cc) -o $@ $(OBJECTS) \
	$(SETLINKMODE) -lstt -ltt -lisam  $(RESETLINKMODE) $(LDLIBS)

include $(TT_DIR)/GNUmakefile-tail.mk
