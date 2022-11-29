#!/usr/bin/perl

################################################################################
 # Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
 #
 # Permission is hereby granted, free of charge, to any person obtaining a
 # copy of this software and associated documentation files (the "Software"),
 # to deal in the Software without restriction, including without limitation
 # the rights to use, copy, modify, merge, publish, distribute, sublicense,
 # and/or sell copies of the Software, and to permit persons to whom the
 # Software is furnished to do so, subject to the following conditions:
 #
 # The above copyright notice and this permission notice shall be included
 # in all copies or substantial portions of the Software.
 #
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 # OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 # MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 # IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 # OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 # ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 # OTHER DEALINGS IN THE SOFTWARE.
 #
 # Except as contained in this notice, the name of Maxim Integrated
 # Products, Inc. shall not be used except as stated in the Maxim Integrated
 # Products, Inc. Branding Policy.
 #
 # The mere transfer of this software does not imply any licenses
 # of trade secrets, proprietary technology, copyrights, patents,
 # trademarks, maskwork rights, or any other form of intellectual
 # property whatsoever. Maxim Integrated Products, Inc. retains all
 # ownership rights.
 #
 ###############################################################################

my $filename = $ARGV[0];
my $git_sha = `git rev-parse HEAD`;
$git_sha =~ s/\s+//g;
open(my $fh, '>', $filename);
$now_string = localtime;  # e.g., "Thu Oct 13 04:54:34 1994"
print $fh '/* This is autogenerated. Please do not modify this file. */';
print $fh "\n";
print $fh '#ifndef _BUILD_INFO_H_';
print $fh "\n";
print $fh '#define _BUILD_INFO_H_';
print $fh "\n";
print $fh '#define __BUILD_TIME__		"' . $now_string . '"';
print $fh "\n";
print $fh '#define __BUILD_COMMIT_ID__	"' . $git_sha . '"';
print $fh "\n";
print $fh '#endif';
print $fh "\n";
close $fh;
print "\n";
print $now_string;
print "\nBuild info was written to ";
print $filename;
print "\nCommit-Id: ";
print $git_sha;
print "\n";