setlocal
set OUTDIR=%1
set ROOTDIR=%2

xcopy /y %ROOTDIR%Libraries\fmod\lib\win\fmod.dll %OUTDIR%
xcopy /y %ROOTDIR%Libraries\GLEW\lib\win\x86\glew32.dll %OUTDIR%
xcopy /y %ROOTDIR%Libraries\SDL\Windows\lib\x86\SDL2.dll %OUTDIR%
xcopy /y %ROOTDIR%Libraries\GLEW\lib\win\x86\glew32.dll %OUTDIR%
xcopy /y %ROOTDIR%Libraries\zlib\Windows\lib\x86\zlib.dll %OUTDIR%

xcopy /y /s /e /d %ROOTDIR%Assets %OUTDIR%Assets\

endlocal