#include "ofApp.h"
#include "ofxPS3EyeGrabber.h"

#define THRESHOLD_INCREMENT 2


BeamCamera::BeamCamera(const string dir) : cam_data(dir)
{
    grabber.setGrabber(std::make_shared<ofxPS3EyeGrabber>());

    grabber.setPixelFormat(OF_PIXELS_RGB);
    grabber.setDesiredFrameRate(FRAMERATE);
    grabber.setup(WIDTH, HEIGHT);

    //PS3 Eye specific settings
    grabber.getGrabber<ofxPS3EyeGrabber>()->setAutogain(false);
    grabber.getGrabber<ofxPS3EyeGrabber>()->setAutoWhiteBalance(false);

    //allocate our working surfaces
    raw.allocate(WIDTH, HEIGHT);
    grey_bg.allocate(WIDTH, HEIGHT);
    grey_working.allocate(WIDTH, HEIGHT);
    grey_beam_working.allocate(WIDTH, HEIGHT);

    threshold = INIT_THRESHOLD;
}

BeamCamera::~BeamCamera()
{
    //release our surfaces
    raw.clear();
    grey_bg.clear();
    grey_working.clear();
    grey_beam_working.clear();
}

void BeamCamera::update()
{
    grabber.update();

    if(grabber.isFrameNew())
    {
        raw.setFromPixels(grabber.getPixels());

        //perform background subtraction
        grey_working = raw;
        cvSub(grey_working.getCvImage(), grey_bg.getCvImage(), grey_working.getCvImage());
        grey_working.flagImageChanged();

        //apply our intensity threshold
        grey_working.threshold(threshold);
    }
}

void BeamCamera::draw(int x, int y)
{
    raw.draw(x, y);
}

int BeamCamera::get_threshold()
{
    return threshold;
}

void BeamCamera::adjust_threshold(int delta)
{
    //clamp to [0, 255]
    int new_thresh = threshold + delta;
    if(new_thresh < 0) new_thresh = 0;
    else if(new_thresh > 255) new_thresh = 255;
    threshold = new_thresh;
}

void BeamCamera::learn_background()
{
    grey_bg = raw;
}

void BeamCamera::get_hands_for_beam(int beam)
{
    //apply the mask that corresponds to this beam
    grey_beam_working = grey_working;
    //TODO

    //find our hand blobs
    contourFinder.findContours(grey_beam_working, 100, (WIDTH*HEIGHT)/4, 10, false);	// find holes

    //TODO: return something
}


//--------------------------------------------------------------
void ofApp::setup()
{

    std::vector<ofVideoDevice> devices = ofxPS3EyeGrabber().listDevices();

    // Now cycle through the devices and set up grabbers for each.
    for (std::size_t i = 0; i < devices.size(); ++i)
    {
        std::stringstream ss;

        ss << devices[i].id << ": " << devices[i].deviceName << " : " << devices[i].serialID;

        if (!devices[i].bAvailable)
        {
            ss << " - unavailable ";
        }

        ofLogNotice("ofApp::setup") << ss.str();
    }

    cam_left = new BeamCamera("left");
}

//--------------------------------------------------------------
void ofApp::update()
{
    cam_left->update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0);
    ofSetColor(255);

    cam_left->draw(0, 0);

    //for(int i = 0; i < contourFinder.nBlobs; i++)
    //{
    //    contourFinder.blobs[i].draw(0, 0);
    //    contourFinder.blobs[i].draw(WIDTH, 0);
    //}

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
        case 'q':
            std::exit(0);
		case ' ':
			cam_left->learn_background();
			break;
		case OF_KEY_UP:
		    cam_left->adjust_threshold(THRESHOLD_INCREMENT);
			break;
		case OF_KEY_DOWN:
		    cam_left->adjust_threshold(-THRESHOLD_INCREMENT);
			break;
	}
}
