
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
 
#
# The ToolTalk server-library makefile
#
# This directory contains routines that are not needed in the
# ToolTalk client library, yet are shared among multiple
# ToolTalk binaries (ttsession, tt_type_comp, ttdbck)
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ..
THIS_DIR = mini_isam
SUBDIRS=

include $(TT_DIR)/GNUmakefile-head.mk

# alone among ToolTalk components, isam does not need headers from
# $(TT_LIB)/lib.  We used to omit them from the compiles by doing
#  CPPFLAGS = 
# here, but that makes it impossible to push needed -D options
# in from -head.  Probably ought to have separate CPPINCL and CPPDEFINE
# macros.

all::	$(PDCOBJDIR)/libisam.a

proto_here::	$(PDCOBJDIR)/libisam.a

clean_here::
	$(RM) -fr $(PDCOBJDIR)/libisam.a

LIBISAM_CONTENTS= \
isaddindex.o isamaddindex.o isopen.o isamopen.o isdelcurr.o isamdelcurr.o \
isdelrec.o isamdelrec.o isbuild.o isvars.o isrewrec.o iserase.o iscntl.o \
isstart.o isaddprimary.o isrewcurr.o isclose.o isread.o iswrite.o \
isamwrite.o issync.o isapplmr.o ismngfcb.o issort.o ischange1key.o isfab.o \
isdiskbufs2.o isfd.o isbtree.o isdel1key.o isfcb.o isfixrec.o issignals.o \
isapplmw.o isfcbwatchfd.o isamhooks.o isfcbindex.o isalloc.o isbytearray.o \
isdatconv.o isminmax.o isfname.o isvarrec.o isperm.o iserror.o iskeycmp.o \
iskeycalc.o iskeyvalid.o isamerror.o iskeyaux.o isfileio.o \
ispageio.o isdlink.o isbtree2.o isbtree3.o isindfreel.o isindexconv.o \
isbsearch.o isadd1key.o iskeyconv.o isversion.o isrepair.o iswrrec.o \
isrename.o isindexinfo.o 

OBJECTS	=	$(LIBISAM_CONTENTS:%=$(PDCOBJDIR)/%)

$(PDCOBJDIR)/libisam.a:		$(PDCOBJDIR) $(OBJECTS)
	rm -fr $@;$(AR) qc $@ $(OBJECTS); $(RANLIB) $@

$(PDCOBJDIR)/isversion.o : $(PDCOBJDIR)/%.o : %.c
	rm -f $@
	$(COMPILE.c) $(CPPFLAGS-version) -o $@ $<	

include $(TT_DIR)/GNUmakefile-tail.mk
