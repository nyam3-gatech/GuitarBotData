#ifndef MIDI_READER_H
#define MIDI_READER_H

#include <string>
#include <vector>
#include <fstream>

using namespace std;

class MTrk_Event
{
private:

	friend class Track_Chunk;

	int delta_time;
	vector<unsigned char> data;

	int read(ifstream&, unsigned char&);

	// Helper function to read a variable-length quantity from an input file and stores it in an int buffer. Returns the number of bytes read.
	int readVLQ(ifstream&, int&);

public:

	int getDeltaTime() const;
	unsigned char getData(int) const;
	vector<unsigned char> getData() const;

	string toString() const;

};

class Track_Chunk
{
private:

	friend class MIDI_Reader;

	int length;
	vector<MTrk_Event> events;

	void read(ifstream&);

public:

	int getLength() const;
	int getNumEvents() const;
	MTrk_Event getEvent(int) const;
	vector<MTrk_Event> getEvents() const;

	string toString() const;

};

class MIDI_Reader
{
private:

	string filename;
	bool errorflag;

	unsigned char format;
	unsigned short int num_tracks;
	unsigned short int division;
	unsigned char divType;
	unsigned short int divTick;
	char nSMPTEFormat;

	vector<Track_Chunk> tracks;

public:

	MIDI_Reader();
	MIDI_Reader(const string);

	void read(const string);
	void read();

	bool good() const;

	string getFilename() const;
	char getformat() const;
	short int getNumTracks() const;
	short int getDivision() const;
	char getDivType() const;
	short int getDivTick() const;
	short int getNSMPTEFormat() const;

	Track_Chunk getTrack(int) const;
	vector<Track_Chunk> getTracks() const;

	string toString() const;
	string headerToString() const;
};

#endif


