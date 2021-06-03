#ifndef GTAB
#define GTAB

#include "GuitarEvent.h"

#include <list>

#define MAX_FRET_RANGE 3

using namespace std;

class Fingering
{
private:
	char strings[6];
	char frets[6];
	int intrafit;

	friend class ChordFingerings; // Allows ChordFingerings to access private variables
	friend class GTabNode;
	friend class GTab; // Allows GTab to access private variables

public:
	Fingering();
	Fingering(const Fingering&);
	Fingering& operator=(const Fingering&);

	void assignFingering(ChordEvent*);
};

class ChordFingerings
{
private:
	uint64_t notes;
	vector<Fingering*> fingerings;

	friend class GTabNode;
	friend class GTab; // Allows GTab to access private variables

public:
	ChordFingerings();
	ChordFingerings(uint64_t, vector<Fingering*>&);

	Fingering getFingering(char);

	vector<Fingering*>& getFingerings();

	bool isMatching(uint64_t);
};

class GTabNode
{
private:
	GTabNode* previous;
	Fingering* f;
	ChordEvent* chord;
	int score;

	friend class GTab;

	GTabNode();
	GTabNode(Fingering*, ChordEvent*);
};

class GTab
{
private:

	unsigned char tuning[6];

	vector<ChordFingerings> possibleFingerings[6];

	list<vector<GTabNode*>> fingeringGraph;

	int searchForChord(int, uint64_t);

	ChordFingerings& getPossibleFingerings(ChordEvent*);
	void searchFingerings(vector<vector<char>>&, char, Fingering&, vector<Fingering*>&);
	void searchAllFingerings(vector<vector<char>>&, char, Fingering&, vector<Fingering*>&);

	int getIntraFitness(Fingering&);
	int getInterFitness(GTabNode*, GTabNode*);

public:

	GTab();
	GTab(unsigned char*);
	~GTab();
	
	void setFrets(vector<ChordEvent*>&);
};

#endif