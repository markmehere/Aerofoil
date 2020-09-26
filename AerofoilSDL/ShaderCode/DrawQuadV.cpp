#define GP_GL_SHADER_CODE_DRAWQUADV_GLSL	"uniform vec4 ndcOriginAndDimensions;\n"\
"uniform vec4 surfaceDimensions_TextureRegion;\n"\
"attribute vec2 posUV;\n"\
"varying vec4 texCoord;\n"\
"\n"\
"void main()\n"\
"{\n"\
"	vec2 ndcPos = ndcOriginAndDimensions.xy + posUV.xy * ndcOriginAndDimensions.zw;\n"\
"\n"\
"	gl_Position = vec4(ndcPos.x, ndcPos.y, 0.0, 1.0);\n"\
"	texCoord = vec4(posUV.xyxy * surfaceDimensions_TextureRegion.zwxy);\n"\
"}\n"

namespace GpBinarizedShaders
{
	const char *g_drawQuadV_GL2 = GP_GL_SHADER_CODE_DRAWQUADV_GLSL;
}
