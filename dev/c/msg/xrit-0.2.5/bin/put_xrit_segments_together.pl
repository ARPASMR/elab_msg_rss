#!/usr/bin/env perl

# Copyleft (C) 2010 Fabrice Ducos, ducos@loa.univ-lille1.fr

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

use strict;
use warnings;
use Carp;
use Getopt::Std;
use File::Basename;
use English;
use FileHandle; # or use IO::File;
use File::Path; # mkpath
use Time::Local; # timegm

my $appname = basename $0;
my $version = '0.1';
my $options = 'CD:v'; # with getopts
my %opt_flags;

unless (exists $ENV{TMP} && -d $ENV{TMP}) {
    die "$appname: fatal error: $appname needs a directory for decompressing the data. Usually temporary files on Unix are written to a directory\n"
    	. "specified in the TMP environment variable. Unfortunately, this TMP variable is not set on your system, or the directory it specifies\n"
        . "is not reachable (use export or setenv to define the TMP directory in your .profile or .login file, if you don't understand what it\n"
	. "means, ask your system administrator)\n";
}

my $opt_verbose = 0;
my $tmp_directory = $ENV{'TMP'} . "/put_segments_together.$$";
my $target_directory = '.';

sub display_version {
    print STDERR "$appname: version $version\n";
} # display_version

# returns the current date and time in readable form
sub now() {
    my @localtime = localtime;
    my ($cur_sec, $cur_min, $cur_hour, $cur_day, $cur_month, $cur_year) = @localtime;
    $cur_year += 1900;
    $cur_month += 1;
    my $now = sprintf "%04d-%02d-%02d %02d:%02d:%02d", $cur_year, $cur_month, $cur_day,
      $cur_hour, $cur_min, $cur_sec;
    return $now;
}

sub print_stamp($) {
    my $comment = $_[0];
    my $now = &now();
    print "$comment at $now\n";
}

sub trim {
    for (@_) {
	s/^\s+//;
	s/\s+$//;
    }
}

sub usage {
    print STDERR "usage: $appname [OPTIONS] HRIT_PROLOGUE_FILE [CHANNEL1 CHANNEL2 ...]\n\n";
    print STDERR "The first argument is the path to a HRIT PROLOGUE file.\n";
    print STDERR "If no CHANNEL is specified, all the available channels will be processed.\n";
    print STDERR "Valid values for CHANNELS: HRV IR_016 IR_039 IR_087 IR_097 IR_108 IR_120 IR_134 VIS006 VIS008 WV_062 WV_073\n";
    print STDERR "\nOPTIONS:\n\n";
    print STDERR "  -D target_directory     specifies the directory to which the output file will be written\n";
    print STDERR "  -v                      enable verbose mode\n";
    print STDERR "  -C                      compatibility with the SATMOS naming convention and the old ICARE catslot tool\n";
    print STDERR "                          (removes the underscores in the channel names and the suffix -__ in the output file)\n";
    print STDERR "                          by default, the output name will be kept as close as possible to the EUMETSAT input names\n";
    print STDERR "                          wihtout the -C option: H-000-MSG1__-MSG1________-IR_087___-CYCLE____-200608011200-__ (EUTMETSAT-like, the default)\n";
    print STDERR "                          with the -C option:    H-000-MSG1__-MSG1________-IR087____-CYCLE____-200608011200    (SATMOS-like)\n";
    print STDERR "\n";
    print STDERR "The MSG data received from antenna (XRIT format) are split into segments of 464 rows for transmission purpose\n";
    print STDERR "(8 for low resolution channels,24 for the High Resolution Visible).\n";
    print STDERR "This tool puts them together to build a single file, that can be given in input\n";
    print STDERR "to a tool like xrit2raw, which converts the data in a raw array of values, without header, easily readable in C, Fortran\n";
    print STDERR "or any other tool that can read raw binary values, such as Msphinx, a raw data viewer ( http://www-loa.univ-lille1.fr/Msphinx/ )\n";
    print STDERR "\n";
    print STDERR "Example:\n\n";
    print STDERR "Let's assume that the whole MSG data (prologue, segments, epilogue) are under directories like MSG_DATA/YYYYMMDDhhmm\n";
    print STDERR "Then all you have to do is:\n\n";
    print STDERR "put_xrit_segments_together.pl /PATH/TO/MSG_DATA/200608011200/H-000-MSG1__-MSG1________-_________-PRO______-200608011200-__ VIS006\n\n";
    print STDERR "This will create a file H-000-MSG1__-MSG1________-VIS006___-CYCLE____-200608011200-__ (in your current directory, unless you have\n";
    print STDERR "specified the -D option) and you will then be able to extract the data with a data dumper such as xrit2raw.\n";
    print STDERR "Just do :\n\n";
    print STDERR "xrit2raw -v -f H-000-MSG1__-MSG1________-VIS006___-CYCLE____-200608011200-__\n\n";
    print STDERR "It will create a file with the same name but the extension .raw\n";
    print STDERR "This file is only a raw image that can easily be viewed with raw data viewers like Msphinx, or\n";
    print STDERR "read in C (or in Fortran, but be aware that there is no record mark before and after\n";
    print STDERR "the array) : an array of 3712x3712 values of radiances (for low resolution channels) or 11136x11136 values (for HRV, the High Resolution Channel)\n";
    print STDERR "Here the values are 32-bit float (real simple precision) since the flag -f has been used (there is also a flag -d for double precision,\n";
    print STDERR "by default the original 16-bit unsigned numeric counts (proportionnal to radiances) are extracted.\n";
    print STDERR "(here the verbose flag -v will let you see the dimensions and the data type of the array that you've built, if you don't know it)\n";
    
}

