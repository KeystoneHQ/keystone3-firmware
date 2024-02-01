@echo off
setlocal

set "BUILD_FOLDER=%CD%\build"
set "TOOLS_FOLDER=%CD%\tools"
set "MAKE_OAT_FILE_PATH=%TOOLS_FOLDER%\ota_file_maker"
set "ASTYLE_PATH=%TOOLS_FOLDER%\AStyle.bat"
set "LANGUAGE_PATH=%CD%\src\ui\lv_i18n"
set "LANGUAGE_SCRIPT=py data_loader.py"

set "CLEAN_BUILD=0"
set "PERFORM_COPY=0"
set "FORMAT_CODE=0"
set "BUILD_TYPE="
set "BUILD_RU=0"
set "BUILD_CN=0"

:parse_args
if "%~1"=="" goto end_parse_args
if /i "%~1"=="log" set "CLEAN_BUILD=1"
if /i "%~1"=="copy" set "PERFORM_COPY=1"
if /i "%~1"=="format" set "FORMAT_CODE=1"
if /i "%~1"=="debug" set "BUILD_TYPE=-DDEBUG_MEMORY=true"
if /i "%~1"=="production" set "BUILD_TYPE=-DBUILD_PRODUCTION=true"
if /i "%~1"=="screen" set "BUILD_TYPE=-DENABLE_SCREEN_SHOT=true"
if /i "%~1"=="RU" (set "BUILD_TYPE=%BUILD_TYPE% -DRU_SUPPORT=true" & set "BUILD_RU=1")
if /i "%~1"=="CN" (set "BUILD_TYPE=%BUILD_TYPE% -DCN_SUPPORT=true" & set "BUILD_CN=1")
shift
goto parse_args
:end_parse_args

if "%FORMAT_CODE%"=="1" (
	pushd %TOOLS_FOLDER%
	echo format file...
    call "%ASTYLE_PATH%"
	exit /b 0
)

if "%CLEAN_BUILD%"=="1" (
    if exist "%BUILD_FOLDER%" rd /s /q "%BUILD_FOLDER%"
)

if not exist "%BUILD_FOLDER%" (
    mkdir "%BUILD_FOLDER%"
)

if not exist "%BUILD_FOLDER%\padding_bin_file.py" (
    copy "%TOOLS_FOLDER%\padding_bin_file\padding_bin_file.py" "%BUILD_FOLDER%" /Y
)

if "%PERFORM_COPY%"=="1" (
    del "%BUILD_FOLDER%\mh1903.elf" 2>nul
    del "%BUILD_FOLDER%\mh1903.map" 2>nul
    del "%BUILD_FOLDER%\mh1903.hex" 2>nul
    del "%BUILD_FOLDER%\mh1903.bin" 2>nul
)

if "%BUILD_CN%"=="1" (
    set "LANGUAGE_SCRIPT=%LANGUAGE_SCRIPT% --zh"
)

if "%BUILD_RU%"=="1" (
    set "LANGUAGE_SCRIPT=%LANGUAGE_SCRIPT% --ru"
)

pushd %LANGUAGE_PATH%
%LANGUAGE_SCRIPT%
popd

pushd "%BUILD_FOLDER%"
if not "%BUILD_TYPE%"=="" (
    cmake -G "Unix Makefiles" %BUILD_TYPE% ..
) else (
    cmake -G "Unix Makefiles" ..
)

:: Build the project and handle the 'log' argument to output to a file
if "%CLEAN_BUILD%"=="1" (
    make -j16 > makefile.log 2>&1
) else (
    make -j16 | stdbuf -oL tr '\n' '\n'
)

:: Run padding_bin_file.py
python3 "%BUILD_FOLDER%\padding_bin_file.py" "%BUILD_FOLDER%\mh1903.bin"
popd

:: Handle 'copy' argument to generate .bin files
if "%PERFORM_COPY%"=="1" (
    pushd "%MAKE_OAT_FILE_PATH%"
    echo Generating pillar.bin file...
    call make_ota_file.bat "%BUILD_FOLDER%\pillar.bin"
    call make_ota_file.bat "%BUILD_FOLDER%\keystone3.bin"
    call make_ota_file.bat d:\pillar.bin
    popd
)

:: End localization of environment changes
endlocal