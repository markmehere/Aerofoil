mkdir ReleasePkg\Aerofoil
mkdir ReleasePkg\Aerofoil\Packaged
mkdir ReleasePkg\Aerofoil\Resources
mkdir ReleasePkg\Aerofoil\Tools

copy /Y x64\Release\Aerofoil.exe ReleasePkg\Aerofoil
copy /Y x64\Release\GpAudioDriver_XAudio2.dll ReleasePkg\Aerofoil
copy /Y x64\Release\GpDisplayDriver_D3D11.dll ReleasePkg\Aerofoil
copy /Y x64\Release\GpInputDriver_XInput.dll ReleasePkg\Aerofoil
copy /Y x64\Release\FreeType.dll ReleasePkg\Aerofoil
copy /Y x64\Release\GpApp.dll ReleasePkg\Aerofoil

copy /Y x64\Release\flattenmov.exe ReleasePkg\Aerofoil\Tools
copy /Y x64\Release\bin2gp.exe ReleasePkg\Aerofoil\Tools
copy /Y x64\Release\hqx2bin.exe ReleasePkg\Aerofoil\Tools
copy /Y x64\Release\hqx2gp.exe ReleasePkg\Aerofoil\Tools
copy /Y x64\Release\MakeTimestamp.exe ReleasePkg\Aerofoil\Tools
copy /Y x64\Release\FTagData.exe ReleasePkg\Aerofoil\Tools
copy /Y x64\Release\gpr2gpa.exe ReleasePkg\Aerofoil\Tools
copy /Y x64\Release\unpacktool.exe ReleasePkg\Aerofoil\Tools

xcopy /I /E /Y /K Packaged ReleasePkg\Aerofoil\Packaged
xcopy /I /E /Y /K Resources ReleasePkg\Aerofoil\Resources
xcopy /I /E /Y /K Documentation ReleasePkg\Aerofoil\Documentation

pause
