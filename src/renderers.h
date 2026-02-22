#pragma once

#include "ofMain.h"
#include <string>
#include <vector>
#include <algorithm>

// ---------------------------------------------------------------------------
// Renderer — base class. Reads from effectData and draws to screen.
// ---------------------------------------------------------------------------
struct Renderer {
    bool        enabled = false;
    std::string name;

    virtual void render(unsigned char* data, int w, int h,
                        float dispX, float dispY, float dispW, float dispH) = 0;
    virtual ~Renderer() = default;
};

// ---------------------------------------------------------------------------
// TextureRenderer — uploads effectData to a GPU texture and draws it.
// ---------------------------------------------------------------------------
struct TextureRenderer : Renderer {
    ofTexture texture;

    TextureRenderer() { name = "Texture"; enabled = true; }

    void allocate(int w, int h) {
        texture.allocate(w, h, GL_RGB);
    }

    void render(unsigned char* data, int w, int h,
                float dispX, float dispY, float dispW, float dispH) override {
        texture.loadData(data, w, h, GL_RGB);
        ofSetColor(255);
        texture.draw(dispX, dispY, dispW, dispH);
    }
};

// ---------------------------------------------------------------------------
// AsciiRenderer — maps effectData brightness to ASCII characters.
//
// cellW / cellH = pixel dimensions of each character cell on screen.
//   - cellW: also the horizontal spacing between chars. Default 8 matches
//     the bitmap font's natural glyph width (8px), giving a tight grid.
//     Increase for coarser, blockier output; decrease to pack more chars.
//   - cellH: vertical spacing between rows. Default 14 gives a small gap
//     above the ~13px-tall bitmap glyphs so rows don't visually collide.
//   - numCols = dispW / cellW,  numRows = dispH / cellH
//   Each char is sampled from the centre of its cell in effectData,
//   and drawn at exactly (dispX + c*cellW, dispY + (r+1)*cellH).
//
// Color modes:
//   0 = monochrome (CRT green)  — color set once per frame
//   1 = row-tinted (center px)  — color set once per row, sampled from center column
//   2 = per-char color          — color set per character
//
// Keys: 5=toggle, 6=cycle mode, m/n=cellW -/+, ,/.=char set
// ---------------------------------------------------------------------------
struct AsciiRenderer : Renderer {
    int cellW = 8;
    int cellH = 14;
    int colorMode    = 0;
    int charSetIndex = 0;

    const std::vector<std::string> charSets = {
        " .:-=+*#%@",   // standard 10 levels
        " .+*#@",        // sparse / bold
        " .+#@W",        // dense / grid (ASCII-only)
        " .-+oO0@#",     // organic
    };

    AsciiRenderer() { name = "Ascii"; }

    void render(unsigned char* data, int w, int h,
                float dispX, float dispY, float dispW, float dispH) override {

        const std::string& chars = charSets[charSetIndex % (int)charSets.size()];
        int numLevels = (int)chars.size() - 1;

        int numCols = std::max(1, (int)(dispW / cellW));
        int numRows = std::max(1, (int)(dispH / cellH));

        // Set color once for monochrome mode
        if (colorMode == 0) ofSetColor(0, 255, 0);

        for (int r = 0; r < numRows; r++) {
            float drawY = dispY + (r + 1) * cellH;  // Y = baseline of this row

            // Row-tinted: sample from the center column of this row.
            // Averaging across all columns cancels complementary colors → gray;
            // a single center sample preserves actual hue.
            if (colorMode == 1) {
                int sy  = ofClamp((int)((r + 0.5f) * h / numRows), 0, h - 1);
                int idx = (sy * w + w / 2) * 3;
                ofSetColor(data[idx], data[idx + 1], data[idx + 2]);
            }

            // Draw each character at its exact cell position
            for (int c = 0; c < numCols; c++) {
                int sx = ofClamp((int)((c + 0.5f) * w / numCols), 0, w - 1);
                int sy = ofClamp((int)((r + 0.5f) * h / numRows), 0, h - 1);
                int idx = (sy * w + sx) * 3;
                int brightness = (data[idx] + data[idx + 1] + data[idx + 2]) / 3;
                int ci = std::min((brightness * numLevels) / 255, numLevels);

                if (colorMode == 2) ofSetColor(data[idx], data[idx + 1], data[idx + 2]);

                ofDrawBitmapString(std::string(1, chars[ci]), dispX + c * cellW, drawY);
            }
        }
    }
};
