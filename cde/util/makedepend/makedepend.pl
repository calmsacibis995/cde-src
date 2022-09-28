#!/usr/local/bin/perl

$objfile = ".o";
$startat = "# DO NOT DELETE THIS LINE -- make depend depends on it.";
$width = 78;
$append = 0;
$verbose = 0;
$show_multiple_includes = 0;
$makefile = "Makefile";
$cppcommand = "cc -E";

do parse_command_line();
do copy_makefile();
do check_include_dirs();
do make_depends();

sub check_include_dirs
{
    foreach $directory (@includelist)
    {
	if (! -d $directory)
	{
	    print STDERR "$0: warning: Include directory $directory does ",
			 "not exist\n";
	}
    }
}

sub make_depends
{
    $cpplist = "";
    foreach $cppitem (@cpplist)
    {
	$cpplist .= "$cppitem ";
    }
    foreach $filename (@filelist)
    {

	$objname = $filename;
	$objname =~ s|.*/||;
	$objname =~ s|\.[^\.]*$||;
	$objname .= $objfile;
	open(cppoutput, "$cppcommand $cpplist$filename 2>/tmp/md$$|");
	print STDERR "...$filename...\n";
	do process_cppoutput();
	close(cppoutput);
	do process_cpperrors();
    }
}

sub process_cpperrors
{
    open(cpperror, "/tmp/md$$");
    unlink("/tmp/md$$");
    while ($line = <cpperror>)
    {
	if (($f1, $f2, $f3) =
  ($line =~ m/^([^:]+):\s([^:]+):\sUnable to find include file\s\'([^\']+)\'/))
	{
	    print STDERR "$0: warning in $filename: $f1(line #$f2) cannot ",
			 "find include file '$f3'\n";
	}
	else
	{
	    print STDERR $line;
	}
    }
    close(cpperror);
}

sub process_cppoutput
{
    undef %modules;
    undef @modules;
    undef %includestack;
    undef @includestack;
    while ($line = <cppoutput>)
    {
	if (substr($line, 0, 4) eq "# 1 ")
	{
	    $line =~ s/.*\"(.*)\"\s*\n/$1/;
	    if (defined($modules{$line}))
	    {
		if (!$modules{$line} && $show_multiple_includes)
		{
		    print STDERR "$0: warning: $filename includes $line ",
				 "more than once!\n",
				 "First include sequence:\n",
				 "$includestack{$line}\t$line\n",
				 "Second include sequence:\n";
		    foreach $module (@includestack)
		    {
			print STDERR "\t$module ->\n";
		    }
		    print STDERR "\t$line\n";
		}
		$modules{$line} = 1;
	    }
	    else
	    {
		$modules{$line} = 0;
		foreach $item (@includestack)
		{
		    $includestack{$line} .= "\t$item ->\n";
		}
		push (@modules, $line);
	    }
	    push (@includestack, $line);
	}
	elsif ($line =~ s/^# \d+\s+\"(.*)\"\s*\n/$1/)
	{
	    pop (@includestack);
	}
    }
    $outputline = "";
    foreach $module (@modules)
    {
	do append_outputline();
    }
    if (length($outputline))
    {
	do tomakefile();
    }
}

sub append_outputline
{
    if (length($outputline) != 0)
    {
	if (length($outputline) + length($module) + 1 > $width)
	{
	    do tomakefile();
	}
	else
	{
	    $outputline .= " $module";
	    return;
	}
    }
    $outputline = "$objname: $module";
}

sub tomakefile
{
    print outstream $outputline, "\n";
}

sub parse_command_line
{
    $endmarker = 0;
    while ($#ARGV + 1)
    {
	if ($endmarker && $endmarker eq $ARGV[0])
	{
	    $endmarker = 0;
	}
	elsif (($firstchar = substr($ARGV[0], 0, 1)) ne "-")
	{
	    if ($endmarker && $firstchar eq "+")
	    {
		push(@cpplist, ($ARGV[0]));
	    }
	    else
	    {
		push(@filelist, ($ARGV[0]));
	    }
	}
	else
	{
	    $firstchar = substr($ARGV[0], 1, 1);
	    if ($firstchar eq "-")
	    {
		$endmarker = substr($ARGV[0], 2);
		if (length($endmarker) == 0)
		{
		    $endmarker = $ARGV[0];
		}
	    }
	    elsif ($firstchar eq "I")
	    {
		push(@cpplist, ($ARGV[0]));
		$temp = substr($ARGV[0], 2);
		if (!defined($includelist{$temp}))
		{
		    $includelist{$temp} = 1;
		    push(@includelist, $temp);
		}
	    }
	    elsif ($firstchar eq "a" && !$endmarker)
	    {
		$append = 1;
	    }
	    elsif ($firstchar eq "m" && !$endmarker)
	    {
		$show_multiple_includes = 1;
	    }
	    elsif ($firstchar eq "c" && !$endmarker)
	    {
		$cppcommand = &getarg();
	    }
	    elsif ($firstchar eq "w" && !$endmarker)
	    {
		$width = &getarg() + 0;
	    }
	    elsif ($firstchar eq "o" && !$endmarker)
	    {
		$objfile = &getarg();
	    }
	    elsif ($firstchar eq "v" && !$endmarker)
	    {
		$verbose = 1;
	    }
	    elsif ($firstchar eq "s" && !$endmarker)
	    {
		$startat = &getarg();
		if (substr($startat, 0, 1) ne "#")
		{
		    do fatal("-s flag's value should start with '#'");
		}
	    }
	    elsif ($firstchar eq "f" && !$endmarker)
	    {
		$makefile = &getarg();
	    }
	    elsif ($firstchar eq "?" && !$endmarker)
	    {
		do usage();
	    }
	    else
	    {
		push(@cpplist, ($ARGV[0]));
	    }
	}
	shift @ARGV;
    }
}

sub getarg
{
    if (length($temp = substr($ARGV[0], 2)) == 0)
    {
	if ($#ARGV <= 0)
	{
	    do usage();
	}
	shift @ARGV;
	$ARGV[0];
    }
    else
    {
	$temp;
    }
}

sub usage
{
    print STDERR "Usage: $0 [-a] [-c<cc_command>] [-f<makefile>] ",
		 " [-m]\n",
		 " " x (8 + length($0)),
		 "[-o<objsuffix>] [-s<string>] [-v] [-w<width>]\n\n",
		 "    -a: Append to makefile\n",
		 "    -m: Show multiple inclusions\n",
		 "    -v: Verbose\n";
    exit 1;
}

sub fatal
{
    print STDERR "$0 : @_\n";
    exit 1;
}

sub copy_makefile
{
    if (!(-e $makefile))
    {
	do fatal("File '$makefile' does not exist");
    }
    $temp = "$makefile.bak";
    if (-e $temp && (unlink($temp) != 1))
    {
	do fatal("Cannot unlink file '$temp'");
    }
    if (rename($makefile, $temp) != 1)
    {
	do fatal("Cannot rename '$makefile' to '$temp'");
    }
    if (!open(instream, "<$temp"))
    {
	do fatal("Cannot open '$temp' for reading");
    }
    if (!open(outstream, ">$makefile"))
    {
	do fatal("Cannot open '$makefile' for writing");
    }
    while ($line = <instream>)
    {
	print outstream $line;
	if ($append)
	{
	    next;
	}
	chop $line;
	if ($line eq $startat)
	{
	    last;
	}
    }
    if (!$append && !$line)
    {
	print outstream "\n# --------------------------------------------",
	"-----------------------------\n",
	"# dependencies generated by makedepend\n\n$startat\n";
    }
    printf outstream "\n";
    close instream;
}
