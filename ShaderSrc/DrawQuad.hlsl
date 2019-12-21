cbuffer SDrawQuadVertexConstants : register(b0)
{
	float4 ndcOriginAndDimensions;
};

struct SVertexInput
{
	float3 posUV : POSITION;
};

struct SPixelInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

struct SPixelOutput
{
	float4 color : SV_TARGET;
};

SPixelInput VSMain(SVertexInput input)
{
	SPixelInput result;
	
	result.pos = float4(input.posUV.x, input.posUV.y, input.posUV.z, 1.0);
	result.texCoord = input.posUV.xy;

	return result;
}

SPixelOutput PSMain(SPixelInput input)
{
	SPixelOutput result;
	result.color = float4(input.texCoord.x, input.texCoord.y, 0.0, 1.0);
	return result;
}
