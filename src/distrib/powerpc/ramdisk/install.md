#	$OpenBSD: install.md,v 1.3 1998/04/06 20:27:29 pefo Exp $
#
#
# Copyright rc) 1996 The NetBSD Foundation, Inc.
# All rights reserved.
#
# This code is derived from software contributed to The NetBSD Foundation
# by Jason R. Thorpe.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#        This product includes software developed by the NetBSD
#        Foundation, Inc. and its contributors.
# 4. Neither the name of The NetBSD Foundation nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

#
# machine dependent section of installation/upgrade script.
#

# Machine-dependent install sets
MDSETS="kernel"

md_set_term() {
	if [ ! -z "$TERM" ]; then
		return
	fi
	echo -n "Specify terminal type [vt100]: "
	getresp "vt100"
	TERM="$resp"
	export TERM
}

md_makerootwritable() {
}

md_machine_arch() {
	cat /kern/machine
}

md_get_diskdevs() {
	# return available disk devices
	cat /kern/msgbuf | egrep "^[sw]d[0-9] " | cut -d" " -f1 | sort -u
}

md_get_cddevs() {
	# return available CDROM devices
	cat /kern/msgbuf | egrep "^a?cd[0-9] " | cut -d" " -f1 | sort -u
}

md_get_ifdevs() {
	# return available network devices
	cat /kern/msgbuf | egrep "^?de[0-9] " | cut -d" " -f1 | sort -u
}

md_get_partition_range() {
    # return range of valid partition letters
    echo "[a-p]"
}

md_installboot() {
	echo "Installing boot in the msdos partition /dev/${1}i"
	if mount -t msdos /dev/${1}i /mnt2 ; then
		cp /usr/mdec/ofwboot /mnt2
		umount /mnt2
	else
		echo "Failed, you will not be able to boot from /dev/${1}."
	fi
}

md_native_fstype() {
    echo "msdos"
}

md_native_fsopts() {
    echo "ro"
}

md_init_mbr() {
	# $1 is the disk to init
	echo
	echo "You will now be asked if you want to initialize the disk with a 1Mb"
	echo "MSDOS partition. This is the recomended setup and will allow you to"
	echo "store the boot and other interesting things here."
	echo
	echo "If you want to have a different setup, exit 'install' now and do"
	echo "the MBR initialization by hand using the 'fdisk' program."
	echo
	echo "If you choose to manually setup the MSDOS partition, "
	echo "consult your PowerPC OpenFirmware manual -and- the"
	echo "PowerPC BSD Installation Guide for doing setup this way."
	echo
	echo -n "Do you want to init the MBR and the MSDOS partition? [y]"
	getresp "y"
	case "$resp" in
	n*|N*)
		exit 0;;
	*)
		echo
		echo "A MBR record with an OpenBSD usable partition table will now be copied"
		echo "to your disk. Unless you have special requirements you will not need"
		echo "to edit this MBR. After the MBR is copied an empty 1Mb MSDOS partition"
		echo "will be created on the disk. You *MUST* setup the OpenBSD disklabel"
		echo "to have a partition include this MSDOS partition."
		echo "You will have an opportunity to do this shortly."
		echo
		echo "You will probably see a few '...: no disk label' messages"
		echo "It's completly normal. The disk has no label yet."
		echo "This will take a minute or two..."
		sleep 2
		echo -n "Creating Master Boot Record (MBR)..."
		dd if=/usr/mdec/mbr of=/dev/r$1c >/dev/null 2>&1
		echo "..done."
		echo -n "Copying 1MB MSDOS partition to disk..."
		gunzip < /usr/mdec/msdos1mb.gz | dd of=/dev/r$1c bs=512 seek=1 >/dev/null 2>&1
		echo "..done."
	;;
	esac
}

