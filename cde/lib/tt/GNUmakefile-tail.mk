# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)GNUmakefile-tail.mk	1.3 93/09/07
# Copyright (c) 1992 by Sun Microsystems, Inc.

# Define all the "here" targets as double colon targets, so we don't
# have to put empty ones in all the individual makefiles.

bin_here install_here proto_here bb_here :: 

clean_here::
		rm -fr $(PDCOBJDIR) $(PICOBJDIR)

sccs-clean_here::
		sccs clean


$(PICOBJDIR) $(PDCOBJDIR):
	test -d $@ || mkdir $@

# Rules for making .o files from .C or .C files

# Rules for making .i files (cpp output) from .c or .C files.
# used for debugging preprocessor problems.

Derived-src/%.i:	%.C
	$(PREPROCESS.cc)

Derived-src/%.i:	%.c
	$(PREPROCESS.c)

# Rules for making .files from .C, listing all includes and nested includes.

Derived-src/%.files:	%.C
	$(LISTINCLUDES.cc)


# Rules for making C source from C++ source, on those rare occasions
# you need to see the output of cfront.

Derived-src/%.c:	%.C
	$(CFRONT.cc)

# Rules for making assembly source from C++ source, on those even rarer
# occasions you need to see that.

Derived-src/%.s:	%.C
	$(LISTASM.cc)

# Rule for installing binaries.

$(INSTALL_DIR)/bin/% : $(PDCOBJDIR)/%
	$(TTINSTALL) $(INSTALL_DIR)/bin $<



#
# Recursive targets.
#
_RECURSIVE_TARGETS := bin clean sccs-clean install proto bb 

ifdef SUBDIRS

# Let e.g. "bin" depend on ".recursive-bin." and "bin_here"
# The reason for the two level scheme is to ensure subdirs are built
# before "<foo>_here" is built.

$(_RECURSIVE_TARGETS): % : .recursive-%. %_here

# Create a list of e.g. ".recursive-bin." targets
_RECURSIVE	:= $(_RECURSIVE_TARGETS:%=.recursive-%.)

$(_RECURSIVE) : .recursive-%. :
	@for dir in $(SUBDIRS) ; do \
		cd $$dir; \
		$(MAKE) $(@:.recursive-%.=%) ; \
		cd .. ; \
	done
else

$(_RECURSIVE_TARGETS): % : %_here

endif


FRC:
