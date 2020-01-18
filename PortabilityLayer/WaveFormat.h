#pragma once

#include "PLLittleEndian.h"

namespace PortabilityLayer
{
	namespace WaveConstants
	{
		static const uint16_t kFormatPCM = 1;
		static const uint32_t kRiffChunkID = 0x46464952;
		static const uint32_t kWaveChunkID = 0x45564157;
		static const uint32_t kFormatChunkID = 0x20746d66;
		static const uint32_t kDataChunkID = 0x61746164;
	}

	struct RIFFTag
	{
		LEUInt32_t m_tag;
		LEUInt32_t m_chunkSize;		// Actual size is padded to 2-byte alignment
	};

	struct WaveFormatChunkV1
	{
		LEUInt16_t m_formatCode;
		LEUInt16_t m_numChannels;
		LEUInt32_t m_sampleRate;
		LEUInt32_t m_bytesPerSecond;
		LEUInt16_t m_blockAlignmentBytes;
		LEUInt16_t m_bitsPerSample;
	};

	struct WaveFormatChunkV2
	{
		WaveFormatChunkV1 m_v1;
		LEUInt16_t m_extSize;
	};

	struct WaveFormatChunkV3
	{
		WaveFormatChunkV2 m_v2;

		LEUInt16_t m_validBitsPerSample;
		LEUInt32_t m_channelMask;
		uint8_t m_subFormatGUID[16];
	};
}
