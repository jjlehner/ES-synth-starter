# [ClampedCounter](../lib/ClampedCounter/ClampedCounter.hpp)
A simple class to create a saturating thread-safe counter with bounds.
# Methods
```c++
void increment() volatile;
```
Atomically saturatingly-increments the internal counter using the clamped values created at construction
```c++
void decrement() volatile;
```
Atomically saturatingly-decrements the internal counter using the clamped values created at construction

```c++
size_t getInternalCounter() volatile;
```
Atomically retrieves the internal counter value