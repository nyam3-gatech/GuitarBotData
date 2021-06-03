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

	for (unsigned int i = 0; i < chord->getNumNotes(); i++)
	{
		chord->getNotes()[i].setGuitarString(strings[i]);
		chord->getNotes()[i].setFret(frets[i]);
	}

	chord->setPlayable();
}

ChordFingerings::ChordFingerings()
{
	notes = 0;
}

ChordFingerings::ChordFingerings(uint64_t notes_, vector<Fingering*>& fingerings_)
{
	notes = notes_;
	fingerings_.shrink_to_fit();
	fingerings = fingerings_;
}

Fingering ChordFingerings::getFingering(char n)
{
	return *(fingerings[n]);
}

vector<Fingering*>& ChordFingerings::getFingerings()
{
	return fingerings;
}

bool ChordFingerings::isMatching(uint64_t chord)
{
	return (notes == chord);
}

GTabNode::GTabNode()
{
	previous = 0;
	f = 0;
	chord = 0;
	score = 0x80000000; // Minimum value of signed 32-bit int in two's complement
}

GTabNode::GTabNode(Fingering* f_, ChordEvent* chord_)
{
	previous = 0;
	f = f_;
	chord = chord_;
	score = 0x80000000; // Minimum value of signed 32-bit int in two's complement
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

GTab::~GTab()
{
	for (vector<GTabNode*> v : fingeringGraph)
	{
		for (GTabNode* nodeptr : v) delete nodeptr;
	}
	for (int i = 0; i < 6; i++)
	{
		for (ChordFingerings& c : possibleFingerings[i])
		{
			for (Fingering* fptr : c.getFingerings()) delete fptr;
		}
	}
}

int GTab::searchForChord(int numNotes, uint64_t chord)
{
	vector<ChordFingerings>& list = possibleFingerings[numNotes - 1];

	for (unsigned int i = 0; i < list.size(); i++)
	{
		if (list[i].isMatching(chord)) return i;
	}
	return -1;
}

ChordFingerings& GTab::getPossibleFingerings(ChordEvent* chord)
{
	unsigned char numNotes = chord->getNumNotes();
	uint64_t chordNotes = chord->notesToLong();

	vector<ChordFingerings>& list = possibleFingerings[numNotes - 1];

	int index = searchForChord(numNotes, chordNotes);
	if (index != -1) return list[index];

	vector<vector<char>> possibleFrets;
	vector<Fingering*> fingerings;
	bool hammerFlag = 0;
	Fingering buffer;

	for (int i = 0; i < numNotes; i++) possibleFrets.push_back(chord->getNotes()[i].getPossibleFrets(tuning));

	if (numNotes == 2)
	{
		searchAllFingerings(possibleFrets, 2, buffer, fingerings);
	}
	else
	{
		searchFingerings(possibleFrets, numNotes, buffer, fingerings);
	}

	list.push_back(ChordFingerings(chordNotes, fingerings));
	return list[list.size() - 1];
}

void GTab::searchFingerings(vector<vector<char>>& possibleFrets, char n, Fingering& buffer, vector<Fingering*>& fingerings)
{
#define SCORE_CUTOFF -1000

	if (n == 0)
	{
		getIntraFitness(buffer);
		if (buffer.intrafit > SCORE_CUTOFF) fingerings.push_back(new Fingering(buffer));
	}
	else
	{
		int noteIndex = possibleFrets.size() - n;

		for (unsigned int i = 0; i < possibleFrets[noteIndex].size(); i++)
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

void GTab::searchAllFingerings(vector<vector<char>>& possibleFrets, char n, Fingering& buffer, vector<Fingering*>& fingerings)
{
	if (n == 0)
	{
		getIntraFitness(buffer);
		if (buffer.intrafit > SCORE_CUTOFF) fingerings.push_back(new Fingering(buffer));
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
				if ((buffer.strings[j] == (i + 1)) && (buffer.frets[j] == fret))
				{
					duplicate = 1;
					break;
				}
			}
			if (duplicate) continue;

			buffer.strings[noteIndex] = i + 1;
			buffer.frets[noteIndex] = fret;

			searchAllFingerings(possibleFrets, n - 1, buffer, fingerings);
		}
	}
}

int GTab::getIntraFitness(Fingering& f)
{
	int fitness = 0; // base fitness

	int nonOpenCount = 0;
	int fretSum = 0;
	int stringSum = 0;

	char minFret = 25;
	char maxFret = -1;

	for (int n = 0; n < 6; n++)
	{
		if (f.strings[n] == 0)
		{
			break;
		}

		stringSum += f.strings[n];

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

	//fitness -= 5 * log(stringSum + 1);

	f.intrafit = fitness;

	return fitness;
}
int GTab::getInterFitness(GTabNode* node1, GTabNode* node2)
{
	Fingering& f1 = *(node1->f);
	Fingering& f2 = *(node2->f);

	int fitness = 0;

	int nonOpenCount1 = 0;
	int fretSum1 = 0;
	int nonOpenCount2 = 0;
	int fretSum2 = 0;

	for (int i = 0; i < 6; i++)
	{
		if (f1.strings[i] != 0)
		{
			char fret1 = f1.frets[i];

			if (fret1)
			{
				nonOpenCount1++;
				fretSum1 += fret1;
			}
		}

		if (f2.strings[i] != 0)
		{
			char fret2 = f2.frets[i];

			if (fret2)
			{
				nonOpenCount2++;
				fretSum2 += fret2;
			}
		}
	}

	if (nonOpenCount2)
	{
		if (nonOpenCount1)
		{
			int averageFretDiff = (fretSum1 / nonOpenCount1) + (fretSum2 / nonOpenCount2);
			fitness -= 10 * (averageFretDiff * averageFretDiff);
		}
		else if(node1->previous)
		{
			Fingering& fprev = *(node1->previous->f);

			for (int i = 0; i < 6; i++)
			{
				if (fprev.strings[i] == 0) break;

				char fret1 = fprev.frets[i];

				if (fret1)
				{
					nonOpenCount1++;
					fretSum1 += fret1;
				}
			}

			if (nonOpenCount1)
			{
				int averageFretDiff = (fretSum1 / nonOpenCount1) + (fretSum2 / nonOpenCount2);
				fitness -= 8 * (averageFretDiff * averageFretDiff);
			}
		}
	}

	return fitness;
}

void GTab::setFrets(vector<ChordEvent*>& chords)
{
	bool first = fingeringGraph.size() == 0 ? 1 : 0;

	for (int i = 0; i < chords.size(); i++)
	{
		ChordEvent* chord = chords[i];

		vector<Fingering*>& fingerings = getPossibleFingerings(chord).getFingerings();

		if (fingerings.size() == 0) continue;

		vector<GTabNode*> nodes;
		nodes.reserve(fingerings.size());

		if (first)
		{
			for (Fingering* f : fingerings)
			{
				GTabNode* node = new GTabNode(f, chord);
				node->score = f->intrafit;
				nodes.push_back(node);
			}

			first = 0;
		}
		else
		{
			for (Fingering* f : fingerings)
			{
				GTabNode* node = new GTabNode(f, chord);

				vector<GTabNode*>& priorNodes = fingeringGraph.back();

				for (int j = 0; j < priorNodes.size(); j++)
				{
					GTabNode* prior = priorNodes[j];

					int tempScore = prior->score + getInterFitness(prior, node);

					if (tempScore > node->score)
					{
						node->previous = prior;
						node->score = tempScore;
					}
				}

				node->score += f->intrafit;

				nodes.push_back(node);
			}
		}

		fingeringGraph.push_back(nodes);
	}

	vector<GTabNode*>& endNodes = fingeringGraph.back();
	GTabNode* nodeptr = endNodes[0];
	for (int i = 1; i < endNodes.size(); i++)
	{
		if (endNodes[i]->score > nodeptr->score)
		{
			nodeptr = endNodes[i];
		}
	}

	while (nodeptr)
	{
		(nodeptr->f)->assignFingering(nodeptr->chord);
		nodeptr = nodeptr->previous;
	}
}