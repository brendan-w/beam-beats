
#pragma once
#include <cmath>
#include <ofPoint.h>

ofPoint rotate_point(ofPoint point, ofPoint center, float angle)
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
