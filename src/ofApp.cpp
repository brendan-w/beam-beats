
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

    init_calibration();

    cam_left = new BeamCamera(0, "left");
    cam_right = new BeamCamera(1, "right");
}

void ofApp::init_calibration()
{

    //living life on the edge. only using 1 calibration image
    objectPoints.resize(1);
    imagePointsLeft.resize(1);
    imagePointsRight.resize(1);

    imagePointsLeft[0].resize(CALIB_BEAMS);
    imagePointsRight[0].resize(CALIB_BEAMS);

    //generate the desired coordinates of the targets
    for(int x = 0; x < CALIB_BEAMS; x++)
    {
        for(int y = 0; y < CALIB_HEIGHTS; y++)
        {
            objectPoints[0].push_back(Point3f((float) x,
                                              ((float) y / (CALIB_HEIGHTS - 1)),
                                              0.0));
        }
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

    vector<ofxCvBlob> blobs = cam_right->blobs_for_beam(0);

    for(ofxCvBlob& blob : blobs)
    {
        blob.draw(0, HEIGHT);
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
            else
            {
                //this must be a calibration point recorder
                //3D calibration keys
                for(int n = 0; n < CALIB_BEAMS * CALIB_HEIGHTS; n++)
                {
                    if(key == CALIB_KEYS[n])
                    {
                        save_calibration_point(n);
                        break;
                    }
                }
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

void ofApp::save_calibration_point(int n)
{
    //figure out which beam this point SHOULD be in
    int beam = n % CALIB_BEAMS;
    vector<ofxCvBlob> blobs_left = cam_left->blobs_for_beam(beam);
    vector<ofxCvBlob> blobs_right = cam_left->blobs_for_beam(beam);

    //make sure that only one blob was detected in each camera
    if(blobs_left.size() != 1 || blobs_right.size() != 1)
    {
        ofLog() << "Could not save calibration point";
        ofLog() << "There were either zero or multiple blobs, or one camera couldn't see the blob";
        return;
    }

    //record the blob locations for openCV
    imagePointsLeft[0][n].x = blobs_left[0].centroid.x;
    imagePointsLeft[0][n].y = blobs_left[0].centroid.y;

    imagePointsRight[0][n].x = blobs_right[0].centroid.x;
    imagePointsRight[0][n].y = blobs_right[0].centroid.y;
}

void ofApp::compute_calibration()
{
    Mat cameraMatrixLeft = Mat::eye(3, 3, CV_64F);
    Mat cameraMatrixRight = Mat::eye(3, 3, CV_64F);
    Mat distCoeffsLeft;
    Mat distCoeffsRight;
    Mat R, T, E, F;
    double rms = stereoCalibrate(objectPoints,
                                 imagePointsLeft,
                                 imagePointsRight,
                                 cameraMatrixLeft, distCoeffsLeft,
                                 cameraMatrixRight, distCoeffsRight,
                                 Size(WIDTH, HEIGHT),
                                 R, T, E, F,
                                 TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 100, 1e-5),
                                 CALIB_FIX_ASPECT_RATIO +
                                 CALIB_ZERO_TANGENT_DIST +
                                 CALIB_SAME_FOCAL_LENGTH +
                                 CALIB_RATIONAL_MODEL +
                                 CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5);
}
