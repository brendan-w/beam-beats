
#include <cmath>
#include "beam.h"



Twang::Twang(int dir)
{
    done = false;
    direction = dir;
    frame = 1;
}

void Twang::draw()
{
    //stop animating if the twang is over
    if(frame >= TWANG_TIME)
    {
        done = true;
        return;
    }

    ofPushStyle();
    ofFill();
    ofSetRectMode(OF_RECTMODE_CENTER);

    //intensity fades over time
    int alpha = ofMap(frame, 0, TWANG_TIME, 255, 0);
    //homebrew easing function for the beam twang
    //essentially a decaying cosine
    float pos = cos((frame - 1) * TWANG_SPEED) / sqrt(frame);
    pos = ofClamp(pos, -1.0, 1.0);
    pos *= direction;
    pos = ofMap(pos, -1, 1, 0, 1);

    ofSetColor(255, 255, 255, alpha);
    ofDrawRectangle(0.5, pos, 1, HAND_BEAM_WIDTH);

    ofPopStyle();

    frame++;
}




Beam::Beam(int channel, int base_note, int color) :
    channel(channel), base_note(base_note), color(color), previous_bend(0x2000)
{
    previous_num_hands = 0;
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
    ofPushStyle();
    ofFill();
    ofSetRectMode(OF_RECTMODE_CENTER);

    for(Hand& hand : hands)
    {
        //intensity is based on bend value
        int alpha = ofMap(abs(hand.vel.x), 0, 1, HAND_BEAM_MIN, 255);
        alpha = ofClamp(alpha, 0, 255);
        ofSetColor(255, 255, 255, alpha);

        float y = ofMap(hand.pos.x, -1, 1, 0, 1);
        ofDrawRectangle(0.5, y, 1, HAND_BEAM_WIDTH);
    }

    //draw any running twangs
    for(auto i = twangs.begin(); i != twangs.end();)
    {
        Twang& twang = *i;

        //remove twangs that are done
        if(twang.done)
        {
            i = twangs.erase(i);
        }
        else
        {
            twang.draw();
            i++;
        }
    }

    ofPopStyle();
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
        //fastest hand dominates
        if(hand.speed() > regions[r].hand.speed())
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

            //start a twang
            int direction = (previous_regions[r].hand.pos.x > 0) ? 1 : -1;

            //if a hand actually left the beam
            //(not just travelled to a different note)
            if(hands.size() < previous_num_hands)
                twangs.push_back(Twang(direction));
        }

        //walk the buffers
        previous_regions[r] = regions[r];
    }

    //
    previous_num_hands = hands.size();
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

