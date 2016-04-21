
#include <cmath>
#include "beam.h"


Beam::Beam(int beam, int octave) : beam(beam), octave(octave)
{

}

Beam::~Beam()
{

}

void Beam::read_blobs(vector<ofxCvBlob> blobs)
{

	//now that we've processed these, shift them into history
	old_blobs = blobs;
}

vector<Note> Beam::get_midi()
{

}
