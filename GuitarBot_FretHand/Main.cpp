#include <iostream>
#include "MIDI_Reader.h"
#include "GuitarTrack.h"

using namespace std;

int main()
{
	//MIDI_Reader m("midis/Gil's Reggae.mid");

	//cout << m.toString();

	//GuitarTrack gTrack(m);

	//cout << gTrack.toString();

	//gTrack.run();

	string port = ""; // NEED TO ADD COM PORT

	SerialInterface arduino(port);

	if (arduino.isConnected())
	{
		arduino.startSending();

		arduino.addOrder(OrderTicket(0, ON, 3));

		arduino.addOrder(OrderTicket(2000, OFF, 3));
	}
	else
	{
		cout << "Error: Arduino not connected." << endl;
	}


}