md_checkfordisklabel() {
	# $1 is the disk to check
	local rval

	echo
	echo "Power.4e systems need a MBR and MSDOS partition on the boot disk."
	echo "This is necessary because the OpenFirmware doesn't know about"
	echo "OpenBSD, or how to boot the system from a BSD partition."
	echo
	echo "Install will put a boot program with the name 'ofwboot' "
	echo "that you will configure OpenFirmware to use when booting OpenBSD."
	echo
	echo -n "Have you initialized a MSDOS partition using OpenFirmware? [n]"
	getresp "n"
	case "$resp" in
	n*|N*)
		md_init_mbr $1;;
	*)
		echo
		echo "You may keep your current setup if you want to be able to use any"
		echo "already loaded OS. However you will be asked to prepare an empty"
		echo "partition for OpenBSD later. There must also be at least ~0.5Mb free space"
		echo "in the boot partition to hold the OpenBSD kernel boot."
		echo
		echo "Also note that the boot partition must be included as partition"
		echo "'i' in the OpenBSD disklabel."
		echo
		echo -n "Do You want to keep the current MSDOS partition setup? [y]"
		getresp "y"
		case "$resp" in
		n*|N*)
			md_init_mbr $1;;
		*)
		;;
		esac
	;;
	esac


	disklabel -r $1 > /dev/null 2> /tmp/checkfordisklabel
	if grep "no disk label" /tmp/checkfordisklabel; then
		rval=1
	elif grep "disk label corrupted" /tmp/checkfordisklabel; then
		rval=2
	else
		rval=0
	fi

	rm -f /tmp/checkfordisklabel
	return $rval
}

md_prep_fdisk()
{
	local _disk
	local _done

	_disk=$1
	echo
	echo "This disk has not previously been used with OpenBSD. You may share"
	echo "this disk with other operating systems. However, to be able to boot"
	echo "the system you will need a small DOS partition in the begining of"
	echo "the disk to hold the kernel boot. OpenFirmware understands"
	echo "how to read an MSDOS style format from the disk."
	echo
	echo "This DOS style partitioning has been taken care of if"
	echo "you choosed to do that initialization earlier in the install."
	echo
	echo "WARNING: Wrong information in the BIOS partition table might"
	echo "render the disk unusable."

	echo -n "Press [Enter] to continue "
	getresp ""

	echo
	echo "Current partition information is:"
	fdisk ${_disk}
	echo -n "Press [Enter] to continue "
	getresp ""

	_done=0
	while [ $_done = 0 ]; do
		echo
		cat << \__md_prep_fdisk_1

An OpenBSD partition should have type (i.d.) of 166 (A6), and should be the
only partition marked as active. Also make sure that the size of the partition
to be used by OpenBSD is correct, otherwise OpenBSD disklabel installation
will fail. Furthermore, the partitions must NOT overlap each others.

The fdisk utility will be started update mode (interactive.)
You will be able to add / modify this information as needed.
If you make a mistake, simply exit fdisk without storing the new
information, and you will be allowed to start over.
__md_prep_fdisk_1
		echo
		echo -n "Press [Enter] to continue "
		getresp ""

		fdisk -e ${_disk}

		echo
		echo "The new partition information is:"
		fdisk ${_disk}

		echo
		echo "(You will be permitted to edit this information again.)"
		echo "-------------------------------------------------------"
		echo -n "Is the above information correct? [n] "
		getresp "n"

		case "$resp" in
		n*|N*) ;;
		*) _done=1 ;;
		esac
	done

	echo
	echo "Please take note of the offset and size of the OpenBSD partition"
	echo "*AND* the MSDOS partitions you may want to access from OpenBSD."
	echo "At least the MSDOS partition used for booting must be accessible"
	echo "by OpenBSD as partition 'i'. You may need this information to "
	echo "fill in the OpenBSD disk label later."
	echo -n "Press [Enter] to continue "
	getresp ""
}

md_prep_disklabel()
{
	local _disk

	_disk=$1
	md_checkfordisklabel $_disk
	case $? in
	0)
		echo -n "Do you wish to edit the disklabel on $_disk? [y]"
		;;
	1)
		md_prep_fdisk ${_disk}
		echo "WARNING: Disk $_disk has no label"
		echo -n "Do you want to create one with the disklabel editor? [y]"
		;;
	2)
		echo "WARNING: Label on disk $_disk is corrupted"
		echo -n "Do you want to try and repair the damage using the disklabel editor? [y]"
		;;

	esac

	getresp "y"
	case "$resp" in
	y*|Y*) ;;
	*)	return ;;
	esac

	# display example
	cat << \__md_prep_disklabel_1

