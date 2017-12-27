by pts@fazekas.hu at Sat Dec 16 00:48:34 CET 2017

Liigboot is a boot manager and bootloader which can boot Linux, Windows and
DOS operating systems from primary partitions of an MBR-partitioned hard
disk, letting the user choose which partition to boot from. Liigboot
autodetects which partitions contain bootable operating system. Liigboot
provides full with the default settings, not configuration needed. The
bootloader code resides in the initial 1 MiB of the hard disk, in front of
first parititon. Liigboot also contains a copy of GRUB (GRUB4DOS, which is
more similar to legacy than GRUB2) and memtest86+. Liigboot comes with a
Liigresc, a larger, 128 MiB image (out of which 16 MiB is used), which
contains a command-line rescue system based on Tiny Core Linux, with (wired)
networking support. This can be used for some rescue and data recovery in
case the main operating systems fail to boot.

Feature support:

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
   the device and it can cause other data losses as well. If unsure, don't do it.

   If you understand the risks, run the command below without the `echo'.

2. Just copy liigboot.zip to the beginning of the device:

     $ sudo echo dd if=liigboot.zip of=/dev/sdX bs=1M

Fun fact: Installation also works as `python liigboot.zip install ...' if you
have Python 2.6, 2.7 or 3.x installed.

How to try Liigboot without installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
You can try it (e.g. running memtest) in QEMU:

  $ qemu-system-i386 -m 4 -hda liigboot.zip -enable-kvm -net none

If it doesn't start, run it without the `-enable-kvm'.

Please note that memtest hangs with `-enable-kvm' in QEMU 2.0.0, as a
workaround run QEMU without this flag.

__END__
