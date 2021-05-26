#include <iostream>
#include "MIDI_Reader.h"
#include "GuitarTrack.h"

using namespace std;

int main()
{
	MIDI_Reader m("midis/Jumbo Music.mid");

	//cout << m.toString();

	GuitarTrack gTrack(m);

	/* EXAMPLE - Accessing Chord Data
	
	vector<ChordEvent*>& chords = gTrack.getChords();

	chords[0]->getTick();
	chords[0]->getDirection();
	chords[0]->getTechnique();

	*/

	cout << gTrack.toString();
	
	//string port = ""; // NEED TO ADD COM PORT

	/*
	SerialInterface arduino(port);

	if (arduino.isConnected())
	{
		gTrack.run();
	}
	else
	{
		cout << "Error: Arduino not connected." << endl;
	}
	*/

}