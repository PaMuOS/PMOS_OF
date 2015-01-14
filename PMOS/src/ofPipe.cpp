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
    ofSetCircleResolution(300);
    ofSetLineWidth(2);
    
    if(isHit){
        
        alpha = 255;
        
        
    }else{
        
        alpha*=0.8;
        if(alpha < 0){
            alpha = 0;
        }
    }
    ofFill();
    ofSetColor(255,123,172,alpha);
    ofCircle(x, y, radius);
    ofNoFill();
    ofSetColor(0);
    ofCircle(x, y, radius);
   /* if(isHit){
        ofSetColor(0, 255, 0);
        ofDrawBitmapString(ofToString(frequency), x,y);
    }*/
   
}
