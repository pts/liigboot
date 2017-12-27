#! /usr/bin/python
# by pts@fazekas.hu at Sun Dec 24 09:55:55 CET 2017

"""16-bit i386 machine code compressor for liigmain.bin, using UPX."""

import array
import os
import os.path
import struct
import sys
import subprocess


def parse_struct(fields, data):
  values = struct.unpack('<' + ''.join(f[1] for f in fields), data)
  return dict(zip((f[0] for f in fields), values))


def dump_struct(fields, data):
  format = '<' + ''.join(f[1] for f in fields)
  values = struct.unpack(format, data)
  print '--- Header ' + format
  for (field_name, field_type), value in zip(fields, values):
    if isinstance(value, (int, long)):
      value = '0x%x' % value
    else:
      value = repr(value)
    print '%s = %s' % (field_name, value)
  print '---/Header'


SIGNATURE_OFS = 0x2c
SIGNATURE = '\xeb\x22' + '\x53\x5b' * 16 + '\xfa\xf4'


def bmcompress(text, load_addr, tmp_filename, method='--ultra-brute',
               signature_start_ofs_max=0):
  """Compresses 16-bit i386 machine code.

  Under the hood a DOS .exe file is created, it is compressed with UPX, and
  the result is composed using the compressed output of UPX.

  Please note that compressed code behaves differently than the uncompressed
  code:

  * After decompression, most registers (including ax, bc, cx, dx, bp, si,
    di) will be destroyed and become undefined. (More info on:
    http://www.tavi.co.uk/phobos/exeformat.html)
  * ss:sp is restored (kept) after decompression. Decompression doesn't use
    much stack space there.
  * cs, ds and es will be reset to 0 after decompression.
  * The entry point (ip) after decompression is the byte after the
    bmcompress signature.
  * The memory region containing the bmcompress signature gets destroyed
    (overwritten) during decompressoin.
  * In the resulting output compressed file the region containing the
    bmcompress signature gets overwritten by some decompression trampoline
    code.

  Args:
    text: The 16-bit i386 machine code to be compressed. Must contain the
        bmcompress signature (SIGNATURE) near its begining: at least offset
        0x2c (SIGNATURE_OFS), at 12 (SIGNATURE_OFS & 15) bytes after a
        16-byte boundary. Everything earlier than the signature will be kept
        intact (uncompressed) in the output. The signature will be destroyed (overwritten). The
        just-before-compression entry point is the beginning of the
        signature.
    load_addr: Absolute address to which `text' is loaded before decompression.
    tmp_filename: Temporary filename to use during the compression.
        Will be deleted on success.
    method: Comma-or-whitespace-separated list of UPX command-line flags to
        select the compression method.
    signature_start_ofs_max: Maximum offset in `text' where the bmcompress
        signature can be found, minus SIGNATURE_OFS. E.g. iff 0, then the
        bmcompress signature should be at SIGNATURE_OFS.
  Returns:
    Compressed 16-bit i386 machine code equivalent to data. This machine
    code decompresses itself and then jumps to the byte after the bmcompress
    signature. See above what else is done during decompression. It is
    exactly the same as the input `text' if compression can't make it smaller.
  """
  if not isinstance(text, str):
    raise TypeError
  if load_addr < 0x500:  # http://wiki.osdev.org/Memory_Map_(x86)
    raise ValueError('Code to be loaded too early in memory.')
  if load_addr + len(text) > 0x9f000:  # http://wiki.osdev.org/Memory_Map_(x86)
    # The actual limit is lower than 0x9fc00 (start of EBDA), we may need some
    # space for stack etc.
    raise ValueError('Code to be compressed too long.')
  assert SIGNATURE_OFS + len(SIGNATURE) == 0x50
  signature_ofs = text.find(
      SIGNATURE,
      SIGNATURE_OFS, SIGNATURE_OFS + signature_start_ofs_max + len(SIGNATURE))
  if signature_ofs < 0:
    raise ValueError('Missing bmcompress signature.')
  load_ofs = signature_ofs + len(SIGNATURE)
  load_addr += load_ofs
  if load_addr & 0xf:
    raise ValueError(
        'bmcompress signature not aligned to 16-byte boundary + 12.')

  if not method or method == '--none':
    return text  # Keep it uncompressed.
  if not method.startswith('--'):
    # Example good: '--best'.
    # Example good: '--brute'.
    # Example good: '--ultra-brute --lzma'.
    raise ValueError('Bad method: %s' % method)

  # ---
  #
  # This is going to get hacky. Below we create a 16-bit DOS .exe file, call
  # UPX to compress it, and then we patch up the result (filling text[44 : 80]
  # above with our fixup code).
  #
  # Documentation about DOS .exe files:
  #
  # * http://www.tavi.co.uk/phobos/exeformat.html (best, describing all registers)
  # * https://en.wikibooks.org/wiki/X86_Disassembly/Windows_Executable_Files#MS-DOS_header
  # * http://www.delorie.com/djgpp/doc/exe/
  exe_header_fields = (
      ('dosexe_signature', '2s'),  # 'MZ'.
      ('lastsize', 'H'),
      ('nblocks', 'H'),
      ('nreloc', 'H'),
      ('hdrsize', 'H'),  # Always even. Header size (including relocations): hdrsize << 4 bytes.
      ('minalloc', 'H'),
      ('maxalloc', 'H'),
      ('ss', 'H'),
      ('sp', 'H'),
      ('checksum', 'H'),  # Can be 0.
      ('ip', 'H'),
      ('cs', 'H'),
      ('relocpos', 'H'),
      ('noverlay', 'H'),
      ('rofs', 'H'),
      ('rseg', 'H'),
#     short reserved1[4];
#     short oem_id;
#     short oem_info;
#     short reserved2[10];
#     long  e_lfanew; // Offset to the 'PE\0\0' signature relative to the beginning of the file
  )

  exe_size = 0x20 + len(text) - load_ofs
  #sp_magic = 0x06fe
  #sp_magic = 0x72fe
  sp_magic = 0x2050  # Doesn't matter, won't be used.
  #assert 0, '0x%x' % ((-(load_addr >> 4) + 2) & 0xffff)  # 0xf780
  #assert 0, '0x%x' % ((-(load_addr >> 4) + 5) & 0xffff)  # 0xf783
  #sp_magic = 0
  exe_header = struct.pack(
      '<2sHH8sHH14s',
      'MZ',
      (exe_size & 511) or 512,
      (exe_size + 511) >> 9,
      '\x00\x00'  # nreloc.
      '\x02\x00'  # hdrsize.
      '\x01\x00'  # minalloc.
      '\x01\x00',  # maxalloc.
      0, # SS (before relocation).
      #0xfffe, # -(load_addr >> 4) & 0xffff,  # SS (before relocation). GOOD.
      #(-(load_addr >> 4) + 5) & 0xffff,  # SS (before relocation). BAD. Why?
      sp_magic,  # SP.
      '\x00\x00'  # Checksum.
      '\x00\x00'  # Initial IP.
      '\x00\x00'  # CS (before relocation).
      '\x00\x00\x00\x00\x00\x00\x00\x00',
  )
  assert len(exe_header) == 32
  #dump_struct(exe_header_fields, exe_header)
  open(tmp_filename, 'wb').write(exe_header + text[load_ofs:])
  sys.stdout.flush()
  # !! What if can't improve with compression? Keep original.
  #    Or if file too small?
  # -qqq is totally quiet. -qq prints one line.
  # !! upx: liigmain.bin.tmp: NotCompressibleException
  # TODO(pts): Experiment with --ultra-brute --lzma.
  subprocess.check_call(
      [(os.path.dirname(__file__) or '.') + '/tools/upx', '-qq'] +
      method.replace(',', ' ').split() +
      ['--', tmp_filename])
  data = open(tmp_filename, 'rb').read()
  exe_header = data[:0x20]

  h = parse_struct(exe_header_fields, exe_header)
  #dump_struct(exe_header_fields, exe_header)
  if h['dosexe_signature'] != 'MZ':
    raise ValueError('Expected dosexe_signature from UPX.')
  if h['hdrsize'] != 2:
    raise ValueError('Expected hdrsize=2 from UPX.')
  if h['ip'] != 0:
    raise ValueError('Expected ip=0 from UPX.')
  if h['cs'] != 0:
    raise ValueError('Expected cs=0 from UPX.')
  if h['nblocks'] <= 0:
    raise ValueError('Expected positive nblocks from UPX.')
  if h['lastsize'] > 512:
    raise ValueError('Expected small lastsize from UPX.')
  if len(data) != ((h['nblocks'] - 1) << 9) + h['lastsize']:
    raise ValueError('Bad .exe file size from UPX.')
  # For grub4dos.bs --ultra-brute: ss=0x339a,sp=0x200
  # For hiiimain.compressed.bin --ultra-brute: ss=0x9fe, sp=0x200
  data_ary = array.array('B', data[0x20:])
  relocpos = h['relocpos']
  extra_code2_size = 8
  sp_addr = None
  # !! Make method == '--lzma' work (currently it emits h['nreloc'] == 0).
  #    Fixing up the ss:sp afterwards will need >=6 bytes more space.
  #    The absolute jump \xea is also missing.
  # !! Also make sure that the ss:sp in the uncompressed exe_header is large
  #    enough for us to return (and do some interrupts as well -- add 0x200
  #    to sp, like UPX does?).
  assert h['nreloc'] == 1, h['nreloc']  # We want it, for setting sp_addr.
  os.unlink(tmp_filename)
  for _ in xrange(h['nreloc']):
    # !! Compile without knowing load_addr.
    rofs, rseg = struct.unpack('<HH', data[relocpos : relocpos + 4])
    #print 'relocation ofs=0x%x seg=0x%x' % (rofs, rseg)
    dofs = (rseg << 4) + rofs
    # jmp word 0x...:0x...
    assert data_ary[dofs - 3 : dofs + 2].tostring() == '\xea\x00\x00\x00\x00'
    #00023AE3  8D860000          lea ax,[bp+0x0]
    #00023AE7  8ED0              mov ss,ax
    #00023AE9  BC5020            mov sp,0x2050
    assert data_ary[dofs - 12 : dofs - 3].tostring() == struct.pack(
        '<7sH', '\x8d\x86\0\0\x8e\xd0\xbc', sp_magic)  # mov sp, 0x....
    #sp_addr = dofs - 5 + load_addr
    data_ary[dofs - 12 : dofs + 2] = array.array(
        'B',
        '\x31\xc0' +  # xor ax, ax
        '\x8e\xd8' +  # mov ds, ax
        '\x8e\xc0' +  # mov es, ax
        struct.pack('<BHH', 0xea, load_addr - extra_code2_size, 0) +  # jmp word 0x...:0x...
        '\x90' * 3)
    #xseg = (struct.unpack('<H', data_ary[dofs : dofs + 2])[0] + (load_addr >> 4)) & 0xffff
    #data_ary[dofs : dofs + 2] = array.array('B', struct.pack('<H', xseg))
    relocpos += 4
  ss_addr_x = load_addr - 7
  sp_addr_x = load_addr - 2
  extra_code1 = (  # Run before the on-the-fly decompression.
      struct.pack('<BBH', 0x8c, 0x16, ss_addr_x) +  # mov [...], ss
      struct.pack('<BBH', 0x89, 0x26, sp_addr_x) +  # mov [...], sp
      struct.pack('<BH', 0xb8, (load_addr - 0x100) >> 4) +  # mov ax, ...
      '\x8e\xd8' +  # mov ds, ax
      '\x8e\xc0' +  # mov es, ax
      struct.pack('<BH', 0x05, h['ss'] + 0x10) +  # add ax, ... + 0x10
      '\x8e\xd0' +      # mov ss, ax  ; Automatic cli for the next instr.
      struct.pack('<BH', 0xbc, h['sp']) +   # mov sp, ...
      struct.pack('<BHH', 0xea, 0, (load_addr >> 4)) +  # jmp word 0x...:0
      '')
  extra_code2 = (  # Run after the on-the-fly decompression.
      #'\x6a\x2b' +  # push '+'
      # These must be the last 8 bytes, ss_addr_x and sp_addr_x use them.
      struct.pack('<BH', 0xb8, 0) +  # mov ax, ...
      '\x8e\xd0' +  # mov ss, ax  ; Automatic cli for the next instr.
      struct.pack('<BH', 0xbc, 0) +  # mov sp, ...
      '')
  assert len(extra_code2) == extra_code2_size, len(extra_code2)
  assert len(extra_code1) + len(extra_code2) == 0x24
  return ''.join((
      text[:signature_ofs],  # Kept intact.
      extra_code1, extra_code2, data_ary.tostring()))


