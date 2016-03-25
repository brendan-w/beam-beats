#include "ofApp.h"
#include "ofxPS3EyeGrabber.h"

#define WIDTH 640
#define HEIGHT 480

//--------------------------------------------------------------
void ofApp::setup(){
    // Set the video grabber to the ofxPS3EyeGrabber.
    vidGrabber.setGrabber(std::make_shared<ofxPS3EyeGrabber>());
    vidGrabber.setup(WIDTH, HEIGHT);

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
		grayDiff.absDiff(grayBg, grayImage);
		grayDiff.threshold(threshold);

		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		//contourFinder.findContours(grayDiff, 20, (WIDTH*HEIGHT)/3, 10, true);	// find holes
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    ofSetColor(255);
    //vidGrabber.draw(0, 0);
    grayDiff.draw(0,0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
