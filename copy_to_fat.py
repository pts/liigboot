#! /usr/bin/python
# by pts@fazekas.hu at Mon Dec 25 16:27:10 CET 2017

"""Copy a single file to a FAT filesystem image, using mtools, preserving
mtime."""

import os
import pipes
import subprocess
import sys


def main(argv):
  input_filename = output_filename = image_filename = mtime = None
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
      output_filename = arg[arg.find('=') + 1:].strip('/')
    elif arg.startswith('--img='):
      image_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--mtime='):
      mtime = int(arg[arg.find('=') + 1:], 0)
    else:
      sys.exit('fatal: unknown command-line flag: ' + arg)
  if input_filename is None:
    sys.exit('fatal: missing --in=')
  if output_filename is None:
    sys.exit('fatal: missing --out=')
  if image_filename is None:
    sys.exit('fatal: missing --img=')
  if mtime is None:
    sys.exit('fatal: missing --mtime=')

  tmp_filename = 'mcopy.tmp'
  data = open(input_filename, 'rb').read()  # TODO(pts): Read less.
  open(tmp_filename, 'wb').write(data)
  del data  # Save memory.
  os.utime('mcopy.tmp', (mtime, mtime))
  # `-D o' to allow overwrites. TODO(pts): Silently fail.
  cmd = ((os.path.dirname(__file__) or '.') + '/tools/mtools',
      '-c', 'mcopy',
      '-m',  # Keep mtime.
      '-i', image_filename,
      '-D', 'o',
      tmp_filename,
      '::' + output_filename)
  print >>sys.stderr, 'info: running: %s' % ' '.join(map(pipes.quote, cmd))
  subprocess.check_call(cmd, stdin=open('/dev/null'))
  os.unlink(tmp_filename)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
