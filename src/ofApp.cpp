#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	camWidth = 640;
	camHeight = 480;

	myCamFeed.listDevices();
	myCamFeed.setDeviceID(1);
	myCamFeed.initGrabber(camWidth, camHeight);

	effectData = new unsigned char[camWidth * camHeight * 3];
	effectTexture.allocate(camWidth, camHeight, GL_RGB);

	// initialize frame buffer
	currentFrameIndex = 0;
	for (int i = 0; i < NUM_FRAMES; i++) {
		frameBuffer[i] = new unsigned char[camWidth * camHeight * 3];
		memset(frameBuffer[i], 0, camWidth * camHeight * 3);
	}

	// Default effect states
	effectWave = true;
	effectRGB = true;
	effectSlitscan = false;
	effectBlockDisplace = false;

	// Default parameters
	waveSpeed = 3.0;
	waveHAmount = 6.0;
	waveVAmount = 10.0;
	rgbShiftAmount = 7;
	slitscanDepth = 30;
	blockSize = 16;
	blockAmount = 10.0;
}

//--------------------------------------------------------------
void ofApp::update(){
	myCamFeed.update();

	if (myCamFeed.isFrameNew()) {
		unsigned char* pixelData = myCamFeed.getPixels().getData();
		int nTotalBytes = camWidth * camHeight * 3;
		float time = ofGetElapsedTimef();

		// Store current frame in buffer
		memcpy(frameBuffer[currentFrameIndex], pixelData, nTotalBytes);

		// Process each pixel
		for (int i = 0; i < nTotalBytes; i++) {
			int pixelIndex = i / 3;
			int row = pixelIndex / camWidth;
			int col = pixelIndex % camWidth;
			int channel = i % 3;

			// Start with current position
			int srcRow = row;
			int srcCol = col;
			int srcFrame = currentFrameIndex;  // which frame to sample from

			// --- SLITSCAN (samples from different frames) ---
			if (effectSlitscan) {
				int frameOffset = (col * slitscanDepth) / camHeight;
				srcFrame = (currentFrameIndex - frameOffset + NUM_FRAMES) % NUM_FRAMES;
			}

			// --- WAVE (displaces position) ---
			if (effectWave) {
				int hShift = waveHAmount * sin(srcRow * 0.03 + time * waveSpeed);
				int vShift = waveVAmount * sin(srcCol * 0.02 + time * waveSpeed * 0.7);
				srcCol = (srcCol + hShift + camWidth) % camWidth;
				srcRow = (srcRow + vShift + camHeight) % camHeight;
			}

			// --- BLOCK DISPLACEMENT ---
			if (effectBlockDisplace) {
				int blockX = srcCol / blockSize;
				int blockY = srcRow / blockSize;
				int blockShiftX = blockAmount * sin(blockY * 0.5 + time * 2.0);
				int blockShiftY = blockAmount * 0.5 * sin(blockX * 0.3 + time * 1.5);
				srcCol = (srcCol + blockShiftX + camWidth) % camWidth;
				srcRow = (srcRow + blockShiftY + camHeight) % camHeight;
			}

			// --- RGB SEPARATION (per-channel offset) ---
			if (effectRGB) {
				if (channel == 0) {
					srcCol = (srcCol - rgbShiftAmount + camWidth) % camWidth;
				}
				else if (channel == 2) {
					srcCol = (srcCol + rgbShiftAmount) % camWidth;
				}
			}

			// Sample from the appropriate frame
			int srcIndex = (srcRow * camWidth + srcCol) * 3 + channel;
			effectData[i] = frameBuffer[srcFrame][srcIndex];
		}

		currentFrameIndex = (currentFrameIndex + 1) % NUM_FRAMES;

		effectTexture.loadData(effectData, camWidth, camHeight, GL_RGB);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);
	ofSetColor(255);

	// Draw the effect scaled to fit window while maintaining aspect ratio
	float windowW = ofGetWidth();
	float windowH = ofGetHeight();
	float camAspect = (float)camWidth / camHeight;
	float windowAspect = windowW / windowH;

	float effectW, effectH, effectX, effectY;
	if (windowAspect > camAspect) {
		// Window is wider - fit to height
		effectH = windowH;
		effectW = effectH * camAspect;
		effectX = (windowW - effectW) / 2;
		effectY = 0;
	} else {
		// Window is taller - fit to width
		effectW = windowW;
		effectH = effectW / camAspect;
		effectX = 0;
		effectY = (windowH - effectH) / 2;
	}
	effectTexture.draw(effectX, effectY, effectW, effectH);

	// Draw original camera feed in top left corner (small preview)
	int previewW = camWidth / 4;
	int previewH = camHeight / 4;
	int margin = 10;
	myCamFeed.draw(margin, margin, previewW, previewH);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == '1') effectWave = !effectWave;
	if (key == '2') effectRGB = !effectRGB;
	if (key == '3') effectSlitscan = !effectSlitscan;
	if (key == '4') effectBlockDisplace = !effectBlockDisplace;

	// Parameter adjustments
	if (key == 'q') slitscanDepth = min(slitscanDepth + 5, 59);
	if (key == 'a') slitscanDepth = max(slitscanDepth - 5, 1);
	if (key == 'w') blockSize = min(blockSize + 4, 64);
	if (key == 's') blockSize = max(blockSize - 4, 4);
	if (key == 'e') blockAmount += 2.0;
	if (key == 'd') blockAmount = max(blockAmount - 2.0f, 0.0f);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
