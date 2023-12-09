# 测试报告

## 任务3
### 应用场景设计
在该实验中，我们设计一个智能温控系统，该系统涉及温度检测，控温调节，数据显示，警报系统等多个并行任务，每个任务的优先级，执行时间，ddl需求不同，且是周期性任务。
- **任务1：温度检测**： 优先级中，执行时间中，ddl晚。
- **任务2：控温调节**： 优先级较高，执行时间长，ddl中。
- **任务3：数据显示**： 优先级低，执行时间短，ddl较晚。
- **任务4：警报系统**： 优先级高，执行时间短，ddl早。

### 测试用例介绍

针对上述四个任务，分别周期性运行四个线程，分别通过参数priority,time_capacity,deadline,weight,entry即period来配置各线程的优先级，执行时间，ddl，wrr调度的权重，入口函数和周期。
**参数配置如下：**
| 任务线程 | 优先级 | 执行时间(ms) | ddl(ms) | WRR权重 | 周期(ms) |
| -------- | ------ | ------------ | ------- | ------- | -------- |
| 温度检测 | 44     | 4            | 20      | 1       | 20       |
| 控温调节 | 66     | 10           | 15      | 5       | 20       |
| 数据显示 | 22     | 2            | 18      | 1       | 20       |
| 警报系统 | 88     | 2            | 10      | 10      | 20       |

具体代码如下：
```c++
  tattr.priority = 44;
  tattr.entry = temperature_check;
  tattr.processor_affinity = 0;
  tattr.time_capacity = 4;
  tattr.deadline = 20000000;
  tattr.period = 20000000;
  tattr.weight = 1;

  ret = pok_thread_create(&tid, &tattr);
  printf("[P1] pok_thread_create (1)temperature_check return=%d\n", ret);

  tattr.priority = 66;
  tattr.entry = temperature_control;
  tattr.time_capacity = 10;
  tattr.deadline = 15000000;
  tattr.weight = 5;
  tattr.period = 20000000;

  ret = pok_thread_create(&tid, &tattr);
  printf("[P1] pok_thread_create (2)temperature_control return=%d\n", ret);
  tattr.priority = 22;
  tattr.entry = data_display;
  tattr.time_capacity = 2;
  tattr.deadline = 18000000;
  tattr.weight = 1;
  tattr.period = 20000000;

  ret = pok_thread_create(&tid, &tattr);
  printf("[P1] pok_thread_create (3)data_display return=%d\n", ret);
  tattr.priority = 88;
  tattr.entry = alarm;
  tattr.time_capacity = 2;
  tattr.deadline = 10000000;
  tattr.weight = 10;
  tattr.period = 20000000;

  ret = pok_thread_create(&tid, &tattr);
  printf("[P1] pok_thread_create (4)alarm return=%d\n", ret);

  pok_partition_set_mode(POK_PARTITION_MODE_NORMAL);
  pok_thread_wait_infinite();
```

### 测试结果
#### PPS
```
POK_SCHED_PPS : partition 0
POK kernel initialized
[P1] pok_thread_create (1)temperature_check return=0
[P1] pok_thread_create (2)temperature_control return=0
[P1] pok_thread_create (3)data_display return=0
[P1] pok_thread_create (4)alarm return=0
--- Scheduling processor: 0
    elected thread 4 (priority 88)
ALARM!!!
thread [1][4] finished at 3005394, deadline met, next activation: 21714734
--- Scheduling processor: 0
    elected thread 2 (priority 66)
adjust temp
thread [1][2] finished at 13008009, deadline met, next activation: 21604106
--- Scheduling processor: 0
    elected thread 1 (priority 44)
check temp
thread [1][1] finished at 17009055, deadline met, next activation: 21170813
--- Scheduling processor: 0
    elected thread 3 (priority 22)
display
thread [1][3] finished at 19000359, deadline met, next activation: 21659420
--- Scheduling processor: 0
    elected thread 5 (priority 0)
thread [1][1] activated at 21170813, deadline at 41170813
thread [1][2] activated at 21604106, deadline at 36604106
thread [1][3] activated at 21659420, deadline at 39659420
thread [1][4] activated at 21714734, deadline at 31714734
--- Scheduling processor: 0
    elected thread 4 (priority 88)
thread [1][4] finished at 24006276, deadline met, next activation: 41714734
--- Scheduling processor: 0
    elected thread 2 (priority 66)
thread [1][2] finished at 34008891, deadline met, next activation: 41604106
--- Scheduling processor: 0
    elected thread 1 (priority 44)
thread [1][1] finished at 38000718, deadline met, next activation: 41170813
--- Scheduling processor: 0
    elected thread 3 (priority 22)
thread [1][3] finished at 40001241, deadline miss, next activation: 41659420
--- Scheduling processor: 0
    elected thread 5 (priority 0)
thread [1][1] activated at 41170813, deadline at 61170813
thread [1][2] activated at 41604106, deadline at 56604106
thread [1][3] activated at 41659420, deadline at 59659420
thread [1][4] activated at 41714734, deadline at 51714734
```


