:: reference from https://forums.unrealengine.com/t/build-lighting-from-command-line/297535
:: Not working using RunUAT.bat
:: RunUAT.bat RebuildLightmaps -project="E:\unreal\coop-game-fleep\CoopGameFleep.uproject" -noP4 -platform=Win64 -clientconfig=Development
:: Running AutomationTool...
:: Using bundled DotNet SDK version: 6.0.302
:: Starting AutomationTool...
:: Parsing command line: RebuildLightmaps -project=E:\unreal\coop-game-fleep\CoopGameFleep.uproject -noP4 -platform=Win64 -clientconfig=Development
:: Initializing script modules...
:: Total script module initialization time: 0.26 s.
:: Command RebuildLightMaps requires P4 functionality.
:: ERROR: System.ComponentModel.Win32Exception (2): An error occurred trying to start process 'p4.exe' with working directory 'E:\unreal\UE_5.2'. O sistema não conseguiu localizar o ficheiro especificado.
::           at System.Diagnostics.Process.StartWithCreateProcess(ProcessStartInfo startInfo)
::           at System.Diagnostics.Process.Start()
::           at AutomationTool.CommandUtils.Run(String App, String CommandLine, String Input, ERunOptions Options, Dictionary`2 Env, SpewFilterCallbackType SpewFilterCallback, String Identifier, String WorkingDir) in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\ProcessUtils.cs:line 915
::           at AutomationTool.P4Environment.DetectP4Port() in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\P4Environment.cs:line 269
::           at AutomationTool.P4Environment..ctor(CommandEnvironment CmdEnv) in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\P4Environment.cs:line 124
::           at AutomationTool.CommandUtils.InitP4Environment() in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\P4Utils.cs:line 990
::           at AutomationTool.Automation.ProcessAsync(ParsedCommandLine AutomationToolCommandLine, StartupTraceListener StartupListener, HashSet`1 ScriptModuleAssemblies) in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\Automation.cs:line 149
::        (see C:\Users\filipe\AppData\Roaming\Unreal Engine\AutomationTool\Logs\E+unreal+UE_5.2\Log.txt for full exception trace)
:: ERROR: Exception performing nothrow action "Kill All Processes": System.InvalidOperationException: No process is associated with this object.
::           at System.Diagnostics.Process.EnsureState(State state)
::           at System.Diagnostics.Process.get_HasExited()
::           at AutomationTool.ProcessResult.get_HasExited() in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\ProcessUtils.cs:line 419
::           at AutomationTool.ProcessManager.KillAll() in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\ProcessUtils.cs:line 122
::           at AutomationTool.Automation.<>c.<ProcessAsync>b__3_1() in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\Automation.cs:line 212
::           at AutomationTool.Automation.NoThrow(Action Action, String ActionDesc) in D:\build\++UE5\Sync\Engine\Saved\CsTools\Engine\Source\Programs\AutomationTool\AutomationUtils\Automation.cs:line 227
set ueLocation=%~1
set projectLocation=%~2
set projectName=%~3
set testSuiteToRun=%~4
set testReportFolder=%~5
set testLogName=%~6
set UnrealEditorCmd=%~7
set LightQuality=%~8
:: set MapToRebuild=%~9


"%ueLocation%\Engine\Binaries\Win64\%UnrealEditorCmd%" "%projectLocation%\%projectName%" -run=resavepackages -buildlighting -allowcommandletrendering -quality=%LightQuality%

:: "%ueLocation%\Engine\Binaries\Win64\%UnrealEditorCmd%" "%projectLocation%\%projectName%" -run=resavepackages -buildlighting -allowcommandletrendering -quality=%LightQuality% -map=%MapToRebuild%