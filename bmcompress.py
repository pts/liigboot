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


# It's by coincidence that both SHORT and LONG need the same SIGNATURE_PHASE.
SIGNATURE_PHASE = 0xc
# Sixteen-Bit-Aligned-Fixed-Executable-Upx-Ucl compression.
# Ucl means UCL, i.e. non-LZMA.
SHORT_SIGNATURE1 = '\xeb"SBAFEUU_COMPRESSION_AFTER_SLASH__/'
# Sixteen-Bit-Aligned-Relocatable-Executable-Register-Preserving compression.
LONG_SIGNATURE1 = '\xeb?_SBARERP_COMPRESSION_WILL_BE_APPLIED_AFTER_THE_SLASH__________/'
# Like LONG_SIGNATURE1, but enable LZMA even if `bmcompress.py --method=lzma' is not
# specified.
LONG_SIGNATURE2 = '\xeb?_SBARERP_COMPRESSION_WILL_BE_APPLIED_AFTER_THE_SLASH______LZMA/'
assert SIGNATURE_PHASE + len(SHORT_SIGNATURE1) == 0x30
assert SIGNATURE_PHASE + len(LONG_SIGNATURE1) == 0x4d
assert SIGNATURE_PHASE + len(LONG_SIGNATURE2) == 0x4d


