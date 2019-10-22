by pts@fazekas.hu at Sat Dec 16 00:48:34 CET 2017

Liigboot is a boot manager and bootloader which can boot Linux, Windows and
DOS operating systems from primary partitions of an MBR-partitioned hard
disk, letting the user choose which partition to boot from. Liigboot
autodetects which partitions contain bootable operating systems. Liigboot
doesn't need a config file, it has autodetection and good defaults, and the
user can override the defaults at boot time. The bootloader code resides in
the initial 1 MiB of the drive (typically HDD, SSD or pen drive), in front
of first partititon. Liigboot also contains a copy of GRUB (GRUB4DOS, which
is more similar to legacy than GRUB2) and memtest86+. Liigboot comes with a
Liigresc, a larger, 128 MiB image (out of which 16 MiB is used), which
contains a command-line rescue system based on Tiny Core Linux, with (wired)
networking support. This can be used for some rescue and data recovery in
case the main operating systems fail to boot.

Use cases of Liigboot:

* Discover bootable Linux and Windows partitions on the system, and boot any
  of them, without configuration.

* Boot Linux (mostly Debian and Ubuntu) even if it's not bootable (e.g. GRUB
  wasn't installed or it's broken).

* Copy a bootable Linux filesystem from a pen drive to an internal drive,
  and boot Linux from it.

  This is a software distribution mechanism: on computer A; you create a
  bootable Linux filesystem; yo copy this filesystem to a partition on the
  pen drive; you install Liigboot to the pen drive, you boot Liigboot on
  computer B from the pen drive; you run the `install ...' command, you run
  the `copy fs ...' command. Each time you need to reimage computer B, you
  repeat.

  This is useful for upgrading the Linux software of airgapped computers
  (i.e. with networking disabled on purpose).

