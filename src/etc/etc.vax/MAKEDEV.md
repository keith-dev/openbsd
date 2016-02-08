vers(__file__,
	{-$OpenBSD: MAKEDEV.md,v 1.12 2002/12/05 04:30:21 kjc Exp $-},
etc.MACHINE)dnl
dnl
dnl Copyright (c) 2001 Todd T. Fries <todd@OpenBSD.org>
dnl All rights reserved.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl 2. The name of the author may not be used to endorse or promote products
dnl    derived from this software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
dnl INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
dnl AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
dnl THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
dnl EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
dnl PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
dnl OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
dnl WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
dnl OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
dnl ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl
dnl
dnl *** vax specific definitions
dnl
__devitem(dhu, dhu*, unibus dhu11)dnl
__devitem(dmz, dmz*, unibus dmz32)dnl
__devitem(dmf, dmf*, unibus dmf32)dnl
__devitem(dh, dh*, {-unibus dh11 and emulations (e.g. Able dmax, Emulex cs-11)-})
__devitem(vt, vt*, {-console-})dnl
__devitem(dz, dz*, unibus dz11 and dz32)dnl
__devitem(dl, dl*, unibus dl11)dnl
_mkdev(dz, dz*,
{-case $U in
       [0-7])
	       i=0
	       while [ $i -lt 8 ]; do
		       no=Add(Mult($U, 8), $i)
		       if [ $no -lt 10 ]; then
			       no="0${no}"
		       fi
		       M tty${no} c 1 $no 600
		       let i=i+1
	       done
	       ;;
	*)
	       echo bad unit for dz in: $i
	       ;;
       esac-})dnl
dnl XXX split this up abit?
_mkdev(dhu, dhu*|dmz*|dmf*|dh*|vt*,
{-set -A cnvtbl 0 1 2 3 4 5 6 7 8 9 a b c d e f g h i j k l m n o p q r s t u v
	case $i in
	vt*)    name=vt; major=68; count=8;
		case $U in
		0) ch=w ;;
		*) echo bad unit for $name in: $i ;;
		esac;;
	dmz*)   name=dmz; major=37; count=24;
		case $U in
		0) ch=a ;; 1) ch=b ;; 2) ch=c ;; 3) ch=e ;; 4) ch=f ;;
		*) echo bad unit for $name in: $i ;;
		esac;;
	dmf*)   name=dmf; major=22; count=8;
		case $U in
		0) ch=A ;; 1) ch=B ;; 2) ch=C ;; 3) ch=E ;;
		4) ch=F ;; 5) ch=G ;; 6) ch=H ;; 7) ch=I ;;
		*) echo bad unit for $name in: $i ;;
		esac;;
	dhu*)   name=dhu; major=34; count=16;
		case $U in
		0) ch=S ;; 1) ch=T ;; 2) ch=U ;; 3) ch=V ;;
		4) ch=W ;; 5) ch=X ;; 6) ch=Y ;; 7) ch=Z ;;
		*) echo bad unit for $name in: $i ;;
		esac;;
	dh*)    name=dh; major=12; count=16;
		case $U in
		0) ch=h ;; 1) ch=i ;; 2) ch=j ;; 3) ch=k ;;
		4) ch=l ;; 5) ch=m ;; 6) ch=n ;; 7) ch=o ;;
		*) echo bad unit for $name in: $i ;;
		esac;;
	esac
	i=0
	while [ $i -lt $count ]; do
		let=${cnvtbl[$i]}
		if [ -n "$let" ] ;then
			M tty${ch}${let} c $major Add(Mult($U, $count), $i) 600
		else
			echo bad count for ${name}: $U, $count, $i
		fi
		let i=i+1
	done
	;;

dl*)
	major=66
	let=${cnvtbl[$U]}
	if [ -n "$let" ] ;then
		M ttyJ${let} c $major $U 600
	else
		echo bad number for ${name}: $U
	fi-})dnl
