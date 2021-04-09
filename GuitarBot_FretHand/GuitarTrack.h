#ifndef GUITAR_TRACK_H
#define GUITAR_TRACK_H

#include "MIDI_Reader.h"
#include "GuitarEvent.h"
#include "SerialInterface.h"

class GuitarTrack
{
private:

	// MIDI Data

	unsigned short int division;
	unsigned char divType;
	unsigned short int divTick;
	char nSMPTEFormat;

	// Track Data

	vector<GuitarEvent*> g_track;

	// Track state to prevent running the sane track at the same time

	#define READY 1
	#define SETUP 2
	#define RUNNING 3
	#define STOP 4
	char state;

	// Tempo variables and functions

	float tempo;
	unsigned int tempo_us;
	unsigned int tick;
	long tick_us;

	void setTempo(float = 120);
	void setTempoMicroseconds(unsigned int = 500000);
	void calcTickTime();

	// Private functions to run a track

	void run_track();
	static void run_track_static(GuitarTrack*);

	// Physical quantities and handling microcontroller pins

	unsigned char tuning[6]; // Tuning of the guitar

	SerialInterface* arduinoInterface;

	char frets[6]; // Current frets being pressed down
	char carFret; // Which fret are the servos currently at

	char getServo(char string, char fret);

public:

	GuitarTrack(MIDI_Reader&);

	~GuitarTrack();

	void processMIDI(MIDI_Reader&);

	void run();
	void stop();

	char getState();

	void addSerialInterface(SerialInterface*);

	string toString();
};

#endif