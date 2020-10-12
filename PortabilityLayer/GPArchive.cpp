#include "GPArchive.h"
#include "ResTypeID.h"

#include <string.h>

static const char *gs_forbiddenNames[] =
{
	"COM1",
	"COM2",
	"COM3",
	"COM4",
	"COM5",
	"COM6",
	"COM7",
	"COM8",
	"COM9",
	"LPT1",
	"LPT2",
	"LPT3",
	"LPT4",
	"LPT5",
	"LPT6",
	"LPT7",
	"LPT8",
	"LPT9",
};

static bool IsCharForbidden(char c)
{
	return (c < ' ' || c == '<' || c == '>' || c == ':' || c == '\"' || c == '/' || c == '\\' || c == '|' || c == '?' || c == '*' || c > '~' || c == '$' || c == '#');
}

namespace PortabilityLayer
{
	GpArcResourceTypeTag GpArcResourceTypeTag::Encode(const ResTypeID &tag)
	{
		static const char *nibbles = "0123456789abcdef";

		char chars[4];
		tag.ExportAsChars(chars);

		GpArcResourceTypeTag output;
		memset(&output, 0, sizeof(output));

		char *outChar = output.m_id;
		for (int i = 0; i < 4; i++)
		{
			char c = chars[i];

			bool isForbidden = IsCharForbidden(c);

			if (i == 3)
			{
				if (c == ' ' || c == '.')
					isForbidden = true;
				else
				{
					for (int fi = 0; fi < sizeof(gs_forbiddenNames) / sizeof(gs_forbiddenNames[0]); fi++)
					{
						if (!memcmp(chars, gs_forbiddenNames[fi], 4))
						{
							isForbidden = true;
							break;
						}
					}
				}
			}

			if (isForbidden)
			{
				uint8_t byteValue;
				memcpy(&byteValue, &c, 1);

				*outChar++ = '$';
				int highNibble = (byteValue >> 4) & 0xf;
				int lowNibble = (byteValue & 0xf);

				*outChar++ = nibbles[highNibble];
				*outChar++ = nibbles[lowNibble];
			}
			else
			{
				*outChar++ = c;
			}
		}

		return output;
	}

	bool GpArcResourceTypeTag::Load(const char *str)
	{
		size_t l = strlen(str);
		if (l < sizeof(m_id))
		{
			memcpy(m_id, str, l);
			m_id[l] = '\0';
			return true;
		}

		return false;
	}

	bool GpArcResourceTypeTag::Decode(ResTypeID &outTag)
	{
		char decodedChars[4];
		size_t parseOffset = 0;
		for (int i = 0; i < 4; i++)
		{
			char parseChar = m_id[parseOffset];
			if (parseChar == '\0')
				return false;

			if (parseChar == '$')
			{
				char nibbles[2] = { m_id[parseOffset + 1], m_id[parseOffset + 2] };
				parseOffset += 2;

				int decodedNibbles[2];

				for (int n = 0; n < 2; n++)
				{
					char nibble = nibbles[n];
					if (nibble >= '0' && nibble <= '9')
						decodedNibbles[n] = nibble - '0';
					else if (nibble >= 'a' && nibble <= 'f')
						decodedNibbles[n] = nibble - 'a' + 0xa;
					else
						return false;
				}

				decodedChars[i] = (decodedNibbles[0] << 4) | (decodedNibbles[1]);
			}
			else
				decodedChars[i] = parseChar;

			parseOffset++;
		}

		outTag = ResTypeID(decodedChars);
		return true;
	}
}
