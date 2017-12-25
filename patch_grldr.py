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
  for line in menu_data.replace('\r', '\n').split('\n'):
    line = line.strip()
    if '\0' in line:
      raise ValueError
    if line and not line.startswith('##'):
      cmd = line.split(None, 1)[0].lower()
      if cmd in ('title', 'label'):
        output.append('\n')
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


def main(argv):
  input_filename = output_filename = menu_filename = None
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
    else:
      sys.exit('fatal: unknown command-line flag: ' + arg)
  if output_filename is None:
    sys.exit('fatal: missing --out=')
  if menu_filename is None:
    sys.exit('fatal: missing --menu=')

  menu_data = open(menu_filename, 'rb').read()
  menu_data = simplify_menu(menu_data)

  if input_filename is None:
    print >>sys.stderr, 'info: creating menu file: %s' % output_filename
    open(output_filename, 'wb').write(menu_data)
  else:
    data = open(input_filename, 'rb').read()
    if not data.startswith('\xeb\x3e\x80'):
      raise ValueError('Not a grldr file.')
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
    print >>sys.stderr, 'info: creating grldr file: %s' % output_filename
    open(output_filename, 'wb').write(data)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
