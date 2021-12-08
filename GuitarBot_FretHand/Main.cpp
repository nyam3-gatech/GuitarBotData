#include <iostream>

#include "MIDI_Reader.h"
#include "GuitarTrack.h"
#include "MotorInterface.h"

#include <iostream>

using namespace std;

int main()
{
	//MIDI_Reader m("midis/Gil's Music 3 min.mid");
	//MIDI_Reader m("midis/tempotest.mid");
	//MIDI_Reader m("midis/Jumbo Music.mid");
	//MIDI_Reader m("midis/Gil's Reggae.mid");

	//cout << m.toString();

	//GuitarTrack gTrack(m);

	//cout << gTrack.toString();

	MotorInterface motors;
	int control = 0;
	// current = 200 for normal press, 50 for damped
	int current = 200;

	if (motors.getStatus())
	{
		cout << "Motor setup successful." << endl;

#define ESC_ASCII_VALUE                 0x1b

		for (int id = 0; id < 6; id++)
		{
			motors.disableTorque(id);
			motors.setPositionControl(id);
			motors.enableTorque(id);
			motors.setGoalPosition(id, 1800);
		}

		int input;
		bool flags[6] = { 0,0,0,0,0,0};

		while (current)
		{
			do
			{
				cout << "Motor ID: ";
				cin >> input;
				if (input < 6 && input >= 0) flags[input] = 1;
			}
			while (input < 6 && input >= 0);
			cout << "Current: ";
			cin >> current;
			cout << "Set to current control." << endl;
			for (int id = 0; id < 6; id++)
			{
				if (flags[id])
				{
					motors.disableTorque(id);
					motors.setCurrentControl(id);
					motors.enableTorque(id);
					motors.setGoalCurrent(id, current);
					flags[id] = 0;
				}
				else
				{
					motors.disableTorque(id);
					motors.setPositionControl(id);
					motors.enableTorque(id);
					motors.setGoalPosition(id, 1800);
				}
			}
		}

		for (int id = 0; id < 6; id++)
		{
			motors.disableTorque(id);
			motors.setPositionControl(id);
			motors.enableTorque(id);
			motors.setGoalPosition(id, 1800);
			motors.disableTorque(id);
		}

	}
	else
	{
		cout << "Motor setup failed." << endl;
	}


}