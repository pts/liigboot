#! /usr/bin/python
# by pts@fazekas.hu at Tue Jan  9 17:30:58 CET 2018

"""elftool.py analyze ELF executables."""

import struct
import sys

# --- ELF tools.

PT_LOAD = 1

SHT_NULL = 0
SHT_PROGBITS = 1
SHT_SYMTAB = 2
SHT_STRTAB = 3
SHT_REL = 9
SECTION_TYPES = (
    0x0 or 'NULL',  # Section header table entry unused
    0x1 and 'PROGBITS',  # Program data
    0x2 and 'SYMTAB',  # Symbol table
    0x3 and 'STRTAB',  # String table
    0x4 and 'RELA',  # Relocation entries with addends
    0x5 and 'HASH',  # Symbol hash table
    0x6 and 'DYNAMIC',  # Dynamic linking information
    0x7 and 'NOTE',  # Notes
    0x8 and 'NOBITS',  # Program space with no data (bss)
    0x9 and 'REL',  # Relocation entries, no addends
    0x0A and 'SHLIB',  # Reserved
    0x0B and 'DYNSYM',  # Dynamic linker symbol table
    0x0E and 'INIT_ARRAY',  # Array of constructors
    0x0F and 'FINI_ARRAY',  # Array of destructors
    0x10 and 'PREINIT_ARRAY',  # Array of pre-constructors
    0x11 and 'GROUP',  # Section group
    0x12 and 'SYMTAB_SHNDX',  # Extended section indeces
)

R_386_32 =   1  # Direct 32 bit.
R_386_PC32 = 2  # PC relative 32 bit.
RELOCATION_TYPES = (
  0 or 'NONE',
  1 and '32',
  2 and 'PC32',
)

def load_elf(filename):
  """Removes section headers and unnecessary functions fom an ELF executable.

  It also changes ei_osabi to GNU/Linux.

  Args:
    filename: Input filename. The file ill be modified in place.
  """
  f = open(filename, 'rb')
  try:
    ehdata = f.read(52)
    if len(ehdata) != 52:
      raise ValueError
    # https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
    if not ehdata.startswith('\x7fELF\1\1\1'):
      raise ValueError('Not a 32-bit LSB-first ELF v1 file.')
    (e_osabi, e_abiversion, e_pad, e_type, e_machine, e_version, e_entry,
     e_phoff, e_shoff, e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize,
     e_shnum, e_shstrndx) = struct.unpack('<BB7sHHLLLLLHHHHHH', ehdata[7 : 52])
    if e_osabi not in (0, 3):  # 0: System V, 3: Linux.
      raise ValueError
    if e_abiversion != 0:
      raise ValueError
    #if e_pad != '\0\0\0\0\0\0\0':
    #  raise ValueError
    if e_type != 2:
      raise ValueError('Expected an executable file.')
    if e_machine != 3:  # x86.
      raise ValueError('Expected i386.')
    if e_version != 1:
      raise ValueError
    if e_ehsize != 52:
      raise ValueError
    if e_phentsize != 32:
      raise ValueError
    if e_flags != 0:
      raise ValueError
    if e_shentsize != 40:
      raise ValueError
    if e_phnum >> 7:  # Typically there is 1 or 2 in statically linked exec.
      raise ValueError('Too many program header entries.')
    f.seek(e_phoff)
    phdata = f.read(32 * e_phnum)
    if len(phdata) != 32 * e_phnum:
      raise ValueError
    #truncate_ofs = max(52, e_phoff + 32 * e_phnum)
    ph = []
    for pho in xrange(0, 32 * e_phnum, 32):
      (p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags,
       p_align) = struct.unpack('<8L', phdata[pho : pho + 32])
      if p_type == PT_LOAD:
        ph.append((p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz,
                   p_flags, p_align))
      #truncate_ofs = max(truncate_ofs, p_offset + p_filesz)

    f.seek(e_shoff)
    shdata = f.read(40 * e_shnum)
    if len(shdata) != 40 * e_shnum:
      raise ValueError

    # Load section names.
    shi = e_shstrndx
    if shi == 0 or shi * 40 + 40 > len(shdata):
      raise ValueError
    str_ofs, str_size = struct.unpack('<LL',
        buffer(shdata, shi * 40 + 4 * 4, 8))
    f.seek(str_ofs)
    str_data = f.read(str_size)
    if len(str_data) != str_size:
      raise ValueError

    sh = []
    for sho in xrange(0, 40 * e_shnum, 40):
      (sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link,
       sh_info, sh_addralign, sh_entsize,
       ) = struct.unpack('<10L', shdata[sho : sho + 40])
      j = str_data.find('\0', sh_name)
      if j < 0 or (j == sh_name and sh_name != 0):
        raise ValueError(sh_name)
      sh_name = str_data[sh_name : j]
      sh__type_name = SECTION_TYPES[sh_type]  # Can raise IndexError.
      sh.append((
          sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link,
          sh_info, sh_addralign, sh_entsize, sh__type_name))
      #truncate_ofs = max(truncate_ofs, p_offset + p_filesz)

  finally:
    f.close()  # Works even if fout == f.
  return e_entry, ph, sh


LOG2 = dict((1 << i, i) for i in xrange(32))


def main(argv):
  elf_filename = None
  sec_filename = None
  i = 1
  while i < len(argv):
    arg = argv[i]
    i += 1
    if arg.startswith('--in='):
      elf_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--sec='):
      sec_filename = arg[arg.find('=') + 1:]
    else:
      sys.exit('fatal: unknown command-line flag: ' + arg)
  if elf_filename is None:
    sys.exit('fatal: missing --in=')
  if sec_filename is None:
    sys.exit('fatal: missing --sec=')
  e_entry, ph, sh = load_elf(elf_filename)
  output = [
     '\n%s: file format elf32-i386\n\nSections:\n'
     'Idx Name          Size      VMA       LMA       File off  Algn\n' %
     elf_filename]
  i = 0
  for she in sh:
    (sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link,
     sh_info, sh_addralign, sh_entsize, sh__type_name) = she
    if sh_type in (SHT_NULL, SHT_STRTAB, SHT_SYMTAB):
      continue
    # Our value for LMA (sh_addr) doesn't always match objdump.
    # Also we always print an X instead of the section flags.
    output.append(
        '%3d %-13s %08x  %08x  %08x  %08x  2**%d\n                  X\n' %
        (i, sh_name, sh_size, sh_addr, sh_addr, sh_offset, LOG2[sh_addralign]))
    i += 1
  open(sec_filename, 'w').write(''.join(output))


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv))
  except (RuntimeError, SystemExit), e:
    if isinstance(e, SystemExit) and (not e.args or e.args[0] is None or
                                      isinstance(e.args[0], int)):
      raise
    print >>sys.stderr, 'fatal: %s' % e
    sys.exit(1)
