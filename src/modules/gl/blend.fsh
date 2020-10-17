"\
uniform sampler2D tex0;\
uniform sampler2D tex1;\
\
varying vec2 vUv;\
\
void main( )\
{\
	gl_FragColor = texture2D(tex1, vUv);\
}\
"
