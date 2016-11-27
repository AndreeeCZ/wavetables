SAMPLERATE=44100
WAVETABLE_SIZE=250

all:
	g++ wavetables.cpp -o wavetables -lm -lquadmath
clean:
	rm -f ./wavetables
generate:
	rm -f output/saw.h
	rm -f output/gauss.h
	rm -f output/triangle.h
	rm -f output/square.h
	./wavetables $(SAMPLERATE) $(WAVETABLE_SIZE) Saw > output/saw.h
	./wavetables $(SAMPLERATE) $(WAVETABLE_SIZE) Triangle > output/triangle.h
	./wavetables $(SAMPLERATE) $(WAVETABLE_SIZE) Gauss > output/gauss.h
	./wavetables $(SAMPLERATE) $(WAVETABLE_SIZE) Square > output/square.h
