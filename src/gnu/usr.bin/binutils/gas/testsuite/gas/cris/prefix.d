#objdump: -dr
#name: prefix

.*:     file format .*-cris

Disassembly of section \.text:
00000000 <start>:
[	 ]+0:[	 ]+0001 454a[ 	]+move\.b \[r0\+0\],r4
[	 ]+4:[	 ]+0031 564e[ 	]+move\.w \[r6=r3\+0\],r4
[	 ]+8:[	 ]+ff11 454a[ 	]+move\.b \[r1-1\],r4
[	 ]+c:[	 ]+ff01 564e[ 	]+move\.w \[r6=r0-1\],r4
[	 ]+10:[	 ]+8011 454a[ 	]+move\.b \[r1-128\],r4
[	 ]+14:[	 ]+8071 564e[ 	]+move\.w \[r6=r7-128\],r4
[	 ]+18:[	 ]+7f11 454a[ 	]+move\.b \[r1\+127\],r4
[	 ]+1c:[	 ]+7fb1 564a[ 	]+move\.w \[r11\+127\],r4
[	 ]+20:[	 ]+4f4d 0000 679e[ 	]+move\.d \[r7=r4\+0\],r9
[	 ]+26:[	 ]+4f5d 0100 454a[ 	]+move\.b \[r5\+1\],r4
[	 ]+2c:[	 ]+4f6d ffff 679e[ 	]+move\.d \[r7=r6-1\],r9
[	 ]+32:[	 ]+4f6d 80ff 679a[ 	]+move\.d \[r6-128\],r9
[	 ]+38:[	 ]+4f6d 7f00 564e[ 	]+move\.w \[r6=r6\+127\],r4
[	 ]+3e:[	 ]+5f4d 0000 679e[ 	]+move\.d \[r7=r4\+0\],r9
[	 ]+44:[	 ]+5f5d 0100 454a[ 	]+move\.b \[r5\+1\],r4
[	 ]+4a:[	 ]+5f6d ffff 679e[ 	]+move\.d \[r7=r6-1\],r9
[	 ]+50:[	 ]+5f6d 80ff 679a[ 	]+move\.d \[r6-128\],r9
[	 ]+56:[	 ]+5f6d 7f00 564e[ 	]+move\.w \[r6=r6\+127\],r4
[	 ]+5c:[	 ]+5f6d 7fff 679a[ 	]+move\.d \[r6-129\],r9
[	 ]+62:[	 ]+5f6d 8000 679a[ 	]+move\.d \[r6\+128\],r9
[	 ]+68:[	 ]+5f6d 0080 454a[ 	]+move\.b \[r6-32768\],r4
[	 ]+6e:[	 ]+5f6d ff7f 555e[ 	]+move\.w \[r5=r6\+32767\],r5
[	 ]+74:[	 ]+6f4d 0000 0000 679e[ 	]+move\.d \[r7=r4\+0( <start>)?\],r9
[	 ]+7c:[	 ]+6f5d 0100 0000 454a[ 	]+move\.b \[r5\+1( <start\+0x1>)?\],r4
[	 ]+84:[	 ]+6f6d ffff ffff 679e[ 	]+move\.d \[r7=r6\+(0xffffffff|ffffffff <end\+0xfffffe33>)\],r9
[	 ]+8c:[	 ]+6f6d 80ff ffff 679a[ 	]+move\.d \[r6\+(0xffffff80|ffffff80 <end\+0xfffffdb4>)\],r9
[	 ]+94:[	 ]+6f6d 7f00 0000 564e[ 	]+move\.w \[r6=r6\+(127|7f <start\+0x7f>)\],r4
[	 ]+9c:[	 ]+6f6d 7fff ffff 679a[ 	]+move\.d \[r6\+(0xffffff7f|ffffff7f <end\+0xfffffdb3>)\],r9
[	 ]+a4:[	 ]+6f6d 8000 0000 679a[ 	]+move\.d \[r6\+(128|80 <start\+0x80>)\],r9
[	 ]+ac:[	 ]+6f6d 0080 ffff 454a[ 	]+move\.b \[r6\+(0xffff8000|ffff8000 <end\+0xffff7e34>)\],r4
[	 ]+b4:[	 ]+6f6d ff7f 0000 555e[ 	]+move\.w \[r5=r6\+(32767|7fff <end\+0x7e33>)\],r5
[	 ]+bc:[	 ]+6f6d ff7f ffff 564e[ 	]+move\.w \[r6=r6\+(0xffff7fff|ffff7fff <end\+0xffff7e33>)\],r4
[	 ]+c4:[	 ]+6f6d 0080 0000 564a[ 	]+move\.w \[r6\+(32768|8000 <end\+0x7e34>)\],r4
[	 ]+cc:[	 ]+6f6d 0000 fbff 454a[ 	]+move\.b \[r6\+(0xfffb0000|fffb0000 <end\+0xfffafe34>)\],r4
[	 ]+d4:[	 ]+6f6d f5ad 4701 555e[ 	]+move\.w \[r5=r6\+(0x[0]?147adf5|147adf5 <end\+0x147ac29>)\],r5
[	 ]+dc:[	 ]+6f6d 0000 0080 679a[ 	]+move\.d \[r6\+(0x80000000|80000000 <end\+0x7ffffe34>)\],r9
[	 ]+e4:[	 ]+6f6d ffff ff7f 454a[ 	]+move\.b \[r6\+(0x7fffffff|7fffffff <end\+0x7ffffe33>)\],r4
[	 ]+ec:[	 ]+6f6d 0000 0000 555e[ 	]+move\.w \[r5=r6\+0( <start>)?\],r5
[ 	]+ee:[ 	]+(R_CRIS_)?32[ 	]+external_symbol
[	 ]+f4:[	 ]+4029 664e[ 	]+move\.d \[r6=r2\+\[r0\]\.b\],r4
[	 ]+f8:[	 ]+5029 454a[ 	]+move\.b \[r2\+\[r0\]\.w\],r4
[	 ]+fc:[	 ]+6029 664e[ 	]+move\.d \[r6=r2\+\[r0\]\.d\],r4
[	 ]+100:[	 ]+4a29 664e[ 	]+move\.d \[r6=r2\+\[r10\]\.b\],r4
[	 ]+104:[	 ]+5a29 454a[ 	]+move\.b \[r2\+\[r10\]\.w\],r4
[	 ]+108:[	 ]+6a29 664e[ 	]+move\.d \[r6=r2\+\[r10\]\.d\],r4
[	 ]+10c:[	 ]+422d 564a[ 	]+move\.w \[r2\+\[r2\+\]\.b\],r4
[	 ]+110:[	 ]+5b2d 555e[ 	]+move\.w \[r5=r2\+\[r11\+\]\.w\],r5
[	 ]+114:[	 ]+6a2d 564a[ 	]+move\.w \[r2\+\[r10\+\]\.d\],r4
[	 ]+118:[	 ]+4305 454a[ 	]+move\.b \[r3\+r0\.b\],r4
[	 ]+11c:[	 ]+5535 564e[ 	]+move\.w \[r6=r5\+r3\.w\],r4
[	 ]+120:[	 ]+6dd5 454a[ 	]+move\.b \[r13\+r13\.d\],r4
[	 ]+124:[	 ]+4665 564e[ 	]+move\.w \[r6=r6\+r6\.b\],r4
[	 ]+128:[	 ]+5dd5 454a[ 	]+move\.b \[r13\+r13\.w\],r4
[	 ]+12c:[	 ]+6bc5 564e[ 	]+move\.w \[r6=r11\+r12\.d\],r4
[	 ]+130:[	 ]+5545 454a[ 	]+move\.b \[r5\+r4\.w\],r4
[	 ]+134:[	 ]+4335 564e[ 	]+move\.w \[r6=r3\+r3\.b\],r4
[	 ]+138:[	 ]+6555 555e[ 	]+move\.w \[r5=r5\+r5\.d\],r5
[	 ]+13c:[	 ]+7f0d 0000 0000[ 	]+dip 0( <start>)?
[	 ]+142:[	 ]+679e[ 	]+move.d \[r7\+\],r9
[	 ]+144:[	 ]+7f0d 0100 0000 454a[ 	]+move\.b \[(0x1|1 <start\+0x1>)\],r4
[	 ]+14c:[	 ]+7f0d ffff ffff[ 	]+dip 0xffffffff
[	 ]+152:[	 ]+679e[ 	]+move.d \[r7\+\],r9
[	 ]+154:[	 ]+7f0d 80ff ffff 679a[ 	]+move\.d \[(0xffffff80|ffffff80 <end\+0xfffffdb4>)\],r9
[	 ]+15c:[	 ]+7f0d 7f00 0000[ 	]+dip (0x7f|7f <start\+0x7f>)
[	 ]+162:[	 ]+564e[ 	]+move.w \[r6\+\],r4
[	 ]+164:[	 ]+7f0d 7fff ffff 679a[ 	]+move\.d \[(0xffffff7f|ffffff7f <end\+0xfffffdb3>)\],r9
[	 ]+16c:[	 ]+7f0d 8000 0000 679a[ 	]+move\.d \[(0x80|80 <start\+0x80>)\],r9
[	 ]+174:[	 ]+7f0d 0080 ffff 454a[ 	]+move\.b \[(0xffff8000|ffff8000 <end\+0xffff7e34>)\],r4
[	 ]+17c:[	 ]+7f0d ff7f 0000[ 	]+dip (0x7fff|7fff <end\+0x7e33>)
[	 ]+182:[	 ]+555e[ 	]+move.w \[r5\+\],r5
[	 ]+184:[	 ]+7f0d ff7f ffff[ 	]+dip (0xffff7fff|ffff7fff <end\+0xffff7e33>)
[	 ]+18a:[	 ]+564e[ 	]+move.w \[r6\+\],r4
[	 ]+18c:[	 ]+7f0d 0080 0000 564a[ 	]+move\.w \[(0x8000|8000 <end\+0x7e34>)\],r4
[	 ]+194:[	 ]+7f0d 0000 fbff 454a[ 	]+move\.b \[(0xfffb0000|fffb0000 <end\+0xfffafe34>)\],r4
[	 ]+19c:[	 ]+7f0d f5ad 4701[ 	]+dip (0x[0]?147adf5|147adf5 <end\+0x147ac29>)
[	 ]+1a2:[	 ]+555e[ 	]+move.w \[r5\+\],r5
[	 ]+1a4:[	 ]+7f0d 0000 0080 679a[ 	]+move\.d \[(0x80000000|80000000 <end\+0x7ffffe34>)\],r9
[	 ]+1ac:[	 ]+7f0d ffff ff7f 454a[ 	]+move\.b \[(0x7fffffff|7fffffff <end\+0x7ffffe33>)\],r4
[	 ]+1b4:[	 ]+7f0d 0000 0000[ 	]+dip 0( <start>)?
[ 	]+1b6:[ 	]+(R_CRIS_)?32[ 	]+external_symbol
[	 ]+1ba:[	 ]+555e[ 	]+move.w \[r5\+\],r5
[	 ]+1bc:[	 ]+7a09[ 	]+dip \[r10\]
[	 ]+1be:[	 ]+664e[ 	]+move.d \[r6\+\],r4
[	 ]+1c0:[	 ]+7b09 674a[ 	]+move\.d \[\[r11\]\],r4
[	 ]+1c4:[	 ]+720d 564a[ 	]+move\.w \[\[r2\+\]\],r4
[	 ]+1c8:[	 ]+7b0d[ 	]+dip \[r11\+\]
[	 ]+1ca:[	 ]+555e[ 	]+move.w \[r5\+\],r5
