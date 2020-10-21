#pragma once

struct IGpThreadRelay
{
	typedef void (*Callback_t) (void *context);

	virtual void Invoke(Callback_t callback, void *context) const = 0;
};
