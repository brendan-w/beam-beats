#pragma once

#include "ofxOpenCv.h"
#include "beamDescriptor.h"
#include "hand.h"

#define NOT_LEARNING -1



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

    int get_exposure();
    void adjust_exposure(int delta);

    bool is_learning();
    bool handles_beam(int beam);

private:
    //funcs --------------------
    void load_data();
    void compute_beam_blob(int beam);
    bool mask_exists(int beam);
    void new_beam(int beam);

    //data --------------------
    const string cam_name;
    int threshold;
    int exposure;
    int learning;

    ofVideoGrabber grabber;

    //surfaces for openCV processing
    ofxCvColorImage raw;                   // the current frame from the camera
    ofxCvGrayscaleImage grey_bg;           // the saved background used for subtraction
    ofxCvGrayscaleImage grey_working;      // the background-subbed, thresheld frame
    ofxCvGrayscaleImage grey_beam_working; // working buffer for single-beam masking operations

    //masks for differentiating each beam
    //WARNING: sparse array, use mask_exists()
    vector<BeamDescriptor*> beams;

    //blob detector
    ofxCvContourFinder contourFinder;
};
