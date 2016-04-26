#pragma once

#include "ofxOpenCv.h"
#include "hand.h"


#define WIDTH 320
#define HEIGHT 240

#define BLOB_AREA_MIN 100
#define BLOB_AREA_MAX (WIDTH * HEIGHT / 4)
#define N_BLOBS 10

#define FRAMERATE 60
#define INIT_THRESHOLD 128
#define BACKGROUND_FILE "background"
#define IMAGE_FORMAT "png"

#define NOT_LEARNING -1


ofPoint rotate_point(ofPoint point, ofPoint center, float angle);


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
    Hand blob_to_hand(ofxCvBlob blob);

    ofxCvGrayscaleImage mask;
    ofxCvBlob blob;

    //beam details in pixel coordinates
    ofPoint top;
    ofPoint bottom;
    float radius;
};



class BeamCamera
{
public:
    BeamCamera(int deviceID, const string dir);
    ~BeamCamera();

    void update();
    void draw_raw(int x, int y);
    void draw_working(int x, int y);
    void draw_masks(int x, int y);

    vector<Hand> hands_for_beam(int beam);

    void learn_background();
    void start_learning_beam(int beam);
    void stop_learning_beam();

    int get_threshold();
    void adjust_threshold(int delta);

    bool is_learning();

private:
    //funcs --------------------
    void load_data();
    void compute_beam_blob(int beam);
    bool mask_exists(int beam);
    void new_beam(int beam);

    //data --------------------
    const string cam_name;
    int threshold;
    int learning;

    ofVideoGrabber grabber;

    //surfaces for openCV processing
    ofxCvColorImage raw;                   // the current frame from the camera
    ofxCvGrayscaleImage grey_bg;           // the saved background used for subtraction
    ofxCvGrayscaleImage grey_working;      // the background-subbed, thresheld frame
    ofxCvGrayscaleImage grey_beam_working; // working buffer for single-beam masking operations

    //masks for differentiating each beam
    //WARNING: could be a sparse array, use mask_exists()
    vector<BeamDescriptor*> beams;

    //vector of previous hand objects, for reporting velocity
    vector<Hand> old_hands;

    //blob detector
    ofxCvContourFinder contourFinder;
};
