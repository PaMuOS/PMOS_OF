//
//  person.cpp
//  sib_monument
//
//  Created by Karina Jensen on 13/10/14.
//
//

#include "ofPerson.h"


ofPerson::ofPerson(float _x, float _y, float _freq, int _idNum)
{
    x = _x;
    y = _y;
    idNum = _idNum;

    frequency = _freq;
    pX = _x;
    pY = _y;
    
    isHere = true;
    pipeID = 0;
    
    x = ofRandomWidth();
    y = ofRandomHeight();
}

void ofPerson::update(){
   // x += idNum/2;
   // y += idNum/2;
    
    if(x>ofGetWidth()){
        x=0;
    }
    if(y>ofGetHeight()){
        y=0;
    }
}

void ofPerson::draw(){
    ofFill();
    if(isHere){
        ofSetColor(255, 0, 0);
    }else{
        ofSetColor(0);
    }
    ofCircle(x, y, 10);
    ofSetColor(0, 255, 0);
    
}
