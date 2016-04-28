
#include <cmath>
#include "beam.h"


Beam::Beam(int channel, int base_note) : channel(channel), base_note(base_note)
{
    //zero out our note status array
    for(size_t r = 0; r < sizeof_array(midi_scale); r++)
    {
        regions[r] = false;
    }
}

void Beam::update(vector<Hand> hands, ofxMidiOut& midi_out)
{
    bool current_regions[sizeof_array(midi_scale)] = { false };
    float current_speeds[sizeof_array(midi_scale)] = { 0.0 };

    for(Hand& hand : hands)
    {
        size_t r = hand_to_region(hand);
        //mark that there is a hand in this region
        current_regions[r] = true;
        //fastest hand dominates
        current_speeds[r] = max(hand.speed(), current_speeds[r]);
    }

    for(size_t r = 0; r < sizeof_array(midi_scale); r++)
    {
        int note = region_to_note(r);
        int vel = speed_to_midi_velocity(current_speeds[r]);

        //if the hand is new, and it has non-zero speed
        if(current_regions[r] && !regions[r])
        {
            //NOTE ON
            ofLog() << "ON " << note << " : " << vel;
            midi_out.sendNoteOn(1, note, vel);
        }
        else if(!current_regions[r] && regions[r])
        {
            //a hand just LEFT a region
            //NOTE OFF
            ofLog() << "OFF " << note << " : " << vel;
            midi_out.sendNoteOff(1, note, 64);
        }

        //walk the buffers
        regions[r] = current_regions[r];
    }
}

size_t Beam::hand_to_region(Hand& hand)
{
    return (size_t) floor(ofLerp(0, sizeof_array(midi_scale) + 1, hand.pos.y));
}

int Beam::region_to_note(size_t region)
{
    return base_note + midi_scale[region];
}


size_t Beam::speed_to_midi_velocity(float speed)
{
    return midi_velocities[3]; //TODO
}

