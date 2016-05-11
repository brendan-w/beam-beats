
#pragma once

#include "ofxOpenCv.h"
#include "settings.h"
#include "hand.h"

/*
 * Class for storing and computing the details of a single beam of light.
 *
 * Beams are learned by creating beam-masks. These masks not only prevent
 * outside light sources from interfering, but are used to determine the
 * beam-relative position of a hand. Beam masks are blob detected, and a
 * minimum-area rectangle is fitted. The major axis of this rectangle is
 * assumed to be the axis of the beam. The blob_to_hand() function performs
 * these computations, and returns a hand with normalized, beam-local
 * coordinates.
 *
 * This class is completely computed from the beam mask, and can be recreated
 * by loading a mask image from disk.
 */

class BeamDescriptor
{
public:
    BeamDescriptor();
    BeamDescriptor(ofImage& image);
    ~BeamDescriptor();

    void draw(int x, int y);
    void zero();
    void learn();
    bool found_beam();
    void add_to_mask(ofxCvGrayscaleImage partial);
    Hand blob_to_hand(ofxCvBlob hand);

    ofxCvGrayscaleImage mask;
    ofxCvBlob blob;

    //beam details in pixel coordinates
    ofPoint top;
    ofPoint bottom;
    float width; //may differ from blob due to angle
    float height;

    //not really the perfect place for this,
    //stores hand positions from the last frame
    //for velocity computations
    vector<Hand> old_hands;
private:
    void find_blob();
    void find_details();
    float left_or_right(ofPoint p);
    ofPoint rotate_point(ofPoint point, ofPoint center, float angle);
};

