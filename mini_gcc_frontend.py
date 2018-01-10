#! /usr/bin/python
# by pts@fazekas.hu at Wed Jan 10 00:08:29 CET 2018

"""A minimalistic GCC frontend for static compilation.

Calls cc1 -E, cc1, as and ld, like /usr/bin/gcc does.
"""

import os
import os.path
import pipes
import subprocess
import sys

tmp_filenames = []

def get_tmp_filename(suffix='', _seq=[0]):
  # This method is not thread-safe.
  # TODO(pts): Support Windows.
  tmpdir = os.getenv('TMPDIR', '/tmp')
  seq = _seq[0] = _seq[0] + 1
  filename = os.path.join(
      tmpdir, 'minigcc.%d.%d%s' % (os.getpid(), seq, suffix))
  tmp_filenames.append(filename)
  # !! TODO(pts): Recover from O_EXCL creation failure.
  os.close(os.open(
      filename, os.O_WRONLY | os.O_CREAT | os.O_TRUNC | os.O_EXCL, 0600))
  return filename


def run_command(cmd, cmd_name, is_verbose):
  if is_verbose:
    sys.stderr.write('minigcc: info: run %s: %s\n' %
                     (cmd_name, ' '.join(map(pipes.quote, cmd))))
  exit_code = subprocess.call(cmd)
  if exit_code:
    sys.stderr.write('minigcc: fatal: prog %s failed\n' % cmd_name)
    sys.exit(1)


