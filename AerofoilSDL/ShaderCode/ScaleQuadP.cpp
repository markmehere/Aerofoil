#include "Functions.h"

#define GP_GL_SHADER_CODE_SCALEQUADP_GLSL	"uniform sampler2D surfaceTexture;\n"\
"varying vec4 texCoord;\n"\
"\n"\
"uniform vec4 dxdy_dimensions;\n"\
"\n"\
"vec3 SamplePixel(vec2 coord)\n"\
"{\n"\
"	return texture2D(surfaceTexture, (coord + vec2(0.5, 0.5)) / dxdy_dimensions.zw).rgb;\n"\
"}\n"\
"\n"\
"void main()\n"\
"{\n"\
"	float dx = dxdy_dimensions.x;\n"\
"	float dy = dxdy_dimensions.y;\n"\
"	vec2 dimensions = dxdy_dimensions.zw;\n"\
"	vec2 texCoordInteger = dxdy_dimensions.zw;\n"\
"\n"\
"	vec2 pixelTopLeftCoord = max(vec2(0.0, 0.0), texCoord.zw - vec2(dx, dy) * 0.5);\n"\
"	vec2 pixelBottomRightCoord = pixelTopLeftCoord + min(vec2(1.0, 1.0), vec2(dx, dy));\n"\
"\n"\
"	vec2 topLeftCoordInteger = floor(pixelTopLeftCoord);\n"\
"	vec2 bottomRightCoordInteger = floor(pixelBottomRightCoord);\n"\
"\n"\
"	vec2 interpolators = saturate((pixelBottomRightCoord - bottomRightCoordInteger) / vec2(dx, dy));\n"\
"\n"\
"	vec3 topLeftColor = SamplePixel(topLeftCoordInteger);\n"\
"	vec3 topRightColor = SamplePixel(vec2(bottomRightCoordInteger.x, topLeftCoordInteger.y));\n"\
"	vec3 bottomLeftColor = SamplePixel(vec2(topLeftCoordInteger.x, bottomRightCoordInteger.y));\n"\
"	vec3 bottomRightColor = SamplePixel(bottomRightCoordInteger);\n"\
"\n"\
"	vec3 topColor = (1.0 - interpolators.x) * topLeftColor + interpolators.x * topRightColor;\n"\
"	vec3 bottomColor = (1.0 - interpolators.x) * bottomLeftColor + interpolators.x * bottomRightColor;\n"\
"	vec3 interpolatedColor = (1.0 - interpolators.y) * topColor + interpolators.y * bottomColor;\n"\
"\n"\
"	gl_FragColor = vec4(LinearToSRGB(interpolatedColor), 1.0);\n"\
"}\n"

namespace GpBinarizedShaders
{
	const char *g_scaleQuadP_GL2 = GP_GL_SHADER_CODE_FUNCTIONS_H GP_GL_SHADER_CODE_SCALEQUADP_GLSL;

	extern const char *g_drawQuadRGBP_GL2;
	extern const char *g_drawQuad15BitP_GL2;

	extern const char *g_drawQuadRGBICCP_GL2;
	extern const char *g_drawQuad15BitICCP_GL2;

}
