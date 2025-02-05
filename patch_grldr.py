#! /usr/bin/python
# by pts@fazekas.hu at Mon Dec 25 16:27:10 CET 2017

"""Patch some binary code change and the default config for GRUB4DOS grldr."""

import array
import os
import os.path
import stat
import subprocess
import sys


def patch_code(data, patches):
  data_ary = array.array('B', data)
  for ofs, old, new in patches:
    if not isinstance(old, array.array):
      old = array.array('B', old)
    if not isinstance(new, array.array):
      new = array.array('B', new)
    if len(old) != len(new):
      raise ValueError
    if not old:
      raise ValueError
    if ofs + len(old) > len(data_ary):
      raise ValueError
    if data_ary[ofs : ofs + len(old)] != old:
      raise ValueError
    data_ary[ofs : ofs + len(old)] = new
  return data_ary.tostring()


def simplify_menu(menu_data):
  """Simplifies a GRUB menu.lst or a Syslinux syslinux.cfg file.

  Removes whitespace and ## comments."""
  output = []
  do_remove = True
  for line in menu_data.replace('\r', '\n').split('\n'):
    line = line.strip()
    if '\0' in line:
      raise ValueError
    if line and not line.startswith('##'):
      cmd = line.split(None, 1)[0].lower()
      if cmd in ('title', 'label'):
        do_remove = False
        output.append('\n')
      # This is not needed by GRUB4DOS anymore, removing it.
      if not (do_remove and line == 'root (hd0)'):
        output.append(line + '\n')
  return ''.join(output)


def patch_menu(data, menu_data):
  if not data.endswith('\0'):
    raise ValueError
  if '\0' in menu_data:
    raise ValueError
  i = data.rfind('\0', 0, len(data) - 1)
  if i < 0:
    raise ValueError
  assert i < len(data) - 1
  return ''.join((data[:i + 1], menu_data, '\0'))


