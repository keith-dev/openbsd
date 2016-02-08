#	$OpenBSD: localcommand.sh,v 1.1 2007/10/29 06:57:13 dtucker Exp $
#	Placed in the Public Domain.

tid="localcommand"

echo 'PermitLocalCommand yes' >> $OBJ/ssh_proxy
echo 'LocalCommand echo foo' >> $OBJ/ssh_proxy

for p in 1 2; do
	verbose "test $tid: proto $p localcommand"
	a=$(${SSH} -F $OBJ/ssh_proxy -$p somehost true)
	if [ "$a" != "foo" ] ; then
		fail "$tid proto $p"
	fi
done
