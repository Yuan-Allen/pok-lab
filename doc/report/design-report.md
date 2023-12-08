# Lab 1 设计与实现报告

## 各个调度算法的实现

### 线程调度
#### 共性修改
在实现新的线程调度方式时，需要声明并实现新的调度函数，添加新的调度方式枚举类型，配置新的预定义宏等，替换默认的调度方式。

#### 抢占式优先级调度（PPS）
利用原有的thread参数priority，遍历当前进程的可运行线程,得到并调度到优先级最高的线程。抢占式的实现在于，每一tick结束时进行检测，允许调度到更高优先级的线程。

```c++
#ifdef POK_NEEDS_SCHED_PPS
uint32_t pok_sched_part_pps(const uint32_t index_low, const uint32_t index_high,
                            const uint32_t prev_thread,
                            const uint32_t current_thread) {
  uint32_t from = current_thread != IDLE_THREAD ? current_thread : prev_thread;
  int32_t max_prio = -1;
  uint32_t max_thread = current_thread;
  uint8_t current_proc = pok_get_proc_id();

  if (prev_thread == IDLE_THREAD)
    from = index_low;

  uint32_t i = from;
  do {
    if (pok_threads[i].state == POK_STATE_RUNNABLE &&
        pok_threads[i].processor_affinity == current_proc &&
        pok_threads[i].priority > max_prio) {
      max_prio = pok_threads[i].priority;
      max_thread = i;
    }
    i++;
    if (i >= index_high) {
      i = index_low;
    }
  } while (i != from);

  uint32_t elected = max_prio >= 0 ? max_thread : IDLE_THREAD;

#ifdef POK_NEEDS_DEBUG
  printf("--- Scheduling processor: %hhd\n    elected thread %d "
         "(priority "
         "%d)\n",
         current_proc, elected, pok_threads[elected].priority);
#endif

  return elected;
}
#endif // POK_NEEDS_SCHED_PPS
```


#### 抢占式EDF调度（PEDF）

抢占式EDF调度的实现与抢占式优先级调度的实现类似，不同的是将更高的优先级替换为更早的deadline。但经过代码阅读，我们发现pok中thread参数deadline实际上是相对deadline，每个周期相同，和EDF中所需的ddl不同，因此我们在thread参数中增加绝对deadline————ddl，其将在每个任务的新周期重置参数时进行更新。

```c++
//更新ddl
      if ((thread->state == POK_STATE_WAIT_NEXT_ACTIVATION) &&
          (thread->next_activation <= now)) {
        // 传入的相对deadline，周期任务到达时要计算新的绝对ddl
        thread->ddl = thread->next_activation + thread->deadline;
        assert(thread->time_capacity);
        thread->state = POK_STATE_RUNNABLE;
        thread->remaining_time_capacity = thread->time_capacity;       
        thread->next_activation = thread->next_activation + thread->period;
      }
```
```c++
//pedf调度
#ifdef POK_NEEDS_SCHED_PEDF
uint32_t pok_sched_part_pedf(const uint32_t index_low,
                             const uint32_t index_high,
                             const uint32_t prev_thread,
                             const uint32_t current_thread) {
  uint32_t from = current_thread != IDLE_THREAD ? current_thread : prev_thread;
  uint64_t earliest_ddl = 0;
  uint32_t max_thread = IDLE_THREAD;
  uint8_t current_proc = pok_get_proc_id();

  if (prev_thread == IDLE_THREAD)
    from = index_low;

  uint32_t i = from;
  do {
    if (pok_threads[i].state == POK_STATE_RUNNABLE &&
        pok_threads[i].processor_affinity == current_proc &&
        (pok_threads[i].ddl < earliest_ddl || earliest_ddl <= 0)) {
      earliest_ddl = pok_threads[i].ddl;
      max_thread = i;
    }
    i++;
    if (i >= index_high) {
      i = index_low;
    }
  } while (i != from);

  uint32_t elected = earliest_ddl > 0 ? max_thread : IDLE_THREAD;

#ifdef POK_NEEDS_DEBUG
  printf("--- Scheduling processor: %hhd\n    elected thread %d "
         "(ddl "
         "%llu)\n",
         current_proc, elected, pok_threads[elected].ddl);
#endif

  return elected;
}
#endif // POK_NEEDS_SCHED_PEDF
```


#### Round-Robin与Weight-Round-Robin调度

RR调度与WRR调度的实现类似，当WRR中W均相等时，二者实际等价，以下介绍WRR。

