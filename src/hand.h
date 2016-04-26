
#pragma once

#include <math.h>

//sane bounds for how fast a hand can travel
#define HAND_MAX_VX 0.1
#define HAND_MAX_VY 0.05


class Hand
{
public:
    float x;
    float y;
    float vx;
    float vy;

    void compute_velocity(Hand& old_hand)
    {
        vx = x - old_hand.x;
        vy = y - old_hand.y;
    };

    bool same_hand_as(Hand& other)
    {
        return (abs(x - other.x) <= HAND_MAX_VX &&
                abs(y - other.y) <= HAND_MAX_VY);
    };
};
