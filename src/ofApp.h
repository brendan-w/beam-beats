#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

#define WIDTH 320
#define HEIGHT 240
#define FRAMERATE 60


class BeamCamera
{
public:
    BeamCamera(const string cam_data_dir);
    ~BeamCamera();

    void update();
    void learn_background();
    void start_learning_beam(int beam);
    void stop_learning_beam();

private:
    ofVideoGrabber grabber;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImg;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayWorking;

    ofxCvContourFinder contourFinder;

    std::vector<ofxCvGrayscaleImage> beam_masks;
};


class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key) {};
		void mouseMoved(int x, int y) {};
		void mouseDragged(int x, int y, int button) {};
		void mousePressed(int x, int y, int button) {};
		void mouseReleased(int x, int y, int button) {};
		void mouseEntered(int x, int y) {};
		void mouseExited(int x, int y) {};
		void windowResized(int w, int h) {};
		void dragEvent(ofDragInfo dragInfo) {};
		void gotMessage(ofMessage msg) {};

        ofVideoGrabber cam_left;
        ofVideoGrabber cam_right;

        ofxCvColorImage			colorImg;
        ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;

        ofxCvContourFinder 	contourFinder;

		int 				threshold;
		bool				bLearnBakground;
};
