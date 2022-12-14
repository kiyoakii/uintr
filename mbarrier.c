#include "thread.h"
#include <linux/membarrier.h>
#include <sys/syscall.h>
#include <sys/time.h>

#define NUM 1000000

void reader_fn_private() {
  syscall(SYS_membarrier, MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED, 0, 0);
  while (1) {
    int x = 0;
    for (int i = 0; i < 1000000000; i++) {
      x = x + i;
    }
  }
}

void reader_fn() {
  syscall(SYS_membarrier, MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED, 0, 0);
  while (1) {
    int x = 0;
    for (int i = 0; i < 1000000000; i++) {
      x = x + i;
    }
  }
}

void writer_fn_private() {
  sleep(5);
  for (int i = 0; i < NUM; i++) {
    syscall(SYS_membarrier, MEMBARRIER_CMD_PRIVATE_EXPEDITED, 0, 0);
  }
  cancel();
}

void writer_fn() {
  sleep(5);
  for (int i = 0; i < NUM; i++) {
    syscall(SYS_membarrier, MEMBARRIER_CMD_GLOBAL_EXPEDITED, 0, 0);
  }
  cancel();
}

int main() {
  for (int i = 0; i < NREADER; i++) {
    create(reader_fn);
  }

  pthread_t self = pthread_self();
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);
  pthread_setaffinity_np(self, sizeof(cpuset), &cpuset);

  writer_fn();
  return 0;
}
