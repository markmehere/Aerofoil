move ReleasePkg\en-us\Aerofoil-installpkg.msi ReleasePkg\Aerofoil-installpkg.msi
Tools\7z.exe a -bd -r -mx=9 ReleasePkg\Aerofoil.zip ReleasePkg\Aerofoil
Tools\7z.exe a -bd -r -mx=9 ReleasePkg\Aerofoil-PDBs.7z ReleasePkg\PDBs
rmdir /S /Q ReleasePkg\Aerofoil
rmdir /S /Q ReleasePkg\en-us
rmdir /S /Q ReleasePkg\PDBs
