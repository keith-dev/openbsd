#!./perl

# $RCSfile: fork.t,v $$Revision: 1.1.1.1 $$Date: 1996/08/19 10:13:18 $

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
