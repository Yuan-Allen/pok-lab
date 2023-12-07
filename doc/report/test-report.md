# 测试报告

## MLFQ算法的测试用例介绍

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

## MLFQ算法的实验结果及其说明

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
