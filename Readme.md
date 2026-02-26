# Parallel File Encryptor/Decryptor in C++

A command-line file encryption tool built in C++ that demonstrates three parallel processing architectures: sequential threading, multiprocessing, and (in progress) a hybrid model. Built as an applied systems programming project to explore OS-level concurrency primitives.
Had originally intended on following [Lovepreet Singh's](https://www.youtube.com/@SinghDevHub) file encryptor project videos, but eventually decided that it would be better if I tried to work on it myself through trial and error and learning on the go. So, used the videos only as reference.

## Features

- XOR-based file encryption/decryption
- Three execution modes: threaded, multiprocess, hybrid (coming)
- Benchmarking built-in to compare approaches
- Processes entire directories recursively
- Encryption key stored in `.env` file

## Usage

```bash
./encrypt <directory> <ENCRYPT|DECRYPT> <num_workers> <threaded|multiprocess>
```

**Examples:**
```bash
./encrypt ./files ENCRYPT 8 threaded
./encrypt ./files DECRYPT 4 multiprocess
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
`std::queue` uses internal heap pointers, which is valid for all threads in mutlithreaded approach as they all  see the same heap, but meaningless across process boundaries. A fixed-size array with head/tail indices is entirely self-contained within the shared memory region.

## Benchmarks (1000 x 1KB files, 6-core & 12 logical processors machine)

| Workers | Threaded | Multiprocess |
|---------|----------|--------------|
| 2       | ~105ms   | ~114ms       |
| 4       | ~65ms    | ~64ms        |
| 8       | ~50ms    | ~53ms        |
| 10      | ~50ms    | ~43ms        |

**Key findings:**
- Both approaches scale similarly up to ~4 workers
- Multiprocess pulls ahead at higher counts due to CPU cache isolation per process
- Performance flatlines near core count (12) - context switching overhead dominates beyond this
- Process creation overhead makes multiprocess slower at low worker counts

## Project Structure

```
src/
├── main.cpp               # Entry point, mode selection, benchmarking
├── Task.hpp               # Task struct (file path + action)
├── FileReader.hpp/.cpp    # File reading
├── FileWriter.hpp/.cpp    # File writing
├── Cryptor.hpp/.cpp       # XOR encryption/decryption
├── ProcessManager.hpp/.cpp    # Thread pool implementation
├── SharedQueue.hpp/.cpp       # Shared memory queue + MultiProcessManager
├── envReader.hpp/.cpp     # .env file parser
├── Makefile
└── .env                   # ENCRYPTION_KEY=<char>
```

## Build

```bash
cd src
make
```

Requires g++ with C++17 support and pthreads.

## Concepts Demonstrated

- `std::thread`, `std::mutex`, `std::condition_variable`, `std::unique_lock`
- `fork()`, `waitpid()`, `exit()`
- `mmap` for anonymous shared memory
- POSIX semaphores and process-shared mutexes
- Circular buffer data structure
- Producer-consumer synchronization pattern
- Empirical performance benchmarking