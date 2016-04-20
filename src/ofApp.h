#pragma once

#include "ofMain.h"
#include "beamCamera.h"
#include "ofxMidi.h"

//UI
#define THRESHOLD_INCREMENT 2

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
        //funcs ---------------------
        void list_devices();
        void stop_learning_beams();

        //data ----------------------
        bool show_raw;
        BeamCamera* cam_left;
        BeamCamera* cam_right;
};
