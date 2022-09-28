#!/bin/ksh

#  test.ksh
#
# this script contains an automated regression
# test of the makedepend program.  It sets up a 
# number of test input files, and then runs makedepend 
# on the test input file.  It checks the resulting
# dependencies in the resulting "make" file, and
# reports pass or fail status for each test.
#
# to debug, reduce the list of tests to the failed test,
# and remove the "rm" that blows away the test file
# after makedepend is run on the test file (see below)
#  then run makedepend by hand on the failing test case.
# 
# Or, of course, you can cut and paste test test case in 
# by hand.
#

TestMakefile=test.mk
TestInputFile=/tmp/t$$

md="./makedepend -ftest.mk"

test1()
{
     cat > /tmp/t$$ <<-END
	#define foo
	#if defined foo
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test2()
{
     cat > /tmp/t$$ <<-END
	#define foo 5
	#if defined foo
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test3()
{
     cat > /tmp/t$$ <<-END
	#define foo 5
	#if foo > 4
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test4()
{
     cat > /tmp/t$$ <<-END
	#define foo 5
	#if foo < 6
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test5()
{
     cat > /tmp/t$$ <<-END
	#define foo 5
	#define bar 6
	#if foo < bar
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test6()
{
     cat > /tmp/t$$ <<-END
	#define foo
	#if defined( foo )
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test7()
{
     cat > /tmp/t$$ <<-END
	#define foo
	#define bar
	#if defined( foo ) && defined( bar )
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test8()
{
     cat > /tmp/t$$ <<-END
	#define foo
	#if defined( foo ) && defined( bar )
	#include "bad.h"
	#else
	#include "good.h"
	#endif
END
}

test9()
{
     cat > /tmp/t$$ <<-END
	#define foo
	#if defined( bar ) || defined( foo )
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test10()
{
     cat > /tmp/t$$ <<-END
	#define bar 7
	#if defined( foo ) \\
	    || defined( bar )
	#include "good.h"
	#else
	#include "bad.h"
	#endif
END
}

test11()
{
     cat > /tmp/t$$ <<-END
	#define foo 7
	#if defined( foo ) \\
	    && defined( bar )
	#include "bad.h"
	#else
	#include "good.h"
	#endif
END
}

test12()
{
     cat > /tmp/t$$ <<-END
	#define __hp_osf
	#if defined(__apollo) || defined(_SUN_OS) || defined(__hp_osf)
	# include "good.h"
	# else
	# include "bad.h"
	# endif
END
}

rm -f good.h
rm -f bad.h
touch good.h
touch bad.h

tests="test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12"
#tests="test7 test8 test9"
#tests="test10 test11 test12"

for t in $tests
do

    # 
    # clean up the world
    #
    rm -f  $TestInputFile $TestMakefile
    touch  $TestMakefile

    # 
    # set up a test file by calling the next test function
    #
    $t

    # 
    # now run makedepend on the test file
    #
    $md    $TestInputFile > /dev/null 2>&1
    rm -f  $TestInputFile

    #
    # check to see that "good.h" is present in the resulting
    # list of dependencies
    #
    grep good $TestMakefile > /dev/null
    if [ $? -ne 0 ]; then
	echo "*** $t: failed"
    else
	echo "    $t: passed"
    fi
done


#
# clean up our mess
#
rm -f $TestMakefile TestMakefile.bak 
rm -f good.h
rm -f bad.h