* Convenient way to install many Linux distributions from the network or to
  do rescue work using netboot.xyz. (Install Liigboot to a pen drive on
  another computer, it contains a copy of the nextboot.xyz kernel, then boot
  Liigboot from the pen drive, then run the `netboot.xyz' command.)

* Convenient way to run memtest (checking for memory hardware errors).

* Convenient way to run GRUB4DOS for filesystem exploration and rescue
  booting.

* Customizable syslinux.cfg file on a FAT filesystem image: preconfigure any
  boot options, change the defaults etc.

Feature support (++ is supported, -- is not supported):

* -- booting from floppy disk: Works with GRUB, not officially supported.
* -- booting an operating system which is not on the current boot hard disk:
  Works with GRUB, not officially supported.
* -- booting from extended parititions: Works with GRUB, not officially
  supported.
* -- booting from a GPT-partitioned hard disk: Not supported.
* -- booting using UEFI: Not supported. In the system (BIOS) settings,
  legacy (BIOS) boot must be enabled.
* -- booting from encrypted partitions: Not officially supported, it may
  work.
* -- booting with an old BIOS which doesn't support LBA: Not supported.
* -- secure boot: Not supported.
* -- booting on an 16-bit system (e.g. Intel 286): Not supported. The
* -- booting on a system with less than 2 MiB of RAM: Not supported.
* minimum requirement is an Intel 386 or compatible processor.
* ++ booting on a 32-bit system: Works on Intel 386 or compatible
  processors, but not on other architectures, e.g. 32-bit ARM.
* ++ booting on a 64-bit system: Works on Intel x86_64 (amd64) or compatible
  processors, but not on other architectures, e.g. 64-bit ARM.
* ++ booting from pen drive: Supported on systems which support USB booting.
* -- booting from CD or DVD: Not supported.

Installation is almost as easy as copying the 1 MiB liigboot.img to the
beginning of the hard drive. But this destroys the parition table, and it
causes data loss if the 1st parititon starts too early. To fix these, an
Linux-based installer is provided.

Implementation status: High-level design done, low-level design almost
finished, implementation in progress.

Installation
~~~~~~~~~~~~
Preparations:

0. To install Liigboot, you need a working Linux system with i386 or amd64
   architecture. It doesn't have to be the same system on which you will boot
   Liigboot.

1. Please note that Liigboot must be installed to the beginning of the device,
   i.e. to the first 1 MiB starting with the Master Boot Record (MBR). The
   installer will keep the original paritition table, and it will safely refuse
   to install if any of the existing partitions would overlap with Liigboot.

2. Download Liigboot with the following command:

  $ curl -OL https://github.com/pts/liigboot/releases/download/latest/liigboot.zip

Installation to a device (hard drive, USB pen drive etc.):

1. Connect the target device to the computer, and mount one of the
   filesystems on it (assuming /media/FOO/BAR from now).

2. Install Liigboot:

     $ unzip -o liigboot.zip liigboot.zip.install
     $ sudo ./liigboot.zip.install install /media/FOO/BAR

   This will install Liigboot to the MBR of the corresponding device, and it
   will also activate the specificed partition for next boot.

   If you are getting the error `unzip: invalid zip magic 53903CEB', then
   you should use the regular `unzip' command rather than the `unzip' tool
   in Busybox.

Alternative installation, if the device doesn't have a mountable parititon
on it yet:

1. Connect the target device to the computer, and figure out the device
   name (e.g /dev/sdX, where X is a letter, typically b).

2. Install Liigboot:

     $ unzip -o liigboot.zip liigboot.zip.install
     $ sudo ./liigboot.zip.install install /dev/sdX
     $ sync

Alternative installation, if the installer refuses to install, and you want
to force it, risking data loss:

1. Connect the target device to the computer, and figure out the device
   name (e.g /dev/sdX, where X is a letter, typically b).

2. Install Liigboot:

     $ unzip -o liigboot.zip liigboot.zip.install
     $ sudo ./liigboot.zip.install install image /dev/sdX
     $ sync

Alternative installation, to a disk image:

1. Create your disk image. The minimum is (assuming that the disk image file
   is my.img):

     $ touch my.img

2. Install Liigboot:

     $ unzip -o liigboot.zip liigboot.zip.install
     $ ./liigboot.zip.install install image my.img

Alternative installation, if you don't want to run the installer:

1. Be careful, this causes the loss (destruction) of the parition table on
   the device and it can cause other data losses as well. If unsure, don't
   do it.

   If you understand the risks, run the command below without the `echo'.

2. Just copy liigboot.zip to the beginning of the device:

     $ sudo echo dd if=liigboot.zip of=/dev/sdX bs=1M

Fun fact: Installation also works as `python liigboot.zip install ...' if you
have Python 2.6, 2.7 or 3.x installed.

TODO(pts): Write a Python-only installer (main_py_code in mkzip.py) which
doesn't run the `ligboot.zip.install' binary, and it works on any Unix system
with Python.

How to try Liigboot without installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
You can try it (e.g. running memtest) in QEMU:

  $ qemu-system-i386 -m 4 -hda liigboot.zip -enable-kvm -net none

If it doesn't start, run it without the `-enable-kvm'.

Please note that memtest hangs with `-enable-kvm' in QEMU 2.0.0, as a
workaround run QEMU without this flag.

You can also try booting installers and other rescue software from
http://netboot.xyz/ from Liigboot like this command.

  $ qemu-system-i386 -m 256 -hda liigboot.zip -enable-kvm -net nic -net user

Then at the `ligboot:' prompt, type `netboot' (without the quotes) and press
<Enter>.

How to build Liigboot
~~~~~~~~~~~~~~~~~~~~~
These build instructions are for an i386 or amd64 Linux host system. You
need to install Git to the build system, or copy the liigboot source folder
there.

Run these commands (without the leading `$'):

  $ git clone https://github.com/pts/liigboot
  $ cd liigboot
  $ PATH=/dev/null/missing tools/make
  $ ls -l liigboot.zip
  -rw-r--r-- 1 user group 1048576 Jan  9 15:49 liigboot.zip

The build should be reproducible, i.e. it should create a bit-by-bit
identical liigboot.zip output file when run again.

The PATH=/dev/null/missing setting makes sure that the compilers and other
tools on the system (e.g. nasm, gcc, ld, perl, python) are not used, not
even by accident.

__END__
