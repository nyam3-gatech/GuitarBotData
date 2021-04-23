#include "GuitarEvent.h"
#include <algorithm>

#include <iostream>

using namespace std;

GuitarEvent::GuitarEvent(int tick_, char type_)
{
	tick = tick_;
	type = type_;
}

GuitarEvent::~GuitarEvent() {}

int GuitarEvent::getTick() const
{
	return tick;
}

char GuitarEvent::getType() const
{
	return type;
}

// TempoEvent

TempoEvent::TempoEvent(int tick_, int tempo_us_) : GuitarEvent(tick_, TEMPO)
{
	tempo_us = tempo_us_;
}

TempoEvent::~TempoEvent() {}

int TempoEvent::getTempoMicroseconds() const
{
	return tempo_us;
}

string TempoEvent::toString()
{
	return "Placeholder\n";
}

// NoteEvent

NoteEvent::NoteEvent(int tick_, int channel_, int note_, int velocity_) : GuitarEvent(tick_, NOTE)
{
	channel = channel_;
	note = note_;
	velocity = velocity_;
	endTick = 0;
	g_string = 0;
	fret = -1;
}

NoteEvent::~NoteEvent() {}

int NoteEvent::getChannel()
{
	return channel;
}
int NoteEvent::getDuration()
{
	return endTick - tick;
}
int NoteEvent::getEndTick()
{
	return endTick;
}
int NoteEvent::getNote()
{
	return note;
}
char NoteEvent::getGuitarString()
{
	return g_string;
}
char NoteEvent::getFret()
{
	return fret;
}

void NoteEvent::setDuration(int d)
{
	endTick = tick + d;
}
void NoteEvent::setEndTick(int t)
{
	endTick = t;
}
void NoteEvent::setGuitarString(int s)
{
	g_string = s;
}
void NoteEvent::setFret(unsigned char* tuning)
{
	if (g_string > 0 && g_string < 7) fret = note - tuning[g_string - 1];
}

vector<char> NoteEvent::getPossibleFrets(unsigned char* tuning)
{
	vector<char> frets;

	for (int i = 0; i < 6; i++)
	{
		char fret = note - tuning[i];
		if (fret > MAXFRET) fret = -1;
		else if (fret < 0) fret = -1;
		frets.push_back(fret);
	}

	return frets;
}

string NoteEvent::toString()
{
	string str = "<note :: tick = " + to_string(tick) + ", duration = " + to_string(getDuration());
	str += ", channel = " + to_string(channel) + ", note = " + to_string(note) + ", velocity = " + to_string(velocity);
	str += ", string = " + to_string(g_string) + ", fret = " + to_string(fret) + ">";
	return str;
}

// ChordEvent

ChordEvent::ChordEvent(int tick_, NoteEvent n) : GuitarEvent(tick_, CHORD)
{
	playable = 0;
	notes.push_back(n);
}

ChordEvent::~ChordEvent() {}

void ChordEvent::addNote(NoteEvent n)
{
	notes.push_back(n);
}

int ChordEvent::getDuration()
{
	int duration = 1;
	for (NoteEvent n : notes)
	{
		int temp = (n.getTick() - this->getTick()) + n.getDuration();
		if (temp > duration)
		{
			duration = temp;
		}
	}
	return duration;
}

unsigned char ChordEvent::getLowestFret()
{
	unsigned char lowest = MAXFRET + 1;

	for (NoteEvent n : notes)
	{
		if (n.getFret() != 0 && n.getFret() < lowest) lowest = n.getFret();
	}

	if (lowest == MAXFRET + 1) return 0;
	return lowest;
}

unsigned char ChordEvent::getHighestFret()
{
	unsigned char highest = 0;
	for (NoteEvent n : notes)
	{
		if (n.getFret() > highest) highest = n.getFret();
	}
	return highest;
}

bool compareByPitch(NoteEvent a, NoteEvent b)
{
	return a.getNote() < b.getNote();
}

void ChordEvent::sortByPitch()
{
	std::sort(notes.begin(), notes.end(), compareByPitch);
}

bool compareByTime(NoteEvent a, NoteEvent b)
{
	return a.getTick() < b.getTick();
}

void ChordEvent::sortByTime()
{
	std::sort(notes.begin(), notes.end(), compareByTime);
}

void ChordEvent::setFrets(unsigned char* tuning)
{
	sortByPitch();

	int g_string = 1;

	int note = notes[0].getNote();

	vector<vector<char>> possibleFrets;

	for (int i = 0; i < notes.size(); i++) possibleFrets.push_back(notes[i].getPossibleFrets(tuning));

	vector<char> buffer;
	vector<char> bestFit = getBestFit(0, buffer, possibleFrets);

	if (bestFit.size() > notes.size())
	{
		for (int i = 0; i < notes.size(); i++)
		{
			char strFret = bestFit[i];
			notes[i].setGuitarString(extractString(strFret) + 1);
			notes[i].setFret(tuning);
		}
		playable = 1;
	}
}

