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

#define USE_QUAD_MATH

#ifdef USE_QUAD_MATH

#include <quadmath.h>
#define float __float128
#undef M_PI
#define M_PI M_PIq
#define sinf sinq
#define cosf cosq
#define round roundq
#define expf expq
#define logf logq
#endif

#define FLOAT float


using namespace std;



int main(int argc, char* argv[]) {
	//create a logarithmic table from 0 to 1
	float logTable[1000];
	for (int i=0; i<1000; i++) {
		logTable[i] = expf((logf(1.f)-logf(0.001f))*(i/1000.f)+logf(0.001f));
	}

	//prepare a float[] tables for band limited saw waves @ 48kHz, one octave each
	float numTables = atoi(argv[2]);
	int SRT = atoi(argv[1]);
	std::string type = argv[3];
	

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
		freq += 25.f;

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

		cout << (double)freq << "Hz         ";
		cout << "Sample Count: " << (double)tableLength << endl;

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


	cout << "Sample rate: " << (double)SRT << endl;
	cout << endl << "Total number of floats: " << (double)totalSampleCount << endl << "Memory taken: " << totalSampleCount*sizeof(double)/2 << " bytes. */" << endl << endl;

	int plotOffset = 0;
	int plotOffset2 = 0;

	int maximums[(int)numTables];
	int minimums[(int)numTables];

	for (int i=0; i<numTables; i++) {
		// iterate over BLSawTable[i] and generate sine waves at all harmonics until the highest freq<24kHz

		freq = BLSawFreqTable[i];
		//cout << "Creating new waveform, fundamental at " << freq << "Hz" << endl;
		tableLength = BLSawSamplesTable[i];
		float PA = 0.f; // phase accumulator 0 .. 2pi
		float PAInc = 0.f; // increment to the phase accumulator
		float triangleAlt = 1.f;
		float gaussPhaseRotate = 0.f;
		freq = BLSawFreqTable[i];
		int totalHarmonics = 0;
		for (int h = 1; freq<=SRT*0.5f; h++) {
			// iterate over each harmonic to generate a a saw wave
			//float h = 1.f;
			freq = BLSawFreqTable[i]*h;

			totalHarmonics = h;
			PA = 0.f;
			PAInc = 2.f*M_PI/SRT*freq;
			int cycleSize = SRT/freq;
			float mult = 1.f;
			if (h%2 == 0) {
				// boost even harmonics
				mult = 1.1f;
				triangleAlt*=-1.f;
			}
			if (h == 3) {
				// boost 3rd harmonic
				mult = 1.3f;
			}
			float gaussPhases[8] = {0.f, M_PI/4.f, M_PI, -M_PI/8.f, 2*M_PI, M_PI/12.f, 3.f*M_PI, -M_PI/16.f};

			for (int s = 0; s<tableLength; s++) {
				// generate a sine wave at freq
				
				float sine = sinf(PA)*0.4f; // the sinwave
				float freqFraction = freq/((float)SRT*0.5f);
				float addFilter = 1.f;
				if (freqFraction>0.7f) {
					addFilter = cosf((freqFraction-0.7f)*14.f);
					//std::cout << "freq: " << freq << " fract: " << freqFraction << " filt: " << addFilter << std::endl;
				}
				float addFilter2 = 1.f;
				if (freqFraction>0.65) {
					addFilter2 = cosf((freqFraction-0.65f)*10.f);
				}
				if (addFilter<0.f || addFilter2<0.f) {
                    //std::cout << "Zeroing on at " << freq << std::endl;
                    sine = 0.f;
                }
				float even = 1.f;
				if (h%2==0) {
					even = 1.1f;
				}
				sine *= addFilter;
				sine *= addFilter2;
				sine *= even;
				if (type == "Saw") {
					// we go from -0.37 to 0.37
					float saw1 = sine*(1.f/sqrtf(h))*0.1f; // a nice saw
					float saw2 = sine*(1.f/h)*0.5f; // an ugly saw
					float saw1Coeff = 1.f;
                    float saw2Coeff = 0.f;
					float addFilter3 = 1.f;
					if (freqFraction>0.5f) {
						addFilter3 = cosf((freqFraction-0.5f)*2.f);
					}
					BLSawTable[i][s] += (saw1*saw1Coeff + saw2*saw2Coeff) * addFilter3 * mult;
					
				} else if (type == "Square") {
					if (h%2 != 0) {
						// we go from -0.37 to 0.37
						float square1 = sine*(1.f/h); // a nice square
						float square2 = sine*(1.f/sqrtf(h))*0.3f; // an ugly square
						float square1Coeff = 1.f;
                        float square2Coeff = 0.f;
						BLSawTable[i][s] += square1*square1Coeff + square2*square2Coeff;
					}
				} else if (type == "Triangle") {
					if (h%2 != 0) {
						BLSawTable[i][s] += sine*(1.f/(h*h))*triangleAlt*0.7f;
					}
				} else if (type == "Gauss") {
					float steep = 0.11f;
					float coeff = powf(h, h*steep)+3.f;
					float x = PA;
					float phase = round((fmodf(h*4.f, 4.f) + 0.5f))*0.25f-0.25f;
					//float wave = cosf(h * (x + phase*M_PI))/coeff;
					float wave = cosf(1*x)/coeff;
					BLSawTable[i][s] += wave;
				}
				PA += PAInc;
			}

		}
		if (type == "Gauss") {
			float min;
			float max;
			min = 0.f;
			max = 0.f;
			for (int s = 0; s<tableLength; s++) {
				BLSawTable[i][s] = BLSawTable[i][s]-0.9f;
				BLSawTable[i][s] *= 0.36f;
				// find minimums and maximums
				if (BLSawTable[i][s]>max) {
					max = BLSawTable[i][s];
				}
				if (BLSawTable[i][s]<min) {
					min = BLSawTable[i][s];
				}
			}
			minimums[i] = min;
			maximums[i] = max;

			//std::cout << "// Minimum for " << i << ": " << min << std::endl;
			//std::cout << "// Maximum for " << i << ": " << max << std::endl;
		}

        #ifdef USE_QUAD_MATH
            #undef float
            //#define float float
        #endif
		
		
		// print all as arrays
		cout << endl << "// fundamental at " << (double)BLSawFreqTable[i] << "Hz, " << (int)BLSawSamplesTable[i] << " samples, number of harmonics: " << totalHarmonics << endl;
		cout << "static const float BL" << type << "Table" << i << "[" << (int)tableLength << "] = {";
		for (int s = 0; s<tableLength; s++) {
			plotOffset++;
            float print = BLSawTable[i][s];
			cout << fixed << std::setprecision(30) << print << "f";
			if (s<tableLength-1) {
				cout << ", ";
			}
		}
		cout << "};" << endl << endl;

	}

	float* wavetable = new float[(int)numTables];
	//wavetable[0] = BLSawTable;
    
	cout << "// fundamental frequency for each table" << endl;
	cout << "static const float BL" << type << "FreqTable[" << (int)numTables << "] = {";
	for (int i=0; i<numTables; i++) {
        float print = BLSawFreqTable[i];
		cout << print << "f";
		if (i<numTables-1) {
			cout << ", ";
		}
	}
	cout << "};" << endl << endl;

	cout << "// number of samples in each table" << endl;
	cout << "static const int BL" << type << "SamplesTable[" << (int)numTables << "] = {";
	for (int i=0; i<numTables; i++) {
		cout << (int)BLSawSamplesTable[i];
		if (i<numTables-1) {
			cout << ", ";
		}
	}
	cout << "};" << endl << endl;

	cout << "// link everything in one array" << endl;
	cout << "static const float* wavetable" << type << "[" << (int)numTables << "] = {";
	for (int i=0; i<numTables; i++) {
		cout << "BL" << type << "Table" << (int)i;
		if (i<numTables-1) {
			cout << ", ";
		}
	}
	cout << "};" << endl << endl;
	cout << "#ifndef WAVETABLEINCLUDED" << endl;
	cout << "#define WAVETABLEINCLUDED" << endl;
	cout << "int BLNumTables = " << (int)numTables << ";" << endl;
	cout << "#endif" << endl;
	return 0;
}
