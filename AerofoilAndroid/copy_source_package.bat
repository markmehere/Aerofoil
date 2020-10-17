cd ..
del AerofoilAndroid\app\src\main\assets\SourceCode.zip
del AerofoilAndroid\app\src\main\assets\SourceCode.pkg
git archive -0 --format zip -o AerofoilAndroid\app\src\main\assets\SourceCode.zip HEAD
tools\7z.exe d AerofoilAndroid\app\src\main\assets\SourceCode.zip GliderProData\
cd AerofoilAndroid\app\src\main\assets
rename SourceCode.zip SourceCode.pkg
