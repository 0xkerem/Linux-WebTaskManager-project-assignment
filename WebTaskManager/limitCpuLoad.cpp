#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

// Definiert in sched_setattr(2)
struct sched_attr {
  uint32_t size;            // Size of this structure
  uint32_t sched_policy;    // Policy (SCHED_*)
  uint64_t sched_flags;     // Flags
  int32_t sched_nice;       // Nice value (SCHED_OTHER, SCHED_BATCH)
  uint32_t sched_priority;  // Static priority (SCHED_FIFO, SCHED_RR)
  // Die restlichen Felder sind für SCHED_DEADLINE
  uint64_t sched_runtime;
  uint64_t sched_deadline;
  uint64_t sched_period;
};

int main(int argc, char *argv[]) {
  struct sched_attr attributes;
  int returnVal;
  int pid = atoi(argv[2]);
  int runtime = atoi(argv[1]) * 10000;
  int period = 1000000;
  char command[100];

  sprintf(command, "taskset -pc 0,1 %d > /dev/null", pid);

  system(command);

  memset(&attributes, 0, sizeof(struct sched_attr));

  if (runtime == period) {
    attributes.sched_policy = SCHED_OTHER;
    returnVal = syscall(SYS_sched_setattr, pid, &attributes, 0);
  } else {
    attributes.sched_policy = SCHED_DEADLINE;
    attributes.sched_runtime = runtime;
    attributes.sched_period = period;
    attributes.sched_deadline = period;
    returnVal = syscall(SYS_sched_setattr, pid, &attributes, 0);
  }

  return (0);
}
