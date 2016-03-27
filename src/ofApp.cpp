#include "ofApp.h"
#include "ofxPS3EyeGrabber.h"

#define WIDTH 320
#define HEIGHT 240

//--------------------------------------------------------------
void ofApp::setup(){
    // Set the video grabber to the ofxPS3EyeGrabber.
    vidGrabber.setGrabber(std::make_shared<ofxPS3EyeGrabber>());

    vidGrabber.setPixelFormat(OF_PIXELS_RGB);
    vidGrabber.setDesiredFrameRate(60);
    vidGrabber.setup(WIDTH, HEIGHT);

    //PS3 Eye specific settings
    vidGrabber.getGrabber<ofxPS3EyeGrabber>()->setAutogain(false);
    vidGrabber.getGrabber<ofxPS3EyeGrabber>()->setAutoWhiteBalance(false);

    colorImg.allocate(WIDTH, HEIGHT);
    grayImage.allocate(WIDTH, HEIGHT);
    grayBg.allocate(WIDTH, HEIGHT);
    grayDiff.allocate(WIDTH, HEIGHT);

    bLearnBakground = true;
    threshold = 80;
}

//--------------------------------------------------------------
void ofApp::update(){
    vidGrabber.update();

    if(vidGrabber.isFrameNew())
    {
        colorImg.setFromPixels(vidGrabber.getPixels());
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

		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayDiff, 20, (WIDTH*HEIGHT)/3, 10, true);	// find holes
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
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
void ofApp::keyPressed(int key){

	switch(key)
    {
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
