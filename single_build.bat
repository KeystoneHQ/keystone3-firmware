@echo off

SET BUILD_FOLDER=%CD%\build
SET TOOLS_FOLDER=%CD%\tools
SET ALWAYSE_BUILD_FILE=%CD%\driver\drv_sys.c
SET MAKE_OAT_FILE_PATH=%TOOLS_FOLDER%\ota_file_maker

if not exist %BUILD_FOLDER% (
	mkdir %BUILD_FOLDER%
)

if exist %BUILD_FOLDER%\make_file_test.bat (
	copy %MAKE_OAT_FILE% %BUILD_FOLDER%\make_file_test.bat /Y
)

touch %ALWAYSE_BUILD_FILE%
del %BUILD_FOLDER%\mh1903.elf
del %BUILD_FOLDER%\mh1903.map
del %BUILD_FOLDER%\mh1903.hex
del %BUILD_FOLDER%\mh1903.bin

pushd build
cmake -G "Unix Makefiles" ..
make -j16 | stdbuf -oL tr '\n' '\n'
popd
