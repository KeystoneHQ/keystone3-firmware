@echo off

set ZIP_TOOL_PATH=%CD%\tools\zip\7z.exe
set SRC_PATH=build
set BIN_PATH=%CD%\build\mh1903.bin

set COMPILE_TIME=%date:~0,10%-%time:~0,8%

set COMPILE_YEAR=%COMPILE_TIME:~0,4%
set COMPILE_MONTH=%COMPILE_TIME:~5,2%
set COMPILE_DAY=%COMPILE_TIME:~8,2%
set COMPILE_HOUR=%COMPILE_TIME:~11,2%
set COMPILE_HOUR_BAK=%COMPILE_TIME:~12,2%
if %COMPILE_HOUR% LEQ 8 (
	set COMPILE_HOUR=%COMPILE_TIME:~12,1%
)
set COMPILE_MIN=%COMPILE_TIME:~14,2%
set COMPILE_SEC=%COMPILE_TIME:~17,2%

set FW_FILE=build\KEYSTONE3-FW-%COMPILE_YEAR%-%COMPILE_MONTH%-%COMPILE_DAY%-%COMPILE_HOUR%-%COMPILE_MIN%-%COMPILE_SEC%.zip

echo %FW_FILE%

if exist %BIN_PATH% (
	echo %BIN_PATH% is exist!
	%ZIP_TOOL_PATH% a -tzip %FW_FILE% @listfile.txt
) else (
	echo %BIN_PATH% is not exist!
	goto end
)

:success
echo ******************************************************
echo *                                                    *
echo *                                                    *
echo *                                                    *
echo *                                                    *
echo *                    SUCCESS                         *
echo *                                                    *
echo *                                                    *
echo *                                                    *
echo *                                                    *
echo ******************************************************
rem pause
exit /b 0
:end
echo ******************************************************
echo *                                                    *
echo *                                                    *
echo *                                                    *
echo *                                                    *
echo *                      FAIL                          *
echo *                                                    *
echo *                                                    *
echo *                                                    *
echo *                                                    *
echo ******************************************************
rem pause
exit /b 0