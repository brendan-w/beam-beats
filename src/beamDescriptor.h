
#pragma once

#include "ofxOpenCv.h"
#include "settings.h"
#include "hand.h"

class BeamDescriptor
{
//this class is completely computed from the beam mask
public:
    BeamDescriptor();
    BeamDescriptor(ofImage& image);
    ~BeamDescriptor();

    void zero();
    void learn();
    void add_to_mask(ofxCvGrayscaleImage partial);
    Hand blob_to_hand(ofxCvBlob hand);

    ofxCvGrayscaleImage mask;
    ofxCvBlob blob;

    //beam details in pixel coordinates
    ofPoint top;
    ofPoint bottom;
    float width; //may differ from blob due to angle
    float height;
private:
    void find_blob();
    void find_details();
    ofPoint rotate_point(ofPoint point, ofPoint center, float angle);
};

