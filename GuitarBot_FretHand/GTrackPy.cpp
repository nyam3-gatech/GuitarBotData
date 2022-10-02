#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "GuitarTrack.h"

namespace py = pybind11;

std::string module_test()
{
    return "GBotData Module Loaded!";
}

#ifndef PCOMMAND
#define PCOMMAND

#define PC_DEFAULT_FRET 2

#define PC_OPEN 0
#define PC_PLAY 1
#define PC_DAMP 2

#endif

std::array<std::array<int, 6>, 2> genPlayCommand(ChordEvent& chord)
{
    std::array<std::array<int, 6>, 2> pCommand = {0};
    bool setFlag[6] = {false};

    unsigned int avgFret = 0;
    unsigned int nonOpenNotes = 0;

    for (NoteEvent n : chord.getNotes())
    {
        unsigned int index = n.getGuitarString() - 1;
        char fret = n.getFret();
        if (fret != 0)
        {
            pCommand[0][index] = n.getFret();
            pCommand[1][index] = PC_PLAY;
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
        if (pCommand[1][i] == 0) {
            if (!setFlag[i]) pCommand[1][i] = PC_DAMP;
            pCommand[0][i] = avgFret;
        }
    }

    return pCommand;
}

PYBIND11_MODULE(GBotData, m)
{
    m.doc() = "Test";

    m.def("module_test", &module_test);
    m.def("genPlayCommand", &genPlayCommand);

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
        .def("getNotes", &ChordEvent::getNotes)
        .def("getNumNotes", &ChordEvent::getNumNotes)
        .def("toString", &ChordEvent::toString);

    py::class_<GuitarTrack>(m, "GuitarTrack")
        .def(py::init<std::string>())
        .def("getChords", &GuitarTrack::getChords)
        .def("getChord", &GuitarTrack::getChord)
        .def("toString", &GuitarTrack::toString);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif

}