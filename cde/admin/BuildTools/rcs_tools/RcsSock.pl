#!/usr/local/bin/perl

eval "exec /usr/local/bin/perl -S $0 $*"
    if $running_under_some_shell;

package sock;

;# USAGE:
;# ======
;#
;# To open a connection to a socket:
;#
;#	$handle = &sock'open($hostname, $port) || die $!;
;#	# hostname & port can each be either a name or a number
;#
;# To open and listen to a socket: (server code)
;#	$handle = &sock'listen($port) || die $!;
;#	# port can be either a name of a number.
;#
;# Read and write the same as with any other file handle:
;#
;#	print $handle "hello, socket\n";
;#	$response = <$handle>;
;#
;# To close cleanly:
;#
;#	&sock'close($handle);
;#
;# To close all open sockets, in case of an emergency exit:
;#
;#	&sock'close_all;
;#
;# AUTHOR:	David Noble (dnoble@ufo.jpl.nasa.gov)
;# DATE:	11 Feb 1993
;#
;# Modify and use as you see fit, but please leave my name on
;# it as long as it still resembles the original code.
;#
;#############################################################################

;# Get system-specific socket parameters, make assumptions if necessary.
$sockaddr_t = 'S n a4 x8';
sub sockaddr { $sockaddr_t; }

eval "require 'sys/socket.ph'";
eval <<'END_SOCKET_DEFINITIONS' if $@;
  sub AF_INET		{ 2; }
  sub SOCK_STREAM	{ 2; }
  sub SOL_SOCKET	{ 65535; }
  sub SO_REUSEADDR	{ 4; }
END_SOCKET_DEFINITIONS

;# Seed the generation of names for file handles.
$latest_handle = 'sock0000000001';

sub open {
  local ($remote_host, $remote_port) = @_;
  if (!$remote_port) {
    $! = "bad arguments to sock'open()";
    return 0;
  }
  $sock = ++$latest_handle;

  ;# Look up the port if it was specified by name instead of by number.
  if ($remote_port =~ /\D/o) {
    ($name,$aliases,$remote_port) = getservbyname($remote_port,'tcp');
  }

  ;# Look up the address if it was specified by name instead of by number.
  if ($remote_host =~ /\D/o) {
    ($name,$aliases,$type,$len,$remote_addr) = gethostbyname($remote_host);
  } else {
    $remote_addr = $remote_host;
  }

  ;# Make the socket structures.
  $this = pack($sockaddr_t, &AF_INET, 0, "\0\0\0\0");
  $remote_sock = pack($sockaddr_t, &AF_INET, $remote_port, $remote_addr);

  ;# Make the socket filehandle.
  ($name,$aliases,$proto) = getprotobyname('tcp');
  socket($sock, &AF_INET, &SOCK_STREAM, $proto) || return 0;

  ;# Set up the port so it's freed as soon as we're done.
  setsockopt($sock, &SOL_SOCKET, &SO_REUSEADDR, 1);

  ;# Bind this socket to an address.
  bind($sock, $this) || return 0;

  ;# Call up the remote socket.
  connect($sock,$remote_sock) || return 0;

  $handles{$sock} = 1;
  $oldfh = select($sock); $| = 1; select($oldfh);
  return "sock'" . $sock;
}

sub listen {
  local ($port) = @_;
  if (!$port) {
    $! = "bad arguments to sock'listen()";
    return 0;
  }
  local($sock) = ++$latest_handle;

  ($name,$aliases,$proto) = getprotobyname('tcp');
  ;# Look up the port if it was specified by name instead of by number.
  if ($port =~ /\D/o) {
    ($name,$aliases,$port) = getservbyname($port,'tcp');
  }

  ;# Make the socket structures.
  $this = pack($sockaddr_t, &AF_INET, $port, "\0\0\0\0");

  ;# Make the socket filehandle.
  socket($sock, &AF_INET, &SOCK_STREAM, $proto) || return 0;

  ;# Set up the port so it's freed as soon as we're done.
  setsockopt($sock, &SOL_SOCKET, &SO_REUSEADDR, 1);

  ;# Bind this socket to an address.
  bind($sock, $this) || return 0;
  listen($sock,5) || return 0;

  $handles{$sock} = 1;
  $oldfh = select($sock); $| = 1; select($oldfh);
  return "sock'" . $sock;
}

sub close {
  local ($sock) = shift(@_) || return 0;
  shutdown ($sock, 2);
  delete $handles{$sock};
}

sub close_all {
  for $sock (keys %handles) {
    shutdown ($sock, 2);
    delete $handles{$sock};
  }
}

1;
