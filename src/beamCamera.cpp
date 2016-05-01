
#include <cmath>
#include "ofxPS3EyeGrabber.h"
#include "beamCamera.h"
#include "settings.h"



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

    load_data();
}

BeamCamera::~BeamCamera()
{
    //release our surfaces
    raw.clear();
    grey_bg.clear();
    grey_working.clear();
    grey_beam_working.clear();

    for(BeamDescriptor* beam : beams)
    {
        if(beam != NULL)
            delete beam;
    }
}

void BeamCamera::load_data()
{
    ofDirectory dir(cam_name);
    if(!dir.exists())
    {
        dir.create();
        return;
    }

    dir.allowExt(IMAGE_FORMAT);
    dir.listDir();

    for(ofFile file : dir)
    {
        ofImage img;

        if(file.getBaseName() == BACKGROUND_FILE)
        {
            img.load(file);
            grey_bg = img;
        }
        else
        {
            int beam = ofToInt(file.getBaseName());

            if(beam < 0)
                continue;

            img.load(file);
            //make sure our mask array has a spot for this beam
            if(beam >= (int) beams.size())
                beams.resize(beam + 1, NULL);

            beams[beam] = new BeamDescriptor(img);
        }
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
        grey_working.blur(BLUR);
        //apply our intensity threshold
        grey_working.threshold(threshold);
        //grey_working.blur(BLUR);

        if(is_learning() && beams[learning] != NULL)
        {
            beams[learning]->add_to_mask(grey_working);
        }
    }
}

void BeamCamera::draw_raw(int x, int y)
{
    ofSetHexColor(0xFFFFFF);
    raw.draw(x, y);
}

void BeamCamera::draw_working(int x, int y)
{
    ofSetHexColor(0xFFFFFF);
    grey_working.draw(x, y);
}

void BeamCamera::draw_masks(int x, int y)
{
    ofSetHexColor(0xFFFFFF);
    cvZero(grey_beam_working.getCvImage());

    for(BeamDescriptor* beam : beams)
    {
        if(beam != NULL)
        {
            cvOr(beam->mask.getCvImage(),
                 grey_beam_working.getCvImage(),
                 grey_beam_working.getCvImage());
        }
    }

    grey_beam_working.flagImageChanged();
    grey_beam_working.draw(x, y);

    for(BeamDescriptor* beam : beams)
    {
        if(beam != NULL)
        {
            ofPushStyle();
            beam->blob.draw(x, y);
            ofSetHexColor(0xFF0000);
            ofDrawCircle(beam->top.x, beam->top.y, 3);
            ofSetHexColor(0x0000FF);
            ofDrawCircle(beam->bottom.x, beam->bottom.y, 3);
            ofPopStyle();
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

    //save the background to a file
    ofImage background;
    background.setFromPixels(grey_bg.getPixels());
    background.setImageType(OF_IMAGE_GRAYSCALE);

    stringstream filename;
    filename << cam_name << "/" << BACKGROUND_FILE << "." << IMAGE_FORMAT;
    background.save(filename.str());
}

bool BeamCamera::is_learning()
{
    return learning != NOT_LEARNING;
}

void BeamCamera::start_learning_beam(int beam)
{
    //if we were learning before, stop, before moving on
    if(learning != NOT_LEARNING)
        stop_learning_beam();

    ofLog() << cam_name << " started learning beam " << beam;
    new_beam(beam);
    learning = beam;
}

void BeamCamera::stop_learning_beam()
{
    //can't stop a stopped learning
    if(learning == NOT_LEARNING)
        return;

    //compute and save the minimum area rect
    BeamDescriptor* beam = beams[learning];
    beam->learn();

    //save the mask to a file
    ofImage mask;
    mask.setFromPixels(beam->mask.getPixels());
    mask.setImageType(OF_IMAGE_GRAYSCALE);

    stringstream filename;
    filename << cam_name << "/" << learning << "." << IMAGE_FORMAT;
    mask.save(filename.str());

    ofLog() << cam_name << " stopped learning beam " << learning;
    learning = NOT_LEARNING;
}

vector<Hand> BeamCamera::hands_for_beam(int beam)
{
    vector<Hand> all_hands;
    vector<Hand> hands_with_velocity;

    //return empty array if this camera doesn't handle a beam
    if(!mask_exists(beam))
        return vector<Hand>();

    //apply the mask that corresponds to this beam
    cvAnd(grey_working.getCvImage(),
          beams[beam]->mask.getCvImage(),
          grey_beam_working.getCvImage());
    grey_beam_working.flagImageChanged();

    //find our hand blobs
    contourFinder.findContours(grey_beam_working,
                               BLOB_AREA_MIN,
                               BLOB_AREA_MAX,
                               N_BLOBS,
                               false); //find holes

    //contourFinder.blobs is now populated

    for(ofxCvBlob blob : contourFinder.blobs)
    {
        Hand hand = beams[beam]->blob_to_hand(blob);

        //figure out where this hand was in the past
        for(Hand& old_hand : beams[beam]->old_hands)
        {
            if(hand.same_hand_as(old_hand))
            {
                hand.compute_velocity(old_hand);
                hands_with_velocity.push_back(hand);
                break;
            }
        }

        all_hands.push_back(hand);
    }

    beams[beam]->old_hands = all_hands;

    //return hands_with_velocity;
    return all_hands;
}

bool BeamCamera::mask_exists(int beam)
{
    return (beam < (int)beams.size()) && (beams[beam] != NULL);
}

bool BeamCamera::handles_beam(int beam)
{
    return mask_exists(beam) && beams[beam]->found_beam();
}

void BeamCamera::new_beam(int beam)
{
    //make sure our mask array has a spot for this beam
    if(beam >= (int) beams.size())
        beams.resize(beam + 1, NULL);

    if(mask_exists(beam))
        beams[beam]->zero();
    else
        beams[beam] = new BeamDescriptor();
}
