#ifndef GTAB
#define GTAB

#include "GuitarEvent.h"

#include <list>

#define MAX_FRET_RANGE 3

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
	std::vector<Fingering*> fingerings;

	friend class GTabNode;
	friend class GTab; // Allows GTab to access private variables

public:
	ChordFingerings();
	ChordFingerings(uint64_t, std::vector<Fingering*>&);

	Fingering getFingering(char);

	std::vector<Fingering*>& getFingerings();

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

	std::vector<ChordFingerings> possibleFingerings[6];

	std::list<std::vector<GTabNode*>> fingeringGraph;

	int searchForChord(int, uint64_t);

	ChordFingerings& getPossibleFingerings(ChordEvent*);
	void searchFingerings(std::vector<std::vector<char>>&, char, Fingering&, std::vector<Fingering*>&);
	void searchAllFingerings(std::vector<std::vector<char>>&, char, Fingering&, std::vector<Fingering*>&);

	int getIntraFitness(Fingering&);
	int getInterFitness(GTabNode*, GTabNode*);

public:

	GTab();
	GTab(unsigned char*);
	~GTab();
	
	void setFrets(std::vector<ChordEvent>&);
	void setFrets(std::vector<ChordEvent>&, int);
};

#endif