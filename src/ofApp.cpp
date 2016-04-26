
#include "ofApp.h"
#include "ofxPS3EyeGrabber.h"



//--------------------------------------------------------------
ofApp::~ofApp()
{
    delete cam_left;
    delete cam_right;
}

//--------------------------------------------------------------
void ofApp::setup()
{
    show_raw = false;
    list_devices();

    cam_left = new BeamCamera(0, "left");
    cam_right = new BeamCamera(1, "right");
    midi_out.openPort(1);
}

void ofApp::list_devices()
{
    ofLog() << "Cameras ---------------";
    for(ofVideoDevice device : ofxPS3EyeGrabber().listDevices())
    {
        ofLog() << "ID:" << device.id << ", Name:" << device.deviceName << ", Available:" << device.bAvailable;
    }

    ofLog() << "MIDI Ports ------------";
    ofxMidiOut::listPorts();
}

//--------------------------------------------------------------
void ofApp::update()
{
    cam_left->update();
    cam_right->update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0);
    ofSetColor(255);

    if(show_raw)
    {
        cam_left->draw_raw(0, 0);
        cam_right->draw_raw(0, HEIGHT);
    }
    else
    {
        cam_left->draw_working(0, 0);
        cam_right->draw_working(0, HEIGHT);
    }

    cam_left->draw_masks(WIDTH, 0);
    cam_right->draw_masks(WIDTH, HEIGHT);

    vector<Hand> hands;
    hands = cam_left->hands_for_beam(1);

    /*
    blobs = cam_right->blobs_for_beam(0);
    for(ofxCvBlob& blob : blobs)
    {
        blob.draw(0, HEIGHT);
    }
    */

    ofSetHexColor(0xffffff);
    stringstream t;
    t << "FPS: " << ofGetFrameRate();

    ofDrawBitmapString(t.str(), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch(key)
    {
        case ' ':
            cam_left->learn_background();
            cam_right->learn_background();
            break;
        case OF_KEY_TAB:
            show_raw = !show_raw;
            break;
        case OF_KEY_RETURN:
            stop_learning_beams();
            break;
        case OF_KEY_UP:
            cam_left->adjust_threshold(THRESHOLD_INCREMENT);
            cam_right->adjust_threshold(THRESHOLD_INCREMENT);
            break;
        case OF_KEY_DOWN:
            cam_left->adjust_threshold(-THRESHOLD_INCREMENT);
            cam_right->adjust_threshold(-THRESHOLD_INCREMENT);
            break;

        default:
            //test for beam-learning hotkeys
            if(key >= '1' && key <= '9')
            {
                int beam = key - '1';

                stop_learning_beams(); //stop learning any previous beams

                if(!cam_left->is_learning())
                    cam_left->start_learning_beam(beam);
                if(!cam_right->is_learning())
                    cam_right->start_learning_beam(beam);
            }
    }
}

void ofApp::stop_learning_beams()
{
    if(cam_left->is_learning())
        cam_left->stop_learning_beam();
    if(cam_right->is_learning())
        cam_right->stop_learning_beam();
}
