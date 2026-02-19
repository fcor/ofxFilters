# ofxFilters

A real-time webcam effect system built with openFrameworks. The vision is a composable filter machine — like analog modular synthesis, where patching transformations together produces a visual result.

## Current Effects

| Key | Effect |
|-----|--------|
| `1` | Wave — sinusoidal pixel displacement |
| `2` | RGB Split — per-channel horizontal offset |
| `3` | Slitscan — samples across a rolling frame buffer |
| `4` | Block Displace — grid-based spatial distortion |

### Parameters

| Key | Action |
|-----|--------|
| `q` / `a` | Slitscan depth up / down |
| `w` / `s` | Block size up / down |
| `e` / `d` | Block amount up / down |

## Architecture

Effects operate as coordinate transforms — each modifies **where to sample from** rather than the pixel value directly. They chain in order, output feeding into the next, like signal flow in a modular synth.

Planned: a full `EffectModule` / `Renderer` split to make the pipeline reorderable, extensible, and ready for an ASCII renderer layer.
