# Tasks

The synthesiser uses six distinct tasks that are performed with either an interrupt or a thread.

## Threads 
### Scan Keys - Priority 3
- This thread goes through the keyboard matrix and determines which keys have been pressed. The latter are stored locally in a global keysPressed ThreadSafeList type. This thread is also responsible for sending announcements of when a key is pressed and released to the message output queue.
### Message Decode - Priority 2
- Thread to process messages on the queue. It is initialised by the availability of data on the incoming message queue. The call to ```xQueueReceive(msgInQ, RX_Message, portMAX_DELAY)``` will block and yield the thread to other tasks until a message is available in the queue.
### Message Transmit - Priority 2
- This thread extracts messages from the message output queue and places them in the output mailboxes. As only three slots are available in the outgoing mailboxes, a semaphore guards access to this resource, preventing this thread from placing messages in the mailbox when they are full.
### Display Update - Priority 1
- Thread, triggered every 100 ms, responsible for updating the OLED display and toggling the LD3 LED on the MCU module. The display dynamically shows the following:
    - The volume, varies from 0 to 16.
    - The octave of the notes being played.
    - Which note (if any) is being pressed. If multiple keys are pressed, only the one pressed for the longest time will be displayed.
    - The selected waveform, either a sine wave or a sawtooth wave.

## Interrupts
### Sample ISR
- Interrupt with 22 kHz frequency that plays the sound of the note(s) currently pressed. The user can choose either to output a sine wave or a sawtooth wave by turning knob 1 (second starting from the right).
### CAN TX ISR
- Interrupt responsible for giving a mutex to indicate whether the transmit queue is full or not. 
