@echo off

set CurrDir=%~dp0
set CopyDir=%1

set BuildToolsDir=D:\git\gildor2\BuildTools\bin

set PATH=%PATH%;%BuildToolsDir%


rem build.
%BuildToolsDir%\bash.exe build.sh --64


rem copy and auto run.
if defined CopyDir (
	echo Input CopyDir: %CopyDir%

	copy /y %CurrDir%\umodel.exe %CopyDir%\umodel.exe
	copy /y %CurrDir%\SDL2.dll %CopyDir%\SDL2.dll

	cd %CopyDir%
	start umodel.exe
	cd %CurrDir%
)
