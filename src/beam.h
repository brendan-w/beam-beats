#pragma once

#include "ofxOpenCv.h" //ofxCvBlob


#define sizeof_array(a) (sizeof(a)/sizeof(a[0]))

const int midi_velocities[] = { 32, 64, 96, 128 };
const int midi_scale[] = { 0, 3, 5, 7, 10, 12 }; //pentatonic

class Note
{
    int note;
    int velocity;
};

class Beam
{
public:
    Beam(int beam, int octave);
    ~Beam();

    void read_blobs(vector<ofxCvBlob> blobs);
    vector<Note> get_midi();

private:
    const int beam;
    const int octave;

    //blobs from the previous frame, for velocity purposes
    vector<ofxCvBlob> old_blobs;
};