def patch_loader(stage2_data, loader_data):
  if not loader_data.startswith('\xeb\x3c\x90'):
    raise ValueError('Missing loader signature.')
  if len(loader_data) != 0x600:
    raise ValueError('Bad loader size.')
  if (stage2_data[0x00 : 0x00 + 2] != '\xea\x70' or
      stage2_data[0x06 : 0x06 + 2] != '\3\2' or
      stage2_data[0x12 : 0x12 + 5] != '0.97\0' or
      stage2_data[0x10] != '\0'):
    raise ValueError('Missing pre_stage2.')
  if not stage2_data.endswith('\0'):
    raise ValueError('Missing trailing \\0.')

  # Layout of GRUB pre_stage2.
  #
  # It is based in stage2/asm.S, stage2/builtins.c etc. in the GRUG4DOS sources.
  #
  # 0x00: jmp word 0:0x8270
  # 0x05: db 0
  #        bit 0 = 1: disable pxe in stage2/common.c: if (! ((*(char *)0x8205) & 0x01))   /* if it is not disable pxe */
  #        bit 1 = 1: disable keyboard intervention in boot process
  #        bit 2 = 1: disable the "unconditional command-line entrance" (UCE) feature; disabling it is DUCE
  #        bit 3 = 1: disable geometry tune; chs-no-tune; don't call geometry_tune !! what does it mean?
  # 0x06: db 3, 2  ; Compatibility version number: COMPAT_VERSION_MAJOR, COMPAT_VERSION_MINOR
  #       from stage2/builtins.c:
  #       (*((short *) (stage2_second_buffer + STAGE2_VER_MAJ_OFFS)) != COMPAT_VERSION)
  # 0x08: dd 0xffffff  ; install_partition; STAGE2_INSTALLPART !! how is this used?
  #       the paritition number is acually at the byte 0x0a (0-based offset)
  #        grub_printf ("(%cd%d", (current_drive & 0x80) ? 'h' : 'f', current_drive & ~0x80);
  #        if ((current_partition & 0xFF0000) != 0xFF0000)
  #            grub_printf (",%d", (current_partition >> 16) & 0xFF);  /* partition number */
  #        if ((current_partition & 0x00FF00) != 0x00FF00)
  #            grub_printf (",%c", 'a' + ((current_partition >> 8) & 0xFF));
  #        grub_printf (")");
  #       printf ("Partition type for (hd%d,%d) is 0x%X.\n", (current_drive & 0x7F), (current_partition >> 16), new_type);
  #       movb    %dh, 0x820A     /* this is the boot partition number */
  #       #define STAGE2_INSTALLPART      0x8
  # 0x0c: dd 0x84cc  ; saved_entryno pointing to preset_menu  ; !! GRUB4DOS-specific commandline preset_menu
  #       !! what is the built-in preset_menu??
  #       !! it's also called saved_entryno, STAGE2_SAVED_ENTRYNO
  #       !! (0 = cleared; common.c)
  #       mem addr 0x84cc is at file offset 0x8cc: dd 0x3b25c
  #       preset_menu is at file offset 0x3366c; mem addr 0x3b26c
  #       \x6c\xb2\x03\x00
  #       Note: GRUB for DOS uses this for the commandline preset_menu.
  #       A preset_menu can be embedded in the commandline of GRUB.EXE.
  #       This new preset_menu overrides the built-in preset_menu.
  #       If the variable is not touched, and the first byte at config_file is 0,
  #       then the new menu at 0x0800 will work.
  #       If the variable here is cleared to 0, or the first byte at config_file is
  #       not 0, then the built-in preset_menu will work.
  #       ; clear saved_entryno so that force_cdrom_as_boot_device be cleared
  #       ; later in common.c
  #       movl    %ebp, 0x820C    /* EBP=0, clear saved_entryno */
  #       #define STAGE2_SAVED_ENTRYNO    0xc
  # 0x10: db 0  ; STAGE2_ID_STAGE2
  # 0x11: db 0  ; force_lba
  # 0x12: db '0.97', 0  ; Version string. Ubuntu stage2 has the same.
  #       #define STAGE2_VER_STR_OFFS     0x12
  # 0x17: db '/boot/grub/menu.lst', 0  ; Default config file name. The command `configfile' without arguments will load this as default argument. May or may not start with (hd0)/... etc.
  # 0x2b: db 0, ...
  # 0x6c: dd 0x3b25c  ; __bss_start, same as edata
  # 0x70: db 0xeb, 0x4e  ; jmp short 0x82c0 ; real_codestart
  # 0x72: ... padding of db 0, ...
  # 0x80: dd 0  ; boot_drive  (will be set in asm.S)
  # 0x84: dd 0  ; pxe_yip
  # 0x88: dd 0  ; pxe_sip
  # 0x8c: dd 0  ; pxe_gip
  # 0x90: dq 0  ; filesize
  # 0x94: dd 0  ; saved_mem_upper
  # 0x98: dd 0  ; saved_partition
  # 0x9c: dd 0  ; saved_drive
  # 0xa0: dd 0  ; no_decompression
  # 0xa4: ... padding of db 0, ...
  # 0xc0: 0xfa, 0xfc ; cli; cld ... boot code starts here


  # Also called preset_menu and PRESENT_MENU_STRING in the GRUB4DOS sources.
  fallback_menu_ofs = stage2_data.rfind('\0', 0, len(stage2_data) - 1) + 1
  if fallback_menu_ofs <= 0:
    raise ValueError('Missing beginning of fallback_menu.')
  # fallback_menu_addr = fallback_menu_ofs + 0x8200
  stage2_data_ary = array.array('B', stage2_data)

  def set_in_stage2(ofs, data):
    stage2_data_ary[ofs : ofs + len(data)] = array.array('B', data)

  stage2_data_ary[0x5] |= (
       1  |  # Disable PXE.
       8)  # Disable geometry-tune.
  stage2_data_ary[0x11] = 1  # Force LBA.
  # Clear saved_entryno, useless. First I thought it was useless,
  # but it has the positive side effect of an implicit `root (hd0)'.
  set_in_stage2(0xc, '\0\0\0\0')
  # Default `configfile' argument.
  set_in_stage2(0x17, '/menu.lst\0\0\0\0\0\0\0\0\0\0\0')
  return loader_data + stage2_data_ary.tostring()


def call_compress_flat16(filename):
  mydir = os.path.dirname(__file__) or '.'
  cmd = (
      sys.executable, '-E',  # Run the same Python interpreter which runs us.
      '%s/tools/upxbc' % mydir,
      '--upx=%s/tools/upx' % mydir,
      '--flat16', '--load-addr=0x7c00', '--sig-ofs-max=0x1f0', '--', filename)
  try:
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE)
  except OSError:
    raise RuntimeError('Program not found: %s' % cmd[0])
  try:
    p.communicate('')
  finally:
    exit_code = p.wait()
  if exit_code:
    raise RuntimeError('Program failed with exit_code=0x%x.' % exit_code)


