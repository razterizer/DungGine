@echo off

REM Set up the Visual Studio environment
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" || 
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" || 
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"

echo Building on Windows...
echo Building with VC++...

REM Compile the demo using VC++
cl /std:c++20 /EHsc /Fe:bin/demo.exe /Fo:bin/demo.obj .\demo.cpp /I../..

REM Create necessary directories and copy resources
if not exist bin\fonts mkdir bin\fonts
xcopy /Y /E ..\..\..\lib\Termin8or\fonts bin\fonts

if not exist bin\textures mkdir bin\textures
xcopy /Y /E textures bin\textures
