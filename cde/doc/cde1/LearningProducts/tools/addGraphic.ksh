#! /bin/ksh

function refresh_display
{

	clear
	print "  ___________________________________________________________________________"
	print ""
	print "                       Adding new figure to the Makefile"
	print "  ___________________________________________________________________________"
	print ""
	print -n "  Entity name:   "
if [[ $ENT_NAME != "" ]]
then
	print -n \"$ENT_NAME\"
fi
	print ""
	print ""
	print "       _________"
if [[ $MODE = "h" || $MODE = "b" ]]
then
	print "  (X)  Hardcopy:"
else
	print "  ( )  Hardcopy:"
fi
	print ""
	print "       <!-- "$HC_COMMENT" -->"
	print ""
	print -n "       Source file:  "
if [[ $HARDCOPY_SRC != "" ]]
then
	print -n \"$HARDCOPY_SRC\"
	if [[ $HC_FMT != "" ]]
	then
		case $HC_FMT in
			"p")	print -n "  (Convert to PCL";;
			"t")	print -n "  (Convert to TIFF";;
		esac
		if [[ $HC_FMT_EX != "" ]]
		then
			print -n "; expand $HC_FMT_EX"
		fi
		print -n ")"
	fi
fi
	print ""
	print ""
	print "       _______"
if [[ $MODE = "o" || $MODE = "b" ]]
then
	print -n "  (X)  Online:"
else
	print -n "  ( )  Online:"
fi
if [[ $ONLINE_VOLUME != "" ]]
then 
	if [[ $ONLINE_VOLUME = "shared" ]]
	then
		print "  (Shared)"
	else
		print "  ("$ONLINE_VOLUME")"
	fi
else
	print ""
fi
	print ""
	print "       <!-- "$OL_COMMENT" -->"
	print "    "
	print -n "       Source file:  "
if [[ $ONLINE_SRC != "" ]]
then
	print -n \"$ONLINE_SRC\"
	case $OL_FMT in
		"t")	print -n "  (Convert to TIFF)";;
		"a")	print -n "  (Use in raw form)";;
	esac
fi
	print ""
	print "  ___________________________________________________________________________"
	print ""
}


function prompt_for_values
{
while [[ $ENT_NAME = "" ]]
do
	refresh_display
	print -n "  New entity name: "
	read ENT_NAME
done

while [[ $MODE != "h" && $MODE != "o" && $MODE != "b" ]] 
do
	refresh_display
	print -n "  Build for hardcopy (h), online (o), or both (b): "
	read MODE
done

if [[ $MODE = "h" || $MODE = "b" ]]
then
	while [[ $HC_COMMENT = "" ]]
	do
		refresh_display
		print -n "  Hardcopy Comment: "
		read HC_COMMENT
	done

	while [[ $HARDCOPY_SRC = "" ]] 
	do
		refresh_display
		print -n "  Source file for hardcopy: "
		read HARDCOPY_SRC
	done

	while [[ $HC_FMT != "p" && $HC_FMT != "t" ]]
	do
		refresh_display
		print -n "  Format for needed for hardcopy, PCL (p) or TIFF (t): "
		read HC_FMT
	done
	while [[ $HC_FMT_EX = "" ]]
	do
		refresh_display
		print -n "  Pixel expansion for hardcopy: "
		read HC_FMT_EX
	done
fi


if [[ $MODE = "o" || $MODE = "b" ]]
then
	while [[ $OL_COMMENT = "" ]]
	do
		refresh_display
		print -n "  Online Comment: "
		read OL_COMMENT
	done

	while [[ $ONLINE_VOLUME = "" ]] 
	do
		refresh_display
		print -n "  Used in which volume (enter \"shared\" if more than one): "
		read ONLINE_VOLUME
	done

	while [[ $ONLINE_SRC = "" ]] 
	do
		refresh_display
		print -n "  Source file for online: "
		read ONLINE_SRC
	done

	while [[ $OL_FMT != "a" && \
		 $OL_FMT != "t" ]]
	do
		refresh_display
		print -n "  Format for needed for online, As Is (a) or TIFF (t): "
		read OL_FMT
	done
fi

}

##################################
###  Build Make Instructions   ###
##################################

function build_make_instructions
{

print "*** Building Makefile dependencies ..."

TEMP=/tmp/addGraphic.tmp

case $HC_FMT in
	"p")	HARDCOPY_DEST=${HARDCOPY_SRC%.???}.pcl;;
	"t")	HARDCOPY_DEST=${HARDCOPY_SRC%.???}.tif;;
esac
case $OL_FMT in
	"t")	ONLINE_DEST=${ONLINE_SRC%.???}.tif;;
	*)	ONLINE_DEST=$ONLINE_SRC;;
esac

print "##########   >>>  $ENT_NAME   <<<   ##########" > $TEMP
print "" >> $TEMP

