#!/bin/csh -f
# @(#)testVariables.csh	1.6 97/06/03

setenv IMAP_ACCOUNT abcde12345

setenv exitCode 0

if ( ! $?MTEST_VARS_SET_OK ) then

	if ( ! $?NFS_DIRECTORY ) then
		echo "NFS_DIRECTORY is not set. This is needed"
		echo "to test NFS mounted non-IMAP folders."
		echo "Please set the environmental variable NFS_DIRECTORY"
		echo "to a directory that is NFS mounted and where there"
		echo "is RWX to the user."
		setenv exitCode 1
		setenv somethigFailed 1
		exit 4
	endif

	isNFS $NFS_DIRECTORY
	if ($status != 0) then
		setenv exitCode 1
		setenv somethigFailed 1
		exit 5
	endif
	
	# If TEST_DIR not set, try to find it.
	if ( ! $?TEST_DIR ) then
		if ( -f testVariables.csh ) then
			setenv TEST_DIR `pwd`
		endif

		if ( -d c-client ) then
			if ( -x c-client/mtest ) then
				setenv TEST_DIR `pwd'/tests
			endif
		endif

		if ( -x mtest ) then
			setenv TEST_DIR `pwd'/../tests
		endif
	endif

	# Verify it is valid or found
	if ( ! $?TEST_DIR ) then
		echo "TEST_DIR environmental variable not set\!"
		setenv exitCode 1
		setenv somethigFailed 1
		exit 1
	endif

	#
	echo "Using '${TEST_DIR}' as location of test scripts."

	# Validate it.
	if ( ! -d ${TEST_DIR}/../c-client ) then
		echo "Can not find directory '${TEST_DIR}/c-client'."
		setenv exitCode 1
		setenv somethigFailed 1
		exit 2
	endif

	if ( ! -x ${TEST_DIR}/../c-client/mtest ) then
		echo "Can not find '${TEST_DIR}/../c-client/mtest' program"
		setenv exitCode 1
		setenv somethigFailed 1
		exit 3
	endif

	echo "Using '${TEST_DIR}/../c-client/mtest' as mtest program"

	set path = ( ${TEST_DIR}/../c-client $path)
	setenv LD_LIBRARY_PATH ${TEST_DIR}/../c-client

	setenv MTEST_VARS_SET_OK 1

	setenv LOCAL_HOST_NAME `uname -n`

endif

echo ""

