#version 120
#extension GL_ARB_texture_rectangle : enable

uniform float u_width;
uniform float u_height;
uniform float u_time;
uniform int u_colorMode;
uniform vec3 u_sVal;
uniform vec3 u_sThresh;

vec3 colorA = vec3(0.3,0.5,0.9);
vec3 colorB = vec3(0.3,0.6,0.6);

void main() {
    
    // FFT変換した各音域の音量
    float low = u_sVal.x;
    float mid = u_sVal.y;
    float high = u_sVal.z;
    // 音域ごとの域値
    float lowThresh = u_sThresh.x;
    float midThresh = u_sThresh.y;
    float highThresh = u_sThresh.z;
    
    
    if (u_colorMode == 1) {
        //white
        //gl_FragColor = vec4(1.0,1.0,1.0,low/lowThresh);
        //green and blue
        vec2 center = vec2(u_width/2.0, u_height/2.0);
        float dist = distance(center, gl_FragCoord.xy);
        dist = smoothstep(0, u_width/2.0, dist) * 0.5;
        float pct = (sin((dist-u_time*5.0)*2.0)+1.0)/2.0;
        vec3 c = mix(colorA, colorB, pct);
        gl_FragColor = vec4(c, 1.0);
    } else if (u_colorMode == 2) {
        //red
        gl_FragColor = vec4(1.0,0.8,0.8,low/lowThresh);
    } else if (u_colorMode == 3) {
        //green
        gl_FragColor = vec4(0.8,1.0,0.8,low/lowThresh);
    } else if (u_colorMode == 4) {
        //blue
        gl_FragColor = vec4(0.7,0.7,1.0,low/lowThresh);
    } else if (u_colorMode == 5) {
        //rgb audio reactive
        gl_FragColor = vec4(low+0.1, high+0.1, mid+0.1, 1.0);
    }
    
    
    //gl_FragColor = gl_Color;
    
    /*
    //green and blue
    vec2 center = vec2(u_width/2.0, u_height/2.0);
    float dist = distance(center, gl_FragCoord.xy);
    dist = smoothstep(0, u_width/2.0, dist) * 0.5;
    float pct = (sin((dist-u_time*5.0)*2.0)+1.0)/2.0;
    vec3 c = mix(colorA, colorB, pct);
    gl_FragColor = vec4(c, 1.0);
    */
    
    
}
