#include "DrawQuad.h"

SamplerState surfaceSampler : register(s0);
Texture2D<float3> surfaceTexture : register(t0);

cbuffer SScaleQuadPixelConstants : register(b0)
{
	float4 dxdy_Unused;
};

struct SDrawQuadPixelOutput
{
	float4 color : SV_TARGET;
};

float3 SamplePixel(int2 coord)
{
	return surfaceTexture.Load(int3(coord, 0)).rgb;
}

SDrawQuadPixelOutput PSMain(SDrawQuadPixelInput input)
{
	float dx = dxdy_Unused.x;
	float dy = dxdy_Unused.y;

	float2 pixelTopLeftCoord = max(0.0, input.texCoord.xy - float2(dx, dy) * 0.5);
	float2 pixelBottomRightCoord = pixelTopLeftCoord + min(float2(1.0, 1.0), float2(dx, dy));

	int2 topLeftCoordInteger = int2(floor(pixelTopLeftCoord));
	int2 bottomRightCoordInteger = int2(floor(pixelBottomRightCoord));

	float2 interpolators = saturate((pixelBottomRightCoord - float2(bottomRightCoordInteger)) / float2(dx, dy));

	float3 topLeftColor = SamplePixel(topLeftCoordInteger);
	float3 topRightColor = SamplePixel(int2(bottomRightCoordInteger.x, topLeftCoordInteger.y));
	float3 bottomLeftColor = SamplePixel(int2(topLeftCoordInteger.x, bottomRightCoordInteger.y));
	float3 bottomRightColor = SamplePixel(bottomRightCoordInteger);

	float3 topColor = (1.0 - interpolators.x) * topLeftColor + interpolators.x * topRightColor;
	float3 bottomColor = (1.0 - interpolators.x) * bottomLeftColor + interpolators.x * bottomRightColor;
	float3 interpolatedColor = (1.0 - interpolators.y) * topColor + interpolators.y * bottomColor;

	SDrawQuadPixelOutput result;
	result.color = float4(interpolatedColor, 1.0);

	return result;
}
