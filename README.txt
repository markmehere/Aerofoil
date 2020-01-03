Aerofoil is a collection of porting infrastructure to run Glider PRO.

It's divided into a few parts:
- PortabilityLayer: A static library that supports interaction with various vintage Mac-specific formats and API calls.  Some parts are capable of running by themselves, others require the support of drivers.
- Aerofoil:  This is the base application.  It loads drivers and loads the application.
- GpDisplayDriver_D3D11: Direct3D 11 display driver.
- GpAudioDriver_XAudio2: XAudio2 audio driver.
- GpApp: Glider PRO application DLL.


Other parts:
- CompileShadersD3D11: Compiles D3D11 shaders into C++ files containing the compiled shader data.
- ConvertColorCursors: Extracts color cursors and icons from the application resources and dumps them to .CUR and .ICO files.  Required because Windows has no API for creating color cursors at runtime.
- GpApp2: To be renamed.  Contains Glider resource sources.
- hqx2bin: Converts BinHex to MacBinary
- hqx2gp: Converts BinHex to .gpr, .gpd, and .gpf (resource fork data, data fork data, and Finder data, respectively)
- FTagData: Copies a data-only file to a .gpd and creates a .gpf for it
- ImportCharSet: Imports the Unicode MacRoman description into a code page table.
- PictChecker: Experimental app that extracts all of the PICT resources from all of the houses and dumps them to PNG.  Used to verify that the PICT loader works.


Aerofoil is intended to be primarily a straight port, retaining most of the look and feel of the original.

Planned additions:
- 32-bit color support (mainly matters for houses that have higher-resolution PICT resources)
- Resolution changes while the game is running
- Bring back the gray-to-color intro fade that was removed in later versions
- Gamepad support