#### PEDF
```
POK_SCHED_PEDF : partition 0
POK kernel initialized
[P1] pok_thread_create (1)temperature_check return=0
[P1] pok_thread_create (2)temperature_control return=0
[P1] pok_thread_create (3)data_display return=0
[P1] pok_thread_create (4)alarm return=0
--- Scheduling processor: 0
    elected thread 4 (ddl 11760829)
ALARM!!!
thread [1][4] finished at 3005394, deadline met, next activation: 21760829
--- Scheduling processor: 0
    elected thread 2 (ddl 16604106)
adjust temp
thread [1][2] finished at 13008009, deadline met, next activation: 21604106
--- Scheduling processor: 0
    elected thread 3 (ddl 19696296)
display
thread [1][3] finished at 15008532, deadline met, next activation: 21696296
--- Scheduling processor: 0
    elected thread 1 (ddl 21170813)
check temp
thread [1][1] finished at 19000359, deadline met, next activation: 21170813
--- Scheduling processor: 0
    elected thread 5 (ddl 0)
thread [1][1] activated at 21170813, deadline at 41170813
thread [1][2] activated at 21604106, deadline at 36604106
thread [1][3] activated at 21696296, deadline at 39696296
thread [1][4] activated at 21760829, deadline at 31760829
--- Scheduling processor: 0
    elected thread 4 (ddl 31760829)
thread [1][4] finished at 24006276, deadline met, next activation: 41760829
--- Scheduling processor: 0
    elected thread 2 (ddl 36604106)
thread [1][2] finished at 34008891, deadline met, next activation: 41604106
--- Scheduling processor: 0
    elected thread 3 (ddl 39696296)
thread [1][3] finished at 36000195, deadline met, next activation: 41696296
--- Scheduling processor: 0
    elected thread 1 (ddl 41170813)
thread [1][1] finished at 40001241, deadline met, next activation: 41170813
--- Scheduling processor: 0
    elected thread 5 (ddl 0)
thread [1][1] activated at 41170813, deadline at 61170813
thread [1][2] activated at 41604106, deadline at 56604106
thread [1][3] activated at 41696296, deadline at 59696296
thread [1][4] activated at 41760829, deadline at 51760829
```

#### RR
即WRR中weight都置为1.
```
POK_SCHED_WRR : partition 0
POK kernel initialized
[P1] pok_thread_create (1)temperature_check return=0
[P1] pok_thread_create (2)temperature_control return=0
[P1] pok_thread_create (3)data_display return=0
[P1] pok_thread_create (4)alarm return=0
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
check temp
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 1)
adjust temp
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
display
--- Scheduling processor: 0
    elected thread 4 (remaining_timeslice 1)
ALARM!!!
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
thread [1][3] finished at 8002092, deadline met, next activation: 21622544
--- Scheduling processor: 0
    elected thread 4 (remaining_timeslice 1)
thread [1][4] finished at 9006963, deadline met, next activation: 21659420
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
thread [1][1] finished at 12003138, deadline met, next activation: 21161594
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
thread [1][2] finished at 19000359, deadline miss, next activation: 21576449
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
thread [1][1] activated at 21161594, deadline at 41161594
thread [1][2] activated at 21576449, deadline at 36576449
thread [1][3] activated at 21622544, deadline at 39622544
thread [1][4] activated at 21659420, deadline at 31659420
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 4 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
thread [1][3] finished at 27002451, deadline met, next activation: 41622544
--- Scheduling processor: 0
    elected thread 4 (remaining_timeslice 1)
thread [1][4] finished at 28007322, deadline met, next activation: 41659420
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
thread [1][1] finished at 33004020, deadline met, next activation: 41161594
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 2 (remaining_timeslice 1)
thread [1][2] finished at 40001241, deadline miss, next activation: 41576449
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
thread [1][1] activated at 41161594, deadline at 61161594
thread [1][2] activated at 41576449, deadline at 56576449
thread [1][3] activated at 41622544, deadline at 59622544
thread [1][4] activated at 41659420, deadline at 51659420
```

