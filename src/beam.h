
#pragma once

#include "ofxOpenCv.h" //ofxCvBlob
#include "ofxMidi.h"
#include "settings.h"
#include "hand.h"


class BeamRegion
{
public:
    bool status = false;
    Hand hand;
};


class Beam
{
public:
    Beam(int channel, int base_note, int color);
    void update(vector<Hand> hands, ofxMidiOut& midi_out);
    void draw_bg();
    void draw(vector<Hand> hands);

private:
    const int channel;
    const int base_note;
    const int color;

    //table of region statuses
    BeamRegion previous_regions[SCALE_SIZE];
    int previous_bend;

    size_t hand_to_region(Hand& hand);
    int region_to_note(size_t region);
    size_t area_to_velocity(float area);
};
