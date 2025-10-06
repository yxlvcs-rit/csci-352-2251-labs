#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/memlayout.h"
#include "user/user.h"

int ugetpid_test();
int ugetpid_perm_test();
int print_kpgtbl();
int pgaccess_test();

int
main(int argc, char *argv[])
{
  int all_success = 1;
  all_success = ugetpid_test() && all_success;
  all_success = ugetpid_perm_test() && all_success;
  all_success = print_kpgtbl() && all_success;
  all_success = pgaccess_test() && all_success;
  if (all_success) {
    printf("pgtbltest: all tests succeeded\n");
    exit(0);
  }
  exit(1);
}

char *testname = "???";

int
err(char *why)
{
  printf("pgtbltest: %s failed: %s, pid=%d\n", testname, why, getpid());
  return 0;
}

int
ugetpid_test()
{
  int i;

  printf("ugetpid_test starting\n");
  testname = "ugetpid_test";

  for (i = 0; i < 64; i++) {
    int ret = fork();
    if (ret != 0) {
      wait(&ret);
      if (ret != 0)
        return err("child process failed");
      continue;
    }
    if (getpid() != ugetpid()) {
      err("mismatched PID");
      exit(1);
    }
    exit(0);
  }
  printf("ugetpid_test: OK\n");
  return 1;
}

int
ugetpid_perm_test()
{
  printf("ugetpid_perm_test starting\n");
  testname = "ugetpid_perm_test";

  int ret = fork();
  if (ret != 0) {
    wait(&ret);
    if (ret == 0) {
      return err("usyscall region is not read-only from user space");
    }
  } else {
    struct usyscall *usyscall = (struct usyscall *) USYSCALL;
    usyscall->pid = 0;
    exit(0);
  }
  printf("ugetpid_perm_test: OK\n");
  return 1;
}

int
print_kpgtbl()
{
  printf("print_kpgtbl starting\n");
  kpgtbl();
  printf("print_kpgtbl: OK\n");
  return 1;
}

int
pgaccess_test()
{
  char *buf;
  unsigned int abits;
  printf("pgaccess_test starting\n");
  testname = "pgaccess_test";
  buf = malloc(32 * PGSIZE);
  if (pgaccess(buf, 32, &abits) < 0)
    return err("pgaccess failed");
  buf[PGSIZE * 1] += 1;
  buf[PGSIZE * 2] += 1;
  buf[PGSIZE * 30] += 1;
  if (pgaccess(buf, 32, &abits) < 0)
    return err("pgaccess failed");
  if (abits != ((1 << 1) | (1 << 2) | (1 << 30)))
    return err("incorrect access bits set");
  free(buf);
  printf("pgaccess_test: OK\n");
  return 1;
}
