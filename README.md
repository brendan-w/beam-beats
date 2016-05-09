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

Beam Beats is a musical instrument that is controlled by placing your hands in beams of light (like a laser harp, but with more expression). By mapping the height of a user's hand to a fixed scale, multiple beams can be used to produce sound. This repository contains a computer vision system for optically detecting hands in multiple beams, and for producing the corresponding MIDI data. The beams of light are intended to be produced by a colimated digital projector. In such a setup, this software will also produce beam animations that respond to the user's interactions with the instrument.

Dependencies
------------

- [ofxPS3EyeGrabber](https://github.com/bakercp/ofxPS3EyeGrabber)
- [ofxMidi](https://github.com/danomatika/ofxMidi)
- [custom tweaks to ofxOpenCv](https://github.com/brendan-w/ofxOpenCv)


Controls
--------

- `TAB` switches between status mode and running mode.
- `F1` in status mode, toggles between the raw camera view, and the threshold (blob) view.
- `SPACE` capture a background frame for background subtraction. Useful for ignoring ambient room light and fog effects. Writes the new background frame to disk
- `LEFT/RIGHT` decreases/increases the exposure setting of the PS3 Eye cameras.
- `UP/DOWN` increases/decreases the brightness threshold for blob (hand) detection.
- `1-9` start learning beam number `N` (builds a mask, and locates the start/end-points of a beam)
- `ENTER` stops learning beams, and writes any new training data to disk
- `ESC` quits beam-beats
