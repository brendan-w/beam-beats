
#pragma once

#include "ofxOpenCv.h" //ofxCvBlob
#include "settings.h"
#include "hand.h"

#define sizeof_array(a) (sizeof(a)/sizeof(a[0]))

const int midi_velocities[] = { 32, 64, 96, 128 };
const int midi_scale[] = { 0, 3, 5, 7, 10, 12 }; //pentatonic

class Note
{
    int note;
    int channel;
    int velocity;
};

class Beam
{
public:
    Beam(int channel, int octave);
    vector<Note> update(vector<Hand> hands);

private:
    const int channel;
    const int octave;

    //blobs from the previous frame, for velocity purposes
    bool notes[sizeof_array(midi_scale)];
};
