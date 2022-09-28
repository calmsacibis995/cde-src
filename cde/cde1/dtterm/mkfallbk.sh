#!/bin/sh

# this file will make a source file for the fallback resources from
# the app-defaults file.

# quote all double quotes (i.e., `"' becomes `\"') and
# strip off all comments (anything after a `!')...
sed -e 's@"@\\"@g' \
    -e 's@[	 ]*!.*@@' |

# now let's convert the rest with awk...
awk '
BEGIN {
    printf "#include <Xm/Xm.h>\n\n";		# to define String...
    printf "String fallbackResources[] = {\n";	# preamble...
    contLine = 0;
    buffer = "";
}
{
    # drop blank lines... 
    if (length($0) == 0) {
	next;
    }

    # if this is not a continuation of the previous line, begin it with
    # a "...
    if (!contLine) {
	printf "    \"";
    }
	
    # if the last char is a backslash \, then it is continued on the
    # next line...
    if (substr($0, length($0), 1) == "\\") {
	contLine = 1;
    } else {
	contLine = 0;
    }

    # print out the line...
    printf "%s", $0;

    # if it is not continued on the next line, then terminate the string...
    if (!contLine) {
	printf "\",";
    }

    # print the newline...
    printf "\n";
}
END {
    # postamble...
    printf "    NULL,\n";
    printf "};\n"
}'
