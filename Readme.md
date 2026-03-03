# Parallel File Encryptor/Decryptor in C++

A command-line file encryption tool built in C++ that demonstrates four parallel processing architectures: multithreading, multiprocessing, a hybrid model, and async I/O using `io_uring`. Built as an applied systems programming project to explore OS-level concurrency primitives and identify real performance bottlenecks empirically.

Had originally intended on following [Lovepreet Singh's](https://www.youtube.com/@SinghDevHub) file encryptor project videos, but eventually decided that it would be better to work on it through trial and error and learning on the go. The videos were used only as reference.

## Features

- XOR-based file encryption/decryption
- Four execution modes: threaded, multiprocess, hybrid, async
- Benchmarking built-in to compare approaches
- Processes entire directories recursively
- Encryption key stored in `.env` file

## Usage

```bash
# Threaded
./encrypt <directory> <ENCRYPT|DECRYPT> <num_threads> threaded

# Multiprocess
./encrypt <directory> <ENCRYPT|DECRYPT> <num_processes> multiprocess

# Hybrid
./encrypt <directory> <ENCRYPT|DECRYPT> <num_processes> <threads_per_process> hybrid

# Async IO
./encrypt <directory> <ENCRYPT|DECRYPT> <queue_depth> async
```

**Examples:**
```bash
./encrypt ./files ENCRYPT 8 threaded
./encrypt ./files DECRYPT 12 multiprocess
./encrypt ./files ENCRYPT 4 4 hybrid
./encrypt ./files DECRYPT 32 async
```

## Architecture

### Threaded Mode
- Persistent thread pool using `std::thread`
- Thread-safe task queue protected by `std::mutex`
- Workers block on `std::condition_variable` when queue is empty
- Graceful shutdown via stop flag + `notify_all()`

### Multiprocess Mode
- Worker processes spawned via `fork()`
- Shared memory task queue using `mmap` with fixed-size circular buffer
- Cross-process synchronization using `pthread_mutex_t` with `PTHREAD_PROCESS_SHARED`
- POSIX semaphores (`sem_t`) for producer-consumer coordination
- Parent waits for children via `waitpid()`

### Why a circular buffer for shared memory?
`std::queue` uses internal heap pointers - valid for threads since they share the same heap, but meaningless across process boundaries. A fixed-size array with head/tail indices is entirely self-contained within the shared memory region.

### Hybrid Mode
- Forks N processes, each spawning M threads internally
- All threads across all processes pull from the same `SharedQueue` in shared memory
- Combines process-level cache isolation with thread-level parallelism within each process

### Async IO Mode (`io_uring`)
- Single thread, no worker pool
- Submits batches of read requests to the kernel via `io_uring` submission queue
- Collects completions asynchronously - never blocks waiting for a single file
- On read completion: XOR transforms buffer in place, submits write via same `IOContext`
- On write completion: cleans up `IOContext`, frees slot for next file
- Keeps up to `queue_depth` operations in flight simultaneously

## Benchmarks (1000 x 1KB files, 6-core / 12 logical processors, WSL2)

### Wall-clock time

| Approach | Config | Best Time |
|----------|--------|-----------|
| Threaded | 2 threads | ~77ms |
| Threaded | 8 threads | ~47ms |
| Threaded | 12 threads | ~47ms |
| Multiprocess | 8 processes | ~40ms |
| Multiprocess | 12 processes | ~26ms |
| Hybrid | 4×4 (16 total) | ~24ms |
| Hybrid | 2×8 (16 total) | ~26ms |
| Async IO | depth 8 | ~42ms |

### Resource efficiency (perf stat comparison)

| Metric | Multiprocess (12) | Async (depth 8) |
|--------|------------------|-----------------|
| CPUs utilized | 9.6 | 1.5 |
| Page faults | 1210 | 155 |
| User time | 0.44s | 0.06s |
| Sys time | 0.94s | 0.15s |
| Total CPU consumed | 1.40s | 0.19s |

Async IO achieves similar wall-clock time with **7x less total CPU consumption** - a critical advantage on shared production servers.

## Key Findings

**Why performance plateaus:** Instrumenting queue vs I/O time showed queue operations take 0–3 microseconds while file I/O takes 200–500 microseconds (300x longer). The disk is the bottleneck, not the parallelism strategy. `perf stat` confirmed threads spend more time in kernel space than user space, with only 1.5 CPUs utilized despite 4 threads.

**Why multiprocess outperforms threading at scale:** Each process has isolated CPU cache. Threads share cache - at high counts, cache line invalidation between threads causes contention. Processes avoid this entirely.

**Why hybrid shows no dramatic improvement:** With I/O dominating at 300x the queue time, adding more parallelism layers doesn't address the actual bottleneck.

**Why async IO matters beyond wall-clock time:** A single thread keeps the disk saturated without spawning worker pools. On a production server where CPU is shared, async IO's 7x lower CPU consumption makes it the practical choice - even when wall-clock times are similar to multiprocessing.

**WSL caveat:** On WSL2, the filesystem is virtualized and the OS page cache absorbs much of the I/O. On native Linux with real disk latency and larger files, async IO would pull further ahead.

## Project Structure

```
src/
├── main.cpp                   # Entry point, mode selection, benchmarking
├── Task.hpp                   # Task struct (file path + action)
├── FileReader.hpp/.cpp        # File reading
├── FileWriter.hpp/.cpp        # File writing
├── Cryptor.hpp/.cpp           # XOR encryption/decryption
├── ProcessManager.hpp/.cpp    # Thread pool implementation
├── SharedQueue.hpp/.cpp       # Shared memory queue + MultiProcessManager
├── HybridManager.hpp          # Hybrid process+thread manager
├── AsyncManager.hpp/.cpp      # io_uring async IO manager
├── envReader.hpp/.cpp         # .env file parser
├── Makefile
└── .env                       # ENCRYPTION_KEY=<char>
```

## Build

```bash
cd src
make
```

Requires g++ with C++17 support, pthreads, and liburing (`sudo apt install liburing-dev`).

## Concepts Demonstrated

- `std::thread`, `std::mutex`, `std::condition_variable`, `std::unique_lock`
- `fork()`, `waitpid()`, `exit()`
- `mmap` for anonymous shared memory
- POSIX semaphores and process-shared mutexes
- Circular buffer data structure
- Producer-consumer synchronization pattern
- `io_uring` submission/completion rings via liburing
- `IOContext` lifecycle management for async operations
- Empirical bottleneck identification via instrumentation and `perf stat`