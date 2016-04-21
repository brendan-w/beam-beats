#pragma once

#include "ofMain.h"
#include "beamCamera.h"
#include "ofxMidi.h"

#define sizeof_array(a) (sizeof(a)/sizeof(a[0]))

//UI
#define THRESHOLD_INCREMENT 2

//BEAM SETTINGS
const int pentatonic[] = {64, 67, 69, 71, 74, 76};


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
        ofxMidiOut midi_out;

        // status for each region of the beam
        bool beam_regions[sizeof_array(pentatonic)];
};