#### WRR
```
POK_SCHED_WRR : partition 0
POK kernel initialized
[P1] pok_thread_create (1)temperature_check return=0
[P1] pok_thread_create (2)temperature_control return=0
[P1] pok_thread_create (3)data_display return=0
[P1] pok_thread_create (4)alarm return=0
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
check temp
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 5)
adjust temp
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
display
--- Scheduling processor: 0
    elected thread 4 (remaining_timeslice 10)
ALARM!!!
thread [1][4] finished at 10002615, deadline met, next activation: 21677858
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 5)
thread [1][2] finished at 16004184, deadline met, next activation: 21576449
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
thread [1][3] finished at 17009055, deadline met, next activation: 21622544
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 1 (remaining_timeslice 1)
thread [1][1] finished at 19000359, deadline met, next activation: 21152375
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
thread [1][1] activated at 21152375, deadline at 41152375
thread [1][2] activated at 21576449, deadline at 36576449
thread [1][3] activated at 21622544, deadline at 39622544
thread [1][4] activated at 21677858, deadline at 31677858
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 5)
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 4 (remaining_timeslice 10)
thread [1][4] finished at 30007845, deadline met, next activation: 41677858
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 5)
thread [1][2] finished at 36000195, deadline met, next activation: 41576449
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
thread [1][3] finished at 37005066, deadline met, next activation: 41622544
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 1 (remaining_timeslice 1)
thread [1][1] finished at 40001241, deadline met, next activation: 41152375
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
thread [1][1] activated at 41152375, deadline at 61152375
thread [1][2] activated at 41576449, deadline at 56576449
thread [1][3] activated at 41622544, deadline at 59622544
thread [1][4] activated at 41677858, deadline at 51677858
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 5)
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 4 (remaining_timeslice 10)
thread [1][4] finished at 50003856, deadline met, next activation: 61677858
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
    elected thread 2 (remaining_timeslice 5)
thread [1][2] finished at 56005425, deadline met, next activation: 61576449
--- Scheduling processor: 0
    elected thread 3 (remaining_timeslice 1)
thread [1][3] finished at 57001077, deadline met, next activation: 61622544
--- Scheduling processor: 0
    elected thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
  scheduling self-thread 1 (remaining_timeslice 1)
thread [1][1] finished at 60006471, deadline met, next activation: 61152375
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
--- Scheduling processor: 0
    elected thread 5 (IDLE_THREAD remaining_timeslice 0)
```

### 各算法优劣比较
经过比较，我们发现：
- 在PPS调度方式下，可以保证高优先级的线程先得到执行，但其忽视了任务的执行时间与deadline，可能导致一些较低优先级任务miss deadline。此外可能出现任务饥饿，得不到执行等问题。
- 在PEDF调度方式下，可调度集问题下，总能尽可能地满足所有ddl。然而，这种方式忽略了任务的优先级与执行时间，在不可调度问题集下，不能让尽可能少的任务miss ddl，也不能尽量保证高优先级任务met ddl。此外可能出现任务饥饿，得不到执行等问题。
- 在RR调度方式下，各线程调度更公平，线程不会饿死。但调度死板，各任务的差异被忽视，优先级失去意义，ddl也更可能miss。
- 在WRR调度方式下，可能得到较为公平且尽可能满足ddl的调度，但这需要用户在充分了解任务的基础上聪明地配置线程权重。

## 任务4
### 通常任务与紧急任务的调度实验

### 测试用例介绍

