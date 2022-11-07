import GBotData as gbd

print(gbd.module_test())

gTrack = gbd.GuitarTrack("midis/strings.mid")

for index, chord in enumerate(gTrack.getChords()):

    print("Chord Index:")
    print(index)
    print("LH Play Command:")
    print(gbd.genPlayCommand(chord))
    print("RH Move Command:")
    print(gbd.genRHMoveCommand(gTrack, index).toString())
    print("RH Play Command:")
    print(gbd.genRHPlayCommand(chord).toString())



