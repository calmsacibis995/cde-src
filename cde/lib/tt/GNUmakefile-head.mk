# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)GNUmakefile-head.mk	1.4 93/09/07
# Copyright (c) 1992 by Sun Microsystems, Inc.

all::

NETISAM		:=	$(TT_DIR)/mini_isam
# the first part of the version string is the ToolTalk version number
# which only changes when ToolTalk makes functional changes.  The 
# second part (after the underscore) is the OpenWindows build identifier.
# Increment the second part (only) for every OpenWindows build.
TT_VERSION	:=	1.2_CDE_DEV_CONF




# Possible variants:
# debug (-g)		- only for development
# optimized (-O)	- build product this way
# tcov (-tcov)		- for test coverage and performance tuning
# gprof (-pg)		- for performance tuning
# src (-source)		- pseudovariant for storing preprocessor output
VARIANTS	=	X-g X-O X-tcov X-pg

OS		:=	$(shell uname -s)

# When building in the CDE tree, TOP_INSTALL_DIR will be set; to make sure
# relative paths are handled, ignore what it's set to and use
# $(TT_DIR)/install.  When building outside, install in a OS-specific
# directory, to allow multiple platforms to be built in one tree.
ifeq (,$(TOP_INSTALL_DIR))
  INSTALL_DIR	= 	$(TT_DIR)/install-$(OS)
else
  INSTALL_DIR	=	$(TT_DIR)/install
endif

# Set expected parameters according to current OS.  This
# allows the programmer to just run gnumake to get the
# "typical" configuration for the platform, while permitting
# the imake-generated Makefile to override the parameters
# with the proper ones for the desired configuration.

# Parameters supported:
# VARIANT={-g,-O} 	for debug/optimized.  Future will include -tcov and
#			-gprof for measurements.
# In order to fit in with current CDE imake-based build, if VARIANT
# is not set but OPTCDBUGFLAGS is, we guess the variant based on 
# its value.
# BUILDSHAREDLIBS={Y,N}	should shared libs be built?
# BUILDARCHIVELIBS={Y,N}should archive libs be used? 
#			Specifying both BUILDSHAREDLIBS and
#			BUILDARCHIVELIBS as N will not get you very far.
# LINKSHAREDLIBS={Y,N}	should binaries be linked against shared libs?
# LINKARCHIVELIBS={Y,N} should binaries be linked against archive libs?
# 			Specifying BUILDSHAREDLIBS=N and LINKSHAREDLIBS=Y
#			or BUILDARCHIVELIBS=N and LINKARCHIVELIBS=N or
#			both LINKARCHIVELIBS and LINKSHAREDLIBS=N is no good.
# XLIB			Path to libX11
# XTOOLLIB		Path to libXt
# X11INCLUDE_DIRS	directories in which to find Xlib and Xt includes
# RPCINCLUDE_DIRS	directories in which to find patched rpc includes


ifeq (,$(VARIANT))
# VARIANT not set, apply default based on OPTCDBUGFLAGS
  ifneq (,$(findstring -g,$(OPTCDBUGFLAGS)))
    VARIANT := -g
  else
    ifneq (,$(findstring -O,$(OPTCDBUGFLAGS)))
      VARIANT := -O
    else
      # Default variant, if building outside of CDE imake scheme, is debug.
      VARIANT := -g
    endif
  endif
endif

# Compute values for X11INCLUDES and RPCINCLUDES from X11INCLUDE_DIRS
# and RPCINCLUDE_DIRS if set
# Pick out the absolute paths and just put -I on them
# Pick out the relative paths and put -I$(TT_DIR) on them
# Note that theis filter/filter-out technique may reorder the
# list.  This turns out not to hurt in the cases we care about
# (generally all the paths are either relative or absolute, and
# the order is unimportant anyway.)

ifneq (,$(X11INCLUDE_DIRS))
  X11INCLUDES = \
    $(addprefix -I,$(filter /%,$(X11INCLUDE_DIRS))) \
    $(addprefix -I$(TT_DIR)/,$(filter-out /%,$(X11INCLUDE_DIRS)))
