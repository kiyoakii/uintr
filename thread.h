#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NWRITER 1
#define NREADER 20

enum {
  T_FREE = 0,
  T_LIVE,
  T_DEAD,
};
struct thread {
  int id, status;
  pthread_t thread;
  void (*entry)(int);
};

struct thread tpool[NREADER], *tptr = tpool;

void *wrapper(void *arg) {
  struct thread *thread = (struct thread *)arg;
  thread->entry(thread->id);
  return NULL;
}

void create(void *fn) {
  assert(tptr - tpool < NREADER);
  *tptr = (struct thread){
      .id = (int)(tptr - tpool) + 1,
      .status = T_LIVE,
      .entry = (void (*)(int))fn,
  };
  pthread_create(&(tptr->thread), NULL, wrapper, tptr);
  ++tptr;
}

void join() {
  for (int i = 0; i < NREADER; i++) {
    struct thread *t = &tpool[i];
    if (t->status == T_LIVE) {
      pthread_join(t->thread, NULL);
      t->status = T_DEAD;
    }
  }
}

void cancel() {
  for (int i = 0; i < NREADER; i++) {
    struct thread *t = &tpool[i];
    if (t->status == T_LIVE) {
      pthread_cancel(t->thread);
      t->status = T_DEAD;
    }
  }
}

__attribute__((destructor)) void cleanup() { join(); }
