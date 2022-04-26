#include "GuitarTrack.h"

#include <cmath>
#include <iostream>

using namespace std;

GuitarTrack::GuitarTrack(const MIDI_Reader* m)
{
	tuning[0] = 40; // E
	tuning[1] = 45; // A
	tuning[2] = 50; // D
	tuning[3] = 55; // G
	tuning[4] = 59; // B
	tuning[5] = 64; // E

	gTab = GTab(tuning);

	readFromMIDI(m);

	setTempoBPM(120);
}

GuitarTrack::~GuitarTrack()
{
	for (GuitarEvent* ptr : g_track)
	{
		delete ptr;
	}
}

// Process MIDI data into playable chord events stored in the guitar track.
void GuitarTrack::readFromMIDI(const MIDI_Reader* m_reader)
{
	division = m_reader->getDivision();
	divType = m_reader->getDivType();
	divTick = m_reader->getDivTick();
	nSMPTEFormat = m_reader->getNSMPTEFormat();

	vector<NoteEvent*> tempNotes; // holds NoteEvents without a play_time
	vector<GuitarEvent*> tempTrack;

	int minimumDuration = 0x7FFFFFFF;

	for (Track_Chunk m_track : m_reader->getTracks())
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
					tempNotes.push_back(new NoteEvent(tick, status & 0x0F, m_event.getData(index), m_event.getData(index + 1)));
					continue;
				}
				zero_velocity = 1;
			}
			if (((status & 0xF0) == 0b10000000) || zero_velocity) // Note off event
			{
				for (unsigned int i = 0; i < tempNotes.size(); i++)
				{
					NoteEvent* n_ptr = tempNotes[i];

					if (n_ptr->getNote() == m_event.getData(index))
					{
						if (n_ptr->getChannel() == (status & 0x0F))
						{
							n_ptr->setEndTick(tick);
							if (n_ptr->getDuration() != 0 && n_ptr->getDuration() < minimumDuration)
							{
								minimumDuration = n_ptr->getDuration();
							}
							tempTrack.push_back(n_ptr);
							tempNotes.erase(tempNotes.begin() + i);
							break;
						}
					}
				}
			}
			else if (status == 0xFF) // Meta-Events
			{
				if (m_event.getData(index) == 0x51) // Set Tempo
				{
					int microsecondsPerQuarter = (m_event.getData(index + 1) << 16) + (m_event.getData(index + 2) << 8) + m_event.getData(index + 3);
					TempoEvent* t_ptr = new TempoEvent(tick, microsecondsPerQuarter);
					cout << "Tempo: " << microsecondsPerQuarter << endl;
					tempTrack.push_back(t_ptr);
				}
			}
		}
	}

	int exponent = round(log2((1.0 * minimumDuration) / divTick));
	int updown_beat_tick = round(divTick * pow(2, exponent));

	ChordEvent* chord_ptr = 0;

	for (unsigned int i = 0; i < tempTrack.size(); i++)
	{

		GuitarEvent* ptr = tempTrack[i];
		if (ptr->getType() == NOTE)
		{
			NoteEvent* n_ptr = (NoteEvent*) ptr;

			// check conditions to make new ChordEvent
			if (!chord_ptr || (n_ptr->getTick() >= chord_ptr->getTick() + chord_ptr->getDuration()) || chord_ptr->getNotes().size() >= 6)
			{
				// create a new ChordEvent and add it to the track
				chord_ptr = new ChordEvent(n_ptr->getTick(), *n_ptr);
				g_track.push_back(chord_ptr);
				chordEvents.push_back(chord_ptr);
			}
			else // else add note to active ChordEvent
			{
				chord_ptr->addNote(*n_ptr);
			}
			delete ptr;
		}
		else if (ptr->getType() == TEMPO)
		{
			g_track.push_back(ptr);
			tempoChanges.push_back((TempoEvent*) ptr);
		}
		else
		{
			delete ptr;
		}
	}

	gTab.setFrets(chordEvents);

	setChordDirections(updown_beat_tick);
}

// Set the picking direction for picked notes
void GuitarTrack::setChordDirections(int updown_beat_tick)
{
	ChordEvent* last = 0;
	ChordEvent* current = chordEvents[0];
	ChordEvent* next = (chordEvents.size() > 1) ? chordEvents[1] : 0;

	int i = 0;
	while (i < chordEvents.size())
	{
		// Check if the chord is just a single picked note
		if (current->getTechniqueClass() == PICK)
		{
			// Check conditions for when picking direction should be up
			if (next && (next->getTechniqueClass() == PICK) ?
					next->getNotes()[0].getGuitarString() < current->getNotes()[0].getGuitarString()
						|| next->getNotes()[0].getGuitarString() == current->getNotes()[0].getGuitarString()
						&& last && last->getTechniqueClass() == PICK && last->getDirection() == DOWN
					: last && last->getTechniqueClass() == PICK && last->getDirection() == DOWN
				)
			{
				current->setDirection(UP);
			}
			else
			{
				current->setDirection(DOWN);
			}
		}
		else // set strumming direction
		{
			if ((current->getTick() / updown_beat_tick) % 2)
			{
				current->setDirection(UP);
			}
			else
			{
				current->setDirection(DOWN);
			}
		}

		current->setContactStrings();

		i++;

		last = current;
		current = next;
		next = (i < chordEvents.size()) ? chordEvents[i] : 0;
	}
}

// Set functions for tempo

void GuitarTrack::setTempoBPM(float t)
{
	tempo_bpm = t;
	tempo_us = 60000000.0 / t;
	calcTickTime();
}

void GuitarTrack::setTempoMicroseconds(unsigned int t_us)
{
	tempo_bpm = 60000000.0 / t_us;
	tempo_us = t_us;
	calcTickTime();
}

// Calculates how long each tick is in microseconds
void GuitarTrack::calcTickTime()
{
	tick_us = tempo_us / divTick;
}

vector<ChordEvent*>& GuitarTrack::getChords()
{
	return chordEvents;
}

// Returns pointer to a GuitarEvent
const GuitarEvent* GuitarTrack::getEvent(int index)
{
	if (index < 0 || index >= g_track.size()) return 0;

	return g_track[index];
}

// Returns the time in microseconds when a GuitarEvent executes
long long GuitarTrack::getEventTime(int index)
{
	if (index < 0 || index >= g_track.size()) return -1;

	return 0; // PLACEHOLDER
}

long long GuitarTrack::ticks_to_us(int ticks)
{
	return ticks * tick_us;
}

double GuitarTrack::ticks_to_seconds(int ticks)
{
    return 60.0 * ticks / (tempo_bpm * divTick);
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