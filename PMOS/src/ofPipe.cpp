//
//  ofPipe.cpp
//  PMOS
//
//  Created by Matti Niinimäki on 10/9/14.
//  
//

#include "ofPipe.h"

ofPipe::ofPipe(float _x, float _y, float _radius, float _length, float _height, float _frequency, int _idNum, int _element, int _open)
{
    x = _x;
    y = _y;
    radius = _radius;
    length = _length;
    height = _height;
    frequency = _frequency;
    idNum = _idNum;
    element = _element;
    isHit = false;
    openClosed = _open;
}

void ofPipe::update(){
    
}

void ofPipe::draw(){
    ofSetLineWidth(4);
    ofFill();
    if(isHit){
        ofSetColor(255, 80, 80);
    }else{
        if(openClosed==1){
            ofSetColor(200);
        }else{
            ofSetColor(230);
        }
    }
    ofCircle(x, y, radius);
    ofNoFill();
    ofSetLineWidth(2);
    ofSetColor(10, 20, 20);
    ofCircle(x, y, radius);
    if(openClosed==2){
        //ofDrawBitmapStringHighlight(ofToString(idNum), x,y);
    }
}

void ofPipe::drawGrafik(){
    ofSetLineWidth(1);
    ofSetColor(255);
    ofNoFill();
    if(isHit){
        ofFill();
        
    }else{
        ofNoFill();
    }
    if(openClosed==1){
        ofCircle(x, y, radius-4);
        //ofCircle(x, y, radius-8);
        ofCircle(x, y, radius);
        //ofSetColor(200);
    }else{
        ofCircle(x, y, radius);
    }

   
}
