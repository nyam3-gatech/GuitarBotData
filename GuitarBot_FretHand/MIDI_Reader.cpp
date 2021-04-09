#include "MIDI_Reader.h"
#include <iostream>
#include <exception>
#include <sstream>

using namespace std;

// Default constructor
MIDI_Reader::MIDI_Reader()
{
	errorflag = 1;
}

// MIDI_Reader Constructor takes in MIDI filename as a string and will attempt to read data from it.
MIDI_Reader::MIDI_Reader(const string MIDI_file)
{
	filename = MIDI_file;

	try
	{
		read();
	}
	catch (ifstream::failure& ex)
	{
		cerr << ex.what();
		errorflag = 1;
	}
}

// Attempts to read data from a MIDI file using the provided filename.
void MIDI_Reader::read(const string MIDI_file)
{
	filename = MIDI_file;

	try
	{
		read();
	}
	catch (ifstream::failure& ex)
	{
		cerr << ex.what();
		errorflag = 1;
	}
}


/*
 * Main Implementation of read() and readVLQ() functions for all classes
 */

// Attempts to read data from the MIDI file
void MIDI_Reader::read()
{
	// create an ifstream using the filename provided from the constructor
	ifstream inputFile(filename, ifstream::in | ifstream::binary);

	// Check if the file is good
	if(!inputFile.good()) throw ifstream::failure("File could not be found/read.");
	errorflag = 0; // file is good, clear the error flag

	inputFile >> noskipws; // Do not skip white spaces in file

	unsigned char c; // input buffer for 1 byte of data

	inputFile >> c >> c >> c >> c; // Reads in "MThd" marker for header chunk

	inputFile >> c >> c >> c >> c; // Reads in the length for the header chunk -> should always be six

	// Reads in the format data
	inputFile >> c >> c;
	format = c;

	// Reads in the number of tracks
	inputFile >> c;
	num_tracks = c; // assigns most significant byte
	num_tracks <<= 8; // bit shift left by 8
	inputFile >> c;
	num_tracks += c; // assigns least significant byte

	// Reads in the division information
	inputFile >> c;
	division = c;
	division <<= 8;
	inputFile >> c;
	division += c;

	divType = division >> 15; // gets bit 15 of division data
	if (divType)
	{
		nSMPTEFormat = division >> 8; // gets bits 15 to 8
		divTick = division & 0x00FF; // gets bits 7 to 0
	}
	else
	{
		divTick = division & 0x7FFF; // gets bits 14 to 0
	}

	// Clear the tracks vector of any old data
	tracks.clear();

	// For each track, read data from file
	for (int i = 0; i < num_tracks; i++)
	{
		tracks.push_back(Track_Chunk());
		tracks[i].read(inputFile);
	}

}

// Read data for a single track from a file
void Track_Chunk::read(ifstream& inputFile)
{
	unsigned char c; // input buffer for 1 byte of data

	// Reads in "MTrk" marker for track chunk
	inputFile >> c >> c >> c >> c;

	// Read in the length of the track chunk
	for (int i = 0; i < 4; i++)
	{
		inputFile >> c;
		length <<= 8;
		length += c;
	}

	// Clear the events vector
	events.clear();

	
	// Counter for how many bytes of this track has been read.
	int bytesRead = 0;
	// Keep reading the file for track events data until the bytes read equal the length of the track
	for (int i = 0; bytesRead < length; i++)
	{
		events.push_back(MTrk_Event()); // Create new MTrk_Event object and add it to the events vector
		bytesRead += events[i].read(inputFile, c); // Read event data from file, add the number of bytes read to the counter
	}
}

// Read data for a single MTrk event from a file
int MTrk_Event::read(ifstream& inputFile, unsigned char& runningStatus)
{
	data.clear(); // clear the data vector

	int bytesRead = 1; // Counter for how many bytes have been read
	unsigned char c; // input buffer for 1 byte of data

	delta_time = 0;
	bytesRead += readVLQ(inputFile, delta_time); // Reads in the delta time value for the event

	// Get the status byte for the event
	inputFile >> c;


	// Special case for when status byte is omitted
	if (c < 0x80)
	{
		inputFile.putback(c);
		bytesRead--;
	}
	else
	{
		data.push_back(c);
		runningStatus = c;
	}

	int bytesToRead = 0; // How many additional bytes need to be read for this event

	// Use the status byte to determine how many data bytes to read
	if (c == 0xF3 || (c >> 4) == 0xC || (c >> 4) == 0xD)
	{
		bytesToRead = 1;
	}
	else if (c < 0xF0)
	{
		bytesToRead = 2;
	}
	else if (c == 0xF0)
	{
		// Read in the length of the following event data
		bytesRead += readVLQ(inputFile, bytesToRead);
	}
	else if (c == 0xFF)
	{
		// Read in the event type byte
		inputFile >> c;
		data.push_back(c);

		// Read in the length of the following event data
		bytesRead += 1 + readVLQ(inputFile, bytesToRead);
	}

	// Read in any data bytes for this event
	for (int i = 0; i < bytesToRead; i++)
	{
		inputFile >> c;
		data.push_back(c);
	}

	return bytesRead + bytesToRead;
}

