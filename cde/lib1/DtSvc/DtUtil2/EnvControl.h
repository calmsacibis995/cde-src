/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/****************************<+>*************************************
 **
 **   File:     EnvControl.h
 **
 **   RCS:	$XConsortium: EnvControl.h /main/cde1_maint/1 1995/07/14 20:33:09 drk $
 **   Project:  HP DT Runtime Library
 **
 **   Description: Defines structures, and parameters used
 **                for communication with the environment
 **
 **   (c) Copyright 1990 by Hewlett-Packard Company
 **
 ****************************<+>*************************************/

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO
extern int _DtEnvControl() ;
extern int _DtEnvRemove() ;
extern int _DtWsmSetBackdropSearchPath() ;
#else
#ifdef __cplusplus
extern "C" {
#endif
extern int _DtEnvControl(
                        int mode) ;
extern int _DtEnvRemove(
                        char *str,
                        int length) ;
extern int _DtWsmSetBackdropSearchPath(
                        Screen *screen,
                        char   *backdropDir,
                        Boolean useMultiColorIcons) ;
#ifdef __cplusplus
}
#endif
#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#define DT_ENV_DEBUG 1
/********    Debug facilities  ********/
#ifdef DT_ENV_DEBUG
#ifdef _NO_PROTO
extern int _DtEnvPrint() ;
#else
extern int _DtEnvPrint( void ) ;
#endif /* _NO_PROTO */
#endif /* DT_ENV_DEBUG */

/********    End Debug facilities  ********/
/**********************************************************************
 * Command parameters to the function, which double as result codes.
 * If the invocation is successful, the same is returned; 
 * else DT_ENV_NO_OP is returned.

DT_ENV_SET
	Sets the HP DT environment.

DT_ENV_RESTORE_PRE_DT
	Restores the pre-HP DT application environment

DT_ENV_RESTORE_POST_DT
	Reinstalls the HP DT environment after a restoring pre-DT
	environment

DT_ENV_NO_OP
	Does nothing

DT_ENV_SET_BIN
	Sets the HP DT environment PLUS sets the PATH= variable to
	where the HP DT files live.

 **********************************************************************/
#define DT_ENV_SET			0
#define DT_ENV_RESTORE_PRE_DT		1
#define DT_ENV_RESTORE_POST_DT	2
#define DT_ENV_NO_OP			3
#define DT_ENV_SET_BIN			4

/****************************        eof       **********************/