print -n "$ENT_NAME:\t" >> $TEMP

case $MODE in
  "h") print "\$(HC_ENT_DIR)/$ENT_NAME" >> $TEMP;;
  "o") print "\t\t\$(OL_ENT_DIR)/$ENT_NAME" >> $TEMP;;
  "b") print "\$(HC_ENT_DIR)/$ENT_NAME\t\$(OL_ENT_DIR)/$ENT_NAME" >> $TEMP;;
esac
 
if [[ $MODE = "h" || $MODE = "b" ]]
then
	print "" >> $TEMP
	print "\$(HC_ENT_DIR)/$ENT_NAME:\t\$(HARDCOPY)/$HARDCOPY_DEST" >> $TEMP
	print "\t\$(MakeHardcopyEntity)  \"$HC_COMMENT\"" >> $TEMP

	print "" >> $TEMP
	print "\$(HARDCOPY)/$HARDCOPY_DEST:\t\$(SOURCE)/$HARDCOPY_SRC" >> $TEMP
	if [[ $HC_FMT = "p" ]]
	then
		case $HC_FMT_EX in
			"2") print "\t\$(XWD-PCL-2)" >> $TEMP;;
			"3") print "\t\$(XWD-PCL-3)" >> $TEMP;;
			"4") print "\t\$(XWD-PCL-4)" >> $TEMP;;
			*) print "\t\$(XWD-PCL-1)" >> $TEMP;;
		esac
	else
		case $HC_FMT_EX in
			"2") print "\t\$(XWD-TIFF-2)" >> $TEMP;;
			"3") print "\t\$(XWD-TIFF-3)" >> $TEMP;;
			"4") print "\t\$(XWD-TIFF-4)" >> $TEMP;;
			*) print "\t\$(XWD-TIFF-1)" >> $TEMP;;
		esac
	fi
fi

if [[ $MODE = "o" || $MODE = "b" ]]
then
	print "" >> $TEMP
	print "\$(OL_ENT_DIR)/$ENT_NAME:\t\$(ONLINE)/$ONLINE_DEST" >> $TEMP
	print "\t\$(MakeOnlineEntity)  \"$OL_COMMENT\"" >> $TEMP
	print "" >> $TEMP

	if [[ $OL_FMT = "t" ]]
	then
	     print "\$(ONLINE)/$ONLINE_DEST:\t\$(SOURCE)/$ONLINE_SRC" >> $TEMP
	     print "\t\$(XWD-TIFF-ONLINE)" >> $TEMP
	fi
fi

} ## end function build_make_instructions




#####################################################
#####################################################
###                                               ###
###   Main Body (scripts starts executing here)   ###
###                                               ###
#####################################################
#####################################################

RESPONSE=""

while [[ $RESPONSE != "s" && $RESPONSE != "a" ]]
do
	prompt_for_values
	refresh_display
	print -n "  Save new figure (s), Try again (t), Abort (a)?: "
	read RESPONSE

	if [[ $RESPONSE = "t" ]]
	then
		ENT_NAME=""
		MODE=""
		HARDCOPY_SRC=""
		HC_FMT=""
		HC_FMT_EX=""
		ONLINE_SRC=""
		OL_FMT=""
		HC_COMMENT=""
		OL_COMMENT=""
	fi
done

if [[ $RESPONSE = "s" ]]
then
  build_make_instructions
  /doc/Rivers/graphics/util/mergeMake.ksh /doc/Rivers/graphics/Makefile
  mv /doc/Rivers/graphics/Makefile /doc/Rivers/graphics/Makefile.old
  mv /doc/Rivers/graphics/Makefile.new /doc/Rivers/graphics/Makefile
  exit 0
else
  print "Aborted!"
  exit 1
fi


##  if [[ $MODE = "h" ]]
##  then
##     case $HC_FMT in
##  	"p")  print "       For hardcopy only (PCL).";;
##  	"t")  print "       For hardcopy only (TIFF).";;
##  	*)    print "       For hardcopy only.";;
##     esac
##  elif [[ $MODE = "o" ]]
##  then
##     case $OL_FMT in
##  	"x")  print "       For online only (XWD).";;
##  	"p")  print "       For online only (XPM).";;
##  	"b")  print "       For online only (XBM).";;
##  	"t")  print "       For online only (TIFF).";;
##  	*)    print "       For online only.";;
##     esac
##  	print "       For online only."
##  else
##     print -n "       For hardcopy"
##     case $HC_FMT in
##  	"p")  print -n " (PCL)";;
##  	"t")  print -n " (TIFF)";;
##  	*)    print -n "";;
##     esac
##     print -n " and online"
##     case $OL_FMT in
##  	"x")  print " (XWD).";;
##  	"p")  print " (XPM).";;
##  	"b")  print " (XBM).";;
##  	"t")  print " (TIFF).";;
##  	*)    print ".";;
##     esac
##  fi
##  




