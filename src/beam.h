
#pragma once

#include "ofxOpenCv.h" //ofxCvBlob
#include "ofxMidi.h"
#include "settings.h"
#include "hand.h"

/*
 * Simple class for keeping track of animations that persist after a hand
 * has left the beam. Produces the appearance of a vibrating string upon
 * "release" of a note.
 */

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


/*
 * BeamRegions are the segments of our beams that trigger different notes.
 * These are used for keeping track of the dominant hands in each region.
 * It's neccessary that we keep this status, because MIDI is change-based.
 */

class BeamRegion
{
public:
    bool status = false;
    Hand hand; //the dominant hand in a note/beam region
};


/*
 * Class representing the musical and visual properties of a beam. Performs
 * hand-to-note conversions, and generates hand-tracked graphics in draw()
 */

class Beam
{
public:
    Beam(int channel, int base_note, int color, bool invert);
    void update(vector<Hand> hands, ofxMidiOut& midi_out);
    void draw_bg();
    void draw(vector<Hand> hands);

private:
    const int channel;   //MIDI channel that this beam operates on
    const int base_note; //The lowest MIDI note for this beam's scale
    const int color; //the background color for drawing this beam
    const bool invert; //inverts the direction of twang animations to account for inversions in the optics

    //table of region statuses, for reporting notes in the change-based
    //format used by MIDI
    BeamRegion previous_regions[SCALE_SIZE];
    int previous_bend;
    int previous_num_hands; //used for telling when hands leave the beam (produces Twang animations)

    list<Twang> twangs; //Twang animations that persist after a hand has left the beam.

    size_t hand_to_region(Hand& hand);
    int region_to_note(size_t region);
    size_t speed_to_midi_velocity(float speed); //experimental, currently not used
};
