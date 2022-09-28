# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.7 93/09/07

TT_DIR	= ../..

all:	ttmv ttrm ttcp ttrmdir

include $(TT_DIR)/simple-macros.mk

TTMV_OBJECTS= ttmv.o mover.o
TTRM_OBJECTS= ttrm.o remover.o
TTCP_OBJECTS= ttcp.o copier.o

ttmv:	$(TTMV_OBJECTS) $(LDDEP)
	$(CCC) $(CCCFLAGS) $(LDCCCFLAGS) -o ttmv \
	$(TTMV_OBJECTS) $(LDCCCLIBS)

ttrm:	$(TTRM_OBJECTS) $(LDDEP)
	$(CCC) $(CCCFLAGS) $(LDCCCFLAGS) -o ttrm \
	$(TTRM_OBJECTS) $(LDCCCLIBS)

ttcp:	$(TTCP_OBJECTS) $(LDDEP)
	$(CCC) $(CCCFLAGS) $(LDCCCFLAGS) -o ttcp \
	$(TTCP_OBJECTS) $(LDCCCLIBS)

ttrmdir: ttrm
	$(RM) ttrmdir; ln ttrm ttrmdir

copier.o:	copier.C
	$(CCC) $(LIBCCCFLAGS) $(VEROPT) -o copier.o -c copier.C

mover.o:	mover.C
	$(CCC) $(LIBCCCFLAGS) $(VEROPT) -o mover.o -c mover.C

remover.o:	remover.C
	$(CCC) $(LIBCCCFLAGS) $(VEROPT) -o remover.o -c remover.C

ttcp.o:	ttcp.C
	$(CCC) $(LIBCCCFLAGS) -o ttcp.o -c ttcp.C

ttmv.o:	ttmv.C
	$(CCC) $(LIBCCCFLAGS) -o ttmv.o -c ttmv.C

ttrm.o:	ttrm.C
	$(CCC) $(LIBCCCFLAGS) -o ttrm.o -c ttrm.C

clean:
	$(RM) $(TTMV_OBJECTS) ttmv $(TTRM_OBJECTS) ttrm $(TTCP_OBJECTS) ttcp

install: all
	@test -d $(INSTALLBIN) || mkdir -p $(INSTALLBIN)
	$(RM) $(INSTALLBIN)/ttcp; cp ttcp $(INSTALLBIN); chmod 555 $(INSTALLBIN)/ttcp
	$(RM) $(INSTALLBIN)/ttmv; cp ttmv $(INSTALLBIN); chmod 555 $(INSTALLBIN)/ttmv
	$(RM) $(INSTALLBIN)/ttrm; cp ttrm $(INSTALLBIN); chmod 555 $(INSTALLBIN)/ttrm
	$(RM) $(INSTALLBIN)/ttrmdir; ln $(INSTALLBIN)/ttrm $(INSTALLBIN)/ttrmdir
