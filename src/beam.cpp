
#include <cmath>
#include "beam.h"


Beam::Beam(int channel, int octave) : channel(channel), octave(octave)
{
    //zero out our notes
    for(bool& note : notes)
        note = false;
}

vector<Note> Beam::update(vector<Hand> hands)
{

}
