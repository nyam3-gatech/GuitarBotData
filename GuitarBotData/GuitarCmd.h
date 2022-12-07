#ifndef GUITAR_CMD_H
#define GUITAR_CMD_H
#include "GuitarTrack.h"
#include <array>

#ifndef PCOMMAND
#define PCOMMAND
#define PC_DEFAULT_FRET 2
#define PC_IDLE 0
#define PC_OPEN 1
#define PC_PRESS 2
#define PC_DAMP 3
#define PC_HAMMERON 4
#define PC_SLIDE 5
#define LH_PREP_TIME 0.17
#endif

#ifndef RH_PARAM
#define RH_PARAM
#define RH_UP 1
#define RH_DOWN -1
#define RH_PREP_TIME 0.15
#endif

class GuitarCmd
{
public:
    float time;
    float duration;
    int pos_i, pos_f;
    bool is_strum, is_sound;
    int direction, velocity;
    std::array<int, 6> iplaycmd, ifretnum;
    bool moveLH, moveRH, circularRH;
    int pos_next;

    GuitarCmd(GuitarTrack& track, size_t index, bool is_sound, GuitarCmd* prev = 0);
    friend std::vector<GuitarCmd> genGuitarCmdArray(GuitarTrack& track);
private:
    void setTiming(GuitarTrack& track, size_t index, bool is_sound);
    void setLHCmd(ChordEvent& chord);
    void setRHCmd(GuitarTrack& track, size_t index, bool is_sound);
    void setRHCmd(bool sound = false, int posi = 0, int posf = 0, int vel = 0, int dir = 0);

    int getFirstPos(ChordEvent& chord);
    int getLastPos(ChordEvent& chord);
    int getChordVelocity(ChordEvent& chord);
    int getRHDirection(ChordEvent& chord);

    bool checkLHEqual(GuitarCmd& gCmd);
    bool checkCircular(GuitarCmd& gCmd);
};

std::vector<GuitarCmd> genGuitarCmdArray(GuitarTrack& track);

#endif