#pragma once

#include "ofxXmlSettings.h"
#include "ofxOpenCv.h"

#define WIDTH 320
#define HEIGHT 240
#define FRAMERATE 60
#define INIT_THRESHOLD 80
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

    vector<ofxCvBlob> blobs_for_beam(int beam);

    void learn_background();
    void start_learning_beam(int beam);
    void stop_learning_beam();

    int get_threshold();
    void adjust_threshold(int delta);

    bool is_learning();

private:
    void save_config();
    void load_config();
    void add_to_mask(int beam);
    void release_beam_masks();

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
    //WANRING: could be a sparse array
    vector<ofxCvGrayscaleImage> beam_masks;

    //blob detector
    ofxCvContourFinder contourFinder;
};