在该实验中，我们设想如下场景：有一个自动驾驶系统，周期性地使用神经网络识别道路上的交通标志，以提供车辆行驶需要的信息。然而由于计算硬件并不单纯处理交通标志识别任务，因此交通标志识别任务会偶尔错过deadline，此时系统将立即向云端发送车辆当前位置，请求附近的交通标志信息。系统中的任务设计如下：
```c++
  // 通常任务，神经网络识别交通标志的任务
  tattr.priority = 40;
  tattr.entry = general_task;
  tattr.processor_affinity = 0;
  tattr.time_capacity = 5e2;
  tattr.period = 10e8;
  tattr.deadline = 8e8;
  tattr.weight = 1e4;

  // 其他任务，会周期性地使通常任务 miss deadline
  tattr.priority = 254;
  tattr.entry = trouble_task;
  tattr.processor_affinity = 0;
  tattr.time_capacity = 4e2;
  tattr.period = 20e8;
  tattr.deadline = 6e8;
  tattr.weight = 1e4;

  //紧急任务，在通常任务miss deadline时生成，向云端请求附近的交通标志信息
  tattr.priority = 255;
  tattr.entry = urgent_task;
  tattr.processor_affinity = 0;
  tattr.time_capacity = 1e2;
  tattr.period = -1;
  tattr.deadline = 2e8;
  tattr.weight = 10e10;
```
为实现紧急任务的生成，向系统中添加如下代码：
```c++
#ifdef POK_NEEDS_DDL_PUNISH
        if ((POK_CURRENT_THREAD.deadline > 0) && (POK_CURRENT_THREAD.ddl <
        now) && (urgent_task_flag)) {
              urgent_task_flag = FALSE;
              uint32_t tid;
              pok_ret_t ret;
              pok_thread_attr_t tattr;
              //set tattr...
              ret = pok_partition_thread_create(&tid, &tattr,pok_current_partition);
              printf("[P1] pok_thread_create (urgent) return=%d at %llu, deadline is %llu\n", ret, POK_GETTICK(),POK_GETTICK() + tattr.deadline);
        }
#endif
```
然后测试、分析不同的调度策略的调度效果。

### 实验结果

