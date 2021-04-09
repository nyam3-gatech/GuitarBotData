#include "MIDI_Runner.h"
#include <chrono>
#include <mutex>
#include <iostream>

using namespace std;

mutex tr_lock;

MIDI_Runner::MIDI_Runner(MIDI_Reader m)
{
	m_reader = m;

	setTempo(120);
	calcTickTime();

	tick = 0;

	state = READY;
}

MIDI_Runner::MIDI_Runner(string MIDI_file)
{
	m_reader = MIDI_Reader(MIDI_file);

	setTempo(120);
	calcTickTime();

	tick = 0;

	state = READY;
}

// Set functions for tempo

void MIDI_Runner::setTempo(float t)
{
	tempo = t;
	tempo_us = 60000000.0 / t;
}

void MIDI_Runner::setTempoMicroseconds(unsigned int t_us)
{
	tempo = 60000000.0 / t_us;
	tempo_us = t_us;
}

void MIDI_Runner::calcTickTime()
{
	tick_us = tempo_us / m_reader.getDivTick();
}


void MIDI_Runner::run_track(Track_Chunk track)
{
	// Marker for when an event should occur
	unsigned int tick_marker = 0;

	// Used to store running status
	unsigned char running_status;
	
	tr_lock.lock();
	tracks_running++;
	tr_lock.unlock();

	// Wait until setup is done
	while (state == SETUP);

	for (int i = 0; i < track.getNumEvents(); i++)
	{
		// Get the ith event in the track
		MTrk_Event event = track.getEvent(i);

		// Use the delta time to determine when this event should occur
		tick_marker += event.getDeltaTime();

		// Wait until tick >= tick_marker
		while (tick < tick_marker);

		// Run the event
		run_event(event, running_status);
	}

	tr_lock.lock();
	tracks_running--;
	tr_lock.unlock();
}

void MIDI_Runner::run_event(MTrk_Event& event, unsigned char& rs)
{
	int index = 0;

	unsigned char status = event.getData(index);

	if (status < 0x80)
	{
		status = rs;
	}
	else
	{
		rs = status;
		index++;
	}

	if ((status & 0xF0) == 0b10000000) // Note off event
	{
		playNote(tick, status & 0x0F, event.getData(index), 0);
	}
	else if ((status & 0xF0) == 0b10010000) // Note on event
	{
		playNote(tick, status & 0x0F, event.getData(index), event.getData(index + 1));
	}
}

void MIDI_Runner::run()
{
	if (state != READY) return;

	state = SETUP;

	tracks_running = 0;

	tick = 0;

	for (int i = 0; i < m_reader.getNumTracks(); i++)
	{
		thread t(run_track_static, this, m_reader.getTrack(i));
		t.detach();
	}

	while (state == SETUP)
	{
		if (tracks_running == m_reader.getNumTracks()) state = RUNNING;
	}

	auto start = chrono::high_resolution_clock::now();
	auto elapsed = chrono::high_resolution_clock::now();
	long long time_us = chrono::duration_cast<chrono::microseconds>(elapsed - start).count();
	double time_marker = tick_us;

	while (state == RUNNING)
	{
		elapsed = chrono::high_resolution_clock::now();
		time_us = chrono::duration_cast<chrono::microseconds>(elapsed - start).count();
		while (time_us > time_marker)
		{
			tick++;
			time_marker += tick_us;
		}
		if (tracks_running <= 0) state = READY;
	}

}

void MIDI_Runner::run_track_static(MIDI_Runner* mr, Track_Chunk track)
{
	(*mr).run_track(track);
}

void MIDI_Runner::setPlayNoteFunction(void(*playNoteFunction)(unsigned int, int, int, int))
{
	playNote = playNoteFunction;
}