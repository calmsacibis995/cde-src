# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
SUBDIRS = Desktop Media

all: TICCCM.txt TICCCM.mml TICCCM.ps

TICCCM: Intro MsgList $(SUBDIRS:%=%/All)
	rm -f $@
	cat an.ticccm >> $@
	soroff Intro >> $@
	echo  >> $@
	for dir in $(SUBDIRS); do \
		(cd $$dir; make All); \
		(cd $$dir; soroff All) >> $@; \
	done

include Make.rules

MsgList: $(SUBDIRS:%=%/Synopses)
	rm -f $@
	first=1; \
	for dir in $(SUBDIRS); do \
		if [ $$first ]; then first=; else echo "	" >>$@; fi; \
		(cd $$dir; make Synopses); \
		cat $$dir/Synopses >> $@; \
	done

%/Synopses %/All:
	(cd $*; make $(@F))

clean::
	for dir in $(SUBDIRS); do \
		(cd $$dir; make $@); \
	done
	rm -f MsgList TICCCM TICCCM.txt TICCCM.mml TICCCM.ps
