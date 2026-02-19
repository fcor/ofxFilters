#pragma once

#include "ofMain.h"

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
		ofTexture effectTexture;

		int camWidth;
		int camHeight;

		// Frame buffer for slitscan
		static const int NUM_FRAMES = 60;
		unsigned char* frameBuffer[60];
		int currentFrameIndex;

		// Enable effects
		bool effectWave;
		bool effectRGB;
		bool effectSlitscan;
		bool effectBlockDisplace;

		// Params
		float waveSpeed;
		float waveHAmount;
		float waveVAmount;
		int rgbShiftAmount;
		int slitscanDepth;
		int blockSize;
		float blockAmount;

};
