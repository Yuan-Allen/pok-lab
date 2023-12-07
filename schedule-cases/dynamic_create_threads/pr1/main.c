#include <libc/stdio.h>
#include <libc/string.h>
#include <core/thread.h>
#include <core/partition.h>
#include <types.h>

void *main_thread();
void *task();

int main() {
    uint32_t tid;
    pok_thread_attr_t tattr;
    memset(&tattr, 0, sizeof(pok_thread_attr_t));

    tattr.entry = main_thread;
    tattr.processor_affinity = 0;
    tattr.priority = 0;

    pok_thread_create(&tid, &tattr);
    
    pok_partition_set_mode(POK_PARTITION_MODE_NORMAL);
    pok_thread_wait_infinite();

  return (0);
}

void *main_thread() {
    const uint64_t period = 5e9;
    uint64_t time_capacity = 2;
    pok_thread_attr_t tattr;

    for(uint8_t priority = 1; priority <= 4; priority++) {
        uint32_t tid;
        memset(&tattr, 0, sizeof(tattr));
        tattr.priority = priority;
        tattr.time_capacity = time_capacity;
        tattr.period = period;
        tattr.entry = task;
        tattr.is_dynamic = TRUE;

        pok_ret_t ret;
        ret = pok_thread_create(&tid, &tattr);
        printf("[P1] pok_thread_create (%d) return=%d\n", priority + 1, ret);
        pok_thread_sleep(5e6);
    }
}

void *task() {
    while (1) {

    }
}