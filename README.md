# coop-game-fleep

![build workflow status](https://github.com/filfreire/coop-game-fleep/actions/workflows/build.yml/badge.svg)

This is my personal repo with exercises, experiments and classwork of the <https://www.udemy.com/course/unrealengine-cpp> class.
It was forked and is related to [@tomlooman](https://github.com/tomlooman)'s original work on: <https://github.com/tomlooman/CoopHordeShooter>

> Note repository is in a WIP state.

## Prerequisites

For Windows 10/11:

- Install [Unreal Engine 5.2](https://www.unrealengine.com/en-US/download) (and all needed sub-dependencies)
- Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/)

(Tested on a Windows 10 Pro, version 22H2)

For Linux/MacOS:

/todo - work in progress

## How to build

/todo - work in progress

## How to package and run

First, run on a Powershell terminal:

```powershell
& 'D:\EpicGames\Epic Games\UE_5.2\Engine\Build\BatchFiles\RunUAT.bat' BuildCookRun -project="D:/unreal/coop-game-fleep/CoopGameFleep.uproject" -nop4 -utf8output -nocompileeditor -skipbuildeditor -cook -project="D:/unreal/coop-game-fleep/CoopGameFleep.uproject" -target=CoopGameFleep -platform=Win64 -installed -stage -archive -package -build -pak -iostore -compressed -prereqs -archivedirectory="D:/builds" -clientconfig=Development -nocompile -nocompileuat
```

> note: replace `D:\EpicGames\Epic Games\UE_5.2` and `unreal/coop-game-fleep` with the respective paths where you installed Unreal Engine 5.2 and cloned the repository:

If packaging works successfully, you should see a log like:

```plaintext
UATHelper: Packaging (Windows): ********** ARCHIVE COMMAND COMPLETED **********
UATHelper: Packaging (Windows): BuildCookRun time: 636.56 s
UATHelper: Packaging (Windows): BUILD SUCCESSFUL
UATHelper: Packaging (Windows): AutomationTool executed for 0h 10m 37s
UATHelper: Packaging (Windows): AutomationTool exiting with ExitCode=0 (Success)
```

And you should see a packaged build in `D:/builds` (or the `-archivedirectory` you've picked).

> Note: First packaging is taking between 10 to 20 minutes on an AMD Ryzen 7 5800 processor. It might take longer or less time depending on your specs.