Disk partition sizes and offsets are in sector (most likely 512 bytes) units.
You may set these size/offset pairs on cylinder boundaries 
     (the number of sector per cylinder is given in )
     (the `sectors/cylinder' entry, which is not shown here)
Also, you *must* make sure that the 'i' partition points at the MSDOS
partition that will be used for booting. The 'c' partition shall start
at offset 0 and include the entire disk. This is most likely correct when
you see the default label in the editor.

Do not change any parameters except the partition layout and the label name.

   [Here is an example of what the partition information may look like.]
10 partitions:
#        size   offset    fstype   [fsize bsize   cpg]
  a:   120832    10240    4.2BSD     1024  8192    16   # (Cyl.   11*- 142*)
  b:   131072   131072      swap                        # (Cyl.  142*- 284*)
  c:  6265200        0    unused     1024  8192         # (Cyl.    0 - 6809)
  e:   781250   262144    4.2BSD     1024  8192    16   # (Cyl.  284*- 1134*)
  f:  1205000  1043394    4.2BSD     1024  8192    16   # (Cyl. 1134*- 2443*)
  g:  2008403  2248394    4.2BSD     1024  8192    16   # (Cyl. 2443*- 4626*)
  h:  2008403  4256797    4.2BSD     1024  8192    16   # (Cyl. 4626*- 6809*)
  i:    10208       32     MSDOS                        # (Cyl.    0*- 11*)
[End of example]
__md_prep_disklabel_1
	echo -n "Press [Enter] to continue "
	getresp ""
	disklabel -W ${_disk}
	disklabel ${_disk} >/tmp/label.$$
	disklabel -r -R ${_disk} /tmp/label.$$
	rm -f /tmp/label.$$
	disklabel -e ${_disk}
}

md_copy_kernel() {
}

md_welcome_banner() {
{
	if [ "$MODE" = "install" ]; then
		echo ""
		echo "Welcome to the OpenBSD/PowerPC ${VERSION} installation program."
		cat << \__welcome_banner_1

This program is designed to help you put OpenBSD on your disk,
in a simple and rational way.  You'll be asked several questions,
and it would probably be useful to have your disk's hardware
manual, the installation notes, and a calculator handy.
__welcome_banner_1

	else
		echo ""
		echo "Welcome to the OpenBSD/PowerPC ${VERSION} upgrade program."
		cat << \__welcome_banner_2

This program is designed to help you upgrade your OpenBSD system in a
simple and rational way.

As a reminder, installing the `etc' binary set is NOT recommended.
Once the rest of your system has been upgraded, you should manually
merge any changes to files in the `etc' set into those files which
already exist on your system.
__welcome_banner_2
	fi

cat << \__welcome_banner_3

As with anything which modifies your disk's contents, this
program can cause SIGNIFICANT data loss, and you are advised
to make sure your data is backed up before beginning the
installation process.

Default answers are displayed in brackets after the questions.
You can hit Control-C at any time to quit, but if you do so at a
prompt, you may have to hit return.  Also, quitting in the middle of
installation may leave your system in an inconsistent state.

__welcome_banner_3
} | more
}

md_not_going_to_install() {
	cat << \__not_going_to_install_1

OK, then.  Enter `halt' at the prompt to halt the machine.  Once the
machine has halted, power-cycle the system to load new boot code.

__not_going_to_install_1
}

md_congrats() {
	local what;
	if [ "$MODE" = "install" ]; then
		what="installed";
	else
		what="upgraded";
	fi
	cat << __congratulations_1

CONGRATULATIONS!  You have successfully $what OpenBSD!
To boot the installed system, enter halt at the command prompt. Once the
system has halted, reset the machine and boot from the disk.

__congratulations_1
}
