# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
SOURCES = \
	Deposit \
	Display \
	Edit \
	Mail \
	Print \
	Translate

#	Abstract \
#	EditSelection \
#	Interpret \

all: All.mml All.txt

All: Intro Synopses $(SOURCES)
	rm -f $@
	for file in Intro $(SOURCES); do \
		cat $$file >> $@; echo  >> $@; \
	done
	chmod -w $@

SYNOPSES = Synopses

include ../Make.rules
