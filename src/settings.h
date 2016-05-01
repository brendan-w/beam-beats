
#pragma once


#define FRAMERATE 60
#define WIDTH 320
#define HEIGHT 240

#define BLOB_AREA_MIN 100
#define BLOB_AREA_MAX (WIDTH * HEIGHT / 4)
#define N_BLOBS 10

#define INIT_THRESHOLD 128
#define BLUR 11
#define BACKGROUND_FILE "background"
#define IMAGE_FORMAT "png"
#define BEAM_BAR_DIVISOR 3
#define THRESHOLD_INCREMENT 2

const int midi_velocities[] = { 32, 64, 96, 127 };
const int midi_scale[] = { 0, 3, 5, 7, 10, 12 }; //pentatonic

#define sizeof_array(a) (sizeof(a)/sizeof(a[0]))
#define SCALE_SIZE sizeof_array(midi_scale)
#define VEL_SIZE sizeof_array(midi_scale)
