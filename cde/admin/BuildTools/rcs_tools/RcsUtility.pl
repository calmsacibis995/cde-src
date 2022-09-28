#! /usr/local/bin/perl

eval "exec /usr/local/bin/perl -S $0 $*"
    if $running_under_some_shell;

package main;

$needEncryption = 1;

#
# routine to respond to a dying server
#
sub checkForDeadServer {
	local($message) = @_;
	local($errorCode);
	if ($message =~ m%^>>### RCSSERVER EXIT CODE ([01]) ###<<$%) {
		$errorCode = $1;
		print STDERR "\n";
		while (<$SOCK_HANDLE>) {
			if (m%^>>### ERROR MESSAGE DONE ###<<$%) {
				last;
			}
			print STDERR "	$_";
		}
		print STDERR "\n";
		#
		# remove encryption directory if present
		#
		if (defined($cryptdir)) {
			if (-d $cryptdir) {
				unlink <$cryptdir/*>;
				rmdir ("$cryptdir");
			}
		}
		exit $errorCode;
	}
	return ($message);
}

#
# routine to kill client and send error messages
#
sub Exit {
	local($errorCode,@message) = @_;

	&sock'close_all;
	#
	# send error messages
	#
	foreach $error (@message) {
		print STDERR "	$error\n";
	}
	#
	# remove encryption directory if present
	#
	if (defined($cryptdir)) {
		if (-d $cryptdir) {
			unlink <$cryptdir/*>;
			rmdir ("$cryptdir");
		}
	}
	exit $errorCode;
}

	
# take the wild card files from the server and stuff them into
# the remote exchange files.
#
sub addWildCardsToRemoteFiles {
	local(*allWildCards,$needReadableFile) = @_;
	while (@allWildCards) {
		$tmpFile = shift(@allWildCards);
		$tmpFileBase = `basename $tmpFile ,v`;
		chop($tmpFileBase);
		#
		# reject if not a text file
		#
		if ( $needReadableFile && (! -r $tmpFileBase || (! -T $tmpFileBase))) {
				$exitValue = 1;
				print STDERR "	ERROR - Not readable or Not a Text File: \"$tmpFileBase\"\n";
		}
		else {
			push(@remoteExchangeFiles,$tmpFileBase);
			push(@remoteCommaVFiles,$tmpFile);
			push(@allRemoteExchangeFiles,$tmpFileBase);
			push(@allRemoteExchangeFiles,$tmpFile);
			push(@originalFiles,$tmpFileBase);
		}
	}
}

#
# subroutine to get wild cards from the server
# It then calls addWildCardsToRemoteFiles to add the wildcards to 
# remote exchange files.
#
sub getWildCardsFromServer {
	local($needReadableFile) = @_;
	local($tmpWild);
	#
	# get the complete wildcard paths from the server.
	#
	while( <$SOCK_HANDLE> ) {
		&checkForDeadServer($_);
		if (m%^>>### BEGIN EXTRA WILDCARD FILE PATHS ###<<$%) {
			while( <$SOCK_HANDLE> ) {
				&checkForDeadServer($_);
				if (m%^>>### END EXTRA WILDCARD FILE PATHS ###<<$%) {
					last;
				}
				$tmpWild = $_;
				chop($tmpWild);
				push(@allWildCards,$tmpWild);
			}
		}
		last;
	}
	#
	# get the correct local paths for wildcards and stuff into
	# remote exchange files.
	#
	if (defined(@allWildCards)) {
		&addWildCardsToRemoteFiles(*allWildCards,$needReadableFile);
	}
}

#
# check for system utilities needed by perl scripts
#
# check if certain key programs are in users path
#
sub checkForUtilities {
	local(@errorMessage);
	@absdirs = reverse grep(m!^/!, split(/:/, $ENV{'PATH'}, 999));

	@utilities = ('crypt', 'uuencode', 'basename');
	foreach $utility (@utilities) {
		$isMissingUtility = 1;	
		foreach $dir (@absdirs) {
			if (-x "${dir}/${utility}") {
				$isMissingUtility = 0;
				last;
			}
		}
		if ($isMissingUtility) {
			push(@errorMessage,"Missing system utility -> $utility.");
			push(@errorMessage,"This command is either not in your path or not on your system.");
			push(@errorMessage,"RCS Aborted");
			&Exit(1,@errorMessage);
		}
	}
}


# perl uudecode subroutine
# has the advantage of allowing specification of local file name
#
sub uudecodeP {
	local($uuFile,$cryptFile) = @_;
	local($tmpline);
	if (!open(LOCAL_UUFILE,"$uuFile")) {
		print STDERR "	Cannot open uuencoded File $uuFile\n";
		return 0;
	}
	if (!open(LOCAL_CRYPTFILE,">$cryptFile")) {
		print STDERR "	Cannot open encryption File $cryptFile\n";
		return 0;
	}
	$tmpline = <LOCAL_UUFILE>;
	while (<LOCAL_UUFILE>) {
		last if /^end/;
		next if /[a-z]/;
		next unless int((((ord() -32) & 077) +2) / 3) == int(length() /4);
		print LOCAL_CRYPTFILE unpack("u",$_);
	}
	close(LOCAL_UUFILE);
	close(LOCAL_CRYPTFILE);
	unlink "$uuFile";
	return 1;
}


#
# If RCS is a symlink to nowhere assume it is a cose directory.
# In that case, all non ,v files will be part of the remote exchange.
# Otherwise, run the local rcs commands.
# We check for symlink to nowhere by attempting to open the RCS 
# directory.
#
sub GetRcsDirPath {
	local($rcsPath,$commaVFile,$originalPath) = @_;
	local($actualRcsPath);
	if (($rcsPath =~ s%/RCS/.*$%/RCS%) || ($rcsPath =~ s%^RCS/.*$%RCS%)) {
		$actualRcsPath = $rcsPath;
		#
		# can RCS directory be opened, if it can, it is accessible
		# else, if it is a link, assume it is Remote
		#
		if (!opendir(BOGUSHANDLE,"$actualRcsPath")){ # RCS not accessible
			if (-l $actualRcsPath) {
				$areRemoteFiles = 1;
				while (defined($actualRcsPath = readlink($rcsPath))) {
					$rcsPath = $actualRcsPath;
				}
			}
			else { # not a link and not accessible, error condition
				print STDERR "\n	WARNING! $actualRcsPath directory for file $originalPath is not accessible.\n";
				print STDERR "	Only real or symlinked RCS directories can be used for source access.\n";
				print STDERR "	File $originalPath will NOT be processed.\n";
				$exitValue = 1;
			}
		}
		else { #RCS can be opened 
			$areLocalFiles = 1;
			closedir(BOGUSHANDLE);
		}
		return("${rcsPath}/${commaVFile}");
	}
	else {# not RCS in path, just ,v ; file is local
		$areLocalFiles = 1;
		return($rcsPath);
	}	
}
	



sub IsRcsDirectoryLocal {
	local($realRcsDir) = @_;
	
	if (-l $realRcsDir && (!opendir(BOGUSHANDLE,$realRcsDir))){
		$rcsDirIsLocal = 0;
		while (defined($tmpRcsDir = readlink($realRcsDir))) {
			$realRcsDir = $tmpRcsDir;
		}
	}
	else {
		$rcsDirIsLocal = 1;
		closedir(BOGUSHANDLE);
	}
	return($realRcsDir);

}

sub GetFilePaths {
	local($needReadableFile) = @_;
	local($nextFile);
	local($actualRcsPath);
	#
	# store first file
	#
	$cacheFile = shift(@allFiles);
	NEXTPATH:
	while (@allFiles) {
		$rejectFile = 0;
		$nextFile = shift(@allFiles);
		$areLocalFiles = 0;
		$areRemoteFiles = 0;
		$cacheFileBasename = `basename $cacheFile ,v`;
		chop $cacheFileBasename;
		$nextFileBasename = `basename $nextFile ,v`;
		chop $nextFileBasename;

		# 
		# if 2 consecutive files have the same basename
		# and 1 of them is a ,v, treat them as an RCS pair
		if ("$cacheFileBasename" eq "$nextFileBasename") {
			# find out which one is the ,v file and get its basename
			# both are ,v files with same basename, reject them both
			if (($cacheFile =~ /,v$/) && ($nextFile =~ /,v$/)) {
				$rejectFile = 1;
				print STDERR "\n	ERROR-> 2 files with same basename are not allowed\n";
				print STDERR "	NOT PROCESSING $cacheFile\n";
				print STDERR "	NOT PROCESSING $nextFile\n";
				# go to next file
			}
			elsif ($cacheFile =~ /,v$/) {
				$commaVFile = `basename $cacheFile`;
				chop $commaVFile;
				$normalFile = `basename $nextFile ,v`; #remove dup ,v
				chop $normalFile;
				$commaVPath = $cacheFile;
				$normalFilePath = $nextFile;
			}
			elsif ($nextFile =~ /,v$/) {
				$commaVFile = `basename $nextFile`;
				chop $commaVFile;
				$normalFile = `basename $cacheFile`;
				chop $normalFile;
				$commaVPath = $nextFile;
				$normalFilePath = $cacheFile;
			}
			else { # both not ,v files reject them
				$rejectFile = 1;
				print STDERR "\n	ERROR-> 2 files with same basename are not allowed\n";
				print STDERR "	NOT PROCESSING $cacheFile\n";
				print STDERR "	NOT PROCESSING $nextFile\n";
			}

			if ( $needReadableFile && (! -r $normalFilePath || (! -T $normalFilePath))) {
				$rejectFile = 1;
				print STDERR "	WARNING - Not readable or Not a Text File: \"$normalFilePath\"\n";
			}
				
			if ($rejectFile) {
				$exitValue = 1;
				# go to next file
				if (@allFiles) {
					$cacheFile = shift(@allFiles);
				}
				else { # no more files
					undef($cacheFile);
				}
				next NEXTPATH;
			}
			#
			# get RCS directory path and see if local RCS dir.
			$actualRcsPath = &GetRcsDirPath($commaVPath,$commaVFile,$normalFilePath);

			# store remote files in their own array
			# and local files in their own array
			# Also store original paths of both files
			if ($areRemoteFiles){
				push(@remoteExchangeFiles,$normalFile);
				push(@remoteCommaVFiles,$actualRcsPath);
				push(@allRemoteExchangeFiles,$normalFile);
				push(@allRemoteExchangeFiles,$actualRcsPath);
				# save path for restoring file if necessary(Ci, Co)
				push(@originalFiles,$normalFilePath);
			}
			elsif ($areLocalFiles) { # local
				push(@localExchangeFiles,$normalFilePath);
				push(@localExchangeFiles,$actualRcsPath);
			}

			# get the next file
			if (@allFiles) {
				$cacheFile = shift(@allFiles);
			}
			else { # no more files
				undef($cacheFile);
			}
		}
		else { #nextfile and cachefile have different basenames
			#
			# Both files have different basenames.
			# They are not of the type filename and filename,v.
			# Process the first one.
			#
			# if the file is not a ,v file and has a path greater
			# than basename (.e.g. foo/foofile), this is an 
			# rcs error. If you don't specify a ,v file, you must
			# be using files in your present directory
			#
			$commaVFile = `basename $cacheFile`;
			chop $commaVFile;
			if ($cacheFile !~ /,v$/) {
				$cacheFile =~ s%^\.\/%%; # remove any leading ./
				if ("$cacheFile" ne "$commaVFile") {
					$rejectFile = 1;
					print STDERR "\n	WARNING! Filenames without an associated RCS/filename,v command line\n";	
					print STDERR "	entry must not have a path outside of your current directory\n";
					print STDERR "	e.g. ! $origPrgName $cacheFile\n";
					print STDERR "	Sorry. Those are RCS's rules.\n";
				}
				else { # file has a local path directory is just RCS
					$normalFile = $cacheFile;
					$commaVFile = "${cacheFile},v";
					$commaVPath = "RCS/${cacheFile},v";
					$actualRcsPath = &GetRcsDirPath($commaVPath,$commaVFile,$normalFile);
					if ( $needReadableFile && (! -r $normalFile || (! -T $normalFile))) {
						$rejectFile = 1;
						print STDERR "	WARNING - Not readable or Not a Text File: \"$normalFile\"\n";
					}
				}	
				if ($rejectFile) {
					$exitValue = 1;
					# get the next file
					$cacheFile = $nextFile;
					next NEXTPATH;
				}

				if ($areRemoteFiles){
					push(@remoteExchangeFiles,$normalFile);
					push(@remoteCommaVFiles,$actualRcsPath);
					push(@allRemoteExchangeFiles,$normalFile);
					push(@allRemoteExchangeFiles,$actualRcsPath);
					# for single file case use just file name for non-,v file
					push(@originalFiles,$normalFile);
				}
				elsif ($areLocalFiles) { # local
					push(@localExchangeFiles,$normalFile);
				}
			}
			else { # cacheFile is a ,v file
				$normalFile = `basename $cacheFile ,v`; #remove dup ,v
				chop $normalFile;
				$commaVFile = `basename $cacheFile`;
				chop $commaVFile;
				$commaVPath = $cacheFile;
				# resolve the RCS dir and see if local or remote
				$actualRcsPath = &GetRcsDirPath($commaVPath,$commaVFile,$commaVPath);

				if ( $needReadableFile && (! -r $normalFile || (! -T $normalFile))) {
					$rejectFile = 1;
					print STDERR "	WARNING - Not readable or Not a Text File: \"$normalFile\"\n";
				}

				if ($rejectFile) {
					$exitValue = 1;
					# get the next file
					$cacheFile = $nextFile;
					next NEXTPATH;
				}
				# store remote files in their own array
				# and local files in their own array
				if ($areRemoteFiles){
					push(@remoteExchangeFiles,$normalFile);
					push(@remoteCommaVFiles,$actualRcsPath);
					push(@allRemoteExchangeFiles,$normalFile);
					push(@allRemoteExchangeFiles,$actualRcsPath);
					# for single file case use just file name for non-,v file
					push(@originalFiles,$normalFile);
				}
				elsif ($areLocalFiles) { # local
					push(@localExchangeFiles,$actualRcsPath);
				}
			}

			# get the next file
			$cacheFile = $nextFile;
		}
	}
	if ($cacheFile) {
		$rejectFile = 0;
		$areLocalFiles = 0;
		$areRemoteFiles = 0;
		#
		# if the file is not a ,v file and has a path greater
		# than basename (.e.g. foo/foofile), this is an 
		# rcs error. If you don't specify a ,v file, you must
		# be using files in your present directory
		#
		$commaVFile = `basename $cacheFile`;
		chop $commaVFile;
		if ($cacheFile !~ /,v$/) {
			$cacheFile =~ s%^\.\/%%; # remove any leading ./
			if ("$cacheFile" ne "$commaVFile") {
					$rejectFile = 1;
					print STDERR "\n	WARNING! Filenames without an associated RCS/filename,v command line\n";	
					print STDERR "	entry must not have a path outside of your current directory.\n";
					print STDERR "	e.g. ! $origPrgName $cacheFile\n";
					print STDERR "	Sorry. Those are RCS's rules.\n";
			}
			else { # file has a local path
				$normalFile = $cacheFile;
				$commaVFile = "${cacheFile},v";
				$commaVPath = "RCS/${cacheFile},v";
				# resolve the RCS dir and see if local or remote
				$actualRcsPath = &GetRcsDirPath($commaVPath,$commaVFile,$normalFile);

				if ( $needReadableFile && (! -r $normalFile || (! -T $normalFile))) {
					$rejectFile = 1;
					print STDERR "	WARNING - Not readable or Not a Text File: \"$normalFile\"\n";
				}

				# store remote files in their own array
				# and local files in their own array
				if (! $rejectFile) {
					if ($areRemoteFiles){
						push(@remoteExchangeFiles,$normalFile);
						push(@remoteCommaVFiles,$actualRcsPath);
						push(@allRemoteExchangeFiles,$normalFile);
						push(@allRemoteExchangeFiles,$actualRcsPath);
						# for single file case use just file name for non-,v file
						push(@originalFiles,$normalFile);
					}
					elsif ($areLocalFiles) { # local
						push(@localExchangeFiles,$normalFile);
					}
				}	
				else {
					$exitValue = 1;
				}
			}
		}
		else { # cacheFile is a ,v file
			$normalFile = `basename $cacheFile ,v`; #remove dup ,v
			chop $normalFile;
			$commaVFile = `basename $cacheFile`;
			chop $commaVFile;
			$commaVPath = $cacheFile;
			# resolve the RCS dir and see if local or remote
			$actualRcsPath = &GetRcsDirPath($commaVPath,$commaVFile,$normalFile);

			# store remote files in their own array
			# and local files in their own array
			if ( $needReadableFile && (! -r $normalFile || (! -T $normalFile))) {
				$rejectFile = 1;
				print STDERR "	WARNING - Not readable or Not a Text File: \"$normalFile\"\n";
			}

			# store remote files in their own array
			# and local files in their own array
			if (! $rejectFile) {
				if ($areRemoteFiles){
					push(@remoteExchangeFiles,$normalFile);
					push(@remoteCommaVFiles,$actualRcsPath);
					push(@allRemoteExchangeFiles,$normalFile);
					push(@allRemoteExchangeFiles,$actualRcsPath);
					# for single file case use just file name for non-,v file
					push(@originalFiles,$normalFile);
				}
				elsif ($areLocalFiles) { # local
					push(@localExchangeFiles,$actualRcsPath);
				}
			}
			else {
				$exitValue = 1;
			}
		}
	}			
}
						

#
# Check that no two files evaluate to the same basename
# (This can cause a problem on the server side)
#
sub CheckBasenames {
	$quit = 0;
	for ( @remoteCommaVFiles) {
		m%^.*/([^/]+),v$% || m%(.*),v%;
		if ( $count{$1}++ > 0 ) {
			print STDERR "At least 2 files have the same base name: \"$1\"\n";
			print STDERR  "Separate \"${prgName}\" requests are required.\n";
			$quit = 1;
		}
	}

	if ( $quit ) {
		@errorMessage = ("Aborting \"${prgName}\"");
		&Exit(1,@errorMessage);
	}
}

sub InitializeVariables {
	local(@errorMessage);
	# get effective uid for those folks who have done su2
	if (! ($userName = (getpwuid($>))[0])) {
		print STDERR "\n	unknown user name\n";
		print STDERR "	Who are you? $!";
		push(@errorMessage,"unknown user name");
		push(@errorMessage,"Who are you? $!");
		&Exit(1,@errorMessage);
	}
	chop($prgName=`basename $0`);
	$RcsPrg = "/x/toolsrc/BuildTools/rcs_tools/old/${prgName}";
	$origPrgName = $prgName;
	$prgName =~ tr/A-Z/a-z/;
	$hostname="hp";
	$Usage=" Usage: $prgName [-H host] [-P port] [RCS options] file ...\n";
	$RemoteRcsPrg = $prgName;
	$port  = 8790;

	#
	# get local host name to determine relay host
	# 
	$local_host = `uname -n`;
	chop $local_host;
	($fooname,$fooaliases,$footype,$foolen,@lcl_addr) = 
		gethostbyname($local_host);
	@local_addr = unpack('C4',$lcl_addr[0]);
	print "local_addr = @local_addr\n" if $Debug;

	#####################
	# find relay machines
	#####################
	# hp corvallis 
	if (($local_addr[0] == 15) && ($local_addr[1] == 0)) {
		if (($local_addr[2] >= 208) && ($local_addr[2] < 216)) {
				$rhost = "hpcvusx.cv.hp.com"; # hpcv xanadu
		}
		else {
			push(@errorMessage,"Unauthorized Hewlett-Packard Machine.");
			&Exit(1,@errorMessage);
		}
	}
	# hp xanadu
	elsif (($local_addr[0] == 15) && ($local_addr[1] == 255)) {
		if (($local_addr[2] == 72) && ($local_addr[3] == 18)) {
			$rhost = "cvcs1.external.hp.com"; # hpcv cvcs1
		}
		else {
			push(@errorMessage,"Unauthorized Hewlett-Packard Machine.");
			&Exit(1,@errorMessage);
		}
	}
	# novell slate
	elsif (($local_addr[0] == 147) && ($local_addr[1] == 2)) {
		if (($local_addr[2] == 128) && ($local_addr[3] == 45)) {
			$rhost = "cvcs1.external.hp.com"; # hpcv cvcs1
		}
		else {
			push(@errorMessage,"Unauthorized NOVELL Machine.");
			&Exit(1,@errorMessage);
		}
	}
	# ibm austin
	elsif (($local_addr[0] == 129) && ($local_addr[1] == 35)) {
		if (($local_addr[2] == 83) || ($local_addr[2] == 84)) {
			$rhost = "trident1.austin.ibm.com"; # austin trident1
		}
		else {
			push(@errorMessage,"Unauthorized IBM Austin Machine.");
			&Exit(1,@errorMessage);
		}
	}
	# sun palo alto
	elsif (($local_addr[0] == 129) && ($local_addr[1] == 144)) {
			$rhost = "cde-fire"; # cde-fire sun
	}
	else { # unauthorized machine
			push(@errorMessage,"Unauthorized Machine.");
			&Exit(1,@errorMessage);
	}
	&checkForUtilities();
}

1;
