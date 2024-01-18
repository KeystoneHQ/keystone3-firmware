@echo off

SET BUILD_FOLDER=%CD%\build
SET TOOLS_FOLDER=%CD%\tools
SET ALWAYSE_BUILD_FILE=%CD%\driver\drv_sys.c
SET MAKE_OAT_FILE_PATH=%TOOLS_FOLDER%\ota_file_maker
SET MAKE_PADDING_FILE_PATH=%TOOLS_FOLDER%\padding_bin_file
SET ASTYLE_PATH=%TOOLS_FOLDER%\AStyle.bat
SET UPDATE_LANGUAGE_PATH=%CD%\src\ui\lv_i18n
SET PACK_PATH=%CD%\pack.bat

if "%1"=="log" (
	rd /s /q %BUILD_FOLDER%
)

if not exist %BUILD_FOLDER% (
	mkdir %BUILD_FOLDER%
)

if not exist %BUILD_FOLDER%\padding_bin_file.py (
	copy %TOOLS_FOLDER%\padding_bin_file\padding_bin_file.py %BUILD_FOLDER%\padding_bin_file.py /Y
)

if "%1" == "copy" (
	del %BUILD_FOLDER%\mh1903.elf
	del %BUILD_FOLDER%\mh1903.map
	del %BUILD_FOLDER%\mh1903.hex
	del %BUILD_FOLDER%\mh1903.bin
)

if "%2"=="language" (
	pushd build
)

pushd build
if "%2"=="production" (
	cmake -G "Unix Makefiles" -DBUILD_PRODUCTION=true ..
) else if "%2"=="screen" (
	cmake -G "Unix Makefiles" -DENABLE_SCREEN_SHOT=true ..
) else if "%2"=="debug" (
	cmake -G "Unix Makefiles" -DDEBUG_MEMORY=true=true ..
) else (
	cmake -G "Unix Makefiles" ..
)

if "%1"=="log" (
	make -j16 > makefile.log 2>&1
) else (
	make -j16 | stdbuf -oL tr '\n' '\n'
)
python3 .\padding_bin_file.py .\mh1903.bin
popd


if "%1" == "copy" (
	pushd %MAKE_OAT_FILE_PATH%
	echo generating pillar.bin file...
	call make_ota_file.bat %CD%\build\pillar.bin
	call make_ota_file.bat %CD%\build\keystone3.bin
	call make_ota_file.bat d:\pillar.bin
	popd
)

if "%1" == "format" (
	pushd %TOOLS_FOLDER%
	echo format file...
	call %ASTYLE_PATH%
	popd
)

if "%1" == "release" (
	pushd %MAKE_OAT_FILE_PATH%
	echo generating pillar.bin file...
	call make_ota_file.bat %CD%\build\pillar.bin
	call make_ota_file.bat %CD%\build\keystone3.bin
	popd
	@REM call %PACK_PATH%
)