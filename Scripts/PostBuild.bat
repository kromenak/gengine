setlocal
set OUTDIR=%1
set ROOTDIR=%2

rem Copy ffmpeg libraries.
xcopy /y %ROOTDIR%\Libraries\ffmpeg\lib\win\avcodec-58.dll %OUTDIR%
xcopy /y %ROOTDIR%\Libraries\ffmpeg\lib\win\avformat-58.dll %OUTDIR%
xcopy /y %ROOTDIR%\Libraries\ffmpeg\lib\win\avutil-56.dll %OUTDIR%
xcopy /y %ROOTDIR%\Libraries\ffmpeg\lib\win\swresample-3.dll %OUTDIR%
xcopy /y %ROOTDIR%\Libraries\ffmpeg\lib\win\swscale-5.dll %OUTDIR%

rem Copy fmod libraries.
xcopy /y %ROOTDIR%\Libraries\fmod\lib\win\fmod.dll %OUTDIR%

rem Copy GLEW libraries.
xcopy /y %ROOTDIR%\Libraries\GLEW\lib\win\x86\glew32.dll %OUTDIR%

rem Copy SDL libraries.
xcopy /y %ROOTDIR%\Libraries\SDL\win\lib\x86\SDL2.dll %OUTDIR%

rem Copy Zlib libraries.
xcopy /y %ROOTDIR%\Libraries\zlib\lib\win\x86\zlib1.dll %OUTDIR%

endlocal