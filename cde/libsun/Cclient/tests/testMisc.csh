#!/bin/csh -f
# @(#)testMisc.csh	1.12 97/06/06

set NAME=testMisc

if ( ! $?MTEST_VARS_SET_OK) then
	source testVariables
endif

setenv NETPATH ""
source setUpTmp

# Cleanup from any previous run.
/bin/rm -f A.${NAME}.out* >& /dev/null

# First time it should create the index file.
echo "${NAME} Testing various mtest functions (3, 8, C, D, E, H, I, T, U, W, Z)"
mtest -c ${TEST_DIR}/A.${NAME}
if ( $status != 0 ) then
	echo "${NAME}/#1 FAILURE"
	set testFailed=1
else
	grep -v '^MM_NOTIFY:%131 busy' A.${NAME}.out | grep -v '^MM_NOTIFY:%Parse time' | grep -v '^MM_LOG_STREAM:131' > A.${NAME}.out.1 
	diff A.${NAME}.out.1 Gold.${NAME}.1
	if ( ${status} != 0 ) then
		echo diff A.${NAME}.out.1 Gold.${NAME}.1 FAILED.
		echo "${NAME}/#1 FAILED"
		set testFailed=1
	else
		echo "${NAME}/#1 PASSED"
	endif
endif

# Now test it with UID Flags
echo "Testing Miscellaneous Again With UID Flags Set."
setenv FETCH_FLAGS FT_UID
mtest -c ${TEST_DIR}/A.${NAME}
if ( $status != 0 ) then
       	echo "${NAME}/#2 FAILURE"
       	set testFailed=1
else
	grep -v '^MM_NOTIFY:%131 busy' A.${NAME}.out | grep -v '^MM_NOTIFY:%Parse time' | grep -v '^MM_LOG_STREAM:131' > A.${NAME}.out.2 
	diff A.${NAME}.out.2 Gold.${NAME}.2
	if ( ${status} != 0 ) then
		echo diff A.${NAME}.out.2 Gold.${NAME}.2 FAILED.
		echo "${NAME}/#2 FAILED"
		set testFailed=1
	else
		echo "${NAME}/#2 PASSED"
	endif
endif

if ( $?testFailed ) then
	setenv somethingFailed=1
	echo "${NAME}/FAILED"
	unset testFailed
else
	echo "${NAME} PASSED"
endif
