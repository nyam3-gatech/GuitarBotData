#include "GuitarCmd.h"

using namespace std;

GuitarCmd::GuitarCmd(GuitarTrack& track, size_t index, bool is_sound, GuitarCmd* prev)
{
    this->setTiming(track, index, is_sound);
    this->setLHCmd(track.getChord(index));
    this->setRHCmd(track, index, is_sound);
    if (prev)
    {
        this->moveLH = !checkLHEqual(*prev);
        prev->circularRH = prev->is_sound && checkCircular(*prev);
        this->moveRH = is_sound || (moveRH && !(prev->circularRH));
    }
}

// GuitarCmd Constructor Helper Functions

inline void GuitarCmd::setTiming(GuitarTrack& track, size_t index, bool is_sound)
{
    ChordEvent& chord = track.getChord(index);
    if (is_sound)
    {
        time = track.ticks_to_seconds(chord.getTick());
        duration = track.ticks_to_seconds(chord.getDuration());
    }
    else
    {
        time = track.ticks_to_seconds(chord.getTick()) - LH_PREP_TIME;
        duration = LH_PREP_TIME;
    }
}

inline void GuitarCmd::setLHCmd(ChordEvent& chord)
{
    bool setFlag[6] = { false };

    unsigned int avgFret = 0;
    unsigned int nonOpenNotes = 0;

    for (NoteEvent n : chord.getNotes())
    {
        unsigned int index = n.getGuitarString() - 1;
        char fret = n.getFret();
        if (fret != 0)
        {
            ifretnum[index] = n.getFret();
            iplaycmd[index] = PC_PRESS;
            setFlag[index] = true;
            avgFret += n.getFret();
            nonOpenNotes++;
        }
        else
        {
            iplaycmd[index] = PC_OPEN;
            setFlag[index] = true;
        }
    }

    if (nonOpenNotes != 0) avgFret /= nonOpenNotes;
    else avgFret = PC_DEFAULT_FRET;

    for (int i = 0; i < 6; i++)
    {
        if (iplaycmd[i] == 0 || iplaycmd[i] == PC_OPEN) {
            if (!setFlag[i]) iplaycmd[i] = PC_DAMP;
            ifretnum[i] = avgFret;
        }
    }

    moveLH = true;
}

inline void GuitarCmd::setRHCmd(GuitarTrack& track, size_t index, bool is_sound)
{
    ChordEvent& chord = track.getChord(index);
    if (!is_sound) setRHCmd(false, index == 0 ? 0 : getLastPos(track.getChord(index-1)), getFirstPos(chord));
    else setRHCmd(true, getFirstPos(chord), getLastPos(chord), getChordVelocity(chord), getRHDirection(chord));
}
inline void GuitarCmd::setRHCmd(bool sound, int posi, int posf, int vel, int dir)
{
    is_sound = sound; pos_i = posi; pos_f = posf; velocity = vel;
    is_strum = posf < 7;
    direction = (posi < posf) ? RH_DOWN : (posi > posf) ? RH_UP : dir;
    moveRH = sound || (posi != posf);
}

// GuitarCmd helper functions

int GuitarCmd::getFirstPos(ChordEvent& chord)
{
    int gstring = (chord.getDirection() == DOWN) ? chord.getLowestString() : chord.getHighestString();
    if (chord.getTechniqueClass() == TC_PICK) return gstring + 6;
    return gstring + ((chord.getDirection() == DOWN) ? -1 : 0);
}
int GuitarCmd::getLastPos(ChordEvent& chord)
{
    int gstring = (chord.getDirection() == DOWN) ? chord.getHighestString() : chord.getLowestString();
    if (chord.getTechniqueClass() == TC_PICK) return gstring + 6;
    return gstring + ((chord.getDirection() == DOWN) ? 0 : -1);
}
inline int GuitarCmd::getChordVelocity(ChordEvent& chord)
{
    return chord.getNotes()[0].getVelocity();
}
inline int GuitarCmd::getRHDirection(ChordEvent& chord)
{
    return chord.getDirection() == UP ? RH_UP : RH_DOWN;
}

bool inline GuitarCmd::checkLHEqual(GuitarCmd& gCmd)
{
    return (ifretnum == gCmd.ifretnum) && (iplaycmd == gCmd.iplaycmd);
}
bool inline GuitarCmd::checkCircular(GuitarCmd& gCmd)
{
    return is_strum && gCmd.is_strum && (direction != gCmd.direction);
}

vector<GuitarCmd> genGuitarCmdArray(GuitarTrack& track)
{
    const size_t N = track.getNumChords();
    vector<GuitarCmd> gCmdArray;
    gCmdArray.reserve(2*N);
    GuitarCmd* prevCmd = 0;
    for (size_t i = 0; i < N; i++)
    {
        gCmdArray.push_back(GuitarCmd(track, i, false, prevCmd));
        prevCmd = &gCmdArray.back();
        gCmdArray.push_back(GuitarCmd(track, i, true, prevCmd));
        prevCmd = &gCmdArray.back();
    }
    return gCmdArray;
}