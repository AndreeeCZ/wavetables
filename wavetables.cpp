/*
* Bandlimited saw wave generation, implemented for Segment Synthesizer
* Copyright (C) 2015 Andre Sklenar <andre.sklenar@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* For a full copy of the GNU General Public License see the doc/GPL.txt file.
*/

#include <iostream>
#include <math.h>
#include <iomanip>
#include <cstdlib>


using namespace std;



int main(int argc, char* argv[])
{
    //create a logarithmic table from 0 to 1
    float logTable[1000];
    for (int i=0; i<1000; i++) {
		logTable[i] = expf((logf(1.f)-logf(0.001f))*(i/1000.f)+logf(0.001f));
	}

    //prepare a float[] tables for band limited saw waves @ 48kHz, one octave each
    float numTables = atoi(argv[2]);
    int SRT = atoi(argv[1]);

    // the tables themselves
    float **BLSawTable = new float*[(int)numTables];

    // tables for storing the frequency and samples sizes
    float *BLSawFreqTable = new float[(int)numTables];
    float *BLSawSamplesTable = new float[(int)numTables];

    float freq = 0;
    float tableLength = 0;
    int totalSampleCount = 0;
    int prevTableLength = -1;
    int skippedI = 0;
    int skippedTables = 0;

    cout << "/* Frequency listing for each wavetable:" << endl;
    for (float i=0; i<numTables; i++) {

        // get linear part 0..1 of where the freq should be (minimum 100Hz)
        freq = (i+skippedI)*100/(numTables+skippedTables)/100;
        // convert this to log
        freq = logTable[(int)(freq*1000)];
        // convert this to actual frequency
        freq *= SRT/2;
        freq += 80.f;

        // calculate the length of the table for freq
        tableLength = SRT/freq;

        // round it up for obvious reasons
        tableLength = round(tableLength);

        // recalculate new frequency
        freq = SRT/tableLength;

        // print values
        if (prevTableLength==tableLength) {
            // we already have this and dont need this
            i--;
            skippedI++;
            numTables--;
            skippedTables++;
            continue;
        }
        prevTableLength = tableLength;

        cout << freq << "Hz         ";
        cout << "Sample Count: " << tableLength << endl;

        // allocate individual wavetables
        BLSawTable[(int)i] = new float[(int)tableLength];

        // value at [0] is the frequency at which the table is sampled
        BLSawFreqTable[(int)i] = freq;
        BLSawSamplesTable[(int)i] = tableLength;

        // set all values in the table to 0.f
        for (int j = 0; j<tableLength; j++) {
            totalSampleCount ++;
            BLSawTable[(int)i][j] = 0.f;
        }
    }


	cout << "Sample rate: " << SRT << endl;
    cout << endl << "Total number of floats: " << totalSampleCount << endl << "Memory taken: " << totalSampleCount*sizeof(float) << " bytes. */" << endl << endl;

    int plotOffset = 0;
    int plotOffset2 = 0;

    for (int i=0; i<numTables; i++) {
        // iterate over BLSawTable[i] and generate sine waves at all harmonics until the highest freq<24kHz

        freq = BLSawFreqTable[i];
        //cout << "Creating new waveform, fundamental at " << freq << "Hz" << endl;
        tableLength = BLSawSamplesTable[i];
        float PA = 0.f; // phase accumulator 0 .. 2pi
        float PAInc = 0.f; // increment to the phase accumulator

        freq = BLSawFreqTable[i];
        for (int h = 1; freq<=SRT/2; h++) {
            // iterate over each harmonic to generate a a saw wave
            //float h = 1.f;
            freq = BLSawFreqTable[i]*h;

            PA = 0.f;
            PAInc = 2.f*M_PI/SRT*freq;
            int cycleSize = SRT/freq;

            for (int s = 0; s<tableLength; s++) {
                // generate a sine wave at freq

                float sine = sinf(PA); // the sinwave

                BLSawTable[i][s] += sine*(1.f/h);
                PA += PAInc;
                
            }
        }

        // print all as arrays
        cout << "// fundamental at " << BLSawFreqTable[i] << "Hz, " << (int)BLSawSamplesTable[i] << " samples" << endl;
        cout << "static const float BLSawTable" << i << "[" << (int)tableLength << "] = {";
        for (int s = 0; s<tableLength; s++) {
            plotOffset++;
            float print = BLSawTable[i][s];
            cout << fixed << std::setprecision(10) << print << "f";
            if (s<tableLength-1) {
                cout << ", ";
            }
        }
        cout << "};" << endl << endl;

    }

    float* wavetable = new float[(int)numTables];
    //wavetable[0] = BLSawTable;

    cout << "// fundamental frequency for each table" << endl;
    cout << "static const float BLSawFreqTable[" << (int)numTables << "] = {";
    for (int i=0; i<numTables; i++) {
        cout << BLSawFreqTable[i] << "f";
        if (i<numTables-1) {
            cout << ", ";
        }
    }
    cout << "};" << endl << endl;

    cout << "// number of samples in each table" << endl;
    cout << "static const int BLSawSamplesTable[" << (int)numTables << "] = {";
    for (int i=0; i<numTables; i++) {
        cout << (int)BLSawSamplesTable[i];
        if (i<numTables-1) {
            cout << ", ";
        }
    }
    cout << "};" << endl << endl;

    cout << "// link everything in one array" << endl;
    cout << "static const float* wavetable[" << (int)numTables << "] = {";
    for (int i=0; i<numTables; i++) {
        cout << "BLSawTable" << (int)i;
        if (i<numTables-1) {
            cout << ", ";
        }
    }
    cout << "};" << endl << endl;

    cout << "int BLNumTables = " << (int)numTables << ";" << endl;
    return 0;
}
