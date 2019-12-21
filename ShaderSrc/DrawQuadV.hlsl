#include "DrawQuad.h"

cbuffer SDrawQuadVertexConstants : register(b0)
{
	float4 ndcOriginAndDimensions;
	float4 surfaceDimensions_Unused;
};

struct SDrawQuadVertexInput
{
	float2 posUV : POSITION;
};

SDrawQuadPixelInput VSMain(SDrawQuadVertexInput input)
{
	SDrawQuadPixelInput result;

	float2 ndcPos = ndcOriginAndDimensions.xy + input.posUV.xy * ndcOriginAndDimensions.zw;

	result.pos = float4(ndcPos.x, ndcPos.y, 0.0, 1.0);
	result.texCoord = input.posUV.xy * surfaceDimensions_Unused.xy;

	return result;
}
