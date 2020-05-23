float SRGBToLinear(float v)
{
	if (v <= 0.04045)
		return v * (1.0 / 12.92);
	else
		return pow(((v + 0.055) * (1.0 / 1.055)), 2.4);
}

float2 SRGBToLinear(float2 v)
{
	return float2(SRGBToLinear(v.x), SRGBToLinear(v.y));
}

float3 SRGBToLinear(float3 v)
{
	return float3(SRGBToLinear(v.x), SRGBToLinear(v.y), SRGBToLinear(v.z));
}

float LinearToSRGB(float v)
{
	if (v <= 0.0031308)
		return 12.92 * v;
	else
		return 1.055 * pow(v, 1.0 / 2.4) - 0.055;
}

float2 LinearToSRGB(float2 v)
{
	return float2(LinearToSRGB(v.x), LinearToSRGB(v.y));
}

float3 LinearToSRGB(float3 v)
{
	return float3(LinearToSRGB(v.x), LinearToSRGB(v.y), LinearToSRGB(v.z));
}

float4 ApplyFlicker(int2 coordinate, int startThreshold, int endThreshold, float4 color)
{
	int flickerTotal = dot(constants_FlickerAxis, coordinate);
	if (flickerTotal < startThreshold)
		return float4(0, 0, 0, 0);
	else if (flickerTotal >= endThreshold)
		return color;

	return float4(1, 1, 1, 1);
}

float4 ApplyDesaturation(float desaturation, float4 color)
{
	if (desaturation == 0.0)
		return color;
	
	float3 srgbColor = LinearToSRGB(color.rgb);
	float grayLevel = dot(srgbColor, float3(3.0, 6.0, 1.0) / 10.0);

	srgbColor = srgbColor * (1.0 - desaturation) + float3(grayLevel, grayLevel, grayLevel) * desaturation;
	
	return float4(SRGBToLinear(srgbColor), color.a);
}
