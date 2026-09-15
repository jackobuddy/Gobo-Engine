1. Install Visual Studio 2022 with Desktop development with C++ and C++ CMake tools for Windows.
2. Open PowerShell inside the cloned repository folder.
3. Initialize the included dependencies:
	git submodule update --init --recursive
	powershell -ExecutionPolicy Bypass -File scripts/bootstrap.ps1
4. Open Visual Studio.
5. Select Open a local folder and choose the repository folder containing CMakeLists.txt.
6. Wait for CMake configuration to finish.
7. Select GoboEngine.exe from the startup-item dropdown.
8. Select the Windows x64 Debug configuration.
9. Press F5 to build and run the engine.