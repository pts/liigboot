/* by pts@fazekas.hu at Thu Dec 21 23:15:28 CET 2017 */
/* !! does fsync on the block device make a difference? do sync()? */
#ifdef __LINTINY__
#include <lintiny.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#ifndef O_DIRECTORY
#define O_DIRECTORY 0200000
#endif
#define getdents(a, b, c)  syscall(SYS_getdents, (a), (b), (c))
struct linux_dirent {
  ino_t d_ino;
  off_t d_off;
  uint16_t d_reclen;
  char d_name[];
};
#endif

#if !defined(__i386) && !defined(__amd64)
/* This is because of unaligned pointer access. */
#error Only i386 and amd64 architectures supported.
#endif

typedef char xbool_t;

#define PE_ACTIVE 0x80  /* In a bit of (struct pte).flags. */

#define PT_EMPTY 0x0  /* In (struct pte).type. */

/* Partition table entry.
 * https://en.wikipedia.org/wiki/Master_boot_record#Partition_table_entries
 */
struct pte {
  uint8_t flags;  /* Bit field. */
  uint8_t chs_first[3];
  uint8_t type;
  uint8_t chs_last[3];
  uint32_t lba_first;  /* Sector count in front of this partition. */
  uint32_t lba_sector_count;
} __attribute__((packed));

static void write_err(const char *msg) {
  write(2, msg, strlen(msg));
}

static xbool_t do_rm = 0;
__attribute__((aligned(1))) static char *argv0 = NULL;

__attribute__((noreturn)) static void bye(int exit_code) {
  if (do_rm) {
    unlink(argv0);
  }
  _exit(exit_code);
}

static void die(const char *msg) {
  write(2, msg, strlen(msg));
  bye(2);
}

/* Returns 0 on success. */
static int find_device_name(unsigned mbr_dev, char *dev_name, size_t dev_name_size) {
  char dirent_buf[4096];
  int fd;
  unsigned nread, bpos;
  if (dev_name_size < 7) return 1;
  memcpy(dev_name, "/dev/", 5);
  if ((fd = open("/dev", O_RDONLY | O_DIRECTORY, 0)) < 0) return -1;
  for (;;) {
    nread = getdents(fd, dirent_buf, sizeof dirent_buf);
    if (nread == 0 - 1U) goto done;
    if (nread == 0) { nread = 1; goto done; }
    for (bpos = 0; bpos < nread;) {
      struct linux_dirent *d = (struct linux_dirent *) (dirent_buf + bpos);
      struct stat st;
      if (strlen(dev_name) + 6 > dev_name_size) continue;
      strcpy(dev_name + 5, d->d_name);
      /* char d_type = *(buf + bpos + d->d_reclen - 1); */
      if (0 == lstat(dev_name, &st) && S_ISBLK(st.st_mode) &&
          st.st_rdev == mbr_dev) {
        nread = 0; goto done;  /* Found it. */
      }
      bpos += d->d_reclen;
    }
  }
 done:
  close(fd);
  return nread;
}

/* active_part is the partition number (1..4) to make active. Any other number
 * won't make any change to active flags.
 */
