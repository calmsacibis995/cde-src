#!/bin/csh -f
# @(#)testEverything.csh	1.12 97/06/03

source testVariables
if ($exitCode != 0 ) then
	exit 1
endif

echo testExisting
source testExisting

if ( -x testExisting.imapWSU40 ) then
	echo testExisting.imapWSU40
	source testExisting.imapWSU40
else
	echo ""
	echo "No 'testExisting.imapWSU40' - skipping test"
	echo ""
endif

if ( -x testExisting.imapWSU41 ) then
	echo testExisting.imapWSU41
	source testExisting.imapWSU41
else
	echo ""
	echo "No 'testExisting.imapWSU41' - skipping test"
	echo ""
endif

if ( -x testExisting.imapSIM10 ) then
	echo testExisting.imapSIMS10
	source testExisting.imapSIMS10
else
	echo ""
	echo "No 'testExisting.imapSIMS10' - skipping test"
	echo ""
endif

if ( -x testExisting.imapSIM20 ) then
	echo testExisting.imapSIMS20
	source testExisting.imapSIMS20
else
	echo ""
	echo "No 'testExisting.imapSIMS20' - skipping test"
	echo ""
endif


echo testNewMail
source testNewMail

echo testIndexFile.script
source testIndexFile.script

echo testMisc
source testMisc

echo testMisc.imapWSU41
source testMisc.imapWSU41

echo testSmtp
source testSmtp

echo -n "Overall results - $0 "
if ( $?somethingFailed ) then
	echo "$0 FAIL"
	exit 1
endif
echo "$0 PASS"

exit 0
