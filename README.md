# Uni-Processor-Scheduler  
 
The purpose of this project is to use C++ to implement a single-threaded
program to simulate a scheduling system of a computer. The scheduling system should be able to
admit new processes to ready queue(s), select a process from ready queue to run using a particular
scheduling algorithm, move the running process to a blocked queue when it has to wait for an event
like I/O completion or mutex signals and put them back to ready queue(s) when the event occurs

## Reqirements

In this project, there is no need to actually schedule processes on a real CPU, which requires
deeper knowledge about Linux system and is not suitable for this project. The implemented scheduling
system would work in a simulated way. Both processes and processor are simulated
by program. So, when we say a process is dispatched to CPU or I/O device in the program,
no new process or thread would be created or actually be dispatched to the device, we need to just
simply mark the process’s status as “running on CPU” or “using I/O device”.

### 1. Overall Work Flow

Implement a simulated scheduling system of a computer with only one CPU, 
one keyboard and one disk. Each of these devices provides service to one process at a time, so
scheduling is required to coordinate the processes. Scheduling system should work in a loop to
emulate the of behavior the CPU. Each loop is called a tick. In each loop iteration, the tick is
incremented by one to emulate the elapsed internal time of the CPU.

![image](https://user-images.githubusercontent.com/60174747/126771673-118b337a-8091-463f-9a30-0c0803021f08.png)

**Step 1.** At the beginning of each tick, the scheduler would check whether there are new processes
to be admitted to the ready queue. If multiple processes arrive at this tick, they should be enqueued
in ascending order of their process IDs.

**Step 2.** There is one block queue for each I/O device and each mutex. Processes waiting for an
I/O device or a mutex are inserted into the corresponding block queue. The event handler would
dispatch processes from block queues in FCFS manner to the I/O devices they are waiting for if the
devices become available. If a process is done with the device, it would be re-inserted to the ready
queue. Besides, the event handler would also check processes waiting for mutexes. Similar to I/O,
only the first process waiting in the mutex block queue is able to lock the mutex after the mutex is
unlocked by another process.

**Step 3.** If there is no process running on the CPU, your scheduling system would dispatch a
process to CPU from the ready queue according to one of the following scheduling algorithms:
First-Come-First-Served (FCFS), Round Robin (RR) and Feedback Scheduling (FB). At the end of
the tick, your scheduler should look ahead the service requested by the currently running process in
the next tick to determine the action required. For example, if the service next tick is disk I/O, then
blocking of the current process is required. If mutex operations (lock & unlock), which are assumed
to have a time cost of zero tick, are encountered while looking ahead, they should be executed
immediately and the scheduler would keep finding the next service which is not a mutex operation.
Finally, if the process on CPU uses up its time quantum, it should be preempted and placed at the
end of the ready queue (RR & FB only).

To make the implementation simpler, when a process is dispatched to CPU or I/O devices, it can be
kept in its current ready queue or block queue. In other word, there is no need to move the processes
somewhere else in your program to simulate that they are in the CPU or I/O devices.

### 2. Processes

The information of processes is given in a text file with the following format.

    # process_id arrival_time service_number
    service_type service_description
    service_type service_description
    … … … …
    service_type service_description

There are 3 integers in the first line and the # marks the beginning of a process. The first number is
the process ID. The second number in the arrival time of the process in number of ticks. The third
number is the number of services requested by the process. Services are the resources the process
needs or some particular operations that the process executes. The following _service_number_
lines are the services requested and the description of the service. There are 5 different types of
services, namely C (CPU), K (keyboard input), D (disk I/O), L (mutex lock) and U (mutex unlock).
For service type C, K and D, the service description is an integer which is the number of ticks
required to complete the service. It is worth noting that the number of ticks required to complete
service K or D does NOT include the waiting time in the block queue. The keyboard and the disk
provide I/O service to processes in a FCFS manner. In other words, only the first process in each
block queue can receive I/O service. For service type L (mutex lock) and U (mutex unlock), the
service description is a string representing the name of the mutex. Similarly, if multiple processes
are waiting for a mutex to unlock, after other process unlocks it, the first process waiting in the
queue would get the mutex and lock it. The lock and unlock operations take 0
ticks, which means they should be executeted immediately.

    # 0 0 8
    C 2
    D 6
    C 3
    K 5
    C 4
    L mtx
    C 5
    U mtx

Above is an example of a short process. This process with ID 0 arrives at scheduling system at
tick 0. It requires 8 services in total. To complete its task, we need to schedule 2 ticks on CPU, 6
ticks for disk I/O, then another 3 ticks on CPU, 5 ticks to wait for keyboard input, then 4 ticks on
CPU, 0 tick to lock the mutex mtx, 5 ticks on CPU, and finally 0 ticks to unlock the mutex mtx.
For simplicity and reasonability, the possible service type at the end of every service sequences
could only be C or U. And the process IDs are assumed to be consecutive integers from 0 to N-1, if
there are N processes.

### 3. Blocking

Several events may lead to the blocking of current process, for example, I/O interrupts or mutex
lock. Task is to implement multiple blocked queues for efficiently storing processes that are
blocked by such events. To be more exact, every kind of I/O interrupt (K and F) and every mutex
should have its own blocked queue. The event handler of scheduling system should then handle
the processes in blocked queues in a FCFS manner: only the first process at each blocked queue can
receive the I/O service or mutex signals.

#### 3.1 I/O Interrupts

Implement two I/O interrupts in your scheduling system: keyboard input and
disk I/O. When an I/O interrupt occurs, your scheduling system should block the current process,
putting it into the corresponding blocked queue and after I/O completes, put the process back to
ready queue.

#### 3.2 Mutexes

Since scheduling system is a single-threaded program, the mutexes from pthread.h cannot be
applied to system. Instead, we should implement own mutex structure to allow mutual
exclusion in this simulated system. Mutex should at least have one variable to store its status
(locked or unlocked) and have two functions mutex_lock and mutex_unlock to lock or unlock the
mutex, just like the ones in pthread.h. All mutexes used by the given processes would be inferred
from the given text file. Additionally, when several processes are waiting for a mutex in block queue,
only the one at the front can get the mutex once it is unlocked.

#### 3.3 Scheduling Algorithms

Implement 3 scheduling algorithms: FCFS, RR and Feedback scheduling.

##### 3.3.1 FCFS

First-Come-First-Served (FCFS) scheduling is the simplest scheduling algorithm. When the current
process ceases to execute, FCFS selects the process that has been in the ready queue the longest.

##### 3.3.2 RR

Round Robin (RR) algorithm uses preemption based on the tick of your scheduling system. Clock
interrupts would be generated every K (K=5) ticks. When a clock interrupt occurs, the currently
running process is placed in the ready queue, select next ready job on a FCFS basis.

##### 3.3.3 FB

Feedback Scheduling (FB) is a complicated algorithm and has many implementations. In this
project, we will implement a simple version of Feedback Scheduling using a three-level
feedback ready queue, namely RQ0, RQ1 and RQ2. Different queues are of different level of priority.
RQ0 has the highest priority and RQ2 has the lowest. Specifically, FB should follow the four
rules below:

1. A new process is inserted at the end of the top-level ready queue RQ0.


2. When current process ceases to execute, select the first process from the first non-empty ready
queue that has the highest priority.

3. Each ready queue works similarly to RR with a time quantum of K (K=5) ticks. If a process all
up its quantum time, it is preempted by other processes in ready queue. The processes
preempted are demoted to the next lower priority queue. (i.e. from RQi to RQi+1) There is no
process demotion for those in RQ2 since RQ2 has the lowest priority.

4. If a process blocks for I/O interrupts or mutexes, after it is ready again, it is re-inserted to the
tail of the same ready queue where it is dispatched earlier.

#### 4. Input & Output Samples

program must accept 3 arguments from command line. For example, your program should be
able to be compiled by the following command:

    g++ scheduler.cpp -o scheduler

And executes with:

    .scheduler FCFS processes.txt outputs.txt

The first argument is the name of scheduling algorithm, namely FCFS, RR and FB. The second
argument is the path of the process text file. The last argument is the name of the output file.

Program should output a text file which stores the scheduling details of each processes. For
each process, two lines should be written. The first line is the process ID of that process, i.e. process
0. The second line includes the time period that the process runs on CPU. Specifically, if a process
is scheduled to run on CPU from the a-th tick to b-th tick and from c-th tick to d-th tick, then
second line should be “a b c d”


### Visualisation of the output

How to visualize:
1. open vis.html
2. click the open button on the page, select the output txt file you want to visualize
3. visualization done




