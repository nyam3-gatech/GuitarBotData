#include "GBotFret.h"
#include <iostream>

int main()
{
	MIDI_Reader* m = new MIDI_Reader("midis/Gil's Music 3 min.mid");
	//MIDI_Reader m("midis/tempotest.mid");
	//MIDI_Reader m("midis/Jumbo Music.mid");
	//MIDI_Reader m("midis/Gil's Reggae.mid");

	//cout << m.toString();

	GuitarTrack* gTrack = new GuitarTrack(m);

	std::cout << gTrack->toString();

	delete gTrack;
	delete m;
}
