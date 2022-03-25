# [Phase Accumulation Pool](../lib/PhaseAccPool/PhaseAccPool.hpp)

A helper class which wraps 36 static phase accumulators required for polyphony. The class provides the function ```aquirePhaseAcc``` for the scanKeysTask and DecodeTask to dynamically associate pressed notes with entries in the accumulator array. The class also provides the function ```releasePhaseAcc``` to disassociate notes with entires in the accumulator array. The function ```setPhaseAcc``` and ```getPhaseAcc``` are used in the Sample interrupt and SoundGenerator class to generate the voltage output for the speaker. All these functions can be used safely concurrently as they wrap modifications of shared data in critical sections.

# Functions

```C++ 
static size_t aquirePhaseAcc()
```
- Associate a phase accumulator from the array with a note. Locks that index, preventing other notes from associating with it. The new index locked is returned. 

```C++
static void releasePhaseAcc(const size_t &index);
```
- Releases a phase accumulator given an index. 
```C++
static int32_t getPhaseAcc(const size_t& index);
```

- Returns the value of an associated phase accumulator.
```C++
static void setPhaseAcc(const size_t& index, const int32_t & val)
```
- Sets the value of an associated phase accumulator