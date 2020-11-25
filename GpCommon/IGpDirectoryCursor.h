#pragma once

struct IGpDirectoryCursor
{
public:
	virtual bool GetNext(const char *&outFileName) = 0;
	virtual void Destroy() = 0;
};
