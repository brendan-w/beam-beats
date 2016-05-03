
#include <cmath>
#include "beam.h"


Beam::Beam(int channel, int base_note, int color) :
    channel(channel), base_note(base_note), color(color), previous_bend(0x2000)
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
    float max_vel = 0.0;

    //sort hands into their respective regions
    for(Hand& hand : hands)
    {
        size_t r = hand_to_region(hand);
        //mark that there is a hand in this region
        regions[r].status = true;

        //biggest hand dominates
        if(hand.blob.area > regions[r].hand.blob.area)
            regions[r].hand = hand;

        //hand with the most velocity dominates pitch bending
        if(hand.vel.x > max_vel)
            max_vel = (hand.vel.x < 0) ? 0 : hand.vel.x;
    }

    int bend = 0x2000 + (0x8800 * max_vel);
    if(previous_bend != bend)
    {
        midi_out.sendPitchBend(channel, bend);
        previous_bend = bend;
    }

    //check the current region status against the previous status,
    //to determine whether to send note ON or OFF
    for(size_t r = 0; r < SCALE_SIZE; r++)
    {
        int note = region_to_note(r);

        //if the hand is new
        if(regions[r].status && !previous_regions[r].status)
        {
            //NOTE ON
            int vel = area_to_velocity(regions[r].hand.blob.area);
            ofLog() << "ON " << note << " : " << vel;
            midi_out.sendNoteOn(channel, note, vel);
        }
        else if(!regions[r].status && previous_regions[r].status)
        {
            //a hand just LEFT a region
            //NOTE OFF
            ofLog() << "OFF " << note;
            midi_out.sendNoteOff(channel, note, 64);
        }

        //walk the buffers
        previous_regions[r] = regions[r];
    }
}

size_t Beam::hand_to_region(Hand& hand)
{
    int r = floor(ofLerp(0, SCALE_SIZE, hand.pos.y));
    return max(0, min(r, (int) SCALE_SIZE - 1)); //clamp
}

int Beam::region_to_note(size_t region)
{
    return base_note + midi_scale[region];
}

size_t Beam::area_to_velocity(float area)
{
    int vel = ofMap(area, 30, 100, 0, VEL_SIZE - 1);
    vel = max(0, min(vel, (int) VEL_SIZE - 1)); //clamp
    return midi_velocities[vel];
}

