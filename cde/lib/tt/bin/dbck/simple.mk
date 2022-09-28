# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.7 93/09/07

TT_DIR	= ../..

all:	ttdbck

include $(TT_DIR)/simple-macros.mk

TTDBCK_OBJECTS=\
binkey.o \
binkey_utils.o \
common.o \
options.o \
options_tt.o \
prop.o \
prop_utils.o \
spec.o \
spec_repair.o \
spec_utils.o \
ttdbck.o

ttdbck:	$(TTDBCK_OBJECTS) $(LDSDEP)
	$(CCC) $(CCCFLAGS) $(LDSCCCFLAGS) -L$(ISAMLIBDIR) -o ttdbck \
	$(TTDBCK_OBJECTS) $(LDSCCCLIBS) -l$(ISAMLIB)

binkey.o:	binkey.C
	$(CCC) $(LIBCCCFLAGS) -o binkey.o -c binkey.C

binkey_utils.o:	binkey_utils.C
	$(CCC) $(LIBCCCFLAGS) -o binkey_utils.o -c binkey_utils.C

common.o:	common.C
	$(CCC) $(LIBCCCFLAGS) -o common.o -c common.C

options.o:	options.C
	$(CCC) $(LIBCCCFLAGS) $(VEROPT) -o options.o -c options.C

options_tt.o:	options_tt.C
	$(CCC) $(LIBCCCFLAGS) -o options_tt.o -c options_tt.C

prop.o:	prop.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o prop.o -c prop.C

prop_utils.o:	prop_utils.C
	$(CCC) $(LIBCCCFLAGS) -o prop_utils.o -c prop_utils.C

spec.o:	spec.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o spec.o -c spec.C

spec_repair.o:	spec_repair.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o spec_repair.o -c spec_repair.C

spec_utils.o:	spec_utils.C
	$(CCC) $(LIBCCCFLAGS) -o spec_utils.o -c spec_utils.C

ttdbck.o:	ttdbck.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o ttdbck.o -c ttdbck.C

clean:
	$(RM) $(TTDBCK_OBJECTS) ttdbck

install: ttdbck
	@test -d $(INSTALLBIN) || mkdir -p $(INSTALLBIN)
	$(RM) $(INSTALLBIN)/ttdbck; cp ttdbck $(INSTALLBIN); chmod 555 $(INSTALLBIN)/ttdbck

