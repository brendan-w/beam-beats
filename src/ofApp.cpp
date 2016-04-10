
#include <sys/stat.h> //mkdir()
#include <cmath>

#include "ofApp.h"
#include "ofxPS3EyeGrabber.h"

#define NOT_LEARNING -1
#define THRESHOLD_INCREMENT 2


BeamCamera::BeamCamera(int deviceID, const string name) : cam_name(name)
{
    grabber.setGrabber(std::make_shared<ofxPS3EyeGrabber>());
    grabber.setDeviceID(deviceID);

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
    learning = NOT_LEARNING;

    //make sure our data diractories exist
    mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    load_config();
}

BeamCamera::~BeamCamera()
{
    save_config();

    //release our surfaces
    raw.clear();
    grey_bg.clear();
    grey_working.clear();
    grey_beam_working.clear();

    release_beam_masks();
}

void BeamCamera::release_beam_masks()
{
    for(ofxCvGrayscaleImage& mask : beam_masks)
    {
        if(mask.bAllocated)
            mask.clear();
    }
}

void BeamCamera::update()
{
    grabber.update();

    if(grabber.isFrameNew())
    {
        raw.setFromPixels(grabber.getPixels());

        //perform background subtraction
        grey_working = raw;
        cvSub(grey_working.getCvImage(),
              grey_bg.getCvImage(),
              grey_working.getCvImage());
        grey_working.flagImageChanged();

        //apply our intensity threshold
        grey_working.threshold(threshold);

        if(is_learning())
        {
            add_to_mask(learning);
        }
    }
}

void BeamCamera::draw_raw(int x, int y)
{
    raw.draw(x, y);
}

void BeamCamera::draw_working(int x, int y)
{
    grey_working.draw(x, y);
}

void BeamCamera::draw_masks(int x, int y)
{
    //draw the beam masks
    //figure out how many cols/rows we need for tiling
    int cell_divisor = (int) ceil(sqrt((double) beam_masks.size()));
    if(cell_divisor < 1)
        cell_divisor = 1;

    const int mask_width = WIDTH / cell_divisor;
    const int mask_height = HEIGHT / cell_divisor;

    for(size_t i = 0; i < beam_masks.size(); i++)
    {
        ofxCvGrayscaleImage& mask = beam_masks[i];
        if(mask.bAllocated)
        {
            int mx = x + (mask_width * i);
            int my = y + (mask_height * (i / cell_divisor));
            mask.draw(mx, my, mask_width, mask_height);
        }
    }
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

bool BeamCamera::is_learning()
{
    return learning != NOT_LEARNING;
}

void BeamCamera::start_learning_beam(int beam)
{
    ofLog() << cam_name << " started learning beam " << beam;

    //make sure our mask array has a spot for this beam
    if(beam >= (int) beam_masks.size())
        beam_masks.resize(beam + 1);

    // if the image exists, reset it
    if(beam_masks[beam].bAllocated)
        beam_masks[beam].clear();

    //allocate the new image
    beam_masks[beam].allocate(WIDTH, HEIGHT);
    cvZero(beam_masks[beam].getCvImage());

    learning = beam;
}

void BeamCamera::stop_learning_beam()
{
    ofLog() << cam_name << " stopped learning beam " << learning;
    learning = NOT_LEARNING;
}

void BeamCamera::add_to_mask(int beam)
{
    //this function assumes that we already have a mask allocated
    if(beam >= (int) beam_masks.size() || !beam_masks[beam].bAllocated)
        return;

    cvOr(grey_working.getCvImage(),
         beam_masks[beam].getCvImage(),
         beam_masks[beam].getCvImage());

    beam_masks[beam].flagImageChanged();
}

vector<ofxCvBlob> BeamCamera::blobs_for_beam(int beam)
{
    //return early if there's nothing to process
    if(beam >= (int) beam_masks.size() || !beam_masks[beam].bAllocated)
        return vector<ofxCvBlob>();

    //apply the mask that corresponds to this beam
    cvAnd(grey_working.getCvImage(),
          beam_masks[beam].getCvImage(),
          grey_beam_working.getCvImage());
    grey_beam_working.flagImageChanged();

    //find our hand blobs
    contourFinder.findContours(grey_beam_working, 100, (WIDTH*HEIGHT)/4, 10, false);    // find holes
    return contourFinder.blobs;
}

void BeamCamera::load_config()
{
    release_beam_masks(); //dump any existing masks
}

void BeamCamera::save_config()
{

}




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
    print_camera_list();

    cam_left = new BeamCamera(0, "left");
    cam_right = new BeamCamera(1, "right");
}

void ofApp::print_camera_list()
{
    for(ofVideoDevice device : ofxPS3EyeGrabber().listDevices())
    {
        ofLog() << "ID:" << device.id << ", Name:" << device.deviceName << ", Available:" << device.bAvailable;
    }
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
