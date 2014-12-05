//
//  Externals.h
//  sib_monument
//
//  Created by Karina Jensen on 05/11/14.
//
//



#pragma once

extern "C" {
    void pink_tilde_setup();
    void spigot_tilde_setup();
    void vbap_setup();
    void expr_setup();
    void demultiplex_tilde_setup();
    void drunk_setup();
    
}




class Externals{
    
public:
    
    static void setup(){
        pink_tilde_setup();
        spigot_tilde_setup();
        vbap_setup();
        expr_setup();
        demultiplex_tilde_setup();
        drunk_setup();
    }
};

