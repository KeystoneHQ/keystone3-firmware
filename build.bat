@echo off

SET BUILD_FOLDER=%CD%\build
SET TOOLS_FOLDER=%CD%\tools
SET ALWAYSE_BUILD_FILE=%CD%\driver\drv_sys.c
SET MAKE_OAT_FILE_PATH=%TOOLS_FOLDER%\ota_file_maker
SET ASTYLE_PATH=%TOOLS_FOLDER%\AStyle.bat
SET PACK_PATH=%CD%\pack.bat

if "%1"=="log" (
	rd /s /q %BUILD_FOLDER%
)

if not exist %BUILD_FOLDER% (
	mkdir %BUILD_FOLDER%
)

if exist %BUILD_FOLDER%\make_file_test.bat (
	copy %MAKE_OAT_FILE% %BUILD_FOLDER%\make_file_test.bat /Y
)

touch %ALWAYSE_BUILD_FILE%
if "%1" == "copy" (
	del %BUILD_FOLDER%\mh1903.elf
	del %BUILD_FOLDER%\mh1903.map
	del %BUILD_FOLDER%\mh1903.hex
	del %BUILD_FOLDER%\mh1903.bin
)

pushd build
if "%2"=="production" (
	cmake -G "Unix Makefiles" -DBUILD_PRODUCTION=true ..
) else (
	cmake -G "Unix Makefiles" ..
)

if "%1"=="log" (
	make -j16 > makefile.log 2>&1
) else (
	make -j16 | stdbuf -oL tr '\n' '\n'
)
popd

if "%1" == "copy" (
	pushd %MAKE_OAT_FILE_PATH%
	echo generating pillar.bin file...
	call make_ota_file.bat d:\pillar.bin
	call make_ota_file.bat %CD%\build\pillar.bin
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