#### 抢占式固定优先级调度
```
POK kernel initialized
thread [0][1] actived at 0, deadline 600000000
thread [0][2] actived at 0, deadline 800000000
thread [1][0] scheduled at 1004871
thread [1][2] scheduled at 1530354
[TROUBLE] I'm trouble task.
...
thread [1][2] finished at 401008062, deadline met, next activation: 2001170813
thread [1][1] scheduled at 401008062
[GENERAL] I'm general task.
...
thread [1][1] scheduled at 802006905, deadline miss
[P1] pok_thread_create (urgent) return=0 at 802006905, deadline is 1002006905
thread [1][3] scheduled at 802006905
...
thread [1][3] finished at 902005398, deadline met, next activation: 4553976100792185
...
```
观察日志，由于紧急任务具有最高优先级，因此紧急任务生成后立刻抢占CPU，并在deadline到来前完成。然而，抢占式的固定优先级策略有可能使得系统中的低优先级任务始终无法得到计算资源，如多媒体系统、空调控制系统等。
#### round-robin
```
POK kernel initialized
thread [1][0] scheduled at 1004871
thread [1][2] scheduled at 1585668
[TROUBLE] I'm trouble task.
...
thread [1][2] finished at 401008237, deadline met, next activation: 2001170839
thread [1][1] scheduled at 401008237
[GENERAL] I'm general task.
...
thread [1][1] scheduled at 802006815, deadline miss
[P1] pok_thread_create (urgent) return=0 at 802006815, deadline is 1002006815
thread [1][1] scheduled at 802006815
...
thread [1][1] finished at 901000527, deadline miss, next activation: 2001170813
thread [1][3] scheduled at 901000527
...
thread [1][3] finished at 1001008239, deadline miss, next activation: 4553932951113020
```
观察日志，紧急任务生成后，round-robin调度策略会继续调度通常任务直到通常任务完成，然而此时通常任务早已错过deadline。相应地，此时才开始调度的紧急任务也错过了deadline。round-robin策略无法识别任务的紧急程度进行调度，在时间片较大时容易使紧急任务不能及时执行；而如果时间片太小，上下文切换的开销则过高。
#### weighted-round-robin
```
POK kernel initialized
thread [1][0] scheduled at 1004871
thread [1][2] scheduled at 1585597
[TROUBLE] I'm trouble task.
...
thread [1][2] finished at 401008292, deadline met, next activation: 2001170841
thread [1][1] scheduled at 401008292
[GENERAL] I'm general task.
...
thread [1][1] scheduled at 802006412, deadline miss
[P1] pok_thread_create (urgent) return=0 at 802006412, deadline is 1002006412
thread [1][1] scheduled at 802006412
...
thread [1][1] finished at 901000527, deadline miss, next activation: 2001170841
thread [1][3] scheduled at 901000527
...
thread [1][3] finished at 1001008233, deadline miss, next activation: 4553932951113020
```
观察日志，与普通round-robin的问题相同，尽管weighted-round-robin能够为重要任务分配更多的时间，但是作为一个非抢占式的调度策略，其仍然不具有处理的能力。紧急任务生成后，weighted-round-robin调度策略会继续调度通常任务直到通常任务完成，然而此时通常任务早已错过deadline。相应地，这时候才开始调度的紧急任务也错过了deadline。
#### 抢占式EDF
```
POK kernel initialized
thread [1][0] scheduled at 1004871
thread [1][2] scheduled at 1604106 (ddl 601207689)
...
thread [1][2] finished at 401008062, deadline met, next activation: 2001207689
thread [1][1] scheduled at 401008062 (ddl 801170813)
...
thread [1][1] scheduled at 801002034 (ddl 801170813)
[P1] pok_thread_create (urgent) return=0 at 802006905, deadline is 1002006905
thread [1][1] scheduled at 802006905 (ddl 801170813)
...
thread [1][1] finished at 901000527, deadline miss, next activation: 1001170813
thread [1][3] scheduled at 901000527 (ddl 1002006905)
...
thread [1][3] finished at 1002008239, deadline miss, next activation: 4554053410203513
```
观察日志，抢占式EDF并不会放弃已经错过deadline的任务，因此紧急任务生成后，抢占式EDF调度策略仍会继续调度通常任务直到通常任务完成，然而此时通常任务早已错过deadline。相应地，这时候才开始调度的紧急任务也错过了deadline。
#### 抢占式weighted-round-robin
```
POK kernel initialized
thread [1][0] scheduled at 1004871
[P1] pok_thread_create (1) return=0
--- Scheduling processor: 0
  preemptive thread 2 (remaining_timeslice 10000)
  continue thread 2 (remaining_timeslice 9999)
  ...
  elected thread 1 (remaining_timeslice 10000)
  continue thread 1 (remaining_timeslice 9999)
  ...
  preemptive thread 2 (remaining_timeslice 10000)
  continue thread 2 (remaining_timeslice 9999)
  ...
  elected thread 1 (remaining_timeslice 10000)
  continue thread 1 (remaining_timeslice 9999)
  ...
  [P1] pok_thread_create (urgent) return=0 at 802006905, deadline is 1002006905
  preemptive thread 3 (remaining_timeslice 1000000000)
  continue thread 3 (remaining_timeslice 999999999)
  ...
  thread [1][3] finished at 902005398, deadline met, next activation: 4554100654843769
```
观察日志，抢占式weighted-round-robin首先轮流调度通常任务和其他任务，通常任务错过deadline，紧急任务生成后，紧急任务立即抢占，独占资源进行执行，并在deadline之前完成，补救了通常任务的miss deadline。由于抢占机制和round-robin的机制的共存，该调度算法既有处理紧急任务的能力，又能够使得一些低优先级的任务能够正常执行不至于饿死。
## 任务5
### MLFQ算法的测试用例介绍

MLFQ算法及其测试用例均使用Python实现。算法设计与实现可参考设计文档。测试中的对象定义以及测试用例内容如下所示。

具体来书，测试用例中共构造了四个任务，其到达时间分别为0、2、4、6，运行完成所需时间分别为6、5、8、2。

```Python
class Process:
    def __init__(self, name, arrival_time, burst_time):
        self.name = name
        self.arrival_time = arrival_time
        self.burst_time = burst_time
        self.started = False

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

### MLFQ算法的实验结果及其说明

测试结果如下所示。

系统中共定义了四个队列，`Queue 0`的优先级最高。`P1`在时刻为0时到达，`P2`、`P3`和`P4`则分别在时刻为2、4、6时到达。`burst_time`为任务完成所需剩余时间。从结果中可以看到，任务若结束了当前时间片的执行还未运行结束，就会被放入更低优先级的队列。

此外，系统分别在时刻为5、10、15、20时将所有任务都提到了最高优先级，这是由于我们在系统中设置的`REINSERT_INTERVAL`值为5。

```
[current time: 0]
Queue 0: [name: P1, burst_time: 6]
Queue 1:
Queue 2:
Queue 3:

