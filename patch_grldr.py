#! /usr/bin/python
# by pts@fazekas.hu at Mon Dec 25 16:27:10 CET 2017

"""Patch some binary code change and the default config for GRUB4DOS grldr."""

import array
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


def main(argv):
  input_filename = output_filename = menu_filename = loader_filename = None
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
    )
    data = patch_code(data, patches)
    data = patch_menu(data, menu_data)
    data = patch_loader(data[0x2000:], loader_data)

    print >>sys.stderr, 'info: creating grldr file: %s' % output_filename
    open(output_filename, 'wb').write(data)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
