call ..\..\Core\build.bat
 
REM Compile the demo using VC++
cl /std:c++20 /EHsc /Fe:bin/demo.exe /Fo:bin/demo.obj .\demo.cpp /I../..

if %errorlevel% neq 0 (
    echo Build failed with error code %errorlevel%.
    exit /b %errorlevel%
)
 
REM Create necessary directories and copy resources
if not exist bin\fonts mkdir bin\fonts
xcopy /Y /E ..\..\Termin8or\fonts bin\fonts
 
if not exist bin\textures mkdir bin\textures
xcopy /Y /E textures bin\textures
