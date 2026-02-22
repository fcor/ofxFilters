#pragma once

#include "ofMain.h"
#include "effects.h"
#include "renderers.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofVideoGrabber myCamFeed;

		unsigned char* effectData;

		int camWidth;
		int camHeight;

		// Frame buffer for slitscan
		static const int NUM_FRAMES = 60;
		unsigned char* frameBuffer[60];
		int currentFrameIndex;

		// Effect chain (ordered; each module modifies PixelContext)
		std::vector<EffectModule*> effectChain;
		WaveEffect*          waveEffect;
		SlitscanEffect*      slitscanEffect;
		BlockDisplaceEffect* blockDisplaceEffect;
		RgbSplitEffect*      rgbSplitEffect;

		// Renderer chain (each reads effectData and draws)
		std::vector<Renderer*> renderers;
		TextureRenderer* textureRenderer;
		AsciiRenderer*   asciiRenderer;
};