dnl
dnl
_TITLE(make)
_DEV(all)
_DEV(std)
_DEV(loc)
_TITLE(tap)
_DEV(st, 60, 21)
_DEV(mt, 38, 15)
_DEV(ht, 5, 1)
_DEV(tm, 14, 5)
_DEV(ts, 16, 6)
_DEV(ut, 17, 10)
_TITLE(dis)
_DEV(hp, 4, 0)
_DEV(hk, 11, 3)
_DEV(up, 13, 2)
_DEV(ra, 9, 9)
_DEV(rb, 23, 11)
_DEV(rx, 30, 12)
_DEV(rl, 32, 14)
_DEV(hd, 58, 19)
_DEV(sd, 59, 20)
_DEV(cd, 61, 22)
_DEV(ch, 63)
_DEV(ss, 64)
_DEV(uk, 65)
_DEV(rd, 62, 23)
_DEV(flo, 54, 16)
_DEV(vnd, 55, 18)
_DEV(ccd, 54, 17)
_TITLE(pty)
_DEV(tty, 20)
_DEV(pty, 21)
_TITLE(prn)
_TITLE(term)
_DEV(dhu, 34)
_DEV(dh, 12)
_DEV(dmf, 22)
_DEV(dmz, 37)
_DEV(vt, 68)
_DEV(dz, 1)
_DEV(dl, 66)
_TITLE(spec)
_DEV(oppr)
_DEV(bpf, 56)
_DEV(pf, 42)
_DEV(lkm, 28)
_DEV(tun, 57)
_DEV(rnd, 67)
_DEV(mag)
_DEV(xfs, 51)
_DEV(raid, 73, 25)
_DEV(fdesc, 53)
_DEV(ses, 124)
_DEV(systrace, 49)
dnl
divert(7)dnl
dnl
ramdisk)
	_recurse std random
	_recurse fd0 sd0 sd1 rd0
	;;

_std(2, 3, 50, 7, 33)
	M ttyg0		c 25 0 600
	M ttyg1		c 25 1 600
	M ttyg2		c 25 2 600
	M ttyg3		c 25 3 600
	M crl		c 35 0 600
	M csa1		c 51 0 600
	M csa2		c 51 1 600
	M tu0		b 8 0 600
	M tu1		b 8 1 600
	M floppy	c 8 0 600
	M kUmem		c 3 3 600
	;;

ht*|tm*|mt*|ts*|ut*)
	case $i in
	ht*) n=ht b=1 c=5;;
	mt*) n=mt b=15 c=38;;
	tm*) n=tm b=5 c=14;;
	ts*) n=ts b=6 c=16;;
	ut*) n=ut b=10 c=17;;
	esac
	case $U in
	[0-7])
		four=Add($U, 4)
		eight=Add($U, 8)
		twelve=Add($U, 12)
		twenty=Add($U, 20)
		M $n$U		b $b $U	660 operator
		M $n$four	b $b $four	660 operator
		M $n$eight	b $b $eight	660 operator
		M $n$twelve	b $b $twelve	660 operator
		M n$n$U		b $b $four	660 operator;: sanity w/pdp11 v7
		M n$n$eight	b $b $twelve	660 operator;: ditto
		M nr$n$U	c $c $four	660 operator;: ditto
		M nr$n$eight	c $c $twelve	660 operator;: ditto
		M r$n$U		c $c $U		660 operator
		M r$n$four	c $c $four	660 operator
		M r$n$eight	c $c $eight	660 operator
		M r$n$twelve	c $c $twelve	660 operator
		if [ "$i" = "ut" ];
		then
			M $n$twenty	b $b $twenty 660 operator
			M r$n$twenty	c $b $twenty 660 operator
		fi
		;;
	*)
		echo bad unit for tape in: $i
		;;
	esac
	umask 77
	;;
dnl
dnl *** vax specific targets
dnl
dnl target(all, ses, 0)dnl
dnl target(all, ut, 0)dnl
dnl target(all, ch, 0)dnl
target(all, ss, 0)dnl
dnl target(all, xfs, 0)dnl
target(all, pty, 0, 1)dnl
target(all, bpf, 0, 1, 2, 3, 4, 5, 6, 7)dnl
target(all, tun, 0, 1)dnl
dnl target(all, xy, 0, 1, 2, 3)dnl
dnl target(all, hk, 0, 1, 2, 3)dnl
dnl target(all, up, 0, 1, 2, 3)dnl
dnl target(all, rd, 0)dnl
target(all, cd, 0)dnl
target(all, sd, 0, 1, 2, 3)dnl
target(all, vnd, 0)dnl
target( all, ccd, 0)dnl
target( all, hd, 0, 1, 2)dnl
target( all, mt, 0, 1)dnl
target( all, ts, 0, 1)dnl
target( all, uu, 0)dnl
target( all, st, 0, 1)dnl
target( all, dhu, 0)dnl
dnl target( all, dmz, 0)dnl
dnl target( all, dmf, 0)dnl
dnl target( all, dh, 0)dnl
target( all, dz, 0)dnl
target( all, dl, 0)dnl
target( all, vt, 0)dnl
target(ramd, fd, 0)dnl
target(ramd, sd, 0, 1, 2, 3)dnl
target(ramd, rd, 0)dnl
target(ramd, cd, 0)dnl
target(ramd, bpf, 0)dnl