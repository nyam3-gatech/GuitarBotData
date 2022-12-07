import GBotData as gbd
import time

print(gbd.module_test())

gTrack = gbd.GuitarTrack("midis/Gil's Music 3 min.mid")
gCmdArray = gbd.genGuitarCmdArray(gTrack)

numCmds = len(gCmdArray)
cmdIndex = 0
setup_time = 1
time_stamp = -setup_time
t_init = time.time() + setup_time
songLength = gTrack.getTrackLength() + 1
endOfSong = t_init + songLength
while (cmdIndex < numCmds) and (time_stamp < songLength):
    gCmd = gCmdArray[cmdIndex]
    time_stamp = time.time() - t_init
    if (time_stamp) >= gCmd.time:
        print("t = " + str(time_stamp))
        print("  is_sound = " + str(gCmd.is_sound));
        if(gCmd.moveLH):
            # move LH
            print("  moveLH")
        if(gCmd.moveRH):
            # move RH
            print("  moveRH")
        cmdIndex += 1
        print("  tn = " + str(gCmdArray[cmdIndex].time));

