#ifndef SPEAKER_H
#define SPEAKER_H

#include "gui.h"

//REMEMBER:
    //IF USING SPEAKER, ALWAYS TURN OFF MIC FIRST.
        //M5.Mic.end()
        //M5.Speaker.begin()
    
    //WHEN LEAVING FUNCTION:
    //  do {
//             delay(1);
//             M5.update();
//         } while (M5.Speaker.isPlaying());
        //M5.Speaker.end()
        //M5.Mic.begin()

    //This will pause ALL operations until speaker finishes.


    void play_recording(char* buffer, size_t bufferSize);


#endif