endif
ifneq (,$(RPCINCLUDE_DIRS))
  # Pick out the absolute paths and just put -I on them
  # Pick out the relative paths and put -I$(TT_DIR) on them

  RPCINCLUDES = \
    $(addprefix -I,$(filter /%,$(RPCINCLUDE_DIRS))) \
    $(addprefix -I$(TT_DIR)/,$(filter-out /%,$(RPCINCLUDE_DIRS)))
endif

# Similarly, ensure that any relative paths in XLIB and XTOOLLIB
# are relative to $(TT_DIR)

RELATIVE_XLIB = \
  $(filter -%,$(XLIB)) \
  $(filter /%,$(XLIB)) \
    $(addprefix $(TT_DIR)/,$(filter-out -%,$(filter-out /%,$(XLIB))))
RELATIVE_XTOOLLIB = \
  $(filter -%,$(XTOOLLIB)) \
  $(filter /%,$(XTOOLLIB)) \
    $(addprefix $(TT_DIR)/,$(filter-out -%,$(filter-out /%,$(XTOOLLIB))))

# Set default parameters according to current OS.
# (The real builds pass in the real values; this is only
# for development convenience, allowing us to just type "gnumake"
# w/o needing a whole Imake/config setup.

# For SunOS, the default is to build (only) shared libs, and to find
# X stuff under $(OPENWINHOME).

ifeq ($(OS),SunOS)
  ifeq ($(OPENWINHOME),)
    OPENWINHOME := /usr/openwin
  endif
  ifeq ($(BUILDSHAREDLIBS),)
    BUILDSHAREDLIBS := Y
  endif
  ifeq ($(BUILDARCHIVELIBS),)
    BUILDARCHIVELIBS := N
  endif
  ifeq ($(LINKSHAREDLIBS),)
    LINKSHAREDLIBS := Y
  endif
  ifeq ($(LINKARCHIVELIBS),)
    LINKARCHIVELIBS := N
  endif
  ifeq ($(XLIB),)
    XLIB := -L$(OPENWINHOME)/lib -lX11
  endif
  ifeq ($(XTOOLLIB),)
    XTOOLLIB := -L$(OPENWINHOME)/lib -lXt
  endif
  ifeq ($(X11INCLUDES),)
    X11INCLUDES = -I$(OPENWINHOME)/include
  endif
endif

# For HP-UX, the default is to build (only) archive libs, and to find
# X stuff under /usr/include/X11R5 and /usr/lib/X11R5.

ifeq ($(OS),HP-UX)
  ifeq ($(BUILDSHAREDLIBS),)
    BUILDSHAREDLIBS := N
  endif
  ifeq ($(BUILDARCHIVELIBS),)
    BUILDARCHIVELIBS := Y
  endif
  ifeq ($(LINKSHAREDLIBS),)
    LINKSHAREDLIBS := N
  endif
  ifeq ($(LINKARCHIVELIBS),)
    LINKARCHIVELIBS := Y
  endif
  ifeq ($(XLIB),)
    XLIB := -L/usr/lib/X11R5 -lX11
  endif
  ifeq ($(XTOOLLIB),)
    XTOOLLIB := -L/usr/lib/X11R5 -lXt
  endif
  ifeq ($(X11INCLUDES),)
    X11INCLUDES = -I/usr/include/X11R5
  endif
endif

# For AIX, the default is to build (only) archive libs, and to find
# X stuff in the default spots chosen by the compiler

ifeq ($(OS),AIX)
  ifeq ($(BUILDSHAREDLIBS),)
    BUILDSHAREDLIBS := N
  endif
  ifeq ($(BUILDARCHIVELIBS),)
    BUILDARCHIVELIBS := Y
  endif
  ifeq ($(LINKSHAREDLIBS),)
    LINKSHAREDLIBS := N
  endif
  ifeq ($(LINKARCHIVELIBS),)
    LINKARCHIVELIBS := Y
  endif
  ifeq ($(XLIB),)
    XLIB := -lX11
  endif
  ifeq ($(XTOOLLIB),)
    XTOOLLIB := -lXt
  endif
  ifeq ($(X11INCLUDES),)
    X11INCLUDES = 
  endif
