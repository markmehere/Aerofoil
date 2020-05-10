Aerofoil is a project to port Glider PRO to other systems.

It's divided into a few parts:
- PortabilityLayer: A static library that supports interaction with various vintage Mac-specific formats and API calls.  Some parts are capable of running by themselves, others require the support of drivers.
- Aerofoil:  This is the base application.  It loads drivers and loads the application.
- GpDisplayDriver_D3D11: Direct3D 11 display driver.
- GpAudioDriver_XAudio2: XAudio2 audio driver.
- GpInputDriver_XInput: XInput gamepad driver.
- GpApp: Glider PRO application DLL.


Other parts:
- CompileShadersD3D11: Compiles D3D11 shaders into C++ files containing the compiled shader data.
- ConvertColorCursors: Extracts color cursors and icons from the application resources and dumps them to .CUR and .ICO files.  Required because Windows has no API for creating color cursors at runtime.
- GliderProData: Glider PRO resource sources.
- ApplicationResourcePatches: Modified resources (used to change config dialogs).
- hqx2bin: Converts BinHex to MacBinary (not really used any more)
- hqx2gp: Converts BinHex to .gpr, .gpd, and .gpf (resource fork data, data fork data, and Finder data, respectively)
- gpr2gpa: Converts Macintosh resource files to a .gpa (a ZIP archive containing the resources in more common formats)
- FTagData: Copies a data-only file to a .gpd and creates a .gpf for it
- PictChecker: Experimental app that extracts all of the PICT resources from all of the houses and dumps them to PNG.  Used to verify that the PICT loader works.
- MacRomanConverter: Micro-library that converts the Mac Roman character set to Unicode.
- unpacktool: A utility for extracting vintage StuffIt and Compact Pro archives.