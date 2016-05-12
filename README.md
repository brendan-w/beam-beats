Beam Beats
==========

Making music with light


```
\\ //    \\ //    \\ //    \\ //
 ===      ===      ===      ===
 |                 |||
 |                 |||
 |        ___      |||
 |        |||      |||
 |        |||      |||
 |        |||      |||
 |        |||      |||      ___
 |        |||      |||      |||
 |        |||      |||      |||
 |        |||      |||      |||
 |__      |||      |||      |||
 |||      |||      |||      |||
 |||      |||      |||      |||
 |||      |||      |||      |||
 |||      |||      |||      |||
 |||      |||      |||      |||
 ===      ===      ===      ===
// \\    // \\    // \\    // \\
```

Beam Beats is a musical instrument that is controlled by placing your hands in beams of light (like a laser harp, but with more expression). By mapping the height of a user's hand to a fixed scale, multiple beams can be used to produce sound via MIDI. This repository contains a computer vision system for optically detecting hands in multiple beams, and for producing the corresponding MIDI data. The beams of light are intended to be produced by a colimated digital projector. In such a setup, this software will also produce beam animations that respond to the user's interactions with the instrument.

Building & Running
------------------

Beam Beats is an [OpenFrameworks](http://openframeworks.cc/) application developed to run on Fedora Linux.

- Install and compile OpenFrameworks
- Install dependent OpenFrameworks addons
  - [ofxPS3EyeGrabber](https://github.com/bakercp/ofxPS3EyeGrabber)
  - [ofxMidi](https://github.com/danomatika/ofxMidi)
  - [custom tweaks to ofxOpenCv](https://github.com/brendan-w/ofxOpenCv)
- Run `make` to build `bin/beam-beats`
- Plug in a USB MIDI adapter
- Plug in the USB PS3 Eye cameras
- Unload the default Linux camera driver with `sudo modprobe -r gspca_ov534`. OR, a better solution is to blacklist the module entirely with `echo "blacklist gspca_ov534" >> /etc/modprobe.d/blacklist-ps3eye.conf`
- Run `./bin/beam-beats` as root (libusb needs priveleges to access your USB)
- Resize the application to cover all of your attached screens/projectors


Usage & Training
----------------

In order for the computer to know where each beam of light is in the cameras view, it must be taught. Beam beats is built to learn multiple vertical beams, and to perform bright object hand-detection in those beams. Beams may be viewed by different cameras.

Beam Beats is currently setup to handle 2 PS3 Eye cameras and 4 beams. If you wish to change this, do so now, by editting [ofApp.cpp here](https://github.com/brendan-w/beam-beats/blob/master/src/ofApp.cpp#L30-L36) (I apologize, I didn't have time to implement a config file system).

First, setup you cameras such that they can view the entirety of their respective beams. The cameras must be able to see the lit side of a users hand. While the application can tolerate most camera angles, it currently performs no perspective correction, so care must be taken to *not* place the cameras at unreasonbly severe angles. Our setup used cameras tilted at 45° relative to the beams with tolerable perspective distortion.

Once cameras are mounted, you can begin training with your beams. Launch the application, resize it to cover all of your projectors, and press `TAB` to enter running mode. This will project light for each of your defined beams. Make sure that no objects are in any of the beams, and capture a background frame by pressing `SPACE`. This background frame will help ignore ambient room light and fog effects. To "train" a beam is actually to construct a black white mask of the beams location in the camera's view. To train a single beam:

- Press the number key (`1-9`) for the beam you want to train. This will erase the previous training data, and will begin watching for an object in the beam.
- Slowly pass a large, flat object through the length of the beam. Direction does not matter. You are essentially painting the beams location in the camera's view (as a mask).
- Press `ENTER` when done, and the new beam mask will be written to disk in each camera's data/ folder.
- Press `TAB` to switch back to status mode. Camera's are listed in rows. The live feed is shown on the left, and the trained masks are collectively shown on the right. Take a look at the mask that you just trained, and gut-check it. If the mask appears too lean, or if it is missing sections, you may need to lower the brightness threshold of object detection (this can be done with the `UP/DOWN` keys). Conversely, if the mask is billowing wider than the actual beam, you may need to increase this threshold.
- Press `TAB` again to go back into running mode, and repeat for each of your beams, as many times as you need.

If you wish to edit these masks rather than recapturing them, they are simply numbered `.png`s in the app's data/ directory. You may use any photo editor to paint/fix these masks. When done, simply restart the application to reload them.

Besides adjusting the threshold, the PS3 Eye cameras also have onboard exposure control. This can be adjusted with the `LEFT/RIGHT` keys. By default the exposure is set extremely low to help ignore fog effects. While normal theatrical haze doesn't usually pose a problem, other effects such as dry-ice can produce clouds that may be mistaken as hands. If the instrument begins "playing itself" with fog, the usual tactic is to lower the exposure slightly, and re-capture a background frame.

Instrument Controls
-------------------

Once running properly, your beams should respond to the following:

- vertical position (hand height): pitch across a fixed pentatonic scale of 6 notes
- horizontal velocity (hand shake): vibrato

`bin/beam-beats` will print note ON/OFF data to your terminal. If you wish to preview the raw MIDI data (that includes pitch bend information), it is helpful run [MIDIOX](http://www.midiox.com/) on a windows computer.

Key Bindings
------------

- `TAB` switches between status mode and running mode.
- `F1` in status mode, toggles between the raw camera view, and the threshold (blob) view.
- `SPACE` captures a background frame for background subtraction. Useful for ignoring ambient room light and fog effects. Writes the new background frame to disk
- `LEFT/RIGHT` decreases/increases the exposure setting of the PS3 Eye cameras.
- `UP/DOWN` increases/decreases the brightness threshold for blob (hand) detection.
- `1-9` starts learning beam number `N` (builds a mask, and locates the start/end-points of a beam)
- `ENTER` stops learning beams, and writes any new training data to disk
- `ESC` quits beam-beats

