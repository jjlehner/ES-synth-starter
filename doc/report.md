<style>body {text-align: justify}</style>
# JBMN Systems Music Synthesiser

A music synthesiser created by JBMN Systems, consisting of

| Name | Shortcode|
| --- | ---- |
|Jonah Lehner  | jjl119 |
|Bertil de Germay de Cirfontaine | bd519 |
|Martin Lindström | ml3621 |
|Neel Dugar | nd419 |

# Important Notes

- Please View Markdown inside VS Code to see equation rendering

# Contents
In this file:
- Threading
- Interrupts
- Thread-Safe Data Exchange
    - Generic ThreadSafeList
    - Inter-Task Blocking
- Execution Times & Initiation Intervals
- CPU Utilisation
- Advanced Features

Library Documentation In Other Files:
- [ThreadSafeList\<T>](ThreadSafeList.md) - details how safe concurrent data access is achieved.
- [Tasks](Tasks.md) - details the tasks and how they are implemented
- [CAN Frame](CANFrame.md) 
- [Knob](Knob.md)
- [SoundGenerator](SoundGenerator.md)
- [Clamped Counter](ClampedCounter.md)
- [Phase Accumulator Pool](PhaseAccPool.md)


# Knob Roles From Left to Right
 | Knob 0 | Knob 1 | Knob 2 | Knob 3 |
 |---|---|---|---|
| Not Used | Type of Wave Produced | Octave | Volume |

# Application  of  Real-Time  Programming Techniques
The tasks of the music synthesiser must appear to operate in parallel which can be achieved through threading and interrupts. FreeRTOS provides macros for managing threads using a task scheduler and a hardware abstraction layer allows for configuring interrupts using hardware timers.

## Threading
In our implementation, the threaded tasks are defined in the Tasks library with each task being started within the setup of the main thread to improve maintainability of the codebase. Each task is initiated with a predefined and tuned stack size by use of the FreeRTOS macro `uxTaskGetStackHighWaterMark`, which helped determine the maximum memory used throughout runtime. This software uses three threads: `scanKeysTask`,`displayUpdateTask`,`decodeTask` and `transmitTask`. The defined functionality of each thread can be found in [Tasks.md](Tasks.md)

| Task | Stack Size /32 bit words |
|--|--|
|scanKeys| 128|
|displayUpdate| 256 |
|decodeTask| 64|
|transmit Task| 64|


The `scanKeysTask` reads the values directly from the input "keyMatrix" and produces a `std::bitset` with the values flat-packed. From this bitset, the device can register a key change event and advertise this key event on the CAN bus. Each device also internally registers a key event by using a ThreadSafeList data structure called ```notesPressed```. As detailed in [ThreadSafeList.md](ThreadSafeList.md), this structure makes use of critical sections to enable safe concurrent access. This task also updates values stored in the global Knob objects representing the rotation of each knob.

The `displayUpdateTask` makes use of the ThreadSafeList data structure to read the first note pressed and display it on the u8g2 display. The thread also performs concurrent reads of the global knob objects when rendering the volume, octave and current wave mode. When measuring the execution times, improving performance per iteration of this task was not easy to improve as the largest bottleneck is the `sendBuffer` method which is expected due to the large memory transfers.

The `decodeTask` extracts messages from the ```msgInQueue``` detailing key events on external keyboards. This thread then registers these keys events by pushing/removing data from the ```notesPressed``` ThreadSafeList - given a KeyPressed event, the thread pushes data to the list and given a KeyReleased event, removes elements from the list. Messages filling the ```msgInQueue``` arrive through the CAN bus, triggering an interrupt which fills a thread-safe queue.

The `transmitTask` extracts messages from the `msgOutQueue` and places them into the CANFrame mailbox. As the mailbox only has space for three messages, execution of the loop is guarded by a counting semaphore. This ensures no messages are lost due to lack of space in the output mailbox. The counting semaphore is decremented by the `CAN_TX_ISR` which is triggered when the output mailbox is emptied.

## Interrupts
The music synthesiser also makes use of hardware interrupts for fast hardware parallelism. The interrupts are triggered by events in order to handle both the messaging on the CAN bus and the speaker control using the `SampleISR`, which is an interrupt service routine that is triggered at a rate of 22kHz.

## Thread-Safe Data Exchange

### Generic ThreadSafeList\<T>
The global variable `notesPressed` is a list containing the notes currently being pressed on all connected keyboards. Since this list is used by multiple tasks, it was decided to wrap this list in a generic ThreadSafeList type. Further information on how the member functions guarantee safe access and synchronisation can be found in the [ListSafeList\<T> documentation](ThreadSafeList.md)


### Inter-Task Blocking

