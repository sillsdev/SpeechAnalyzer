@echo off
set path=%path%;c:\program files\sil\fontutils

echo Renaming the font...
call ttfname -q -f "Musique" -n "Musique" "Musique Unicode.ttf" Musique-Base.ttf
call ttfname -q -n "Copyright (c) 2000-2007 SIL International" -t 0 Musique-Base.ttf Musique00.ttf
call ttfname -q -n "Musique" -t 1 Musique00.ttf Musique01.ttf
call ttfname -q -n "Regular" -t 2 Musique01.ttf Musique02.ttf
call ttfname -q -n "Musique Regular" -t 3 Musique02.ttf Musique03.ttf
call ttfname -q -n "Musique" -t 4 Musique03.ttf Musique04.ttf
call ttfname -q -n "Version 1.1, %DATE%" -t 5 Musique04.ttf Musique05.ttf
call ttfname -q -n "Musique" -t 6 Musique05.ttf Musique06.ttf
call ttfname -q -n "Musique" -t 16 Musique06.ttf Musique16.ttf
call ttfname -q -n "Regular" -t 17 Musique16.ttf Musique17.ttf
call ttfname -q -n "Musique" -t 18 Musique17.ttf Musique18.ttf
call ttfname -q -n "Musique" -t 20 Musique18.ttf "Musique-Renamed.ttf"
echo.

echo Generating the documentation
call fret Musique-Renamed.ttf
echo.

echo.
echo The next step will clean up the intermediate files and remap to Unicode.

pause

del Musique-Base.ttf
del Musique0*.ttf
del Musique1*.ttf

call ttfbuilder -c silmus_uni.xml Musique-Renamed.ttf MusiqueU.ttf

echo.
echo Generating the documentation
call fret MusiqueU.ttf

echo.
echo Conversion complete!
pause
