#!/bin/csh -f
# @(#)testIndexFile.script.csh	1.5 97/05/23

set NAME=testIndexFile
set FOLDER=inboxExisting
set DIR=/tmp
set INDEX=${DIR}/.${FOLDER}_dt_index

if ( ! $?MTEST_VARS_SET_OK) then
	source testVariables
endif

setenv NETPATH ""
source setUpTmp

if ( $?testFailed ) then
	setenv somethingFailed 1
	echo "${NAME} FAILED"
	unset testFailed
	exit 1
endif

# This creates the index file we use below
/bin/rm -f A.${NAME}.2
cp A.testExisting.2 A.${NAME}.2
mtest -c A.${NAME}.2

# Remove any existing lock or index file
echo "/bin/rm -f ${DIR}/${FOLDER}.lock*"
/bin/rm -f ${DIR}/${FOLDER}.lock* >& /dev/null

# Cleanup from any previous run.
/bin/rm -f ${NAME}.out* >& /dev/null

echo "${NAME}/#1 Testing INDEX FILE"
${NAME} -f ${DIR}/${FOLDER} > ${NAME}.out
if ( $status != 0 ) then
	echo "${NAME}/#1 FAILURE"
	set testFailed=1
else
	diff ${NAME}.out Gold.${NAME}
	if ( ${status} != 0 ) then
		echo diff ${NAME}.out Gold.${NAME} FAILED.
		echo "${NAME}/#1 FAILED"
		set testFailed=1
	else
		echo "${NAME}/#1 PASSED"
	endif
endif

if ( $?testFailed ) then
	setenv somethingFailed 1
	echo "${NAME} FAILED"
	unset testFailed
else
	echo "${NAME} PASSED"
endif
