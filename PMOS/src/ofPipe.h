//
//  ofPipe.h
//  PMOS
//
//  Created by Matti Niinimäki on 10/9/14.
//
//

#ifndef _OF_PIPE
#define _OF_PIPE

#include "ofMain.h"

class ofPipe {
    
public: // place public functions or variables declarations here
    
    // methods, equivalent to specific functions of your class objects
    void update();  // update method, used to refresh your objects properties
    void draw();    // draw method, this where you'll do the object's drawing
    void drawGrafik();
    // variables
    float x;        // position
    float y;
    int radius;        // radius
    float length;   
    float height;
    float frequency;
    int idNum;      // number of the pipe
    int element;    // element number
    bool isHit;    // is it triggered
    int openClosed;
    // constructor - used to initialize an object, if no properties are passed the program sets them to the default value
    ofPipe(float _x, float _y, float _radius, float _length, float _height, float _frequency, int _idNum, int _element, int _open);
    
private: // place private functions or variables declarations here
    
}; // dont't forget the semicolon!!

#endif /* defined(_OF_PIPE) */
