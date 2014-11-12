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
#include "AppCore.h"

//--------------------------------------------------------------
void AppCore::setup(const int numOutChannels, const int numInChannels,
                    const int sampleRate, const int ticksPerBuffer) {

	ofSetFrameRate(25);
	ofSetVerticalSync(true);
	//ofSetLogLevel(OF_LOG_VERBOSE);

	// double check where we are ...
	cout << ofFilePath::getCurrentWorkingDirectory() << endl;

	if(!pd.init(numOutChannels, numInChannels, sampleRate, ticksPerBuffer)) {
		OF_EXIT_APP(1);
	}
    Externals::setup();
	midiChan = 1; // midi channels are 1-16

	// subscribe to receive source names
	pd.subscribe("toOf");
	pd.subscribe("env");

    pd.addReceiver(*this);   // automatically receives from all subscribed sources
	pd.ignore(*this, "env"); // don't receive from "env"
	
	pd.addMidiReceiver(*this);  // automatically receives from all channels

	pd.addToSearchPath("pd/abs");

	pd.start();
    //Patch patch = pd.openPatch("pd/somename.pd");
    
    //cout << patch << endl;
    
//----------------------------------- KINECT -------------------------------------------
    
    kinect.init();
    kinect.open();
    kinect.setCameraTiltAngle(0);
    grayImage.allocate(kinect.width, kinect.height);

    kinect1.init();
    kinect1.open();
    kinect1.setCameraTiltAngle(20);
    grayImage1.allocate(kinect1.width, kinect1.height);
    
    bothKinects.allocate(kinect.height*2, kinect.width);
    combinedVideo = (unsigned char*)malloc(640 * 480 * 2 * sizeof(unsigned char*));
    
    blobs.resize(100);
    blobCenterX.resize(100);
    blobCenterXmap.resize(100);
    blobCenterY.resize(100);
    blobCenterYmap.resize(100);
    
    //ofSetFrameRate(15);
    
//--------------------------------------------------------------------------------------

    sender.setup(HOST, PORT);
    
    allPipes = new ofPipe*[TUBE_NUM]; // an array of pointers for the objects
    persons = new ofPerson*[PERSON_NUM];
    
    //the string is printed at the top of the app
    //to give the user some feedback
    message = "loading data.xml";
    
    //we load our data file
    
    if( XML.loadFile("data.xml") ){
        message = "data.xml loaded!";
    }else{
        message = "unable to load data.xml check data/ folder";
    }
    
    XML.pushTag("document");
    
    for(int i=0; i<TUBE_NUM; i++){
        
        XML.pushTag("tube",i);
        
        float cX = ofGetWidth()/2;
        float cY = ofGetHeight()/2;
        
        float x = XML.getValue("y",0.0);
        float y = XML.getValue("x",0.0);
        
        float angle = ofDegToRad(-20);
        // rotate
        float rX = (x*cos(angle) - y*sin(angle));
        float rY = (x*sin(angle) + y*cos(angle));
        
        // map to the of window size
        x = (rX * 5.5 + ofGetWidth() / 2) - 50;
        y = rY * 5.5 + ofGetHeight() / 2;
        
        
        float radius = XML.getValue("diameter",0.0 ) / 2.4;
        float length = XML.getValue("length",0.0 );
        float height= XML.getValue("height",0.0);
        float frequency = 342 / ((length*2)/100);
        int idNum = XML.getValue("num",0);
        int element = XML.getValue("element",0 );
        int open = XML.getValue("oc",0 );
        allPipes[i] = new ofPipe(x,y,radius,length,height,frequency, idNum, element, open);
        
        XML.popTag();
    }
    
    for(int i = 0; i<PERSON_NUM; i++){
        patches[i]=pd.openPatch("pd/PMOS_Synth_workcopy(2nd_ver).pd");
        persons[i]= new ofPerson(0.0,0.0,0.0, i);
    }
    
}

//--------------------------------------------------------------
void AppCore::update() {
    
    ofBackground(150);
    kinect.update();
    kinect1.update();
    
    
    if(kinect.isFrameNew() /*&& kinect1.isFrameNew()*/){
        
        grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
        grayImage1.setFromPixels(kinect1.getDepthPixels(), kinect1.width, kinect1.height);
        
        GrayPixel = grayImage.getPixelsRef();
        GrayPixel.rotate90(1);
        
        GrayPixel1 = grayImage1.getPixelsRef();
        GrayPixel1.rotate90(1);
        
        for(int i=0; i<640; i++){
            memcpy(combinedVideo + (i*960), GrayPixel.getPixels()+(i*480), 480);
            memcpy(combinedVideo + (i*960+480), GrayPixel1.getPixels()+(i*480), 480);
            bothKinects.setFromPixels(combinedVideo, 480*2, 640);
        }
        
        bothKinects.threshold(thresholdVal);
        contourFinder.findContours(bothKinects, 300, 25000, maxInput, false);
        
    }
    
    grayImage.flagImageChanged();
    grayImage1.flagImageChanged();
    
    currentInput = contourFinder.nBlobs;
	
//--------------------------------------------------------------------------------------
    for (int i = 0; i < TUBE_NUM; i++){
        allPipes[i]->update();
    }
}

