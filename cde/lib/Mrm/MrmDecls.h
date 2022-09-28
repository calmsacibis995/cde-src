/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $RCSfile: MrmDecls.h,v $ $Revision: 1.5 $ $Date: 94/04/19 07:46:03 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
#ifndef MrmDecls_H
#define MrmDecls_H

/*----------------------------------*/
/* Error messages                   */
/*----------------------------------*/
/* The symbol _MrmConst is used for constant data that cannot be
 * declared const in the header file because of usage as arguments to
 * routines which have string arguments that are not declared const.
 *
 * So, _XmConst is always defined to be nothing in header files.
 * In the source file, however, _XmConst is defined to be const,
 * so as to allow shared data in a shared library environment.
 */

#ifndef _MrmConst
#define _MrmConst
#endif

externalref _MrmConst char _MrmMsg_0000[];
externalref _MrmConst char _MrmMsg_0001[];
externalref _MrmConst char _MrmMsg_0002[];
externalref _MrmConst char _MrmMsg_0003[];
externalref _MrmConst char _MrmMsg_0004[];
externalref _MrmConst char _MrmMsg_0005[];
externalref _MrmConst char _MrmMsg_0006[];
externalref _MrmConst char _MrmMsg_0007[];
externalref _MrmConst char _MrmMsg_0008[];
externalref _MrmConst char _MrmMsg_0009[];
externalref _MrmConst char _MrmMsg_0010[];
externalref _MrmConst char _MrmMsg_0011[];
externalref _MrmConst char _MrmMsg_0012[];
externalref _MrmConst char _MrmMsg_0013[];
externalref _MrmConst char _MrmMsg_0014[];
externalref _MrmConst char _MrmMsg_0015[];
externalref _MrmConst char _MrmMsg_0016[];
externalref _MrmConst char _MrmMsg_0017[];
externalref _MrmConst char _MrmMsg_0018[];
externalref _MrmConst char _MrmMsg_0019[];
externalref _MrmConst char _MrmMsg_0020[];
externalref _MrmConst char _MrmMsg_0021[];
externalref _MrmConst char _MrmMsg_0022[];
externalref _MrmConst char _MrmMsg_0023[];
externalref _MrmConst char _MrmMsg_0024[];
externalref _MrmConst char _MrmMsg_0025[];
externalref _MrmConst char _MrmMsg_0026[];
externalref _MrmConst char _MrmMsg_0027[];
externalref _MrmConst char _MrmMsg_0028[];
externalref _MrmConst char _MrmMsg_0029[];
externalref _MrmConst char _MrmMsg_0030[];
externalref _MrmConst char _MrmMsg_0031[];
externalref _MrmConst char _MrmMsg_0032[];
externalref _MrmConst char _MrmMsg_0033[];
externalref _MrmConst char _MrmMsg_0034[];
externalref _MrmConst char _MrmMsg_0035[];
externalref _MrmConst char _MrmMsg_0036[];
externalref _MrmConst char _MrmMsg_0037[];
externalref _MrmConst char _MrmMsg_0038[];
externalref _MrmConst char _MrmMsg_0039[];
externalref _MrmConst char _MrmMsg_0040[];
externalref _MrmConst char _MrmMsg_0041[];
externalref _MrmConst char _MrmMsg_0042[];
externalref _MrmConst char _MrmMsg_0043[];
externalref _MrmConst char _MrmMsg_0044[];
externalref _MrmConst char _MrmMsg_0045[];
externalref _MrmConst char _MrmMsg_0046[];
externalref _MrmConst char _MrmMsg_0047[];
externalref _MrmConst char _MrmMsg_0048[];
externalref _MrmConst char _MrmMsg_0049[];
externalref _MrmConst char _MrmMsg_0050[];
externalref _MrmConst char _MrmMsg_0051[];
externalref _MrmConst char _MrmMsg_0052[];
externalref _MrmConst char _MrmMsg_0053[];
externalref _MrmConst char _MrmMsg_0054[];
externalref _MrmConst char _MrmMsg_0055[];
externalref _MrmConst char _MrmMsg_0056[];
externalref _MrmConst char _MrmMsg_0057[];
externalref _MrmConst char _MrmMsg_0058[];
externalref _MrmConst char _MrmMsg_0059[];
externalref _MrmConst char _MrmMsg_0060[];
externalref _MrmConst char _MrmMsg_0061[];
externalref _MrmConst char _MrmMsg_0062[];
externalref _MrmConst char _MrmMsg_0063[];
externalref _MrmConst char _MrmMsg_0064[];
externalref _MrmConst char _MrmMsg_0065[];
externalref _MrmConst char _MrmMsg_0066[];
externalref _MrmConst char _MrmMsg_0067[];
externalref _MrmConst char _MrmMsg_0068[];
externalref _MrmConst char _MrmMsg_0069[];
externalref _MrmConst char _MrmMsg_0070[];
externalref _MrmConst char _MrmMsg_0071[];
externalref _MrmConst char _MrmMsg_0072[];
externalref _MrmConst char _MrmMsg_0073[];
externalref _MrmConst char _MrmMsg_0074[];
externalref _MrmConst char _MrmMsg_0075[];
externalref _MrmConst char _MrmMsg_0076[];
externalref _MrmConst char _MrmMsg_0077[];
externalref _MrmConst char _MrmMsg_0078[];
externalref _MrmConst char _MrmMsg_0079[];
externalref _MrmConst char _MrmMsg_0080[];
externalref _MrmConst char _MrmMsg_0081[];
externalref _MrmConst char _MrmMsg_0082[];
externalref _MrmConst char _MrmMsg_0083[];
externalref _MrmConst char _MrmMsg_0084[];
externalref _MrmConst char _MrmMsg_0085[];
externalref _MrmConst char _MrmMsg_0086[];
externalref _MrmConst char _MrmMsg_0087[];
externalref _MrmConst char _MrmMsg_0088[];
externalref _MrmConst char _MrmMsg_0089[];
externalref _MrmConst char _MrmMsg_0090[];
externalref _MrmConst char _MrmMsg_0091[];
externalref _MrmConst char _MrmMsg_0092[];
externalref _MrmConst char _MrmMsg_0093[];
externalref _MrmConst char _MrmMsg_0094[];
externalref _MrmConst char _MrmMsg_0095[];
externalref _MrmConst char _MrmMsg_0096[];
externalref _MrmConst char _MrmMsg_0097[];
externalref _MrmConst char _MrmMsg_0098[];
externalref _MrmConst char _MrmMsg_0099[];
externalref _MrmConst char _MrmMsg_0100[];
externalref _MrmConst char _MrmMsg_0101[];
externalref _MrmConst char _MrmMsg_0102[];
externalref _MrmConst char _MrmMsg_0103[];
externalref _MrmConst char _MrmMsg_0104[];
externalref _MrmConst char _MrmMsg_0105[];
externalref _MrmConst char _MrmMsg_0106[];
externalref _MrmConst char _MrmMsg_0107[];
externalref _MrmConst char _MrmMsg_0108[];
externalref _MrmConst char _MrmMsg_0109[];
/* BEGIN OSF Fix CR 4859 */
externalref _MrmConst char _MrmMsg_0110[];
/* END OSF Fix CR 4859 */