vector<char> ChordEvent::getBestFit(char note, vector<char> path, vector<vector<char>> pFrets)
{
	if (note >= pFrets.size())
	{
		char fitness = calculateFitness(path);
		path.push_back(fitness);
		return path;
	}

	vector<char>& current = pFrets[note];
	vector<char> result;
	char bestFitness = -1;

	for (int i = 0; i < 6; i++)
	{
		if (current[i] < 0) continue;

		bool flag = 0;
		for (int j = 0; j < path.size(); j++)
		{
			if (i == extractString(path[j])) flag = 1;
			else if (current[i]*extractFret(path[j]) != 0  && (current[i] - extractFret(path[j]) > FRET_RANGE - 1 || current[i] - extractFret(path[j]) < -(FRET_RANGE - 1))) flag = 1;
		}

		if (flag) continue;

		vector<char> copy = path;
		copy.push_back(condenseStringFret(i, current[i]));
		vector<char> temp = getBestFit(note + 1, copy, pFrets);

		if (temp.size() < 1) continue;
		else
		{
			char fitness = temp[temp.size() - 1];
			if (fitness > bestFitness)
			{
				result = temp;
				bestFitness = fitness;
			}
		}
	}

	return result;

}

char ChordEvent::calculateFitness(vector<char> frets)
{
	char max = -1;
	char min = MAXFRET + 1;
	int sum = 0;
	int countOpen = 0;

	for (int i = 0; i < frets.size(); i++)
	{
		char fret = extractFret(frets[i]);
		if (fret == 0) countOpen++;
		else
		{
			if (fret < min) min = fret;
			if (fret > max) max = fret;
		}
		sum += fret;
	}

	if (max - min > FRET_RANGE - 1) return -2;
	return 120 - sum;
}

// Fixes duration of duplicate notes when the same note should be played twice in the same chord
void ChordEvent::fixDuplicates()
{
	int duplicateIndex = -1;
	for (int i = 0; i < notes.size(); i++)
	{
		if (notes[i].getDuration() == 0) duplicateIndex = i;
	}

	if (duplicateIndex != -1)
	{
		for (int i = 0; i < notes.size(); i++)
		{
			if (i == duplicateIndex) continue;
			if (notes[i].getNote() == notes[duplicateIndex].getNote())
			{
				notes[duplicateIndex].setEndTick(notes[i].getEndTick());
				break;
			}
		}
	}
}

bool ChordEvent::checkConflict(unsigned char* tuning)
{
	for (NoteEvent n : notes)
	{
		if (n.getGuitarString() < 1 || n.getGuitarString() > 6) return 1;
		else if (n.getFret() < 0) return 1;
	}
	return 0; // No conflict
}

int ChordEvent::checkForNote(int n)
{
	int index = 0;
	do
	{
		if (notes[index].getNote() == n) return index;
		index++;
	} while (index < notes.size());
	return -1;
}

bool ChordEvent::removeNote(int n)
{
	int index = checkForNote(n);
	if (index == -1) return 0;
	notes.erase(notes.begin() + index);
}

vector<NoteEvent>& ChordEvent::getNotes()
{
	return notes;
}

char ChordEvent::condenseStringFret(char g_string, char fret)
{
	return (g_string << 5) + fret;
}
char ChordEvent::extractString(unsigned char gStrFret)
{
	return (gStrFret >> 5) & 0b00000111;
}
char ChordEvent::extractFret(unsigned char gStrFret)
{
	return gStrFret & 0x1F;
}

void ChordEvent::setDirection(char d)
{
	direction = d;
}

char ChordEvent::getTechnique()
{
	if (notes.size() == 1) return PICK;
	return STRUM;
}
char ChordEvent::getDirection()
{
	return direction;
}

char ChordEvent::getHighestString()
{
	char highest = 0;
	for (NoteEvent note : notes)
	{
		if (note.getGuitarString() > highest) highest = note.getGuitarString();
	}
	return highest;
}
char ChordEvent::getLowestString()
{
	char lowest = 7;
	for (NoteEvent note : notes)
	{
		if (note.getGuitarString() < lowest) lowest = note.getGuitarString();
	}
	return lowest;
}

void ChordEvent::setContactStrings()
{
	if (getDirection() == DOWN)
	{
		contact_string = getLowestString();
		final_contact_string = getHighestString();
	}
	else
	{
		contact_string = getHighestString();
		final_contact_string = getLowestString();
	}
}

char ChordEvent::getContactString()
{
	return contact_string;
}
char ChordEvent::getFinalContactString()
{
	return final_contact_string;
}
float ChordEvent::getPreparePosition()
{
	if (getTechnique() == PICK)
	{
		return contact_string + ((getDirection() == DOWN) ? PD_P : -PU_P);
	}
	else
	{
		return contact_string + ((getDirection() == DOWN) ? SD_P : -SU_P);
	}
}
float ChordEvent::getFinalPosition()
{
	if (getTechnique() == PICK)
	{
		return final_contact_string + ((getDirection() == DOWN) ? -PD_E : PU_E);
	}
	else
	{
		return final_contact_string + ((getDirection() == DOWN) ? -SD_E : SU_E);
	}
}

string ChordEvent::toString()
{
	string str = "chord :: tick = " + to_string(tick) + ", technique = " + (getTechnique() == STRUM ? "strum" : "pick");
	str += ", direction = " + (string) (direction == UP ? "up" : "down");
	str += ", cs = " + to_string(contact_string) + ", fcs = " + to_string(final_contact_string) + "\n";
	if (!playable) str += "   <NOT PLAYABLE - range restriction>\n";
	
	for (NoteEvent n : notes)
	{
		str += "   " + n.toString() + "\n";
	}
	return str;
	
}