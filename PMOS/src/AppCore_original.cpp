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

	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	//ofSetLogLevel(OF_LOG_VERBOSE);

	// double check where we are ...
	cout << ofFilePath::getCurrentWorkingDirectory() << endl;

	if(!pd.init(numOutChannels, numInChannels, sampleRate, ticksPerBuffer)) {
		OF_EXIT_APP(1);
	}

	midiChan = 1; // midi channels are 1-16

	// subscribe to receive source names
	pd.subscribe("toOf");
	pd.subscribe("env");

    pd.addReceiver(*this);   // automatically receives from all subscribed sources
	pd.ignore(*this, "env"); // don't receive from "env"
	
	pd.addMidiReceiver(*this);  // automatically receives from all channels

	pd.addToSearchPath("pd/abs");

	pd.start();
    Patch patch = pd.openPatch("pd/somename.pd");
    
    for(int i = 0; i<PERSON_NUM; i++){
        patches[i] = pd.openPatch("pd/somename.pd");
    }
    
    cout << patch << endl;
    
    
    
    //from the old patch
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
        
        float x = XML.getValue("y",0.0) * 5 + ofGetWidth() / 2;
        float y = XML.getValue("x",0.0) * 5 + ofGetHeight() / 2;
        float radius = XML.getValue("diameter",0.0 ) / 2.4;
        float length = XML.getValue("length",0.0 );
        float height= XML.getValue("height",0.0);
        float frequency = 342 / ((length*2)/100);
        int idNum = XML.getValue("num",0);
        int element = XML.getValue("element",0 );
        allPipes[i] = new ofPipe(x,y,radius,length,height,frequency, idNum, element);
        
        XML.popTag();
    }
    
    for(int i = 0; i<PERSON_NUM; i++){
        persons[i]= new ofPerson(0.0,0.0,0.0, i);
    }


}

//--------------------------------------------------------------
void AppCore::update() {
	ofBackground(100, 100, 100);
    //pd.sendFloat("fromOf", ofGetAppPtr()->mouseX);
	
    // update scope array from pd
	//pd.readArray("scope", scopeArray);
    pd.sendFloat("fromOf", sendFreq);
    
    for (int i = 0; i < TUBE_NUM; i++){
        allPipes[i]->update();
        
    }
    
    
    
    for(int u = 0; u<PERSON_NUM; u++){
        persons[u]->frequency = 0;
        for (int i = 0; i < TUBE_NUM; i++){
        
            
            float dist = ofDist(allPipes[i]->x,allPipes[i]->y,persons[u]->x,persons[u]->y);
            
            if(dist<allPipes[i]->radius){
                persons[u]->frequency=allPipes[i]->frequency;
                allPipes[i]->isHit=true;
            }
            
        }
    }
    
    for (int i = 0; i < PERSON_NUM; i++){
        persons[i]->update();
        pd.sendFloat(patches[i].dollarZeroStr()+"-fromOf",persons[i]->frequency);
        
    }
}

//--------------------------------------------------------------
void AppCore::draw() {

	
    for (int i = 0; i < TUBE_NUM; i++){
        allPipes[i]->draw();
    }
    
    
    for (int i = 0; i < PERSON_NUM; i++){
        persons[i]->draw();
        
    }
    
    ofSetColor(0, 0, 0);
    ofDrawBitmapString(message, 20,20);
    ofDrawBitmapString("Sending the frequency as OSC message to port " + ofToString(PORT), 600,40);
    ofDrawBitmapString("frequency:  " + ofToString(sendFreq), 600,60);
    ofDrawBitmapString(info, ofGetAppPtr()->mouseX+6,ofGetAppPtr()->mouseY);
}

//--------------------------------------------------------------
void AppCore::exit() {}

//--------------------------------------------------------------
void AppCore::playTone(int pitch) {
	pd << StartMessage() << "pitch" << pitch << FinishList("tone") << Bang("tone");
}

void AppCore::mouseMoved(int x, int y ){
    
    info = "";
    sendFreq = 0;
    
    for(int i=0; i<TUBE_NUM; i++){
        float distance = ofDist(allPipes[i]->x, allPipes[i]->y, x, y);
        if(distance < allPipes[i]->radius){
            allPipes[i]->isHit=true;
            sendFreq = allPipes[i]->frequency;
            info = ofToString( allPipes[i]->idNum) + " - " + ofToString(sendFreq);
            
        }else{
            allPipes[i]->isHit=false;
        }
    }
    //ofxOscMessage m;
	//m.setAddress("/freq");
	//m.addFloatArg(sendFreq);
	//sender.sendMessage(m);
    
}

//--------------------------------------------------------------
void AppCore::keyPressed (int key) {
/*
	switch(key) {

	case 'a':
		playTone(60);
		break;
	case 'w':
		playTone(61);
		break;
	case 's':
		playTone(62);
		break;
	case 'e':
		playTone(63);
		break;
	case 'd':
		playTone(64);
		break;
	case 'f':
		playTone(65);
		break;
	case 't':
		playTone(66);
		break;
	case 'g':
		playTone(67);
		break;
	case 'y':
		playTone(68);
		break;
	case 'h':
		playTone(69);
		break;
	case 'u':
		playTone(70);
		break;
	case 'j':
		playTone(71);
		break;
	case 'k':
		playTone(72);
		break;

	case ' ':
		if(pd.isReceiving(*this, "env")) {
			pd.ignore(*this, "env");
			cout << "ignoring env" << endl;
		}
		else {
			pd.receive(*this, "env");
			cout << "receiving from env" << endl;
		}
		break;

	default:
		break;
	}*/
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