[current time: 1]
Queue 0:
Queue 1: [name: P1, burst_time: 5]
Queue 2:
Queue 3:

[current time: 2]
Queue 0: [name: P2, burst_time: 5]
Queue 1:
Queue 2: [name: P1, burst_time: 4]
Queue 3:

[current time: 3]
Queue 0:
Queue 1: [name: P2, burst_time: 4]
Queue 2: [name: P1, burst_time: 4]
Queue 3:

[current time: 4]
Queue 0: [name: P3, burst_time: 8]
Queue 1:
Queue 2: [name: P1, burst_time: 4] [name: P2, burst_time: 3]
Queue 3:

Reinserted all processes to the highest priority queue at time 5

[current time: 5]
Queue 0: [name: P3, burst_time: 7] [name: P1, burst_time: 4] [name: P2, burst_time: 3]
Queue 1:
Queue 2:
Queue 3:

[current time: 6]
Queue 0: [name: P1, burst_time: 4] [name: P2, burst_time: 3] [name: P4, burst_time: 2]
Queue 1: [name: P3, burst_time: 6]
Queue 2:
Queue 3:

[current time: 7]
Queue 0: [name: P2, burst_time: 3] [name: P4, burst_time: 2]
Queue 1: [name: P3, burst_time: 6] [name: P1, burst_time: 3]
Queue 2:
Queue 3:

[current time: 8]
Queue 0: [name: P4, burst_time: 2]
Queue 1: [name: P3, burst_time: 6] [name: P1, burst_time: 3] [name: P2, burst_time: 2]
Queue 2:
Queue 3:

[current time: 9]
Queue 0:
Queue 1: [name: P3, burst_time: 6] [name: P1, burst_time: 3] [name: P2, burst_time: 2] [name: P4, burst_time: 1]
Queue 2:
Queue 3:

Reinserted all processes to the highest priority queue at time 10

[current time: 10]
Queue 0: [name: P1, burst_time: 3] [name: P2, burst_time: 2] [name: P4, burst_time: 1] [name: P3, burst_time: 5]
Queue 1:
Queue 2:
Queue 3:

[current time: 11]
Queue 0: [name: P2, burst_time: 2] [name: P4, burst_time: 1] [name: P3, burst_time: 5]
Queue 1: [name: P1, burst_time: 2]
Queue 2:
Queue 3:

[current time: 12]
Queue 0: [name: P4, burst_time: 1] [name: P3, burst_time: 5]
Queue 1: [name: P1, burst_time: 2] [name: P2, burst_time: 1]
Queue 2:
Queue 3:

[current time: 13]
Queue 0: [name: P3, burst_time: 5]
Queue 1: [name: P1, burst_time: 2] [name: P2, burst_time: 1]
Queue 2:
Queue 3:

[current time: 14]
Queue 0:
Queue 1: [name: P1, burst_time: 2] [name: P2, burst_time: 1] [name: P3, burst_time: 4]
Queue 2:
Queue 3:

Reinserted all processes to the highest priority queue at time 15

[current time: 15]
Queue 0: [name: P2, burst_time: 1] [name: P3, burst_time: 4] [name: P1, burst_time: 1]
Queue 1:
Queue 2:
Queue 3:

[current time: 16]
Queue 0: [name: P3, burst_time: 4] [name: P1, burst_time: 1]
Queue 1:
Queue 2:
Queue 3:

[current time: 17]
Queue 0: [name: P1, burst_time: 1]
Queue 1: [name: P3, burst_time: 3]
Queue 2:
Queue 3:

[current time: 18]
Queue 0:
Queue 1: [name: P3, burst_time: 3]
Queue 2:
Queue 3:

[current time: 19]
Queue 0:
Queue 1:
Queue 2: [name: P3, burst_time: 2]
Queue 3:

Reinserted all processes to the highest priority queue at time 20

[current time: 20]
Queue 0: [name: P3, burst_time: 1]
Queue 1:
Queue 2:
Queue 3:

