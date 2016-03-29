#include "ofApp.h"
#include "ofxPS3EyeGrabber.h"



BeamCamera::BeamCamera(const string cam_data_dir)
{
    
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



    // Set the video grabber to the ofxPS3EyeGrabber.
    cam_left.setGrabber(std::make_shared<ofxPS3EyeGrabber>());

    cam_left.setPixelFormat(OF_PIXELS_RGB);
    cam_left.setDesiredFrameRate(60);
    cam_left.setup(WIDTH, HEIGHT);

    //PS3 Eye specific settings
    cam_left.getGrabber<ofxPS3EyeGrabber>()->setAutogain(false);
    cam_left.getGrabber<ofxPS3EyeGrabber>()->setAutoWhiteBalance(false);

    colorImg.allocate(WIDTH, HEIGHT);
    grayImage.allocate(WIDTH, HEIGHT);
    grayBg.allocate(WIDTH, HEIGHT);
    grayDiff.allocate(WIDTH, HEIGHT);

    bLearnBakground = true;
    threshold = 80;
}

//--------------------------------------------------------------
void ofApp::update()
{
    cam_left.update();

    if(cam_left.isFrameNew())
    {
        colorImg.setFromPixels(cam_left.getPixels());
        grayImage = colorImg;
		if (bLearnBakground == true)
        {
            // the = sign copys the pixels from grayImage into grayBg (operator overloading)
			grayBg = grayImage;
			bLearnBakground = false;
		}

		// take the abs value of the difference between background and incoming and then threshold:
		//grayDiff.absDiff(grayBg, grayImage);
        cvSub(grayImage.getCvImage(), grayBg.getCvImage(), grayDiff.getCvImage());
        grayDiff.flagImageChanged();
        grayDiff.threshold(threshold);

		contourFinder.findContours(grayDiff, 100, (WIDTH*HEIGHT)/4, 10, false);	// find holes
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0);
    ofSetColor(255);

    colorImg.draw(0, 0);
    grayDiff.draw(WIDTH, 0);

    for(int i = 0; i < contourFinder.nBlobs; i++)
    {
        contourFinder.blobs[i].draw(0, 0);
        contourFinder.blobs[i].draw(WIDTH, 0);
    }

    ofSetHexColor(0xffffff);
    stringstream t;
    t << "Threshold: " << threshold << std::endl
      << "Blobs: " << contourFinder.nBlobs << std::endl
      << "FPS: " << ofGetFrameRate();

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
			bLearnBakground = true;
			break;
		case OF_KEY_UP:
			threshold++;
			if (threshold > 255) threshold = 255;
			break;
		case OF_KEY_DOWN:
			threshold--;
			if (threshold < 0) threshold = 0;
			break;
	}
}