def bmcompress(text, load_addr, tmp_filename, method='--ultra-brute',
               signature_start_ofs_max=0):
  """Compresses 16-bit i386 machine code to self-decompressing code.

  The input machine code must contain one of the bmcompress signatures
  (SHORT_SIGNATURE1, LONG_SIGNATURE1 or LONG_SIGNATURE2). If in doubt,
  use LONG_SIGNATURE1. Everything before
  the signature will remain uncompressed and unchanged, and everything
  starting at the start of the signature will be compressed (thus changed).

  All signatures start with a `jmp short' instruction which jumps over the
  signature. Thus the signature can be inserted anywhere to the uncompressed
  assembly source, and the code will still work in its uncompressed form.

  The signatures must be aligned in memory: thir address must be 12 bytes
  (SIGNATURE_PHASE) after any byte offset divisible by 16. This is because
  of a limitation of the compressor: compressed data must start at a 16-byte
  boundary.

  Notes about behavior of compressed code:

  * After decompression, some registers will be destoyed and/or become
    undefined. See the mode-specific details later.
  * ss:sp is restored (kept) after decompression. Decompression doesn't use
    much stack space there.
  * The entry point after decompression is the byte after the
    bmcompress signature. (cs may be changed though, see the mode-specific
    details later.)
  * The memory region containing the bmcompress signature gets destroyed
    (overwritten) during decompressoin.
  * In the resulting output compressed file the region containing the
    bmcompress signature gets overwritten by some decompression trampoline
    code.

  Depending on which signature is used, different behavior is triggered:
  
  * SHORT_SIGNATURE1 enables short mode (see below).
  * LONG_SIGNATURE1 enables long mode (see below) without LZMA.
  * LONG_SIGNATURE2 enables long mode with possibly LZMA. The only
    disadvantage of LZMA is that decompression is slower than the other
    (i.e. UCL-based) ones.
  * If in doubt, use LONG_SIGNATURE1.

  In short mode:

  * Short mode has many restrictions (see below), the only advantage is
    that the signature (and thus the output as well) is 28 bytes shorter
    than in long mode.
  * The compressed code is not position-independent, the memory address
    to which it will be loaded must be specified in the load_addr argument.
  * LZMA compression doesn't work in short mode.
  * load_addr must not be larger than 0xffff, so the code can't be loaded
    to anywhere after the first 64 KiB.
  * After decompression, cs, ds and es will be reset to 0.
  * After decompression, flags, ax, bx, cx, dx, bp, si and di will be
    destroyed (i.e. they have an undefined value).
  * After decompression, ip contains the address of the byte after the
    bmcompress signature (sincs cs is 0). Hence the 0xffff limit on
    load_addr.

  In long mode:
  
  * The signature (and thuse the output as well) is 28 bytes longer than
    in short mode.
  * The decompressor in the compressed code is position-independent i.e. it
    can be loaded anywhere in memory, as long as it's properly aligned.
  * Both LZMA and UCL compressions work in long mode. To enable LZMA
    (in addition to UCL), use LONG_SIGNATURE2.
  * The upper limit on load_addr is 0x9f000 (636 KiB), which is based on the
    16-bit memory map (http://wiki.osdev.org/Memory_Map_(x86) ).
  * After decompression, the value of all registers (flags, ip, cs,
    ds, es, ss, sp, ax, bx, cx, dx, bp, si, di) is preserved.

  Implementation detail: under the hood a DOS .exe file is created, it is
  compressed with UPX, and the result is composed using the compressed
  output of UPX.

  Args:
    text: The 16-bit i386 machine code to be compressed. Must contain the
        bmcompress signature (SIGNATURE) near its begining:
        at 12 (SIGNATURE_PHASE) bytes after any
        16-byte boundary. Everything earlier than the signature will be kept
        intact (uncompressed) in the output. The signature will be destroyed
        (overwritten). The just-before-compression entry point is the
        beginning of the signature.
    load_addr: Absolute address to which (the start of) `text' is loaded
        before decompression. Ignored in long mode (except for the alignment
        check), because that works with any load address. Maximum
        value is 0x9f000.
    tmp_filename: Temporary filename to use during the compression.
        Will be deleted on success.
    method: Comma-or-whitespace-separated list of UPX command-line flags to
        select the compression method. The -- prefix will be added
        automatically. Recommended: 'ultra-brute' or
        'ultra-brute,lzma'. The latter picks the smaller of ultra-brute and
        lzma. Please note that lzma decompression is slower than the others
        (ultra-brute, brute, best, i.e. UCL).
    signature_start_ofs_max: Maximum offset in `text' where the bmcompress
        signature can be found, minus SIGNATURE_PHASE. E.g. iff 0, then the
        bmcompress signature should be at SIGNATURE_PHASE.
  Returns:
    Compressed 16-bit i386 machine code equivalent to data. This machine
    code decompresses itself and then jumps to the byte after the bmcompress
    signature. See above what else is done during decompression. It is
    exactly the same as the input `text' if compression can't make it smaller.
  """
  if not isinstance(text, str):
    raise TypeError

  if not method or method == '--none':
    return text  # Keep it uncompressed.
  method = ['--' + arg.lstrip('-') for arg in method.replace(',', ' ').split()
            if arg.lstrip('-')]
  # Example good: '--best'.
  # Example good: '--brute'.
  # Example good: '--ultra-brute --lzma'.
  if not method:
    method.append('--ultra-brute')

  for signature in (LONG_SIGNATURE1, LONG_SIGNATURE2, SHORT_SIGNATURE1):
    signature_ofs = text.find(
        signature, SIGNATURE_PHASE,
        SIGNATURE_PHASE + signature_start_ofs_max + len(signature))
    if signature_ofs >= 0:
      mode = ('short', 'long')[len(signature) == len(LONG_SIGNATURE1)]
      break
  else:
    raise ValueError('Missing bmcompress signature.')
  if (((load_addr or 0) + signature_ofs) & 0xf) != 0xc:
    raise ValueError(
        'bmcompress signature not aligned to 16-byte boundary + 12.')
  # http://wiki.osdev.org/Memory_Map_(x86)
  if load_addr is not None and load_addr < 0x500:
    raise ValueError('Code to be loaded too early in memory.')
  # http://wiki.osdev.org/Memory_Map_(x86)
  #
  # The actual limit is lower than 0x9fc00 (start of EBDA), we may need some
  # space for stack etc.
  if (load_addr or 0) + len(text) >= 0x9f000:
    raise ValueError('Code to be compressed too long.')

  load_ofs = signature_ofs + len(signature)
  if mode == 'short':
    load_addr += load_ofs
    if load_addr > 0xffff:
      raise ValueError('Code to be compressed too long for short mode.')
    if '--lzma' in method:
      raise ValueError('--lzma method not supported with short signature.')
  else:
    load_addr = None  # Not needed, long mode is relocatable.
    if signature == LONG_SIGNATURE2:
      method.append('--lzma')  # Also try the original method (--ultra-brute).

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
      # short reserved1[4];
      # short oem_id;
      # short oem_info;
      # short reserved2[10];
      # long  e_lfanew; // Offset to the 'PE\0\0' signature relative to the beginning of the file
  )

  if mode == 'short':
    compressed_after_code = ''
  else:
    # 00000040  A1CC00            mov ax,[0xcc]
    # 00000043  8ED0              mov ss,ax
    # 00000045  8B26CF00          mov sp,[0xcf]
    # 00000049  1F                pop ds
    # 0000004A  07                pop es
    # 0000004B  61                popaw
    # 0000004C  CF                iretw
    compressed_after_code = (
        'A1CC008ED08B26CF001F0761CF'.decode('hex'))
    assert len(compressed_after_code) == 13
  exe_size = 0x20 + len(text) - load_ofs + len(compressed_after_code)
  # Make the stack large enough so that there are a few bytes after the
  # code. The few bytes are useful in case there is an interrupt. The stack
  # is short-lived, it is used only for a few instructions after the
  # decompressor returns, but before the after_code changes the stack
  # pointer back.
  sp_magic = (exe_size + 0xf) >> 4
  exe_header = struct.pack(
      '<2sHH8sHH14s',
      'MZ',
      (exe_size & 511) or 512,
      (exe_size + 511) >> 9,
      '\x00\x00'  # nreloc.
      '\x02\x00'  # hdrsize.
      '\x01\x00'  # minalloc.
      '\x01\x00',  # maxalloc.
      0x200, # SS (before relocation).
      sp_magic,  # SP.
      '\x00\x00'  # Checksum.
      '\x00\x00'  # Initial IP.
      '\x00\x00'  # CS (before relocation).
      '\x00\x00\x00\x00\x00\x00\x00\x00',
  )
  assert len(exe_header) == 0x20
  #dump_struct(exe_header_fields, exe_header)
  open(tmp_filename, 'wb').write(''.join((
      exe_header,compressed_after_code, text[load_ofs:])))
  sys.stdout.flush()
  # !! What if can't improve with compression? Keep original.
  #    Or if file too small?
  # !! upx: liigmain.bin.tmp: NotCompressibleException
  # -qqq is totally quiet. -qq prints one line.
  subprocess.check_call(
      [(os.path.dirname(__file__) or '.') + '/tools/upx', '-qq'] +
      method +
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
  os.unlink(tmp_filename)
  data_ary = array.array('B', data[0x20:])
  if mode == 'long':
    assert h['nreloc'] <= 1, h['nreloc']  # We want it, for setting sp_addr.
    relocpos = h['relocpos']
    for _ in xrange(h['nreloc']):
      rofs, rseg = struct.unpack('<HH', data[relocpos : relocpos + 4])
      #print >>sys.stderr, 'info: relocation ofs=0x%x seg=0x%x' % (rofs, rseg)
      dofs = (rseg << 4) + rofs
      #xseg = (struct.unpack('<H', data_ary[dofs : dofs + 2])[0] + (load_addr >> 4)) & 0xffff
      # Original code:
      #   00023AE3  8D860000          lea ax,[bp+0x0]
      #   00023AE7  8ED0              mov ss,ax
      #   00023AE9  BC5020            mov sp,0x2050
      #   00023AEC  EA00000000        jmp word 0:0  ; relocation on the segment
      assert data_ary[dofs - 12 : dofs + 2].tostring() == struct.pack(
          '<7sH5s', '\x8d\x86\0\2\x8e\xd0\xbc', sp_magic,
          '\xea\x00\x00\x00\x00')
      # We don't need this relocation, because we don't want to adjust sp here.
      # We change it to:
      #   00000050  8CD8              mov ax,ds
      #   00000052  83C010            add ax,byte +0x10
      #   00000055  50                push ax
      #   00000056  6A00              push byte +0x0
      #   00000058  CB                retf
      data_ary[dofs - 12 : dofs + 2] = array.array(
          'B',
          '\x8c\xd8\x83\xc0\x10\x50\x6a\x00\xcb' +
          '\x90' * 5)  # nop...
      relocpos += 4
    # 0000000C  9C                pushfw
    # 0000000D  0E                push cs
    # 0000000E  0E                push cs  ; After this: $sp == 0x7b58
    # 0000000F  60                pushaw
    # 00000010  06                push es
    # 00000011  1E                push ds
    # 00000012  E80000            call word 0x15
    # 00000015  58                pop ax
    # 00000016  83C038            add ax,byte +0x38  ; After this: $ax == 0x806d
    # 00000019  89E5              mov bp,sp
    # 0000001B  894614            mov [bp+0x14],ax  ; $bp+0x14 == 0x7b58
    # 0000001E  8CCB              mov bx,cs
    # 00000020  C1E804            shr ax,byte 0x4
    # 00000023  01C3              add bx,ax
    # 00000025  8D47F0            lea ax,[bx-0x10]
    # 00000028  8ED8              mov ds,ax
    # 0000002A  8EC0              mov es,ax
    # 0000002C  8C16CC00          mov [0xcc],ss
    # 00000030  8926CF00          mov [0xcf],sp
    # 00000034  053412            add ax,0x1234  (with relocation h['ss'] + 0x10)
    # 00000037  8ED0              mov ss,ax
    # 00000039  BC0002            mov sp,0x200
    # 0000003C  53                push bx
    # 0000003D  6A00              push byte +0x0
    # 0000003F  CB                retf
    code_before = (
        '9C0E0E60061EE800005883C03889E58946148CCBC1E80401C38D47F08ED88EC08C16CC008926CF0005'.decode('hex') +
        struct.pack('<H', h['ss'] + 0x10) +
        '8ED0BC'.decode('hex') +
        struct.pack('<H', h['sp']) +
        '536A00CB'.decode('hex'))
    assert len(code_before) == 0x40 - 12
    code_after = ''
  elif mode == 'short':
    code_after_size = 8
    sp_addr = None
    assert h['nreloc'] == 1, h['nreloc']  # We want it, for setting sp_addr.
    relocpos = h['relocpos']
    for _ in xrange(h['nreloc']):
      rofs, rseg = struct.unpack('<HH', data[relocpos : relocpos + 4])
      #print >>sys.stderr, 'info: relocation ofs=0x%x seg=0x%x' % (rofs, rseg)
      dofs = (rseg << 4) + rofs
      # jmp word 0x...:0x...
      assert data_ary[dofs - 3 : dofs + 2].tostring() == '\xea\x00\x00\x00\x00'
      #00023AE3  8D860000          lea ax,[bp+0x0]
      #00023AE7  8ED0              mov ss,ax
      #00023AE9  BC5020            mov sp,0x2050
      assert data_ary[dofs - 12 : dofs - 3].tostring() == struct.pack(
          '<7sH', '\x8d\x86\0\2\x8e\xd0\xbc', sp_magic)  # ss, ...; mov sp, 0x....
      #sp_addr = dofs - 5 + load_addr
      data_ary[dofs - 12 : dofs + 2] = array.array(
          'B',
          '\x31\xc0' +  # xor ax, ax
          '\x8e\xd8' +  # mov ds, ax
          '\x8e\xc0' +  # mov es, ax
          struct.pack('<BHH', 0xea, load_addr - code_after_size, 0) +  # jmp word 0x...:0x...
          '\x90' * 3)
      #xseg = (struct.unpack('<H', data_ary[dofs : dofs + 2])[0] + (load_addr >> 4)) & 0xffff
      #data_ary[dofs : dofs + 2] = array.array('B', struct.pack('<H', xseg))
      relocpos += 4
    ss_addr_x = load_addr - 7
    sp_addr_x = load_addr - 2
    code_before = (  # Run before the on-the-fly decompression.
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
    code_after = (  # Run after the on-the-fly decompression.
        #'\x6a\x2b' +  # push '+'
        # These must be the last 8 bytes, ss_addr_x and sp_addr_x use them.
        struct.pack('<BH', 0xb8, 0) +  # mov ax, ...
        '\x8e\xd0' +  # mov ss, ax  ; Automatic cli for the next instr.
        struct.pack('<BH', 0xbc, 0) +  # mov sp, ...
        '')
    assert len(code_after) == code_after_size, len(code_after)
    assert len(code_before) + len(code_after) == 0x24
  else:
    raise AssertionError('Unknown mode: %s' % (mode,))
  return ''.join((
      text[:signature_ofs],  # Kept intact.
      code_before, code_after, data_ary.tostring()))


def main(argv):
  method = '--ultra-brute'
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
    elif arg.startswith('--method='):
      method = arg[arg.find('=') + 1:]
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
  text = bmcompress(text, load_addr, tmp_filename, method,
                    signature_start_ofs_max)
  # !! If compressed is longer than original, emit original.
  # ndisasm -b 16 -o $(LOAD_ADDR) -e 0x2c hiiimain.uncompressed.bin
  open(output_filename, 'wb').write(text)
  print >>sys.stderr, 'info: bmcompress output: %s (%d bytes)' % (
      output_filename, len(text))


if __name__ == '__main__':
  sys.exit(main(sys.argv))
