/* Exercises short and long sleeps under concurrent load. */

#include <stdbool.h>
#include "devices/timer.h"
#include "tests/threads/tests.h"
#include "threads/synch.h"
#include "threads/thread.h"

#define WORKER_COUNT 12
#define SLEEP_ROUNDS 16

struct worker_data
  {
    int id;
    bool woke_early;
    struct semaphore *done;
  };

static void alarm_worker (void *aux);

void
test_alarm_extended (void)
{
  struct worker_data workers[WORKER_COUNT];
  struct semaphore done;
  int64_t start;
  int i;

  timer_sleep (0);
  timer_sleep (-1);

  start = timer_ticks ();
  timer_sleep (1);
  if (timer_elapsed (start) < 1)
    fail ("one-tick sleep returned early");

  start = timer_ticks ();
  timer_sleep (10 * TIMER_FREQ);
  if (timer_elapsed (start) < 10 * TIMER_FREQ)
    fail ("long sleep returned early");

  sema_init (&done, 0);
  for (i = 0; i < WORKER_COUNT; i++)
    {
      workers[i].id = i;
      workers[i].woke_early = false;
      workers[i].done = &done;
      if (thread_create ("alarm-worker", PRI_DEFAULT,
                         alarm_worker, &workers[i]) == TID_ERROR)
        fail ("could not create worker %d", i);
    }

  for (i = 0; i < WORKER_COUNT; i++)
    sema_down (&done);
  for (i = 0; i < WORKER_COUNT; i++)
    if (workers[i].woke_early)
      fail ("worker %d woke before its deadline", i);

  pass ();
}

static void
alarm_worker (void *aux)
{
  struct worker_data *worker = aux;
  int round;

  for (round = 0; round < SLEEP_ROUNDS; round++)
    {
      int64_t duration = 1 + (worker->id + round) % 7;
      int64_t start = timer_ticks ();

      timer_sleep (duration);
      if (timer_elapsed (start) < duration)
        worker->woke_early = true;
    }
  sema_up (worker->done);
}
