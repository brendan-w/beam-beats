#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

#define WIDTH 320
#define HEIGHT 240
#define FRAMERATE 60
#define INIT_THRESHOLD 80


class BeamCamera
{
public:
    BeamCamera(const string dir);
    ~BeamCamera();

    void update();
    void draw(int x, int y);
    void get_hands_for_beam(int beam);

    void learn_background();
    void start_learning_beam(int beam);
    void stop_learning_beam();

    int get_threshold();
    void adjust_threshold(int delta);

private:
    const string cam_data;
    int threshold;

    ofVideoGrabber grabber;

    //surfaces for openCV processing
    ofxCvColorImage raw;                   // the current frame from the camera
    ofxCvGrayscaleImage grey_bg;           // the saved background used for subtraction
    ofxCvGrayscaleImage grey_working;      // the background-subbed, thresheld frame
    ofxCvGrayscaleImage grey_beam_working; // working buffer for single-beam masking operations

    //masks for differentiating each beam
    std::vector<ofxCvGrayscaleImage> beam_masks;

    //blob detector
    ofxCvContourFinder contourFinder;
};


class ofApp : public ofBaseApp{
	public:
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

        BeamCamera* cam_left;
        //BeamCamera* cam_right;
};
