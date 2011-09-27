#!/usr/bin/perl
# purpose of this script is to get an initial superuser for the db

use lib {/usr/local/asterisk/var/lib/asterisk/agi-bin};
use Lifeline::DB;
use Term::ReadKey;
use strict;

# login  password  created  vid  perms  notes

