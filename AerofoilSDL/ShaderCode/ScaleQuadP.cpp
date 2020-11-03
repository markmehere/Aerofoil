#include "Functions.h"

#define GP_GL_SHADER_CODE_SCALEQUADP_GLSL	"uniform sampler2D surfaceTexture;\n"\
"varying vec4 texCoord;\n"\
"\n"\
"uniform vec4 dxdy_dimensions;\n"\
"\n"\
"void main()\n"\
"{\n"\
"	vec2 tc = texCoord.xy;\n"\
"\n"\
"	gl_FragColor = vec4(texture2D(surfaceTexture, tc).rgb, 1.0);\n"\
"}\n"

namespace GpBinarizedShaders
{
	const char *g_scaleQuadP_GL2 = GP_GL_SHADER_CODE_MEDIUM_PRECISION_PREFIX GP_GL_SHADER_CODE_FUNCTIONS_H GP_GL_SHADER_CODE_SCALEQUADP_GLSL;

	extern const char *g_drawQuadRGBP_GL2;
	extern const char *g_drawQuad15BitP_GL2;

	extern const char *g_drawQuadRGBICCP_GL2;
	extern const char *g_drawQuad15BitICCP_GL2;

}
