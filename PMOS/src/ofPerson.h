//
//  person.h
//  sib_monument
//
//  Created by Karina Jensen on 13/10/14.
//
//

#ifndef __sib_monument__person__
#define __sib_monument__person__

#include <iostream>
#include "ofMain.h"




class ofPerson {
    
public: // place public functions or variables declarations here
    
    // methods, equivalent to specific functions of your class objects
    void update();  // update method, used to refresh your objects properties
    void draw();    // draw method, this where you'll do the object's drawing
    
    // variables
    float x;        // position
    float y;
    
    float frequency;
    int idNum;      
    float height;
    float diameter;
    float length;
    int openClosed;
    
    
    bool isHere;    // is it triggered
    
    // constructor - used to initialize an object, if no properties are passed the program sets them to the default value
    ofPerson(float _x, float _y, float _freq, int _idNum);
    
private: // place private functions or variables declarations here
    
}; // dont't forget the semicolon!!



#endif /* defined(__sib_monument__person__) */
