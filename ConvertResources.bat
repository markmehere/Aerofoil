mkdir Packaged
mkdir Packaged\Houses
mkdir Packaged\WinCursors
mkdir ResourceTemp

x64\Release\MiniRez.exe "GliderProData\Glider PRO.r" Packaged\ApplicationResources.gpr

x64\Release\gpr2gpa.exe Packaged\ApplicationResources.gpr Packaged\ApplicationResources.gpa

x64\Release\ConvertColorCursors.exe

x64\Release\hqx2gp.exe "GliderProData\Houses\Art Museum.binhex" "Packaged\Houses\Art Museum"
x64\Release\hqx2gp.exe "GliderProData\Houses\California or Bust!.binhex" "Packaged\Houses\California or Bust!"
x64\Release\hqx2gp.exe "GliderProData\Houses\Castle o' the Air.binhex" "Packaged\Houses\Castle o' the Air"
x64\Release\hqx2gp.exe "GliderProData\Houses\CD Demo House.binhex" "Packaged\Houses\CD Demo House"
x64\Release\hqx2gp.exe "GliderProData\Houses\Davis Station.binhex" "Packaged\Houses\Davis Station"
x64\Release\hqx2gp.exe "GliderProData\Houses\Demo House.binhex" "Packaged\Houses\Demo House"
x64\Release\hqx2gp.exe "GliderProData\Houses\Empty House.binhex" "Packaged\Houses\Empty House"
x64\Release\hqx2gp.exe "GliderProData\Houses\Fun House.binhex" "Packaged\Houses\Fun House"
x64\Release\hqx2gp.exe "GliderProData\Houses\Grand Prix.binhex" "Packaged\Houses\Grand Prix"
x64\Release\hqx2gp.exe "GliderProData\Houses\ImagineHouse PRO II.binhex" "Packaged\Houses\ImagineHouse PRO II"
x64\Release\hqx2gp.exe "GliderProData\Houses\In The Mirror.binhex" "Packaged\Houses\In The Mirror"
x64\Release\hqx2gp.exe "GliderProData\Houses\Land of Illusion.binhex" "Packaged\Houses\Land of Illusion"
x64\Release\hqx2gp.exe "GliderProData\Houses\Leviathan.binhex" "Packaged\Houses\Leviathan"
x64\Release\hqx2gp.exe "GliderProData\Houses\Metropolis.binhex" "Packaged\Houses\Metropolis"
x64\Release\hqx2gp.exe "GliderProData\Houses\Nemo's Market.binhex" "Packaged\Houses\Nemo's Market"
x64\Release\hqx2gp.exe "GliderProData\Houses\Rainbow's End.binhex" "Packaged\Houses\Rainbow's End"
x64\Release\hqx2gp.exe "GliderProData\Houses\Sampler.binhex" "Packaged\Houses\Sampler"
x64\Release\hqx2gp.exe "GliderProData\Houses\Slumberland.binhex" "Packaged\Houses\Slumberland"
x64\Release\hqx2gp.exe "GliderProData\Houses\SpacePods.binhex" "Packaged\Houses\SpacePods"
x64\Release\hqx2gp.exe "GliderProData\Houses\Teddy World.binhex" "Packaged\Houses\Teddy World"
x64\Release\hqx2gp.exe "GliderProData\Houses\The Asylum Pro.binhex" "Packaged\Houses\The Asylum Pro"
x64\Release\hqx2gp.exe "GliderProData\Houses\Titanic.binhex" "Packaged\Houses\Titanic"

x64\Release\gpr2gpa.exe "Packaged\Houses\Art Museum.gpr" "Packaged\Houses\Art Museum.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\California or Bust!.gpr" "Packaged\Houses\California or Bust!.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Castle o' the Air.gpr" "Packaged\Houses\Castle o' the Air.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\CD Demo House.gpr" "Packaged\Houses\CD Demo House.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Davis Station.gpr" "Packaged\Houses\Davis Station.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Demo House.gpr" "Packaged\Houses\Demo House.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Empty House.gpr" "Packaged\Houses\Empty House.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Fun House.gpr" "Packaged\Houses\Fun House.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Grand Prix.gpr" "Packaged\Houses\Grand Prix.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\ImagineHouse PRO II.gpr" "Packaged\Houses\ImagineHouse PRO II.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\In The Mirror.gpr" "Packaged\Houses\In The Mirror.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Land of Illusion.gpr" "Packaged\Houses\Land of Illusion.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Leviathan.gpr" "Packaged\Houses\Leviathan.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Metropolis.gpr" "Packaged\Houses\Metropolis.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Nemo's Market.gpr" "Packaged\Houses\Nemo's Market.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Rainbow's End.gpr" "Packaged\Houses\Rainbow's End.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Sampler.gpr" "Packaged\Houses\Sampler.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Slumberland.gpr" "Packaged\Houses\Slumberland.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\SpacePods.gpr" "Packaged\Houses\SpacePods.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Teddy World.gpr" "Packaged\Houses\Teddy World.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\The Asylum Pro.gpr" "Packaged\Houses\The Asylum Pro.gpa"
x64\Release\gpr2gpa.exe "Packaged\Houses\Titanic.gpr" "Packaged\Houses\Titanic.gpa"

x64\Release\FTagData.exe "GliderProData\Houses\Art Museum.mov", "Packaged\Houses\Art Museum.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Castle o' the Air.mov", "Packaged\Houses\Castle o' the Air.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\CD Demo House.mov", "Packaged\Houses\CD Demo House.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Davis Station.mov", "Packaged\Houses\Davis Station.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Demo House.mov", "Packaged\Houses\Demo House.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Grand Prix.mov", "Packaged\Houses\Grand Prix.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\ImagineHouse PRO II.mov", "Packaged\Houses\ImagineHouse PRO II.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Land of Illusion.mov", "Packaged\Houses\Land of Illusion.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Leviathan.mov", "Packaged\Houses\Leviathan.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Nemo's Market.mov", "Packaged\Houses\Nemo's Market.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Rainbow's End.mov", "Packaged\Houses\Rainbow's End.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Slumberland.mov", "Packaged\Houses\Slumberland.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\SpacePods.mov", "Packaged\Houses\SpacePods.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Teddy World.mov", "Packaged\Houses\Teddy World.mov" MooV ozm5 0 0 locked
x64\Release\FTagData.exe "GliderProData\Houses\Titanic.mov", "Packaged\Houses\Titanic.mov" MooV ozm5 0 0 locked

del /Q Packaged\Houses\*.gpr
del /Q Packaged\ApplicationResources.gpr

pause
