@echo off
REM Batch script to compile and run the University Help Guidebook.
REM The 'gcc' command must be available in your system's PATH for this to work.

set EXECUTABLE_NAME=university_guide
set SOURCE_FILE=university_guide.c

echo ===========================================
echo 1. COMPILING C SOURCE CODE
echo ===========================================
echo Attempting to compile %SOURCE_FILE%...

REM Compile the C file
gcc %SOURCE_FILE% -o %EXECUTABLE_NAME%

REM Check if compilation was successful
if errorlevel 1 goto :COMPILATION_ERROR

echo.
echo Compilation successful! Executable created: %EXECUTABLE_NAME%.exe
echo.
echo ===========================================
echo 2. RUNNING THE APPLICATION
echo ===========================================
echo Starting the University Help Guidebook...
echo.

REM Run the compiled executable
%EXECUTABLE_NAME%.exe

goto :END

:COMPILATION_ERROR
echo.
echo ===========================================
echo !!! ERROR !!!
echo ===========================================
echo Compilation FAILED! Please check your C code (%SOURCE_FILE%)
echo for errors and ensure GCC is correctly installed and configured.
pause
goto :END

:END