
#pragma once

#include <math.h>
#include <ofxOpenCv.h>
#include <ofPoint.h>

//sane bounds for how fast a hand can travel
#define HAND_MAX_VX 0.1
#define HAND_MAX_VY 0.05


class Hand
{
public:
    ofPoint pos; //beam-normalized position
    ofPoint vel;
    ofPoint pixel_intersection; //location of the hand in the center of the beam
    ofxCvBlob blob;

    Hand() { };
    Hand(ofPoint _pos, ofPoint _pixel_intersection, ofxCvBlob _blob)
    {
        pos = _pos;
        pixel_intersection = _pixel_intersection;
        blob = _blob;
    };

    void compute_velocity(Hand& old_hand)
    {
        vel = old_hand.pos - pos;
    };

    bool same_hand_as(Hand& other)
    {
        return (abs(pos.x - other.pos.x) <= HAND_MAX_VX &&
                abs(pos.y - other.pos.y) <= HAND_MAX_VY);
    };
};
