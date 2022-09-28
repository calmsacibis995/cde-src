.ds SI @(#) 1.2 94/12/19
.Ho Contents 3
.tr#
.TS
tab(@);
l1w(2c) l1w(1.5c) l1w(10.5c) rw(0.6c).
.sp 1v
\s+2\f3Chapter@1@Window Management Services\s0\a@1\f1
##@1.1@###Introduction\a@1
##@1.2@###Data Formats\a@2
##@1.3@###Rationale and Open Issues\a@2
.sp 1v
\s+2\f3Chapter@2@Workspace Management Services\s0\a@5\f1
##@2.1@###Introduction\a@5
##@2.2@###Functions\a@5
##@##@\f2DtWsmAddCurrentWorkspaceCallback\f1\^(\|)\a@6
##@##@\f2DtWsmAddWorkspaceFunctions\f1\^(\|)\a@8
##@##@\f2DtWsmAddWorkspaceModifiedCallback\f1\^(\|)\a@10
##@##@\f2DtWsmFreeWorkspaceInfo\f1\^(\|)\a@12
##@##@\f2DtWsmGetCurrentBackdropWindow\f1\^(\|)\a@13
##@##@\f2DtWsmGetCurrentWorkspace\f1\^(\|)\a@14
##@##@\f2DtWsmGetWorkspaceInfo\f1\^(\|)\a@15
##@##@\f2DtWsmGetWorkspaceList\f1\^(\|)\a@18
##@##@\f2DtWsmGetWorkspacesOccupied\f1\^(\|)\a@20
##@##@\f2DtWsmOccupyAllWorkspaces\f1\^(\|)\a@22
##@##@\f2DtWsmRemoveWorkspaceCallback\f1\^(\|)\a@23
##@##@\f2DtWsmRemoveWorkspaceFunctions\f1\^(\|)\a@24
##@##@\f2DtWsmSetCurrentWorkspace\f1\^(\|)\a@25
##@##@\f2DtWsmSetWorkspacesOccupied\f1\^(\|)\a@26
##@2.3@###Headers\a@27
##@##@\f3<Dt/Wsm.h>\f1\a@28
##@2.4@###Rationale and Open Issues\a@30
.sp 1v
\s+2\f3Chapter@3@Session Management Services\s0\a@31\f1
##@3.1@###Introduction\a@31
##@3.2@###Functions\a@31
##@##@\f2DtSaverGetWindows\f1\^(\|)\a@32
##@##@\f2DtSessionRestorePath\f1\^(\|)\a@33
##@##@\f2DtSessionSavePath\f1\^(\|)\a@34
##@3.3@###Headers\a@35
##@##@\f3<Dt/Saver.h>\f1\a@36
##@##@\f3<Dt/Session.h>\f1\a@37
##@3.4@###Actions\a@38
##@##@\f3<dtsessionaction>\f1\a@39
##@3.5@###Capabilities\a@40
##@3.6@###Rationale and Open Issues\a@41
.sp 1v
\s+2\f3Chapter@4@Help Services\s0\a@43\f1
##@4.1@###Introduction\a@43
##@4.2@###Widgets\a@43
##@##@\f2DtHelpDialog\f1\^(\|)\a@44
##@##@\f2DtHelpQuickDialog\f1\^(\|)\a@56
##@4.3@###Functions\a@70
##@##@\f2DtCreateHelpDialog\f1\^(\|)\a@71
##@##@\f2DtCreateHelpQuickDialog\f1\^(\|)\a@72
##@##@\f2DtHelpQuickDialogGetChild\f1\^(\|)\a@73
##@##@\f2DtHelpReturnSelectedWidgetId\f1\^(\|)\a@74
##@##@\f2DtHelpSetCatalogName\f1\^(\|)\a@76
##@4.4@###Headers\a@77
##@##@\f3<Dt/Help.h>\f1\a@78
##@##@\f3<Dt/HelpDialog.h>\f1\a@80
##@##@\f3<Dt/HelpQuickD.h>\f1\a@81
##@4.5@###Actions\a@82
##@##@\f3<dtmanaction>\f1\a@83
##@##@\f3<dthelpaction>\f1\a@84
##@4.6@###Formats\a@85
##@4.7@###Capabilities\a@93
##@4.7.1@######Presentation in the Quick Help Window\a@93
##@4.7.2@######Navigation in the Quick Help Window\a@93
##@4.7.3@######Presentation for the General Help Window\a@94
##@4.7.4@######Navigation for the General Help Window\a@94
##@4.8@###Rationale and Open Issues\a@94
.sp 1v
\s+2\f3Chapter@5@Calendar and Appointment Services\s0\a@97\f1
##@5.1@###Introduction\a@97
##@5.2@###Functions\a@97
##@##@\f2csa_x_process_updates\f1\^(\|)\a@99
##@5.3@###Headers\a@101
##@5.4@###Command-Line Interfaces\a@101
##@##@\f2dtcm_admin\f1\^\a@102
##@##@\f2dtcm_delete\f1\^\a@106
##@##@\f2dtcm_insert\f1\^\a@109
##@##@\f2dtcm_lookup\f1\^\a@113
##@5.5@###Actions\a@116
##@##@\f3<dtcalendaraction>\f1\a@117
##@5.6@###Messages\a@119
##@5.7@###Formats\a@119
##@5.7.1@######Calendar Archive File Format\a@119
##@5.7.1.1@#########Attribute Definition\a@120
##@5.7.1.2@#########Long Values\a@122
##@5.7.2@######Calendar Entry Format\a@122
##@5.8@###Capabilities\a@125
##@5.8.1@######Calendar Main Window\a@125
##@5.8.2@######Options/Properties\a@127
##@5.8.3@######Appointment Editing\a@127
##@5.8.4@######Appointment Listing\a@129
##@5.8.5@######Appointment Finding\a@129
##@5.8.6@######To-Do Editing\a@129
##@5.8.7@######To-Do Listing\a@130
##@5.8.8@######Multi-User Calendar Accessing\a@131
##@5.8.9@######Drag and Drop Capabilities\a@131
##@5.8.10@######Printing\a@132
##@5.8.11@######Other Capabilities\a@132
##@5.9@###Rationale and Open Issues\a@132
.sp 1v
\s+2\f3Chapter@6@Mail Services\s0\a@135\f1
##@6.1@###Introduction\a@135
##@6.2@###Actions\a@136
##@##@\f3<dtmailaction>\f1\a@137
##@6.3@###Messages\a@138
##@6.4@###Formats\a@138
##@6.5@###Capabilities\a@139
##@6.5.1@######Managing Mailboxes\a@140
##@6.5.2@######Managing Message Lists\a@140
##@6.5.3@######Viewing and Manipulating Existing Messages\a@142
##@6.5.4@######Composing New Messages\a@143
##@6.5.5@######Drag and Drop Capabilities\a@146
##@6.5.6@######Other Capabilities\a@147
##@6.6@###Rationale and Open Issues\a@147
.sp 1v
\s+2\f3Chapter@7@File Management Services\s0\a@149\f1
##@7.1@###Introduction\a@149
##@7.2@###Actions\a@149
##@##@\f3<dtfileaction>\f1\a@150
##@##@\f3<dttrashaction>\f1\a@152
##@7.3@###Messages\a@153
##@7.4@###Capabilities\a@153
##@7.4.1@######Folder Window\a@154
##@7.4.2@######Application Folder Window\a@154
##@7.4.3@######Trash Folder Window\a@155
##@7.4.4@######Workspaces\a@155
##@7.4.5@######Object Movement and Modification\a@155
##@7.4.6@######Object Search\a@155
##@7.4.7@######Folder Traversal\a@156
##@7.4.8@######Object Type/Action Association\a@156
##@7.4.9@######Registering Objects as Drop Sites\a@156
##@7.4.10@######Exit Services\a@156
##@7.5@###Rationale and Open Issues\a@157
.sp 1v
\s+2\f3Chapter@8@Front Panel Services\s0\a@159\f1
##@8.1@###Introduction\a@159
##@8.2@###Formats\a@159
##@8.2.1@######File Format\a@159
##@8.2.2@######Record Types\a@160
##@8.2.3@######Keyword and Value Descriptions\a@164
##@8.3@###Capabilities\a@172
##@8.3.1@######General Layout\a@172
##@8.3.2@######Special Controls\a@173
##@8.3.3@######Other Capabilities\a@174
##@8.4@###Rationale and Open Issues\a@174
.sp 1v
\s+2\f3Chapter@9@Text Editing Services\s0\a@175\f1
##@9.1@###Introduction\a@175
##@9.2@###Widgets\a@175
##@##@\f2DtEditor\f1\^(\|)\a@176
##@9.3@###Functions\a@201
##@##@\f2DtCreateEditor\f1\^(\|)\a@202
##@##@\f2DtEditorAppend\f1\^(\|)\a@203
##@##@\f2DtEditorAppendFromFile\f1\^(\|)\a@205
##@##@\f2DtEditorChange\f1\^(\|)\a@207
##@##@\f2DtEditorCheckForUnsavedChanges\f1\^(\|)\a@209
##@##@\f2DtEditorClearSelection\f1\^(\|)\a@210
##@##@\f2DtEditorCopyToClipboard\f1\^(\|)\a@211
##@##@\f2DtEditorCutToClipboard\f1\^(\|)\a@212
##@##@\f2DtEditorDeleteSelection\f1\^(\|)\a@213
##@##@\f2DtEditorDeselect\f1\^(\|)\a@214
##@##@\f2DtEditorFind\f1\^(\|)\a@215
##@##@\f2DtEditorFormat\f1\^(\|)\a@216
##@##@\f2DtEditorGetContents\f1\^(\|)\a@218
##@##@\f2DtEditorGetInsertionPosition\f1\^(\|)\a@220
##@##@\f2DtEditorGetLastPosition\f1\^(\|)\a@221
##@##@\f2DtEditorGetSizeHints\f1\^(\|)\a@222
##@##@\f2DtEditorGoToLine\f1\^(\|)\a@224
##@##@\f2DtEditorInsert\f1\^(\|)\a@225
##@##@\f2DtEditorInsertFromFile\f1\^(\|)\a@227
##@##@\f2DtEditorInvokeFindChangeDialog\f1\^(\|)\a@229
##@##@\f2DtEditorInvokeFormatDialog\f1\^(\|)\a@230
##@##@\f2DtEditorPasteFromClipboard\f1\^(\|)\a@231
##@##@\f2DtEditorReplace\f1\^(\|)\a@232
##@##@\f2DtEditorReplaceFromFile\f1\^(\|)\a@235
##@##@\f2DtEditorReset\f1\^(\|)\a@237
##@##@\f2DtEditorSaveContentsToFile\f1\^(\|)\a@238
##@##@\f2DtEditorSelectAll\f1\^(\|)\a@241
##@##@\f2DtEditorSetContents\f1\^(\|)\a@242
##@##@\f2DtEditorSetContentsFromFile\f1\^(\|)\a@244
##@##@\f2DtEditorSetInsertionPosition\f1\^(\|)\a@246
##@##@\f2DtEditorTraverseToEditor\f1\^(\|)\a@247
##@##@\f2DtEditorUndoEdit\f1\^(\|)\a@248
##@9.4@###Headers\a@249
##@##@\f3<Dt/Editor.h>\f1\a@250
##@9.5@###Command-Line Interfaces\a@258
##@##@\f2dtpad\f1\^\a@259
##@9.6@###Actions\a@264
##@##@\f3<dttextaction>\f1\a@265
##@9.7@###Messages\a@266
##@9.8@###Capabilities\a@268
##@9.8.1@######File Management\a@268
##@9.8.2@######Presentation\a@269
##@9.8.3@######Text Editing\a@269
##@9.9@###Rationale and Open Issues\a@270
.sp 1v
\s+2\f3Chapter@10@Icon Editing Services\s0\a@271\f1
##@10.1@###Introduction\a@271
##@10.2@###Actions\a@271
##@##@\f3<dticonaction>\f1\a@272
##@10.3@###Messages\a@273
##@10.4@###Capabilities\a@273
##@10.5@###Rationale and Open Issues\a@274
.sp 1v
\s+2\f3Chapter@11@GUI Scripting Services\s0\a@275\f1
##@11.1@###Introduction\a@275
##@11.2@###Command-Line Interfaces\a@275
##@##@\f2dtksh\f1\^\a@276
.sp 1v
\s+2\f3Chapter@12@Terminal Emulation Services\s0\a@339\f1
##@12.1@###Introduction\a@339
##@12.2@###Functions\a@339
##@##@\f2DtCreateTerm\f1\^(\|)\a@340
##@##@\f2DtTermDisplaySend\f1\^(\|)\a@341
##@##@\f2DtTermInitialize\f1\^(\|)\a@342
##@##@\f2DtTermSubprocReap\f1\^(\|)\a@343
##@##@\f2DtTermSubprocSend\f1\^(\|)\a@344
##@12.3@###Widgets\a@345
##@##@\f2DtTerm\f1\^(\|)\a@346
##@12.4@###Headers\a@369
##@##@\f3<Dt/Term.h>\f1\a@370
##@12.5@###Command-Line Interfaces\a@371
##@##@\f2dtterm\f1\^\a@372
##@12.6@###Actions\a@395
##@##@\f3<dttermaction>\f1\a@396
##@12.7@###Formats\a@397
##@12.7.1@######Received Escape Sequences\a@397
##@12.7.2@######Reset\a@416
##@12.7.3@######Transmitted Escape Sequences\a@418
##@12.7.4@######Cursor Key Mode\a@418
##@12.7.5@######Application Keypad Mode\a@418
##@12.7.6@######Standard Function Keys\a@419
##@12.7.7@######Sun Function Keys\a@420
##@12.8@###Capabilities\a@421
##@12.9@###Rationale and Open Issues\a@422
.sp 1v
\s+2\f3Chapter@13@Style Management Services\s0\a@425\f1
##@13.1@###Introduction\a@425
##@13.2@###Actions\a@425
##@##@\f3<dtstyleaction>\f1\a@426
##@13.3@###Capabilities\a@427
##@13.4@###Rationale and Open Issues\a@428
.sp 1v
\s+2\f3Chapter@14@Application Building Services\s0\a@429\f1
##@14.1@###Introduction\a@429
##@14.2@###Command-Line Interfaces\a@430
##@##@\f2dtcodegen\f1\^\a@431
##@14.3@###Actions\a@437
##@##@\f3<dtbuilderaction>\f1\a@438
##@14.4@###Capabilities\a@439
##@14.4.1@######Project and Module Files\a@439
##@14.4.2@######Project Management\a@440
##@14.4.3@######Object Palette\a@440
##@14.4.4@######Object Layout\a@442
##@14.4.5@######Object Properties\a@442
##@14.4.6@######Browser Window\a@454
##@14.4.7@######Application Framework\a@455
##@14.4.8@######Connections\a@458
##@14.4.9@######Drag and Drop Capabilities\a@458
##@14.5@###Rationale and Open Issues\a@459
.sp 1v
\s+2\f3Chapter@15@Application Integration Services\s0\a@461\f1
##@15.1@###Introduction\a@461
##@15.2@###Command-Line Interfaces\a@461
##@##@\f2dtappintegrate\f1\^\a@462
##@15.3@###Actions\a@466
##@##@\f3<dtappaction>\f1\a@467
##@15.4@###Rationale and Open Issues\a@468
.sp 1v
\s+2\f3Chapter@16@Action Creation Services\s0\a@469\f1
##@16.1@###Introduction\a@469
##@16.2@###Actions\a@469
##@##@\f3<dtactionaction>\f1\a@470
##@16.3@###Capabilities\a@471
##@16.4@###Rationale and Open Issues\a@472
.sp 1v
\s+2\f3Chapter@17@Print Job Services\s0\a@473\f1
##@17.1@###Introduction\a@473
##@17.2@###Actions\a@473
##@##@\f3<dtprintinfoaction>\f1\a@474
##@17.3@###Capabilities\a@475
##@17.4@###Rationale and Open Issues\a@475
.sp 1v
\s+2\f3Chapter@18@Calculator Services\s0\a@477\f1
##@18.1@###Introduction\a@477
##@18.2@###Actions\a@477
##@##@\f3<dtcalcaction>\f1\a@478
##@18.3@###Capabilities\a@479
##@18.3.1@######General Calculator Capabilities\a@479
##@18.3.2@######Arithmetic Operations\a@480
##@18.3.3@######Scientific Operations\a@480
##@18.3.4@######Financial Operations\a@481
##@18.3.5@######Logical Operations\a@482
##@18.4@###Rationale and Open Issues\a@482
.sp 1v
\s+2\f3Chapter@19@Application Conventions\s0\a@483\f1
##@19.1@###Font Conventions\a@483
##@19.1.1@######Standard Application Font Names\a@483
##@19.1.1.1@#########Background\a@483
##@19.1.1.2@#########Rationale\a@483
##@19.1.1.3@#########The Standard Names for the Latin-1 Character Set\a@485
##@19.1.1.4@#########XLFD Field Values for the Standard Application Font Names\a@485
##@19.1.1.5@#########Point Sizes\a@486
##@19.1.1.6@#########Example XLFD Patterns for the Standard Names\a@487
##@19.1.1.7@#########Implementation of Font Names\a@488
##@19.1.1.8@#########Default \s-1XCDE\s+1 Mappings for Latin-1 Locales\a@488
##@19.1.1.9@#########Font Names in app-defaults Files\a@489
##@19.1.1.10@#########Other Character Sets in the Common Locales\a@489
##@19.1.2@######Standard Interface Font Names\a@491
##@19.1.2.1@#########Background\a@491
##@19.1.2.2@#########Rationale\a@491
##@19.1.2.3@#########XLFD Field Values for the Standard Interface Font Names\a@492
##@19.1.2.4@#########Restricted Set of Styles Available\a@494
##@19.1.2.5@#########Named Set of Point Sizes Available\a@494
##@19.1.2.6@#########Example XLFD Patterns for the Standard Names\a@495
##@19.1.2.7@#########Implementation of Font Names\a@496
##@19.1.2.8@#########Default \s-1XCDE\s+1 Mapping of the Standard Interface Font Names\a@499
##@19.2@###Icon Conventions\a@499
##@19.2.1@######File Naming\a@499
##@19.2.2@######Icon Sizes\a@500
##@19.2.3@######Icon File Locations\a@501
##@19.3@###Rationale and Open Issues\a@501
.sp 1v
\s+2\f3Chapter@20@Application Style Checklist\s0\a@503\f1
##@20.1@###Preface\a@504
##@20.2@###Input Models\a@504
##@20.2.1@######The Keyboard Focus Model\a@504
##@20.2.2@######The Input Device Model\a@505
##@20.3@###Navigation\a@510
##@20.3.1@######Mouse-Based Navigation\a@510
##@20.3.2@######Keyboard-Based Navigation\a@515
##@20.3.3@######Menu Traversal\a@521
##@20.3.4@######Scrollable Component Navigation\a@527
##@20.4@###Selection\a@528
##@20.4.1@######Selection Models\a@528
##@20.4.1.1@#########Mouse-Based Single Selection\a@528
##@20.4.1.2@#########Mouse-Based Browse Selection\a@529
##@20.4.1.3@#########Mouse-Based Multiple Selection\a@529
##@20.4.1.4@#########Mouse-Based Range Selection\a@530
##@20.4.1.5@#########Mouse-Based Discontiguous Selection\a@533
##@20.4.1.6@#########Keyboard Selection\a@535
##@20.4.1.7@#########Canceling a Selection\a@540
##@20.4.1.8@#########Autoscrolling and Selection\a@541
##@20.4.1.9@#########Selecting and Deselecting All Elements\a@541
##@20.4.1.10@#########Using Mnemonics for Elements\a@542
##@20.4.2@######Selection Actions\a@542
##@20.4.3@######Transfer Models\a@544
##@20.4.3.1@#########Clipboard Transfer\a@546
##@20.4.3.2@#########Primary Transfer\a@548
##@20.4.3.3@#########Quick Transfer\a@549
##@20.4.3.4@#########Drag Transfer\a@551
##@20.5@###Component Activation\a@557
##@20.5.1@######Basic Activation\a@557
##@20.5.2@######Accelerators\a@559
##@20.5.3@######Mnemonics\a@560
##@20.5.4@######Tear-Off Activation\a@561
##@20.5.5@######Help Activation\a@562
##@20.5.6@######Default Activation\a@562
##@20.5.7@######Expert Activation\a@563
##@20.5.8@######Previewing and Autorepeat\a@564
##@20.5.9@######Cancel Activation\a@564
##@20.6@###Window Management\a@565
##@20.6.1@######Window Support\a@565
##@20.6.2@######Window Decorations\a@566
##@20.6.3@######Window Navigation\a@568
##@20.6.4@######Icons\a@568
##@20.6.5@######Application Window Management\a@568
##@20.6.5.1@#########Window Placement\a@568
##@20.6.5.2@#########Window (Document) Clustering\a@569
##@20.6.5.3@#########Window Management Actions\a@570
##@20.6.6@###### Session Management Support\a@571
##@20.7@###Application Design Principles\a@572
##@20.7.1@######Layout\a@572
##@20.7.1.1@#########Main Window\a@572
##@20.7.1.2@#########Window Titles\a@574
##@20.7.1.3@#########Menu Bar\a@576
##@20.7.1.4@#########File Menu Contents\a@578
##@20.7.1.5@#########Help Menu Contents\a@585
##@20.7.1.6@#########Pop-up Menus\a@587
##@20.7.1.7@#########Dialog Boxes\a@594
##@20.7.1.8@#########Menu Design\a@594
##@20.7.1.9@#########Dialog Box Design\a@597
##@20.7.1.10@#########File Selection Dialog Box\a@603
##@20.7.1.11@#########About Dialog Box\a@605
##@20.7.1.12@#########Dialog Box Layout\a@606
##@20.7.1.13@#########Designing Drag and Drop\a@607
##@20.7.2@######Attachments\a@612
##@20.7.3@######Installation\a@613
##@20.7.4@######Interaction\a@613
##@20.7.5@######Visuals\a@617
##@20.7.6@######Toolbars\a@618
##@20.7.7@######Expandable Windows\a@620
##@20.7.8@######Messages\a@622
##@20.7.9@######Work-in-Progress Feedback\a@628
##@20.8@###Controls, Groups and Models\a@629
##@20.8.1@######CheckButton\a@629
##@20.8.2@######CommandBox\a@631
##@20.8.3@######FileSelectionBox\a@632
##@20.8.4@######List\a@638
##@20.8.5@######Option Button\a@639
##@20.8.6@######Paned Window\a@640
##@20.8.7@######Panel\a@641
##@20.8.8@######Push Button\a@641
##@20.8.9@######Radio Button\a@642
##@20.8.10@######Sash\a@643
##@20.8.11@######Scale\a@644
##@20.8.12@######ScrollBar\a@647
##@20.8.13@######SelectionBox\a@649
##@20.8.14@######Text\a@650
##@20.8.15@######Gauge\a@653
##@20.9@###Accessibility\a@653
.sp 1v
##@##@\s+2\f3Index\s0\a@657\f1
.TE
.tr ##
.eF e
