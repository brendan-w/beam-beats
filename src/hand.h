
#pragma once

#include <math.h>
#include <ofxOpenCv.h>
#include <ofPoint.h>

//sane bounds for how fast a hand can travel
#define HAND_MAX_VX 0.1
#define HAND_MAX_VY 0.05


/*
 * Class that represents a hand in a beam. Contains the original ofxCvBlob
 * from the BeamCameras, and computes velocities.
 */

class Hand
{
public:
    ofPoint pos; //beam-normalized position
    ofPoint vel; //beam-normalized velocity
    ofPoint intersection; //perpedicular intersection with the center of the beam in pixels coordinates
    ofxCvBlob blob; //the original ofxCvBlob that this hand was generated from.
    float area_vel; //the rate of change in the beam-exposed area of the hand. (experimental tool for computing velocity)

    Hand() { };
    Hand(ofPoint _pos, ofPoint _intersection, ofxCvBlob _blob)
    {
        pos = _pos;
        intersection = _intersection;
        blob = _blob;
    };

    //computes this hand's velocity based on the position of the same
    //hand from the previous frame
    void compute_velocity(Hand& old_hand)
    {
        vel = old_hand.pos - pos;
        area_vel = old_hand.blob.area - blob.area;
    };

    bool same_hand_as(Hand& other)
    {
        return (abs(pos.x - other.pos.x) <= HAND_MAX_VX &&
                abs(pos.y - other.pos.y) <= HAND_MAX_VY);
    };

    float speed() { return vel.length() * area_vel; }

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