WRR调度中，为thread_attr增加了weight参数，使用户可以配置thread的调度权重。此外，在thread结构中增加了weight和remaining_timeslice参数，前者用于记录该线程权重，后者记录该线程剩余时间片。该参数的增加同时需要更新部分初始化函数，将不在此赘述。

当线程调度时，依据weight大小为其填充时间片，当时间片未消耗完成时，将继续执行该线程，当时间片归零后，将依据rr调度的方式寻找该进程的新的可运行线程，并调度到该线程。

```c++
#ifdef POK_NEEDS_SCHED_WRR
uint32_t pok_sched_part_wrr(const uint32_t index_low, const uint32_t index_high,
                            const uint32_t prev_thread,
                            const uint32_t current_thread) {
  uint32_t from = current_thread != IDLE_THREAD ? current_thread : prev_thread;
  uint8_t current_proc = pok_get_proc_id();

  if (pok_threads[current_thread].remaining_timeslice > 0)
    pok_threads[current_thread].remaining_timeslice--;

  if (pok_threads[current_thread].state == POK_STATE_RUNNABLE &&
      pok_threads[current_thread].remaining_time_capacity > 0 &&
      pok_threads[current_thread].remaining_timeslice > 0) {
#ifdef POK_NEEDS_DEBUG
    printf("--- Scheduling processor: %hhd\n  continue thread %d "
           "(remaining_timeslice "
           "%u)\n",
           current_proc, current_thread,
           pok_threads[current_thread].remaining_timeslice);
#endif
    return current_thread;
  }

  uint32_t i = from;
  if (i == IDLE_THREAD) {
    i = index_low;
  } else {
    i++;
    if (i >= index_high) {
      i = index_low;
    }
  }

  do {
    if (pok_threads[i].state == POK_STATE_RUNNABLE &&
        pok_threads[i].processor_affinity == current_proc && i != IDLE_THREAD) {
#ifdef POK_NEEDS_DEBUG
      printf("--- Scheduling processor: %hhd\n    elected thread %d "
             "(remaining_timeslice "
             "%u)\n",
             current_proc, i, pok_threads[i].remaining_timeslice);
#endif
      if (current_thread != IDLE_THREAD) {
        pok_threads[current_thread].remaining_timeslice =
            pok_threads[current_thread].weight;
      }
      return i;
    }
    i++;
    if (i >= index_high) {
      i = index_low;
    }
  } while (i != from);

  if (pok_threads[current_thread].state == POK_STATE_RUNNABLE &&
      pok_threads[current_thread].processor_affinity == current_proc &&
      current_thread != IDLE_THREAD) {
    pok_threads[current_thread].remaining_timeslice =
        pok_threads[current_thread].weight;
#ifdef POK_NEEDS_DEBUG
    printf("--- Scheduling processor: %hhd\n  scheduling self-thread %d "
           "(remaining_timeslice "
           "%u)\n",
           current_proc, current_thread,
           pok_threads[current_thread].remaining_timeslice);
#endif

    return current_thread;
  }

#ifdef POK_NEEDS_DEBUG
  printf("--- Scheduling processor: %hhd\n    elected thread %d "
         "(IDLE_THREAD remaining_timeslice "
         "%u)\n",
         current_proc, IDLE_THREAD,
         pok_threads[IDLE_THREAD].remaining_timeslice);
#endif
  if (current_thread != IDLE_THREAD) {
    pok_threads[current_thread].remaining_timeslice =
        pok_threads[current_thread].weight;
  }
  return IDLE_THREAD;
}
#endif // POK_NEEDS_SCHED_WRR
```

#### 优先级抢占式Weight-Round-Robin调度
该调度是任务四所要求的新调度方式，在WRR调度的基础上，可以为线程设置优先级，并允许优先级严格更高的线程在低优先级线程尚未用完时间片时进行抢占。该调度方式使得场景中的紧急任务能够更快的得到执行，理论上将能取得更好的效果。经实验检测，结果如预期。

