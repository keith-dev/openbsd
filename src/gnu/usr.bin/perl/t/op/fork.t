#!./perl

# $RCSfile: fork.t,v $$Revision: 1.2 $$Date: 1997/11/30 08:05:21 $

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    unless ($Config{'d_fork'}) {
	print "1..0\n";
	exit 0;
    }
}

$| = 1;
print "1..2\n";

if ($cid = fork) {
    sleep 2;
    if ($result = (kill 9, $cid)) {print "ok 2\n";} else {print "not ok 2 $result\n";}
}
else {
    $| = 1;
    print "ok 1\n";
    sleep 10;
}
