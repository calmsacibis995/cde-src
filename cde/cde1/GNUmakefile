#
#+SNOTICE
#
#	$Revision: 1.1 $
#
#	RESTRICTED CONFIDENTIAL INFORMATION:
#	
#	The information in this document is subject to special
#	restrictions in a confidential disclosure agreement between
#	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
#	document outside HP, IBM, Sun, USL, SCO, or Univel without
#	Sun's specific written approval.  This document and all copies
#	and derivative works thereof must be returned or destroyed at
#	Sun's request.
#
#	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
#
#+ENOTICE
#
# @(#)GNUmakefile	1.3 16 Sep 1993
#
ifndef CDESRC
CDESRC		=	../..
endif

ifndef GNUMASTER_DIR
GNUMASTER_DIR	=	$(CDESRC)/cde1/dtcompat
endif

SUBDIRS		=	dtcompat dtmail dtcm

all clean depend includes Makefile Makefiles::
	@for dir in $(SUBDIRS) ; do		\
		if [ -d $$dir ] ; then		\
			cd $$dir ;		\
			$(MAKE) $@ ;		\
			cd .. ;			\
		fi				\
	done
