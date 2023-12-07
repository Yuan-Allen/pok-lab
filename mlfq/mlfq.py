class Process:
    def __init__(self, name, arrival_time, burst_time):
        self.name = name
        self.arrival_time = arrival_time
        self.burst_time = burst_time
        self.started = False

# 打印队列里面所有的进程，包括进程的状态
def log_queue(queue, current_time):
    print(f"\n[current time: {current_time}]")
    for i, q in enumerate(queue):
        print(f"Queue {i}: ", end="")
        for p in q:
            print(f"[name: {p.name}, burst_time: {p.burst_time}] ", end="")
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
                print(f"\nReinserted all processes to the highest priority queue at time {current_time}")

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
