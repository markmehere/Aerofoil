#include "DrawQuad.h"

SamplerState surfaceSampler : register(s0);
Texture2D<float> surfaceTexture : register(t0);

struct SDrawQuadPixelOutput
{
	float4 color : SV_TARGET;
};

SDrawQuadPixelOutput PSMain(SDrawQuadPixelInput input)
{
	float surfaceColor = surfaceTexture.Sample(surfaceSampler, input.texCoord.xy).r;

	SDrawQuadPixelOutput result;
	result.color = float4(surfaceColor, input.texCoord.x, input.texCoord.y, 1.0);
	return result;
}
