rmdir /S /Q app\src\main\assets\Packaged\ApplicationResources
..\Tools\7z.exe x -oapp\src\main\assets\Packaged\ApplicationResources ..\Packaged\ApplicationResources.gpa
cd app
cd src
cd main
cd assets
cd Packaged
rmdir /S /Q Houses
mkdir Houses
copy ..\..\..\..\..\..\Packaged\Houses\* Houses\
cd Houses
..\..\..\..\..\..\..\Tools\7z.exe x "-oArt Museum" "Art Museum.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oArt Museum.mov" "Art Museum.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oCalifornia or Bust!" "California or Bust!.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oCastle o' the Air" "Castle o' the Air.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oCastle o' the Air.mov" "Castle o' the Air.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oCD Demo House" "CD Demo House.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oCD Demo House.mov" "CD Demo House.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oDavis Station" "Davis Station.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oDavis Station.mov" "Davis Station.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oDemo House" "Demo House.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oDemo House.mov" "Demo House.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oEmpty House" "Empty House.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oFun House" "Fun House.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oGrand Prix" "Grand Prix.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oGrand Prix.mov" "Grand Prix.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oImagineHouse PRO II" "ImagineHouse PRO II.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oImagineHouse PRO II.mov" "ImagineHouse PRO II.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oIn The Mirror" "In The Mirror.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oLand of Illusion" "Land of Illusion.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oLand of Illusion.mov" "Land of Illusion.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oLeviathan" "Leviathan.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oLeviathan.mov" "Leviathan.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oMetropolis" "Metropolis.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oNemo's Market" "Nemo's Market.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oNemo's Market.mov" "Nemo's Market.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oRainbow's End" "Rainbow's End.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oRainbow's End.mov" "Rainbow's End.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oSampler" "Sampler.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oSlumberland" "Slumberland.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oSlumberland.mov" "Slumberland.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oSpacePods" "SpacePods.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oSpacePods.mov" "SpacePods.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oTeddy World" "Teddy World.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oTeddy World.mov" "Teddy World.mov.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oThe Asylum Pro" "The Asylum Pro.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oTitanic" "Titanic.gpa"
..\..\..\..\..\..\..\Tools\7z.exe x "-oTitanic.mov" "Titanic.mov.gpa"
del /Q *.gpa
cd ..
