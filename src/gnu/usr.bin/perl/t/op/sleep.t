#!./perl

# $RCSfile: sleep.t,v $$Revision: 1.3 $$Date: 1999/04/29 22:52:38 $

print "1..1\n";

$x = sleep 3;
if ($x >= 2 && $x <= 10) {print "ok 1\n";} else {print "not ok 1 $x\n";}
