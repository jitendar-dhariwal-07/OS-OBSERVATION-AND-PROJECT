# Multithreading-single-Multiprocessor-OS

Multithreading is when a program is split into multiple threads so it can do more than one task “at once.” The key difference is whether those threads are truly running simultaneously or just sharing CPU time by switching quickly.

Single processor multithreading
On a single-processor system, only one thread can execute at any instant, because there is only one CPU to do the work. The operating system uses context switching to alternate between threads, which creates the illusion of parallel work; this is concurrency, not true simultaneous execution.

Multi processor multithreading
On a multiprocessor system, multiple threads can run at the same time on different processors or cores. That means the program can get parallel execution, which is real simultaneous work and usually better performance for CPU-heavy tasks.

Simple example
Imagine 3 people waiting for 1 cashier versus 3 cashiers:

Single processor multithreading: 1 cashier serves people one by one, but very quickly switches between them.

Multi processor multithreading: 3 cashiers serve 3 people at the same time.

USE CASE : FACTORY QUALITY CHECK 
