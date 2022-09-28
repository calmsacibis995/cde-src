# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)SunMakefile-head.mk	1.63 95/02/08
# Copyright (c) 1992 by Sun Microsystems, Inc.

.KEEP_STATE:

all::

NETISAM		=	$(TT_DIR)/mini_isam
# the first part of the version string is the ToolTalk version number
# which only changes when ToolTalk makes functional changes.  The 
# second part (after the underscore) is the OpenWindows build identifier.
# Increment the second part (only) for every OpenWindows build.
TT_VERSION	=	1.2_dev
PROTO_DIR 	=	$(TT_DIR)/proto
# if we are building in the OpenWindows tree, DESTDIR is set to the
# prototype OpenWindows, which is where we should install.
INSTALL_DIR:sh	= 	echo ${DESTDIR:-'$(TT_DIR)/install'}

# Default variant is debug. Generally this is overridden by
# conditional macros in the various subdirectory makefiles.
VARIANT		=	-g

# Possible variants:
# debug (-g)		- only for development
# optimized (-O)	- build product this way
# tcov (-tcov)		- for test coverage and performance tuning
# lprof(-lprof)		- for test coverage and performance tuning
# gprof (-pg)		- for performance tuning
# src (-source)		- pseudovariant for storing preprocessor output
VARIANTS	=	X-g X-O X-tcov X-lprof X-pg X-source

OS_NAME:sh = uname
OS_VERSION:sh = uname -r | sed 's/\..*//'

CC_glob		=	cc
CC_SunOS_4	=	acc
CC_loc		=	$(CC_$(OS_NAME)_$(OS_VERSION))
CC		=	$(CC_glob$(CC_loc))$(CC_loc)

REORD_FLAG_glob	=
REORD_FLAG_SunOS_5=	-xF
REORD_FLAG_loc	=	$(REORD_FLAG_$(OS_NAME)_$(OS_VERSION))
REORD_FLAG	=	$(REORD_FLAG_glob$(REORD_FLAG_loc))$(REORD_FLAG_loc)

CFLAGS-g	=	$(REORD_FLAG) -g
# Normally, do not generate debug symbols for _utils files.
%_utils.o	:=	CCFLAGS-g	= $(CCFLAGS-O)
CFLAGS-O	= 	$(REORD_FLAG) -xO2
CFLAGS-tcov	=	-xa
CFLAGS-pg	=	-xpg
CFLAGS-lprof	=	-ql

_SB_CFLAG	=	test -d $(TT_DIR)/.sb && echo -xsb || true
SB_CFLAG	=	$(_SB_CFLAG:sh)

RELOC_glob	=
RELOC_SunOS_5	=	-K pic
RELOC_SunOS_4	=	-pic
RELOC_loc	=	$(RELOC_$(OS_NAME)_$(OS_VERSION))
RELOC		=	$(RELOC_glob$(RELOC_loc))$(RELOC_loc)

CFLAGS-lib-g	=	$(RELOC)
CFLAGS-lib-O	=	$(RELOC)
CFLAGS-lib-tcov =
CFLAGS-lib-lprof =	$(RELOC)
CFLAGS-lib-pg	=	
CFLAGS-lib	= 	$(CFLAGS-lib$(VARIANT))
CFLAGS-bin	=
CFLAGS-always	=	$(SB_CFLAG)
CCFLAGS-g	=	$(REORD_FLAG) -g0
# Normally, do not get symbols for _utils files.
%_utils.o	:=	CCFLAGS-g	= $(CCFLAGS-O)
CCFLAGS-O	= 	$(REORD_FLAG) -O2
CCFLAGS-tcov	=	-a
CCFLAGS-lprof	=	-ql
CCFLAGS-pg	=	-pg

_SB_CCFLAG	=	test -d $(TT_DIR)/.sb && echo -sb || true
SB_CCFLAG	=	$(_SB_CCFLAG:sh)


CCFLAGS-lib-g	=	-pic
CCFLAGS-lib-O	=	-pic
CCFLAGS-lib-tcov =
CCFLAGS-lib-lprof =	-pic
CCFLAGS-lib-pg	=
# When using C++ 4.0, tell the compiler we're not using exceptions. This
# generates better code and removes any dependency on libC.
CCFLAGS-lib-cafe=	-noex
CCFLAGS-lib	= 	$(CCFLAGS-lib$(VARIANT)) $(CCFLAGS-lib-$(CAFE_IN_USE))
CCFLAGS-bin	=
CCFLAGS-always	=	$(SB_CCFLAG)


CPPFLAGS-mt	=	-DMTSAFE
CPPFLAGS-bin	=	
CPPFLAGS-version=	-DTT_VERSION_STRING=\"$(TT_VERSION)\"
CPPFLAGS-always	=	$(CPPFLAGS-$(MT_INSTALLED)) -I$(TT_DIR)/lib
CPPFLAGS-isam	=	-I$(NETISAM)

LDFLAGS-g	=
# We used to specify -s for LDFLAGS-O which caused binaries and shared
# library to be stripped.  However, no other windows-consolidation
# component does this, so we remove it to be consistent. See bug 1106620.
LDFLAGS-O	=
LDFLAGS-tcov	=
LDFLAGS-pg	=

