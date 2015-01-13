/*
 * Copyright (c) 2011 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/danomatika/ofxPd for documentation
 *
 */
#pragma once

#include "ofMain.h"

#include "ofxPd.h"

#include "ofPipe.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "ofPerson.h"
#include "Externals.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxLibwebsockets.h"
#include "ofxJSON.h"

#define HOST "localhost"
#define PORT 12345
#define TUBE_NUM 569
#define PERSON_NUM 10

// a namespace for the Pd types
using namespace pd;

class AppCore : public PdReceiver, public PdMidiReceiver {

	public:

		// main
		void setup(const int numOutChannels, const int numInChannels,
		           const int sampleRate, const int ticksPerBuffer);
		void update();
		void draw();
		void exit();

		// do something
		void playTone(int pitch);
		
		// input callbacks
		void keyPressed(int key);
		
		// audio callbacks
		void audioReceived(float * input, int bufferSize, int nChannels);
		void audioRequested(float * output, int bufferSize, int nChannels);
		
		// pd message receiver callbacks
		void print(const std::string& message);
		
		void receiveBang(const std::string& dest);
		void receiveFloat(const std::string& dest, float value);
		void receiveSymbol(const std::string& dest, const std::string& symbol);
		void receiveList(const std::string& dest, const List& list);
		void receiveMessage(const std::string& dest, const std::string& msg, const List& list);
		
		// pd midi receiver callbacks
		void receiveNoteOn(const int channel, const int pitch, const int velocity);
		void receiveControlChange(const int channel, const int controller, const int value);
		void receiveProgramChange(const int channel, const int value);
		void receivePitchBend(const int channel, const int value);
		void receiveAftertouch(const int channel, const int value);
		void receivePolyAftertouch(const int channel, const int pitch, const int value);
		
		void receiveMidiByte(const int port, const int byte);
		
		// demonstrates how to manually poll for messages
		void processEvents();
		
        void mouseMoved(int x, int y);
		
        ofxPd pd;
		//vector<float> scopeArray;
        //vector<Patch> instances;
        Patch patches[PERSON_NUM];
        Patch mousePatch;
		
		int midiChan;
    
        ofxXmlSettings XML;
        ofxXmlSettings newXML;
        string message;
        string info;
        float sendFreq;
    
        ofxOscSender sender;
    
        //-------------------------------------------------------
        
        ofxKinect kinect;
        ofxKinect kinect1;
        
        ofxCvGrayscaleImage grayImage;
        ofxCvGrayscaleImage grayImage1;
        ofxCvGrayscaleImage bothKinects;
        ofxCvContourFinder contourFinder;
        
        unsigned char* combinedVideo;
        ofPixels GrayPixel;
        ofPixels GrayPixel1; 
        
        int maxInput = 30;
        int currentInput;
        int thresholdVal = 30;
   
        vector<ofxCvBlob> blobs;
        vector<float> blobCenterX;
        vector<float> blobCenterXmap;
        vector<float> blobCenterY;
        vector<float> blobCenterYmap;
        long long      timeStamp;
    
        // variables for testing with the mouse
        ofPerson* mPerson;
        int mID;
        int mFreq;
        bool outputState;
        bool tryConnecting;
        bool grafics = true;
    
        //-------------------------------------------------------
    
        ofxLibwebsockets::Client client;
        ofxLibwebsockets::ClientOptions options;
    
        // websocket methods
        void onConnect( ofxLibwebsockets::Event& args );
        void onOpen( ofxLibwebsockets::Event& args );
        void onClose( ofxLibwebsockets::Event& args );
        void onIdle( ofxLibwebsockets::Event& args );
        void onMessage( ofxLibwebsockets::Event& args );
        void onBroadcast( ofxLibwebsockets::Event& args );
    
        ofxJSONElement jsonOut;
    
        ofTrueTypeFont f; 
    
    private:
        ofPipe** allPipes;
        ofPerson** persons;
    ofColor** colors;
};
