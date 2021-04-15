#ifndef GUITAR_EVENT_H
#define GUITAR_EVENT_H

#include <vector>
#include <string>

// Event Types

#define TEMPO 1
#define NOTE 2
#define CHORD 3

// Fret Restrictions

#define MAXFRET 19
#define FRET_RANGE 3

// Base class
class GuitarEvent
{
protected:

	friend class GuitarTrack;

	int tick;
	char type;

public:

	GuitarEvent(int, char);
	virtual ~GuitarEvent();

	int getTick() const;
	char getType() const;

	virtual std::string toString() = 0;
};

// TempoEvent changes the duration of a quarter note in microseconds
class TempoEvent : public GuitarEvent
{
private:

	int tempo_us;

public:

	TempoEvent(int, int);
	~TempoEvent();

	int getTempoMicroseconds() const;

	std::string toString();
};

// NoteEvent
class NoteEvent : public GuitarEvent
{
private:

	int channel;
	int endTick;
	int note;
	int velocity;
	char g_string;
	char fret;

public:

	NoteEvent(int, int, int, int);
	~NoteEvent();

	int getChannel();
	int getDuration();
	int getEndTick();
	int getNote();
	char getGuitarString();
	char getFret();

	void setDuration(int);
	void setEndTick(int t);
	void setGuitarString(int);
	void setFret(unsigned char*);

	std::vector<char> getPossibleFrets(unsigned char*);

	std::string toString();
};

// ChordEvent is a set of NoteEvents
class ChordEvent : public GuitarEvent
{
private:

	bool playable;
	std::vector<NoteEvent> notes;

	char calculateFitness(std::vector<char>);
	std::vector<char> getBestFit(char, std::vector<char>, std::vector<std::vector<char>>);

	char condenseStringFret(char, char);
	char extractString(unsigned char);
	char extractFret(unsigned char);

public:

	ChordEvent(int, NoteEvent);
	~ChordEvent();

	void addNote(NoteEvent);

	int getDuration();
	unsigned char getLowestFret();
	unsigned char getHighestFret();

	void sortByPitch();
	void sortByTime();

	void setFrets(unsigned char*);

	void fixDuplicates();

	bool checkConflict(unsigned char*);

	int checkForNote(int);
	bool removeNote(int);

	std::vector<NoteEvent>& getNotes();

	std::string toString();
};

#endif