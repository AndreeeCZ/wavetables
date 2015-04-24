# wavetables
Bandlimited wavetable generator stolen from the Segment Synthesizer project.

Generates C/C++ header files to be included in your synthesizers.

To compile it, enter the directory and type:
make

Use it like so:
./wavetables <sample rate> <number of tables>

for example, if you want to generate
~50 wavetables evenly spread from ~80Hz to Fs/2, do like so:

./wavetables 48000 50

If you want to make a header file out of this (which you propably do),
stream it to a file like so:

./wavetables 48000 50 > wavetable.h

-----------------------------------------

To use the wavetables, you need to find the nearest table to your frequency.
The frequencies of individual wavetables are stored in BLSawFreqTable[].

Then, find the table size of that table by reading from BLSawSamplesTable[frequencyIndex].

then, play the samples back by reading from wavetable[closestFrequencyIndex][sample].