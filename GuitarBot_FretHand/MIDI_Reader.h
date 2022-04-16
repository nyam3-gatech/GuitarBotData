#ifndef MIDI_READER_H
#define MIDI_READER_H

#include <string>
#include <vector>
#include <fstream>

class MTrk_Event
{
private:

	friend class Track_Chunk;

	int delta_time;
	std::vector<unsigned char> data;

	bool read(std::ifstream&, unsigned char&);

	// Helper function to read a variable-length quantity from an input file and returns it as an int.
	int readVLQ(std::ifstream&);

public:

	int getDeltaTime() const;
	unsigned char getData(int) const;
	std::vector<unsigned char> getData() const;

	std::string toString() const;

};

class Track_Chunk
{
private:

	friend class MIDI_Reader;

	int length;
	std::vector<MTrk_Event> events;

	void read(std::ifstream&);

public:

	int getLength() const;
	int getNumEvents() const;
	MTrk_Event getEvent(int) const;
	std::vector<MTrk_Event> getEvents() const;

	std::string toString() const;

};

class MIDI_Reader
{
private:

	std::string filename;
	bool errorflag;

	unsigned char format;
	unsigned short int num_tracks;
	unsigned short int division;
	unsigned char divType;
	unsigned short int divTick;
	char nSMPTEFormat;

	std::vector<Track_Chunk> tracks;

public:

	MIDI_Reader();
	MIDI_Reader(const std::string);

	void read(const std::string);
	void read();

	bool good() const;

	std::string getFilename() const;
	char getformat() const;
	short int getNumTracks() const;
	short int getDivision() const;
	char getDivType() const;
	short int getDivTick() const;
	short int getNSMPTEFormat() const;

	Track_Chunk getTrack(int) const;
	std::vector<Track_Chunk> getTracks() const;

	std::string toString() const;
	std::string headerToString() const;
};

#endif


