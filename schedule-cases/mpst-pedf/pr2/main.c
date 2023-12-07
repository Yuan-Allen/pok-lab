#include "activity.h"
#include <core/partition.h>
#include <core/semaphore.h>
#include <core/thread.h>
#include <libc/stdio.h>
#include <types.h>

int main() {
  uint32_t tid;
  pok_ret_t ret;
  pok_thread_attr_t tattr;

  tattr.entry = job1;
  tattr.processor_affinity = 0;
  tattr.time_capacity = 100;
  tattr.deadline = 3000000000; // 3 second
  tattr.period = 3000000000;   // 3 second

  ret = pok_thread_create(&tid, &tattr);
  printf("[P2] pok_thread_create (1) return=%d\n", ret);

  pok_partition_set_mode(POK_PARTITION_MODE_NORMAL);
  pok_thread_wait_infinite();

  return (0);
}