Process P4 completed
Process P2 completed
Process P1 completed
Process P3 completed
```


## 选做任务：在pok中实现mlfq调度并进行测试
### 测试用例
在任务3所设计的场景中，测试mlfq的表现，配置同任务3。此外，为方便观察时间片大小变化以及调度发生原因，间隔单位时间输出当前线程信息。

### 测试结果
```
POK_SCHED_MLFQ : partition 0
POK kernel initialized
[P1] pok_thread_create (1)temperature_check return=0
[P1] pok_thread_create (2)temperature_control return=0
[P1] pok_thread_create (3)data_display return=0
[P1] pok_thread_create (4)alarm return=0
--- Scheduling processor: 0
  sche thread 1 (remaining_timeslice 1)
check temp
--- Scheduling processor: 0
  sche thread 2 (remaining_timeslice 1)
adjust temp
--- Scheduling processor: 0
  sche thread 3 (remaining_timeslice 1)
display
--- Scheduling processor: 0
  sche thread 4 (remaining_timeslice 1)
ALARM!!!
--- Scheduling processor: 0
  sche thread 4 (remaining_timeslice 2)
thread [1][4] finished at 6001569, deadline met, next activation: 21687077
--- Scheduling processor: 0
  sche thread 1 (remaining_timeslice 2)
--- Scheduling processor: 0
  continue thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
  sche thread 2 (remaining_timeslice 2)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  sche thread 3 (remaining_timeslice 2)
thread [1][3] finished at 11007486, deadline met, next activation: 21640982
--- Scheduling processor: 0
  sche thread 1 (remaining_timeslice 4)
thread [1][1] finished at 12003138, deadline met, next activation: 21161594
--- Scheduling processor: 0
  sche thread 2 (remaining_timeslice 4)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 3)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 2)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  sche thread 2 (remaining_timeslice 8)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 7)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 6)
thread [1][2] finished at 19000359, deadline miss, next activation: 21604106
--- Scheduling processor: 0
  sche thread 5 (remaining_timeslice 0)
--- Scheduling processor: 0
  sche thread 5 (remaining_timeslice 0)
--- Scheduling processor: 0
  sche thread 5 (remaining_timeslice 0)
thread [1][1] activated at 21161594, deadline at 41161594
thread [1][2] activated at 21604106, deadline at 36604106
thread [1][3] activated at 21640982, deadline at 39640982
thread [1][4] activated at 21687077, deadline at 31687077
--- Scheduling processor: 0
  sche thread 3 (remaining_timeslice 1)
--- Scheduling processor: 0
  sche thread 4 (remaining_timeslice 1)
--- Scheduling processor: 0
  sche thread 4 (remaining_timeslice 4)
thread [1][4] finished at 25001928, deadline met, next activation: 41687077
--- Scheduling processor: 0
  sche thread 1 (remaining_timeslice 3)
--- Scheduling processor: 0
  continue thread 1 (remaining_timeslice 2)
--- Scheduling processor: 0
  continue thread 1 (remaining_timeslice 1)
--- Scheduling processor: 0
  sche thread 3 (remaining_timeslice 4)
thread [1][3] finished at 29002974, deadline met, next activation: 41640982
--- Scheduling processor: 0
  sche thread 1 (remaining_timeslice 8)
thread [1][1] finished at 30007845, deadline met, next activation: 41161594
--- Scheduling processor: 0
  sche thread 2 (remaining_timeslice 5)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 4)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 3)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 2)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 1)
--- Scheduling processor: 0
  sche thread 2 (remaining_timeslice 16)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 15)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 14)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 13)
--- Scheduling processor: 0
  continue thread 2 (remaining_timeslice 12)
thread [1][2] finished at 40001241, deadline miss, next activation: 41604106
--- Scheduling processor: 0
  sche thread 5 (remaining_timeslice 0)
--- Scheduling processor: 0
  sche thread 5 (remaining_timeslice 0)
thread [1][1] activated at 41161594, deadline at 61161594
thread [1][2] activated at 41604106, deadline at 56604106
thread [1][3] activated at 41640982, deadline at 59640982
thread [1][4] activated at 41687077, deadline at 51687077
```

### 测试分析
可以看到，执行时间较短的任务在时间片未用完时便释放了计算资源，其优先级并未改变，且剩余时间片可以在下一周期中继续使用。
执行时间较长的任务多次降级，其时间片变大，但优先级变低，在后续周期中延后执行。测试结果表明mlfq调度功能正确，符合预期。