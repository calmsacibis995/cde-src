# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.7 93/09/07

TT_DIR	= ../..

all:	ttsession

include $(TT_DIR)/simple-macros.mk

TTSESSION_OBJECTS =\
mp_server.o

ttsession:	$(TTSESSION_OBJECTS) $(LDSDEP)
	$(CCC) $(CCCFLAGS) $(LDSCCCFLAGS) -o ttsession \
	$(TTSESSION_OBJECTS) $(LDSCCCLIBS)


mp_server.o:	mp_server.C
	$(CCC) $(CCCFLAGS) -I$(TT_DIR)/slib $(VEROPT) \
	-o mp_server.o -c mp_server.C

clean:
	$(RM) $(TTSESSION_OBJECTS) ttsession

install: ttsession
	@test -d $(INSTALLBIN) || mkdir -p $(INSTALLBIN)
	$(RM) $(INSTALLBIN)/ttsession; cp ttsession $(INSTALLBIN); chmod 555 $(INSTALLBIN)/ttsession
