#include <iostream>
#include "MIDI_Reader.h"
#include "GuitarTrack.h"

using namespace std;

int main()
{
	MIDI_Reader m("Gil's Reggae.mid");

	//cout << m.toString();

	GuitarTrack gTrack(m);

	cout << gTrack.toString();

	//gTrack.run();
}