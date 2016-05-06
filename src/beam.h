
#pragma once

#include "ofxOpenCv.h" //ofxCvBlob
#include "ofxMidi.h"
#include "settings.h"
#include "hand.h"


class Twang
{
public:
    Twang(int dir); //pass 1 or -1
    void draw(bool invert);
    bool done;

private:
    int direction;
    int frame;
};


class BeamRegion
{
public:
    bool status = false;
    Hand hand;
};


class Beam
{
public:
    Beam(int channel, int base_note, int color, bool invert);
    void update(vector<Hand> hands, ofxMidiOut& midi_out);
    void draw_bg();
    void draw(vector<Hand> hands);

private:
    const int channel;
    const int base_note;
    const int color;
    const bool invert;

    //table of region statuses
    BeamRegion previous_regions[SCALE_SIZE];
    int previous_bend;
    int previous_num_hands;

    list<Twang> twangs;

    size_t hand_to_region(Hand& hand);
    int region_to_note(size_t region);
    size_t speed_to_midi_velocity(float speed);
};
