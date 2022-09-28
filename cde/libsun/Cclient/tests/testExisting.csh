#!/bin/csh -f
# @(#)testExisting.csh	1.10 97/06/06

set NAME=testExisting

if ( ! $?MTEST_VARS_SET_OK) then
	source testVariables
endif

setenv NETPATH ""
source setUpTmp

# Cleanup from any previous run.
/bin/rm -f A.${NAME}.out* >& /dev/null

# First time it should create the index file.
echo "${NAME}/#1 Testing the CREATION of the INDEX file"
mtest -c ${TEST_DIR}/A.${NAME}.1
if ( $status != 0 ) then
	echo "${NAME}/#1 FAILURE"
	set testFailed=1
else
        grep -v '^MM_NOTIFY:%131' A.${NAME}.1.out | grep -v '^MM_NOTIFY:%Parse' | grep -v '^MM_LOG_STREAM:131' > A.${NAME}.out.1
	diff A.${NAME}.out.1 Gold.${NAME}.1
	if ( ${status} != 0 ) then
		echo diff A.${NAME}.out.1 Gold.${NAME}.1 FAILED.
		echo "${NAME}/#1 FAILED"
		set testFailed=1
	else
		echo "${NAME}/#1 PASSED"
	endif
endif

if ( ! $?testFailed ) then
	if ( -f ${INDEX} ) then
		# This time the index file should exits.
		echo "${NAME}/#2 Testing the USAGE of the INDEX file"
		mtest -c ${TEST_DIR}/A.${NAME}.2
		if ( $status != 0 ) then
			echo "${NAME}/#2 FAILURE"
			set testFailed=1
		else
        		grep -v '^MM_NOTIFY:%131' A.${NAME}.2.out | grep -v '^MM_NOTIFY:%Parse' | grep -v '^MM_LOG_STREAM:131' > A.${NAME}.out.2
			diff A.${NAME}.out.2 Gold.${NAME}.2
			if ( ${status} != 0 ) then
				echo diff A.${NAME}.out.2 Gold.${NAME}.2 FAILED.
				echo "${NAME}#2 FAILED"
				set testFailed=1
			else
				echo "PASSED"
			endif		
		endif
	endif
endif

if ( $?testFailed ) then
	setenv somethingFailed 1
	echo "${NAME} FAILED"
	unset testFailed
else
	echo "${NAME} PASSED"
endif

