set ueLocation=%~1
set projectLocation=%~2
set projectName=%~3
set target=%~4
set packageFolder=%~5

"%ueLocation%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%projectLocation%\%projectName%" -noP4 -utf8output -nocompileeditor -skipbuildeditor -project="%projectLocation%\%projectName%" -target=%target% -platform=Win64 -archivedirectory="%projectLocation%\%packageFolder%" -skipstage -package -skipcook -pak -iostore -compressed -clientconfig=Development -nocompile -nocompileuat
