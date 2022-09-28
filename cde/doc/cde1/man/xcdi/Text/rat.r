.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.I
This section offers general rationale for various decisions about \*(Zx
standardisation.
It is not intended to remain in this specification
when it is published by X/Open.
.R
.H 3 "Components Not Standardised"
The following components, not addressed elsewhere in the proposal, are
not planned for standardisation, for the reasons stated:
.VL 13 5
.LI "Other Printing Interfaces"
.br
The X Print Server was omitted from the CDE 1.0 project.
Applications can use existing XPG interfaces for printing.
.mc 6
.LI "I18N"
.mc
There are no unique internationalisation interfaces in CDE 1.0.
Applications can use existing XPG and Motif interfaces for
internationalisation.
.LI "Login"
As with the POSIX standards, the process of logging in and verifying
the user's identity is considered a system administration and
security-related function, outside the scope of the current \*(Zx
standardisation effort in X/Open.
This includes the
.Cm dtlogin ,
.Cm dtgreet ,
.Cm dthello ,
.Cm dtsearchpath ,
and
.Cm dtappgather
utilities.
.LI "Clock"
This is a trivial component, not required for portable applications,
and widely available from various sources.
.LE