endif

# For everybody else, build debug, archive libs, and
# hope to find X under /usr/lib/X11 and /usr/include/X11

ifeq ($(BUILDSHAREDLIBS),)
  BUILDSHAREDLIBS := N
endif
ifeq ($(BUILDARCHIVELIBS),)
  BUILDARCHIVELIBS := Y
endif
ifeq ($(LINKSHAREDLIBS),)
  LINKSHAREDLIBS := N
endif
ifeq ($(LINKARCHIVELIBS),)
  LINKARCHIVELIBS := Y
endif
ifeq ($(XLIB),)
  XLIB := -L/usr/lib/X11 -lX11
endif
ifeq ($(XTOOLLIB),)
  XTOOLLIB := -L/usr/lib/X11 -lXt
endif
ifeq ($(X11INCLUDES),)
  X11INCLUDES = 
endif

# "Usual" definitions of utility macros.  May be overridden in the
# OS-specific parts to follow.

# C compiler
CC		:=	cc
# C++ compiler
CCC		:=	CC
# Post-processor for ar (real BSD-ish systems override with ranlib)
RANLIB		:=	true

# Default main controlling C and C++ options, indexed by variant.
# Note these might be overridden in the platform-dependent section below.

CFLAGS-g	=	-g
CFLAGS-O	= 	-O
CFLAGS-tcov	=	-a
CFLAGS-pg	=	-pg
CCFLAGS-g	=	-g
CCFLAGS-O	= 	-O
CCFLAGS-tcov	=	-a
CCFLAGS-pg	=	-pg


# Occasional overrides in individual Makefiles add these on when needed

CPPFLAGS-version=	-DTT_VERSION_STRING=\"$(TT_VERSION)\"
CPPFLAGS-isam	=	-I$(NETISAM)

# CPP flags that *always* go on can be added to this macro.  Note that
# such things are basically a function of the ToolTalk tree shape, and
# should thus never vary per-platform or per-variant.

CPPFLAGS-always	=	-I$(TT_DIR)/lib $(RPCINCLUDES)


# LD flags that always go on binary links can be added to this macro.
# See note above.

ifeq ($(LINKSHAREDLIBS),Y)
  LDFLAGS-bin	=	-L$(TT_DIR)/lib/$(PICOBJDIR)
else
  LDFLAGS-bin	=	-L$(TT_DIR)/lib/$(PDCOBJDIR)
endif
LDFLAGS-bin	+=	-L$(TT_DIR)/slib/$(PDCOBJDIR) \
			-L$(TT_DIR)/mini_isam/$(PDCOBJDIR)

# Full compilation commands.  Note these are recursively expanded variables
# since individual GNUmakefiles may override or add to CFLAGS, CPPFLAGS, etc.

PREPROCESS.c	= $(CC) -P $(CPPFLAGS) -o $@ $<
PREPROCESS.cc	= $(CCC) -P $(CPPFLAGS) -o $@ $<
LISTINCLUDES.cc	= $(CCC) -E -Qoption acpp -H $(CPPFLAGS) $< 1> /dev/null 2> $@ $<

CFRONT.cc	= $(CCC) -F $(CPPFLAGS) $< > $@
LISTASM.cc	= $(CCC) -O2 $(CCFLAGS) $(CPPFLAGS) -S -o $@ $<

# Commands for installing components

TTINSTALL	= $(TT_DIR)/ttinstall $(TT_VERSION)

# SunOS dependent -- replicate this section for each OS

ifeq ($(OS),SunOS)
#
#	OS:			SunOS
#	REL:			411, 51, 52, ...
#	REL.major:		4, 5
#	CPU:			sparc, i386
# On some platforms, platform depends only on OS, not both OS and CPU
#       PLATFORM:		SunOS-sparc, SunOS-i386
#
  REL		:=	$(shell uname -r | sed 's/\.//g')
  REL.major	:=	$(shell uname -r | sed 's/\..*//g')
  CPU		:=	$(shell uname -p)
  PLATFORM	:=	$(OS)-$(CPU)

