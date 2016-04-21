
#include <cmath>

#include "ofxPS3EyeGrabber.h"
#include "beamCamera.h"


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
}

BeamCamera::~BeamCamera()
{
    //release our surfaces
    raw.clear();
    grey_bg.clear();
    grey_working.clear();
    grey_beam_working.clear();

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
    cvZero(grey_beam_working.getCvImage());

    for(ofxCvGrayscaleImage& mask : beam_masks)
    {
        if(mask.bAllocated)
        {
            cvOr(grey_beam_working.getCvImage(),
                 mask.getCvImage(),
                 grey_beam_working.getCvImage());
            grey_beam_working.flagImageChanged();
        }
    }

    grey_beam_working.draw(x, y);
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
    //save the mask to a file
    ofImage mask;
    mask.setFromPixels(beam_masks[learning].getPixels());
    mask.setImageType(OF_IMAGE_GRAYSCALE);

    stringstream filename;
    filename << cam_name << "/" << learning << ".png";
    mask.save(filename.str());

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
    contourFinder.findContours(grey_beam_working, 100, (WIDTH*HEIGHT)/4, 10, false);
    return contourFinder.blobs;
}
