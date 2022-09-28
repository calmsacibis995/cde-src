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
# @(#)SunMakefile	1.23 93/07/31
# The ToolTalk top level Makefile
#
# Copyright (c) 1991-1993 by Sun Microsystems, Inc.
#

# Most common make targets.  All these are recursive over all appropriate
# directories.
#
# make		- same as make all
# make all	- make proto bin install
# make proto	- make prototype lib and includes, put in PROTO_DIR
# make bin	- make programs based on proto build
# make install  - install programs and libraries built by make proto & make bin
# make clean	- remove derived files
# make sccs-clean - remove sccs work files, force sccs gets

# Specify the variant (-g, -O, -gprof, -tcov) with the VARIANT macro.
# e.g "gnumake VARIANT=-O all"


TT_DIR = .

THIS_DIR = .

SUBDIRS = mini_isam lib slib bin 

include GNUmakefile-head.mk


# Normally during development, make libraries -g, binaries -g, then install.
all::		proto bin install

clean_here::
	$(RM) -r $(PROTO_DIR) ttinstall

# always make ttinstall.  I can't figure out how to do this only 
# when doing actual makes since the "here" targets get done after the
# recursive targets, so adding ttinstall to HEADER_DEPENDENCIES gets
# ttinstall done too late.

proto bin install:: ttinstall

# We should make these targets do something (at least depend)

includes depend::

include GNUmakefile-tail.mk
