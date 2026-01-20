@echo off
SET BIN_NAME=cardiac.exe
SET INSTALL_DIR=%ProgramFiles%\Cardiac
SET BIN_PATH=%INSTALL_DIR%\%BIN_NAME%

echo Installing/updating cardiac...

IF NOT EXIST "%INSTALL_DIR%" (
    mkdir "%INSTALL_DIR%"
)

echo Downloading latest release...
powershell -Command "Invoke-WebRequest -Uri https://github.com/CoryPearl/cardiac/releases/latest/download/cardiac.exe -OutFile '%BIN_PATH%'"

echo Adding to PATH...
SETX PATH "%INSTALL_DIR%;%PATH%"

echo cardiac installed/updated successfully!
echo Please open a new terminal and run: cardiac --help
pause
