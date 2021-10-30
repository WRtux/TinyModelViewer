@echo off
set Fs=
:L0
if not "%1"=="" (
 set Fs=%Fs%%1 
 shift /1
 goto L0
)
if "%Fs%"=="" (
 color E
 echo No input. Using *.c
 set Fs=*.c
)
echo Building project...
rem gcc -Wall -Os -nostdlib -e start -fno-asynchronous-unwind-tables -s -I include %Fs% -l msvcrt -L lib -o bin/build-new.exe
gcc -Wall -Os -s -I include %Fs% -L lib -l png -l z -l opengl32 -l glu32 -l freeglut -mwindows -o bin/build-new.exe
if not exist bin\build-new.exe (
 echo Build failed.
 pause >nul
 exit
)
move /y bin\build-new.exe bin\build.exe >nul
echo Build complete. Running...
bin\build.exe
if "%ERRORLEVEL%"=="0" (
 color A
 echo Run complete.
) else (
 color C
 echo Error: %ERRORLEVEL%
)
pause> nul