LDFLAGS-mt	=	-L/usr/lib/mt
LDFLAGS-lib_SunOS_4=	-assert pure-text
LDFLAGS-lib_SunOS_5=	-G -z defs -z text -h $@ -R $(LD_RUN_PATH)
LDFLAGS-lib_loc	=	$(LDFLAGS-lib_$(OS_NAME)_$(OS_VERSION))
LDFLAGS-lib_glob=
LDFLAGS-lib	=	$(LDFLAGS-lib_glob$(LDFLAGS-lib_loc))$(LDFLAGS-lib_loc)
LDFLAGS-bin	=	-L$(PROTO_DIR)/lib -L$(OPENWINHOME)/lib -R$(LD_RUN_PATH)
LDFLAGS-always  =	$(LDFLAGS-$(MT_INSTALLED))

LD_RUN_PATH-mt	=	/usr/lib/mt:/usr/openwin/lib
LD_RUN_PATH-	=	/usr/openwin/lib
LD_RUN_PATH	=	$(LD_RUN_PATH-$(MT_INSTALLED))

# If variable "SC_VERSION" is 3, define _CAFE_IN_USE as "cafe", which
# changes a few compiler options. If SC_VERSION is 2 or not set,
# leave _CAFE_IN_USE null.

_CAFE_IN_USE_	=
_CAFE_IN_USE_2	=
_CAFE_IN_USE_3	=	cafe
CAFE_IN_USE	=	$(_CAFE_IN_USE_$(SC_VERSION))

LDLIBS_glob	=
LDLIBS_SunOS_5	=	-lnsl -lsocket -lintl -ldl -lthread -lc -lw
LDLIBS_loc	=	$(LDLIBS_$(OS_NAME)_$(OS_VERSION))
LDLIBS		+=	$(LDLIBS_glob$(LDLIBS_loc))$(LDLIBS_loc)

RANLIB_glob	=	true
RANLIB_SunOS_4	=	ranlib
RANLIB_loc	=	$(RANLIB_$(OS_NAME)_$(OS_VERSION))
RANLIB		=	$(RANLIB_glob$(RANLIB_loc))$(RANLIB_loc)

VARIANT_DIRS	=	$(VARIANTS:X%=Derived%)
PROTO_TARGETS	=	$(VARIANTS:X%=Proto%)
BIN_TARGETS	=	$(VARIANTS:X%=Bin%)
INSTALL_TARGETS	=	$(VARIANTS:X%=Install%)
BB_TARGETS	=	$(VARIANTS:X%=Bb%)

PREPROCESS.c	= $(CC) -P $(CPPFLAGS) -o $@ $<
PREPROCESS.cc	= $(CCC) -P $(CPPFLAGS) -o $@ $<
LISTINCLUDES.cc	= $(CCC) -E -Qoption acpp -H $(CPPFLAGS) $< 1> /dev/null 2> $@ $<

CFRONT.cc	= $(CCC) -F $(CPPFLAGS) $< > $@
LISTASM.cc	= $(CCC) -O2 $(CCFLAGS) $(CPPFLAGS) -S -o $@ $<

# Commands for installing components

TTINSTALL	= $(TT_DIR)/ttinstall $(TT_VERSION)

#
# $(SCCS_HEADERS) can be empty. SunOS 4.x make sometimes gives empty
# sh macros garbage from the previous sh macro.  This takes away the
# garbage.
#
SUNOS_MAKE_BUG_1066151:sh = echo ' '

# get a list of all header files in the SCCS directory under here.

SCCS_HEADERS:sh = ls SCCS/s.*.h 2>/dev/null | sed 's:SCCS/s\.::'

# and make most real targets (not clean or sccs clean) dependent on all the headers,
# unless this is an sccs clean

HEADER_DEPENDENCIES += $(SCCS_HEADERS)

# More is added to CPPFLAGS, CFLAGS, CCFLAGS, and LDFLAGS with += in both
# directory-specific SunMakefile and in Makefile-tail.mk.
CPPFLAGS	=	$(CPPFLAGS-always)
CCFLAGS		=	$(CCFLAGS-always)
CFLAGS		=	$(CFLAGS-always)
LDFLAGS		=	$(LDFLAGS-always)

# Define all the "here" targets as double colon targets, so we don't
# have to put empty ones in all the individual makefiles, and make the ones
# that may do real work depend on $(HEADER_DEPENDENCIES).  (This means that
# doing a "local" non-recursive make right after sccs clean may not get
# all the header files, but that's pretty rare.)

$(BIN_TARGETS) $(INSTALL_TARGETS) $(PROTO_TARGETS) $(BB_TARGETS) \
:: $$(HEADER_DEPENDENCIES)

Proto-g		Bin-g		Install-g	Bb-g	:= 	VARIANT = -g
Proto-O		Bin-O		Install-O	Bb-O	:= 	VARIANT = -O
Proto-pg	Bin-pg		Install-pg	Bb-pg	:= 	VARIANT = -pg
Proto-tcov	Bin-tcov	Install-tcov	Bb-tcov	:= 	VARIANT = -tcov
Proto-lprof	Bin-lprof	Install-lprof	Bb-lprof:= 	VARIANT = -lprof


Clean::
		rm -fr $(VARIANT_DIRS)

Sccs-clean::
		sccs clean


