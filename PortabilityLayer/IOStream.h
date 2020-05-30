#pragma once

#ifndef __PL_IOTREAM_H__
#define __PL_IOTREAM_H__

#include "DataTypes.h"
#include "FilePos.h"

namespace PortabilityLayer
{
	class IOStream
	{
	public:
		virtual size_t Read(void *bytesOut, size_t size) = 0;
		virtual size_t Write(const void *bytes, size_t size) = 0;
		virtual bool IsSeekable() const = 0;
		virtual bool IsReadOnly() const = 0;
		virtual bool IsWriteOnly() const = 0;
		virtual bool SeekStart(UFilePos_t loc) = 0;
		virtual bool SeekCurrent(FilePos_t loc) = 0;
		virtual bool SeekEnd(UFilePos_t loc) = 0;
		virtual bool Truncate(UFilePos_t loc) = 0;
		virtual UFilePos_t Size() const = 0;
		virtual UFilePos_t Tell() const = 0;
		virtual void Close() = 0;
		virtual void Flush() = 0;
	};
}

#endif
