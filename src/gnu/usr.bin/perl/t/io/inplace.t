#!./perl

$^I = '.bak';

# $RCSfile: inplace.t,v $$Revision: 1.1.1.1 $$Date: 1996/08/19 10:13:13 $

print "1..2\n";

@ARGV = ('.a','.b','.c');
`echo foo | tee .a .b .c`;
while (<>) {
    s/foo/bar/;
}
continue {
    print;
}

if (`cat .a .b .c` eq "bar\nbar\nbar\n") {print "ok 1\n";} else {print "not ok 1\n";}
if (`cat .a.bak .b.bak .c.bak` eq "foo\nfoo\nfoo\n") {print "ok 2\n";} else {print "not ok 2\n";}

unlink '.a', '.b', '.c', '.a.bak', '.b.bak', '.c.bak';