The `ThreadSafeList` `notesPressed` is shared between the `SampleISR` interrupt and three tasks: `ScanKeys` Task, `UpdateDisplay` Task and the `Decode` Task. The `ThreadSafeList` prevents both race conditions and deadlock by wrapping list mutations in critical section blocks. Using mutexes in this data structure would prevent use of the type by both interrupts and threads, as an interrupt could not yield to a thread holding a lock on the underlying list type, causing deadlock. Critical sections overcome the potential deadlock of mutexes by disabling interrupts and context switches for short sections of code. In essence, this makes executions of critical sections atomic. By calling ```taskENTER_CRITICAL()```, FreeRTOS guarantees a critical section will execute sequentially until the critical section is exited by calling ```taskEXIT_CRITICAL()```. 

# Execution Times, Initiation Intervals & CPU Utilisation
In this section, we will outline and analyse both the ET and II of each thread. The results will then be used in order to perform a critical instant analysis of the rate monotonic scheduler, showing that all deadlines are met under worst-case conditions as well as computing the CPU utilisation.

Execution Time (ET) is defined as the time spent using processing resources (the CPU) by a task (block of code) in a thread. Initiation Interval (II) is the time between sequential initiations of a task.

Execution time of each task was measured by running the synthesiser in a **profiling mode** and calling micros() before and after 32 iterations of a task. The mean execution time of each iteration was then calculated. 

The task initiation intervals were configured explicitly in code with the following values:
- Sample ISR: The specifications say that sound should be emitted with a frequency of 22 kHz, which translates to an II of around 0.045 milliseconds (referring to: Period = 1/frequency = 1/22 kHz).
- CAN TX ISR: The instructions (page 10 / Lab 2) state that the transmit task must trigger in the worst case at a 1.67 ms interval. Since this task is extremely lightweight (takes around 3 microseconds to complete), its load is very small.
- Scan Keys Task: Design choice to use a 20 millisecond polling time. This helps getting better feedback from the knobs and the keys, while putting a comparatively small load on the system's resources.
- Message Decode Task: 25.5 milliseconds, see analysis in the lab notes (page 10 / Lab 2).
- Message Transmit Task: 60 milliseconds, see analysis in the lab notes (page 10 / Lab 2).
- Display Update Task: The specification (point 5) state that the display and the LD3 LED on the MCU need to be updated every 100 milliseconds.

The following is an analysis for a sawtooth wave where a maximum of 4 keys can be pressed at one time. It is possible to allow 5 keys to be pressed at one time, but this lowers the margin to around that of the sine wave at one pressed key. The bottom right cell of the table below reveals a CPU utilisation of around 92%.

| Task | Priority | Initiation Interval &tau;<sub>i</sub> [ms] | Execution Time T<sub>i</sub> [ms]| &lceil;<sup>&tau;<sub>n</sub></sup>&#8260;<sub>&tau;<sub>i</sub></sub>&rceil; T<sub>i</sub> [ms]| <sup>T<sub>i</sub></sup>&#8260;<sub>&tau;<sub>i</sub></sub> [%]|
|-------------------|-----|-----------|---------|---------|--------|
Sample ISR Sawtooth | ISR | 0.04545.. | 0.034   | 74.83   | 74.80  | 
CAN TX ISR          | ISR | 1.67      | 0.003   | 0.18    | 0.18   | 
Scan Keys           | 3   | 20        | 0.104   | 0.52    | 0.52   |
Message Decode      | 2   | 25.2      | 0.007   | 0.028   | 0.03   | 
Message Transmit    | 2   | 60        | 0.011   | 0.022   | 0.02   |  
Display Update      | 1   | 100       | 16.667  | 16.67   | 16.67  | 
Total               |     |           |         | 92.25   | 92.21  |

The following is an analysis for a sine wave where a maximum of 1 key can be pressed at one time. The sine wave implementation is more computationally demanding than the sawtooth wave, and thus our implementation can handle fewer pressed keys. The bottom right cell of the table below reveals a CPU utilisation of around 97%.

| Task | Priority | Initiation Interval &tau;<sub>i</sub> [ms] | Execution Time T<sub>i</sub> [ms]| &lceil;<sup>&tau;<sub>n</sub></sup>&#8260;<sub>&tau;<sub>i</sub></sub>&rceil; T<sub>i</sub> [ms]| <sup>T<sub>i</sub></sup>&#8260;<sub>&tau;<sub>i</sub></sub> [%]|
|-------------------|-----|-----------|---------|---------|--------|
Sample ISR Sine     | ISR | 0.04545.. | 0.036   | 79.24   | 74.20  | 
CAN TX ISR          | ISR | 1.67      | 0.003   | 0.18    | 0.18   | 
Scan Keys           | 3   | 20        | 0.104   | 0.52    | 0.52   | 
Message Decode      | 2   | 25.2      | 0.007   | 0.028   | 0.03   | 
Message Transmit    | 2   | 60        | 0.011   | 0.022   | 0.02   | 
Display Update      | 1   | 100       | 16.667  | 16.67   | 16.67  | µ
Total               |     |           |         | 96.65   | 96.61  |


