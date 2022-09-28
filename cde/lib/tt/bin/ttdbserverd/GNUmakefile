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
# @(#)GNUmakefile	1.3 93/09/07
#
# The ToolTalk rpc.ttdbserverd server makefile
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ../..
THIS_DIR = bin/ttdbserverd
SUBDIRS=

include $(TT_DIR)/GNUmakefile-head.mk

all::		bin install

bin_here::	$(PDCOBJDIR)/rpc.ttdbserverd

install_here::	$(INSTALL_DIR)/bin/rpc.ttdbserverd

TTDBSERVER_CONTENTS = \
	db_server_functions.o \
        db_server_svc.o \
	dm_access_cache.o \
	dm_server.o \
	tt_db_message_info_utils.o \
	tt_db_msg_q_lock.o \
	tt_db_msg_q_lock_entry_utils.o \
	tt_db_partition_global_map_ref.o \
	tt_db_partition_redirection_map.o \
	tt_db_server_db.o \
	tt_db_server_db_utils.o \
	tt_isam_file.o \
	tt_isam_file_utils.o \
	tt_isam_key_descriptor.o \
	tt_isam_key_descriptor_utils.o \
	tt_isam_record.o \
	tt_isam_record_utils.o

ifeq ($(OS),AIX)
TTDBSERVER_CONTENTS += \
	nonsharelibC.o
endif		

CPPFLAGS	+=	$(CPPFLAGS-bin) -I$(TT_DIR)/slib
CCFLAGS		+=	$(CCFLAGS-bin)
CFLAGS		+=	$(CFLAGS-bin)
LDFLAGS		+=	$(LDFLAGS-bin)

#ifeq ($(OS)-$(REL.major),SunOS-5)
# LDFLAGS += -M Mapfile
#endif

ifeq ($(OS)-$(REL.major),SunOS-4)
  LDLIBS	+=	-lX -lI18N
endif

CPPFLAGS += $(CPPFLAGS-isam)

EXTRA_LD_RUN_PATH-g=	/tt/proto/lib:

ISAM_USERS = \
db_server_functions.o \
dm_access_cache.o \
dm_server.o \
tt_db_partition_global_map_ref.o \
tt_db_server_db.o \
tt_db_server_db_utils.o \
tt_isam_file.o \
tt_isam_file_utils.o \
tt_isam_key_descriptor.o \
tt_isam_key_descriptor_utils.o \
tt_isam_record.o \
tt_isam_record_utils.o

PDC_ISAM_USERS = $(ISAM_USERS:%=$(PDCOBJDIR)/%)

$(PDC_ISAM_USERS): $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(CPPFLAGS_isam) -o $@ $<

$(PDCOBJDIR)/db_server_svc.o: $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(CPPFLAGS_isam) $(CPPFLAGS-version) -o $@ $<

OBJECTS =	$(TTDBSERVER_CONTENTS:%=$(PDCOBJDIR)/%)

$(PDCOBJDIR)/rpc.ttdbserverd:	$(PDCOBJDIR) $(OBJECTS) $(LIBDEPENDENCY)
	rm -f $@
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LINK.cc) -o $@ $(OBJECTS) \
	$(SETLINKMODE) -ltt -lisam $(RESETLINKMODE) $(LDLIBS)

include $(TT_DIR)/GNUmakefile-tail.mk
