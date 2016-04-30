
#include "beamDescriptor.h"


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

bool BeamDescriptor::found_beam()
{
    return (blob.pts.size() > 0);
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

Hand BeamDescriptor::blob_to_hand(ofxCvBlob blob)
{
    ofPoint h = blob.centroid;
    ofPoint a = top - bottom;
    ofPoint b = h - bottom;

    //pixel location of the perpedicular intersection
    ofPoint intersection = a;
    intersection.normalize();
    intersection *= intersection.dot(b);;
    intersection += bottom;

    //compute the beam-normalized hand value
    ofPoint beam_pos;
    beam_pos.x = (h - intersection).length() / (width / 2.0);
    beam_pos.x *= left_or_right(h);
    beam_pos.y = (intersection - bottom).length() / height;

    Hand hand(beam_pos, intersection, blob);

    return hand;
}

float BeamDescriptor::left_or_right(ofPoint p)
{
    float determinant = (top.x - bottom.x) * (p.y - bottom.y) -
                        (top.y - bottom.y) * (p.x - bottom.x);
    return (determinant < 0.0) ? -1.0 : 1.0;
}

ofPoint BeamDescriptor::rotate_point(ofPoint point, ofPoint center, float angle)
{
    //convert degrees to radians
    angle = angle * M_PI / 180;

    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    point.x -= center.x;
    point.y -= center.y;

    // rotate point
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;

    // translate point back:
    point.x = xnew + center.x;
    point.y = ynew + center.y;

    return point;
}