```c++
#ifdef POK_NEEDS_SCHED_PWRR
uint32_t pok_sched_part_pwrr(const uint32_t index_low,
                             const uint32_t index_high,
                             const uint32_t prev_thread,
                             const uint32_t current_thread) {
  uint32_t from = current_thread != IDLE_THREAD ? current_thread : prev_thread;
  // preempt by higher priority
  int32_t current_prio = pok_threads[current_thread].priority;
  int32_t max_prio = -1;
  uint32_t max_thread = current_thread;
  uint8_t current_proc = pok_get_proc_id();
  if (pok_threads[current_thread].remaining_timeslice > 0)
    pok_threads[current_thread].remaining_timeslice--;

  if (prev_thread == IDLE_THREAD)
    from = index_low;

  uint32_t i = from;
  do {
    if (pok_threads[i].state == POK_STATE_RUNNABLE &&
        pok_threads[i].processor_affinity == current_proc &&
        pok_threads[i].priority > max_prio) {
      max_prio = pok_threads[i].priority;
      max_thread = i;
    }
    i++;
    if (i >= index_high) {
      i = index_low;
    }
  } while (i != from);

  uint32_t elected = max_prio >= 0 ? max_thread : IDLE_THREAD;
  if (pok_threads[elected].priority > current_prio) {
    if (current_thread != IDLE_THREAD) {
      pok_threads[current_thread].remaining_timeslice =
          pok_threads[current_thread].weight;
    }
#ifdef POK_NEEDS_DEBUG
    printf("--- Scheduling processor: %hhd\n  preemptive thread %d "
           "(remaining_timeslice "
           "%u)\n",
           current_proc, elected, pok_threads[elected].remaining_timeslice);
#endif
    return elected;
  }
  // wrr

  if (pok_threads[current_thread].state == POK_STATE_RUNNABLE &&
      pok_threads[current_thread].remaining_time_capacity > 0 &&
      pok_threads[current_thread].remaining_timeslice > 0) {
#ifdef POK_NEEDS_DEBUG
    printf("--- Scheduling processor: %hhd\n  continue thread %d "
           "(remaining_timeslice "
           "%u)\n",
           current_proc, current_thread,
           pok_threads[current_thread].remaining_timeslice);
#endif
    return current_thread;
  } else if (current_thread != IDLE_THREAD) {
    pok_threads[current_thread].remaining_timeslice =
        pok_threads[current_thread].weight;
  }

  i = from;
  if (i == IDLE_THREAD) {
    i = index_low;
  } else {
    i++;
    if (i >= index_high) {
      i = index_low;
    }
  }

  while (i != from) {
    if (pok_threads[i].state == POK_STATE_RUNNABLE &&
        pok_threads[i].processor_affinity == current_proc) {
#ifdef POK_NEEDS_DEBUG
      printf("--- Scheduling processor: %hhd\n    elected thread %d "
             "(remaining_timeslice "
             "%u)\n",
             current_proc, i, pok_threads[i].remaining_timeslice);
#endif

      return i;
    }
    i++;
    if (i >= index_high) {
      i = index_low;
    }
  }

  if (pok_threads[current_thread].state == POK_STATE_RUNNABLE &&
      pok_threads[current_thread].processor_affinity == current_proc &&
      current_thread != IDLE_THREAD) {
    pok_threads[current_thread].remaining_timeslice =
        pok_threads[current_thread].weight;
#ifdef POK_NEEDS_DEBUG
    printf("--- Scheduling processor: %hhd\n  scheduling self-thread %d "
           "(remaining_timeslice "
           "%u)\n",
           current_proc, current_thread,
           pok_threads[current_thread].remaining_timeslice);
#endif

    return current_thread;
  }
#ifdef POK_NEEDS_DEBUG
  printf("--- Scheduling processor: %hhd\n    elected thread %d "
         "(IDLE_THREAD remaining_timeslice "
         "%u)\n",
         current_proc, IDLE_THREAD,
         pok_threads[IDLE_THREAD].remaining_timeslice);
#endif
  return IDLE_THREAD;
}
#endif // POK_NEEDS_SCHED_PWRR
```

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

## 动态添加线程的实现

POK将创建线程的系统调用封装为用户调用，用户态下可以直接在调用线程创建函数创建线程。

在POK的机制中，只有分区的状态处于`POK_PARTITION_MODE_INIT_COLD`或`POK_PARTITION_MODE_INIT_WARM`时，才能够进行线程的创建。一旦通过设定分区状态为`POK_PARTITION_MODE_NORMAL`启动分区使各线程开始运行后，就不能再创建新线程。

```c++
pok_ret_t pok_partition_thread_create(uint32_t *thread_id,
                                      const pok_thread_attr_t *attr,
                                      const uint8_t partition_id) {
  ...                                      
  if ((pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_COLD) &&
      (pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_WARM)) {
    return POK_ERRNO_MODE;
  }
  ...
}
```

为实现动态创建线程，需要为`pok_thread_attr_t`和`pok_thread_t`增加`bool_t`类型的属性`is_dynamic`，`is_dynamic = TRUE`表示线程是动态线程，可以在分区启动后创建。对于分区启动后创建的线程，其`next_activation_time`应当设置为当前时间刻加上其周期，即`period + POK_GETTICK()`，`weak_up_time`应当为当前时间刻`POK_GETTICK()`。具体代码改动如下：

