# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.4 93/07/30

# The simple.mk makefiles are very simple makefiles that just build
# one variant of the system. They should run with just about any
# version of make (we've tried them with GNU make, SunPro make,
# and some close descendants of UNIX System V.2 make.)

# You'll need to change the macros in simple-macros.mk to reflect
# local conditions.  Really, we expect most people porting ToolTalk
# will end up completely redoing the makefiles to match local requirements.
# Use the simple.mk makefiles, plus the Makefiles (the SunPro make
# versions we use at ToolTalk Galactic HQ) and the GNUMakeFiles
# you can build with imake as references.

SUBDIRS = mini_isam lib slib bin

all clean install:	FRC
	@for d in $(SUBDIRS);\
	do	cd $$d ;\
		echo Down to `pwd`;\
		$(MAKE) -f simple.mk $@;\
		cd ..;\
		echo Up to `pwd`;\
	done

FRC:
