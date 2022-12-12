#include "thread.h"
#include <linux/membarrier.h>
#include <sys/syscall.h>
#include <sys/time.h>

#define NUM 1000000

void reader_fn() {
  syscall(SYS_membarrier, MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED, 0, 0);
  while (1) {
    int x = 0;
    for (int i = 0; i < 1000000000; i++) {
      x = x + i;
    }
  }
}

void writer_fn() {
  sleep(5);
  long unsigned duration = 0;
  for (int i = 0; i < NUM; i++) {
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    syscall(SYS_membarrier, MEMBARRIER_CMD_GLOBAL_EXPEDITED, 0, 0);

    clock_gettime(CLOCK_REALTIME, &end);
    // printf("ts.nsec: %lu\n", end.tv_nsec - start.tv_nsec);
    duration += end.tv_nsec - start.tv_nsec;
  }
  cancel();
  printf("avg membarrier syscall duration: %lu\n", duration);
}

int main() {
  for (int i = 0; i < NREADER; i++) {
    create(reader_fn);
  }

  writer_fn();
  return 0;
}
