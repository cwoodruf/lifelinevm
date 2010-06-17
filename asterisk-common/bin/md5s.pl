#!/usr/bin/perl
use Lifeline::DB;
use strict;
for (my $i = 0; $i < 10000; $i++) {
	my $code = sprintf('%04d', $i);
	$ldb->do("insert into md5s (code,hash) values('$code',md5('$code'))") 
		or die $ldb->errmsg;
}
