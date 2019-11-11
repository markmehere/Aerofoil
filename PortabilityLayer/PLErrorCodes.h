#pragma once
#ifndef __PL_ERROR_CODES_H__
#define __PL_ERROR_CODES_H__

enum ErrorCodes
{
	noErr,

	fnfErr,
	eofErr,
	userCanceledErr,
	dirFulErr,
	dskFulErr,
	ioErr,
	bdNamErr,
	fnOpnErr,
	mFulErr,
	tmfoErr,
	wPrErr,
	fLckdErr,
	vLckdErr,
	fBsyErr,
	dupFNErr,
	opWrErr,
	volOffLinErr,
	permErr,
	wrPermErr,

	genericErr,
};

#endif