def main(argv):
  input_filename = output_filename = menu_filename = loader_filename = None
  do_compress = False
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
    if arg.startswith('--in='):
      input_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--out='):
      output_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--menu='):
      menu_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--loader='):
      loader_filename = arg[arg.find('=') + 1:]
    elif arg == '--do-compress':
      do_compress = True
    else:
      sys.exit('fatal: unknown command-line flag: ' + arg)
  if output_filename is None:
    sys.exit('fatal: missing --out=')
  if menu_filename is None:
    sys.exit('fatal: missing --menu=')
  if input_filename is not None and loader_filename is None:
    sys.exit('fatal: missing --loader=')

  menu_data = open(menu_filename, 'rb').read()
  menu_data = simplify_menu(menu_data)

  if input_filename is None:
    print >>sys.stderr, 'info: creating menu file: %s' % output_filename
    open(output_filename, 'wb').write(menu_data)
  else:
    loader_data = open(loader_filename, 'rb').read()
    if not loader_data.startswith('\xeb\x3c\x90'):
      raise ValueError('Missing loader signature.')
    if len(loader_data) != 0x600:
      raise ValueError('Bad loader size.')
    data = open(input_filename, 'rb').read()
    if not data.startswith('\xeb\x3e\x80'):
      raise ValueError('Missing grldr signature.')
    if (data[0x2000 : 0x2000 + 2] != '\xea\x70' or
        data[0x2006 : 0x2006 + 2] != '\3\2' or
        data[0x2012 : 0x2012 + 5] != '0.97\0'):
      raise ValueError('Missing pre_stage2.')

    patches = (
        # This is to prevent the error message
        # "The BPB hidden_sectors should not be zero..." (ERR_HD_VOL_START_0).
        # Without this patch `chainloader (hd0)+1' wouldn't work. The patch
        # replaces a `jnz short' with a `jmp short'.
        #
        # This is the first instance reported by
        # grub4dos-0.4.4-src/stage/builtins.c.
        (0x17623, '\x85\xc0\x75', '\x85\xc0\xeb'),
        # This is the second instance of ERR_HD_VOL_START_0 reported by
        # grub4dos-0.4.4-src/stage/builtins.c.
        # Another instance of ERR_HD_VOL_START_0.
        (0x17869, '\x85\xc0\x75', '\x85\xc0\xeb'),
        # Make (fd0,0)/... work, removing the `!(current_drive & 0x80)'
        # check from stage2/disk_io.c in function set_device.
        (0x1a080, '\x0f\x89\x2b\x01\x00\x00', '\x90\x90\x90\x90\x90\x90'),
        # Make (fd0,0)/... work, removing the `!(current_drive & 0x80)'
        # check from stage2/disk_io.c in function sane_partition.
        # Call to sane_partition() inline 3 times.
        (0x1a18a, '\x78\x0c', '\xeb\x0c'),
        (0x1abee, '\x78\x0c', '\xeb\x0c'),
        (0x1b009, '\x78\x0c', '\xeb\x0c'),
        # Don't check (current_drive & 0x80) in real_open_partition.
        # This makes `cat (fd0', <Tab> work.
        (0x1ad09, '\x79\x31', '\x90\x90'),
        # Don't check (! (current_drive & 0x80)) in real_open_partition.
        # This makes `using whole disk' appear in `cat (fd0', <Tab>.
        (0x1aeae, '\x78\x0f', '\x90\x90'),
        # Also allow `allow *unique_string == 'f'' in disk_io.c:1628
        # This makes `cat (fd0' <Tab> show a comma.
        # See also grub4dos_fd0_tab_patch.nasm
        (0x1b78e, 'QQR\x8dX\x01S\xe8\xb2\xcf\xff\xff\x83\xc4\x10\x83=\x98\xbe\x03\x00\x01\x0f\x85\xc4\x01\x00\x00\x83\xec\x0cS\xe8m\xcd\xff\xff\x83\xc4\x10\x8d\x14\x03\xa1\x9c\xbe\x03\x00\x808hu\x05\xc6\x02,\xeb]\xc6\x02)\xebX', 'R\x8dX\x01S\xe8\xb4\xcf\xff\xffYY\x83=\x98\xbe\x03\x00\x01\x0f\x85\xc7\x01\x00\x00S\xe8s\xcd\xff\xffY\x8d\x14\x03\xa1\x9c\xbe\x03\x00\xc6\x02)\x808ft\x05\x808hub\xc6\x02,\xeb]\x90\x90\x90\x90\x90'),
    )
    data = patch_code(data, patches)
    data = patch_menu(data, menu_data)
    data = patch_loader(data[0x2000:], loader_data)
    open(output_filename, 'wb').write(data)
    if do_compress:
      tmp_filename = output_filename + '.tmp'
      print >>sys.stderr, (
          'info: uncompressed grub4dos.bs file would be %d bytes long' %
          len(data))
      try:
        call_compress_flat16(output_filename)
      except:
        if os.path.isfile(output_filename):
          os.remove(output_filename)
        raise
    print >>sys.stderr, 'info: created grub4dos.bs file: %s (%d bytes)' % (
        output_filename, os.stat(output_filename).st_size)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
