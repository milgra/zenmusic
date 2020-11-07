"\
attribute vec3 position;\
attribute vec3 texcoord;\
\
uniform mat4 projection;\
\
void main ( )\
{\
    gl_Position = projection * vec4(position,1.0);\
}\
"
