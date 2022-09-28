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
# @(#)GNUmakefile	1.8 93/11/03
# The ToolTalk server-library makefile
#
# This directory contains routines that are not needed in the
# ToolTalk client library, yet are shared among multiple
# ToolTalk binaries (ttsession, tt_type_comp, ttdbck)
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ..
THIS_DIR = slib
SUBDIRS=

include $(TT_DIR)/GNUmakefile-head.mk

all::		proto_here

proto_here::	$(PDCOBJDIR)/libstt.a

Clean::
	$(RM) $(PDCOBJDIR)/libstt.a


LIBSTT_CONTENTS= \
mp_ce_attrs.o \
mp_observer.o		 mp_observer_utils.o				\
mp_otype.o		 mp_otype_utils.o	 mp_ptype.o		\
mp_ptype_utils.o	 mp_rpc_implement.o	 mp_rpc_server.o	\
mp_rpc_server_utils.o	 mp_s_file.o		 mp_s_file_utils.o	\
mp_s_message.o		 mp_s_message_utils.o	 mp_s_mp.o		\
mp_s_pattern.o		 mp_s_pattern_utils.o	 mp_s_procid.o		\
mp_s_procid_utils.o	 mp_s_msg_context.o	 mp_s_pat_context.o	\
mp_s_session.o		 mp_s_session_prop.o	 mp_s_session_utils.o	\
mp_s_xdr_functions.o 	mp_self_procid.o \
mp_signature.o		 mp_signature_utils.o	 mp_typedb.o		\
mp_typedb_utils.o


# A few modules need additional include libraries or other flags
# which we handle with special case static pattern rules.  This
# keeps us from developing the "everything includes everything"
# disease so often seen, and not incidentally keeps the size of
# the make logs down.

# mp_typedb and mp_s_session need X11 stuff

$(PDCOBJDIR)/mp_typedb.o $(PDCOBJDIR)/mp_s_session.o: $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(X11INCLUDES) -o $@ $<
	@$(MCS_COMMAND)

OBJECTS	=	$(LIBSTT_CONTENTS:%=$(PDCOBJDIR)/%)

$(PDCOBJDIR)/libstt.a:		$(PDCOBJDIR) $(OBJECTS)
	rm -fr $@;$(AR) qc $@ $(OBJECTS); $(RANLIB) $@

include $(TT_DIR)/GNUmakefile-tail.mk
