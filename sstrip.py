#! /usr/bin/python

"""Removes section headers and unnecessary parts from an ELF32 executable."""

import struct
import sys


def sstrip_elf32_executable(filename):
  """Removes section headers and unnecessary parts from an ELF32 executable.

  It also changes ei_osabi to GNU/Linux.

  Args:
    filename: Input filename. The file ill be modified in place.
  """
  f = open(filename, 'rb+')
  try:
    ehdata = f.read(0x34)
    if len(ehdata) != 0x34:
      raise ValueError
    # https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
    if not ehdata.startswith('\x7fELF\1\1\1'):
      raise ValueError('Not a 32-bit MSB-first ELF v1 file.')
    (e_phoff, e_shoff, e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize,
     e_shnum, e_shstrndx) = struct.unpack('<LLLHHHHHH', ehdata[0x1c : 0x34])
    if e_phentsize != 0x20:
      raise ValueError
    if ehdata[0x10] != '\2':
      raise ValueError('Expected an executable file.')
    if e_phnum >> 7:  # Typically there is 1 or 2 in statically linked exec.
      raise ValueError('Too many program header entries.')
    f.seek(e_phoff)
    phdata = f.read(0x20 * e_phnum)
    if len(phdata) != 0x20 * e_phnum:
      raise ValueError
    truncate_ofs = max(0x34, e_phoff + 0x20 * e_phnum)
    for pho in xrange(0, 0x20 * e_phnum, 0x20):
      (p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags,
       p_align) = struct.unpack('<8L', phdata[pho : pho + 0x20])
      if p_type == 1 and 0 < p_offset < 0x1000:  # p_type == PT_LOAD.
        # Prevent upx CantPackException: Go-language PT_LOAD.
        # Prevent invalid Phdr p_offset; (without upx --force-exece).
        p_vaddr -= p_offset
        p_paddr -= p_offset
        p_filesz += p_offset
        p_memsz += p_offset
        p_offset = 0
        newphdata = struct.pack('<8L', p_type, p_offset, p_vaddr, p_paddr,
                                p_filesz, p_memsz, p_flags, p_align)
        f.seek(e_phoff + pho)
        f.write(newphdata)
      truncate_ofs = max(truncate_ofs, p_offset + p_filesz)
    f.seek(7)
    f.write('\3')  # ei_osabi = GNU/Linux.
    f.seek(0x20)
    f.write('\0\0\0\0')  # e_shoff = 0.
    f.seek(0x30)
    f.write('\0\0\0\0')  # e_shnum = e_shstrndx = 0.
    # TODO(pts): Remove trailing \0 from EOF.
    f.truncate(truncate_ofs)
  finally:
    f.close()  # Works even if fout == f.


def main(argv):
  if len(argv) != 2:
    print >>sys.stderr, 'Usage: %s <filename.elf>' % argv[0]
    sys.exit(1)
  sstrip_elf32_executable(argv[1])


if __name__ == '__main__':
  sys.exit(main(sys.argv))
