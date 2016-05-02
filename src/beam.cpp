
#include <cmath>
#include "beam.h"


Beam::Beam(int channel, int base_note, int color) :
    channel(channel), base_note(base_note), color(color)
{

}

void Beam::draw_bg()
{
    ofPushStyle();
    ofFill();
    ofSetHexColor(color);
    ofDrawRectangle(0, 0, 1, 1);
    ofPopStyle();
}

void Beam::draw(vector<Hand> hands)
{
    if(hands.size() > 0)
    {
        ofPushStyle();
        ofFill();
        ofSetHexColor(0xAAAAAA);
        ofDrawRectangle(0, 0, 1, 1);
        ofPopStyle();
    }
}

void Beam::update(vector<Hand> hands, ofxMidiOut& midi_out)
{
    BeamRegion regions[SCALE_SIZE];

    //sort hands into their respective regions
    for(Hand& hand : hands)
    {
        size_t r = hand_to_region(hand);
        //mark that there is a hand in this region
        regions[r].status = true;
        //fastest hand dominates
        if(hand.speed() > regions[r].hand.speed())
            regions[r].hand = hand;
    }

    //check the current region status against the previous status,
    //to determine whether to send note ON or OFF
    for(size_t r = 0; r < SCALE_SIZE; r++)
    {
        int note = region_to_note(r);
        int vel = speed_to_midi_velocity(regions[r].hand.speed());

        //if the hand is new
        if(regions[r].status && !previous_regions[r].status)
        {
            //NOTE ON
            ofLog() << "ON " << note << " : " << vel;
            midi_out.sendNoteOn(channel, note, vel);
        }
        else if(!regions[r].status && previous_regions[r].status)
        {
            //a hand just LEFT a region
            //NOTE OFF
            ofLog() << "OFF " << note << " : " << vel;
            midi_out.sendNoteOff(channel, note, 64);
        }

        //walk the buffers
        previous_regions[r] = regions[r];
    }
}

size_t Beam::hand_to_region(Hand& hand)
{
    size_t r = floor(ofLerp(0, SCALE_SIZE, hand.pos.y));
    return max((size_t) 0, min(r, SCALE_SIZE - 1));
}

int Beam::region_to_note(size_t region)
{
    return base_note + midi_scale[region];
}

size_t Beam::speed_to_midi_velocity(float speed)
{
    return midi_velocities[0]; //TODO
}

