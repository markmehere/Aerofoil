mkdir Packaged
mkdir Packaged\Houses
mkdir Packaged\WinCursors
mkdir ResourceTemp

x64\Release\MiniRez.exe "GliderProData\Glider PRO.r" Packaged\ApplicationResources.gpr

x64\Release\gpr2gpa.exe "Packaged\ApplicationResources.gpr" "DefaultTimestamp.timestamp" "Packaged\ApplicationResources.gpa"

x64\Release\ConvertColorCursors.exe

attrib -R Packaged\Houses\*

x64\Release\hqx2gp.exe "GliderProData\Houses\Art Museum.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Art Museum"
x64\Release\hqx2gp.exe "GliderProData\Houses\California or Bust!.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\California or Bust!"
x64\Release\hqx2gp.exe "GliderProData\Houses\Castle o' the Air.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Castle o' the Air"
x64\Release\hqx2gp.exe "GliderProData\Houses\CD Demo House.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\CD Demo House"
x64\Release\hqx2gp.exe "GliderProData\Houses\Davis Station.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Davis Station"
x64\Release\hqx2gp.exe "GliderProData\Houses\Demo House.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Demo House"
x64\Release\hqx2gp.exe "GliderProData\Houses\Empty House.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Empty House"
x64\Release\hqx2gp.exe "GliderProData\Houses\Fun House.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Fun House"
x64\Release\hqx2gp.exe "GliderProData\Houses\Grand Prix.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Grand Prix"
x64\Release\hqx2gp.exe "GliderProData\Houses\ImagineHouse PRO II.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\ImagineHouse PRO II"
x64\Release\hqx2gp.exe "GliderProData\Houses\In The Mirror.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\In The Mirror"
x64\Release\hqx2gp.exe "GliderProData\Houses\Land of Illusion.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Land of Illusion"
x64\Release\hqx2gp.exe "GliderProData\Houses\Leviathan.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Leviathan"
x64\Release\hqx2gp.exe "GliderProData\Houses\Metropolis.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Metropolis"
x64\Release\hqx2gp.exe "GliderProData\Houses\Nemo's Market.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Nemo's Market"
x64\Release\hqx2gp.exe "GliderProData\Houses\Rainbow's End.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Rainbow's End"
x64\Release\hqx2gp.exe "GliderProData\Houses\Sampler.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Sampler"
x64\Release\hqx2gp.exe "GliderProData\Houses\Slumberland.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Slumberland"
x64\Release\hqx2gp.exe "GliderProData\Houses\SpacePods.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\SpacePods"
x64\Release\hqx2gp.exe "GliderProData\Houses\Teddy World.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Teddy World"
x64\Release\hqx2gp.exe "GliderProData\Houses\The Asylum Pro.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\The Asylum Pro"
x64\Release\hqx2gp.exe "GliderProData\Houses\Titanic.binhex" "DefaultTimestamp.timestamp" "Packaged\Houses\Titanic"

x64\Release\gpr2gpa.exe "Packaged\Houses\Art Museum.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Art Museum.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\California or Bust!.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\California or Bust!.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Castle o' the Air.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Castle o' the Air.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\CD Demo House.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\CD Demo House.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Davis Station.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Davis Station.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Demo House.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Demo House.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Empty House.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Empty House.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Fun House.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Fun House.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Grand Prix.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Grand Prix.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\ImagineHouse PRO II.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\ImagineHouse PRO II.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\In The Mirror.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\In The Mirror.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Land of Illusion.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Land of Illusion.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Leviathan.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Leviathan.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Metropolis.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Metropolis.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Nemo's Market.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Nemo's Market.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Rainbow's End.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Rainbow's End.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Sampler.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Sampler.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Slumberland.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Slumberland.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\SpacePods.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\SpacePods.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Teddy World.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Teddy World.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\The Asylum Pro.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\The Asylum Pro.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Titanic.gpr" "DefaultTimestamp.timestamp" "Packaged\Houses\Titanic.gpa"

x64\Release\FTagData.exe "GliderProData\Houses\Art Museum.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Art Museum.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Castle o' the Air.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Castle o' the Air.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\CD Demo House.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\CD Demo House.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Davis Station.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Davis Station.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Demo House.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Demo House.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Grand Prix.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Grand Prix.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\ImagineHouse PRO II.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\ImagineHouse PRO II.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Land of Illusion.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Land of Illusion.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Leviathan.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Leviathan.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Nemo's Market.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Nemo's Market.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Rainbow's End.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Rainbow's End.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Slumberland.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Slumberland.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\SpacePods.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\SpacePods.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Teddy World.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Teddy World.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Titanic.mov" "DefaultTimestamp.timestamp" "Packaged\Houses\Titanic.mov" MooV ozm5 0 0 locked

del /Q Packaged\Houses\*.gpr
del /Q Packaged\ApplicationResources.gpr

attrib +R Packaged\Houses\*

pause
