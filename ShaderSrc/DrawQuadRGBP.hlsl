#include "DrawQuad.h"

SamplerState surfaceSampler : register(s0);
Texture2D<float3> surfaceTexture : register(t0);

struct SDrawQuadPixelOutput
{
	float4 color : SV_TARGET;
};

SDrawQuadPixelOutput PSMain(SDrawQuadPixelInput input)
{
	float3 surfaceColor = surfaceTexture.Load(int3(input.texCoord.xy, 0)).rgb;

	SDrawQuadPixelOutput result;
	result.color = float4(surfaceColor, 1.0);
	return result;
}
