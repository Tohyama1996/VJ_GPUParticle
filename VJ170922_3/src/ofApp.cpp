#include "ofApp.h"

void ofApp::setup(){
    ofSetVerticalSync(true);
    ofBackground(0, 0, 0);
    //ofHideCursor();
    ofDisableAlphaBlending();
    ofEnableDepthTest();
    
    sceneNum = 8;
    colorMode = 4;
    camState = 1;
    rotateMode = 1;
    debugMode = false;
    
    for (int i=0; i<50; i++) {
        ofVec3f target = ofVec3f(ofRandom(360), ofRandom(360), ofRandom(360));
        targets.push_back(target);
    }
    
    fft.setup();
    fft.setNumFFTBins(16);
    fft.setNormalize(true);
    
    panel.setup();
    panel.add(lowThresh.setup("threshold_low", 0.5, 0, 1.0));
    panel.add(midThresh.setup("threshold_mid", 0.5, 0, 1.0));
    panel.add(highThresh.setup("threshold_high", 0.5, 0, 1.0));
    
    cam.setupPerspective();
    
    
    sphere.set(200, 64);
    for (int i=0; i<15; i++) {
        man.push_back(ofxAssimpModelLoader());
    }
    man[0].loadModel("model/man01.obj");
    man[1].loadModel("model/man02.obj");
    man[2].loadModel("model/man03.obj");
    man[3].loadModel("model/man04.obj");
    man[4].loadModel("model/man05.obj");
    man[5].loadModel("model/man06.obj");
    man[6].loadModel("model/man07.obj");
    man[7].loadModel("model/man08.obj");
    man[8].loadModel("model/man09.obj");
    man[9].loadModel("model/man10.obj");
    man[10].loadModel("model/man11.obj");
    man[11].loadModel("model/man12.obj");
    man[12].loadModel("model/man13.obj");
    man[13].loadModel("model/man14.obj");
    man[14].loadModel("model/man15.obj");
    modelNum = 0;
    mesh = man[modelNum].getMesh(0);
    modelScale = ofVec3f(4.0, -4.0, 4.0);
    
//    ofVboMesh mesh = sphere.getMesh();
//    ofVec3f modelScale = ofVec3f(1.0);
    
    //パーティクルの数を設定
    particleNum = 2000000;
    texRes = ceil(sqrt(particleNum));
    
    //シェーダーの読み込み
    render.load("shaders/render");
    updatePos.load("","shaders/update.frag");
    
    // パーティクルの初期設定
    particles.setMode(OF_PRIMITIVE_POINTS);
    for(int i=0;i<texRes;i++){
        for(int j=0;j<texRes;j++){
            int index = i * texRes + j;
            if(index < particleNum){
                particles.addVertex(ofVec3f(0,0,0));
                particles.addTexCoord(ofVec2f(i, j)); // Fboのテクスチャー内で、データを読み出す位置を設定
                particles.addColor(ofFloatColor(1.0,1.0,1.0,0.5));
            }
        }
    }
    
    // パーティクルの座標・加速度の保存用Fbo
    // RGBA32Fの形式で2つのColorbufferを用意
    pingPong.allocate(texRes, texRes, GL_RGBA32F, 3);
    
    // パーティクルの位置と経過時間の初期設定
	float * posAndAge = new float[texRes * texRes * 4];
    for (int x = 0; x < texRes; x++){
        for (int y = 0; y < texRes; y++){
            float r = ofRandom(1920.0);
            float theta = ofRandom(PI);
            float phi = ofRandom(PI * 2.0);
            int i = texRes * y + x;
            posAndAge[i*4 + 0] = r * sin(theta) * cos(phi);
            posAndAge[i*4 + 1] = r * sin(theta) * sin(phi);
            posAndAge[i*4 + 2] = r * cos(theta);
            posAndAge[i*4 + 3] = 0;
        }
    }
    pingPong.src->getTexture(0).loadData(posAndAge, texRes, texRes, GL_RGBA);
    delete [] posAndAge;
    
    // パーティクルの速度と生存期間の初期設定
	float * velAndMaxAge = new float[texRes * texRes * 4];
    for (int x = 0; x < texRes; x++){
        for (int y = 0; y < texRes; y++){
            int i = texRes * y + x;
            velAndMaxAge[i*4 + 0] = 0.0;
            velAndMaxAge[i*4 + 1] = 0.0;
            velAndMaxAge[i*4 + 2] = 0.0;
            velAndMaxAge[i*4 + 3] = ofRandom(1,600);
        }
    }
    pingPong.src->getTexture(1).loadData(velAndMaxAge, texRes, texRes, GL_RGBA);
    delete [] velAndMaxAge;
    
    // アトラクターの位置の初期設定
    float * attracterPos = new float[texRes * texRes * 4];
    for (int x = 0; x < texRes; x++){
        for (int y = 0; y < texRes; y++){
            int i = texRes * y + x;
            unsigned int j = i % mesh.getNumVertices();
            attracterPos[i*4 + 0] = mesh.getVertex(j).x * modelScale.x;
            attracterPos[i*4 + 1] = mesh.getVertex(j).y * modelScale.y;
            attracterPos[i*4 + 2] = mesh.getVertex(j).z * modelScale.z;
            attracterPos[i*4 + 3] = 0.0;
        }
    }
    pingPong.src->getTexture(2).loadData(attracterPos, texRes, texRes, GL_RGBA);
    delete [] attracterPos;
    
}





