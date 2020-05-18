cbuffer SDrawQuadPixelConstants : register(b0)
{
	float4 constants_Modulation;
	
	int2 constants_FlickerAxis;
	int constants_FlickerStartThreshold;
	int constants_FlickerEndThreshold;
};

float4 ApplyFlicker(int2 coordinate, float4 color)
{
	int flickerTotal = dot(constants_FlickerAxis, coordinate);
	if (flickerTotal < constants_FlickerStartThreshold)
		return float4(0, 0, 0, 0);
	else if (flickerTotal >= constants_FlickerEndThreshold)
		return color;
	else
		return float4(1, 1, 1, 1);
}
