#ifndef GUITAR_TRACK_H
#define GUITAR_TRACK_H

#include "MIDI_Reader.h"
#include "GuitarEvent.h"
#include "GTab.h"

class GuitarTrack
{
private:

	// MIDI Data

	unsigned short int division;
	unsigned char divType;
	unsigned short int divTick;
	char nSMPTEFormat;

	// Track Data

	std::vector<GuitarEvent*> g_track;
	std::vector<ChordEvent*> chordEvents;
	std::vector<TempoEvent*> tempoChanges;

	// GTab - Used to calculate strings and frets to use

	GTab gTab;

	// Sets the directions of strumming/picking for all chords
	void setChordDirections(int);

	// Tempo variables and functions

	double tempo_bpm;
	unsigned int tempo_us;
	double tick_us;
	
	void setTempoBPM(float = 120);
	void setTempoMicroseconds(unsigned int = 500000);
	void calcTickTime();

	// Guitar tuning / pitch of open strings
	unsigned char tuning[6];

public:

	GuitarTrack(std::string);
	GuitarTrack(const MIDI_Reader&);
	GuitarTrack(const MIDI_Reader*);

	~GuitarTrack();

	void readFromMIDI(const MIDI_Reader*);

	std::vector<ChordEvent*>& getChords();
	ChordEvent& getChord(int);
	const GuitarEvent* getEvent(int);
	long long getEventTime(int);
	long long ticks_to_us(int);
    double ticks_to_seconds(int);

	std::string toString();
};

#endif