void ofApp::update(){
    
    time = ofGetElapsedTimef() - timeStamp;
    camTime = ofGetElapsedTimef() - timeStamp2;
    
    fft.update();
    low = fft.getLowVal();
    mid = fft.getMidVal();
    high = fft.getHighVal();
    //cout << lowVal << "," << midVal << "," << highVal << endl;
    if (rotateMode == 1) {
        rotation = ofVec3f(0);
    } else if (rotateMode == 2) {
        rotation.x = 0;
        rotation.z = 0;
        rotation.y += 0.5;
    } else if (rotateMode ==3) {
        rotation.x = 0;
        rotation.z = 0;
        rotation.y -= 5.0;
        //rotation.y += 0.5;
        //if (mid > midThresh) rotation.y -= mid * 10;
    } else if (rotateMode == 4) {
        if (camState == 1 && low > lowThresh) {
            timeStamp2 = ofGetElapsedTimef();
            targetNum = ofRandom(50);
            camState = 2;
        } else if (camState == 2) {
            if (camTime < 360) rotation += (targets[targetNum] - rotation) * 0.1;
            else camState = 1;
            if (low > lowThresh && camTime > 0.2) {
                timeStamp2 = ofGetElapsedTimef();
                camState = 1;
            }
        }
    }
    if (rotation.x > 360.0) rotation.x = fmodf(rotation.x, 360);
    if (rotation.y > 360.0) rotation.y = fmodf(rotation.y, 360);
    if (rotation.z > 360.0) rotation.z = fmodf(rotation.z, 360);
    if (rotation.x < 0.0) rotation.x = rotation.x += 360;
    if (rotation.y < 0.0) rotation.y = rotation.y += 360;
    if (rotation.z < 0.0) rotation.z = rotation.z += 360;
    
    // パーティクルの発生位置を更新
    prevEmitterPos = emitterPos;
    emitterPos = 300 * ofVec3f(ofSignedNoise(time, 0, 0),ofSignedNoise(0, time, 0),ofSignedNoise(0, 0, time));
    
    // パーティクルの位置を計算
    pingPong.dst->begin();

    // 複数バッファの書き出しを有効化
    pingPong.dst->activateAllDrawBuffers();
    ofClear(0);
    
    updatePos.begin();
    // パーティクルの位置と経過時間
    updatePos.setUniformTexture("u_posAndAgeTex", pingPong.src->getTexture(0), 0);
    // パーティクルの速度と生存期間
    updatePos.setUniformTexture("u_velAndMaxAgeTex", pingPong.src->getTexture(1), 1);
    // アトラクターの位置
    updatePos.setUniformTexture("u_attracterPosTex", pingPong.src->getTexture(2), 2);
    updatePos.setUniform1f("u_time", time);
    updatePos.setUniform1f("u_timestep", 0.5);
    updatePos.setUniform1f("u_scale", 0.01);
    updatePos.setUniform3f("u_emitterPos", emitterPos.x, emitterPos.y, emitterPos.z);
    updatePos.setUniform3f("u_prevEmitterPos", prevEmitterPos.x, prevEmitterPos.y, prevEmitterPos.z);
    updatePos.setUniform1f("u_pNum", (float)particleNum);
    updatePos.setUniform1f("u_texRes", (float)texRes);
    updatePos.setUniform1i("u_sceneNum", sceneNum);
    updatePos.setUniform3f("u_sVal", low, mid, high);
    updatePos.setUniform3f("u_sThresh", lowThresh, midThresh, highThresh);
    pingPong.src->draw(0, 0);
    updatePos.end();
    
    pingPong.dst->end();
    pingPong.swap();
    
    
}





