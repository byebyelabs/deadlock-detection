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

### Citations

[1] Project Structure Inspiration: https://www.reddit.com/r/C_Programming/comments/qim1zv/comment/hikad1a/
[2] Algorithm Description: https://curtsinger.cs.grinnell.edu/teaching/2026S/CSC313/labs/deadlock/
