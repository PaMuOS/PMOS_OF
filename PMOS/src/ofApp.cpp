#include "ofApp.h"
#include <Poco/Path.h>
#include "RtAudio.h"

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
    
    bool rmeConnected = false;
    int deviceID = 0;
    int numOutputs = 0;
    int builtInDevice;
    //soundStream.listDevices();
    //------------------------------------------------
    ofPtr<RtAudio> audioTemp;
    try {
        audioTemp = ofPtr<RtAudio>(new RtAudio());
    } catch (RtError &error) {
        error.printMessage();
        return;
    }
    int devices = audioTemp->getDeviceCount();
    RtAudio::DeviceInfo info;
    for (int i=0; i< devices; i++) {
        try {
            info = audioTemp->getDeviceInfo(i);
        } catch (RtError &error) {
            error.printMessage();
            break;
        }
        ofLogNotice("ofRtAudioSoundStream") << "device " << i << " " << info.name << "";
        if (info.isDefaultInput) ofLogNotice("ofRtAudioSoundStream") << "----* default ----*";
        ofLogNotice("ofRtAudioSoundStream") << "maximum output channels " << info.outputChannels;
        ofLogNotice("ofRtAudioSoundStream") << "maximum input channels " << info.inputChannels;
        ofLogNotice("ofRtAudioSoundStream") << "-----------------------------------------";
    }
    for (int i=0; i< devices; i++) {
        try {
            info = audioTemp->getDeviceInfo(i);
        } catch (RtError &error) {
            error.printMessage();
            break;
        }
        if(info.name == "Apple Inc.: Built-in Output"){
            //rmeConnected = true;
            deviceID = i;
            numOutputs = 2;
            cout << "found built-in audio";
        }

        if(info.name == "RME: Fireface 400 (963)"){
            rmeConnected = true;
            deviceID = i;
            numOutputs = 8;
            cout << "found fireface";
        }
    }
    
    soundStream.setDeviceID(deviceID);
    soundStream.setup(this,numOutputs,numInputs,44100,ofxPd::blockSize()*ticksPerBuffer,16);

    //------------------------------------------------
    
	// setup OF sound stream
//    if(soundStream.setup(this,8,numInputs,44100,ofxPd::blockSize()*ticksPerBuffer,16)){
//        cout << "interface found";
//        numOutputs = 8;
//    }else{
//        soundStream.stop();
//        soundStream.setDeviceID(2);
//        soundStream.setup(this,2,numInputs,44100,ofxPd::blockSize()*ticksPerBuffer,16);
//        cout << "no sound interface. using built-in output.\n";
//        numOutputs = 2;
//    }
    
	// setup the app core
	core.setup(numOutputs, numInputs, 44100, ticksPerBuffer);

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
