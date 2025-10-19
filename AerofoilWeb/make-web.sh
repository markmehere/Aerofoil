# do not proceed with errors
set -e

# make directories
echo "Making directories..."
mkdir -p obj
mkdir -p bin
mkdir -p res
mkdir -p utils

# compile source objects
echo "Linking AerofoilSDL..."
emcc -c ../AerofoilSDL/AerofoilSDL_Combined.cpp -o obj/AerofoilSDL_Combined.o -s USE_SDL=2 -I../GpCommon/ -I../PortabilityLayer/ -I../Common/ -O3 -DNDEBUG=1 -DGP_DEBUG_CONFIG=0
echo "Linking AerofoilWeb_Combined..."
emcc -c ./AerofoilWeb_Combined.cpp -o obj/AerofoilWeb_Combined.o -s USE_SDL=2 -I../GpCommon/ -I../PortabilityLayer/ -I../Common/ -I../AerofoilPortable/ -I../GpShell/ -O3 -DNDEBUG=1 -DGP_DEBUG_CONFIG=0
echo "Linking GpApp..."
emcc -c ../GpApp/GpApp_Combined.cpp -o obj/GpApp_Combined.o -s USE_SDL=2 -I../GpCommon/ -I../PortabilityLayer/ -I../Common/ -O3 -DNDEBUG=1 -DGP_DEBUG_CONFIG=0
echo "Linking GpShell..."
emcc -c ../GpShell/GpShell_Combined.cpp -o obj/GpShell_Combined.o -s USE_SDL=2 -I../GpCommon/ -I../PortabilityLayer/ -I../Common/ -O3 -DNDEBUG=1 -DGP_DEBUG_CONFIG=0
echo "Linking MacRomanConversion..."
emcc -c ../MacRomanConversion/MacRomanConversion.cpp -o obj/MacRomanConversion.o -I../MacRomanConversion/ -O3 -DNDEBUG=1
echo "Linking PortabilityLayer..."
emcc -c ../PortabilityLayer/PortabilityLayer_Combined.cpp -o obj/PortabilityLayer_Combined.o -s USE_ZLIB=1 -I../GpCommon/ -I../Common/ -I../PortabilityLayer/ -I../rapidjson/include/ -I../MacRomanConversion/ -I../stb/ -O3 -DNDEBUG=1 -DGP_DEBUG_CONFIG=0 -Wno-tautological-constant-out-of-range-compare
echo "Linking AerofoilPortable..."
emcc -c ../AerofoilPortable/GpAllocator_C.cpp -o obj/AerofoilPortable_Combined.o -s USE_SDL=2 -I../GpCommon/ -I../Common/ -O3 -DNDEBUG=1 -DGP_DEBUG_CONFIG=0

# create bin2h utility (POSIX version)
echo "Creating bin2h..."
g++ ../bin2h/bin2h-posix.cpp -o utils/bin2h -O3 -DNDEBUG=1

# run bin2h on Packaged (must have the relevant files in the Android build)
echo "Running bin2h to build ApplicationData.h..."
mkdir -p Packaged
cp ../AerofoilAndroid/app/src/main/assets/Packaged/ApplicationResources.gpf Packaged/
cp ../AerofoilAndroid/app/src/main/assets/Packaged/Fonts.gpf Packaged/
./utils/bin2h ./Packaged res/ApplicationData.h
rm Packaged/ApplicationResources.gpf
rm Packaged/Fonts.gpf
echo "Running bin2h to build GameData.h..."
cp -r ../AerofoilAndroid/app/src/main/assets/Packaged/Houses Packaged/
./utils/bin2h ./Packaged/Houses res/GameData.h
rm -rf Packaged/Houses
rm -r Packaged

# compiling those files to resources object
echo "Building AerofoilWeb_Resources..."
emcc -c ./AerofoilWeb_Resources.cpp -o obj/AerofoilWeb_Resources.o -O3 -DGP_DEBUG_CONFIG=0

# downloading FileSaver.js if required
echo "Downloading FileSaver.js..."
if [ ! -f FileSaverDotJS/dist/FileSaver.js ]; then
    rm -rf FileSaverDotJS
    rm -rf ./FileSaverDotJS/.git
    git clone https://github.com/eligrey/FileSaver.js.git FileSaverDotJS
fi
cp FileSaverDotJS/dist/FileSaver.js bin/FileSaver.js

# preparing final output
echo "Preparing final output..."
emcc obj/AerofoilWeb_Combined.o obj/AerofoilWeb_Resources.o obj/GpShell_Combined.o obj/AerofoilSDL_Combined.o obj/AerofoilPortable_Combined.o obj/GpApp_Combined.o obj/PortabilityLayer_Combined.o obj/MacRomanConversion.o -o bin/aerofoil.html -O3 -s USE_SDL=2 -s USE_ZLIB=1 -s ASYNCIFY -s ASYNCIFY_IGNORE_INDIRECT -s INITIAL_MEMORY=33554432 -s ASYNCIFY_ADVISE -lidbfs.js -s ASYNCIFY_IMPORTS=['InitFileSystem','FlushFileSystem'] --shell-file shell_minimal.html
