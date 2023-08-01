set ueLocation=%~1
set projectLocation=%~2
set projectName=%~3

"%ueLocation%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%projectLocation%\%projectName%" -nop4 -utf8output -cook -stage
