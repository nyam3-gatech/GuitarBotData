#ifndef MIDI_RUNNER_H
#define MIDI_RUNNER_H

#include <thread>
#include "MIDI_Reader.h"

using namespace std;

class MIDI_Runner
{
private:

	MIDI_Reader m_reader;

#define READY 1
#define SETUP 2
#define RUNNING 3

	char state;

	float tempo;
	unsigned int tempo_us;
	unsigned int tick;
	double tick_us;
	
	unsigned char tracks_running;

	void setTempo(float = 120);
	void setTempoMicroseconds(unsigned int = 500000);
	void calcTickTime();
	
	void run_track(Track_Chunk);
	static void run_track_static(MIDI_Runner*, Track_Chunk);

	void run_event(MTrk_Event&, unsigned char&);

	// Function pointers
	void (*playNote)(unsigned int, int, int, int);

public:

	MIDI_Runner(MIDI_Reader);
	MIDI_Runner(string);

	void run();

	// Set the playNote function
	void setPlayNoteFunction(void(*)(unsigned int, int, int, int));

};

#endif

