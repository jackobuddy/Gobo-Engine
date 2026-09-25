@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass ^
  -File "%~dp0..\..\scripts\Build-Game.ps1" ^
  -ProjectDirectory "%~dp0." -Configuration Release
set "RESULT=%ERRORLEVEL%"
pause
exit /b %RESULT%