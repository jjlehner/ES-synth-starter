# [CANFrame](../lib/CANFrame/CANFrame.hpp)

## Note struct
- decision to use struct as opposed to a class was for quick access to member variables
- no encapsulation benefits were deemed necessary here as no internal state propagated

The struct provides an abstraction over CAN messages representing a note by their Note, Octave, timestamp, phaseAccumulator
### Methods
```c++
int32_t getStepSize();
```
Returns the stepsize associated with a given note as computed by its fields
```c++
int32_t getFrequency();
```
Returns the frequency associated with the note.
```c++
void aquirePhaseAcc();
```
Leases from the Phase Accumulator pool.

```c++
bool Note::operator==(const Note &b) 
```
Enables STL `<algorithm>` usage for `std::find` by overriding equality to allow for safe deletion of elements from the ThreadSafeList.

## CANFrame
Class representation of a CAN message frame to allow for more ergonomic access to fields as opposed to a byte array using indices.

### Methods
```c++
bool getKeyPressed();
```
getKeyPressed is a simple getter method to retrieve the whether a message represents a pressed state or a released state.
```c++
uint8_t getOctaveNum();
```
getOctaveNum is also a simple getter to retrieve the octave number.
```c++
uint8_t getNoteNum();
```
getNoteNum is also a simple getter to retrieve the note number.
```c++
void send();
```
send allows a message to be sent to the CANBus using the internal queues
```c++
static void receiveISR();
```
This ISR is namespaced to avoid global namespace pollution, which essentially adds a frame on the interrupt onto the queue.