#! /usr/bin/python
# by pts@fazekas.hu at Thu Dec 21 23:56:14 CET 2017
#

import array
import os
import re
import struct
import subprocess
import sys
import time
import zipfile


def new_zipinfo(file_name, file_mtime, permission_bits=0644):
  zipinfo = zipfile.ZipInfo(file_name, file_mtime)
  zipinfo.external_attr = (0100000 | (permission_bits & 07777)) << 16
  return zipinfo


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
  do_add_install_zip = False
  image_filename = None
  install_filename = None
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
    if arg.startswith('--img='):
      image_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--install='):
      install_filename = arg[arg.find('=') + 1:]
    elif arg.startswith('--mtime='):
      mtime = int(arg[arg.find('=') + 1:], 0)
    elif arg == '--do-add-install-zip':
      do_add_install_zip = True
    else:
      sys.exit('fatal: unknown command-line flag: ' + arg)
  if image_filename is None:
    sys.exit('fatal: missing --bin=')
  if install_filename is None:
    sys.exit('fatal: missing --out=')
  if mtime is None:
    sys.exit('fatal: missing --mtime=')
  gmtime = time.gmtime(mtime)
  zip_output_basename = 'install.zip'

  # TODO(pts): Don't read the entire image (for 128 MiB of RAM use), but read
  # only the beginning and the end.
  img_data = open(image_filename).read()

  # This works in Python 2.6, 2.7 and 3.x.
  #
  # TODO(pts): Check sys.platform ('linux2'), also architecture (i386 and
  # x86_64). Should we?
  main_py_code = (
      'import os,sys,zipfile\n'
      'f=sys.argv[0]\n'
      'z=zipfile.ZipFile(f).open("' + zip_output_basename + '.install")\n'
      'g=f+".install"\n'
      'try:\n'
      ' os.remove(g)\n'
      'except OSError:\n'
      ' pass\n'
      # 493 == 0755. No octal for Python3.
      'd=os.open(g,os.O_WRONLY|os.O_CREAT|os.O_TRUNC,493)\n'
      'os.write(d,z.read())\n'
      'os.close(d)\n'
      'os.execl(g,g,"--rm",*sys.argv[1:])\n'
  )

  README_data = (
      'See https://liigboot.github.io/install\n')

  f = open('liigboot.img.install')
  try:
    install_data = f.read()
  finally:
    f.close()

  ziptmp_filename = image_filename + '.ziptmp'
  zf = zipfile.ZipFile(ziptmp_filename, 'w', zipfile.ZIP_DEFLATED)
  try:
    zf.writestr(new_zipinfo('__main__.py', gmtime), main_py_code)
    zf.writestr(new_zipinfo(zip_output_basename + '.install', gmtime, 0755), install_data)
    zf.writestr(new_zipinfo('README.txt', gmtime), README_data)
  finally:
    zf.close()

  subprocess.check_call(('tools/advzip', '-qz4', '--', ziptmp_filename))
  print >>sys.stderr, 'info: created ziptmp %s (%d bytes)' % (
      ziptmp_filename, os.stat(ziptmp_filename).st_size)

  ziptmp_data = open(ziptmp_filename).read()
  ziptmp_sector_count = (len(ziptmp_data) + 511) // 512
  os.remove(ziptmp_filename)
  if (len(ziptmp_data) > len(img_data) or
      img_data[-len(ziptmp_data):].rstrip('\0')):
    raise ValueError('No room for ziptmp_data in the image.')
  data = img_data[:-len(ziptmp_data)] + ziptmp_data

  if do_add_install_zip:
    # Add install.zip to the FAT. It works only of FAT12 created by
    # liigboot_boot.nasm.

    if (data[:3] != '\xeb\x3c\x90' or
        data[0x36 : 0x3c] not in ('FAT12 ', 'FAT16 ')):
      raise ValueError(
          'Expected liigboot_bs.bin or liigresc_bs.bin FAT header.')
    fields = (
        ('sector_size', 'H'),
        ('sectors_per_cluster', 'B'),
        ('reserved_sector_count', 'H'),
        ('fat_count', 'B'),
        ('rootdir_entry_count', 'H'),
        ('sector_count', 'H'),
        ('media_descriptor', 'B'),
        ('sectors_per_fat', 'H'),
        ('sectors_per_track', 'H'),
        ('heads', 'H'),
        ('hidden', 'L'),
        ('sector_count2', 'L'),
    )
    h = dump_header(fields, data[0xb : 0x24])
    if h['sector_count']:
      h['sector_count3'] = h['sector_count']
    else:
      h['sector_count3'] = h['sector_count2']
    h['fat_bits'] = (12, 16)[data[0x36 : 0x3c] == 'FAT16 ']

    if (h['fat_bits'] not in (12, 16) or
        h['sector_size'] != 512 or
        #h['sectors_per_cluster'] != 1 or
        #h['reserved_sector_count'] != 80 or
        h['fat_count'] != 1 or
        #h['rootdir_entry_count'] != 0x20 or
        #h['sector_count3'] != 0x800 or
        #h['sectors_per_fat'] != 6 or  # !! Is it the smallest possible?
        0):
      raise ValueError(
          'Expected liigboot_bs.bin or liigresc_bs.bin FAT fields.')
    h['cluster_count'] = (
        (h['sector_count3'] - h['fat_count'] * h['sectors_per_fat'] -
         ((h['rootdir_entry_count'] + 15) >> 4) -
         h['reserved_sector_count']) / h['sectors_per_cluster'])
    if h['cluster_count'] <= 0:
      raise ValueError(h['cluster_count'])
    #assert 00, (h['cluster_count'], ziptmp_sector_count)

    ziptmp_cluster_count = (
        (ziptmp_sector_count + h['sectors_per_cluster'] - 1) /
        h['sectors_per_cluster'])
    direntry_file_size = ziptmp_sector_count << 9
    direntry_start_cluster = h['cluster_count'] + 2 - ziptmp_cluster_count

    if h['fat_bits'] == 12:
      def fat12_get(cluster_idx):
        """Returns next_cluster_idx or one of the special values.

        (http://www.dfists.ua.es/~gil/FAT12Description.pdf)
        0x000 Free (unused) cluster.
        0xFF0-0xFF6 Reserved cluster.
        0xFF7 Bad cluster.
        0xFF8-0xFFF Last cluster in a file.
        """
        if not (2 <= cluster_idx < h['cluster_count'] + 2):
          raise ValueError
        j = (h['reserved_sector_count'] << 9) + (cluster_idx >> 1) * 3
        a, b, c = struct.unpack('<BBB', data[j : j + 3])
        if cluster_idx & 1:
          return (c << 4 | b >> 4)
        else:
          return ((b & 0xf) << 8) | a

      def fat12_set(cluster_idx, next_cluster_idx):
        if not (2 <= cluster_idx < h['cluster_count'] + 2):
          raise ValueError
        if next_cluster_idx == - 1:
          next_cluster_idx = 0xfff
        elif not (2 <= next_cluster_idx < h['cluster_count'] + 2):
          raise ValueError(next_cluster_idx)
        j = (h['reserved_sector_count'] << 9) + (cluster_idx >> 1) * 3
        if cluster_idx & 1:
          data[j + 2] = next_cluster_idx >> 4
          data[j + 1] = (data[j + 1] & 0xf) | (next_cluster_idx & 0xf) << 4
        else:
          data[j] = next_cluster_idx & 0xff
          data[j + 1] = (data[j + 1] & 0xf0) | (next_cluster_idx >> 8)

      fat_get, fat_set = fat12_get, fat12_set
    else:
      assert h['fat_bits'] == 16

      def fat16_get(cluster_idx):
        """Returns next_cluster_idx or one of the special values."""
        if not (2 <= cluster_idx < h['cluster_count'] + 2):
          raise ValueError((cluster_idx, h['cluster_count'] + 2))
        j = (h['reserved_sector_count'] << 9) + (cluster_idx << 1)
        return struct.unpack('<H', data[j : j + 2])[0]

      def fat16_set(cluster_idx, next_cluster_idx):
        if not (2 <= cluster_idx < h['cluster_count'] + 2):
          raise ValueError
        if next_cluster_idx == - 1:
          next_cluster_idx = 0xffff
        elif not (2 <= next_cluster_idx < h['cluster_count'] + 2):
          raise ValueError(next_cluster_idx)
        j = (h['reserved_sector_count'] << 9) + (cluster_idx << 1)
        data[j + 1] = next_cluster_idx >> 8
        data[j] = next_cluster_idx & 0xff

      fat_get, fat_set = fat16_get, fat16_set

    # Add the new file to the FAT.
    data = array.array('B', data)
    if ziptmp_cluster_count > 0:
      last = direntry_start_cluster + ziptmp_cluster_count - 1
      for i in xrange(direntry_start_cluster, last + 1):
        if fat_get(i) != 0:
          raise ValueError('Last few FAT entries already occupied.')
        if i == last:
          fat_set(i, -1)  # Last cluster in file.
        else:
          fat_set(i, i + 1)
    data = data.tostring()

    # https://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html
    #
    # Byte 12 in the directory entry can have two bits set in VFAT to indicate all lower case:
    # #define LCASE_BASE        0x08            // filename base in lower case
    # #define LCASE_EXT         0x10            // filename extension in lower case
    #
    # A direntry is 32 bytes.
    # TODO(pts): Preserve modification time.
    if gmtime[0] < 1980:
      raise ValueError(gmtime)
    ATTRIB_SYSTEM = 0x4
    ATTRIB_ARCHIVE = 0x20
    LCASE_BASE = 0x8
    LCASE_EXT = 0x10
    filename83 = 'INSTALL ZIP'
    direntry_new = struct.pack(
        '<11sBB9sLHL',
        filename83,
        ATTRIB_SYSTEM | ATTRIB_ARCHIVE,
        LCASE_BASE | LCASE_EXT,
        '\0\xf8\xbd\x95\x4b\x95\x4b\0\0',
        gmtime[3] << 11 | gmtime[4] << 5 | gmtime[5] >> 1 | (gmtime[0] - 1980) << 25 | gmtime[1] << 21 | gmtime[2] << 16,
        direntry_start_cluster, direntry_file_size)

    # Add entry to FAT root directory.
    root_dir_ofs = (
        h['reserved_sector_count'] +
        h['fat_count'] * h['sectors_per_fat']) << 9
    for i in xrange(root_dir_ofs,
                    root_dir_ofs + (h['rootdir_entry_count'] << 5), 32):
      direntry = data[i : i + 32]
      if direntry[0] in '\xe5\0':  # '\xe5' marks deleted.
        data = data[:i] + direntry_new + data[i + 32:]
        break
    else:
      raise ValueError('No free root directory entry.')

  f = open(image_filename, 'w')
  try:
    f.write(data)
  finally:
    f.close()
  print >>sys.stderr, 'info: modified to zip %s (%d bytes)' % (
      image_filename, os.stat(image_filename).st_size)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
