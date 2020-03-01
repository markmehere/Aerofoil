#include "PLStandardColors.h"
#include "RGBAColor.h"

PortabilityLayer::RGBAColor StdColors::Black()
{
	return PortabilityLayer::RGBAColor::Create(0, 0, 0, 255);
}

PortabilityLayer::RGBAColor StdColors::White()
{
	return PortabilityLayer::RGBAColor::Create(255, 255, 255, 255);
}

PortabilityLayer::RGBAColor StdColors::Red()
{
	return PortabilityLayer::RGBAColor::Create(255, 0, 0, 255);
}

PortabilityLayer::RGBAColor StdColors::Green()
{
	return PortabilityLayer::RGBAColor::Create(0, 255, 0, 255);
}

PortabilityLayer::RGBAColor StdColors::Blue()
{
	return PortabilityLayer::RGBAColor::Create(0, 0, 255, 255);
}

PortabilityLayer::RGBAColor StdColors::Magenta()
{
	return PortabilityLayer::RGBAColor::Create(255, 0, 255, 255);
}
