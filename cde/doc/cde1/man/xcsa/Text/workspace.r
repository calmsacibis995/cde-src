.\" Copyright 1994, The X/Open Company Ltd.
.ds SI @(#) 1.2 94/12/19
.H 1 "Workspace Management Services"
.xR1 wkspmgmtsvcs
.H 2 "Introduction"
The \*(Zx workspace manager provides support for multiple workspaces.
Each workspace is a ``virtual screen'';
windows can be placed in a single workspace, all workspaces
or any combination of individual workspaces.
The initial number of workspaces is determined
by a resource when the workspace manager starts up.
Workspaces may be added, deleted or renamed dynamically.
Workspace switching can be
done through the workspace switch in the front panel or by binding a
workspace manager function to a button, key or window manager menu.
The workspace manager cooperates with the session manager to save the
workspace state between sessions.
.P
The workspace manager provides API support for querying and controlling
the workspace state and adding and removing windows from workspaces.
Clients need not be aware of workspaces or the workspace manager to
operate properly in the \*(Zx.
.H 2 "Functions" S
.xR2 wsmfuncs
This section defines the functions, macros and
external variables that provide \*(Zx workspace management services
to support application portability at the C-language source level.
.so ../fx/DtWsmAddCurrentWorkspaceCallback.mm
.so ../fx/DtWsmAddWorkspaceFunctions.mm
.so ../fx/DtWsmAddWorkspaceModifiedCallback.mm
.so ../fx/DtWsmFreeWorkspaceInfo.mm
.so ../fx/DtWsmGetCurrentBackdropWindows.mm
.so ../fx/DtWsmGetCurrentWorkspace.mm
.so ../fx/DtWsmGetWorkspaceInfo.mm
.so ../fx/DtWsmGetWorkspaceList.mm
.so ../fx/DtWsmGetWorkspacesOccupied.mm
.so ../fx/DtWsmOccupyAllWorkspaces.mm
.so ../fx/DtWsmRemoveWorkspaceCallback.mm
.so ../fx/DtWsmRemoveWorkspaceFunctions.mm
.so ../fx/DtWsmSetCurrentWorkspace.mm
.so ../fx/DtWsmSetWorkspacesOccupied.mm
.ds XC Headers
.SK
.H 2 "Headers" S
This section describes the contents of headers used
by the \*(Zx workspace management service functions,
macros and external variables.
.P
Headers contain the definition of symbolic constants, common structures,
preprocessor macros and defined types.
Each function in
.cX wsmfuncs "" 1
specifies the headers that an application must include in order to use
that function.
In most cases only one header is required.
These headers are present on an application development
system; they do not have to be present on the target execution system.
.so ../hx/DtWsm.mm
.ds XC Rationale
.SK
.H 2 "Rationale and Open Issues"
.I
The following is text copied from the
.R
CDE X/Open Specification Framework Proposal, Draft 2.
.I
It is not intended to remain in this specification when it is published
by X/Open.
.R
.P
APIs are required for application portability.
Protocols are not
planned for standardisation because they are subject to change and
suitable base documentation is not available.
The format and location
of backdrop files are not planned for standardisation because the
location of files is considered an administrative aspect out of scope;
the format is XPM, but without the location, the format is immaterial
to the proposed standardisation targets.
.P
.mc 2
The
.Fn DtWsmRemoveWorkspaceModifiedCallback
and
.Fn DtWsmRemoveCurrentWorkspaceCallback
functions listed in Draft 1 were changed in the architecture to be
a single function:
.Fn DtWsmRemoveWorkspaceCallback .
.P
The CDE Sponsors are discussing XPM standardisation requirements
with OSF, but the final decision has not yet been made.
.br
.mc
.eF e