/*----------------------------------*/
/* URM external routines (Motif)    */
/*----------------------------------*/
#ifndef _ARGUMENTS
#ifdef _NO_PROTO
#define _ARGUMENTS(arglist) ()
#else
#define _ARGUMENTS(arglist) arglist
#endif
#endif

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* mrminit.c */
extern void MrmInitialize  _ARGUMENTS(( void ));

/* mrmlread.c */
extern Cardinal MrmFetchLiteral  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Display *display , XtPointer *value_return , MrmCode *type_return ));
extern Cardinal MrmFetchIconLiteral  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Screen *screen , Display *display , Pixel fgpix , Pixel bgpix , Pixmap *pixmap_return ));
extern Cardinal MrmFetchBitmapLiteral  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Screen *screen , Display *display , Pixmap *pixmap_return , Dimension *width , Dimension *height));
extern Cardinal MrmFetchColorLiteral  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Display *display , Colormap cmap , Pixel *pixel_return ));

/* Deal with Wide stuff now because there is an error in Saber 3.0 */

#if NeedWidePrototypes

extern Cardinal MrmOpenHierarchy  _ARGUMENTS(( int num_files , String *name_list , MrmOsOpenParamPtr *os_ext_list , MrmHierarchy *hierarchy_id_return ));
extern Cardinal MrmOpenHierarchyPerDisplay  _ARGUMENTS(( Display *display , int num_files , String *name_list , MrmOsOpenParamPtr *os_ext_list , MrmHierarchy *hierarchy_id_return ));
extern Cardinal MrmRegisterNames  _ARGUMENTS(( MrmRegisterArglist reglist ,int num_reg ));
extern Cardinal MrmRegisterNamesInHierarchy  _ARGUMENTS(( MrmHierarchy hierarchy_id , MrmRegisterArglist reglist , int num_reg ));

#if defined(__cplusplus) || defined(c_plusplus)
extern Cardinal MrmRegisterClass  _ARGUMENTS(( int class_code , String class_name , String create_name , Widget (*creator )(...), WidgetClass class_record ));
#else
extern Cardinal MrmRegisterClass  _ARGUMENTS(( int class_code , String class_name , String create_name , Widget (*creator )(), WidgetClass class_record ));
#endif

#else

extern Cardinal MrmOpenHierarchy  _ARGUMENTS(( MrmCount num_files , String *name_list , MrmOsOpenParamPtr *os_ext_list , MrmHierarchy *hierarchy_id_return ));
extern Cardinal MrmOpenHierarchyPerDisplay  _ARGUMENTS(( Display *display , MrmCount num_files , String *name_list , MrmOsOpenParamPtr *os_ext_list , MrmHierarchy *hierarchy_id_return ));
extern Cardinal MrmRegisterNames  _ARGUMENTS(( MrmRegisterArglist reglist ,MrmCount num_reg ));
extern Cardinal MrmRegisterNamesInHierarchy  _ARGUMENTS(( MrmHierarchy hierarchy_id , MrmRegisterArglist reglist , MrmCount num_reg ));
extern Cardinal MrmRegisterClass  _ARGUMENTS(( MrmType class_code , String class_name , String create_name , Widget (*creator )(), WidgetClass class_record ));

#endif 

extern Cardinal MrmCloseHierarchy  _ARGUMENTS(( MrmHierarchy hierarchy_id ));
extern Cardinal MrmFetchInterfaceModule  _ARGUMENTS(( MrmHierarchy hierarchy_id , char *module_name , Widget parent , Widget *w_return ));
extern Cardinal MrmFetchWidget  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Widget parent , Widget *w_return , MrmType *class_return ));
extern Cardinal MrmFetchWidgetOverride  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Widget parent , String ov_name , ArgList ov_args , Cardinal ov_num_args , Widget *w_return , MrmType *class_return ));
extern Cardinal MrmFetchSetValues  _ARGUMENTS(( MrmHierarchy hierarchy_id , Widget w , ArgList args , Cardinal num_args ));

/* mrmwci.c */

/* extern Cardinal XmRegisterMrmCallbacks () ; */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#undef _ARGUMENTS

#endif /* MrmDecls_H */
/* DON'T ADD STUFF AFTER THIS #endif */
