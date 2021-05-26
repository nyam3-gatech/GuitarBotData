#include "GTab.h"

Fingering::Fingering()
{
	for (int i = 0; i < 6; i++)
	{
		this->strings[i] = 0;
		this->frets[i] = 0;
	}
	intrafit = 0;
}

Fingering::Fingering(const Fingering& f)
{
	for (int i = 0; i < 6; i++)
	{
		this->strings[i] = f.strings[i];
		this->frets[i] = f.frets[i];
	}
	this->intrafit = f.intrafit;
}

Fingering& Fingering::operator=(const Fingering& f)
{
	for (int i = 0; i < 6; i++)
	{
		this->strings[i] = f.strings[i];
		this->frets[i] = f.frets[i];
	}
	this->intrafit = f.intrafit;
	return *this;
}

void Fingering::assignFingering(ChordEvent* chord)
{
	chord->sortByPitch();

	for (int i = 0; i < chord->getNumNotes(); i++)
	{
		chord->getNotes()[i].setGuitarString(strings[i]);
		chord->getNotes()[i].setFret(frets[i]);
	}

	chord->setPlayable();
}

ChordFingerings::ChordFingerings(uint64_t notes_, vector<Fingering>& fingerings_)
{
	notes = notes_;
	fingerings_.shrink_to_fit();
	fingerings = fingerings_;
}

Fingering ChordFingerings::getFingering(char n)
{
	return fingerings[n];
}

vector<Fingering>& ChordFingerings::getFingerings()
{
	return fingerings;
}

bool ChordFingerings::isMatching(uint64_t chord)
{
	return (notes == chord);
}

GTabNode::GTabNode(Fingering* f_, ChordEvent* chord_)
{
	previous = 0;
	f = f_;
	chord = chord_;
	score = 0;
}

GTab::GTab()
{
	tuning[0] = 40; // E
	tuning[1] = 45; // A
	tuning[2] = 50; // D
	tuning[3] = 55; // G
	tuning[4] = 59; // B
	tuning[5] = 64; // E
}

GTab::GTab(unsigned char* tuning_)
{
	for (int i = 0; i < 6; i++)
	{
		tuning[i] = tuning_[i];
	}
}

int GTab::searchForChord(int numNotes, uint64_t chord)
{
	vector<ChordFingerings>& list = possibleFingerings[numNotes];

	for (int i = 0; i < list.size(); i++)
	{
		if ((list)[i].isMatching(chord)) return i;
	}
	return -1;
}

ChordFingerings& GTab::getPossibleFingerings(ChordEvent& chord)
{
	unsigned char numNotes = chord.getNumNotes();
	uint64_t chordNotes = chord.notesToLong();

	int index = searchForChord(numNotes, chordNotes);
	if (index != -1) return possibleFingerings[numNotes][index];

	vector<vector<char>> possibleFrets;
	vector<Fingering> fingerings;
	bool hammerFlag = 0;
	Fingering buffer;

	for (int i = 0; i < numNotes; i++) possibleFrets.push_back(chord.getNotes()[i].getPossibleFrets(tuning));

	if (numNotes == 2)
	{
		searchAllFingerings(possibleFrets, 2, buffer, fingerings);
	}
	else
	{
		searchFingerings(possibleFrets, numNotes, buffer, fingerings);
	}

	possibleFingerings[numNotes].push_back(ChordFingerings(chordNotes, fingerings));
	return possibleFingerings[numNotes][possibleFingerings[numNotes].size() - 1];
}

void GTab::searchFingerings(vector<vector<char>>& possibleFrets, char n, Fingering& buffer, vector<Fingering>& fingerings)
{
#define SCORE_CUTOFF -1000

	if (n == 0)
	{
		getIntraFitness(buffer);
		if (buffer.intrafit > SCORE_CUTOFF) fingerings.push_back(buffer);
	}
	else
	{
		int noteIndex = possibleFrets.size() - n;

		for (int i = 0; i < possibleFrets[noteIndex].size(); i++)
		{
			char fret = possibleFrets[noteIndex][i];

			if (fret < 0) continue;
			bool sameString = 0;
			for (int j = 0; j < noteIndex; j++)
			{
				if (buffer.strings[j] == i + 1)
				{
					sameString = 1;
					break;
				}
			}
			if (sameString) continue;

			buffer.strings[noteIndex] = i + 1;
			buffer.frets[noteIndex] = fret;

			searchFingerings(possibleFrets, n - 1, buffer, fingerings);
		}
	}
}

