//
//  ofPipe.cpp
//  PMOS
//
//  Created by Matti Niinimäki on 10/9/14.
//  
//

#include "ofPipe.h"

ofPipe::ofPipe(float _x, float _y, float _radius, float _length, float _height, float _frequency, int _idNum, int _element)
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
}

void ofPipe::update(){
    
}

void ofPipe::draw(){
    ofNoFill();
    if(isHit){
        ofSetColor(255, 80, 80);
    }else{
        ofSetColor(30, 80, 80);
    }
    ofCircle(x, y, radius);
}