```c++
pok_ret_t pok_partition_thread_create(uint32_t *thread_id,
                                      const pok_thread_attr_t *attr,
                                      const uint8_t partition_id) {
  ...                                      
  if ((pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_COLD)
      && (pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_WARM)
      /* if the thread is dynamic, !attr->is_dynamic will be FALSE. */
      && (!attr->dynamic)) {
      return POK_ERRNO_MODE;
  }
  ...
  if (attr->period > 0) {
      pok_threads[id].period = attr->period;
      pok_threads[id].next_activation = POK_GETTICK() + attr->period;
  }
  ...
  pok_threads[id].wakeup_time = POK_GETTICK();
  ...
}
```

为测试修改后的POK是否能正常动态创建线程，编写了一测试用例。其中`main_thread`每隔`5s`创建一新线程，共创建4个新线程，4个新线程的`period`、`time_capacity`相同，优先级依次递增。各线程的任务均为`while(1){}`的死循环。

```c++
void *main_thread() {
  const uint64_t period = 5e9;
  uint64_t time_capacity = 2;
  pok_thread_attr_t tattr;

  for (uint8_t priority = 1; priority <= 4; priority++) {
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
```

代码运行结果如下：

```
POK kernel initialized
--- Scheduling processor: 0
    scheduling thread 1 (priority 0)
    non-ready: 0 (1/stopped), 2 (0/stopped), 3 (0/stopped), 4 (0/stopped), 5 (0/stopped), 6 (0/stopped), 7 (0/stopped),)
[P1] pok_thread_create (2) return=0
--- Scheduling processor: 0
    scheduling thread 2 (priority 1)
    non-ready: 0 (1/stopped), 1 (0/waiting), 3 (0/stopped), 4 (0/stopped), 5 (0/stopped), 6 (0/stopped), 7 (0/stopped),)
--- Scheduling processor: 0
    scheduling idle thread
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 3 (0/stopped), 4 (0/stopped), 5 (0/stopped))
--- Scheduling processor: 0
    scheduling thread 2 (priority 1)
    other ready:  1 (0)
    non-ready: 0 (1/stopped), 3 (0/stopped), 4 (0/stopped), 5 (0/stopped), 6 (0/stopped), 7 (0/stopped), 8 (0/stopped),)
--- Scheduling processor: 0
    scheduling thread 1 (priority 0)
    non-ready: 0 (1/stopped), 2 (1/waiting next activation), 3 (0/stopped), 4 (0/stopped), 5 (0/stopped), 6 (0/stopped))
[P1] pok_thread_create (3) return=0
--- Scheduling processor: 0
    scheduling thread 3 (priority 2)
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 4 (0/stopped), 5 (0/stopped), 6 (0/stopped))
--- Scheduling processor: 0
    scheduling idle thread
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 3 (2/waiting next activation), 4 (0/stopped)
--- Scheduling processor: 0
    scheduling thread 2 (priority 1)
    non-ready: 0 (1/stopped), 1 (0/waiting), 3 (2/waiting next activation), 4 (0/stopped), 5 (0/stopped), 6 (0/stopped))
--- Scheduling processor: 0
    scheduling idle thread
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 3 (2/waiting next activation), 4 (0/stopped)
--- Scheduling processor: 0
    scheduling thread 3 (priority 2)
    other ready:  1 (0)
    non-ready: 0 (1/stopped), 2 (1/waiting next activation), 4 (0/stopped), 5 (0/stopped), 6 (0/stopped), 7 (0/stopped))
--- Scheduling processor: 0
    scheduling thread 1 (priority 0)
    non-ready: 0 (1/stopped), 2 (1/waiting next activation), 3 (2/waiting next activation), 4 (0/stopped), 5 (0/stopped)
[P1] pok_thread_create (4) return=0
--- Scheduling processor: 0
    scheduling thread 4 (priority 3)
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 3 (2/waiting next activation), 5 (0/stopped)
--- Scheduling processor: 0
    scheduling idle thread
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 3 (2/waiting next activation), 4 (3/waiting)
--- Scheduling processor: 0
    scheduling thread 2 (priority 1)
    non-ready: 0 (1/stopped), 1 (0/waiting), 3 (2/waiting next activation), 4 (3/waiting next activation), 5 (0/stopped)
--- Scheduling processor: 0
    scheduling idle thread
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 3 (2/waiting next activation), 4 (3/waiting)
--- Scheduling processor: 0
    scheduling thread 3 (priority 2)
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 4 (3/waiting next activation), 5 (0/stopped)
--- Scheduling processor: 0
    scheduling idle thread
    non-ready: 0 (1/stopped), 1 (0/waiting), 2 (1/waiting next activation), 3 (2/waiting next activation), 4 (3/waiting)
--- Scheduling processor: 0
    scheduling thread 4 (priority 3)
    other ready:  1 (0)
    non-ready: 0 (1/stopped), 2 (1/waiting next activation), 3 (2/waiting next activation), 5 (0/stopped), 6 (0/stopped)
--- Scheduling processor: 0
    scheduling thread 1 (priority 0)
    non-ready: 0 (1/stopped), 2 (1/waiting next activation), 3 (2/waiting next activation), 4 (3/waiting next activation)
```

