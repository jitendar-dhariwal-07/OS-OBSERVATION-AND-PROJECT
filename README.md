PROGRAM OF EACH EXPERIMENTS(2-15)

Operating Systems Lab – Programs and Sample Outputs
Programs and sample outputs for each experiment, in order. Experiment 1 (Linux commands, filters and Ubuntu installation) consists of shell commands and is covered in the lab manual.
Experiment 2: Process Creation using fork() and Usage of getpid(), getppid(), wait() Functions
Part A: fork() process creation
```c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main() {
    fork();
    fork();
    printf("Hello World\n");
    return 0;
}
```
Part B: Usage of getpid() and getppid()
```c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
void main() {
    int process_id, p_process_id;
    /* getpid() returns the process id of the calling function */
    process_id = getpid();
    /* getppid() returns the process id of the parent function */
    p_process_id = getppid();
    printf("The process id: %d\n", process_id);
    printf("The process id of parent function: %d\n", p_process_id);
}
```
Part C: Usage of wait()
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int main() {
    pid_t cpid;
    if (fork() == 0)
        exit(0);              /* terminate child */
    else
        cpid = wait(NULL);    /* reaping child */
    printf("Parent pid = %d\n", getpid());
    printf("Child pid = %d\n", cpid);
    return 0;
}
```
Sample output
![Sample output of Experiment 2](images/ex02_output.png)
---
Experiment 3: Multithreading
Part A: Even or odd check using POSIX threads
```c
#include <stdio.h>
#include <pthread.h>
void *check(void *arg) {
    int n = *(int *)arg;
    printf("%d is %s\n", n, (n % 2 == 0) ? "Even" : "odd");
    return NULL;
}
int main() {
    pthread_t t;
    int num = 15;
    pthread_create(&t, NULL, check, &num);
    pthread_join(t, NULL);
    return 0;
}
```
Part B: Single processor multithreading (Factory quality control)
```c
#include <stdio.h>
#include <pthread.h>
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int defective = 0;
void *inspection(void *id) {
    int p = *(int *)id;
    pthread_mutex_lock(&lock);
    if (p % 2 == 0)
        defective++;
    printf("Product %d: %s\n", p, p % 2 == 0 ? "DEFECTIVE" : "ok");
    pthread_mutex_unlock(&lock);
    return NULL;
}
int main() {
    pthread_t t[5];
    int id[5];
    for (int i = 0; i < 5; i++) {
        id[i] = i + 1;
        pthread_create(&t[i], NULL, inspection, &id[i]);
    }
    for (int i = 0; i < 5; i++)
        pthread_join(t[i], NULL);
    printf("Defective : %d\n", defective);
    return 0;
}
```
Part C: Multiprocessor multithreading (Line inspection)
```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define N 4
int defects[N] = {2, 1, 3, 0};
int total = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
void *qc(void *a) {
    int i = *(int *)a;
    if (defects[i] <= 2) {
        pthread_mutex_lock(&m);
        total++;
        pthread_mutex_unlock(&m);
    }
    printf("Line %d inspected, defects = %d\n", i, defects[i]);
    return NULL;
}
int main() {
    pthread_t t[N];
    int id[N];
    for (int i = 0; i < N; i++) {
        id[i] = i;
        pthread_create(&t[i], NULL, qc, &id[i]);
    }
    for (int i = 0; i < N; i++)
        pthread_join(t[i], NULL);
    printf("Accepted = %d\n", total);
    return 0;
}
```
Sample output
![Sample output of Experiment 3](images/ex03_output.png)
---
Experiment 4: Mutual Exclusion using Semaphore and Monitor
Part A: Mutual exclusion using mutex
```c
#include <stdio.h>
#include <pthread.h>
pthread_mutex_t ehr_mutex = PTHREAD_MUTEX_INITIALIZER;
int ehr_version = 0;
void *update_ehr(void *arg) {
    pthread_mutex_lock(&ehr_mutex);
    printf("Thread %ld updating EHR (v%d -> v%d)\n", (long)arg, ehr_version, ehr_version + 1);
    ehr_version++;
    pthread_mutex_unlock(&ehr_mutex);
    return NULL;
}
int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, update_ehr, (void *)1);
    pthread_create(&t2, NULL, update_ehr, (void *)2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Final EHR version: %d\n", ehr_version);
    return 0;
}
```
Part B: Mutual exclusion using semaphore
```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
sem_t ehr_sem;
int ehr_version = 0;
void *update_ehr(void *arg) {
    sem_wait(&ehr_sem);
    printf("Thread %ld updating EHR (v%d -> v%d)\n", (long)arg, ehr_version, ehr_version + 1);
    ehr_version++;
    sem_post(&ehr_sem);
    return NULL;
}
int main() {
    pthread_t t1, t2;
    sem_init(&ehr_sem, 0, 1);
    pthread_create(&t1, NULL, update_ehr, (void *)1);
    pthread_create(&t2, NULL, update_ehr, (void *)2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Final EHR version: %d\n", ehr_version);
    sem_destroy(&ehr_sem);
    return 0;
}
```
Part C: Mutual exclusion using monitor
```c
#include <stdio.h>
#include <pthread.h>
pthread_mutex_t ehr_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ehr_cond = PTHREAD_COND_INITIALIZER;
int ehr_version = 0;
void *update_ehr(void *arg) {
    pthread_mutex_lock(&ehr_mtx);
    while (ehr_version < 0)
        pthread_cond_wait(&ehr_cond, &ehr_mtx);
    printf("Thread %ld updating EHR (v%d -> v%d)\n", (long)arg, ehr_version, ehr_version + 1);
    ehr_version++;
    pthread_cond_signal(&ehr_cond);
    pthread_mutex_unlock(&ehr_mtx);
    return NULL;
}
int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, update_ehr, (void *)1);
    pthread_create(&t2, NULL, update_ehr, (void *)2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Final EHR version: %d\n", ehr_version);
    pthread_mutex_destroy(&ehr_mtx);
    pthread_cond_destroy(&ehr_cond);
    return 0;
}
```
Sample output
![Sample output of Experiment 4](images/ex04_output.png)
---
Experiment 5: Reader-Writer Problem
News-website CMS
```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
sem_t s;
int article = 100;
void *r(void *arg) {
    sem_wait(&s);
    printf("Read %d\n", article);
    sem_post(&s);
    return NULL;
}
void *w(void *arg) {
    printf("Writer is requesting for editing\n");
    sem_wait(&s);
    article += 10;
    printf("write %d\n", article);
    sem_post(&s);
    return NULL;
}
int main() {
    pthread_t tr, tw;
    sem_init(&s, 0, 1);
    pthread_create(&tr, NULL, r, NULL);
    pthread_join(tr, NULL);
    pthread_create(&tw, NULL, w, NULL);
    pthread_join(tw, NULL);
    sem_destroy(&s);
    return 0;
}
```
Sample output
![Sample output of Experiment 5](images/ex05_output.png)
---
Experiment 6: Dining Philosophers Problem
Movie rendering pipeline
```c
#include <stdio.h>
#include <pthread.h>
pthread_mutex_t gpu = PTHREAD_MUTEX_INITIALIZER, buf = PTHREAD_MUTEX_INITIALIZER;
void *p(void *a) {
    pthread_mutex_lock(&gpu);
    pthread_mutex_lock(&buf);
    printf("Render job done\n");
    pthread_mutex_unlock(&buf);
    pthread_mutex_unlock(&gpu);
    return NULL;
}
int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, p, NULL);
    pthread_create(&t2, NULL, p, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
```
Sample output
![Sample output of Experiment 6](images/ex06_output.png)
---
Experiment 7: Banker's Algorithm for Deadlock Avoidance
Part A: Deadlock with CPU and RAM (cloud resources)
```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
pthread_mutex_t cpu, ram;
void *vm1(void *a) {
    pthread_mutex_lock(&cpu);
    sleep(1);
    pthread_mutex_lock(&ram);
    return NULL;
}
void *vm2(void *a) {
    pthread_mutex_lock(&ram);
    sleep(1);
    pthread_mutex_lock(&cpu);
    return NULL;
}
int main() {
    pthread_t t1, t2;
    pthread_mutex_init(&cpu, NULL);
    pthread_mutex_init(&ram, NULL);
    pthread_create(&t1, NULL, vm1, NULL);
    pthread_create(&t2, NULL, vm2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
```
Part B: Banker's algorithm (safe sequence)
```c
#include <stdio.h>
int main() {
    int p = 3, r = 3;
    int a[3][3] = {{0, 1, 0}, {2, 0, 0}, {3, 0, 2}};
    int m[3][3] = {{7, 5, 3}, {3, 2, 2}, {9, 0, 2}};
    int av[3] = {3, 3, 2};
    int n[3][3], f[3] = {0}, s[3], c = 0, i, j, k;
    for (i = 0; i < p; i++)
        for (j = 0; j < r; j++)
            n[i][j] = m[i][j] - a[i][j];
    for (k = 0; k < p; k++)
        for (i = 0; i < p; i++)
            if (!f[i]) {
                for (j = 0; j < r && n[i][j] <= av[j]; j++);
                if (j == r) {
                    for (j = 0; j < r; j++)
                        av[j] += a[i][j];
                    s[c++] = i;
                    f[i] = 1;
                }
            }
    printf("Safe sequence: ");
    for (i = 0; i < c; i++)
        printf("P%d ", s[i]);
    printf("\n");
    return 0;
}
```
Part C: Deadlock avoidance using resource allocation graph
```c
#include <stdio.h>
int main() {
    int edge[3][3] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};
    int req = 0, src = 2, dst = 0;
    edge[src][dst] = 1;
    if (edge[0][1] && edge[1][2] && edge[2][0])
        req = 1;
    if (req)
        printf("Request denied: cycle detected\n");
    else
        printf("Request granted: no cycle\n");
    return 0;
}
```
Sample output
![Sample output of Experiment 7](images/ex07_output.png)
---
Experiment 8: FCFS and SJF Scheduling
Part A: FCFS scheduling
```c
#include <stdio.h>
int main() {
    int n, bt[10], wb = 0, tat, sum_wb = 0, sum_tat = 0, i;
    printf("VMs? ");
    scanf("%d", &n);
    printf("Burst times:\n");
    for (i = 0; i < n; i++)
        scanf("%d", &bt[i]);
    printf("\nVM\tBurst\tWaiting\tTurnaround\n");
    for (i = 0; i < n; i++) {
        tat = wb + bt[i];
        printf("VM%d\t%d\t%d\t%d\n", i + 1, bt[i], wb, tat);
        sum_wb += wb;
        sum_tat += tat;
        wb = tat;
    }
    printf("\nAvg wait: %.2f\nAvg Turnaround: %.2f\n", (float)sum_wb / n, (float)sum_tat / n);
    return 0;
}
```
Part B: SJF scheduling
```c
#include <stdio.h>
int main() {
    int n, bt[10], id[10], t, wt = 0, sw = 0, sta = 0, i, j;
    scanf("%d", &n);
    for (i = 0; i < n; i++) {
        scanf("%d", &bt[i]);
        id[i] = i + 1;
    }
    for (i = 0; i < n - 1; i++)
        for (j = i + 1; j < n; j++)
            if (bt[j] < bt[i]) {
                t = bt[i]; bt[i] = bt[j]; bt[j] = t;
                t = id[i]; id[i] = id[j]; id[j] = t;
            }
    printf("VM\tBurst\tWait\tTAT\n");
    for (i = 0; i < n; i++) {
        int tat = wt + bt[i];
        printf("VM%d\t%d\t%d\t%d\n", id[i], bt[i], wt, tat);
        sw += wt;
        sta += tat;
        wt = tat;
    }
    printf("Avg wait: %.2f Avg TAT: %.2f\n", (float)sw / n, (float)sta / n);
    return 0;
}
```
Sample output
![Sample output of Experiment 8](images/ex08_output.png)
---
Experiment 9: Priority and Round Robin Scheduling
Part A: Priority scheduling (car airbag system)
```c
#include <stdio.h>
int main() {
    char *task[] = {"Music", "Navigation", "Speed", "Collision", "Airbag"};
    int pr[] = {5, 4, 3, 1, 0};
    int i, j, t;
    char *s;
    for (i = 0; i < 5; i++)
        for (j = i + 1; j < 5; j++)
            if (pr[i] > pr[j]) {
                t = pr[i]; pr[i] = pr[j]; pr[j] = t;
                s = task[i]; task[i] = task[j]; task[j] = s;
            }
    for (i = 0; i < 5; i++)
        printf("Running: %s\n", task[i]);
    return 0;
}
```
Part B: Round robin scheduling
```c
#include <stdio.h>
int main() {
    char *task[] = {"Speed", "Navigation", "Music", "Display"};
    int time[] = {2, 1, 3, 2};
    int n = 4, q = 1, i, done;
    do {
        done = 1;
        for (i = 0; i < n; i++) {
            if (time[i] > 0) {
                done = 0;
                printf("Running: %s\n", task[i]);
                time[i] = time[i] > q ? time[i] - q : 0;
            }
        }
    } while (!done);
    return 0;
}
```
Sample output
![Sample output of Experiment 9](images/ex09_output.png)
---
Experiment 10: FIFO Page Replacement Algorithm
Supermarket inventory system
```c
#include <stdio.h>
int main() {
    char *inv[] = {"Milk", "Bread", "Rice"};
    int i, n = 3;
    printf("Supermarket Inventory Queue (FIFO)\n");
    for (i = 0; i < n; i++)
        printf("Added: %s\n", inv[i]);
    printf("Removed first: %s\n", inv[0]);
    for (i = 1; i < n; i++)
        printf("Next: %s\n", inv[i]);
    printf("New item comes at the end\n");
    return 0;
}
```
Sample output
![Sample output of Experiment 10](images/ex10_output.png)
---
Experiment 11: LRU and LFU Page Replacement Algorithm
Part A: LRU page replacement
```c
#include <stdio.h>
int main() {
    int pages[] = {1, 2, 3, 2, 4, 1, 3};
    int frames[3] = {0}, time[8] = {0}, faults = 0;
    int np = sizeof(pages) / sizeof(pages[0]);
    for (int i = 0; i < np; i++) {
        int hit = -1, lru = 0;
        for (int j = 0; j < 3; j++)
            if (frames[j] == pages[i])
                hit = j;
        if (hit != -1) {
            time[hit] = i + 1;
        } else {
            for (int j = 1; j < 3; j++)
                if (time[j] < time[lru])
                    lru = j;
            frames[lru] = pages[i];
            time[lru] = i + 1;
            faults++;
        }
    }
    printf("Total page faults: %d\n", faults);
    return 0;
}
```
Part B: LFU page replacement
```c
#include <stdio.h>
int main() {
    int req[] = {101, 101, 102, 103, 104};
    int cache[3] = {0}, freq[3] = {0}, faults = 0;
    printf("Query | cache[ID(freq)] | status\n");
    for (int i = 0; i < 5; i++) {
        int hit = -1, lfu = 0;
        for (int j = 0; j < 3; j++)
            if (cache[j] == req[i])
                hit = j;
        if (hit != -1) {
            freq[hit]++;
        } else {
            for (int j = 1; j < 3; j++)
                if (freq[j] < freq[lfu])
                    lfu = j;
            cache[lfu] = req[i];
            freq[lfu] = 1;
            faults++;
        }
        printf("%3d | [%d(%d)] [%d(%d)] [%d(%d)] | %s\n", req[i],
               cache[0], freq[0], cache[1], freq[1], cache[2], freq[2],
               hit != -1 ? "Hit" : "Miss");
    }
    printf("Total Cache Misses: %d\n", faults);
    return 0;
}
```
Sample output
![Sample output of Experiment 11](images/ex11_output.png)
---
Experiment 12: Best Fit and Worst Fit Memory Management Policies
Part A: Best fit
```c
#include <stdio.h>
int main() {
    int blocks[] = {100, 500, 200, 300, 600};
    int requests[] = {212, 110, 420};
    int n = 3, m = 5;
    for (int i = 0; i < n; i++) {
        int best = -1;
        for (int j = 0; j < m; j++)
            if (blocks[j] >= requests[i] && (best == -1 || blocks[j] < blocks[best]))
                best = j;
        if (best != -1) {
            blocks[best] -= requests[i];
            printf("Request %d KB -> Allocated to block %d | Block sizes: %d %d %d %d %d\n",
                   requests[i], best, blocks[0], blocks[1], blocks[2], blocks[3], blocks[4]);
        } else {
            printf("Request %d KB -> Allocation failed\n", requests[i]);
        }
    }
    return 0;
}
```
Part B: Worst fit (GPU / graphics rendering)
```c
#include <stdio.h>
int main() {
    int blocks[] = {128, 512, 256, 1024};
    int requests[] = {200, 400};
    int n_req = 2, m = 4;
    for (int i = 0; i < n_req; i++) {
        int worst = -1;
        for (int j = 0; j < m; j++)
            if (blocks[j] >= requests[i] && (worst == -1 || blocks[j] > blocks[worst]))
                worst = j;
        if (worst != -1) {
            blocks[worst] -= requests[i];
            printf("VRAM Request %d MB -> Allocated to Block %d (%d MB left)\n",
                   requests[i], worst, blocks[worst]);
        } else {
            printf("VRAM Request %d MB -> Allocation Failed\n", requests[i]);
        }
    }
    return 0;
}
```
Sample output
![Sample output of Experiment 12](images/ex12_output.png)
---
Experiment 13: Disk Scheduling Algorithm
CCTV and surveillance records
```c
#include <stdio.h>
int main() {
    int a[] = {82, 170, 43, 140, 24, 16, 190};
    int n = 7, h = 50, i, j, t, total = 0;
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - i - 1; j++)
            if (a[j] > a[j + 1]) {
                t = a[j]; a[j] = a[j + 1]; a[j + 1] = t;
            }
    printf("CCTV request Served: ");
    for (i = 0; i < n; i++)
        if (a[i] >= h) {
            printf("%d ", a[i]);
            total += a[i] - h;
            h = a[i];
        }
    total += 199 - h + 199;
    h = 0;
    printf("199 0 ");
    for (i = 0; i < n; i++)
        if (a[i] < 50) {
            printf("%d ", a[i]);
            total += a[i] - h;
            h = a[i];
        }
    printf("\nTotal head movement = %d tracks\n", total);
    return 0;
}
```
Sample output
![Sample output of Experiment 13](images/ex13_output.png)
---
Experiment 14: Sequential and Indexed File Allocation
Part A: Sequential file allocation
```c
#include <stdio.h>
int main() {
    int start, len, i;
    printf("Enter start block and length: ");
    scanf("%d %d", &start, &len);
    printf("Allocated blocks: ");
    for (i = 0; i < len; i++)
        printf("%d ", start + i);
    printf("\n");
    return 0;
}
```
Part B: Indexed file allocation
```c
#include <stdio.h>
int main() {
    int ind, n, i, b[10];
    printf("Enter index block and count: ");
    scanf("%d %d", &ind, &n);
    printf("Enter %d block numbers: ", n);
    for (i = 0; i < n; i++)
        scanf("%d", &b[i]);
    printf("Index %d -> Blocks: ", ind);
    for (i = 0; i < n; i++)
        printf("%d ", b[i]);
    printf("\n");
    return 0;
}
```
Sample output
![Sample output of Experiment 14](images/ex14_output.png)
---
Experiment 15: File Organization Schemes for Single and Two Level Directory
Part A: Single-level directory (digital camera storage)
```c
#include <stdio.h>
#include <string.h>
int main() {
    char dir[5][20];
    int n, i;
    printf("Enter number of photos: ");
    scanf("%d", &n);
    printf("Enter %d photo names:\n", n);
    for (i = 0; i < n; i++)
        scanf("%s", dir[i]);
    printf("\nSD Card Directory (single-level):\n");
    for (i = 0; i < n; i++)
        printf("- %s\n", dir[i]);
    return 0;
}
```
Part B: Two-level directory (lab computers)
```c
#include <stdio.h>
#include <string.h>
int main() {
    char u[2][10] = {"Alice", "Bob"};
    char f[2][2][10] = {{"lab1.c", "notes"}, {"prog.c", "data"}};
    char name[10];
    int i, j;
    printf("Enter user (Alice/Bob): ");
    scanf("%s", name);
    for (i = 0; i < 2; i++)
        if (strcmp(name, u[i]) == 0)
            for (j = 0; j < 2; j++)
                printf("- %s/%s\n", u[i], f[i][j]);
    return 0;
}
```
Sample output
![Sample output of Experiment 15](images/ex15_output.png)