sub is_available_cmd($) {
    my $cmd = $_[0];

    my $path_to_cmd = `which $cmd`;
    chomp $path_to_cmd;
    return (-x $path_to_cmd);
}

sub is_a_valid_prologue_name($) {
    my $name = $_[0];

    return ($name =~ m/H-...-MSG.__-MSG.________-_________-PRO______-............-._/);
}

sub is_a_valid_hrit_name($) {
    my $name = $_[0];

    return ($name =~ m/H-...-MSG...-MSG.........-.........-.........-............-._/);
}

use constant XRIT_SUFFIX_BAD          => 0;
use constant XRIT_SUFFIX_COMPRESSED   => 1;
use constant XRIT_SUFFIX_UNCOMPRESSED => 2;
sub xrit_compression_state($) {
    my $name = $_[0];

    my $len = length $name;
    my $suffix = substr($name, $len - 3);
    if ($suffix eq '-C_') {
	return XRIT_SUFFIX_COMPRESSED;
    }
    elsif ($suffix eq '-__') {
	return XRIT_SUFFIX_UNCOMPRESSED;
    }
    else {
	return XRIT_SUFFIX_BAD;
    }
}

sub xrit_uncompressed_file($) {
    my $name = $_[0];

    $name =~ s/-C_/-__/;
    return $name;
}

my %segments_by_channel = (
    HRV______ => 24,
    IR_016___ => 8,
    IR_039___ => 8,
    IR_087___ => 8,
    IR_097___ => 8,
    IR_108___ => 8,
    IR_120___ => 8,
    IR_134___ => 8,
    VIS006___ => 8,
    VIS008___ => 8,
    WV_062___ => 8,
    WV_073___ => 8
);

# converts a string like 'IR_016' into 'IR_016___' (field expected in HRIT filenames)
# case conversion is performed if necessary (e.g. 'ir_016' is accepted too)
sub canonize_channel($) {
    my $channel = $_[0];
    my $canonic_len = 9; # common size of all the %segment_by_channel keys
    my $channel_len = length $channel;

    if ($channel_len > $canonic_len) {
	return undef;
    }
    my $n_underscores = $canonic_len - $channel_len;
    $channel = uc($channel) . ('_' x $n_underscores);
    
    if (exists $segments_by_channel{$channel}) {
	return $channel;
    }
    else {
	return undef;
    }
}

# the reciprocal function of the previous one
# converts a string like 'IR_016___' into 'IR_016'
sub strip_channel($) {
    my $channel = $_[0];
    
    $channel =~ s/_+$//;
    return $channel;
}

# converts an EUTMETSAT-like filename (the default) into a SATMOS-like one (action of the -C option)
sub eumetsat_to_satmos_name($) {
    my $eumetsat_name = $_[0]; # must be a short name (without directory part)

    my %satmos_channel = (
	'VIS006' => 'VIS006',
	'VIS008' => 'VIS008',
	'IR_016' => 'IR016_',
	'IR_039' => 'IR039_',
	'WV_062' => 'WV062_',
	'WV_073' => 'WV073_',
	'IR_087' => 'IR087_',
	'IR_097' => 'IR097_',
	'IR_108' => 'IR108_',
	'IR_120' => 'IR120_',
	'IR_134' => 'IR134_',
	'HRV___' => 'HRV___'
	);

    my $eumetsat_channel = substr($eumetsat_name, 26, 6);
    unless (exists $satmos_channel{$eumetsat_channel}) {
	die "$appname: eumetsat_to_satmos_name: $eumetsat_name: unexpected EUMETSAT file name (channel not found at the right place), make sure that the name was given without directory part";
    }

    my $satmos_channel = $satmos_channel{$eumetsat_channel};
    my $satmos_name = substr($eumetsat_name, 0, 26) . $satmos_channel . substr($eumetsat_name, 32);
    $satmos_name =~ s/-__$//;
    return $satmos_name;
}

