@setlocal enableextensions
@cd /d "%~dp0"

call remove_symlinks.bat

mklink /D app\jni\SDL2 ..\..\..\SDL2-2.0.12

pause
