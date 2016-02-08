#!./perl

# $RCSfile: sleep.t,v $$Revision: 1.5 $$Date: 2001/05/24 18:36:22 $

print "1..1\n";

$x = sleep 3;
if ($x >= 2 && $x <= 10) {print "ok 1\n";} else {print "not ok 1 $x\n";}
