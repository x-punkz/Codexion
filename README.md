*This project has been created as part of the 42 curriculum by daniviei.*

# Codexion

> Master the race for resources before the deadline masters you.

## Description

Codexion is a concurrency simulation written in C, built on POSIX threads. It is
a variation of the classic *Dining Philosophers* problem.

`number_of_coders` coders sit around a circular co-working hub. Between every
pair of neighbours sits one shared USB **dongle** (so there are exactly as many
dongles as coders). To **compile**, a coder must hold the two dongles adjacent to
it *at the same time*. After compiling it releases both dongles and moves on to
**debug**, then **refactor**, and finally tries to compile again. A coder that
does not manage to start a new compile within `time_to_burnout` milliseconds of
its previous compile **burns out**, and the simulation ends.

The goal is to coordinate access to the scarce dongles so that coders keep
making progress and nobody burns out — while respecting a configurable
arbitration policy (`fifo` or `edf`) and a per-dongle cooldown.

## Instructions

### Build

```sh
make        # builds ./codexion
make clean  # removes object files
make fclean # removes objects and the binary
make re     # fclean + build
```

The project compiles with `cc -Wall -Wextra -Werror -pthread`.

### Run

```sh
./codexion number_of_coders time_to_burnout time_to_compile \
           time_to_debug time_to_refactor number_of_compiles_required \
           dongle_cooldown scheduler
```

| Argument | Meaning |
| --- | --- |
| `number_of_coders` | Number of coders (and of dongles). |
| `time_to_burnout` | Max ms between the start of two compiles before burnout. |
| `time_to_compile` | Duration of a compile (dongles held during this time). |
| `time_to_debug` | Duration of the debug phase. |
| `time_to_refactor` | Duration of the refactor phase. |
| `number_of_compiles_required` | Sim stops once every coder reached this count. |
| `dongle_cooldown` | Ms a dongle stays unavailable after being released. |
| `scheduler` | Arbitration policy: `fifo` or `edf`. |

All values are integers in milliseconds (except the two counts). Invalid input
(negatives, non-integers, an unknown scheduler, wrong argument count) is rejected
with a non-zero exit status.

Example:

```sh
./codexion 5 800 200 200 200 3 100 fifo
```

Log format (one state change per line):

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

The simulation stops either when a coder burns out, or when every coder has
compiled at least `number_of_compiles_required` times.

## Schedulers

When several coders want the same dongle, a per-dongle **priority queue**
(a hand-written binary min-heap — no library container is used) decides who is
served next:

- **`fifo`** — First In, First Out: the coder whose request arrived first.
- **`edf`** — Earliest Deadline First: the coder with the smallest deadline,
  where `deadline = last_compile_start + time_to_burnout`. Ties are broken by
  arrival order, making the policy fully deterministic.

## Blocking cases handled

- **Deadlock prevention (Coffman conditions).** The four Coffman conditions must
  hold together for a deadlock. We break **circular wait** with a *resource
  hierarchy*: every coder always acquires the **lower-numbered dongle first** and
  the higher-numbered one second. In a circle this makes the last coder grab its
  dongles in the opposite order to everyone else, so a full cycle of "each holds
  one, waits for the next" can never form. (Mutual exclusion, hold-and-wait and
  no-preemption are inherent to the problem; removing circular wait is enough.)
- **Starvation prevention.** Dongles are never granted arbitrarily: the priority
  queue enforces the chosen policy. `fifo` guarantees arrival-order service; `edf`
  serves the most urgent coder first, with a deterministic tie-break, so under
  feasible parameters no coder is starved.
- **Cooldown handling.** On release a dongle records `available_at = now +
  dongle_cooldown`. A waiter that is already first in line but still inside the
  cooldown window sleeps with `pthread_cond_timedwait` until exactly that
  instant, instead of busy-waiting.
- **Precise burnout detection.** A dedicated monitor thread scans every coder in
  a tight loop (sub-millisecond granularity) and reports a burnout well within
  the required 10 ms of it actually happening.
- **Log serialization.** All output goes through a single mutex-protected path
  and is emitted with `write`, so two lines can never interleave, and no state
  message is ever printed after `burned out`.
- **No busy-waiting.** Coders block on condition variables while waiting for a
  dongle; they are only woken when a dongle is released, when a cooldown expires,
  or when the simulation stops.
- **Clean shutdown.** When the simulation ends, the monitor broadcasts on every
  dongle so blocked coders wake, observe the stop flag and exit; all threads are
  joined and every mutex, condition variable and allocation is destroyed/freed
  (verified leak-free and race-free).

## Thread synchronization mechanisms

- **`pthread_mutex_t` per dongle** — protects a dongle's state (`held`,
  `available_at`, and its waiter queue). A coder must hold this mutex to inspect
  or change the dongle, which prevents two coders from ever "duplicating" the
  same dongle (the core race condition of the problem).
- **`pthread_cond_t` per dongle** — the waiting queue. A coder that cannot take a
  dongle yet releases the mutex and blocks on the condition variable
  (`pthread_cond_wait`), or `pthread_cond_timedwait` when it is only waiting out
  the cooldown. Releasing a dongle calls `pthread_cond_broadcast` **while holding
  the mutex**, which closes the lost-wakeup window: a waiter is either already
  blocked (and gets signalled) or has not yet released the mutex (and will re-test
  the predicate before blocking).
- **Log mutex (`log_lock`)** — serializes all output. The burnout announcement
  and the setting of the global stop flag happen together under this same mutex,
  which is what guarantees no ordinary message can slip out *after* `burned out`.
- **State mutex (`state_lock`)** — guards the boolean stop flag read by every
  thread via `is_stopped` / `set_stopped`.
- **Sequence mutex (`seq_lock`)** — hands out the monotonically increasing arrival
  numbers used by FIFO and by the EDF tie-break.
- **Per-coder mutex** — guards each coder's `last_compile_start` and `compiles`,
  which are written by the coder and read by the monitor. This is the thread-safe
  channel between coders and the monitor: the monitor reads a consistent snapshot
  to decide burnout and completion, with no torn reads.

A strict, global lock ordering (dongle → state, and log → state, with the
per-coder, state and sequence mutexes always taken as leaves) is respected
everywhere, so the synchronization layer itself cannot deadlock.

## Resources

- Dijkstra, E. W. — *Hierarchical ordering of sequential processes* (the origin
  of the Dining Philosophers problem and the resource-hierarchy solution).
- Coffman, Elphick, Shoshani — *System Deadlocks* (the four deadlock conditions).
- *The Linux Programming Interface*, Michael Kerrisk — chapters on POSIX threads,
  mutexes and condition variables.
- `man` pages: `pthread_create`, `pthread_mutex_init`, `pthread_cond_wait`,
  `pthread_cond_timedwait`, `pthread_cond_broadcast`, `gettimeofday`, `usleep`.
- Liu & Layland — *Scheduling Algorithms for Multiprogramming in a Hard-Real-Time
  Environment* (the basis of Earliest Deadline First scheduling).

### Use of AI

I used AI only to assist me in correcting code details, to speed up the debugging process.
