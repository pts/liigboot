#! /usr/bin/python
# by pts@fazekas.hu at Thu Dec 21 23:56:14 CET 2017
#

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


def main(argv):
  do_add_install_zip = False
  if len(argv) > 1 and argv[1] == '--do-add-install-zip':
    do_add_install_zip = True
    del argv[1]
  if len(argv) > 1 and argv[1].startswith('--mtime='):
    mtime = time.gmtime(int(argv[1][argv[1].find('=') + 1:], 0))
    del argv[1]
  else:
    mtime = time.localtime()[:6]
  if len(argv) < 2:
    img_file_name = 'liigboot.img'
  else:
    img_file_name = argv[1]
  zip_output_file_name = os.path.splitext(img_file_name)[0] + '.zip'
  zip_output_base_name = os.path.basename(zip_output_file_name)
  # Sanitize for the ZipFile open call below.
  if not re.match(r'\A[-_+.a-zA-Z0-9]+\Z', zip_output_base_name):
    raise ValueError(zip_output_base_name)
  img_data = open(img_file_name).read()

  # This works in Python 2.6, 2.7 and 3.x.
  #
  # TODO(pts): Check sys.platform ('linux2'), also architecture (i386 and
  # x86_64). Should we?
  main_py_code = (
      'import os,sys,zipfile\n'
      'f=sys.argv[0]\n'
      'z=zipfile.ZipFile(f).open("' + zip_output_base_name + '.install")\n'
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

  ziptmp_file_name = img_file_name + '.ziptmp'
  zf = zipfile.ZipFile(ziptmp_file_name, 'w', zipfile.ZIP_DEFLATED)
  try:
    zf.writestr(new_zipinfo('__main__.py', mtime), main_py_code)
    zf.writestr(new_zipinfo(zip_output_base_name + '.install', mtime, 0755), install_data)
    zf.writestr(new_zipinfo('README.txt', mtime), README_data)
  finally:
    zf.close()

  subprocess.check_call(('tools/advzip', '-qz4', '--', ziptmp_file_name))
  print >>sys.stderr, 'info: created ziptmp %s (%d bytes)' % (
      ziptmp_file_name, os.stat(ziptmp_file_name).st_size)

  ziptmp_data = open(ziptmp_file_name).read()
  ziptmp_sector_count = (len(ziptmp_data) + 511) // 512
  os.remove(ziptmp_file_name)
  if (len(ziptmp_data) > len(img_data) or
      img_data[-len(ziptmp_data):].rstrip('\0')):
    raise ValueError('No room for ziptmp_data in the image.')
  data = img_data[:-len(ziptmp_data)] + ziptmp_data

  if do_add_install_zip:
    # Add install.zip to the FAT. It works only of FAT12 created by
    # liigboot_boot.nasm.

    direntry_file_size = ziptmp_sector_count << 9
    direntry_start_cluster = 1962 - ziptmp_sector_count
    if ziptmp_sector_count == 3:  # FAT12 cluster chain.
      i, new = 0xab7a, '\x80\x7a\xa9\xf7\xff\0'
    elif ziptmp_sector_count == 4:
      i, new = 0xab79, '\xa7\x87\x7a\xa9\xf7\xff\0'
    elif ziptmp_sector_count == 5:
      i, new = 0xab77, '\x60\x7a\xa7\x87\x7a\xa9\xf7\xff\0'
    elif ziptmp_sector_count == 6:
      i, new = 0xab76, '\xa5\x67\x7a\xa7\x87\x7a\xa9\xf7\xff\x00'
    elif ziptmp_sector_count == 7:
      i, new = 0xab74, '\x40\x7a\xa5\x67\x7a\xa7\x87\x7a\xa9\xf7\xff\x00'
    elif ziptmp_sector_count == 8:
      i, new = 0xab73, '\xa3\x47\x7a\xa5\x67\x7a\xa7\x87\x7a\xa9\xf7\xff\x00'
    elif ziptmp_sector_count == 9:
      i, new = 0xab71, '\x20\x7a\xa3\x47\x7a\xa5\x67\x7a\xa7\x87\x7a\xa9\xf7\xff\x00'
    else:
      raise NotImplementedError('Not implemented --add-install-zip for %d sectors.' % ziptmp_sector_count)
    if data[i : i + len(new)].rstrip('\0'):
      raise ValueError('Last few FAT12 entries already occupied.')
    data = data[:i] + new + data[i + len(new):]
    # https://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html
    #
    # Byte 12 in the directory entry can have two bits set in VFAT to indicate all lower case:
    # #define LCASE_BASE        0x08            // filename base in lower case
    # #define LCASE_EXT         0x10            // filename extension in lower case
    #
    # A direntry is 32 bytes.
    # TODO(pts): Preserve modification time.
    if mtime[0] < 1980:
      raise ValueError(mtime)
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
        mtime[3] << 11 | mtime[4] << 5 | mtime[5] >> 1 | (mtime[0] - 1980) << 25 | mtime[1] << 21 | mtime[2] << 16,
        direntry_start_cluster, direntry_file_size)
    for i in xrange(0xac00, 0xb000, 32):  # 1024 bytes of FAT root directory.
      direntry = data[i : i + 32]
      if direntry[0] in '\xe5\0':  # '\xe5' marks deleted.
        data = data[:i] + direntry_new + data[i + 32:]
        break
    else:
      raise ValueError('No free root directory entry.')

  f = open(zip_output_file_name, 'w')
  try:
    f.write(data)
  finally:
    f.close()
  print >>sys.stderr, 'info: created zip %s (%d bytes)' % (
      zip_output_file_name, os.stat(zip_output_file_name).st_size)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
