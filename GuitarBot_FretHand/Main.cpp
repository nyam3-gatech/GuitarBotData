#include <iostream>
#include "MIDI_Reader.h"
#include "GuitarTrack.h"

using namespace std;

int main()
{
	MIDI_Reader m("midis/Gil's Reggae.mid");

	//cout << m.toString();

	GuitarTrack gTrack(m);

	//cout << gTrack.toString();
	
	string port = ""; // NEED TO ADD COM PORT

	SerialInterface arduino(port);

	if (arduino.isConnected())
	{
		gTrack.run();
	}
	else
	{
		cout << "Error: Arduino not connected." << endl;
	}
	

}