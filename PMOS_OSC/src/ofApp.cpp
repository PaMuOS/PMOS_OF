#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    // open an outgoing connection to HOST:PORT
	sender.setup(HOST, PORT);
    
    allPipes = new ofPipe*[TUBE_NUM]; // an array of pointers for the objects
    
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
}

//--------------------------------------------------------------
void ofApp::update(){
    for (int i = 0; i < TUBE_NUM; i++){
        allPipes[i]->update();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    for (int i = 0; i < TUBE_NUM; i++){
        allPipes[i]->draw();
    }
    
    ofSetColor(0, 0, 0);
    ofDrawBitmapString(message, 20,20);
    ofDrawBitmapString("Sending the frequency as OSC message to port " + ofToString(PORT), 600,40);
    ofDrawBitmapString("frequency:  " + ofToString(sendFreq), 600,60);
    ofDrawBitmapString("Radius:  " + ofToString(sendDia,4), 600,80);
    ofDrawBitmapString("Height:  " + ofToString(sendHeight), 600,100);
    ofDrawBitmapString("XY:  " + ofToString(mouseX) + " " + ofToString(mouseY), 600,120);
    ofDrawBitmapString(info, mouseX+6,mouseY);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
    info = "";
    sendFreq = 0;
    
    for(int i=0; i<TUBE_NUM; i++){
        float distance = ofDist(allPipes[i]->x, allPipes[i]->y, x, y);
        if(distance < allPipes[i]->radius){
            allPipes[i]->isHit=true;
            sendFreq = allPipes[i]->frequency;
            
            
            sendDia =  allPipes[i]->radius*2.4;
            sendHeight = allPipes[i]->height-allPipes[i]->length;
            
            info = ofToString( allPipes[i]->idNum) + " - " + ofToString(sendFreq);
            
        }else{
            allPipes[i]->isHit=false;
        }
    }
    ofxOscMessage m;
	m.setAddress("/freq");
	m.addFloatArg(sendFreq);
	sender.sendMessage(m);
    ofxOscMessage m2;
	m2.setAddress("/diameter");
	m2.addFloatArg(sendDia);
	sender.sendMessage(m2);
    ofxOscMessage m3;
	m3.setAddress("/height");
	m3.addFloatArg(sendHeight);
	sender.sendMessage(m3);
    ofxOscMessage m4;
    m4.setAddress("/xy");
    m4.addFloatArg(ofMap(x, 0, ofGetWidth(), 0, 1));
    m4.addFloatArg(ofMap(y, 0, ofGetHeight(), 1, 0));
    sender.sendMessage(m4);
    
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
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
