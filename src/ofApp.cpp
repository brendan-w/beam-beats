
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
    ofSetFrameRate(FRAMERATE);
    show_raw = false;
    project_beams = false;
    list_devices();

    //=====================================
    //WARNING: hard coded values
    //TODO: move to config file
    //=====================================

    //construct our 4 beams (channel, base note, color)
    beams.push_back(Beam(1, 28, 0x00ffAA)); //green
    beams.push_back(Beam(1, 40, 0xfcb017)); //orange
    beams.push_back(Beam(1, 52, 0x00b0d4)); //blue
    beams.push_back(Beam(1, 64, 0x9c258e)); //purple

    cameras.push_back(new BeamCamera(0, "left"));
    cameras.push_back(new BeamCamera(1, "right"));

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

        if(!project_beams)
        {
            if(show_raw)
                camera->draw_raw(0, row);
            else
                camera->draw_working(0, row);

            camera->draw_masks(WIDTH, row);
        }
        else
        {
            draw_colors();
        }

        //iterate over beams
        for(size_t b = 0; b < beams.size(); b++)
        {
            if(!camera->handles_beam(b))
                continue;

            //get hand objects from the camera, and parse them into notes
            vector<Hand> hands = camera->hands_for_beam(b);
            beams[b].update(hands, midi_out); //parse hands, and send MIDI

            if(project_beams)
            {
                int width = ofGetWindowWidth() / beams.size();
                ofPushMatrix();
                ofTranslate(width * b, 0, 0);
                beams[b].draw(hands, width);
                ofPopMatrix();
            }
            else
            {
                //draw the hand on screen
                for(Hand& hand : hands)
                {
                    hand.draw(0, row);
                }
            }
        }
    }

    ofSetHexColor(0xffffff);
    stringstream t;
    t << "FPS: " << ofGetFrameRate() << endl;
    t << "THRESH: " << cameras[0]->get_threshold() << endl;
    ofDrawBitmapString(t.str(), 20, 20);
}

void ofApp::draw_colors()
{
    const int w = ofGetWindowWidth() / beams.size();
    for(size_t b = 0; b < beams.size(); b++)
    {
        ofPushStyle();
        ofSetHexColor(beams[b].color);
        ofFill();
        ofDrawRectangle(w * b,
                        0,
                        w,
                        ofGetWindowHeight());
        ofPopStyle();
    }
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
        case OF_KEY_F1:
            project_beams = !project_beams;
            break;
        case OF_KEY_RETURN:
            for(BeamCamera* camera : cameras)
                camera->stop_learning_beam();
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

                for(BeamCamera* camera : cameras)
                    camera->start_learning_beam(beam);
            }
    }
}
