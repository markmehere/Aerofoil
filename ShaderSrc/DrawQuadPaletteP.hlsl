#include "DrawQuad.h"

SamplerState nearestNeighborSampler : register(s0);
Texture2D<uint> surfaceTexture : register(t0);
Texture1D<float4> paletteTexture : register(t1);

struct SDrawQuadPixelOutput
{
	float4 color : SV_TARGET;
};

SDrawQuadPixelOutput PSMain(SDrawQuadPixelInput input)
{
	int2 texCoord = input.texCoord.xy;

	uint surfaceColor = surfaceTexture.Load(int3(texCoord, 0)).r;
	float3 paletteColor = paletteTexture.Load(int2(surfaceColor, 0)).rgb;

	SDrawQuadPixelOutput result;
	result.color = float4(paletteColor.rgb, 1.0);
	return result;
}