# SunOS 4.x used a different C compiler name	
  ifeq ($(REL.major),4)
    CC		:=	acc
  endif
  CFLAGS-O	:=	-xO2
  CCFLAGS-g	:=	-g0
  CCFLAGS-O	:=	-O2

# Building of shared libraries is so nonstandard we don't even have a common
# case.
  ifeq ($(REL.major),4)
    LDFLAGS-lib	:= -assert pure-text
    RANLIB	:= ranlib
    CFLAG-pic	:= -PIC
    CCFLAG-pic	:= -PIC
    SHLIBSUFFIX	:= so.1.0
  else
    LDFLAGS-lib	= -G -z defs -z text -h $(@F) 
    LDLIBS	:= -lnsl -lsocket -lintl -ldl -lc
    CFLAG-pic	:= -K PIC
    CCFLAG-pic	:= -PIC
    SHLIBSUFFIX := so.1

    # Some magic for making objects smaller on Sun by stripping out mcs info

    SHARP	:= $(shell /bin/echo \\043\\c)
    MCS_COMMAND = mcs -d -a "@($(SHARP))ToolTalk $(TT_VERSION)" $(@)

  endif	
  ifeq (YY,$(BUILDSHAREDLIBS)$(LINKARCHIVELIBS))
    SETLINKMODE := -B static
    RESETLINKMODE := -B dynamic
  else
#   important to use -R instead of LD_RUN_PATH to avoid inclusion of
#   compiler-specific /opt/SUNWspro/lib directory.
    SETLINKMODE := -R/usr/dt/lib
  endif
endif
# end of SunOS


# HP/UX dependent -- replicate this section for each OS

ifeq ($(OS),HP-UX)
#
#	OS:			HP-UX
#	REL:			A_09_01
#	REL.major:		A_09_01
#	CPU:			9000 
# On some platforms, platform depends only on OS, not both OS and CPU
#       PLATFORM:		SunOS-sparc, SunOS-i386
#
  REL		:=	$(shell uname -r | sed 's/\./_/')
  REL.major	:=	$(REL)
  CPU		:=	$(shell uname -m | sed 's:/.*::')
  PLATFORM	:=	$(OS)-$(CPU)

  CPPFLAGS-always += -D_HPUX_SOURCE
  CFLAGS-g = -Aa -g
  CFLAGS-O = -Aa -O

  LDLIBS	= $(RELATIVE_XTOOLLIB) $(RELATIVE_XLIB)

# Building of shared libraries is so nonstandard we don't even have a common
# case. 

  LDFLAGS-lib	:= -b
  CFLAG-pic	:= +z
  CCFLAG-pic	:= +z
  SHLIBSUFFIX := sl
# HP wants even system libs linked statically if LINKARCHIVELIBS is on.
  ifeq (YY,$(BUILDSHAREDLIBS)$(LINKARCHIVELIBS))
    SETLINKMODE := -Wl,-a,archive
    RESETLINKMODE :=
  endif
endif
# end of HP-UX

# AIX dependent -- replicate this section for each OS

ifeq ($(OS),AIX)
#
#	OS:			AIX
#	REL:			32
#	REL.major:		3
#	CPU:			9000 
# On some platforms, platform depends only on OS, not both OS and CPU
#       PLATFORM:		SunOS-sparc, SunOS-i386
#
  REL.major	:=	$(shell uname -v)
  REL.minor	:= 	$(shell uname -r)
  REL		:=	$REL.major)$(REL.minor)
# Does not seem to be a way to query the sys type on AIX!
  CPU		:=	RS6000
  PLATFORM	:=	$(OS)-$(CPU)

  CPPFLAGS-always += -D_ALL_SOURCE
  CFLAGS-g = -g
  CFLAGS-O = -O
  CCC		:= xlC

  LDLIBS	= $(RELATIVE_XTOOLLIB) $(RELATIVE_XLIB)

# Building of shared libraries is so nonstandard we don't even have a common
# case.  And I don't yet even know how on AIX.  Or even if they have them.

