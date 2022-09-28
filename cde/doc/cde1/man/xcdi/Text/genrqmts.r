.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 2 "\*(Zx Data Format Naming"
.xR2 datafmtnaming
.mc 4
The \*(ZX supports a common name space to describe
file formats that can be interchanged
between applications\*(EM\&the Media name space.
The names in the Media name space describe the form of the
data itself.
Media names are used as ICCCM selection targets (see the \*(Zi);
they are named in the MEDIA field of data type records (see
.cX datatypechap );
and they are used in the
.I type
parameter of Media Exchange messages (see
.cX mediamessages ).
.P
The Media name space is a subset of the
space of selection target atoms as defined by the ICCCM.
All selection targets that specify a data format are valid
Media names, and all valid Media names can be used directly
as selection targets.
Some selection targets specify an
attribute of the selection (for example, LIST_LENGTH)
or a side effect to occur (for example, DELETE),
rather than a data format.
These selection targets are not part of the Media name space.
.P
The space of Media names
follows the ICCCM conventions for selection target atoms.
Names typically
are in all uppercase, with words separated by underscores.
Media names should follow this convention where possible.
.P
The following Media names have the meanings indicated on
all \*(Zx systems.
It is implementation dependent whether facilities
are provided to manipulate any of these formats.
.VL 18 8
.LI ADOBE_PORTABLE_DOCUMENT_FORMAT
.br
Adobe Systems, Incorporated.
.I "Portable Document Format Reference Manual."
Addison-Wesley, ISBN 0-201-62628-4.
.LI APPLE_PICT
.br
Apple Computer, Incorporated.
.I "Inside Macintosh, Volume V."
Chapter 4, ``Color QuickDraw,'' Color Picture Format.
ISBN 0-201-17719-6.
.LI COMPOUND_TEXT
.br
Scheifler, Robert W.
.I "Compound Text Encoding, Version 1.1."
MIT X Consortium Standard.
X Version 11, Release 5, 1991.
.LI ENCAPSULATED_POSTSCRIPT
.sp -0.4v
.LI ENCAPSULATED_POSTSCRIPT_INTERCHANGE
.br
Adobe Systems, Incorporated.
.I "PostScript Language Reference Manual."
Appendix H.
Addison-Wesley, ISBN 0-201-18127-4.
.LI GIF87
Graphics Interchange Format.
June 15, 1987.
CompuServe, Incorporated, 1987.
.DS I
CompuServe, Incorporated
5000 Arlington Centre Blvd.
Columbus, OH  43220
.DE
.LI GIF89
Graphics Interchange Format, Version 89a.
31 July 1990.
CompuServe, Incorporated, 1987-1990.
.LI JFIF
Hamilton, Eric.
.I "JPEG File Interchange Format, Version 1.02."
September 1, 1992.
.DS I
C-Cube Microsystems
1778 McCarthy Blvd.
Milpitas, CA  95035
.DE
.LI POSTSCRIPT
.br
Adobe Systems, Incorporated.
.I "PostScript Language Reference Manual."
Appendix H.
Addison-Wesley, ISBN 0-201-18127-4.
.br
.mc 6
.LI RFC_822_MESSAGE
.br
A mail message formatted in accordance with the \*(Zh
and the \*(Zm.
.br
.mc
.LI SND
Apple Macintosh sound data format
\*(EM
.iN
.mc 6
We do not have the correct Apple document reference.
It will be added in the final standard.
.sA
.mc
.LI STRING
Text encoded in the printable characters,
plus
.tK tab
and
.tK newline ,
of the Latin-1 codeset (see the \*(Z7)
.LI SUN_AUDIO_DATA
.br
Solaris 2.3 SUNBIN CD-ROM part number 258-3779.
Sun Microsystems, Inc., 1993.
Audio demo (SUNWaudmo) package: manual pages
.IR audio_intro (3)
and
.IR audio_filehdr (3).
.LI SUN_RASTER
.br
.I "OpenWindows 3.3 Reference Manual"
Sun Microsystems, Inc., 1990-1993.
Manual page
.IR rasterfile (5).
.LI TIFF
TIFF Revision 6.0, June 3, 1992.
.DS I
Aldus Corporation
411 First Avenue South
Seattle, WA  98104-2871
.DE
.LI WAV
.mc 5
IBM PC sound file format
\*(EM
.I "Multimedia Programming Interface and Data Specifications 1.0,"
a joint design by IBM Corporation and
Microsoft Corporation, August 1991.
.DS I
Microsoft Corporation
Multimedia Systems Group
Product Marketing
One Microsoft Way
Redmond, WA 98052-6399
.DE
.LI XBM
.mc 4
\*(Zl, Section 11.10, ``Manipulating Bitmaps.''
.LI XPM
Le Hors, Arnaud.
.I "XPM Manual: The X PixMap Format, Version 3.0."
Groupe Bull, 1990-1991.
Contributed Software.
X Version 11, Release 5.
.LE
.mc
.eE
