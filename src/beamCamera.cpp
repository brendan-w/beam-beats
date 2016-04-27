
#include <cmath>
#include "ofxPS3EyeGrabber.h"
#include "beamCamera.h"
#include "utils.h"


BeamDescriptor::BeamDescriptor()
{
    mask.allocate(WIDTH, HEIGHT);
    zero();
}

BeamDescriptor::BeamDescriptor(ofImage& image)
{
    //learn from an existing image (saved beam mask)
    mask.allocate(WIDTH, HEIGHT);
    mask = image;
    learn();
}

BeamDescriptor::~BeamDescriptor()
{
    mask.clear();
}

void BeamDescriptor::zero()
{
    cvZero(mask.getCvImage());
    mask.flagImageChanged();
}

void BeamDescriptor::learn()
{
    find_blob();
    if(blob.pts.size() > 0)
        find_details();
}

void BeamDescriptor::find_blob()
{
    ofxCvContourFinder contourFinder;
    contourFinder.findContours(mask,
                               BLOB_AREA_MIN,
                               (WIDTH * HEIGHT), //allow large blobs
                               1, //ofxOpenCv sorts for the largest blob
                               false); //find holes

    if(contourFinder.blobs.size() == 0)
    {
        ofLog() << "No beam mask detected";
        blob = ofxCvBlob(); //null blob
        return;
    }

    blob = contourFinder.blobs[0];
}

void BeamDescriptor::find_details()
{
    //compute endpoints of the blob
    width = blob.minRect.size.width;
    height = blob.minRect.size.height;
    top.x = bottom.x = blob.minRect.center.x;
    top.y = bottom.y = blob.minRect.center.y;

    //The rectangle may be specified in any orientation.
    //Assume that the longer (major) axis is the axis of our beam
    if(height > width)
    {
        top.y    = blob.minRect.center.y - (height / 2.0);
        bottom.y = blob.minRect.center.y + (height / 2.0);
    }
    else //width vs. height are backwards
    {
        top.x    = blob.minRect.center.x + (width / 2.0);
        bottom.x = blob.minRect.center.x - (width / 2.0);

        //swap, to make it semantic
        float tmp = width;
        width = height;
        height = tmp;
    }

    top = rotate_point(top,
                       ofPoint(blob.minRect.center.x, blob.minRect.center.y),
                       blob.minRect.angle);
    bottom = rotate_point(bottom,
                          ofPoint(blob.minRect.center.x, blob.minRect.center.y),
                          blob.minRect.angle);
}


void BeamDescriptor::add_to_mask(ofxCvGrayscaleImage partial)
{
    cvOr(partial.getCvImage(),
         mask.getCvImage(),
         mask.getCvImage());

    mask.flagImageChanged();
}

Hand BeamDescriptor::blob_to_hand(ofxCvBlob hand)
{
    //ofVec3f h = hand.centroid;
    ofPoint h(100, 100, 0);
    ofPoint a = top - bottom;
    ofPoint b = h - bottom;

    ofPoint intersection = a;
    intersection.normalize();
    intersection *= intersection.dot(b);;
    intersection += bottom;

    //compute the beam-normalized hand value
    ofPoint beam_pos;
    beam_pos.x = (h - intersection).length() / (width / 2.0);
    beam_pos.y = (intersection - bottom).length() / height;

    Hand output(beam_pos, hand.centroid, intersection);

    //"a" is now the pixel location of the perpedicular intersection

    ofPushStyle();
    ofFill();
    ofSetHexColor(0x00FF00);
    ofDrawCircle(h.x, h.y, 3);
    ofDrawLine(h.x, h.y, intersection.x, intersection.y);
    ofPopStyle();

    return Hand();
}






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

        //apply our intensity threshold
        grey_working.threshold(threshold);

        if(is_learning() && beams[learning] != NULL)
        {
            beams[learning]->add_to_mask(grey_working);
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
            beam->blob.draw();
            ofPushStyle();
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
    ofLog() << cam_name << " started learning beam " << beam;
    new_beam(beam);
    learning = beam;
}

void BeamCamera::stop_learning_beam()
{
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
    vector<Hand> hands;
    //return early if there's nothing to process
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

    hands.push_back(beams[beam]->blob_to_hand(ofxCvBlob()));

    /*
    for(ofxCvBlob blob : contourFinder.blobs)
    {
        hands.push_back(beams[beam]->blob_to_hand(blob));
    }
    */

    //TODO: compute velocity

    return hands;
}

bool BeamCamera::mask_exists(int beam)
{
    return (beam < (int)beams.size()) && (beams[beam] != NULL);
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
