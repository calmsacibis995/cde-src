# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#) 31 Jul 1993
# Copyright (c) 1992 by Sun Microsystems, Inc.

LIBDEPENDENCY	=	$(PROTO_DIR)/lib/libtt.so.2
# tcov and gprof use strictly static libs.
%-tcov %-pg := LIBDEPENDENCY =
SLIBDEPENDENCY	=	$(PROTO_DIR)/lib/libstt.a

# Use a special variant for cfront/cpp output

%.c %.i %.files	%.s	:=	VARIANT =	-source

# Set up macros based on variant

DERIVED		= 	Derived$(VARIANT)
CFLAGS		+=	$(CFLAGS$(VARIANT))
CCFLAGS		+=	$(CCFLAGS$(VARIANT)) 
LDFLAGS		+=	$(LDFLAGS$(VARIANT))

$(VARIANT_DIRS):
	test -d $@ || mkdir -p $@

# Rules for making objects from sources.

$$(DERIVED)/%.o:	%.C
	$(COMPILE.cc) -o $@ $<

$$(DERIVED)/%.o:	 %.c
	$(COMPILE.c) -o $@ $<

# Rules for making .i files (cpp output) from .c or .C files.
# used for debugging preprocessor problems.

$$(DERIVED)/%.i:	%.C
	$(PREPROCESS.cc)

$$(DERIVED)/%.i:	%.c
	$(PREPROCESS.c)

# Rules for making .files from .C, listing all includes and nested includes.

$$(DERIVED)/%.files:	%.C
	$(LISTINCLUDES.cc)


# Rules for making C source from C++ source, on those rare occasions
# you need to see the output of cfront.

$$(DERIVED)/%.c:	%.C
	$(CFRONT.cc)

# Rules for making assembly source from C++ source, on those even rarer
# occasions you need to see that.

$$(DERIVED)/%.s:	%.C
	$(LISTASM.cc)

# Rule for installing binaries.

$(INSTALL_DIR)/bin/%: %
		$(TTINSTALL) $(INSTALL_DIR)/bin $(@F)


#
# Recursive targets
#
_RECURSIVE_TARGETS= \
$(VARIANTS:X%=bin%) clean sccs-clean $(VARIANTS:X%=install%) $(VARIANTS:X%=proto%) $(VARIANTS:X%=bb%)

bin-g		:=	_TARGET_BELOW = bin-g
bin-O		:=	_TARGET_BELOW = bin-O
bin-pg		:=	_TARGET_BELOW = bin-pg
bin-tcov 	:=	_TARGET_BELOW = bin-tcov
bin-lprof 	:=	_TARGET_BELOW = bin-lprof
install-g	:=	_TARGET_BELOW = install-g
install-O	:=	_TARGET_BELOW = install-O
install-pg	:=	_TARGET_BELOW = install-pg
install-tcov 	:=	_TARGET_BELOW = install-tcov
install-lprof 	:=	_TARGET_BELOW = install-lprof
proto-g		:=	_TARGET_BELOW = proto-g
proto-O		:=	_TARGET_BELOW = proto-O
proto-pg	:=	_TARGET_BELOW = proto-pg
proto-tcov 	:=	_TARGET_BELOW = proto-tcov
proto-lprof 	:=	_TARGET_BELOW = proto-lprof
bb-g		:=	_TARGET_BELOW = bb-g
bb-O		:=	_TARGET_BELOW = bb-O
bb-pg		:=	_TARGET_BELOW = bb-pg
bb-tcov 	:=	_TARGET_BELOW = bb-tcov
bb-lprof 	:=	_TARGET_BELOW = bb-lprof
clean		:=	_TARGET_BELOW = clean
sccs-clean	:=	_TARGET_BELOW = sccs-clean

bin-g		:=	_TARGET_HERE = Bin-g
bin-O		:=	_TARGET_HERE = Bin-O
bin-pg		:=	_TARGET_HERE = Bin-pg
bin-tcov 	:=	_TARGET_HERE = Bin-tcov
bin-lprof 	:=	_TARGET_HERE = Bin-lprof
install-g	:=	_TARGET_HERE = Install-g
install-O	:=	_TARGET_HERE = Install-O
install-pg	:=	_TARGET_HERE = Install-pg
install-tcov 	:=	_TARGET_HERE = Install-tcov
install-lprof 	:=	_TARGET_HERE = Install-lprof
proto-g		:=	_TARGET_HERE = Proto-g
proto-O		:=	_TARGET_HERE = Proto-O
proto-pg	:=	_TARGET_HERE = Proto-pg
proto-tcov 	:=	_TARGET_HERE = Proto-tcov
proto-lprof 	:=	_TARGET_HERE = Proto-lprof
bb-g		:=	_TARGET_HERE = Bb-g
bb-O		:=	_TARGET_HERE = Bb-O
bb-pg		:=	_TARGET_HERE = Bb-pg
bb-tcov 	:=	_TARGET_HERE = Bb-tcov
bb-lprof 	:=	_TARGET_HERE = Bb-lprof
clean		:=	_TARGET_HERE = Clean
sccs-clean	:=	_TARGET_HERE = Sccs-clean

# Let e.g. "bin" depend on ".recursive-bin." and "Bin"
_RECURSIVE_TARGET = $(@:%=.recursive-%.)
$(_RECURSIVE_TARGETS): $$(_RECURSIVE_TARGET) $$(_TARGET_HERE)

# Create a list of ".recursive-$@." targets
_RECURSIVE	= $(_RECURSIVE_TARGETS:%=.recursive-%.)

# Tag the list of subdirectories so the pattern rule below can be very
# specific and not catch unwanted targets.
_SUB_DIRS_EXPANDED = $(SUBDIRS:%=.recursive_dir-%.)

# Make each of the recursive targets depend on all the dirs to dive into.
$(_RECURSIVE): $$(_SUB_DIRS_EXPANDED)

# And define a pattern rule that will do the recursion.
# <<Make automatically passes in the $(MAKEFLAGS) when using the $(MAKE) macro>>
.recursive_dir-%.: FRC
	@if [ -d $(@:.recursive_dir-%.=%) ] ; \
	then	cd $(@:.recursive_dir-%.=%) ; \
		echo Down to `pwd` ; \
		$(MAKE) -f SunMakefile $(_TARGET_BELOW) ; \
		cd ..; \
		echo  Up to `pwd` ;\
	fi

FRC:
