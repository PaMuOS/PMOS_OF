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

	ofSetFrameRate(30);
	ofSetVerticalSync(true);
    
    
    //----------------------------------- WEBSOCKET START -------------------------------------------
    ofSetLogLevel(OF_LOG_VERBOSE);
    // basic connection:
    // client.connect("echo.websocket.org");
    // OR optionally use SSL
    // client.connect("echo.websocket.org", true);
    
    // advanced: set keep-alive timeouts for events like
    // loss of internet
    
    // 1 - get default options
    options = ofxLibwebsockets::defaultClientOptions();
    
    // 2 - set basic params
    options.host = "pmos-website.jit.su";
    //options.host = "echo.websocket.org";
    //options.host = "localhost";
    //options.port = 9092;
    
    // 3 - set keep alive params
    // BIG GOTCHA: on BSD systems, e.g. Mac OS X, these time params are system-wide
    // ...so ka_time just says "check if alive when you want" instead of "check if
    // alive after X seconds"
    //options.ka_time     = 1;
    //options.ka_probes   = 0;
    //options.ka_interval = 0;
    
    // 4 - connect
    client.connect(options);
    
    ofSetLogLevel(OF_LOG_ERROR);
    
    client.addListener(this);
    
    //----------------------------------- WEBSOCKET END -------------------------------------------
    
	//ofSetLogLevel(OF_LOG_VERBOSE);
    
	// double check where we are ...
	cout << ofFilePath::getCurrentWorkingDirectory() << endl;

    //----------------------------------- PD START-------------------------------------------
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
    //----------------------------------- PD END-------------------------------------------
    
    //----------------------------------- KINECT START -------------------------------------------
    kinect.listDevices();
    kinect.init();
   // kinect.open("A00365917784047A");// the one in the ceiling
    kinect.open("0000000000000000");//the other one
    kinect.setCameraTiltAngle(0);
    grayImage.allocate(kinect.width, kinect.height);

    kinect1.init();
    kinect1.open("A00364A11700045A");
    kinect1.setCameraTiltAngle(0);
    grayImage1.allocate(kinect1.width, kinect1.height);
    
    bothKinects.allocate(kinect.height*2, kinect.width);
    combinedVideo = (unsigned char*)malloc(640 * 480 * 2 * sizeof(unsigned char*));
    
    blobs.resize(100);
    blobCenterX.resize(100);
    blobCenterXmap.resize(100);
    blobCenterY.resize(100);
    blobCenterYmap.resize(100);
    
    //----------------------------------- KINECT END -------------------------------------------

    // Setup OSC Sender
    sender.setup(HOST, PORT);
    
    allPipes = new ofPipe*[TUBE_NUM]; // an array of pointers for the objects
    persons = new ofPerson*[PERSON_NUM];
    colors = new ofColor*[PERSON_NUM];
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
    newXML.addTag("document");
    newXML.pushTag("document");
    
    for(int i=0; i<TUBE_NUM; i++){
        
        XML.pushTag("tube",i);
        
        float x = XML.getValue("y",0.0);
        float y = XML.getValue("x",0.0);
        
        // rotate
        float angle = ofDegToRad(-25);
        float rX = (x*cos(angle) - y*sin(angle));
        float rY = (x*sin(angle) + y*cos(angle));
        
        
        // map to the of window size
        float mult = 6;
        x = (rX * mult + ofGetWidth() / 2) - 50;
        y = (rY * mult + ofGetHeight() / 2) + 40;
        
        float radius = XML.getValue("diameter",0.0 ) / 1.8;
        float length = XML.getValue("length",0.0 );
        float height= XML.getValue("height",0.0);
        float frequency = 342 / ((length*2)/100);
        int idNum = XML.getValue("num",0);
        int element = XML.getValue("element",0 );
        int open = XML.getValue("oc",0 );
        
        allPipes[i] = new ofPipe(x,y,radius,length,height,frequency, idNum, element, open);
        
        newXML.addTag("tube");
        newXML.pushTag("tube", i);
        newXML.addValue("num", idNum);
        newXML.addValue("frequency", frequency);
        newXML.addValue("length", length);
        newXML.addValue("height", height);
        newXML.addValue("diameter", radius);
        newXML.addValue("x", ofMap(x, 0, ofGetWidth(), 0, 1));
        newXML.addValue("y", ofMap(y, 0, ofGetHeight(), 0, 1));
        newXML.addValue("element", element);
        newXML.addValue("oc", open);
        
        XML.popTag();
        newXML.popTag();
    }
    
    newXML.saveFile("newXML.xml");
    
    // load the PD patches and create the people
    for(int i = 0; i<PERSON_NUM; i++){
        patches[i] = pd.openPatch("pd/main.pd");
        persons[i] = new ofPerson(0.0,0.0,0.0, i);
        colors[i] = new ofColor;
    }
    // load a separate patch for the mouse
    mousePatch = pd.openPatch("pd/main.pd");
    mPerson = new ofPerson(0.0,0.0,0.0,0);
    outputState=false;
    tryConnecting=true;
    


    //colors[10]->setHex(0x939598);
    ofTrueTypeFont::setGlobalDpi(72);
    f = *new ofTrueTypeFont;
    f.loadFont("Tahoma Bold.ttf", 12,true,true);
    ofSetFullscreen(fullScreen);
    threadedObject.start();

}

//--------------------------------------------------------------
void AppCore::update() {

    if (tryConnecting && !client.isConnected()) {
        if(ofGetFrameNum()%300==299){
            cout<<"trying to connect..."<<endl;
            client.connect(options);
        }
    }
    
    bServerConnected = threadedObject.bServerConnected;
    
    kinect.update();
    kinect1.update();
    
    if(kinect.isFrameNew() && kinect1.isFrameNew()){
        
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
        contourFinder.findContours(bothKinects, 500, 50000, maxInput, false);
        
    }
    
    grayImage.flagImageChanged();
    grayImage1.flagImageChanged();
    
    currentInput = contourFinder.nBlobs;
    
    for(int i = 0; i < currentInput; i++) {
        blobs[i] = contourFinder.blobs.at(i);
        blobCenterX[i] = blobs[i].centroid.x;
        blobCenterY[i] = blobs[i].centroid.y;
        blobCenterXmap[i] = ofMap(blobCenterX[i], 0, kinect.height*2, 0, ofGetWidth());
        blobCenterYmap[i] = ofMap(blobCenterY[i], 0, kinect.width, 0, ofGetHeight());
        
        }
    
    ////////////////////////////////////////////////////////////////////////////////////////
    
    ofxOscBundle b;
    Poco::Timestamp epoch(0);
    Poco::Timestamp now;
    Poco::Timestamp::TimeDiff diffTime = (now - epoch);
    timeStamp = (long long) (diffTime/1000);
    
    if(currentInput>PERSON_NUM){
        currentInput=PERSON_NUM;
    }
    
    for (int i = 0; i < PERSON_NUM; i++){
        persons[i]->frequency = 0;
    }
    
    // reset the pipes and check the mouse
    mPerson->pipeID = 0;
    mPerson->frequency=0;
    ofxOscMessage m;
    m.setAddress("/mouse");
    m.addIntArg(timeStamp); // timestamp
    m.addFloatArg(0); // userID ???
    m.addFloatArg(mPerson->x); // x
    m.addFloatArg(mPerson->y); // y
    for (int i = 0; i < TUBE_NUM; i++){
        allPipes[i]->isHit=false;
        float mouseDist = ofDist(allPipes[i]->x,allPipes[i]->y,mPerson->x,mPerson->y);
        if(mouseDist<allPipes[i]->radius){
            mPerson->frequency=allPipes[i]->frequency;
            mPerson->diameter=allPipes[i]->radius;
            mPerson->height=allPipes[i]->height;
            mPerson->length=allPipes[i]->length;
            mPerson->openClosed=allPipes[i]->openClosed;
            mPerson->pipeID = allPipes[i]->idNum;
            allPipes[i]->isHit=true;
        }
    }
    m.addFloatArg(mPerson->pipeID);                 // tubeID
    m.addFloatArg(mPerson->frequency);              // frequency
    m.addFloatArg(mPerson->diameter);               // diameter
    m.addFloatArg(mPerson->height-mPerson->length); // leng
    m.addFloatArg(mPerson->openClosed);             // oc
    b.addMessage(m);
    m.clear();
    
    // check if the tracked people are hitting the pipes
    for(int u = 0; u<currentInput; u++){
        persons[u]->x=blobCenterXmap[u];
        persons[u]->y=blobCenterYmap[u];
        
        
        persons[u]->frequency=0;
        //persons[u]->diameter=0;   //no need to set these to 0
        //persons[u]->height=0;
        //persons[u]->length=0;
        //persons[u]->openClosed=0;
        
        ofxOscMessage oscMessage;
        oscMessage.setAddress("/messages/" + ofToString(u));
        oscMessage.addIntArg(timeStamp); // timestamp
        oscMessage.addFloatArg(u); // userID ???
        oscMessage.addFloatArg(persons[u]->x); // x
        oscMessage.addFloatArg(persons[u]->y); // y
        
        for (int i = 0; i < TUBE_NUM; i++){
            float dist = ofDist(allPipes[i]->x,allPipes[i]->y,persons[u]->x,persons[u]->y);
            if(dist<allPipes[i]->radius){
                
                persons[u]->x=allPipes[i]->x;//THISSHIT
                persons[u]->y=allPipes[i]->y;//THISSHIT
                
                persons[u]->frequency=allPipes[i]->frequency;
                persons[u]->diameter=allPipes[i]->radius;
                persons[u]->height=allPipes[i]->height;
                persons[u]->length=allPipes[i]->length;
                persons[u]->openClosed=allPipes[i]->openClosed;
                persons[u]->pipeID=allPipes[i]->idNum;
                allPipes[i]->isHit=true;
            }
        }
        
        if(!testmode){
            if(persons[u]->pX != persons[u]->x || persons[u]->pY != persons[u]->y){
                jsonPeople[u]["channel"] = ofToString(u);
                jsonPeople[u]["timestamp"] = ofToString(timeStamp);
                jsonPeople[u]["x"] = ofToString(ofMap(mPerson->x,0,ofGetWidth(),0,1));
                jsonPeople[u]["y"] = ofToString(ofMap(mPerson->y,0,ofGetHeight(),0,1));
                jsonPeople[u]["num"] = ofToString(mPerson->pipeID);
                jsonPeople[u]["frequency"] = ofToString(mPerson->frequency);
                jsonPeople[u]["closed"] = ofToString(mPerson->openClosed);
                jsonPeople[u]["diameter"] = ofToString(mPerson->diameter);
                jsonPeople[u]["height"] = ofToString(mPerson->height-mPerson->length);
            }
        }
        
        if(testmode){
            jsonPeople[u]["channel"] = ofToString(u);
            jsonPeople[u]["timestamp"] = ofToString(timeStamp);
            jsonPeople[u]["x"] = ofToString(ofRandom(1.0));
            jsonPeople[u]["y"] = ofToString(ofRandom(1.0));
            jsonPeople[u]["num"] = ofToString(ofRandom(1000));
            jsonPeople[u]["frequency"] = ofToString(ofRandom(300));
            jsonPeople[u]["closed"] = ofToString(0);
            jsonPeople[u]["diameter"] = ofToString(ofRandom(30));
            jsonPeople[u]["height"] = ofToString(ofRandom(500));
        }
        if (ofGetFrameNum()%5==0 && bServerConnected) {
            client.send(ofToString(jsonPeople[u]));
            //cout << ofToString(jsonPeople[u]);

        }

        oscMessage.addFloatArg(persons[u]->pipeID); // tubeID
        oscMessage.addFloatArg(persons[u]->frequency); // frequency
        b.addMessage(oscMessage);
        oscMessage.clear();
    }
    
    if(currentInput==0){
        for (int i = 0; i < PERSON_NUM; i++){
            persons[i]->frequency = 0;
            persons[i]->diameter=0;
            persons[i]->height=0;
            persons[i]->length=0;
            persons[i]->openClosed=0;
            persons[i]->x=0;
            persons[i]->y=0;
        }
    }
    
    
    // Send stuff to the PD patches
    
    for (int i = 0; i < PERSON_NUM; i++){
        
      //  persons[i]->x+=ofRandom(-0.2,0.2);//THISSHIT
      //  persons[i]->y+=ofRandom(-0.2,0.2);//THISSHIT
        
        pd.sendFloat(patches[i].dollarZeroStr()+"-frequency",persons[i]->frequency);
        pd.sendFloat(patches[i].dollarZeroStr()+"-openClosed",persons[i]->openClosed);
        pd.sendFloat(patches[i].dollarZeroStr()+"-height",persons[i]->height-persons[i]->length);
        pd.sendFloat(patches[i].dollarZeroStr()+"-diameter",persons[i]->diameter);
        // vbap
        // OF screen bottom-left is top-left for the vbap speaker placement
        pd.sendFloat(patches[i].dollarZeroStr()+"-y",ofMap(persons[i]->x,0,ofGetWidth(),1,0));
        pd.sendFloat(patches[i].dollarZeroStr()+"-x",ofMap(persons[i]->y,0,ofGetHeight(),1,0));
        
        persons[i]->pX = persons[i]->x;
        persons[i]->pY = persons[i]->y;
    }
    
    // sending OSC
    sender.sendBundle(b);
    
    // mouse patch
    pd.sendFloat(mousePatch.dollarZeroStr()+"-frequency",mPerson->frequency);
    pd.sendFloat(mousePatch.dollarZeroStr()+"-openClosed",mPerson->openClosed);
    pd.sendFloat(mousePatch.dollarZeroStr()+"-height",mPerson->height-mPerson->length);
    pd.sendFloat(mousePatch.dollarZeroStr()+"-diameter",mPerson->diameter);
    // vbap
    // OF screen bottom-left is top-left for the vbap speaker placement
    pd.sendFloat(mousePatch.dollarZeroStr()+"-y",ofMap(mPerson->x,0,ofGetWidth(),1,0));
    pd.sendFloat(mousePatch.dollarZeroStr()+"-x",ofMap(mPerson->y,0,ofGetHeight(),1,0));
    pd.sendFloat(mousePatch.dollarZeroStr()+"-selectOutput",outputState);
    // WEBSOCKET STUFF
    
    if(mPerson->pX != mPerson->x || mPerson->pY != mPerson->y){
        jsonOut["channel"] = "11";
        jsonOut["timestamp"] = ofToString(timeStamp);
        jsonOut["x"] = ofToString(ofMap(mPerson->x,0,ofGetWidth(),0,1));
        jsonOut["y"] = ofToString(ofMap(mPerson->y,0,ofGetHeight(),0,1));
        jsonOut["num"] = ofToString(mPerson->pipeID);
        jsonOut["frequency"] = ofToString(mPerson->frequency);
        jsonOut["closed"] = ofToString(mPerson->openClosed);
        jsonOut["diameter"] = ofToString(mPerson->diameter);
        jsonOut["height"] = ofToString(mPerson->height-mPerson->length);

        if (bServerConnected && ofGetFrameNum()%5==0) {

              client.send(ofToString(jsonOut));

        }
        //cout << ofToString(jsonOut);
    }
    
    mPerson->pX = mPerson->x;
    mPerson->pY = mPerson->y;
    /*
    // this is not doing anything at the moment
    for (int i = 0; i < TUBE_NUM; i++){
        allPipes[i]->update();
    }
    */
    
}

//--------------------------------------------------------------
void AppCore::draw() {
    if(!grafics){
        ofBackground(140);
        bothKinects.draw(ofGetWidth()-bothKinects.width/4-10,10,bothKinects.width/4,bothKinects.height/4);
        contourFinder.draw(0, 0, ofGetWidth(), ofGetHeight());
    
        for (int i = 0; i < TUBE_NUM; i++){
            allPipes[i]->draw();
        }
        string mText = ofToString(mPerson->pipeID) + "/f:" + ofToString(mPerson->frequency) + "/d:" + ofToString(mPerson->diameter) + "/h:" + ofToString(mPerson->height - mPerson->length);
        ofDrawBitmapStringHighlight(mText, ofGetAppPtr()->mouseX + 5, ofGetAppPtr()->mouseY);
    
        ofSetColor(0, 0, 0);
        ofDrawBitmapStringHighlight(message, 20,ofGetHeight()-40);
        if(outputState){
            ofDrawBitmapStringHighlight("stereo audio (press a to switch) ", 20, ofGetHeight()-20);
        }else{
            ofDrawBitmapStringHighlight("8-channel audio (press a to switch)", 20,ofGetHeight()-20);
        }
    

        for(int i=0; i<currentInput; i++){
            // draw center of blobs
            string pText = ofToString(i) + " " + ofToString(persons[i]->pipeID) + " " + ofToString(persons[i]->frequency);
            ofSetColor(255, 255, 0);
            ofFill();
            ofCircle(persons[i]->x, persons[i]->y, 5);
            ofDrawBitmapStringHighlight(pText, persons[i]->x+5, persons[i]->y-5);
            ofSetColor(255);
        }
    
        if (!bServerConnected) {
            ofDrawBitmapStringHighlight("not connected (s to stop trying)" + ofToString(tryConnecting), 20, ofGetHeight()-60);
        }else{
            ofDrawBitmapStringHighlight("connected", 20, ofGetHeight()-60);
        }
    }
    else if(grafics){
        ofBackground(0);
        for (int i = 0; i < TUBE_NUM; i++){
            allPipes[i]->drawGrafik();
        }
        
        for(int i=0; i<currentInput; i++){
            if(persons[i]->x>0){
                ofSetColor(255,123,172);
                ofFill();
                ofCircle(persons[i]->x, persons[i]->y, 10);
            }
            
            
            if(persons[i]->frequency>0){
                ofRectangle rect = f.getStringBoundingBox(ofToString(persons[i]->frequency,2), 0, 0);
                ofSetColor(0);
                ofRect(persons[i]->x-rect.width-5, persons[i]->y-5,rect.width+10,rect.height+10);
                ofSetColor(255  );
                f.drawString(ofToString(persons[i]->frequency,2),persons[i]->x-rect.width, persons[i]->y+rect.height);
            }
        }
        
    }
    
}

//--------------------------------------------------------------
void AppCore::exit() {

    threadedObject.stopThread();
}

//--------------------------------------------------------------
void AppCore::playTone(int pitch) {
	pd << StartMessage() << "pitch" << pitch << FinishList("tone") << Bang("tone");
}

void AppCore::mouseMoved(int x, int y ){
    mPerson->x = x;
    mPerson->y = y;
}

//--------------------------------------------------------------
void AppCore::keyPressed (int key) {
    if(key=='a'){
        outputState = !outputState;
    }else if(key=='s'){
        tryConnecting = !tryConnecting;
    }else if(key=='g'){
        grafics = !grafics;
        fullScreen = !fullScreen;
        ofSetFullscreen(fullScreen);
    }

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

//--------------------------------------------------------------
void AppCore::onConnect( ofxLibwebsockets::Event& args ){
    cout<<"on connected"<<endl;
}

//--------------------------------------------------------------
void AppCore::onOpen( ofxLibwebsockets::Event& args ){
    cout<<"on open"<<endl;
}

//--------------------------------------------------------------
void AppCore::onClose( ofxLibwebsockets::Event& args ){
    cout<<"on close"<<endl;
}

//--------------------------------------------------------------
void AppCore::onError( ofxLibwebsockets::Event& args ){
    cout<<"on error "<<endl;
}

//--------------------------------------------------------------
void AppCore::onIdle( ofxLibwebsockets::Event& args ){
    cout<<"on idle"<<endl;
}

//--------------------------------------------------------------
void AppCore::onMessage( ofxLibwebsockets::Event& args ){
    cout<<"got message "<<args.message<<endl;
}

//--------------------------------------------------------------
void AppCore::onBroadcast( ofxLibwebsockets::Event& args ){
    cout<<"got broadcast "<<args.message<<endl;
}

