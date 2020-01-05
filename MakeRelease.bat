mkdir ReleasePkg\Aerofoil
mkdir ReleasePkg\Aerofoil\Packaged
mkdir ReleasePkg\Aerofoil\Resources

copy /Y x64\Release\Aerofoil.exe ReleasePkg\Aerofoil
copy /Y x64\Release\GpAudioDriver_XAudio2.dll ReleasePkg\Aerofoil
copy /Y x64\Release\GpDisplayDriver_D3D11.dll ReleasePkg\Aerofoil
copy /Y x64\Release\GpInputDriver_XInput.dll ReleasePkg\Aerofoil
copy /Y x64\Release\FreeType.dll ReleasePkg\Aerofoil
copy /Y x64\Release\GpApp.dll ReleasePkg\Aerofoil


xcopy /I /E /Y Packaged ReleasePkg\Aerofoil\Packaged
xcopy /I /E /Y Resources ReleasePkg\Aerofoil\Resources
pause
