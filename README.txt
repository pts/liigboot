by pts@fazekas.hu at Sat Dec 16 00:48:34 CET 2017

Liigboot is a boot manager and bootloader which can boot Linux, Windows and
DOS operating systems from primary partitions of an MBR-partitioned hard
disk, letting the user choose which partition to boot from. Liigboot
autodetects which partitions contain bootable operating system. Liigboot
provides full with the default settings, not configuration needed. The
bootloader code resides in the initial 1 MiB of the hard disk, in front of
first parititon. Liigboot also contains a copy of GRUB (legacy, not GRUB2)
and memtest86+. Liigboot comes with a Liigresc, a larger, 128 MiB image (out
of which 16 MiB is used), which contains a command-line rescue system based
on Tiny Core Linux, with (wired) networking support. This can be used for
some rescue and data recovery in case the main operating systems fail to
boot.

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

Installation is almost as easy as copying the 1 MiB liigboot.img to the
beginning of the hard drive. But this destroys the parition table, and it
causes data loss if the 1st parititon starts too early. To fix these, an
Linux-based installer is provided.

Implementation status: High-level design done, low-level design almost
finished, implementation in progress.

__END__
