@echo off
setlocal

rem Always operate from the directory containing this script.
pushd "%~dp0"

echo.
echo Configuring Debug build...
cmake --preset windows-x64
if errorlevel 1 goto :failed

echo.
echo Rebuilding Debug game...
cmake --build --preset windows-x64-debug --clean-first
if errorlevel 1 goto :failed

set "BUILD_ROOT=%CD%\out\build\windows-x64"
set "BINARY_DIR=%BUILD_ROOT%\Debug"
set "ASSET_DIR=%BUILD_ROOT%\assets"
set "PACKAGE_DIR=%CD%\out\package\windows-x64\Debug"

echo.
echo Creating standalone Debug package...

cmake -E rm -rf "%PACKAGE_DIR%"
if errorlevel 1 goto :failed

cmake -E make_directory "%PACKAGE_DIR%"
if errorlevel 1 goto :failed

rem Copy the executable, DLLs, and Debug symbols.
cmake -E copy_directory "%BINARY_DIR%" "%PACKAGE_DIR%"
if errorlevel 1 goto :failed

rem Copy the combined Game and Engine assets.
if exist "%ASSET_DIR%\" (
    cmake -E copy_directory "%ASSET_DIR%" "%PACKAGE_DIR%\assets"
    if errorlevel 1 goto :failed
)

echo.
echo Debug build completed successfully.
echo Standalone folder:
echo %PACKAGE_DIR%
echo.
echo Debug builds require the Visual Studio Debug runtime and are not intended
echo for distribution to computers without Visual Studio installed.
echo.

popd
pause
exit /b 0

:failed
echo.
echo Debug rebuild failed.
echo Review the errors shown above.
echo.
popd
pause
exit /b 1