# Modular Effect Architecture + ASCII Renderer

## Vision

Composable filter machine — like analog modular synthesis. Patching a set of transformations together generates a visual result. Effects chain in order; renderers display the result independently.

---

## Core Problem with Current Architecture

The current code conflates two concerns in one monolithic loop:
1. **Signal processing** (coordinate transforms that distort where pixels sample from)
2. **Output rendering** (how the processed buffer is displayed)

This prevents clean composition: can't reorder effects, can't add new renderers, can't run effects in parallel, no feedback loops.

---

## Two-Layer Architecture

### Layer 1 — Effect Pipeline (`update()`)

Each effect modifies **where to sample from** (`srcRow`, `srcCol`, `srcFrame`). Chains compose like signal flow in a synth — output of one feeds into the next.

```
Camera → [Slitscan] → [Wave] → [Block] → [RGB] → effectData buffer
              ↑ vector<EffectModule*>: iterate, each modifies PixelContext
```

### Layer 2 — Renderer Pipeline (`draw()`)

Renderers read from `effectData` and display independently. Multiple can be active and layered.

```
effectData → [TextureRenderer]    raw pixel output (current)
           → [AsciiRenderer]      NEW: ASCII art
           → [future renderers...]
```

---

## New Abstractions

### `PixelContext`

Passed through the chain per pixel. Immutable destination, mutable source.

```cpp
struct PixelContext {
    const int dstRow, dstCol, channel;  // where we're writing (read-only)
    int srcRow, srcCol, srcFrame;        // where we read from (effects modify this)
    const float time;
    const int camW, camH;
};
```

### `EffectModule` (base class)

```cpp
struct EffectModule {
    bool enabled = false;
    std::string name;
    virtual void transform(PixelContext& ctx) = 0;
};
```

Concrete modules (refactored from current if-blocks):
- `WaveEffect`
- `SlitscanEffect`
- `BlockDisplaceEffect`
- `RgbSplitEffect`

**New update loop becomes:**
```cpp
for (auto& effect : effectChain)
    if (effect->enabled) effect->transform(ctx);
```

Single-pass performance is preserved. Pipeline is now reorderable and extensible.

### `Renderer` (base class)

```cpp
struct Renderer {
    bool enabled = false;
    std::string name;
    virtual void render(unsigned char* data, int w, int h,
                        float dispX, float dispY, float dispW, float dispH) = 0;
};
```

---

## ASCII Renderer Design

### Performance: per-row draw calls (NOT per-character)

At 6×10 cells on 640×480 = 5,136 characters across 48 rows:
- Per-char calls: ~154M cycles/sec overhead → too slow
- **Per-row calls: 48 calls → fast** (107× fewer)

Sample `effectData` at cell centers → brightness → char → build string per row → one `ofDrawBitmapString` per row.

### Color Modes

| Mode | Calls/frame | Look |
|------|-------------|------|
| Monochrome (default) | 48 | CRT green/amber/white |
| Row-tinted | 48 setColor + 48 draw | Color tinted rows |
| Per-char tinted | 5,136 draw | Fully colorized (opt-in) |

### Parameters

```cpp
struct AsciiRenderer : Renderer {
    int cellW = 8, cellH = 10;
    int colorMode = 0;      // 0=mono, 1=row-tinted, 2=per-char
    int charSetIndex = 0;

    const vector<string> charSets = {
        " .:-=+*#%@",    // standard 10 levels
        " .+*#@",        // sparse/bold
        " ░▒▓█",         // block elements
        " .-+oO0@#",     // organic
    };
};
```

---

## Files to Create / Modify

| File | Action |
|------|--------|
| `src/effects.h` | NEW — `PixelContext`, `EffectModule` base + 4 effect classes (header-only) |
| `src/renderers.h` | NEW — `Renderer` base + `TextureRenderer` + `AsciiRenderer` (header-only) |
| `src/ofApp.h` | MODIFY — replace bool flags + loose params with `effectChain` and `renderers` vectors |
| `src/ofApp.cpp` | MODIFY — refactor `update()` loop, `draw()`, `keyPressed()` |

---

## New Keyboard Controls

| Key | Action |
|-----|--------|
| `5` | Toggle ASCII renderer |
| `6` | Cycle ASCII color mode (mono → row-tinted → per-char) |
| `[` / `]` | ASCII cell size smaller / larger |
| `,` / `.` | Cycle char set preset |

Existing keys `1`–`4`, `q/a/w/s/e/d` remain, now addressing module params.

---

## What This Unlocks

- **Reorder effects**: `swap(effectChain[0], effectChain[1])` — changes result, like re-patching a synth
- **Parallel effects + blend**: add `BlendEffect` with two sub-chains, mix outputs
- **New renderers**: implement `Renderer`, push to `renderers`, done
- **Feedback loop**: add `FeedbackEffect` that reads from previous frame's `effectData`
- **GPU migration**: swap `EffectModule::transform()` with GLSL shaders, same interface

---

## Verification Checklist

- [ ] `make RunRelease` builds and runs
- [ ] Keys `1`–`4` still toggle original effects
- [ ] Key `5` toggles ASCII renderer on/off
- [ ] Key `6` cycles color modes
- [ ] `[` / `]` changes character density
- [ ] `,` / `.` cycles character sets
- [ ] All original effects compose correctly in any combination
