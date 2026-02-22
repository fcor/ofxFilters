#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	camWidth  = 640;
	camHeight = 480;

	myCamFeed.listDevices();
	myCamFeed.setDeviceID(1);
	myCamFeed.initGrabber(camWidth, camHeight);

	effectData = new unsigned char[camWidth * camHeight * 3];

	// Initialize frame buffer
	currentFrameIndex = 0;
	for (int i = 0; i < NUM_FRAMES; i++) {
		frameBuffer[i] = new unsigned char[camWidth * camHeight * 3];
		memset(frameBuffer[i], 0, camWidth * camHeight * 3);
	}

	// --- Build effect chain ---
	waveEffect          = new WaveEffect();
	waveEffect->enabled = true;
	effectChain.push_back(waveEffect);

	slitscanEffect          = new SlitscanEffect(NUM_FRAMES);
	slitscanEffect->enabled = false;
	effectChain.push_back(slitscanEffect);

	blockDisplaceEffect          = new BlockDisplaceEffect();
	blockDisplaceEffect->enabled = false;
	effectChain.push_back(blockDisplaceEffect);

	rgbSplitEffect          = new RgbSplitEffect();
	rgbSplitEffect->enabled = true;
	effectChain.push_back(rgbSplitEffect);

	// --- Build renderer chain ---
	textureRenderer = new TextureRenderer();
	textureRenderer->allocate(camWidth, camHeight);
	renderers.push_back(textureRenderer);

	asciiRenderer          = new AsciiRenderer();
	asciiRenderer->enabled = false;
	renderers.push_back(asciiRenderer);
}

//--------------------------------------------------------------
void ofApp::update(){
	myCamFeed.update();

	if (!myCamFeed.isFrameNew()) return;

	unsigned char* pixelData  = myCamFeed.getPixels().getData();
	int            nTotalBytes = camWidth * camHeight * 3;
	float          time        = ofGetElapsedTimef();

	// Store current frame in circular buffer
	memcpy(frameBuffer[currentFrameIndex], pixelData, nTotalBytes);

	// Process each byte (pixel channel) through the effect chain
	for (int i = 0; i < nTotalBytes; i++) {
		int pixelIndex = i / 3;
		int row        = pixelIndex / camWidth;
		int col        = pixelIndex % camWidth;
		int channel    = i % 3;

		PixelContext ctx(row, col, channel,
		                 row, col, currentFrameIndex,
		                 time, camWidth, camHeight);

		for (auto* effect : effectChain) {
			if (effect->enabled) effect->transform(ctx);
		}

		int srcIndex   = (ctx.srcRow * camWidth + ctx.srcCol) * 3 + ctx.channel;
		effectData[i]  = frameBuffer[ctx.srcFrame][srcIndex];
	}

	currentFrameIndex = (currentFrameIndex + 1) % NUM_FRAMES;
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);

	// Compute letterboxed display rect
	float windowW    = ofGetWidth();
	float windowH    = ofGetHeight();
	float camAspect  = (float)camWidth / camHeight;
	float winAspect  = windowW / windowH;

	float effectW, effectH, effectX, effectY;
	if (winAspect > camAspect) {
		effectH = windowH;
		effectW = effectH * camAspect;
		effectX = (windowW - effectW) / 2;
		effectY = 0;
	} else {
		effectW = windowW;
		effectH = effectW / camAspect;
		effectX = 0;
		effectY = (windowH - effectH) / 2;
	}

	// Run all renderers
	for (auto* renderer : renderers) {
		if (renderer->enabled) {
			renderer->render(effectData, camWidth, camHeight,
			                 effectX, effectY, effectW, effectH);
		}
	}

	// Camera preview (top-left corner)
	ofSetColor(255);
	int previewW = camWidth  / 4;
	int previewH = camHeight / 4;
	myCamFeed.draw(10, 10, previewW, previewH);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	// Toggle effects
	if (key == '1') waveEffect->enabled         = !waveEffect->enabled;
	if (key == '2') rgbSplitEffect->enabled      = !rgbSplitEffect->enabled;
	if (key == '3') slitscanEffect->enabled      = !slitscanEffect->enabled;
	if (key == '4') blockDisplaceEffect->enabled = !blockDisplaceEffect->enabled;

	// Toggle / configure renderers
	if (key == '0') textureRenderer->enabled     = !textureRenderer->enabled;
	if (key == '5') asciiRenderer->enabled       = !asciiRenderer->enabled;
	if (key == '6') asciiRenderer->colorMode     = (asciiRenderer->colorMode + 1) % 3;
	if (key == 'm') asciiRenderer->cellW         = std::max(asciiRenderer->cellW - 2, 4);
	if (key == 'n') asciiRenderer->cellW         = std::min(asciiRenderer->cellW + 2, 32);
	if (key == ',') asciiRenderer->charSetIndex  = (asciiRenderer->charSetIndex - 1 + 4) % 4;
	if (key == '.') asciiRenderer->charSetIndex  = (asciiRenderer->charSetIndex + 1) % 4;

	// Effect parameters
	if (key == 'q') slitscanEffect->depth           = std::min(slitscanEffect->depth + 5, 59);
	if (key == 'a') slitscanEffect->depth           = std::max(slitscanEffect->depth - 5, 1);
	if (key == 'w') blockDisplaceEffect->blockSize  = std::min(blockDisplaceEffect->blockSize + 4, 64);
	if (key == 's') blockDisplaceEffect->blockSize  = std::max(blockDisplaceEffect->blockSize - 4, 4);
	if (key == 'e') blockDisplaceEffect->blockAmount += 2.0f;
	if (key == 'd') blockDisplaceEffect->blockAmount = std::max(blockDisplaceEffect->blockAmount - 2.0f, 0.0f);
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
