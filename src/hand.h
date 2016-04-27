
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
    ofPoint intersection; //perpedicular intersection with the center of the beam
    ofxCvBlob blob;

    Hand() { };
    Hand(ofPoint _pos, ofPoint _intersection, ofxCvBlob _blob)
    {
        pos = _pos;
        intersection = _intersection;
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

    void draw(int x, int y)
    {
        ofPushStyle();
        ofFill();
        ofSetHexColor(0x00FF00);
        ofDrawCircle(blob.centroid.x, blob.centroid.y, 3);
        ofDrawLine(blob.centroid.x, blob.centroid.y,
                   intersection.x, intersection.y);
        ofPopStyle();
    };
};