//--------------------------------------------------------------
void AppCore::draw() {
    
    ofPushMatrix();
    ofScale(0.25, 0.25);
    bothKinects.draw(0,0);
    contourFinder.draw(0, 0, 960, 640);
    ofPopMatrix();
    //grayImage.draw(0,0);
    //kinect.draw(10, 10, 400, 300);
    for(int i = 0; i < currentInput; i++) {
        blobs[i] = contourFinder.blobs.at(i);
        blobCenterX[i] = blobs[i].centroid.x;
        blobCenterY[i] = blobs[i].centroid.y;
        blobCenterXmap[i] = ofMap(blobCenterX[i], 0, kinect.height*2, 130, screenWidth-100);
        blobCenterYmap[i] = ofMap(blobCenterY[i], 0, kinect.width, 0, screenHeight);
        
        // draw center of blobs
        ofSetColor(255, 255, 0);
        ofFill();
        ofCircle(blobCenterXmap[i], blobCenterYmap[i], 3);
        ofDrawBitmapString(ofToString(i), blobCenterXmap[i], blobCenterYmap[i]);
        ofSetColor(255);
    }
    
    
    ofSetColor(255,0,0);
    ofLine(130, 0, 130, screenHeight);
    ofLine(screenWidth-100, 0, screenWidth-100, screenHeight);
    
    ////////////////////////////////////////////////////////////////////////////////////////
    
    if(currentInput>PERSON_NUM){
        currentInput=PERSON_NUM;
    }
    
    /*if(currentInput==0){
        for (int i = 0; i < PERSON_NUM; i++){
        persons[i]->frequency = 0;
        persons[i]->diameter=0;
        persons[i]->height=0;
        persons[i]->length=0;
        persons[i]->openClosed=0;
        }
    }*/
    
    for(int u = 0; u<currentInput; u++){

        persons[u]->x=blobCenterXmap[u];
        persons[u]->y=blobCenterYmap[u];
        persons[u]->frequency = 0;
        persons[u]->diameter=0;
        persons[u]->height=0;
        persons[u]->length=0;
        persons[u]->openClosed=0;
        
        for (int i = 0; i < TUBE_NUM; i++){
            float dist = ofDist(allPipes[i]->x,allPipes[i]->y,persons[u]->x,persons[u]->y);
            allPipes[i]->isHit=false;
            if(dist<allPipes[i]->radius){
                persons[u]->frequency=allPipes[i]->frequency;
                persons[u]->diameter=allPipes[i]->radius;
                persons[u]->height=allPipes[i]->height;
                persons[u]->length=allPipes[i]->length;
                persons[u]->openClosed=allPipes[i]->openClosed;
                allPipes[i]->isHit=true;
            }
            
        }
        
        for (int i = 0; i < PERSON_NUM; i++){
            persons[i]->update();
            
            pd.sendFloat(patches[i].dollarZeroStr()+"-frequency",persons[i]->frequency);
            pd.sendFloat(patches[i].dollarZeroStr()+"-openClosed",persons[i]->openClosed);
            pd.sendFloat(patches[i].dollarZeroStr()+"-height",persons[i]->height-persons[i]->length);
            pd.sendFloat(patches[i].dollarZeroStr()+"-diameter",persons[i]->diameter*3.4);
            
        }
    }
//--------------------------------------------------------------
    for (int i = 0; i < TUBE_NUM; i++){
        allPipes[i]->draw();
    }
    
    ofSetColor(0, 0, 0);
    ofDrawBitmapString(message, 20,20);
    ofDrawBitmapString("Sending the frequency as OSC message to port " + ofToString(PORT), 600,40);
    ofDrawBitmapString("frequency:  " + ofToString(sendFreq), 600,60);

    for(int i=0; i<currentInput; i++){
        ofDrawBitmapString(info, blobCenterXmap[i]+6,blobCenterYmap[i]);
    }
    
    
}


void AppCore::kinectInput(float _x, float _y){

}
//--------------------------------------------------------------
void AppCore::exit() {}

//--------------------------------------------------------------
void AppCore::playTone(int pitch) {
	pd << StartMessage() << "pitch" << pitch << FinishList("tone") << Bang("tone");
}

void AppCore::mouseMoved(int x, int y ){

    
}

//--------------------------------------------------------------
void AppCore::keyPressed (int key) {

}