分析日志，主线程运行后创建了优先级为1的线程2，然后POK调度线程2运行；此后主线程又创建了优先级为2的线程3，调度器则先后调度线程3、线程2运行；然后主线程创建了线程4，调度器先后调度了线程4、线程2、线程3运行（之所以先调度优先级较低的线程2，是因为线程2比线程3更早到达）。由运行结果可以看出，修改后的POK支持动态创建线程。

## MLFQ算法简述

1. 优先级高的队列比优先级低的队列优先执行。

2. 同一级队列采用Round-Robin运行。

3. 工作进入系统时，放在最高优先级（最上层队列）。

4. 一旦工作用完了其在某一层中的时间配额，就降低其优先级（放入低一级队列）。

5. 经过一段时间，就将系统中所有工作重新加入最高优先级队列。

## MLFQ算法的实现

使用Python实现了一个简单的MLFQ算法，代码位于[../../mlfq/mlfq.py](../../mlfq/mlfq.py)，具体内容如下。

```Python
class Process:
    def __init__(self, name, arrival_time, burst_time):
        self.name = name
        self.arrival_time = arrival_time
        self.burst_time = burst_time
        self.started = False

def log_queue(queue, current_time):
    print(f"[current time: {current_time}]")
    for i, q in enumerate(queue):
        print(f"Queue {i}: ", end="")
        for p in q:
            print(f"[name: {p.name}, burst_time: {p.burst_time}] ", end="")
        print()
    print()

def mlfq_schedule(queue, completed_processes, current_time):
    # Select a process from the queue to execute
    for i in range(len(queue)):
        if queue[i]:
            process = queue[i].pop(0)
            if process.burst_time <= 1:
                # Process completed
                current_time += process.burst_time
                process.burst_time = 0
                completed_processes.append(process)
            else:
                # Process executes for a certain time and then returns to the next level queue
                current_time += 1
                process.burst_time -= 1
                if i+1 == len(queue):
                    queue[i].append(process)  # Return to the original queue
                else:
                    queue[i+1].append(process)
            
            # Check if it's time to reinsert all processes to the highest priority queue
            if current_time % REINSERT_INTERVAL == 0:
                for q in queue[1:]:
                    for p in q:
                        queue[0].append(p)
                    q.clear()
                print(f"Reinserted all processes to the highest priority queue at time {current_time}\n")

            # Only execute one process per call to the schedule function
            break

    return current_time

def run(processes, queue_num=4, reinsert_interval=5):
    global REINSERT_INTERVAL
    REINSERT_INTERVAL = reinsert_interval

    # Create multi-level feedback queue
    queue = [[] for _ in range(queue_num)]
    current_time = 0
    completed_processes = []

    while True:
        for process in processes:
            if not process.started and process.arrival_time <= current_time:
                # Add the process to the first queue, which is the highest priority queue
                queue[0].append(process)
                process.started = True
        
        log_queue(queue, current_time)

        current_time = mlfq_schedule(queue, completed_processes, current_time)

        # Check if all processes have completed
        if all(process.burst_time == 0 for process in processes):
            break

    return completed_processes

if __name__ == "__main__":
    # Test case
    processes = [
        Process("P1", 0, 6),
        Process("P2", 2, 5),
        Process("P3", 4, 8),
        Process("P4", 6, 2)
    ]

    completed_processes = run(processes)

    for process in completed_processes:
        print(f"Process {process.name} completed")
```

简单来说，每个时间片检查是否有新的进程到达，并加入最高优先级队列，然后调用`mlfq_schedule`函数。在`mlfq_schedule`中，从高优先级向低优先级遍历队列，然后使其运行一个时间片。如果时间片用完还未执行完成，则将其放入下一优先级的队列。

此外，在一个设定的时间间隔`REINSERT_INTERVAL`后，所有任务都会被重新设定到最高优先级。
