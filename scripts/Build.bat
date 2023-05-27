set ueLocation=%~1
set projectLocation=%~2

"%ueLocation%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%projectLocation%" -noP4 -platform=Win64 -clientconfig=Development -build

