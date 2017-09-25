#version 120

#extension GL_ARB_texture_rectangle : enable
#extension GL_ARB_draw_buffers : enable

// ユーティリティのインクルード
#pragma include "util.frag"
// 4D Simplex Noiseのインクルード
#pragma include "noise4D.frag"

const float width = 1920.0;
const float height = 1080.0;

uniform sampler2DRect u_posAndAgeTex;
uniform sampler2DRect u_velAndMaxAgeTex;
uniform sampler2DRect u_attracterPosTex;
uniform float u_time;
uniform float u_timestep;
uniform float u_scale;
uniform vec3 u_emitterPos;
uniform vec3 u_prevEmitterPos;
uniform float u_pNum;
uniform float u_texRes;
uniform int u_sceneNum;
uniform vec3 u_sVal;
uniform vec3 u_sThresh;


void main(void){
    vec2 st = gl_TexCoord[0].st;
    vec4 posAndAge = texture2DRect(u_posAndAgeTex,st);// 前の位置情報とパーティクル初期化からの経過時間を取得
    vec4 velAndMaxAge = texture2DRect(u_velAndMaxAgeTex,st);// 前の速度と生存期間を取得
    vec3 attracterPos = texture2DRect(u_attracterPosTex,st).xyz;// アトラクターの位置情報を取得
    vec3 pos = posAndAge.xyz; // 位置
    vec3 vel = velAndMaxAge.xyz; // 速度
    vec3 acc = vec3(0); //加速度
    float age = posAndAge.w; // 経過時間
    float maxAge = velAndMaxAge.w; // 生存期間
    // FFT変換した各音域の音量
    float low = u_sVal.x;
    float mid = u_sVal.y;
    float high = u_sVal.z;
    // 音域ごとの域値
    float lowThresh = u_sThresh.x;
    float midThresh = u_sThresh.y;
    float highThresh = u_sThresh.z;
    
    age++;
    
    if (u_sceneNum <= 7) {
        if(age >= maxAge){
            age = 0;
            maxAge = 50.0 + 250.0 * random(pos.xx);
            //pos = startPos + vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
            float r = random(pos.xy) * width - width/2;
            float theta = random(pos.yz) * PI;
            float phi = random(pos.zx) * PI * 2.0;
            pos = vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
            vel.xyz = vec3(0);
        }
    } else if (u_sceneNum == 8) {
        if(age >= maxAge){
            age = 0;
            maxAge = 50.0 + 250.0 * random(pos.xx);
            float theta = 2.0 * PI * random(pos.yy);
            float phi = PI * random(pos.zz);
            float r = 5.0 * random(pos.xy);
            vec3 startVel = vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
            pos = vec3(0);
            vel.xyz = vec3(startVel);
        }
    } else if (u_sceneNum == 9) {
        if(age >= maxAge){
            age = 0;
            maxAge = 50.0 + 250.0 * random(pos.xx);
            float theta = 2.0 * PI * random(pos.yy);
            float phi = PI * random(pos.zz);
            float r = 5.0 * random(pos.xy);
            vec3 startPos = u_prevEmitterPos + (u_emitterPos - u_prevEmitterPos) * random(pos.yz);
            pos = startPos + vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
            vec3 startVel = vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
            vel.xyz = vec3(startVel);
        }
    }
    
    
    float maxSpeed = 5.0;
    float maxForce = 1.0;
    
    
    if (u_sceneNum == 1) {
        
        //curl noise
        if (low>lowThresh) {
            vel.x = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 0.1352 * u_time * u_timestep));
            vel.y = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 1.2814 * u_time * u_timestep));
            vel.z = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 2.5564 * u_time * u_timestep));
            vel*=10.0;
        }
        pos += vel;
        
    } else if (u_sceneNum == 2) {
        
        //weathering
        if (age <= maxAge * 0.7) {
            vel.x = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 0.1352 * u_time * u_timestep));
            vel.y = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 1.2814 * u_time * u_timestep));
            vel.z = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 2.5564 * u_time * u_timestep));
            vel *= 2.0;
            if (low > lowThresh) acc = (attracterPos - pos) * 0.5;
            vel += acc;
        } else {
            vel.y += 0.5;
        }
        pos += vel;
        if (pos.y >= 300.0) {
            pos.y = 300.0;
            vel.y = 0;
        }
        
    } else if (u_sceneNum == 3) {
    
        //fall out
        vec3 center = vec3(0,300.0,0);
        /*
        if (pos.y == 300.0 && random(pos.xy) < 0.5) {
            if (distance(center,pos)<10.0) pos.y = (low/lowThresh * -900.0 * random(pos.yz)) + 300.0;
            else if (distance(center,pos)<20.0) pos.y = (mid/midThresh * -900.0 * random(pos.yz)) + 300.0;
            else if (distance(center,pos)<30.0) pos.y = (high/highThresh * -900.0 * random(pos.yz)) + 300.0;
        }
        */
        vel.y += 0.5;
        pos += vel;
        if (pos.y >= 300.0) {
            pos.y = 300.0;
            vel += normalize(center - pos) * low/lowThresh;
        }
        
    } else if (u_sceneNum == 4) {
    
        //umeboshi
        vel.x = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 0.1352 * u_time * u_timestep));
        vel.y = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 1.2814 * u_time * u_timestep));
        vel.z = snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 2.5564 * u_time * u_timestep));
        //if (mid > midThresh) vel *= 5.0;
        /*
        if (length(pos) > mid * 200.0) {
            if (mid > midThresh) acc += -pos * 0.01;
            else acc += -pos * 0.05;
        } else {
            acc = vec3(0);
        }
         */
        vel *= 2.0;
        if (length(pos) > low * 400.0 + 100.0) {
            vel += -pos * 0.05;
        }
        
        vel += acc;
        pos += vel;
    
    } else if (u_sceneNum == 5) {
    
        //horizontal
        vel.x += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 0.1352 * u_time * u_timestep));
        vel.y += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 1.2814 * u_time * u_timestep));
        vel.z += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 2.5564 * u_time * u_timestep));
        vel *= 0.8;
        pos.y -= pos.y * 0.1;
        pos += vel;
    
    } else if (u_sceneNum == 6) {
    
        //vertical
        vel.x += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 0.1352 * u_time * u_timestep));
        vel.y += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 1.2814 * u_time * u_timestep));
        vel.z += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 2.5564 * u_time * u_timestep));
        vel *= 0.8;
        pos.xz -= pos.xz * 0.1;
        pos += vel;
    
    } else if (u_sceneNum == 7) {
    
        //wriggler
        pos -= mod(pos, 50.0);
        vel.x += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 0.1352 * u_time * u_timestep));
        vel.y += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 1.2814 * u_time * u_timestep));
        vel.z += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 2.5564 * u_time * u_timestep));
        pos += vel*low;
    
    } else if (u_sceneNum >= 8) {
        pos += vel;
    }
    
    /*
    float r, theta, phi;
    vec3 center;
    
    float num = u_texRes * st.y + st.x;
    if (num < u_pNum/2.0) {
        r = 200.0;
        theta = PI;
        phi = 0;
        center = vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
        r = 100.0;
        theta = random(pos.yz) * PI;
        phi = random(pos.zx) * PI * 2.0;
        attracterPos = center + vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
    } else {
        r = 200.0;
        theta = 0;
        phi = 0;
        center = vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
        r = 100.0;
        theta = random(pos.yz) * PI;
        phi = random(pos.zx) * PI * 2.0;
        attracterPos = center + vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
    }
    
    vel.x += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 0.1352 * u_time * u_timestep));
    vel.y += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 1.2814 * u_time * u_timestep));
    vel.z += snoise(vec4(pos.x * u_scale, pos.y * u_scale, pos.z * u_scale, 2.5564 * u_time * u_timestep));
    vel *= 0.2;
    
    vel += (attracterPos - pos) * 0.1;
    
    pos += vel;
    */
    
    
    
    gl_FragData[0].rgba = vec4(pos, age); // 位置と経過時間を出力
    gl_FragData[1].rgba = vec4(vel, maxAge); //速度と生存期間を出力
    gl_FragData[2].rgba = vec4(attracterPos, 0); //アトラクターの位置を出力

}
