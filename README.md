# coop-game-fleep

[![build workflow status](https://github.com/filfreire/coop-game-fleep/actions/workflows/build.yml/badge.svg)](https://github.com/filfreire/coop-game-fleep/actions/workflows/build.yml)

This is my personal repo with exercises, experiments and classwork of the <https://www.udemy.com/course/unrealengine-cpp> class.
It was forked and is related to [@tomlooman](https://github.com/tomlooman)'s original work on: <https://github.com/tomlooman/CoopHordeShooter>

> Note repository is in a WIP state.

- [coop-game-fleep](#coop-game-fleep)
  - [Prerequisites](#prerequisites)
  - [How to build](#how-to-build)
  - [How to run tests](#how-to-run-tests)
  - [How to package and run](#how-to-package-and-run)

## Prerequisites

For Windows 10/11:

- Install [Unreal Engine 5.2](https://www.unrealengine.com/en-US/download) (and all needed sub-dependencies)
- Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/)
- Install all dependencies mentioned on [official documentation](https://docs.unrealengine.com/5.2/en-US/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine/)

(Tested on a Windows 10 Pro, version 22H2)

> For Linux/MacOS: `Not used yet, work in progress`

## How to build

First, clone the repository, make sure you have everything listed on **Prerequisites** setup and then `cd` into the cloned folder.

Use `.\scripts\Build.bat` batch file to compile/build the project:

```powershell
# UNREAL_PATH - Unreal engine install path, e.g. C:\Epic Games\UE_5.2
# PROJECT_NAME - project name, e.g. CoopGameFleep.uproject

.\scripts\Build.bat $env:UNREAL_PATH (Get-Location).Path $env:PROJECT_NAME
```

## How to run tests

To run tests, use the `.\scripts\RunTests.bat` batch file:

```powershell
# UNREAL_PATH - Unreal engine install path, e.g. C:\Epic Games\UE_5.2
# PROJECT_NAME - project name, e.g. CoopGameFleep.uproject
# TEST_SUITE_TO_RUN - e.g. CoopGameFleepTests.
# TEST_REPORT_FOLDER - e.g. TestResults
# TEST_LOGNAME - e.g. RunTests.log

.\scripts\RunTests.bat $env:UNREAL_PATH (Get-Location).Path $env:PROJECT_NAME $env:TEST_SUITE_TO_RUN $env:TEST_REPORT_FOLDER $env:TEST_LOGNAME
```

## How to package and run

To package a game build for Win64 platform, r   un `.\scripts\Package.bat` on a Powershell terminal:

```powershell
# UNREAL_PATH - Unreal engine install path, e.g. C:\Epic Games\UE_5.2
# PROJECT_NAME - project name, e.g. CoopGameFleep.uproject
# TARGET_NAME - name of target, e.g. CoopGameFleep
# PACKAGE_FOLDER - Folder name where to place the packaged game binaries, e.g. PackageResults

.\scripts\Package.bat $env:UNREAL_PATH (Get-Location).Path $env:PROJECT_NAME $env:TARGET_NAME $env:PACKAGE_FOLDER
```

If packaging works successfully, you should see a log like:

```plaintext
UATHelper: Packaging (Windows): ********** ARCHIVE COMMAND COMPLETED **********
UATHelper: Packaging (Windows): BuildCookRun time: 636.56 s
UATHelper: Packaging (Windows): BUILD SUCCESSFUL
UATHelper: Packaging (Windows): AutomationTool executed for 0h 10m 37s
UATHelper: Packaging (Windows): AutomationTool exiting with ExitCode=0 (Success)
```

And you should see a packaged build in `PACKAGE_FOLDER` (or the `-archivedirectory` you've picked in case you edit the `.\scripts\Package.bat` batch file).

> Note: First packaging is can take between 10 to 20 minutes. This is what it took on an AMD Ryzen 7 5800 processor and an Intel Core i7 6700k. It might take longer or less time depending on your specs.