void GTab::searchAllFingerings(vector<vector<char>>& possibleFrets, char n, Fingering& buffer, vector<Fingering>& fingerings)
{
	if (n == 0)
	{
		getIntraFitness(buffer);
		fingerings.push_back(buffer);
	}
	else
	{
		int noteIndex = possibleFrets.size() - n;

		for (int i = 0; i < possibleFrets[noteIndex].size(); i++)
		{
			char fret = possibleFrets[noteIndex][i];

			if (fret < 0) continue;
			bool duplicate = 0;
			for (int j = 0; j < noteIndex; j++)
			{
				if (buffer.strings[j] == i + 1 && buffer.frets[j] == fret)
				{
					duplicate = 1;
					break;
				}
			}
			if (duplicate) continue;

			buffer.strings[noteIndex] = i + 1;
			buffer.frets[noteIndex] = fret;

			searchFingerings(possibleFrets, n - 1, buffer, fingerings);
		}
	}
}

int GTab::getIntraFitness(Fingering& f)
{
	int fitness = 500; // base fitness

	int nonOpenCount = 0;
	int fretSum = 0;

	char minFret = 25;
	char maxFret = -1;

	for (int n = 0; n < 6; n++)
	{
		if (f.strings[n] == 0)
		{
			break;
		}

		for (int i = 0; i < n; i++)
		{
			if (f.strings[i] == f.strings[n]) fitness -= 50;
		}

		char fret = f.frets[n];

		if (fret == 0)
		{
			fitness += 5;
		}
		else
		{
			nonOpenCount++;
			fretSum += fret;

			if (fret < minFret) minFret = fret;
			if (fret > maxFret) maxFret = fret;
		}
	}

	if(nonOpenCount) fitness -= 3 * (fretSum / nonOpenCount); // Lower average fret is better

	if (maxFret != 25)
	{
		if(maxFret - minFret > MAX_FRET_RANGE - 1) fitness -= 3000; // Cannot be played
	}

	f.intrafit = fitness;

	return fitness;
}
int GTab::getInterFitness(Fingering& f1, Fingering& f2)
{
	return 0;
}

void GTab::setFrets(vector<GuitarEvent*>& track)
{
	vector<vector<GTabNode>> fingeringGraph;
	int graphIndex = 0;

	for (int i = 0; i < track.size(); i++)
	{
		GuitarEvent* gEvent = track[i];

		if (gEvent->getType() != CHORD) continue;

		ChordEvent* chord = (ChordEvent*) gEvent;

		ChordFingerings& chordFingerings = getPossibleFingerings(*chord);

		if (chordFingerings.getFingerings().size() == 0) continue;

		if (graphIndex == 0)
		{
			vector<GTabNode> nodes;
			for (Fingering& f : chordFingerings.getFingerings())
			{
				GTabNode node = GTabNode(&f, chord);
				node.score = f.intrafit;
				nodes.push_back(node);
			}

			fingeringGraph.push_back(nodes);
		}
		else
		{
			vector<GTabNode> nodes;
			for (Fingering& f : chordFingerings.getFingerings())
			{
				GTabNode node = GTabNode(&f, chord);

				int priorIndex = graphIndex - 1;

				for (int j = 0; j < fingeringGraph[priorIndex].size(); j++)
				{
					GTabNode& prior = fingeringGraph[priorIndex][j];

					int tempScore = prior.score + getInterFitness(*prior.f, f);

					if (tempScore > node.score)
					{
						node.previous = &prior;
						node.score = tempScore;
					}
				}

				node.score += f.intrafit;

				nodes.push_back(node);
			}

			fingeringGraph.push_back(nodes);
		}

		graphIndex++;
	}

	graphIndex--;

	GTabNode* nodeptr = &fingeringGraph[graphIndex][0];
	for (int i = 1; i < fingeringGraph[graphIndex].size(); i++)
	{
		if (fingeringGraph[graphIndex][i].score > nodeptr->score)
		{
			nodeptr = &fingeringGraph[graphIndex][i];
		}
	}

	while (nodeptr)
	{
		(nodeptr->f)->assignFingering(nodeptr->chord);
		nodeptr = nodeptr->previous;
	}
}