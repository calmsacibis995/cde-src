# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
SOURCES = \
	Do_Command \
	Set_Environment \
	Set_Geometry \
	Set_Iconified \
	Set_Locale \
	Set_Mapped \
	Modified \
	Get_Modified \
	Pause \
	Quit \
	Raise \
	Save \
	Saved \
	Set_Situation \
	Signal \
	Started \
	Status \
	Get_Status \
	Get_Sysinfo \
	Set_XInfo

#	Created
#	Moved
#	Raised
#	Iconified
#	Mapped
#	Opened

all: All.mml All.txt

All: Intro Synopses $(SOURCES)
	rm -f $@
	for file in Intro $(SOURCES); do \
		cat $$file >> $@; echo  >> $@; \
	done
	chmod -w $@

SYNOPSES = Synopses

include ../Make.rules
