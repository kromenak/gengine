setlocal
set BUILD_DIR=%1
set INSTALL_DIR=%2
set VERSION_NUM=%3

rem Copy exe. Note the "echo d" tells xcopy that INSTALL_DIR is a directory and not a file.
echo d | xcopy /y %BUILD_DIR%\gk3.exe %INSTALL_DIR%

rem Copy all DLLs.
xcopy /y %BUILD_DIR%\*.dll %INSTALL_DIR%

rem Copy assets folder.
xcopy /y /s /e /d  %BUILD_DIR%\Assets %INSTALL_DIR%\Assets\

rem Delete any copyrighted materials.
del %INSTALL_DIR%\Assets\GK3\*.brn
del %INSTALL_DIR%\Assets\GK3\*.bik
del %INSTALL_DIR%\Assets\GK3\*.avi

rem Create zip file.
powershell Compress-Archive -Path '%INSTALL_DIR%' -DestinationPath '%INSTALL_DIR%\GK3-Win-%VERSION_NUM%.zip' -Force
endlocal