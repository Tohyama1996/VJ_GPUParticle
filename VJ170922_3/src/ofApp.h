#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxGui.h"
#include "ofxProcessFFT.h"
#include "pingPongBuffer.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void keyReleased(int key);
    
    int sceneNum, colorMode, camState, rotateMode;
    bool debugMode;
    
    float time, camTime, timeStamp, timeStamp2;
    ofVec3f rotation;
    int targetNum;
    vector<ofVec3f> targets;
		
    ofVboMesh particles;
    ofShader render, updatePos;
    ofEasyCam cam;
    pingPongBuffer pingPong;
    ofVec3f emitterPos, prevEmitterPos;
    int particleNum, texRes;
    
    ofSpherePrimitive sphere;
    vector<ofxAssimpModelLoader> man;
    int modelNum;
    ofVboMesh mesh;
    ofVec3f modelScale;
    
    ofxPanel panel;
    ofxFloatSlider lowThresh, midThresh, highThresh;
    
    ProcessFFT fft;
    float low, mid, high;
    
};
