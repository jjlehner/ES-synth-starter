# [SoundGenerator](../lib/SoundGenerator/SoundGenerator.hpp)

This class implements the sound generation: to translate the time the key was pressed, to the voltage being sent to the speaker. This happens with a frequency of 22 kHz, so the implementation needs to be lightweight. The API is found in `SoundGenerator::getSound()`.

The general workflow in this class is as follows:
- Read the list of notes currently being pressed.
- Read knob 1 (second from the right) to get the waveform to generate.
- For each note pressed (up to the limit for the waveform, see discussion in [main report](report.md/#Advanced-Features)), generate the voltage level for that note. Sum together the waves for the total voltage. 
    - Get the step size or the frequency of the note being pressed.
    - Compensate for the octave given by knob 2 (third from the right). 
- Return the voltage V<sub>out</sub> &isin; [-128,128] (the main method writes 0 &le; V<sub>out</sub> + 128 &le; 256 to the speaker).

# Functions
```cpp
class SoundGenerator{
    private:
        int32_t shiftOctave(int32_t stepSize, int32_t octave); //Shifts the freqency stepSize by octave
        int32_t sawtooth(Note note); //Returns the difference in phase accumulation of a sawtooth wave.
        int32_t sine(Note note); //Returns the Vout for a sine wave, given from a lookup table.
    public:
        SoundGenerator(); //Default constructor.
        int32_t getSound(); //Main API, see above for explanation.
}
```