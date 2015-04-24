all: 
	g++ wavetables.cpp -o wavetables -lm
clean:
	rm -f ./wavetables