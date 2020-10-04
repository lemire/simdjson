#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include "event_counter.h"

/*
 * Prints the best number of operations per cycle where
 * test is the function call, answer is the expected answer generated by
 * test, repeat is the number of times we should repeat and size is the
 * number of operations represented by test.
 */
#define BEST_TIME(name, test, expected, pre, repeat, size, verbose)            \
  do {                                                                         \
    if (verbose)                                                               \
      std::printf("%-40s\t: ", name);                                               \
    else                                                                       \
      std::printf("\"%-40s\"", name);                                               \
    fflush(NULL);                                                              \
    event_collector collector;                                                 \
    event_aggregate aggregate{};                                               \
    for (decltype(repeat) i = 0; i < repeat; i++) {                            \
      pre;                                                                     \
      std::atomic_thread_fence(std::memory_order_acquire);                     \
      collector.start();                                                       \
      if (test != expected) {                                                  \
        std::fprintf(stderr, "not expected (%d , %d )", (int)test, (int)expected);  \
        break;                                                                 \
      }                                                                        \
      std::atomic_thread_fence(std::memory_order_release);                     \
      event_count allocate_count = collector.end();                            \
      aggregate << allocate_count;                                             \
    }                                                                          \
    if (collector.has_events()) {                                              \
      std::printf("%7.3f", aggregate.best.cycles() / static_cast<double>(size));    \
      if (verbose) {                                                           \
        std::printf(" cycles/byte ");                                               \
      }                                                                        \
      std::printf("\t");                                                            \
      std::printf("%7.3f",                                                          \
             aggregate.best.instructions() / static_cast<double>(size));       \
      if (verbose) {                                                           \
        std::printf(" instructions/byte ");                                         \
      }                                                                        \
      std::printf("\t");                                                            \
    }                                                                          \
    double gb = static_cast<double>(size) / 1000000000.0;                      \
    std::printf("%7.3f", gb / aggregate.best.elapsed_sec());                        \
    if (verbose) {                                                             \
      std::printf(" GB/s ");                                                        \
    }                                                                          \
    std::printf("%7.3f", 1.0 / aggregate.best.elapsed_sec());                       \
    if (verbose) {                                                             \
      std::printf(" documents/s ");                                                 \
    }                                                                          \
    std::printf("\n");                                                              \
    std::fflush(NULL);                                                              \
  } while (0)

// like BEST_TIME, but no check
#define BEST_TIME_NOCHECK(name, test, pre, repeat, size, verbose)              \
  do {                                                                         \
    if (verbose)                                                               \
      std::printf("%-40s\t: ", name);                                               \
    else                                                                       \
      std::printf("\"%-40s\"", name);                                               \
    std::fflush(NULL);                                                              \
    event_collector collector;                                                 \
    event_aggregate aggregate{};                                               \
    for (decltype(repeat) i = 0; i < repeat; i++) {                            \
      pre;                                                                     \
      std::atomic_thread_fence(std::memory_order_acquire);                     \
      collector.start();                                                       \
      test;                                                                    \
      std::atomic_thread_fence(std::memory_order_release);                     \
      event_count allocate_count = collector.end();                            \
      aggregate << allocate_count;                                             \
    }                                                                          \
    if (collector.has_events()) {                                              \
      std::printf("%7.3f", aggregate.best.cycles() / static_cast<double>(size));    \
      if (verbose) {                                                           \
        std::printf(" cycles/byte ");                                               \
      }                                                                        \
      std::printf("\t");                                                            \
      std::printf("%7.3f",                                                          \
             aggregate.best.instructions() / static_cast<double>(size));       \
      if (verbose) {                                                           \
        std::printf(" instructions/byte ");                                         \
      }                                                                        \
      std::printf("\t");                                                            \
    }                                                                          \
    double gb = static_cast<double>(size) / 1000000000.0;                      \
    std::printf("%7.3f", gb / aggregate.best.elapsed_sec());                        \
    if (verbose) {                                                             \
      std::printf(" GB/s ");                                                        \
    }                                                                          \
    std::printf("%7.3f", 1.0 / aggregate.best.elapsed_sec());                       \
    if (verbose) {                                                             \
      std::printf(" documents/s ");                                                 \
    }                                                                          \
    std::printf("\n");                                                              \
    std::fflush(NULL);                                                              \
  } while (0)

#endif
