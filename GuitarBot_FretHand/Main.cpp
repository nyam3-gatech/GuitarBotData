#include <iostream>

#include "MIDI_Reader.h"
#include "GuitarTrack.h"

using namespace std;

int main()
{
	MIDI_Reader m("midis/Gil's Music 3 min.mid");
	//MIDI_Reader m("midis/Jumbo Music.mid");
	//MIDI_Reader m("midis/Gil's Reggae.mid");

	//cout << m.toString();

	GuitarTrack gTrack(m);

	cout << gTrack.toString();

	// EXAMPLE - Accessing Chord Data

	vector<ChordEvent*>& chords = gTrack.getChords();

	chords[0]->getTick();
	chords[0]->getDirection();
	chords[0]->getTechnique();

	// Real time communication with Arduino
	
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