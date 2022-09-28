.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Data Typing"
.xR1 datatypechap
.nr Ej 1
.H 2 "Introduction"
.mc 3
The \*(Zx data typing services
provide data capabilities that enhance the use of traditional file systems.
These capabilities includes typing and attribute management.
.P
The data typing services
consist of the data criteria table, the data attributes
table and the API used to access the tables.
Data typing, using
data criteria, can determine the data attributes of a file or byte
vector, based on its name, file permissions, symbolic links and content.
Data attributes determine user-visible interfaces to data:
a human-readable description of the type, the icon to represent it
graphically and the actions that apply to it.
An object's data attributes also indicate the unique string that names
the data interchange format to which the data's contents conform.
.br
.mc
.H 2 "Functions"
.xR2 dtsfuncs
This section defines the functions, macros and
external variables that provide \*(Zx data typing services
to support application portability at the C-language source level.
.so ../fx/DtDtsBufferToAttributeList.mm
.so ../fx/DtDtsBufferToAttributeValue.mm
.so ../fx/DtDtsBufferToDataType.mm
.so ../fx/DtDtsDataToDataType.mm
.so ../fx/DtDtsDataTypeIsAction.mm
.so ../fx/DtDtsDataTypeNames.mm
.so ../fx/DtDtsDataTypeToAttributeList.mm
.so ../fx/DtDtsDataTypeToAttributeValue.mm
.so ../fx/DtDtsFileToAttributeList.mm
.so ../fx/DtDtsFileToAttributeValue.mm
.so ../fx/DtDtsFileToDataType.mm
.so ../fx/DtDtsFindAttribute.mm
.so ../fx/DtDtsFreeAttributeList.mm
.so ../fx/DtDtsFreeAttributeValue.mm
.so ../fx/DtDtsFreeDataType.mm
.so ../fx/DtDtsFreeDataTypeNames.mm
.so ../fx/DtDtsIsTrue.mm
.so ../fx/DtDtsLoadDataTypes.mm
.so ../fx/DtDtsRelease.mm
.so ../fx/DtDtsSetDataType.mm
.ds XC Headers
.SK
.H 2 "Headers"
This section describes the contents of headers used
by the \*(Zx data typing functions, macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in 
.cX dtsfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/DtDts.mm
.ds XC Data Formats
.SK
.H 2 "Data Formats"
.xR2 dtdtsfile
.so ../fx/dtdtfile.mm
.so ../fx/dtdtsfile.mm
.br
.mc
.R
.eF e