//--------------------------------------------------------------
void AppCore::audioReceived(float * input, int bufferSize, int nChannels) {
	pd.audioIn(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void AppCore::audioRequested(float * output, int bufferSize, int nChannels) {
	pd.audioOut(output, bufferSize, nChannels);
}

//--------------------------------------------------------------
void AppCore::print(const std::string& message) {
	cout << message << endl;
}

//--------------------------------------------------------------
void AppCore::receiveBang(const std::string& dest) {
	cout << "OF: bang " << dest << endl;
}

void AppCore::receiveFloat(const std::string& dest, float value) {
	cout << "OF: float " << dest << ": " << value << endl;
}

void AppCore::receiveSymbol(const std::string& dest, const std::string& symbol) {
	cout << "OF: symbol " << dest << ": " << symbol << endl;
}

void AppCore::receiveList(const std::string& dest, const List& list) {
	cout << "OF: list " << dest << ": ";

	// step through the list
	for(int i = 0; i < list.len(); ++i) {
		if(list.isFloat(i))
			cout << list.getFloat(i) << " ";
		else if(list.isSymbol(i))
			cout << list.getSymbol(i) << " ";
	}

	// you can also use the built in toString function or simply stream it out
	// cout << list.toString();
	// cout << list;

	// print an OSC-style type string
	cout << list.types() << endl;
}

void AppCore::receiveMessage(const std::string& dest, const std::string& msg, const List& list) {
	cout << "OF: message " << dest << ": " << msg << " " << list.toString() << list.types() << endl;
}

//--------------------------------------------------------------
void AppCore::receiveNoteOn(const int channel, const int pitch, const int velocity) {
	cout << "OF MIDI: note on: " << channel << " " << pitch << " " << velocity << endl;
}

void AppCore::receiveControlChange(const int channel, const int controller, const int value) {
	cout << "OF MIDI: control change: " << channel << " " << controller << " " << value << endl;
}

// note: pgm nums are 1-128 to match pd
void AppCore::receiveProgramChange(const int channel, const int value) {
	cout << "OF MIDI: program change: " << channel << " " << value << endl;
}

void AppCore::receivePitchBend(const int channel, const int value) {
	cout << "OF MIDI: pitch bend: " << channel << " " << value << endl;
}

void AppCore::receiveAftertouch(const int channel, const int value) {
	cout << "OF MIDI: aftertouch: " << channel << " " << value << endl;
}

void AppCore::receivePolyAftertouch(const int channel, const int pitch, const int value) {
	cout << "OF MIDI: poly aftertouch: " << channel << " " << pitch << " " << value << endl;
}

// note: pd adds +2 to the port num, so sending to port 3 in pd to [midiout],
//       shows up at port 1 in ofxPd
void AppCore::receiveMidiByte(const int port, const int byte) {
	cout << "OF MIDI: midi byte: " << port << " " << byte << endl;
}

//--------------------------------------------------------------
void AppCore::processEvents() {

	cout << "Number of waiting messages: " << pd.numMessages() << endl;

	while(pd.numMessages() > 0) {
		Message& msg = pd.nextMessage();

		switch(msg.type) {

		case pd::PRINT:
			cout << "OF: " << msg.symbol << endl;
			break;

			// events
		case pd::BANG:
			cout << "OF: bang " << msg.dest << endl;
			break;
		case pd::FLOAT:
			cout << "OF: float " << msg.dest << ": " << msg.num << endl;
			break;
		case pd::SYMBOL:
			cout << "OF: symbol " << msg.dest << ": " << msg.symbol << endl;
			break;
		case pd::LIST:
			cout << "OF: list " << msg.list << msg.list.types() << endl;
			break;
		case pd::MESSAGE:
			cout << "OF: message " << msg.dest << ": " << msg.symbol << " "
			     << msg.list << msg.list.types() << endl;
			break;

			// midi
		case pd::NOTE_ON:
			cout << "OF MIDI: note on: " << msg.channel << " "
			     << msg.pitch << " " << msg.velocity << endl;
			break;
		case pd::CONTROL_CHANGE:
			cout << "OF MIDI: control change: " << msg.channel
			     << " " << msg.controller << " " << msg.value << endl;
			break;
		case pd::PROGRAM_CHANGE:
			cout << "OF MIDI: program change: " << msg.channel << " "
			     << msg.value << endl;
			break;
		case pd::PITCH_BEND:
			cout << "OF MIDI: pitch bend: " << msg.channel << " "
			     << msg.value << endl;
			break;
		case pd::AFTERTOUCH:
			cout << "OF MIDI: aftertouch: " << msg.channel << " "
			     << msg.value << endl;
			break;
		case pd::POLY_AFTERTOUCH:
			cout << "OF MIDI: poly aftertouch: " << msg.channel << " "
			     << msg.pitch << " " << msg.value << endl;
			break;
		case pd::BYTE:
			cout << "OF MIDI: midi byte: " << msg.port << " 0x"
			     << hex << (int) msg.byte << dec << endl;
			break;

		case pd::NONE:
			cout << "OF: NONE ... empty message" << endl;
			break;
		}
	}
}