def main(argv):
  input_filename = output_filename = None
  signature_start_ofs_max = 0
  # The in-memory address where the input_filename (starting with the nop*k
  # header) will be loaded. Please note that the nop*k header may be
  # replaced by something else at load time.
  #
  # TODO(pts): Make sure that the .bss is zeroed. How large is it?
  load_addr = None
  skip0 = 0
  i = 1
  while i < len(argv):
    arg = argv[i]
    i += 1
    if arg.startswith('--bin='):
      input_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--out='):
      output_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--load-addr='):
      load_addr = int(arg[arg.find('=') + 1:], 0)
    elif arg.startswith('--skip0='):
      # Skip this many \x00 bytes at start of --bin=.
      skip0 = int(arg[arg.find('=') + 1:], 0)
    elif arg.startswith('--sig-ofs-max='):
      signature_start_ofs_max = int(arg[arg.find('=') + 1:], 0)
    else:
      sys.exit('fatal: unknown command-line flag: ' + arg)
  if input_filename is None:
    sys.exit('fatal: missing --bin=')
  if output_filename is None:
    sys.exit('fatal: missing --out=')
  if load_addr is None:
    sys.exit('fatal: missing --load-addr=')
  if load_addr & (0x10 - 1):
    sys.exit('fatal: --load-addr not divisible by 0x10: 0x%x' % load_addr)
  if not (load_addr == 0x7c00 or 0x8000 <= load_addr <= 0xffff):
    # It doesn't matter much, it's just a sanity check.
    sys.exit('fatal: --load-addr outside its range: 0x%x' % load_addr)

  text = open(input_filename, 'rb').read()
  if skip0:
    print >>sys.stderr, 'info: bmcompress input before skip: %s (%d bytes)' % (
        input_filename, len(text))
    if len(text) < skip0:
      raise ValueError('Input too short for --skip0=')
    if text[:skip0].lstrip('\0'):
      raise ValueError('Nonzero bytes found in --skip0= region.')
    text = text[skip0:]
  print >>sys.stderr, 'info: bmcompress input: %s (%d bytes)' % (
      input_filename, len(text))
  tmp_filename = output_filename + '.tmp'
  text = bmcompress(text, load_addr, tmp_filename, '--ultra-brute',
                    signature_start_ofs_max)
  # !! If compressed is longer than original, emit original.
  # ndisasm -b 16 -o $(LOAD_ADDR) -e 0x2c hiiimain.uncompressed.bin
  open(output_filename, 'wb').write(text)
  print >>sys.stderr, 'info: bmcompress output: %s (%d bytes)' % (
      output_filename, len(text))


if __name__ == '__main__':
  sys.exit(main(sys.argv))