static void install(const char *source, const char *target,
                    uint8_t active_part) {
  char oldmbr[512];
  char buf[4096];  /* Must be at least 512 and a multiple of 512. */
  int fdin, fdout, got, got2;
  uint32_t lba_first_min = (uint32_t)-1;
  uint32_t lba = 0;
  if (active_part > 4) active_part = 0;
  if (active_part != 0) {
    write_err("info: will make partition ");
    buf[0] = '0' + active_part;
    strcpy(buf + 1, " active\n");
    write_err(buf);
  }

  /* TODO(pts): Save bytes by not closing. */
  fdin = open(source, O_RDONLY, 0);
  if (fdin < 0) die("fatal: error opening source\n");
  memset(buf, '\0', sizeof(buf));
  if ((got = read(fdin, buf, sizeof(buf))) < 0) { read_error:
    die("fatal: error reading from source\n");
  }
  if (got < 512) die("fatal: source too short\n");
  if (*(uint16_t*)(buf + 0x1fe) != 0xaa55) die("fatal: bad mbr in source\n");

  fdout = open(target, O_RDWR, 0);
  if (fdout < 0) {
    const int e = errno;
    if (e == EACCES && geteuid() != 0) {
      die("fatal: error opening mbr target; use sudo\n");
    }
    if (e == ENOENT) {
      die("fatal: error opening mbr target; create image file first\n");
    }
    die("fatal: error opening mbr target\n");
  }
  memset(oldmbr, '\0', 512);
  got2 = read(fdout, oldmbr, 512);
  if (got2 != 0) {
    const struct pte *pte, *pte_end;
    if (got2 < 0) die("fatal: error reading from mbr target\n");
    if (got2 < 512) die("fatal: mbr truncated in mbr target\n");
    if (0 != lseek(fdout, 0, SEEK_SET)) {
      die("fatal: error seeking in mbr target\n");
    }
    if (*(uint16_t*)(oldmbr + 0x1fe) == 0xaa55) {
      struct stat st;
      uint32_t source_sector_count;
      /* TODO(pts): Make it work as a device on st.st_size. */
      if (0 != fstat(fdin, &st)) die("fatal: error statting source\n");
      source_sector_count = (st.st_size + 511) >> 9;
      memcpy(buf + 0x1b8, oldmbr + 0x1b8, 0x46);
      for (pte = (const struct pte*)(buf + 0x1be), pte_end = pte + 4;
           pte != pte_end;
           ++pte) {
        if (pte->type != 0 && pte->lba_first < lba_first_min) {
          lba_first_min = pte->lba_first;
        }
      }
      if (lba_first_min < source_sector_count) {
        die("fatal: partition in mbr target starts too early\n");
      }
    } else {  /* No partition table in oldmbr. */
      const char *p = oldmbr, *pend = p + 0x200;
      for (; p != pend && *p == '\0'; ++p) {}
      /* Non-\0 byte found. */
      if (p != pend) die("fatal: refusing to modify unusual mbr\n");
      /* Keep the partition table in source (buf). */
    }
  }
  if (active_part > 0) {
    /* True: assert(active_part <= 4). */
    buf[0x1be + 0x00] &= ~0x80;
    buf[0x1be + 0x10] &= ~0x80;
    buf[0x1be + 0x20] &= ~0x80;
    buf[0x1be + 0x30] &= ~0x80;
    buf[0x1be - 0x10 + active_part * 0x10] |= 0x80;
  }
  while (got != 0) {  /* Copy from fdin to fdout. */
    if (lba + ((got + 511U) >> 9) > lba_first_min) {
      die("fatal: partition in mbr target starts too early, stopping\n");
    }
    if (got + 0U < sizeof(buf)) {  /* Write the last few bytes. */
      got = (got + 511U) & -512;  /* Round up. */
      if (write(fdout, buf, got) != got) { write_error:
        die("fatal: error writing to mbr target\n");
      }
      break;
    }
    if (write(fdout, buf, sizeof(buf)) != sizeof(buf)) goto write_error;
    if ((got = read(fdin, buf, sizeof(buf))) < 0) goto read_error;
  }
}

int main(int argc, char **argv) {
  xbool_t is_target_image = 0;
  char source[1024];
  const char *target;
  uint8_t active_part = 0;

  (void)argc;
  argv0 = *argv++;
  if (*argv && 0 == strcmp(*argv, "--rm")) {
    do_rm = 1;
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
  {  /* Source will be argv[0] without the ".install" suffix. */
    size_t size = strlen(argv0);
    if (size < 9 || size >= sizeof(source) - 8 ||
        0 != memcmp(argv0 + size - 8, ".install", 9)) {
      die("fatal: command name must end with .install\n");
    }
    memcpy(source, argv0, size -= 8);
    source[size] = '\0';
  }
  if (!*argv) {
    target = "/";
  } else if (!argv[1]) {
    target = argv[0];
  } else if (0 == strcmp(*argv, "image") && !argv[2]) {
    target = argv[1];
    is_target_image = 1;
  } else {
    goto usage;
  }
  write_err("info: install source: ");
  write_err(source);
  write_err("\n");
  write_err("info: install target: ");
  write_err(target);
  write_err("\n");
  if (!is_target_image) {
    struct stat st;
    if (0 != stat(target, &st)) {
      die("fatal: target not found\n");
    }
    if (S_ISDIR(st.st_mode)) {
      /* Example: /dev/sdb (mbr_dev) is 0x0810, /dev/sdb2 is 0x0812. */
      const unsigned mbr_dev = st.st_dev & ~15;
      static char dev_name[32];
      if ((st.st_dev & 15) == 0) {
        /* This is because the Ligboot image may overlap with the filesystem. */
        die("fatal: refusing to install to mounted mbr device\n");
      }
      if (find_device_name(mbr_dev, dev_name, sizeof(dev_name)) != 0) {
        die("fatal: mbr device not found in /dev\n");
      }
      target = dev_name;
      active_part = st.st_dev & 15;
    } else if (S_ISBLK(st.st_mode)) {
      if ((st.st_rdev & 15) != 0) {
        die("fatal: refusing to install to partition\n");
      }
    } else {
      die("fatal: target not a directory or blockdev\n");
    }
  }
  write_err("info: install mbr target: ");
  write_err(target);
  write_err("\n");
  install(source, target, active_part);
  write_err("info: install OK\n");
  bye(0);
}
