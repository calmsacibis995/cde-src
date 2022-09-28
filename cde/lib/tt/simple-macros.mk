# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple-macros.mk	1.6 93/09/07

# Simple makefile definitions. Edit these for local quirks.
# You may still have to edit the individual simple.mk files as well

# Version string for this version.  Reported by -v options on binaries.
TT_VERSION=1.1.1

# Compiler names

# C compiler
#CC	= cc

# C++ compiler
CCC	= xlC

# Utility commands

# Command to create a new archive library
AR	= ar r

# Possible command needed after $(AR)
RANLIB	= true

# Command to remove files
RM	= rm -f

# yacc
YACC= yacc

# lex
LEX= lex

# Change these for major compile mode changes, for example to -g.
# Usual C flags
# for Sun
#CCMODE	= -O
# for AIX
#CCMODE = -Aa -D_ALL_SOURCE -g
# for HP/UX
CCMODE = -Aa -D_HPUX_SOURCE -O
# Usual C++ flags
# for Sun
CCCMODE = -O2
# for HPUX
#CCCMODE = -D_HPUX_SOURCE -O
# for AIX
CCCMODE = -D_ALL_SOURCE -g
# Paths to include files

# ToolTalk internal header file dir
TTINCL  = $(TT_DIR)/lib
# ISAM header file dir
ISAMINCL= $(TT_DIR)/mini_isam
# ISAM library dir
ISAMLIBDIR= $(TT_DIR)/mini_isam
# ISAM library name
ISAMLIB= isam
# X11 header file dir
XWININCL= /usr/include/X11R5
# X11 library dir
XWINLIBDIR= /usr/lib/X11R5
# X11 library name
XWINLIB= X11
# C++ versions of system includes
CCCINCL =

# This defines the C/C++ flags used for compiling normal binary objects.
CCFLAGS = $(CCMODE) -I$(TTINCL) -I$(ISAMINCL) -I$(XWININCL)
CCCFLAGS = $(CCCMODE) -I$(TTINCL) -I$(ISAMINCL) -I$(XWININCL)

# This defines the C/C++ flags needed for compiling libtt modules to the
# following macros.  For example, you might need a special switch to enable
# making a shared library.

LIBCCFLAGS= $(CCMODE) -I$(TTINCL) -I$(ISAMINCL) -I$(XWININCL)
LIBCCCFLAGS= $(CCCMODE) -I$(TTINCL) -I$(ISAMINCL) -I$(XWININCL)

# This defines the C++ flags needed to link most binaries

LDCCCFLAGS= $(CCCMODE) -L$(TT_DIR)/lib -L$(XWINLIBDIR) -L/usr/local/lib

# This defines the libraries needed to link most binaries

LDCCCLIBS= -ltt -l$(XWINLIB)

# This defines the C++ flags needed to link the binaries that use libstt.a
# (ttsession and tt_type_comp.)

LDSCCCFLAGS= $(LDCCCFLAGS) -L$(TT_DIR)/slib

# This defines the libraries needed to link the binaries that use libstt.a

LDSCCCLIBS= -lstt $(LDCCCLIBS)

# This defines the dependency to put on the binaries that use libstt.a
# (ttsession and tt_type_comp) so they get relinked when the library
# changes.

LDSDEP= $(TT_DIR)/lib/libtt.a $(TT_DIR)/slib/libstt.a

# This defines the dependency to put on most binaries
LDDEP= $(TT_DIR)/lib/libtt.a

# cpp Option string for defining TT_OPTION_STRING to be the current version
# only the copyright.o file in libtt and the main program in each binary
# need this.

VEROPT	= -DTT_VERSION_STRING=\"$(TT_VERSION)\"

# Where to put the final product.

INSTALLDIR = $(TT_DIR)/install
INSTALLBIN = $(INSTALLDIR)/bin
INSTALLINCL= $(INSTALLDIR)/include/desktop
INSTALLLIB= $(INSTALLDIR)/lib

# At ToolTalk GHQ we depend on the SunPro make dependency tracking,
# enabled by the special .KEEP_STATE target.
# which means that we don't keep track of includes in our Makefiles.
# If you have such a facility, turn it on.  If you don't, you'll
# have to do a make clean after changing any .h files...
.KEEP_STATE:


