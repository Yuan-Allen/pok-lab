# Lab 1 设计与实现报告

## 各个调度算法的实现

### 分区调度

#### 分区调度框架搭建

由于POK原本的分区调度算法是“写死”的，没有留下可扩展性，因此需要在上面实现一套分区调度框架，然后再在框架上增加调度算法。

POK原本的分区调度是通过配置文件配置slots实现的，即通过在配置文件中配置slots的长度以及对应的分区，然后按照该配置执行指定的分区。分区调度的触发，以及分区的选择都与配置文件高度耦合，要在此基础上做改动非常困难。对此，我们想到的解法是针对配置的调度算法对应配置slots以及调度函数。

具体来说，对于抢占式优先级调度与抢占式EDF调度，我们为每个slots设置为固定的`POK_CONFIG_BASE_SLOT`大小（`POK_CONFIG_BASE_SLOT`可在YAML文件中配置，同时提供默认值），以模拟固定的分区调度时间片大小。同时我们的调度框架是向后兼容的，对于POK原先的调度算法，仍然可以通过读取配置文件来初始化slots。slots初始化实现在`pok_init_slots`函数中。

对于调度函数而言，我们对分区调度算法使用不同的函数进行包装，然后通过配置文件配置的调度策略去初始化函数指针，最后通过函数指针去执行对应的调度函数，以提供可扩展性。函数指针设置实现在`pok_setup_scheduler_global`函数中。

这两个初始化函数均会在`pok_sched_init`中被调用。

```c++
void pok_init_slots() {
#ifdef POK_CONFIG_GLOBAL_SCHEDULER
  switch ((pok_sched_t)POK_CONFIG_GLOBAL_SCHEDULER) {
  case POK_SCHED_GLOBAL_PPS:
  case POK_SCHED_GLOBAL_PEDF:
    for (uint8_t i = 0; i < POK_CONFIG_SCHEDULING_NBSLOTS; i++) {
      pok_sched_slots[i] = (uint64_t)POK_CONFIG_BASE_SLOT_LENGTH;
      pok_sched_slots_allocation[i] = i;
    }
    pok_sched_major_frame =
        (uint64_t)POK_CONFIG_SCHEDULING_NBSLOTS * POK_CONFIG_BASE_SLOT_LENGTH;
    break;
  default:
    for (int i = 0; i < POK_CONFIG_SCHEDULING_NBSLOTS; ++i) {
      pok_sched_slots[i] = (uint64_t[])POK_CONFIG_SCHEDULING_SLOTS[i];
      pok_sched_slots_allocation[i] =
          (uint8_t[])POK_CONFIG_SCHEDULING_SLOTS_ALLOCATION[i];
    }
    pok_sched_major_frame = POK_CONFIG_SCHEDULING_MAJOR_FRAME;
    break;
  }
#else
  for (int i = 0; i < POK_CONFIG_SCHEDULING_NBSLOTS; ++i) {
    pok_sched_slots[i] = (uint64_t[])POK_CONFIG_SCHEDULING_SLOTS[i];
    pok_sched_slots_allocation[i] =
        (uint8_t[])POK_CONFIG_SCHEDULING_SLOTS_ALLOCATION[i];
  }
  pok_sched_major_frame = POK_CONFIG_SCHEDULING_MAJOR_FRAME;
#endif
}

void pok_setup_scheduler_global() {
#ifdef POK_CONFIG_GLOBAL_SCHEDULER
  switch (POK_CONFIG_GLOBAL_SCHEDULER) {
  case POK_SCHED_GLOBAL_PPS:
    sched_partiton_func = pok_sched_part_global_pps;
    break;
  case POK_SCHED_GLOBAL_PEDF:
    sched_partiton_func = pok_sched_part_global_pedf;
    break;
  default:
    sched_partiton_func = pok_sched_part_global_timeslice;
    break;
  }

#else
  sched_partiton_func = pok_sched_part_global_timeslice;
#endif
}
```

此外，为分区实现抢占式调度的一个挑战是，分区内部的初始化（执行main thread，创建线程，切换到normal mode等）是在调度到该分区才执行的。我们应该确保那些未初始化完成的分区有机会能完成其初始化，否则某些特定调度算法可能永远都不会调度到这些分区（导致类似于死锁的问题）。对此，我们会判断遍历到的分区是否已经被初始化，然后才去执行特定的调度函数，否则优先选择依然处于INIT状态的分区，使其有机会能完成其初始化。

```c++
uint8_t pok_elect_partition() {
  uint8_t next_partition = POK_SCHED_CURRENT_PARTITION;
#if POK_CONFIG_NB_PARTITIONS > 1
  uint64_t now = POK_GETTICK();

  if (pok_sched_next_deadline <= now) {

    // -- skip --

    pok_sched_current_slot =
        (pok_sched_current_slot + 1) % POK_CONFIG_SCHEDULING_NBSLOTS;
    pok_sched_next_deadline =
        pok_sched_next_deadline + pok_sched_slots[pok_sched_current_slot];
    next_partition = pok_sched_slots_allocation[pok_sched_current_slot];
    if (pok_partitions[next_partition].mode != POK_PARTITION_MODE_INIT_COLD &&
        pok_partitions[next_partition].mode != POK_PARTITION_MODE_INIT_WARM) {
      // We wrap the function call in the condition to give those partitions
      // that are still in init mode a chance to finish their init, or they may
      // never be initialized and scheduled.
      next_partition =
          sched_partiton_func(pok_partitions, pok_current_partition);
    }
  }
#endif /* POK_CONFIG_NB_PARTITIONS > 1 */

  return next_partition;
}
```

最后，我们还需为`gen_delpoyment`脚本补充分区调度相关内容，使其可以直接通过读取YAML配置文件在`deployment.h`中生成我们需要的宏。
```python
    global_scheduler = get_option_or(config, "kernel.scheduler.policy", "TIMESLICE").upper()
    if global_scheduler not in ["TIMESLICE", "PPS", "PEDF"]:
        raise Exception("only TIMESLICE, PPS and PEDF schedulers are implemented right now")
    if global_scheduler != "TIMESLICE":
        configs["GLOBAL_SCHEDULER"] = "POK_SCHED_GLOBAL_{}".format(global_scheduler)
        includes.add("core/schedvalues.h")
    
    base_slot_length = get_option_or(config, "kernel.scheduler.base_slot_length", decode=to_ns, default=1000000000) # 1s
    configs["BASE_SLOT_LENGTH"] = base_slot_length
```

#### 抢占式优先级调度

由于POK中的分区原本没有优先级的概念，因此我们需要为其添加优先级属性。在我们的实现中，分区优先级被定义为该分区中所有状态处于RUNNABLE的线程的优先级总和。这只是一种粗略定义，目的只是为了实现优先级调度算法，实际应用场景可以有其他定义。

一个需要注意的点是，状态为`POK_STATE_WAIT_NEXT_ACTIVATION`的线程要在调度到该分区时才会被唤醒，但未唤醒时由于其状态不是`POK_STATE_RUNNABLE`又不会帮助该分区被调度到。这可能会导致前面提到的死锁问题，即该分区永远都不会被调度到，线程也永远都不会被唤醒。对此我们需要在调度分区时手动去唤醒这些可以被唤醒的线程，以正确调度分区。具体来说，我们实现了`activate_waiting_thread`函数，该函数会在`pok_sched_part_global_pps`中被调用。

```c++
void activate_waiting_threads() {
  uint64_t now = POK_GETTICK();

  for (uint8_t i = 0; i < POK_CONFIG_NB_PARTITIONS; i++) {
    pok_thread_t *thread;
    for (i = 0; i < pok_partitions[i].nthreads; i++) {
      thread = &(pok_threads[pok_partitions[i].thread_index_low + i]);

      if (thread->processor_affinity == pok_get_proc_id()) {

#if defined(POK_NEEDS_LOCKOBJECTS) || defined(POK_NEEDS_PORTS_QUEUEING) ||     \
    defined(POK_NEEDS_PORTS_SAMPLING)
        if ((thread->state == POK_STATE_WAITING) &&
            (thread->wakeup_time <= now)) {
          thread->state = POK_STATE_RUNNABLE;
        }
#endif

        if ((thread->state == POK_STATE_WAIT_NEXT_ACTIVATION) &&
            (thread->next_activation <= now)) {
          assert(thread->time_capacity);
          thread->state = POK_STATE_RUNNABLE;
          thread->remaining_time_capacity = thread->time_capacity;
          thread->next_activation = thread->next_activation + thread->period;
        }
      }
    }
  }
}
```

```c++
// Calculate priority for each partition.
// The priority is the sum of all RUNNABLE threads' priority in the partition.
void caculate_and_update_priority(pok_partition_t pok_partitions[]) {
  for (uint8_t i = 0; i < POK_CONFIG_NB_PARTITIONS; i++) {
    uint8_t priority = 0;
    for (uint32_t j = 0; j < pok_partitions[i].nthreads; j++) {
      if (pok_threads[pok_partitions[i].thread_index_low + j].state ==
          POK_STATE_RUNNABLE) {
        priority +=
            pok_threads[pok_partitions[i].thread_index_low + j].priority;
      }
    }
    pok_partitions[i].priority = priority;
  }
}

uint8_t pok_sched_part_global_pps(pok_partition_t pok_partitions[],
                                  const uint8_t current_partition) {
  uint8_t next_partition = current_partition;
  uint8_t max_priority = 0;

  // Activate waiting threads, or some partitions may never be scheduled.
  activate_waiting_threads();
  caculate_and_update_priority(pok_partitions);

  for (uint8_t i = 0; i < POK_CONFIG_NB_PARTITIONS; i++) {
    if (pok_partitions[i].priority > max_priority) {
      max_priority = pok_partitions[i].priority;
      next_partition = i;
    }
  }

#if defined(POK_NEEDS_DEBUG)
  printf("\nScheduling partition %d\n", next_partition);
#endif

  return next_partition;
}
```

#### 抢占式EDF调度

抢占式EDF调度的实现与抢占式优先级调度的实现类似，因为deadline也算是某种意义上的优先级。唯一不同的是我们对分区的deadline的定义不同，即分区的deadline直接取其内部deadline最小的线程的优先级。

```c++
// Calculate deadline for each partition.
// The deadline is the minimum deadline of all RUNNABLE threads in the
// partition.
void caculate_and_update_deadline(pok_partition_t pok_partitions[]) {
  for (uint8_t i = 0; i < POK_CONFIG_NB_PARTITIONS; i++) {
    uint64_t deadline = __UINT64_MAX__;
    for (uint32_t j = 0; j < pok_partitions[i].nthreads; j++) {
      pok_thread_t thread = pok_threads[pok_partitions[i].thread_index_low + j];
      if (thread.state == POK_STATE_RUNNABLE && thread.deadline < deadline) {
        deadline = thread.deadline;
      }
    }
    pok_partitions[i].deadline = deadline;
  }
}

uint8_t pok_sched_part_global_pedf(pok_partition_t pok_partitions[],
                                   const uint8_t current_partition) {
  uint8_t next_partition = current_partition;
  uint64_t min_deadline = __UINT64_MAX__;

  // Activate waiting threads, or some partitions may never be scheduled.
  activate_waiting_threads();
  caculate_and_update_deadline(pok_partitions);

  for (uint8_t i = 0; i < POK_CONFIG_NB_PARTITIONS; i++) {
    if (pok_partitions[i].deadline < min_deadline) {
      min_deadline = pok_partitions[i].deadline;
      next_partition = i;
    }
  }

#if defined(POK_NEEDS_DEBUG)
  printf("\nScheduling partition %d\n", next_partition);
#endif

  return next_partition;
}
```

#### Round-Robin与Weight-Round-Robin调度

要实现Round-Robin或者Weight-Round-Robin，我们可复用POK分区调度原本的的slots机制，只需要在配置文件里面做出合理的配置即可，无需再做出额外实现。
```yaml
target:
    arch: x86
    bsp: x86-qemu

partitions:
    - name: pr1
      features: [timer, libc]
      threads:
          count: 2
      size: 130kB
      objects: ["main.o", "activity.o"]
    - name: pr2
      features: [timer, libc]
      threads:
          count: 2
      size: 130kB
      objects: ["main.o", "activity.o"]
    - name: pr3
      features: [timer, libc]
      threads:
          count: 2
      size: 130kB
      objects: ["main.o", "activity.o"]

kernel:
    features: [debug]
    scheduler:
        # Weight-Round-Robin example
        major_frame: 6s
        slots:
            - partition: pr1
              duration: 1s
            - partition: pr2
              duration: 3s
            - partition: pr3
              duration: 2s

        # Round-Robin example
        # major_frame: 3s
        # slots:
        #     - partition: pr1
        #       duration: 1s
        #     - partition: pr2
        #       duration: 1s
        #     - partition: pr3
        #       duration: 1s
        ```