def main(argv):
  # For consistent behavior (e.g. error messages) across locales.
  os.environ['LC_ALL'] = 'C'
  tools_dir = (os.path.dirname(__file__) or '.') + '/tools'
  if not os.path.isfile(tools_dir + '/cc1'):
    sys.exit('bad tools dir: %s' % tools_dir)

  missing_required_flags = set((
      '-static', '-nostdinc', '-nostdlib', '-m32'))
  asm_files = []  # .s
  asmcpp_files = []  # .S
  c_files = []  # .c
  object_files = []  # .o
  library_files = []  # .a
  source_files = []
  linkable_files = []
  mode = None
  output_files = []
  cc1_flags = []
  libdir_flags = []
  ld_flags = []
  as_flags = []
  cpp_flags = []
  cpp_only_flags = []
  is_verbose = False

  argi = 1
  while argi < len(argv):
    arg = argv[argi]
    argi += 1
    if not arg.startswith('-') or arg == '-':
      if arg.endswith('.s'):
        source_files.append(arg)
        asm_files.append(arg)
        linkable_files.append(None)
      elif arg.endswith('.S'):
        source_files.append(arg)
        asmcpp_files.append(arg)
        linkable_files.append(None)
      elif arg.endswith('.c'):
        source_files.append(arg)
        c_files.append(arg)
        linkable_files.append(None)
      elif arg.endswith('.o'):
        object_files.append(arg)
        linkable_files.append(arg)
      elif arg.endswith('.a'):
        library_files.append(arg)
        linkable_files.append(arg)
      else:
        sys.exit('unknown source file type: %s' % arg)
      continue
    if arg in ('-static', '-nostdinc', '-nostdlib', '-m32'):
      missing_required_flags.discard(arg)

    if (arg in ('-fPIE', '-p', '-pg', '--sysroot', '--gcc-toolchain', '-target',
                '-m64', '-sysld', '--sysld', '-pie', '-fpic', '-fPIC', '-fpie',
                '-fPIE', '-shared', '-shared-libgcc', '-xstatic', '--xstatic',
                '-iwithprefix', '-isysroot', '-multilib', '-L---', '--') or
        arg.startswith('-M') or
        arg.startswith('-B') or
        (arg.startswith('-W') and arg.find(',', 2) >= 2 and
         arg[2] not in 'al') or
        arg.startswith('--sysroot=') or
        arg.startswith('--gcc-toolchain=...')):
      sys.exit('flag not supported: %s' % arg)
    elif arg in ('-c', '-S', '-E'):
      if mode is not None:
        # gcc allows multiple.
        sys.exit('conflicting modes: %s vs %s' % (mode, arg))
      mode = arg
    elif arg in ('-static',):
      ld_flags.append(arg)
    elif arg in ('-nostdinc',):
      pass
    elif arg in ('-nostdlib', '-s'):
      ld_flags.append(arg)
    elif arg in ('-m32',):
      cpp_flags.append(arg)
      cc1_flags.append(arg)
    elif arg == '-W':
      cc1_flags.append('-Wextra')
    elif (arg in ('-ansi', '-pedantic', '-g', '-g0', '-W') or
          arg.startswith('-std=') or
          arg.startswith('-O') or
          (arg.startswith('-m') and len(arg) > 2 and arg != '-m32') or
          (arg.startswith('-f') and len(arg) > 2) or
          (arg.startswith('-W') and len(arg) > 2 and ',' not in arg)):
      cc1_flags.append(arg)
    elif arg == '-dM':
      cpp_only_flags.append(arg)
    elif arg.startswith('-o'):
      if output_files:
        sys.exit('multiple output files (-o)')
      if len(arg) == 2:
        if argi == len(argv):
          sys.exit('missing argument for flag: %s' % arg)
        output_files.append(argv[argi])
        argi += 1
      else:
        output_files.append(arg[2:])
    elif arg.startswith('-L'):
      if len(arg) == 2:
        if argi == len(argv):
          sys.exit('missing argument for flag: %s' % arg)
        if not argv[argi]:
          sys.exit('empty argument for flag: %s' % arg)
        libdir_flags.append(arg + argv[argi])
        argi += 1
      else:
        libdir_flags.append(arg)
    elif (arg.startswith('-I') or arg.startswith('-D') or arg.startswith('-U')):
      if len(arg) == 2:
        if argi == len(argv):
          sys.exit('missing argument for flag: %s' % arg)
        if not argv[argi]:
          sys.exit('empty argument for flag: %s' % arg)
        cpp_flags.append(arg + argv[argi])
        cc1_flags.append(arg + argv[argi])
        argi += 1
      else:
        cpp_flags.append(arg)
        cc1_flags.append(arg)
    elif arg == '-isystem':
      if argi == len(argv):
        sys.exit('missing argument for flag: %s' % arg)
      if not argv[argi]:
        sys.exit('empty argument for flag: %s' % arg)
      cpp_flags.extend((arg, argv[argi]))
      cc1_flags.extend((arg, argv[argi]))
      argi += 1
    elif arg.startswith('-Wa,'):
      as_flags.extend(filter(None, arg[3:].split(',')))
    elif arg.startswith('-Wl,'):
      ld_flags.extend(filter(None, arg[3:].split(',')))
    elif arg == '-v':
      is_verbose = True
      ld_flags.append(arg)
      as_flags.append(arg)
      cc1_flags.append(arg)
    elif arg in ('-static-libgcc', '-nostdinc++', '-nodefaultlibs',
                 '-nostartfiles'):
      # -nodefaultlibs (ignored, -nostdlib supersedes it)
      # -nostartfiles (ignored, -nostdlib supersedes it)
      pass
    else:
      sys.exit('unknown flag: %s' % arg)

  if mode is None:
    mode = 'link'
  if not output_files:
    if mode == 'link':
      output_files.append('a.out')
    elif mode in ('-c', '-S'):
      pass  # We'll figure it out later.
      #output_files.extend(os.path.splitext(os.path.basename(filename))[0]
      #                    for filename in source_files)
    elif mode == '-E':
      output_files.append('-')
  #is_verbose = True  # !! # For debugging.
  if is_verbose:
    msgs = (
        'frontend = %r' % 'minigcc',
        'argv = %r' % argv,
        'is_verbose = %r' % is_verbose,
        'tools_dir = %r' % tools_dir,
        'missing_required_flags = %r' % missing_required_flags,
        'asm_files = %r' % asm_files,
        'asmcpp_files = %r' % asmcpp_files,
        'c_files = %r' % c_files,
        'object_files = %r' % object_files,
        'library_files = %r' % library_files,
        'source_files = %r' % source_files,
        'linkable_files = %r' % linkable_files,
        'mode = %r' % mode,
        'output_files = %r' % output_files,
        'cc1_flags = %r' % cc1_flags,
        'ld_flags = %r' % ld_flags,
        'as_flags = %r' % as_flags,
        'cpp_flags = %r' % cpp_flags,
        'cpp_only_flags = %r' % cpp_only_flags,
    )
    sys.stderr.write(''.join('%s\n' % msg for msg in msgs))

  if missing_required_flags:
    sys.exit('missing required flags: %s' %
             ' '.join(sorted(missing_required_flags)))
  if mode != '-E' and cpp_only_flags:
    sys.exit('unexpected preprocessor flags for %s: %s' %
             (mode, ' '.join(map(pipes.quote, cpp_only_flags))))
  if mode != '-E' and '-' in source_files:
    sys.exit('unexpected source files for %s: %s' %
             (mode, ' '.join(map(pipes.quote, ('-',)))))
  if mode in ('-c', '-S', '-E') and (object_files or library_files):
    sys.exit('unexpected non-source files for %s: %s' %
             (mode, ' '.join(map(pipes.quote, object_files + library_files))))
  if mode == '-S' and (asm_files or asmcpp_files):
    # TODO(pts): Does gcc support -S foo.S?
    sys.exit('unexpected source files for %s: %s' %
             (mode, ' '.join(map(pipes.quote, asm_files + asmcpp_files))))
  if (mode != 'link' and output_files and
      len(source_files) > 1):
    sys.exit('multiple sources files with -o: %s' %
             ' '.join(map(pipes.quote, source_files)))
  if mode != '-E' and '-' in output_files:
    sys.exit('stdout as output file not supported: -o -')
  if mode == 'link' and len(output_files) != 1:
    # This shouldn't happen, we check above.
    sys.exit('expected single output file for: -o')

  #argv[0] = 'gcc'
  #os.execvp(argv[0], argv)

  def maybe_add_cc1_flags(cmd, flags):
    for flag in flags:
      if flag == '-Wformat' and '-Wall' in cmd:
        continue
      if flag[2:].startswith('no-'):
        neg_flag = flag[:2] + flag[5:]
      else:
        neg_flag = '%sno-%s' % (flag[:2], flag[2:])
      # TODO(pts): Match -Wformat=0 in cmd as -Wno-format.
      # TODO(pts): Better parsing of `cmd', e.g. ['-isystem', '-Wall'].
      if flag not in cmd and neg_flag not in cmd:
        cmd.append(flag)

  def run_cppasm(asmcpp_file, output_file):
    cmd = [os.path.join(tools_dir, 'cc1'), '-E', '-lang-asm', '-quiet', '-nostdinc', '-imultilib', '32', '-imultiarch', 'i386-linux-gnu', asmcpp_file]
    cmd.extend(cc1_flags)
    maybe_add_cc1_flags(cmd, ('-Wformat', '-Wformat-security'))
    cmd.append('-fno-directives-only')  # Unconditionally.
    cmd.extend(('-o', output_file))
    run_command(cmd, 'cppasm', is_verbose)

  def run_cc1(c_file, output_file):
    basename = os.path.basename(c_file)
    cmd = [os.path.join(tools_dir, 'cc1'), '-quiet', '-nostdinc', '-imultilib', '32', '-imultiarch', 'i386-linux-gnu', c_file, '-quiet', '-dumpbase', basename, '-auxbase', os.path.splitext(basename)[0]]
    cmd.extend(cc1_flags)
    maybe_add_cc1_flags(cmd, ('-Wformat', '-Wformat-security'))
    cmd.extend(('-o', output_file))
    run_command(cmd, 'cc1', is_verbose)

  def run_as(asm_file, output_file):
    cmd = [os.path.join(tools_dir, 'as'), '--32', '-o', output_file, asm_file]
    run_command(cmd, 'as', is_verbose)

  def run_ld(linkable_files, output_file):
    cmd = [os.path.join(tools_dir, 'ld'),
           # Regular gcc appends here: --sysroot=/
           '--build-id', '-m', 'elf_i386', '--hash-style=gnu', '--as-needed', '-static', '-z', 'relro']
    cmd.extend(('-o', output_file))  # Regular gcc doesn't append it if 'a.out'.
    cmd.append('-nostdlib')  # Regular gcc doesn't append it unless -Wl,-nostdlib.
    cmd.extend(libdir_flags)
    # Regular gcc (even with -nostdlib -Wl,-nostdlib) appends about 13 -L...
    # directories here, e.g. -L/lib/i386-linux-gnu .
    cmd.extend(ld_flags)
    cmd.extend(linkable_files)
    run_command(cmd, 'ld', is_verbose)

  try:
    if mode == '-E':
      sys.exit('flag not supported yet: -')
    compile_output_files = []
    # Maps from .s to .o filenames.
    assemble_map = {}
    for source_file in source_files:
      if not os.path.isfile(source_file):
        sys.exit('source file missing: %s' % source_file)
    for source_file in source_files:
      base, ext = os.path.splitext(source_file)
      if ext == '.S':
        tmp_asm_file = get_tmp_filename('.s')
        run_cppasm(source_file, tmp_asm_file)
        assemble_map[tmp_asm_file] = os.path.basename(base + '.o')
        compile_output_files.append(tmp_asm_file)
      elif ext == '.s':
        compile_output_files.append(source_file)
        continue
      else:
        if mode != '-S':
          output_file = get_tmp_filename('.s')
          assemble_map[output_file] = os.path.basename(base + '.o')
        elif output_files:
          output_file = output_files[0]
        else:
          output_file = os.path.basename(base + '.s')
        run_cc1(source_file, output_file)
        compile_output_files.append(output_file)
    assemble_output_files = []
    if mode != '-S':
      for asm_file in compile_output_files:
        if not os.path.isfile(asm_file):
          sys.exit('asm file missing: %s' % source_file)
      for asm_file in compile_output_files:
        base, ext = os.path.splitext(asm_file)
        if mode != '-c':
          output_file = get_tmp_filename('.o')
        elif output_files:
          output_file = output_files[0]
        elif asm_file in assemble_map:
          output_file = assemble_map[asm_file]
        else:
          output_file = os.path.basename(base + '.o')
        run_as(asm_file, output_file)
        assemble_output_files.append(output_file)
    if mode == 'link':
      i = 0
      for j in xrange(len(linkable_files)):
        if linkable_files[j] is None:
          linkable_files[j] = assemble_output_files[i]
          i += 1
      assert i == len(assemble_output_files)
      for linkable_file in linkable_files:
        if not os.path.isfile(linkable_file):
          sys.exit('linkable file missing: %s' % linkable_file)
      run_ld(linkable_files, output_files[0])
  finally:
    for tmp_filename in tmp_filenames:
      try:
        os.remove(tmp_filename)
      except OSError:
        pass


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv))
  except (RuntimeError, SystemExit), e:
    if type(e) not in (RuntimeError, SystemExit):
      raise
    if isinstance(e, SystemExit) and (not e.args or e.args[0] is None or
                                      isinstance(e.args[0], int)):
      raise
    print >>sys.stderr, 'minigcc: fatal: %s' % e
    sys.exit(1)
