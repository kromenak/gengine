setlocal
set ROOT_DIR=%1
set BUILD_DIR=%2
set INSTALL_DIR=%3
set VERSION_NUM=%4

rem Make sure install directory exists and is empty.
mkdir %INSTALL_DIR%
del %INSTALL_DIR%\*

rem Copy exe. Note the "echo d" tells xcopy that INSTALL_DIR is a directory and not a file.
echo d | xcopy /y %BUILD_DIR%\gk3.exe %INSTALL_DIR%

rem Copy all DLLs.
xcopy /y %BUILD_DIR%\*.dll %INSTALL_DIR%

rem Copy assets folder.
xcopy /y /s /e /d %ROOT_DIR%\Assets %INSTALL_DIR%\Assets\

rem Make Data directory in app bundle with README file.
mkdir %INSTALL_DIR%\Data
echo d | xcopy /y %ROOT_DIR%\Data\README.md %INSTALL_DIR%\Data\

rem Copy ini file.
echo d | xcopy /y %ROOT_DIR%\GK3.ini %INSTALL_DIR%

rem Create zip file.
powershell Compress-Archive -Path '%INSTALL_DIR%\*' -DestinationPath '%INSTALL_DIR%\GK3-Win-%VERSION_NUM%.zip' -Force
endlocal