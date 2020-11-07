"\
uniform sampler2D samplera;\
uniform sampler2D samplerb;\
\
varying vec3 vUv;\
\
void main( )\
{\
	if (vUv.z == 1.0)\
	{\
		gl_FragColor = texture2D(samplerb, vUv.xy);\
	}\
	else\
	{\
		gl_FragColor = texture2D(samplera, vUv.xy);\
	}\
}\
"
