#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "GuitarTrack.h"
#include "GuitarCmd.h"

// build command for linux
// $ c++ -O3 -Wall -shared -std=c++14 -fPIC $(python3 -m pybind11 --includes) GBotDataCPP/* -o GBotData$(python3-config --extension-suffix)

using namespace std;
namespace py = pybind11;

std::string module_test()
{
    return "GBotData Module Loaded!";
}

PYBIND11_MODULE(GBotData, m)
{
    m.doc() = "Test";

    m.def("module_test", &module_test);

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
        .def("getNumChords", &GuitarTrack::getNumChords)
        .def("ticks_to_us", &GuitarTrack::ticks_to_us)
        .def("ticks_to_seconds", &GuitarTrack::ticks_to_seconds)
        .def("getTrackLength", &GuitarTrack::getTrackLength)
        .def("toString", &GuitarTrack::toString);

    py::class_<GuitarCmd>(m, "GuitarCmd")
        .def_readwrite("time", &GuitarCmd::time)
        .def_readwrite("duration", &GuitarCmd::duration)
        .def_readwrite("pos_i", &GuitarCmd::pos_i)
        .def_readwrite("pos_f", &GuitarCmd::pos_f)
        .def_readwrite("is_strum", &GuitarCmd::is_strum)
        .def_readwrite("is_sound", &GuitarCmd::is_sound)
        .def_readwrite("direction", &GuitarCmd::direction)
        .def_readwrite("velocity", &GuitarCmd::velocity)
        .def_readwrite("iplaycmd", &GuitarCmd::iplaycmd)
        .def_readwrite("ifretnum", &GuitarCmd::ifretnum)
        .def_readwrite("moveLH", &GuitarCmd::moveLH)
        .def_readwrite("moveRH", &GuitarCmd::moveRH)
        .def_readwrite("circularRH", &GuitarCmd::circularRH);

    m.def("genGuitarCmdArray", &genGuitarCmdArray);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}