#!/bin/ksh

sed -e '/^#/d' |
${CPP:-/lib/cpp} |
sed -e '/^[	 ]*$/d' \
    -e '/^#/d' \
    -e 's/[	 ][	 ]*$//' |
awk '
{
    if (NR == 1) {
	printf "%s\n", $0;
	buflen = 0;
	buffer = "";
    } else {
	if ((buflen + length($0)) >= 71) {
	    printf "\t%s\n", buffer;
	    buffer = $0" ";
	    buflen = length($0) + 1;
	} else {
	    buffer = buffer""$0" ";
	    buflen += length($0) + 1;
	}
    }
}
END {
    if (buflen > 0) {
	printf "\t%s\n", buffer;
    }
}'
