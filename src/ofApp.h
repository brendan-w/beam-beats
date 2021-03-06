#pragma once

#include "ofMain.h"
#include "beam.h"
#include "beamCamera.h"
#include "ofxMidi.h"


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
        void goto_beam(int beam);

        //data ----------------------
        bool show_raw;
        bool project_beams;
        ofxMidiOut midi_out;

        vector<BeamCamera*> cameras;
        vector<Beam> beams;
};
