output: Main.o MIDI_Reader.o GuitarEvent.o GTab.o GuitarTrack.o serialib.o SerialInterface.o
	g++ Main.o MIDI_Reader.o GuitarEvent.o GTab.o GuitarTrack.o serilib.o SerialInterface.o -o output

Main.o: Main.cpp
	g++ -c Main.cpp

MIDI_Reader.o: MIDI_Reader.cpp MIDI_Reader.h
	g++ -c MIDI_Reader.cpp

GuitarEvent.o: GuitarEvent.cpp GuitarEvent.h
	g++ -c GuitarEvent.cpp

GTab.o: GTab.cpp GTab.h
	g++ -c GTab.cpp

GuitarTrack.o: GuitarTrack.cpp GuitarTrack.h
	g++ -c GuitarTrack.cpp

serialib.o: serialib.cpp serialib.h
	g++ -c serialib.cpp

SerialInterface.o: SerialInterface.cpp SerialInterface.h Orders.h
	g++ -c SerialInterface.cpp

clean:
	rm *.o output