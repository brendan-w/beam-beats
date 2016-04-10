#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOpenCv.h"

#define WIDTH 320
#define HEIGHT 240
#define FRAMERATE 60
#define INIT_THRESHOLD 80


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


class ofApp : public ofBaseApp{
	public:
        ~ofApp();
        void setup();
        void update();
        void draw();

        void keyPressed(int key);
        //void keyReleased(int key);
        //void mouseMoved(int x, int y);
        //void mouseDragged(int x, int y, int button);
        //void mousePressed(int x, int y, int button);
        //void mouseReleased(int x, int y, int button);
        //void mouseEntered(int x, int y);
        //void mouseExited(int x, int y);
        //void windowResized(int w, int h);
        //void dragEvent(ofDragInfo dragInfo);
        //void gotMessage(ofMessage msg);

    private:
        void print_camera_list();
        void stop_learning_beams();
        bool show_raw;
        BeamCamera* cam_left;
        BeamCamera* cam_right;
};
