#!/bin/sh
# runs each named file through the sed scripts, 
# after renaming original to file.predis

# set to directory containing sed scripts
progpath=`dirname $0`   

for f in $*
do
	mv $f $f.predis
	echo === converting file $f - original in $f.predis ===
	exec sed -f $progpath/todis1.sed "$f.predis" | \
	sed -f $progpath/todis2.sed | \
	sed -f $progpath/todis3.sed | \
	sed -f $progpath/todis4.sed | \
	sed -f $progpath/todis5.sed | \
	sed -f $progpath/todis6.sed > $f
done
