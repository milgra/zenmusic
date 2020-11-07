"\
attribute vec3 position;\
attribute vec3 texcoord;\
\
uniform mat4 projection;\
uniform sampler2D samplera;\
\
varying vec3 vUv;\
\
void main ( )\
{\
    gl_Position = projection * vec4(position,1.0);\
    vUv = texcoord;\
}\
"