endif
# end of AIX

# Now put together pieces based on things set in OS specific parts.

PICOBJDIR	:=	$(PLATFORM)-pic$(VARIANT)
PDCOBJDIR	:=	$(PLATFORM)$(VARIANT)

# get a list of all header files in the SCCS directory under here.

SCCS_HEADERS	:= 	$(shell ls SCCS/s.*.h 2>/dev/null | sed 's:SCCS/s\.::')

# and make most real targets (not clean or sccs clean) dependent on all the headers,
# unless this is an sccs clean

HEADER_DEPENDENCIES += $(SCCS_HEADERS)

# More is added to CPPFLAGS, CFLAGS, CCFLAGS, and LDFLAGS with += in both
# directory-specific GNUMakefile and in Makefile-tail.mk.
CPPFLAGS	=	$(CPPFLAGS-always)
CCFLAGS		=	$(CCFLAGS$(VARIANT))
CFLAGS		=	$(CFLAGS$(VARIANT))
LDFLAGS		=	$(LDFLAGS-always)

# tcov and gprof use strictly static libs.

ifneq (,$(findstring $(VARIANT),-tcov-pg))
LIBDEPENDENCY	:= $(TT_DIR)/lib/$(PDCOBJDIR)/libtt.a
else
  ifeq ($(LINKSHAREDLIBS),N)
    LIBDEPENDENCY := $(TT_DIR)/lib/$(PDCOBJDIR)/libtt.a
  else
    LIBDEPENDENCY := $(TT_DIR)/lib/$(PICOBJDIR)/libtt.$(SHLIBSUFFIX)
  endif
endif

SLIBDEPENDENCY	:=	$(TT_DIR)/slib/$(PDCOBJDIR)/libstt.a

# Rules for making objects from sources.   These come in -head.mk
# since individual directories may override (e.g. lib.)  

COMPILE_PIC.c	= $(CC) $(CFLAGS) $(CFLAG-pic) $(CPPFLAGS) -c
COMPILE_PDC.c	= $(CC)	$(CFLAGS) $(CPPFLAGS) -c
COMPILE_PIC.cc	= $(CCC) $(CCFLAGS) $(CCFLAG-pic) $(CPPFLAGS) -c
COMPILE_PDC.cc	= $(CCC) $(CCFLAGS) $(CPPFLAGS) -c
LINK.cc		= $(CCC) $(CCFLAGS) $(LDFLAGS)

# Objects destined for libtt.a should have MCS comments stripped (on Sun).
# We actually strip them everywhere, which is strictly unnecessary
# since we will strip the binaries before shipping, but it's
# too hard to figure out how to get gnumake to just do it for lib members
# right now.

$(PDCOBJDIR)/%.o: %.C
	rm -f $@
	$(COMPILE_PDC.cc) -o $@ $<
	@$(MCS_COMMAND)

$(PDCOBJDIR)/%.o: %.c
	rm -f $@
	$(COMPILE_PDC.c) -o $@ $<
	@$(MCS_COMMAND)

$(PICOBJDIR)/%.o: %.C
	rm -f $@
	$(COMPILE_PIC.cc) -o $@ $<
	@$(MCS_COMMAND)

$(PICOBJDIR)/%.o: %.c
	rm -f $@
	$(COMPILE_PIC.c) -o $@ $<
	@$(MCS_COMMAND)

# This arcane ritual produces a magic object file which, when included
# in a link step, will make the AIX binder link in libC statically
# so we don't have to ship it.
ifeq ($(OS),AIX)
$(PDCOBJDIR)/nonsharelibC.o:
	rm -f $(PDCOBJDIR)/nonsharelibC.o shr.o ;\
	ar x /usr/lpp/xlC/lib/libC.a shr.o; \
	ld -r shr.o -o $(PDCOBJDIR)/nonsharelibC.o -bnso ;\
	rm -f shr.o
endif

# The following dummy rule keeps gnumake from chaining throug all the 
# default suffix rules trying to find dependencies of GNUmakefile and
# the .mk makefile includes.

%.mk :

GN%makefile :




