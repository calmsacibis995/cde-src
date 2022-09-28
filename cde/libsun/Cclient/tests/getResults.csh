#! /bin/csh -f

if ( $#argv != 1 ) then
	echo "Usage $0 <tcov.out-file>"
	exit 1
endif

if ( ! -f $1 ) then
	echo "$1 does not exist\!"
	exit 2
endif

if ( ! -d results ) then
	echo "Sorry, not 'results' directory - nothing to do."
endif

/bin/grep "Basic blocks in this file" $1 | /bin/wc | /bin/awk '{print $1}' > results/files
/bin/grep "Basic blocks in this file" $1 | /bin/awk '{print $1}' > results/blocks
/bin/grep "Basic blocks executed" $1 | /bin/awk '{print $1}' > results/blocksExecuted
/bin/grep "Percent of the file executed" $1 | /bin/awk '{print $1}' > results/percent
/bin/grep "Total basic block executions" $1 | /bin/awk '{print $1}' > results/executions
/bin/grep "Average executions per basic block" $1 | /bin/awk '{print $1}' > results/averageExPerBlock


set fileCount=`cat results/files`
/bin/echo "${fileCount} Files profiled";

set foo=`/bin/cat results/blocks`
set totalBlocks=`/bin/echo $foo |sed 's/ /+/g'| /bin/bc`
/bin/echo "$totalBlocks Total blocks."

set foo=`/bin/cat results/blocksExecuted`
set totalBlocksExecuted=`/bin/echo $foo |sed 's/ /+/g'| /bin/bc`
/bin/echo "$totalBlocksExecuted Blocks executed"

set perBlocksExecuted=`/bin/echo "scale=3 ; ($totalBlocksExecuted / $totalBlocks) * 100" | /bin/bc`
/bin/echo "$perBlocksExecuted Percent of blocks executed."

set foo=`/bin/cat results/executions`
set totalExecutions=`/bin/echo $foo |sed 's/ /+/g'| /bin/bc`
/bin/echo "$totalExecutions Total block executions."

set foo=`/bin/cat results/averageExPerBlock`
set PerBlockTotal=`/bin/echo $foo |sed 's/ /+/g'| /bin/bc`


set avPerBlockTotal=`/bin/echo "scale=3 ; ($PerBlockTotal / $totalBlocks) * 100" | /bin/bc`
/bin/echo "$avPerBlockTotal Average executions per basic block."
