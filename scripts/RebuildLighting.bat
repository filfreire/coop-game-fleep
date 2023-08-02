:: reference from https://forums.unrealengine.com/t/build-lighting-from-command-line/297535
:: Not working using RunUAT.bat
:: RunUAT.bat RebuildLightmaps -project="E:\unreal\coop-game-fleep\CoopGameFleep.uproject" -noP4 -platform=Win64 -clientconfig=Development
:: Command RebuildLightMaps requires P4 functionality.

set ueLocation=%~1
set projectLocation=%~2
set projectName=%~3
set UnrealEditorCmd=%~4
set LightQuality=%~5
set MapToRebuild=%~6


:: "%ueLocation%\Engine\Binaries\Win64\%UnrealEditorCmd%" "%projectLocation%\%projectName%" -run=resavepackages -buildlighting -allowcommandletrendering -quality=%LightQuality%

"%ueLocation%\Engine\Binaries\Win64\%UnrealEditorCmd%" "%projectLocation%\%projectName%" -run=resavepackages -buildlighting -allowcommandletrendering -quality=%LightQuality% -map=%MapToRebuild%