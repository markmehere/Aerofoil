#include "Functions.h"

#define GP_GL_SHADER_CODE_COPYQUADP_GLSL	"uniform sampler2D surfaceTexture;\n"\
"varying vec4 texCoord;\n"\
"\n"\
"uniform vec4 dxdy_dimensions;\n"\
"\n"\
"void main()\n"\
"{\n"\
"	gl_FragColor = vec4(LinearToSRGB(texture2D(surfaceTexture, texCoord.xy).rgb), 1.0);\n"\
"}\n"

namespace GpBinarizedShaders
{	
	//macos does not support precision in OpenGLES2
	#ifdef __MACOS__
	const char *g_copyQuadP_GL2 = GP_GL_SHADER_CODE_FUNCTIONS_H GP_GL_SHADER_CODE_COPYQUADP_GLSL;
	#else
	const char *g_copyQuadP_GL2 = GP_GL_SHADER_CODE_MEDIUM_PRECISION_PREFIX GP_GL_SHADER_CODE_FUNCTIONS_H GP_GL_SHADER_CODE_COPYQUADP_GLSL;
	#endif
}
