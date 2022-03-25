# [Knob](../lib/Knob/Knob.hpp)
A class used to abstract the inputs of a knob on the synthesiser with a constructor to bind the lower and upper bounds of the ClampedCounter class. The class has setters and getters for the internal rotation value as is maintained by the clamped counter variable. 

## Functions
```c++
static inline std::pair<bool, bool> getAB(const std::bitset<IO::FLAT_KEY_MATRIX_LENGTH> &keyArray, size_t knobIdx);
```
This function allows for quick and ergonomic access to A and B variables pertaining to a given knobIdx from the keyArray, this is used sequentially in the scanKeysTask to update the rotations each task iteration.

## Member Functions 
```c++
void updateRotation(bool newA, bool newB) volatile;
```
This is a basic setter that attempts to make use of the algorithm to deal with fast spinning of knobs to deal with state transitions of the register values.
```c++
size_t getRotation() volatile;
```
This getter simply retrieves the value as is stored within the atomic variable stored within the ClampedCounter.