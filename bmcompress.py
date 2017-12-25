#! /usr/bin/python
# by pts@fazekas.hu at Sun Dec 24 09:55:55 CET 2017

"""16-bit executable compressor for liigmain.bin, using UPX."""

import array
import os
import struct
import sys
import subprocess


def dump_header(fields, data):
  values = struct.unpack('<' + ''.join(f[1] for f in fields), data[:0x20])
  h = {}
  #print '---H'
  for (field_name, field_type), value in zip(fields, values):
    h[field_name] = value
    if isinstance(value, (int, long)):
      value = '0x%x' % value
    else:
      value = repr(value)
    #print '%s: %s' % (field_name, value)
  #print '---/H'
  return h


def main(argv):
  input_filename = None
  output_filename = None
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
    else:
      sys.exit('fatal: unknown command-line flag: ' + arg)
  if input_filename is None:
    sys.exit('fatal: missing --bin=')
  if output_filename is None:
    sys.exit('fatal: missing --out=')
  if load_addr is None:
    sys.exit('fatal: missing --load-addr=')
  if load_addr & (0x10 - 1):
    sys.exit('fatal: --load-addr not divisible by 0x10: 0x%x', load_addr)
  if not (0x8000 <= load_addr <= 0xffff):
    sys.exit('fatal: --load-addr outside its range: 0x%x', load_addr)

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
  if text[44 : 80] != '\xeb\x22' + '\x53\x5b' * 16 + '\xfa\xf4':
    raise ValueError('Missing liigmain.bin uncompressed signature.')
  preheader = text[:44]  # This will be kept intact.
  input_size = len(text)
  text = text[0x50:]  # Code to be compressed.
  load_seg = (load_addr + 0x50) >> 4

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
  fields = (
      ('signature', '2s'),
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

  exe_size = 0x20 + len(text)

  #sp_magic = 0x06fe
  #sp_magic = 0x72fe
  sp_magic = 0x2050  # Doesn't matter, won't be used.
  #assert 0, '0x%x' % ((-load_seg + 2) & 0xffff)  # 0xf780
  #assert 0, '0x%x' % ((-load_seg + 5) & 0xffff)  # 0xf783
  #sp_magic = 0
  exe_header = struct.pack(
      '<2sHH8sHH14s',
      'MZ',
      exe_size & 511,
      (exe_size + 511) >> 9,
      '\x00\x00'  # nreloc.
      '\x02\x00'  # hdrsize.
      '\x01\x00'  # minalloc.
      '\x01\x00',  # maxalloc.
      0, # SS (before relocation).
      #0xfffe, # -load_seg & 0xffff,  # SS (before relocation). GOOD.
      #(-load_seg + 5) & 0xffff,  # SS (before relocation). BAD. Why?
      sp_magic,  # SP.
      '\x00\x00'  # Checksum.
      '\x00\x00'  # Initial IP.
      '\x00\x00'  # CS (before relocation).
      '\x00\x00\x00\x00\x00\x00\x00\x00',
  )
  assert len(exe_header) == 32
  dump_header(fields, exe_header[:0x20])
  open(output_filename + '.tmp', 'wb').write(exe_header + text)
  sys.stdout.flush()
  # !! What if can't improve with compression? Keep original.
  #    Or if file too small?
  # -qqq is totally quiet. -qq prints one line.
  # !! upx: liigmain.bin.tmp: NotCompressibleException
  # TODO(pts): Experiment with --ultra-brute --lzma.
  subprocess.check_call(
      ((os.path.dirname(__file__) or '.') + '/tools/upx', '-qq',
       '--ultra-brute', '--', output_filename + '.tmp'))
  data = open(output_filename + '.tmp', 'rb').read()
  exe_header = data[:0x20]
  # !! Truncate data at: nblocks, nreloc.
  h = dump_header(fields, exe_header)
  if h['hdrsize'] != 2:
    raise ValueError('Expected hdrsize=2 from UPX.')
  if h['ip'] != 0:
    raise ValueError('Expected ip=0 from UPX.')
  if h['cs'] != 0:
    raise ValueError('Expected cs=0 from UPX.')
  data_ary = array.array('B', data[0x20:])
  relocpos = h['relocpos']
  extra_code1_size = 40
  extra_code2_size = 8
  assert extra_code1_size + extra_code2_size == 0x30
  sp_addr = None
  assert h['nreloc'] == 1  # We want it, for setting sp_addr.
  for _ in xrange(h['nreloc']):
    # !! Compile without knowing load_addr (or load_seg).
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
    #sp_addr = dofs - 5 + (load_seg << 4)
    data_ary[dofs - 12 : dofs + 2] = array.array(
        'B',
        '\x31\xc0' +  # xor ax, ax
        '\x8e\xd8' +  # mov ds, ax
        '\x8e\xc0' +  # mov es, ax
        struct.pack('<BHH', 0xea, (load_seg << 4) - extra_code2_size, 0) +  # jmp word 0x...:0x...
        '\x90' * 3)
    #xseg = (struct.unpack('<H', data_ary[dofs : dofs + 2])[0] + load_seg) & 0xffff
    #data_ary[dofs : dofs + 2] = array.array('B', struct.pack('<H', xseg))
    relocpos += 4
  ss_addr_x = (load_seg << 4) - 7
  sp_addr_x = (load_seg << 4) - 2
  extra_code1 = (  # Run before the on-the-fly compression.
      '\x90' * 12 +  # Last 12 bytes of preheader will be used instead.
      struct.pack('<BBH', 0x8c, 0x16, ss_addr_x) +  # mov [...], ss
      struct.pack('<BBH', 0x89, 0x26, sp_addr_x) +  # mov [...], sp
      struct.pack('<BH', 0xb8, load_seg - 0x10) +  # mov ax, ...
      '\x8e\xd8' +  # mov ds, ax
      '\x8e\xc0' +  # mov es, ax
      struct.pack('<BH', 0x05, h['ss'] + 0x10) +  # add ax, ... + 0x10
      '\x8e\xd0' +      # mov ss, ax  ; Automatic cli for the next instr.
      struct.pack('<BH', 0xbc, h['sp']) +   # mov sp, ...
      struct.pack('<BHH', 0xea, 0, load_seg) +  # jmp word 0x...:0
      '')
  extra_code2 = (  # Run after the on-the-fly compression.
      #'\x6a\x2b' +  # push '+'
      # These must be the last 8 bytes, ss_addr_x and sp_addr_x use them.
      struct.pack('<BH', 0xb8, 0) +  # mov ax, ...
      '\x8e\xd0' +  # mov ss, ax  ; Automatic cli for the next instr.
      struct.pack('<BH', 0xbc, 0) +  # mov sp, ...
      '')
  assert len(extra_code2) == extra_code2_size, len(extra_code2)
  assert len(extra_code1) == extra_code1_size, len(extra_code1)
  text = ''.join((
      preheader, extra_code1[12:], extra_code2, data_ary.tostring()))
  # !! If compressed is longer than original, emit original.
  # ndisasm -b 16 -o $(LOAD_ADDR) -e 0x2c hiiimain.uncompressed.bin
  open(output_filename, 'wb').write(text)
  print >>sys.stderr, 'info: bmcompress output: %s (%d bytes)' % (
      output_filename, len(text))
  os.unlink(output_filename + '.tmp')


if __name__ == '__main__':
  sys.exit(main(sys.argv))
