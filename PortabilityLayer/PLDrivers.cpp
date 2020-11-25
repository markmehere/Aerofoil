#include "PLDrivers.h"

GpDriverCollection *PLDrivers::GetDriverCollection()
{
	return &ms_drivers;
}

IGpAudioDriver *PLDrivers::GetAudioDriver()
{
	return ms_drivers.GetDriver<GpDriverIDs::kAudio>();
}

IGpFileSystem *PLDrivers::GetFileSystem()
{
	return ms_drivers.GetDriver<GpDriverIDs::kFileSystem>();
}

IGpDisplayDriver *PLDrivers::GetDisplayDriver()
{
	return ms_drivers.GetDriver<GpDriverIDs::kDisplay>();
}

IGpLogDriver *PLDrivers::GetLogDriver()
{
	return ms_drivers.GetDriver<GpDriverIDs::kLog>();
}

size_t PLDrivers::GetNumInputDrivers()
{
	return ms_drivers.GetDriverCount<GpDriverIDs::kInput>();
}

IGpInputDriver *PLDrivers::GetInputDriver(size_t index)
{
	return ms_drivers.GetDriver<GpDriverIDs::kInput>(index);
}

IGpSystemServices *PLDrivers::GetSystemServices()
{
	return ms_drivers.GetDriver<GpDriverIDs::kSystemServices>();
}

IGpFontHandler *PLDrivers::GetFontHandler()
{
	return ms_drivers.GetDriver<GpDriverIDs::kFont>();
}

IGpVOSEventQueue *PLDrivers::GetVOSEventQueue()
{
	return ms_drivers.GetDriver<GpDriverIDs::kEventQueue>();
}

GpDriverCollection PLDrivers::ms_drivers;
