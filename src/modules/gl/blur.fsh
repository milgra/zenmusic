"#version 120\
\
uniform sampler2D samplera;\
\
varying vec3 vUv;\
\
uniform float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);\
uniform float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);\
\
void main( )\
{\
   gl_FragColor = texture2D(samplera, vUv.xy / 1024.0) * weight[0];\
    for (int i=1; i<3; i++) {\
        gl_FragColor += texture2D(samplera, (vUv.xy + vec2(0.0, offset[i])) / 1024.0) * weight[i];\
        gl_FragColor += texture2D(samplera, (vUv.xy - vec2(0.0, offset[i])) / 1024.0) * weight[i];\
    }\
}\
"