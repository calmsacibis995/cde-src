.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "X Windows and Motif"
.xR1 xchap
.nr Ej 1
.mc 3
This chapter describes the dependencies on X Windows and Motif
as underlying implementations and defines additional widget
interfaces that promote portability of historical applications
into the \*(Zx environment.
.br
.mc
.H 2 "X Protocol"
The interfaces in this document require an underlying implementation
that support the protocols defined in the \*(Zp.
.H 2 "Xlib Library"
The interfaces in this document require an underlying implementation
that support the interfaces defined in the \*(Zl.
.mc 4
The version of that document
based on X Windows Version 11, Release 5 is required for \*(Zx.
.mc
.iN
The specific release is mentioned here because \*(XX is currently
planning to update its X standards to X11R5.
There is no release dependency on the previously mentioned X Protocol.
The Motif in a following section is available from X/Open in
only one version\*(EM1.2\*(EMso it does not need to be cited here.
.sA
.H 2 "Xt Intrinsics"
The interfaces in this document require an underlying implementation
that support the interfaces defined in the \*(Zt.
.mc 4
The version of that document
based on X Windows Version 11, Release 5 is required for \*(Zx.
.mc
.H 2 "ICCCM, CT, XLFD, BDF"
The interfaces in this document require an underlying implementation
that support the interfaces defined in the \*(Zi.
.mc 4
The version of that document
based on X Windows Version 11, Release 5 is required for \*(Zx.
.mc
.H 2 "Motif Libraries"
The interfaces in this document require an underlying implementation
that support the interfaces defined in the \*(ZM.
.H 2 "X Windows and Motif Data Types"
The following data types are defined in X11 and Motif
headers (see the \*(Zl, the \*(Zt, and the \*(ZM):
.DS
.TS
center;
lef3 lef3 lef3 .
ArgList	Pixel	XEvent
Atom	Pixmap	XmFontList
Boolean	Position	XmString
Cardinal	String	XmStringDirection
Colormap	Time	XmStringTable
Cursor	Widget	XmTextPosition
Dimension	WidgetClass	XtCallbackList
Display	XButtonPressedEvent	XtPointer
.TE
.DE
.P
.iN
This list is complete as of the company review draft of \*(XX Motif.
When that draft is updated, we expect additional types
to be added to this list.
.sA
.H 2 "\*(Zx Widgets"
This section defines the widget classes
that provide the \*(Zx-specific widgets
to support application portability at the C-language source level.
.so ../fx/DtComboBox.mm
.so ../fx/DtMenuButton.mm
.so ../fx/DtSpinBox.mm
.ds XC \*(Zx Widget Convenience Functions
.SK
.H 2 "\*(Zx Widget Convenience Functions
.xR2 widfuncs
This section defines the functions, macros and
external variables that provide \*(Zx convenience functions
for the \*(Zx-specific widgets
to support application portability at the C-language source level.
.so ../fx/DtComboBoxAddItem.mm
.so ../fx/DtComboBoxDeletePos.mm
.so ../fx/DtComboBoxSelectItem.mm
.so ../fx/DtComboBoxSetItem.mm
.so ../fx/DtCreateComboBox.mm
.so ../fx/DtCreateMenuButton.mm
.so ../fx/DtCreateSpinBox.mm
.so ../fx/DtSpinBoxAddItem.mm
.so ../fx/DtSpinBoxDeletePos.mm
.so ../fx/DtSpinBoxSetItem.mm
.ds XC \*(Zx Widget Headers
.SK
.H 2 "\*(Zx Widget Headers"
This section describes the contents of headers used
by the \*(Zx drag and drop functions, macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in 
.cX widfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/DtComboBox.mm
.so ../hx/DtMenuButton.mm
.so ../hx/DtSpinBox.mm
.ds XC Rationale
.SK
.H 2 "Rationale and Open Issues"
.I
The following is text copied from the
.R
CDE X/Open Specification Framework Proposal, Draft 2.
.I
It is not intended to remain in this specification
when it is published by X/Open.
.R
.P
The APIs for the custom widgets defined to supplement Motif 1.2 are
required for application portability because the use of these widgets
is mandated by the Style Guide.
.P
.mc 1
The following were removed from the list in
CDE X/Open Specification Framework Proposal, Draft 2:
.VL 15
.LI \f2DtIconGadget\fP
A private interface not required for application portability.
.LI \f2DtPrintBox\fP
This print interface is no longer supported within \*(Zx.
.LE
.eF e
