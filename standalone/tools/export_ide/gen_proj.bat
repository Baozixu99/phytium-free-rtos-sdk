@echo off
setlocal enabledelayedexpansion
set BAT_PATH=%~dp0

if not defined PYTHON (
  set PYTHON=%BAT_PATH%\..\..\..\phytium-rtos-dev-tools\Python38\python
)
set make_path=%BAT_PATH%\..\..\..\phytium-rtos-dev-tools\xpack-windows-build-tools-4.3.0-1\bin
set PATH=%PATH%;%make_path%
set "example_path=%BAT_PATH%../../example"

call :traverseSubfolders "%example_path%"
goto :eof

:traverseSubfolders
for /d %%D in (%1\*) do (
    set "makefile_path=%%D\makefile"
    set "kconfig_path=%%D\Kconfig"

    if exist "!makefile_path!" (
     
	 if exist "!kconfig_path!" (
            echo Entering folder: %%D
			pushd "%%D"
			
			make clean
            make eclipse_proj_link
            popd

		) else (
			call :traverseSubfolders "%%D"
		)
		
    ) else (
		call :traverseSubfolders "%%D"
	)

    
	
)
goto :eof

