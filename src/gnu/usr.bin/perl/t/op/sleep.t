#!./perl

# $RCSfile: sleep.t,v $$Revision: 1.1.1.1 $$Date: 1996/08/19 10:13:22 $

print "1..1\n";

$x = sleep 2;
if ($x >= 2 && $x <= 10) {print "ok 1\n";} else {print "not ok 1 $x\n";}
