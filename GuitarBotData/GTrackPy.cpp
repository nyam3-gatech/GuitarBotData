#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "GuitarTrack.h"

// build command for linux
// $ c++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) GBotDataCPP/* -o GBotData$(python3-config --extension-suffix)

using namespace std;
namespace py = pybind11;

std::string module_test()
{
    return "GBotData Module Loaded!";
}

#ifndef PCOMMAND
#define PCOMMAND
#define PC_DEFAULT_FRET 2
#define PC_IDLE 0
#define PC_OPEN 1
#define PC_PRESS 2
#define PC_DAMP 3
#define PC_HAMMERON 4
#define PC_SLIDE 5
#endif
array<array<int, 6>, 2> genPlayCommand(ChordEvent& chord);

class RHCommand
{
public:
    bool isPlay;
    int pos_i, pos_f, velocity;
    RHCommand(bool ip = false, int pi = 0, int pf = 0, int v = 0)
    {
        isPlay = ip; pos_i = pi; pos_f = pf; velocity = v;
    }
    string toString()
    {
        return "{" + to_string(isPlay) + ", " + to_string(pos_i) + ", " + to_string(pos_f) + ", " + to_string(velocity) + "}";
    }
};
int getFirstPos(ChordEvent& chord);
int getLastPos(ChordEvent& chord);
int getChordVelocity(ChordEvent& chord);
RHCommand genRHMoveCommand(GuitarTrack& track, unsigned int index);
RHCommand genRHMoveCommand(ChordEvent& next, ChordEvent& last);
RHCommand genRHPlayCommand(GuitarTrack& track, unsigned int index);
RHCommand genRHPlayCommand(ChordEvent& chord);

PYBIND11_MODULE(GBotData, m)
{
    m.doc() = "Test";

    m.def("module_test", &module_test);
    m.def("genPlayCommand", &genPlayCommand);
    m.def("genRHMoveCommand", py::overload_cast<GuitarTrack&, unsigned int>(&genRHMoveCommand));
    m.def("genRHMoveCommand", py::overload_cast<ChordEvent&, ChordEvent&>(&genRHMoveCommand));
    m.def("genRHPlayCommand", py::overload_cast<GuitarTrack&, unsigned int>(&genRHPlayCommand));
    m.def("genRHPlayCommand", py::overload_cast<ChordEvent&>(&genRHPlayCommand));

    py::class_<NoteEvent>(m, "NoteEvent")
        .def(py::init<int, int, int, int>())
        .def_property("tick", &NoteEvent::getTick, &NoteEvent::setTick)
        .def_property("endTick", &NoteEvent::getEndTick, &NoteEvent::setEndTick)
        .def_property("duration", &NoteEvent::getDuration, &NoteEvent::setDuration)
        .def_property("channel", &NoteEvent::getChannel, &NoteEvent::setChannel)
        .def_property("note", &NoteEvent::getNote, &NoteEvent::setNote)
        .def_property("velocity", &NoteEvent::getVelocity, &NoteEvent::setVelocity)
        .def("getType", &NoteEvent::getType)
        .def("toString", &NoteEvent::toString);

    py::class_<ChordEvent>(m, "ChordEvent")
        .def(py::init<int, NoteEvent>())
        .def_property("tick", &ChordEvent::getTick, &ChordEvent::setTick)
        .def("getDirection", &ChordEvent::getDirection)
        .def("getTechniqueClass", &ChordEvent::getTechniqueClass)
        .def("getHighestString", &ChordEvent::getHighestString)
        .def("getLowestString", &ChordEvent::getLowestString)
        .def("getInitialContactString", &ChordEvent::getContactString)
        .def("getFinalContactString", &ChordEvent::getFinalContactString)
        .def("getNotes", &ChordEvent::getNotes, py::return_value_policy::reference_internal)
        .def("getNumNotes", &ChordEvent::getNumNotes)
        .def("toString", &ChordEvent::toString);

    py::class_<GuitarTrack>(m, "GuitarTrack")
        .def(py::init<std::string>())
        .def("getChords", &GuitarTrack::getChords, py::return_value_policy::reference_internal)
        .def("getChord", &GuitarTrack::getChord, py::return_value_policy::reference_internal)
        .def("ticks_to_us", &GuitarTrack::ticks_to_us)
        .def("toString", &GuitarTrack::toString);

    py::class_<RHCommand>(m, "RHCommand")
        .def_readwrite("isPlay", &RHCommand::isPlay)
        .def_readwrite("pos_i", &RHCommand::pos_i)
        .def_readwrite("pos_f", &RHCommand::pos_f)
        .def_readwrite("velocity", &RHCommand::velocity)
        .def("toString", &RHCommand::toString);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif

}

array<array<int, 6>, 2> genPlayCommand(ChordEvent& chord)
{
    array<array<int, 6>, 2> pCommand = { 0 };
    bool setFlag[6] = { false };

    unsigned int avgFret = 0;
    unsigned int nonOpenNotes = 0;

    for (NoteEvent n : chord.getNotes())
    {
        unsigned int index = n.getGuitarString() - 1;
        char fret = n.getFret();
        if (fret != 0)
        {
            pCommand[0][index] = n.getFret();
            pCommand[1][index] = PC_PRESS;
            setFlag[index] = true;
            avgFret += n.getFret();
            nonOpenNotes++;
        }
        else
        {
            pCommand[1][index] = PC_OPEN;
            setFlag[index] = true;
        }
    }

    if (nonOpenNotes != 0) avgFret /= nonOpenNotes;
    else avgFret = PC_DEFAULT_FRET;

    for (int i = 0; i < 6; i++)
    {
        if (pCommand[1][i] == 0 || pCommand[1][i] == PC_OPEN) {
            if (!setFlag[i]) pCommand[1][i] = PC_DAMP;
            pCommand[0][i] = avgFret;
        }
    }

    return pCommand;
}

int getFirstPos(ChordEvent& chord)
{
    int gstring = (chord.getDirection() == UP) ? chord.getLowestString() : chord.getHighestString();
    if (chord.getTechniqueClass() == TC_PICK) return gstring + 6;
    return gstring + ((chord.getDirection() == UP) ? -1 : 0);


}
int getLastPos(ChordEvent& chord)
{
    int gstring = (chord.getDirection() == UP) ? chord.getHighestString() : chord.getLowestString();
    if (chord.getTechniqueClass() == TC_PICK) return gstring + 6;
    return gstring + ((chord.getDirection() == UP) ? 0 : -1);
}
int getChordVelocity(ChordEvent& chord)
{
    return chord.getNotes()[0].getVelocity();
}
RHCommand genRHMoveCommand(GuitarTrack& track, unsigned int index)
{
    if (index == 0)
    {
        return RHCommand(false, -1, getFirstPos(track.getChord(index)));
    }
    else return genRHMoveCommand(track.getChord(index),track.getChord(index-1));
}
RHCommand genRHMoveCommand(ChordEvent& next, ChordEvent& last)
{
    return RHCommand(false, getLastPos(last), getFirstPos(next));
}
RHCommand genRHPlayCommand(GuitarTrack& track, unsigned int index)
{
    return genRHPlayCommand(track.getChord(index));
}
RHCommand genRHPlayCommand(ChordEvent& chord)
{
    return RHCommand(false, getFirstPos(chord), getLastPos(chord), getChordVelocity(chord));
}