// Read a variable length quantity from an input file and store the value in a buffer. Returns the number of bytes read.
int MTrk_Event::readVLQ(ifstream& inputFile, int& buffer)
{
	int bytesRead = 0; // Counter for how many bytes have been read
	unsigned char c; // input buffer for 1 byte of data

	buffer = 0;

	// Read one byte first
	do
	{
		inputFile >> c;
		buffer <<= 7; // Each byte contains 7 bits of numerical data, so left shift by 7 bits for each byte read after the first.
		buffer += c & 0x7F; // Add the value of c with the first bit removed.
		bytesRead++;
	}
	while (c > 0x7F); // If the first bit is a 1, read another byte

	return bytesRead;
}

/*
 * get() functions for MIDI_Reader class
 */

bool MIDI_Reader::good() const
{
	return !errorflag;
}

string MIDI_Reader::getFilename() const
{
	return filename;
}

char MIDI_Reader::getformat() const
{
	return format;
}

short int MIDI_Reader::getNumTracks() const
{
	return num_tracks;
}

short int MIDI_Reader::getDivision() const
{
	return division;
}

char MIDI_Reader::getDivType() const
{
	return divType;
}

short int MIDI_Reader::getDivTick() const
{
	return divTick;
}

short int MIDI_Reader::getNSMPTEFormat() const
{
	return nSMPTEFormat;
}

Track_Chunk MIDI_Reader::getTrack(int index) const
{
	return tracks[index];
}

vector<Track_Chunk> MIDI_Reader::getTracks() const
{
	return tracks;
}

/*
 * get() functions for Track_Chunk class
 */

int Track_Chunk::getLength() const
{
	return length;
}

int Track_Chunk::getNumEvents() const
{
	return events.size();
}

MTrk_Event Track_Chunk::getEvent(int index) const
{
	return events[index];
}

vector<MTrk_Event> Track_Chunk::getEvents() const
{
	return events;
}

/*
 * get() functions for MTrk_Event class
 */

int MTrk_Event::getDeltaTime() const
{
	return delta_time;
}

unsigned char MTrk_Event::getData(int index) const
{
	return data[index];
}

vector<unsigned char> MTrk_Event::getData() const
{
	return data;
}

/*
 * toString() functions for all classes
 */

string MIDI_Reader::toString() const
{
	string result = headerToString();
	for (unsigned int i = 0; i < tracks.size(); i++)
	{
		result += tracks[i].toString();
	}
	return result;
}

// Creates a string representation of the MIDI header and data.
string MIDI_Reader::headerToString() const
{
	string header = "MIDI Header:\n- Format: " + to_string(format);
	header += "\n- Number of Tracks: " + to_string(num_tracks);
	header += "\n- Division Type: " + to_string(divType);
	if (divType)
	{
		header += "\n- Negative SMPTE Format: " + to_string(nSMPTEFormat);
		header += "\n- Ticks per Frame: " + to_string(divTick);
	}
	else
	{
		header += "\n- Ticks per Quarter Note: " + to_string(divTick);
	}
	return header + "\n";
}

string Track_Chunk::toString() const
{
	string track = "MIDI Track:";
	track += "\n- Byte Length: " + to_string(length) + "\n";
	for (unsigned int i = 0; i < events.size(); i++)
	{
		track += events[i].toString();
	}
	return track;
}

string MTrk_Event::toString() const
{
	stringstream ss;
	ss << "<delta time = " << dec << delta_time << ", data =" << hex;
	for (unsigned int i = 0; i < data.size(); i++)
	{
		ss << " " << (unsigned int) data[i];
	}
	return ss.str() + ">\n";
}