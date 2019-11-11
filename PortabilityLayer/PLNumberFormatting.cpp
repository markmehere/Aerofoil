#include "PLNumberFormatting.h"
#include "PLPasStr.h"

void StringToNum(const PLPasStr &str, long *num)
{
	if (str.Length() == 0)
	{
		num = 0;
		return;
	}

	const size_t len = str.Length();
	const char *chars = str.Chars();
	const char *charsEnd = chars + len;

	long result = 0;

	if (chars[0] == '-')
	{
		chars++;

		while (chars != charsEnd)
		{
			const char c = *chars++;

			if (c < '0' || c > '9')
			{
				num = 0;
				return;
			}

			result = result * 10 - (c - '0');
		}
	}
	else
	{
		while (chars != charsEnd)
		{
			const char c = *chars++;

			if (c < '0' || c > '9')
			{
				num = 0;
				return;
			}

			result = result * 10 + (c - '0');
		}
	}

	*num = result;
}
