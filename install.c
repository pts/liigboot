/* by pts@fazekas.hu at Thu Dec 21 23:15:28 CET 2017 */
#include <xtiny.h>  /* !! TODO(pts): Compile and link without xtiny. */ 

typedef char xbool_t;

static void write_err(const char *msg) {
  write(2, msg, strlen(msg));
}

static xbool_t is_rm = 0;
static char *argv0 = NULL;

__attribute__((noreturn)) static void bye(int exit_code) {
  if (is_rm) {
    unlink(argv0);
  }
  _exit(exit_code);
}

int main(int argc, char **argv) {
  xbool_t is_image = 0;
  char const *target;

  (void)argc;
  argv0 = *argv++;
  if (*argv && 0 == strcmp(*argv, "--rm")) {
    is_rm = 1;
    ++argv;
  }
  if (!*argv || 0 != strcmp(*argv, "install")) { usage:
    write_err("Tool to install liigboot to Master Boot Record (MBR).\n"
              "See also https://liigboot.github.io/install\n"
              "Usage: ");
    write_err(argv0);
    write_err(" install [<target>]\n");
    write_err("    or ");
    write_err(argv0);
    write_err(" install image <image>\n"
              "<target> is name of directory or blockdev, default is /\n"
              "<image> is name of hard disk image file\n");
    bye(1);
  }
  ++argv;
  if (!*argv) {
    target = "/";
  } else if (!argv[1]) {
    target = argv[0];
  } else if (0 == strcmp(*argv, "image") && !argv[2]) {
    target = argv[1];
    is_image = 1;
  } else {
    goto usage;
  }
  puts(target);
  
  /* !! st_dev the dir */
  /* !! keep parititon table: 0x01b8 ... 0x1fe */
  /* !! check that no partition starts earlier than the file size */

  (void)is_image;
  (void)is_rm;
  bye(0);
}
