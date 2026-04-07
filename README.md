# Deadlock Detection

[GitHub](https://github.com/byebyelabs/deadlock-detection)

### Algorithm

There are four conditions that must all be met for deadlock to occur:

- Mutual Exclusion: the program uses locks
- No Preemption: locks cannot be taken from a thread that holds them
- Hold and Wait: threads can hold one lock while they wait for another
- Circular Wait: a cycle of threads holding a lock while waiting for the next thread in the cycle

The approach taken in this project is to force threads to acquire locks in a consistent order to prevent the last situation.

If a thread T1 acquires lock A and then lock B and then lock C, any thread that acquires any combination of those locks must acquire them in the same order. A thread may acquire lock A and then lock C, but if it tries to then acquire lock B, there is a chance for a deadlock to happen.

### Implementation

#### Lock Graph

We maintain a global graph in [src/detector.c](src/detector.c) using a linked list of nodes (`node_t`). Each node includes `lock_number` that stores the lock identifier (pointer to the lock) and stores a NULL-terminated array of locks that must not be acquired while holding that lock, `avoid_lock_numbers`.

For each thread, a thread-local array tracks the currently held locks (`TRD_LCL_CURR_HELD_LOCKS`). Before a thread attempts to take a lock `m`, `verify_no_deadlock()` checks each currently held lock and checks whether `m` appears in that lock's `avoid_lock_numbers`. If it does, the ordering rule would be violated, meaning the library prints a backtrace and exits.

After a lock is successfully acquired, we update the graph by unioning the current thread's held locks into the new lock's `avoid_lock_numbers`. This is to preserve the invariant: once a lock is seen after another, it must always be acquired after it in the future. We use a global lock to protect the shared graph while checking and updating.

#### Backtracing

#### Library Interposition

We override `pthread_mutex_lock` and `pthread_mutex_unlock` in [src/locks.c](src/locks.c). The wrappers call `before_lock()`/`after_lock()` around the real mutex calls, and the real functions are called through [src/rlocks.c](src/rlocks.c). This is loaded with `LD_PRELOAD`, so any previously compiled code can work with this library without needed modications. We have a similar implementation for unlocks.

### How to run

1. Clone the repository

   ```[bash]
   git clone https://github.com/byebyelabs/deadlock-detection.git
   ```

2. Navigate to the project directory

   ```[bash]
   cd deadlock-detection
   ```

3. Compile with `clang`

   ```[bash]
   make all
   ```

4. Run the bank test (has a deadlock possibility, should raise an error)

   ```[bash]
   cd bank_test && make && cd ..
   make test/bank
   ```

5. To run it on your own compiled code (`main`), run it as such:

   ```[bash]
   LD_PRELOAD=./build/byebye_deadlocks.so ./main
   ```

### Citations

- [1] Project Structure Inspiration: https://www.reddit.com/r/C_Programming/comments/qim1zv/comment/hikad1a/
- [2] Algorithm Description: https://curtsinger.cs.grinnell.edu/teaching/2026S/CSC313/labs/deadlock/
