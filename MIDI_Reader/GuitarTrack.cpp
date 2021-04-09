#include "GuitarTrack.h"

#include <chrono>
#include <mutex>
#include <thread>

#include <iostream>

GuitarTrack::GuitarTrack(MIDI_Reader& m)
{
	tuning[0] = 40; // E
	tuning[1] = 45; // A
	tuning[2] = 50; // D
	tuning[3] = 55; // G
	tuning[4] = 59; // B
	tuning[5] = 64; // E

	processMIDI(m);

	setTempo(120);
	calcTickTime();

	state = READY;
}

GuitarTrack::~GuitarTrack()
{
	for (GuitarEvent* ptr : g_track)
	{
		delete ptr;
	}
}

// Process MIDI data into playable chord events stored in the guitar track.

void GuitarTrack::processMIDI(MIDI_Reader& m_reader)
{
	division = m_reader.getDivision();
	divType = m_reader.getDivType();
	divTick = m_reader.getDivTick();
	nSMPTEFormat = m_reader.getNSMPTEFormat();

	vector<GuitarEvent*> tempNotes; // holds NoteEvents without a duration
	vector<GuitarEvent*> tempTrack;

	for (Track_Chunk m_track : m_reader.getTracks())
	{
		int tick = 0;
		unsigned char rs = 0;

		for (MTrk_Event m_event : m_track.getEvents())
		{
			tick += m_event.getDeltaTime();

			int index = 0;

			unsigned char status = m_event.getData(index);

			if (status < 0x80)
			{
				status = rs;
			}
			else
			{
				rs = status;
				index++;
			}

			bool zero_velocity = 0; // Note-on event with 0-velocity is same as note-off event

			if ((status & 0xF0) == 0b10010000) // Note on event
			{
				if (m_event.getData(index + 1))
				{
					GuitarEvent* ptr = new NoteEvent(tick, status & 0x0F, m_event.getData(index), m_event.getData(index + 1));
					tempNotes.push_back(ptr);
					continue;
				}
				zero_velocity = 1;
			}
			if (((status & 0xF0) == 0b10000000) || zero_velocity) // Note off event
			{
				for (unsigned int i = 0; i < tempNotes.size(); i++)
				{
					NoteEvent* n_ptr = (NoteEvent*) tempNotes[i];

					if (n_ptr->getNote() == m_event.getData(index))
					{
						if (n_ptr->getChannel() == (status & 0x0F))
						{
							n_ptr->setDuration(tick - n_ptr->getTick());
							tempTrack.push_back(n_ptr);
							tempNotes.erase(tempNotes.begin() + i);
							break;
						}
					}
				}
			}
			else if (status == 0xFF) // Meta-Events
			{
				if (m_event.getData(index) == 0x51 && m_event.getData(index + 1) == 0x03) // Set Tempo
				{
					// Placeholder
				}
			}
		}
	}

	ChordEvent* chord_ptr = 0;

	for (unsigned int i = 0; i < tempTrack.size(); i++)
	{
		GuitarEvent* ptr = tempTrack[i];
		if (ptr->getType() == NOTE)
		{
			NoteEvent* n_ptr = (NoteEvent*) ptr;
			if (chord_ptr) // check if a chord already exists
			{
				if (n_ptr->getTick() < chord_ptr->getTick() + chord_ptr->getDuration()) // check if the starting tick of the note overlaps with the chord's duration
				{
					if (chord_ptr->getNotes().size() >= 6)
					{
						chord_ptr->setFrets(tuning);
						chord_ptr->fixDuplicates();
						chord_ptr = new ChordEvent(n_ptr->getTick(), *n_ptr);
						g_track.push_back(chord_ptr);
					}
					else
					{
						chord_ptr->addNote(*n_ptr); // add the note to the chord
						continue;
					}
				}
				else
				{
					// create a new ChordEvent and add it to the track
					chord_ptr->setFrets(tuning);
					chord_ptr->fixDuplicates();
					chord_ptr = new ChordEvent(n_ptr->getTick(), *n_ptr);
					g_track.push_back(chord_ptr);
				}
			}
			else
			{
				// create a new ChordEvent and add it to the track
				chord_ptr = new ChordEvent(n_ptr->getTick(), *n_ptr);
				g_track.push_back(chord_ptr);
			}

			delete ptr;
		}
		else if (ptr->getType() == TEMPO)
		{
			g_track.push_back(ptr);
		}
		else
		{
			delete ptr;
		}
	}

	// Set the frets for the last chord
	chord_ptr->setFrets(tuning);
	chord_ptr->fixDuplicates();
}

// Set functions for tempo

void GuitarTrack::setTempo(float t)
{
	tempo = t;
	tempo_us = 60000000.0 / t;
}

void GuitarTrack::setTempoMicroseconds(unsigned int t_us)
{
	tempo = 60000000.0 / t_us;
	tempo_us = t_us;
}

void GuitarTrack::calcTickTime()
{
	tick_us = tempo_us / divTick;
}

// Runs a guitar track

void GuitarTrack::run_track()
{
	if(state == SETUP) state = RUNNING;
	for (GuitarEvent* event : g_track)
	{
		while (tick < event->getTick() && state == RUNNING);

		if (state != RUNNING) break;

		switch (event->getType())
		{
			case CHORD:
			{
				ChordEvent* chordPtr = (ChordEvent*) event;

				if (chordPtr->getLowestFret() < carFret)
				{
					arduinoInterface->addOrder(OrderTicket(0, MOVE, carFret - chordPtr->getLowestFret()));
				}
				else if (chordPtr->getHighestFret() > carFret + 2)
				{
					arduinoInterface->addOrder(OrderTicket(0, MOVE, chordPtr->getHighestFret() - carFret - 2));
				}

				for (NoteEvent note : chordPtr->getNotes())
				{
					unsigned char pin = ((note.getFret() - carFret) * 6) + note.getGuitarString() + 1;
					arduinoInterface->addOrder(OrderTicket(40, ON, pin));
				}
				break;
			}
			case TEMPO:
			{
				// Placeholder

				break;
			}
		}

		cout << event->toString();
	}

	state = STOP;
}

void GuitarTrack::run_track_static(GuitarTrack* track)
{
	track->run_track();
}

void GuitarTrack::run()
{
	if (state != READY) return;

	state = SETUP;

	tick = -1;

	thread t(run_track_static, this);
	t.detach();

	auto start = chrono::high_resolution_clock::now();
	auto elapsed = chrono::high_resolution_clock::now();
	long long time_us = chrono::duration_cast<chrono::microseconds>(elapsed - start).count();
	long time_marker = tick_us;

	while (state == SETUP);

	tick++;

	while (state == RUNNING)
	{
		elapsed = chrono::high_resolution_clock::now();
		time_us = chrono::duration_cast<chrono::microseconds>(elapsed - start).count();
		while (time_us > time_marker)
		{
			tick++;
			time_marker += tick_us;
		}
	}

	state = READY;
}

void GuitarTrack::stop()
{
	if(state != READY) state = STOP;
}

// Creates a string representation of the track

string GuitarTrack::toString()
{
	string str;
	for (GuitarEvent* ptr : g_track)
	{
		str += ptr->toString();
	}
	return str;
}