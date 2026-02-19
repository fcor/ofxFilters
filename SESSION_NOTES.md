# CvSketch2 - Session Notes

## Overview
Webcam effect system built in openFrameworks with toggleable/layered effects and controllable parameters.

---

## Current Architecture

```
Camera -> [Frame Buffer] -> [Slitscan] -> [Wave] -> [Block] -> [RGB] -> Output
               |                |            |          |         |
            60 frames        toggle       toggle     toggle    toggle
                            + params     + params   + params  + params
```

- Frame buffer stores last 60 frames (for slitscan and future effects)
- Effects are processed in order, each modifying srcRow/srcCol/srcFrame
- All effects can be toggled on/off independently
- Parameters controllable via keyboard (ready for MIDI/OSC)

---

## Keyboard Controls

| Key | Action |
|-----|--------|
| `1` | Toggle Wave effect |
| `2` | Toggle RGB separation |
| `3` | Toggle Slitscan |
| `4` | Toggle Block Displacement |
| `q` / `a` | Slitscan depth +/- |
| `w` / `s` | Block size +/- |
| `e` / `d` | Block amount +/- |

---

## Effects Implemented

### 1. Wave Distortion
- Sine wave displacement on both axes
- Parameters: `waveSpeed`, `waveHAmount`, `waveVAmount`
- Creates organic, fluid movement

### 2. RGB Channel Separation (Chromatic Aberration)
- Red channel shifts left, blue shifts right, green stays centered
- Parameter: `rgbShiftAmount`
- Mimics old CRT TV misalignment

### 3. Slitscan
- Each row samples from a different frame in the buffer
- Parameter: `slitscanDepth` (how many frames back)
- Creates time-smearing trails on movement
- Using `row` = horizontal bands, using `col` = vertical bands

### 4. Block Displacement
- Divides image into blocks, shifts entire blocks together
- Parameters: `blockSize`, `blockAmount`
- Creates chunky/glitchy digital aesthetic
- Key difference from wave: integer division groups pixels

---

## Effects Not Yet Implemented

### From Original List:
- **Center-based Distortion** - distortion increases from center outward (lens effect)
- **Feedback Loop** - blend current frame with previous output, creates trails/ghosting
- **Edge-reactive Distortion** - only distort high contrast areas
- **Scanlines + Color Shift** - CRT/VHS aesthetic, darken every Nth row

### New Ideas:
- **Pixelation/mosaic** - reduce resolution in blocks
- **Mirror/kaleidoscope** - reflect portions of image
- **Color posterization** - reduce color palette
- **Noise/static overlay** - TV static effect
- **Zoom blur from center** - radial blur

---

## Key Concepts Learned

### Slitscan Math
- `frameOffset = (row * slitscanDepth) / camHeight`
- Same row = same frameOffset = horizontal bands
- Same col = same frameOffset = vertical bands
- The variable you use (row/col) becomes the "time axis"

### Block Displacement Math
- `blockX = srcCol / blockSize` (integer division truncates)
- Pixels 0-15 all get blockX=0, pixels 16-31 get blockX=1, etc.
- All pixels in a block share the same shift value
- Wave = continuous/smooth, Block = discrete/stepped

### Why Old TVs Had RGB Separation
- 3 separate electron guns for R, G, B
- Magnetic interference bent beams differently
- Aging components drifted out of alignment
- Analog signal timing issues

---

## Future Considerations

### TouchOSC / MIDI Control
- Add `ofxOsc` addon for OSC messages
- Map sliders/buttons to effect parameters
- Example: `/wave/speed` -> `waveSpeed`

### Web Version (p5.js)
- Code translates almost 1:1 to p5.js
- For better performance, use WebGL shaders
- Hybrid approach: p5.js structure + GLSL shaders

---

## File Structure
```
src/
  ofApp.h      - declarations, effect toggles, parameters
  ofApp.cpp    - setup, update loop with effects, draw, keyboard
```

## Camera Settings
- Resolution: 640x480
- Device ID: 1
- Aspect ratio preserved when scaling to window

---

## Next Session Ideas
1. Add feedback loop effect (trails/ghosting)
2. Set up TouchOSC/MIDI control
3. Add on-screen parameter display
4. Experiment with effect order (changes results!)
5. Try combining multiple effects for new looks
