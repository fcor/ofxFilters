#pragma once

#include <string>
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------------------
// PixelContext — passed through the effect chain per pixel.
// Destination is read-only; effects modify the source coordinates.
// ---------------------------------------------------------------------------
struct PixelContext {
    const int dstRow, dstCol, channel;  // where we're writing (immutable)
    int srcRow, srcCol, srcFrame;        // where we read from (effects modify)
    const float time;
    const int camW, camH;

    PixelContext(int dR, int dC, int ch,
                 int sR, int sC, int sF,
                 float t, int w, int h)
        : dstRow(dR), dstCol(dC), channel(ch),
          srcRow(sR), srcCol(sC), srcFrame(sF),
          time(t), camW(w), camH(h) {}
};

// ---------------------------------------------------------------------------
// EffectModule — base class for all effect modules.
// ---------------------------------------------------------------------------
struct EffectModule {
    bool enabled = false;
    std::string name;

    virtual void transform(PixelContext& ctx) = 0;
    virtual ~EffectModule() = default;
};

// ---------------------------------------------------------------------------
// WaveEffect — sine-wave displacement on both axes.
// Keys: 1 toggles. No param keys assigned here (future).
// ---------------------------------------------------------------------------
struct WaveEffect : EffectModule {
    float speed   = 3.0f;
    float hAmount = 6.0f;
    float vAmount = 10.0f;

    WaveEffect() { name = "Wave"; }

    void transform(PixelContext& ctx) override {
        int hShift = (int)(hAmount * std::sin(ctx.srcRow * 0.03f + ctx.time * speed));
        int vShift = (int)(vAmount * std::sin(ctx.srcCol * 0.02f + ctx.time * speed * 0.7f));
        ctx.srcCol = (ctx.srcCol + hShift + ctx.camW) % ctx.camW;
        ctx.srcRow = (ctx.srcRow + vShift + ctx.camH) % ctx.camH;
    }
};

// ---------------------------------------------------------------------------
// SlitscanEffect — each column samples a different frame, creating time trails.
// Keys: 3 toggles. q/a adjust depth.
// ---------------------------------------------------------------------------
struct SlitscanEffect : EffectModule {
    int depth;
    const int numFrames;

    SlitscanEffect(int nFrames) : depth(30), numFrames(nFrames) { name = "Slitscan"; }

    void transform(PixelContext& ctx) override {
        int frameOffset = (ctx.dstCol * depth) / ctx.camH;
        ctx.srcFrame = (ctx.srcFrame - frameOffset + numFrames) % numFrames;
    }
};

// ---------------------------------------------------------------------------
// BlockDisplaceEffect — divides image into blocks, shifts blocks together.
// Keys: 4 toggles. w/s adjust blockSize. e/d adjust blockAmount.
// ---------------------------------------------------------------------------
struct BlockDisplaceEffect : EffectModule {
    int   blockSize   = 16;
    float blockAmount = 10.0f;

    BlockDisplaceEffect() { name = "BlockDisplace"; }

    void transform(PixelContext& ctx) override {
        int blockX = ctx.srcCol / blockSize;
        int blockY = ctx.srcRow / blockSize;
        int shiftX = (int)(blockAmount * std::sin(blockY * 0.5f + ctx.time * 2.0f));
        int shiftY = (int)(blockAmount * 0.5f * std::sin(blockX * 0.3f + ctx.time * 1.5f));
        ctx.srcCol = (ctx.srcCol + shiftX + ctx.camW) % ctx.camW;
        ctx.srcRow = (ctx.srcRow + shiftY + ctx.camH) % ctx.camH;
    }
};

// ---------------------------------------------------------------------------
// RgbSplitEffect — per-channel horizontal offset (chromatic aberration).
// Keys: 2 toggles.
// ---------------------------------------------------------------------------
struct RgbSplitEffect : EffectModule {
    int shiftAmount = 7;

    RgbSplitEffect() { name = "RgbSplit"; }

    void transform(PixelContext& ctx) override {
        if (ctx.channel == 0) {
            ctx.srcCol = (ctx.srcCol - shiftAmount + ctx.camW) % ctx.camW;
        } else if (ctx.channel == 2) {
            ctx.srcCol = (ctx.srcCol + shiftAmount) % ctx.camW;
        }
    }
};
