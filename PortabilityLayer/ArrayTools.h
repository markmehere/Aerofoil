#pragma once

#include <stdint.h>

namespace PortabilityLayer
{
	class ArrayTools
	{
	public:
		template<class T, class TSize, class TIndex>
		static void RemoveFromArray(T *arr, TSize &count, TIndex index)
		{
			TSize countCopy = count;
			countCopy--;
			if (countCopy != index)
				arr[index] = arr[countCopy];

			count = static_cast<TSize>(countCopy);
		}
	};
}
