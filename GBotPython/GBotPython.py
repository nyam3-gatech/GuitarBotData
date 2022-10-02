import GBotData as gbd

print(gbd.module_test())

gTrack = gbd.GuitarTrack("midis/strings.mid")

for chord in gTrack.getChords():

    print("Play Command: ")
    print(gbd.genPlayCommand(chord))