We can observe from the above tables that the latency L<sub>n</sub> (calculated applying [*] to column five) is always smaller than the initiation time (&tau;<sub>n</sub> = 100ms) of the lowest priority task (Display Update). This critical instant analysis shows that our scheduling works.

<center>
L<sub>n</sub> = &sum;<sub>i</sub> &lceil;<sup>&tau;<sub>n</sub></sup>&#8260;<sub>&tau;<sub>i</sub></sub>&rceil; T<sub>i</sub> &le; &tau;<sub>n</sub> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; [*]
</center> 


# Advanced Features and Team Work
Two advanced features: variable waveform type and polyphony have been implemented by separate members of the team. As with previous sections, this code is designed to be readable and maintainable. The code is structured to use an object-oriented paradigm, favouring readability over relatively insignificant trade-offs in execution speed. When working independently, team members worked in their own separate branches and created pull requests before merging their changes into the master branch. 

The codebase is also written to be extendable, one such example of this is the `SoundGenerator` class. This class provides a single interface for sound generation, regardless if: multiple keyboards are connected, how many keys are pressed, which waveform is selected, and at which octave the sound should be played. It would be simple, for example, to add another possible waveform. 

Those advanced features enhance the system's functionality by providing additional methods to produce music. Please note that the speakers of the music synthesiser are poor, and thus do not adapt well to sine waves, the sound can, however, be perceived better using headphones. 

If we had more time, and the hardware had more resources (see the critical instant analysis above), we would have implemented the following functionalities:
- Additional waveforms. The modularity of the system means that if we have a high resolution waveform in memory (see example for a sine wave below), the system can easily be modified to utilise this functionality.
- Support for pressing more keys at the same time. At the moment, this is restricted in software to conserve system resources, but this requires removing one line of code to reverse. 
- Recording and replay functionalities. By recording the duration which keys were pressed, their sound can be saved at a relatively small memory cost. Replayed notes can then use the existing API for replaying the sounds. 

## Polyphony

To perform polyphony, the synthesiser must associate a phase accumulator with each key that is being pressed. Voltage output to the speaker is then calculated by summing each phase accumulator to superimpose the waveforms of all the notes being pressed. To save memory and remove processing time associated with allocating memory, 36 phase accumulators are declared in a static array at the beginning of the program. When a key is pressed, the PhaseAccumulatorPool helper class dynamically assigns the new key with one entry in the accumulator array. Once the same key is released, the accumulator pool frees the entry in the array so that another note can be assigned to it. See here for more detailed documentation of the [PhaseAccumulator Pool](PhaseAccPool.md). 
 
## Sine Wave/Saw Tooth  
The user can choose in real-time to either play with a sawtooth wave or a sine wave through knob 1 (the second knob from the right). A sine wave with K values over a period is given by g[n] = sin(2&pi;<sup>1</sup>&#8260;<sub>K</sub>n) for n&isin; {1, 2, ..., K}. For a large K, g[n] is high resolution and a good approximation of a continuous sine. A sine wave sin(2&pi; f t) with frequency f at time t can then be extracted from g by setting n = K f t modulo K. We can exploit the periodicity the sine, i.e. g[n+m] = g[n] if m is a multiple of K, to generate an infinite sine wave from a single period. In our case, g was generated with the following code (and scaled to  maximum value 128=256/2):
```python
import numpy as np
K = 1024
ts = np.arange(0,1,1/K)
wave = np.zeros_like(ts)
for i in range(ts.shape[0]):
    wave[i] = 256/2*np.sin(2*np.pi*ts[i])
wave = wave.astype(np.int32)
np.savetxt("sine_wave", wave[None,:], delimiter=',', fmt='%i', newline="")
```
Note that since sin(x) = -sin(x+&pi;), the memory requirement can be halved at the cost of a larger computational load. In fact, since computational resources in the ISR were scarce, we decided to store a full period in memory to save on some computations. Moreover, note that we can provide a better sine approximation by increasing K, but it was found experimentally that values above 250 provided little improvement - the speaker was not good enough to determine that one waveform was better than another.

Moreover, the time variable can be acquired through the `micros()` function, which is built-in to the Arduino. The documentation for this function reveals that it overflows after around 70 minutes, however it is unlikely that a human would depress a single key continuously for more than a few seconds at a time, allowing us to discard this limitation.