sub decompress(@) {
    my $decompression_tool = shift;
    my @input_files = @_;
    for (my $ifile = 0 ; $ifile < @input_files ; $ifile++) {
	my $input_file = $_[$ifile];
	if (xrit_compression_state($input_file) == XRIT_SUFFIX_COMPRESSED) {
	    my $compressed_shortname = basename($input_file);
	    my $uncompressed_shortname = xrit_uncompressed_file($compressed_shortname);
	    my $uncompressed_file = "$tmp_directory/$uncompressed_shortname";
	    mkdir $tmp_directory, 0700;
	    unless (-d $tmp_directory) {
		die "$appname: fatal error: $tmp_directory (temporary directory for decompression) couldn't be created\n";
	    }
	    my $cmd = "cp $input_file $tmp_directory && cd $tmp_directory && $decompression_tool $compressed_shortname";
	    system("$cmd");
	    $input_files[$ifile] = $uncompressed_file;
	    warn "$appname: information: decompressing $input_file -> $uncompressed_file\n" if ($opt_verbose);
	}
	elsif (xrit_compression_state($input_file) == XRIT_SUFFIX_UNCOMPRESSED) {
	    warn "$appname: information: $input_file already decompressed\n" if ($opt_verbose);
	}
	else {
	    die "$appname: fatal error: $input_file: bad compression state suffix\n";
	}
    }
    return @input_files;
}

sub cleanup {
    if (-d $tmp_directory) {
	rmtree($tmp_directory, 0, 0);
    }
}

sub interrupt {
    &cleanup;

    exit 1;
}

$SIG{INT} = \&interrupt;

########################### Main code ##########################

my $decompression_tool = 'xRITDecompress';
my $concat_tool        = 'cat';

&getopts($options, \%opt_flags); # with Getopt::Std only

die "$appname: fatal error: the command 'cat' (needed by this tool) is not available (or at least not in the known PATH) on this system (probably not a Unix)\n"
    unless (is_available_cmd($concat_tool));

die "$appname: fatal error: the decompression tool $decompression_tool is needed but is not available (or at least not in the known PATH) on this system"
	. " (NOTE: this tool is not freely distributable but can be freely obtained from Eumetsat)\n"
	unless (is_available_cmd($decompression_tool));

if ($opt_flags{v}) {
    $opt_verbose = 1;
}

if ($opt_flags{D}) {
    $target_directory = $opt_flags{D};
    unless (-d $target_directory) {
	mkpath($target_directory, 0, 0755);
    }
}

my $nargs = @ARGV;
if ( $nargs < 1) {
    &usage;
    exit 1;
}

my $prologue_file = $ARGV[0];

my @selected_channels;
if ($nargs > 1) {
    foreach (my $iarg = 1 ; $iarg < $nargs ; $iarg++) {
	my $channel_arg = $ARGV[$iarg];
	my $channel = canonize_channel($channel_arg);
	
	unless (defined $channel) {
	    die "$appname: $channel_arg: not a valid MSG channel\n";
	}
	push @selected_channels, $channel;
    }
}
else {
    @selected_channels = sort keys %segments_by_channel;
}

my $input_directory = dirname($prologue_file);
my $prologue_shortname = basename($prologue_file);

die "$appname: fatal error: $prologue_file: not a valid prologue file name\n" unless (is_a_valid_prologue_name($prologue_shortname));
die "$appname: fatal error: $prologue_file: $! (no PROLOGUE file could be found)\n" unless (-r $prologue_file);

my $epilogue_file;
($epilogue_file = $prologue_file) =~ s/PRO______/EPI______/;
die "$appname: fatal error: $epilogue_file: $! (no EPILOGUE file could be found)\n" unless (-r $epilogue_file);

foreach my $channel (@selected_channels) {

    my $segment_pattern = "$input_directory/H-???-MSG???-MSG?????????-${channel}-[0-9]?????___-????????????-?_";
    my @segment_files = sort glob($segment_pattern);
    @segment_files = decompress($decompression_tool, @segment_files);
    
    my $n_segment_files = @segment_files;
    my $n_expected_files = $segments_by_channel{$channel};
    
    if ($n_segment_files != $n_expected_files) {
	my $stripped_channel = strip_channel($channel);
	warn "$appname: error: $prologue_file: channel $stripped_channel: the number of segments found ($n_segment_files) doesn't correspond to the one expected ($n_expected_files), $stripped_channel won't be built\n";
	next;
    }
    
    my @input_files = ($prologue_file, @segment_files, $epilogue_file);
    my $output_basename;
    ($output_basename = basename($segment_files[0])) =~ s/-000001___/-CYCLE____/;
    if ($opt_flags{C}) {
        #$output_basename =~ s/-__$//;
	$output_basename = eumetsat_to_satmos_name($output_basename);
    }
    my $output_file = "$target_directory/$output_basename";
    my $cmd = "$concat_tool @input_files > $output_file";
    system($cmd) if (-d $target_directory && -f $input_files[0]); # make sure that the target directory and input files have not been erased after an interruption

} # foreach my $channel

cleanup();

exit 0;
