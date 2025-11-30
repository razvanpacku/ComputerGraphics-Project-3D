# Computer Graohics - Rocket and Planets

## Team members

- [Păcurariu Răzvan Mihai](https://github.com/razvanpacku)
- [Luparu Ioan Teodor](https://github.com/BrainDBD)

## Build & run

### Prerequisites

- Windows with Visual Studio (MSVC) or MSBuild.
- GPU/drivers supporting OpenGL. (Project includes GLAD/GLFW/GLM headers in the repo.)

### Building

- Recommended: open the solution in Visual Studio
    1. Open [Project.sln](Project.sln) in Visual Studio.
    2. Select Platform = x64 (if needed) and Configuration = Debug or Release.
    3. Build -> Build Solution.
- Command line (MSBuild)
```sh
msbuild Project.sln /p:Configuration=Release /p:Platform=x64
```

### Run

- From Visual Studio: Start Debugging / Start Without Debugging
- From command line: run the built executable (example paths below — adjust for Debug/Release)
```sh
cd x64/Release
Project.exe
```