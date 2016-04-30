
#include "ofApp.h"
#include "ofxPS3EyeGrabber.h"
#include "settings.h"


//--------------------------------------------------------------
ofApp::~ofApp()
{
    for(BeamCamera* camera : cameras)
        delete camera;
}

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetWindowPosition(0, 0);
    show_raw = false;
    list_devices();

    //construct our 4 beams
    beams.push_back(Beam(1, 64)); //channel, octave
    beams.push_back(Beam(2, 64));
    beams.push_back(Beam(3, 64));
    beams.push_back(Beam(4, 64));

    cameras.push_back(new BeamCamera(0, "left"));
    //cameras.push_back(new BeamCamera(1, "right"));

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
    for(BeamCamera* camera : cameras)
        camera->update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0);
    ofSetColor(255);

    for(size_t i = 0; i < cameras.size(); i++)
    {
        BeamCamera* camera = cameras[i];
        int row = HEIGHT * i;

        if(show_raw)
            camera->draw_raw(0, row);
        else
            camera->draw_working(0, row);

        camera->draw_masks(WIDTH, row);

        //iterate over beams
        for(size_t b = 0; b < beams.size(); b++)
        {
            vector<Hand> hands = camera->hands_for_beam(b);
            beams[b].update(hands, midi_out); //parse hands, and send MIDI

            //draw the hand on screen
            for(Hand& hand : hands)
            {
                hand.draw(0, row);
            }
        }
    }

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
            for(BeamCamera* camera : cameras)
                camera->learn_background();
            break;
        case OF_KEY_TAB:
            show_raw = !show_raw;
            break;
        case OF_KEY_RETURN:
            stop_learning();
            break;
        case OF_KEY_UP:
            for(BeamCamera* camera : cameras)
                camera->adjust_threshold(THRESHOLD_INCREMENT);
            break;
        case OF_KEY_DOWN:
            for(BeamCamera* camera : cameras)
                camera->adjust_threshold(-THRESHOLD_INCREMENT);
            break;

        default:
            //test for beam-learning hotkeys
            if(key >= '1' && key <= '9')
            {
                int beam = key - '1';

                stop_learning(); //stop learning any previous beams

                for(BeamCamera* camera : cameras)
                {
                    if(!camera->is_learning())
                        camera->start_learning_beam(beam);
                }
            }
    }
}

void ofApp::stop_learning()
{
    for(BeamCamera* camera : cameras)
    {
        if(!camera->is_learning())
            camera->stop_learning_beam();
    }
}
