@echo off
setlocal

rem Always operate from the directory containing this script.
pushd "%~dp0"

echo.
echo Configuring Release build...
cmake --preset windows-x64
if errorlevel 1 goto :failed

echo.
echo Rebuilding Release game...
cmake --build --preset windows-x64-release --clean-first
if errorlevel 1 goto :failed

set "BUILD_ROOT=%CD%\out\build\windows-x64"
set "BINARY_DIR=%BUILD_ROOT%\Release"
set "ASSET_DIR=%BUILD_ROOT%\assets"
set "PACKAGE_DIR=%CD%\out\package\windows-x64\Release"

echo.
echo Creating standalone Release package...

cmake -E rm -rf "%PACKAGE_DIR%"
if errorlevel 1 goto :failed

cmake -E make_directory "%PACKAGE_DIR%"
if errorlevel 1 goto :failed

rem Copy the executable and required runtime DLLs.
cmake -E copy_directory "%BINARY_DIR%" "%PACKAGE_DIR%"
if errorlevel 1 goto :failed

rem Copy the combined Game and Engine assets.
if exist "%ASSET_DIR%\" (
    cmake -E copy_directory "%ASSET_DIR%" "%PACKAGE_DIR%\assets"
    if errorlevel 1 goto :failed
)

echo.
echo Release build completed successfully.
echo Standalone distributable folder:
echo %PACKAGE_DIR%
echo.

popd
pause
exit /b 0

:failed
echo.
echo Release rebuild failed.
echo Review the errors shown above.
echo.
popd
pause
exit /b 1