.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Miscellaneous Desktop Services"
.xR1 dtsvcchap
.nr Ej 1
.H 2 "Introduction"
.mc 2
This section defines the functions, macros and
external variables that provide miscellaneous \*(Zx services
to support application portability at the C-language source level.
.br
.mc
.H 2 "Functions"
.xR2 miscfuncs
The following functions initialise the Desktop Services library.
.so ../fx/DtInitialize.mm
.ds XC Headers
.SK
.H 2 "Headers"
This section describes the contents of the header used
by miscellaneous \*(Zx message service functions,
macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in 
.cX miscfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/Dt.mm
.eF e
