#include "StuffItCommon.h"


CompressionMethod_t StuffItCommon::ResolveCompressionMethod(int stuffItMethod)
{
	switch (stuffItMethod)
	{
	case 0:
		return CompressionMethods::kNone;
	case 1:
		return CompressionMethods::kStuffItRLE90;
	case 2:
		return CompressionMethods::kStuffItLZW;
	case 3:
		return CompressionMethods::kStuffItHuffman;
	case 5:
		return CompressionMethods::kStuffItLZAH;
	case 6:
		return CompressionMethods::kStuffItFixedHuffman;
	case 8:
		return CompressionMethods::kStuffItMW;
	case 13:
		return CompressionMethods::kStuffIt13;
	case 14:
		return CompressionMethods::kStuffIt14;
	case 15:
		return CompressionMethods::kStuffItArsenic;
	default:
		return CompressionMethods::kUnknown;
	}
}