void ofApp::draw(){
    
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofRotateX(rotation.x);
    ofRotateY(rotation.y);
    ofRotateZ(rotation.z);
    
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofEnableDepthTest();
    ofEnablePointSprites();
    render.begin();
    // パーティクルの位置と経過時間
    render.setUniformTexture("u_posAndAgeTex", pingPong.src->getTexture(0), 0);
    render.setUniform1f("u_width", ofGetWidth());
    render.setUniform1f("u_height", ofGetHeight());
    render.setUniform1f("u_time", ofGetElapsedTimef());
    render.setUniform1i("u_colorMode", colorMode);
    render.setUniform3f("u_sVal", low, mid, high);
    render.setUniform3f("u_sThresh", lowThresh, midThresh, highThresh);
    
    particles.draw();
    
    render.end();
    ofDisablePointSprites();
    ofPopStyle();
    ofPopMatrix();
    
    if ( debugMode ) {
        float x, y, w, h;
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofDisableDepthTest();
        
        x = 0;
        y = ofGetHeight();
        w = ofGetWidth()/3;
        h = low*ofGetHeight()*-1;
        ofFill();
        if (low > lowThresh) ofSetColor(255, 0, 0, 100);
        else ofSetColor(255, 100);
        ofDrawRectangle(x, y, w, h);
        ofNoFill();
        ofSetColor(255, 0, 0, 100);
        ofDrawLine(x, y-lowThresh*y, x+w, y-lowThresh*y);
        
        
        x = ofGetWidth()/3;
        h = mid*ofGetHeight()*-1;
        ofFill();
        if (mid > midThresh) ofSetColor(255, 0, 0, 100);
        else ofSetColor(255, 100);
        ofDrawRectangle(x, y, w, h);
        ofNoFill();
        ofSetColor(255, 0, 0, 100);
        ofDrawLine(x, y-midThresh*y, x+w, y-midThresh*y);
        
        
        x = ofGetWidth()*2/3;
        h = high*ofGetHeight()*-1;
        ofFill();
        if (high > highThresh) ofSetColor(255, 0, 0, 100);
        else ofSetColor(255, 100);
        ofDrawRectangle(x, y, w, h);
        ofNoFill();
        ofSetColor(255, 0, 0, 100);
        ofDrawLine(x, y-highThresh*y, x+w, y-highThresh*y);
        
        ofPopStyle();
        
        panel.draw();
        
        ofDrawBitmapStringHighlight("FrameRate = " + ofToString(ofGetFrameRate()), 0, ofGetHeight() - 5);
        ofDrawBitmapStringHighlight("ParticleNum = " + ofToString(particleNum), 0, ofGetHeight() - 20);
    }
    
}





void ofApp::keyPressed(int key){

}





void ofApp::keyReleased(int key){
    float * attracterPos;
    switch (key) {
        case '1':
            sceneNum = 1;
            timeStamp = ofGetElapsedTimef();
            break;
        case '2':
            sceneNum = 2;
            timeStamp = ofGetElapsedTimef();
            break;
        case '3':
            sceneNum = 3;
            timeStamp = ofGetElapsedTimef();
            break;
        case '4':
            sceneNum = 4;
            timeStamp = ofGetElapsedTimef();
            break;
        case '5':
            sceneNum = 5;
            rotateMode = 2;
            timeStamp = ofGetElapsedTimef();
            break;
        case '6':
            sceneNum = 6;
            rotateMode = 2;
            timeStamp = ofGetElapsedTimef();
            break;
        case '7':
            sceneNum = 7;
            timeStamp = ofGetElapsedTimef();
            break;
        case '8':
            sceneNum = 8;
            timeStamp = ofGetElapsedTimef();
            break;
            
        case 'q':
            rotateMode = 1;
            break;
        case 'w':
            rotateMode = 2;
            break;
        case 'e':
            rotateMode = 3;
            break;
        case 'r':
            rotateMode = 4;
            break;
            
        case 'y':
            colorMode = 1;
            break;
        case 'u':
            colorMode = 2;
            break;
        case 'i':
            colorMode = 3;
            break;
        case 'o':
            colorMode = 4;
            break;
        case 'p':
            colorMode = 5;
            break;
            
        case 'a':
            modelNum++;
            if (modelNum >= 15) modelNum = 0;
            mesh = man[modelNum].getMesh(0);
            
            // アトラクターの位置を更新
            attracterPos = new float[texRes * texRes * 4];
            for (int x = 0; x < texRes; x++){
                for (int y = 0; y < texRes; y++){
                    int i = texRes * y + x;
                    unsigned int j = i % mesh.getNumVertices();
                    attracterPos[i*4 + 0] = mesh.getVertex(j).x * modelScale.x;
                    attracterPos[i*4 + 1] = mesh.getVertex(j).y * modelScale.y;
                    attracterPos[i*4 + 2] = mesh.getVertex(j).z * modelScale.z;
                    attracterPos[i*4 + 3] = 0.0;
                }
            }
            pingPong.src->getTexture(2).loadData(attracterPos, texRes, texRes, GL_RGBA);
            delete [] attracterPos;
            break;
            
        case '_':
            debugMode = !debugMode;
            break;
            
            
        default:
            break;
    }
}
