# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.7 93/09/07

TT_DIR	= ../..

all:	tttar

include $(TT_DIR)/simple-macros.mk

TTTAR_OBJECTS =\
archiver.o \
tttar.o \
tttar_api.o \
tttar_file_utils.o \
tttar_spec.o \
tttar_string_map.o \
tttar_utils.o

tttar:	$(TTTAR_OBJECTS) $(LDDEP)
	$(CCC) $(CCCFLAGS) $(LDCCCFLAGS) -o tttar \
	$(TTTAR_OBJECTS) $(LDCCCLIBS)

archiver.o:	archiver.C
	$(CCC) $(LIBCCCFLAGS) $(VEROPT) -o archiver.o -c archiver.C

tttar.o:	tttar.C
	$(CCC) $(LIBCCCFLAGS) -o tttar.o -c tttar.C

tttar_api.o:	tttar_api.C
	$(CCC) $(LIBCCCFLAGS) -o tttar_api.o -c tttar_api.C

tttar_file_utils.o:	tttar_file_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tttar_file_utils.o -c tttar_file_utils.C

tttar_spec.o:	tttar_spec.C
	$(CCC) $(LIBCCCFLAGS) -o tttar_spec.o -c tttar_spec.C

tttar_string_map.o:	tttar_string_map.C
	$(CCC) $(LIBCCCFLAGS) -o tttar_string_map.o -c tttar_string_map.C

tttar_utils.o:	tttar_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tttar_utils.o -c tttar_utils.C

clean:
	$(RM) $(TTTAR_OBJECTS) tttar

install: tttar
	@test -d $(INSTALLBIN) || mkdir -p $(INSTALLBIN)
	$(RM) $(INSTALLBIN)/tttar; cp tttar $(INSTALLBIN); chmod 555 $(INSTALLBIN)/tttar
