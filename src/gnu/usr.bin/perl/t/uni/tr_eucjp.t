#
# $Id: tr_eucjp.t,v 1.2 2003/12/03 03:02:49 millert Exp $
#
# This script is written intentionally in EUC-JP
# -- dankogai

BEGIN {
    if ($ENV{'PERL_CORE'}){
        chdir 't';
        unshift @INC, '../lib';
    }
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
        print "1..0 # Skip: EBCDIC\n";
        exit 0;
    }
    unless (PerlIO::Layer->find('perlio')){
        print "1..0 # Skip: PerlIO required\n";
        exit 0;
    }
    eval 'use Encode';
    if ($@ =~ /dynamic loading not available/) {
        print "1..0 # Skip: no dynamic loading, no Encode\n";
        exit 0;
    }
    $| = 1;
}

use strict;
#use Test::More qw(no_plan);
use Test::More tests => 6;
use Encode;
use encoding 'euc-jp';

my @hiragana =  map {chr} ord("��")..ord("��");
my @katakana =  map {chr} ord("��")..ord("��");
my $hiragana = join('' => @hiragana);
my $katakana = join('' => @katakana);
my %h2k; @h2k{@hiragana} = @katakana;
my %k2h; @k2h{@katakana} = @hiragana;

# print @hiragana, "\n";

my $str;

$str = $hiragana; $str =~ tr/��-��/��-��/;
is($str, $katakana, "tr// # hiragana -> katakana");
$str = $katakana; $str =~ tr/��-��/��-��/;
is($str, $hiragana, "tr// # hiragana -> katakana");

$str = $hiragana; eval qq(\$str =~ tr/��-��/��-��/);
is($str, $katakana, "eval qq(tr//) # hiragana -> katakana");
$str = $katakana; eval qq(\$str =~ tr/��-��/��-��/);
is($str, $hiragana, "eval qq(tr//) # hiragana -> katakana");

$str = $hiragana; $str =~ s/([��-��])/$h2k{$1}/go;
is($str, $katakana, "s/// # hiragana -> katakana");
$str = $katakana; $str =~ s/([��-��])/$k2h{$1}/go;
is($str, $hiragana, "s/// # hiragana -> katakana");
__END__