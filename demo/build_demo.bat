call ..\..\Core\build.bat

cd demo.vs

SET configuration="Release"
IF "%~1" == "Debug" SET configuration="Debug"
SET target="x64"
IF "%~2" == "x86" SET target="x86"
msbuild demo.vs.sln /p:Configuration=%configuration% /p:Platform=%target%

cd ..

if %errorlevel% neq 0 (
    echo Build failed with error code %errorlevel%.
    exit /b %errorlevel%
)

echo Build succeeded.
