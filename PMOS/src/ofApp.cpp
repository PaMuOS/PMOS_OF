#include "ofApp.h"
#include <Poco/Path.h>

//--------------------------------------------------------------
void ofApp::setup(){
    // the number of libpd ticks per buffer,
	// used to compute the audio buffer len: tpb * blocksize (always 64)
#ifdef TARGET_LINUX_ARM
    // longer latency for Raspberry PI
    int ticksPerBuffer = 32; // 32 * 64 = buffer len of 2048
    int numInputs = 0; // no built in mic4
#else
    int ticksPerBuffer = 8; // 8 * 64 = buffer len of 512
    int numInputs = 0;
#endif
    
    soundStream.listDevices();
    
    soundStream.setDeviceID(8);
	// setup OF sound stream
    soundStream.setup(this,8,numInputs,44100,ofxPd::blockSize()*ticksPerBuffer,16);
    
	// setup the app core
	core.setup(8, numInputs, 44100, ticksPerBuffer);

}

//--------------------------------------------------------------
void ofApp::update(){
	core.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    core.draw();
    core.mouseMoved(mouseX,mouseY);

}

//--------------------------------------------------------------
void ofApp::exit() {
	core.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	core.keyPressed(key);
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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::audioReceived(float * input, int bufferSize, int nChannels) {
	core.audioReceived(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::audioRequested(float * output, int bufferSize, int nChannels) {
	core.audioRequested(output, bufferSize, nChannels);
}
