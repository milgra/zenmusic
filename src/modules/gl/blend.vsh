"\
attribute vec3 position;\
attribute vec2 texcoord;\
\
uniform mat4 projection;\
\
varying vec2 vUv;\
\
void main ( )\
{\
    gl_Position = projection * vec4(position,1.0);\
    vUv = texcoord;\
}\
"
