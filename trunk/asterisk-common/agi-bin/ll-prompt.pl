#!/usr/bin/perl
# purpose of this script is to load the current prompt into the prompt variable
use Asterisk::AGI;
use strict;
my $basedir = "/usr/local/apache2/htdocs/lifeline/prompts";
my $a = Asterisk::AGI->new();
our $prompt;
if (do "$basedir/current") {
	print STDERR "prompt is $prompt\n";
	my $lang = ($prompt=~/^ll-(\w+)-/)[0];
	mkdir "$basedir/$lang",0777 or die "can't make $basedir/$lang: $!" 
		unless -d "$basedir/$lang";
	$a->set_variable('prompt',"$basedir/$lang/$prompt");
	my $stub = ($prompt=~/(.*)\.\w+$/)[0];
	$a->set_variable('prompt_stub',"$basedir/$lang/$stub");
	$a->set_priority(100);
} else {
	print STDERR "no prompt specified!\n";
}
