# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
#
# Boot.mk 1.2	30 Jul 1993
#
DEFINES		= -DIMAKEDIR=$(IMAKEDIR)
DEPENDENCIES	=  IMakeFile $(IMAKEDIR)/Imake.tmpl $(IMAKEDIR)/Imake.rules
MakeFile: $(DEPENDENCIES)
	unset OPENWINHOME; imake -I$(IMAKEDIR) -f IMakeFile -s \$@ $(DEFINES)
GNUMakeFile: $(DEPENDENCIES)
	unset OPENWINHOME; imake -I$(IMAKEDIR) -f IMakeFile -s \$@ $(DEFINES) -Dgnu
