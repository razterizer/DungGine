@echo off

set REPO_DIR=DungGine
set BUILD_DIR=demo

REM Change directory
cd ..

REM Define the required folder name
set "REQUIRED_NAME=lib"
REM Get the current folder name
for %%F in ("%CD%") do set "CURRENT_NAME=%%~nF"
REM Check if the current folder name matches the required name
if not "%CURRENT_NAME%"=="%REQUIRED_NAME%" (
    color 0C
    echo Warning: You are not in the correct folder. It is highly recommended that you check out the 8Beat repo in a folder named "%REQUIRED_NAME%". The demos will run fine but other repos might expect to find it there.
    REM pause
    REM exit /b 1
)
REM Reset color and continue with the script
color 07

REM Run the dependency fetch script
python "%REPO_DIR%\fetch-dependencies.py" "%REPO_DIR%\dependencies"

REM Navigate to the appropriate directory
cd "%REPO_DIR%\%BUILD_DIR%"

REM Run the build script
call build_demo.bat

:askUser
REM Ask the user if they want to run the program
set /p response=Do you want to run the program? (yes/no): 

REM Process the response
if /i "%response%"=="yes" (
    echo Running the program...
    call run_demo.bat
    goto end
) else if /i "%response%"=="no" (
    echo Alright. Have a nice day!
    goto end
) else (
    echo Invalid response. Please answer yes or no.
    goto askUser
)

:end
