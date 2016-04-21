
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

    for(int i = 0; i < sizeof_array(beam_regions); i++)
    {
        beam_regions[i] = false;
    }
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

    vector<ofxCvBlob> blobs;
    bool current_regions[sizeof_array(pentatonic)];

    for(int i = 0; i < sizeof_array(pentatonic); i++)
    {
        current_regions[i] = false;
    }

    blobs = cam_left->blobs_for_beam(0);
    for(ofxCvBlob& blob : blobs)
    {
        //process hands
        float region_f = ofMap(blob.centroid.y, 0, HEIGHT, 0, sizeof_array(pentatonic) - 1);
        int region = (int) roundf(region_f);

        //mark the current region as having a hand
        current_regions[region] = true;
        blob.draw(0, 0);
    }

    //shift the midi out
    for(int i = 0; i < sizeof_array(pentatonic); i++)
    {
        if(current_regions[i] && !beam_regions[i])
        {
            ofLog() << "Note ON " << pentatonic[i];
            midi_out.sendNoteOn(1, pentatonic[i], 64);
        }
        else if(!current_regions[i] && beam_regions[i])
        {
            ofLog() << "Note OFF " << pentatonic[i];
            midi_out.sendNoteOff(1, pentatonic[i], 64);
        }

        beam_regions[i] = current_regions[i];
    }

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
