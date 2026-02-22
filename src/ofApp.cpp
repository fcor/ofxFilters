#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	camWidth  = 640;
	camHeight = 480;

	// Camera
	myCamFeed.listDevices();
	myCamFeed.setDeviceID(1);
	myCamFeed.initGrabber(camWidth, camHeight);

	useVideo = true;
	myVideoPlayer.load("seba.mp4");
	myVideoPlayer.setLoopState(OF_LOOP_NORMAL);
	myVideoPlayer.play();

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
	unsigned char* pixelData = nullptr;

	if (useVideo) {
		myVideoPlayer.update();
		if (!myVideoPlayer.isFrameNew()) return;
		videoFrame = myVideoPlayer.getPixels();
		videoFrame.setImageType(OF_IMAGE_COLOR);  // normalize RGBA -> RGB
		videoFrame.resize(camWidth, camHeight);   // match buffer row stride
		pixelData = videoFrame.getData();
	} else {
		myCamFeed.update();
		if (!myCamFeed.isFrameNew()) return;
		pixelData = myCamFeed.getPixels().getData();
	}
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

	// Source preview (top-left corner)
	ofSetColor(255);
	int previewW = camWidth  / 4;
	int previewH = camHeight / 4;
	if (useVideo) {
		myVideoPlayer.draw(10, 10, previewW, previewH);
	} else {
		myCamFeed.draw(10, 10, previewW, previewH);
	}

	drawUI();
}

//--------------------------------------------------------------
void ofApp::drawUI() {
	const char* charSetNames[]   = { "standard", "sparse", "dense", "organic" };
	const char* colorModeNames[] = { "mono", "row", "per-char" };

	ofColor white    = ofColor(255, 255, 255);
	ofColor onColor  = ofColor(100, 255, 100);
	ofColor offColor = ofColor(130, 130, 130);
	ofColor dimColor = ofColor(160, 160, 160);

	auto badge     = [](bool b) -> std::string { return b ? "[ON ] " : "[OFF] "; };
	auto itemColor = [&](bool b) -> ofColor    { return b ? onColor : offColor; };

	std::string sourceLabel = useVideo ? "[VIDEO] v: cam  p: play/pause"
	                                   : "[CAM]   v: video";

	using P = std::pair<std::string, ofColor>;
	std::vector<P> lines = {
		{"EFFECTS                     FPS: " + ofToString((int)ofGetFrameRate()), white},
		{sourceLabel, dimColor},
		{badge(waveEffect->enabled)          + "1: Wave",                         itemColor(waveEffect->enabled)},
		{badge(rgbSplitEffect->enabled)      + "2: RGB Split",                    itemColor(rgbSplitEffect->enabled)},
		{badge(slitscanEffect->enabled)      + "3: Slitscan    depth: "
		     + ofToString(slitscanEffect->depth),                                  itemColor(slitscanEffect->enabled)},
		{badge(blockDisplaceEffect->enabled) + "4: BlockDisp   size: "
		     + ofToString(blockDisplaceEffect->blockSize) + "  amt: "
		     + ofToString(blockDisplaceEffect->blockAmount, 1),                    itemColor(blockDisplaceEffect->enabled)},
		{"", white},
		{"RENDERERS", white},
		{badge(textureRenderer->enabled) + "0: Texture",                          itemColor(textureRenderer->enabled)},
		{badge(asciiRenderer->enabled)   + "5: ASCII",                            itemColor(asciiRenderer->enabled)},
		{"      mode: " + std::string(colorModeNames[asciiRenderer->colorMode % 3])
		     + "  size: " + ofToString(asciiRenderer->cellW)
		     + "  chars: " + std::string(charSetNames[asciiRenderer->charSetIndex % 4]), dimColor},
	};

	const int lineH  = 16;
	const int padX   = 10;
	const int padY   = 8;
	const int panelW = 380;
	const int panelH = (int)lines.size() * lineH + padY * 2;
	const int panelX = 10;
	const int panelY = ofGetHeight() - panelH - 10;

	ofEnableAlphaBlending();
	ofSetColor(0, 0, 0, 180);
	ofDrawRectangle(panelX, panelY, panelW, panelH);
	ofDisableAlphaBlending();

	for (int i = 0; i < (int)lines.size(); i++) {
		if (lines[i].first.empty()) continue;
		ofSetColor(lines[i].second);
		ofDrawBitmapString(lines[i].first, panelX + padX, panelY + padY + (i + 1) * lineH);
	}

	ofSetColor(255);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	// Source toggle
	if (key == 'v') useVideo = !useVideo;
	if (key == 'p' && useVideo) myVideoPlayer.setPaused(!myVideoPlayer.isPaused());

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
