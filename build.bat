@echo off

SET BUILD_FOLDER=%CD%\build
SET BUILD_SIMULATOR_FOLDER=%CD%\build_simulator
SET TOOLS_FOLDER=%CD%\tools
SET ALWAYSE_BUILD_FILE=%CD%\driver\drv_sys.c
SET MAKE_OAT_FILE_PATH=%TOOLS_FOLDER%\ota_file_maker
SET MAKE_PADDING_FILE_PATH=%TOOLS_FOLDER%\padding_bin_file
SET ASTYLE_PATH=%TOOLS_FOLDER%\AStyle.bat
SET PACK_PATH=%CD%\pack.bat
set "LANGUAGE_PATH=%CD%\src\ui\lv_i18n"
set "LANGUAGE_SCRIPT=py data_loader.py"

SET build_log=false
SET build_copy=false
SET build_production=false
SET build_screen=false
SET build_debug=false
SET build_release=false
SET build_rebuild=false
SET build_btc_only=false
SET build_simulator=false
SET build_language=false

for %%i in (%*) do (
    if /I "%%i"=="log" (
        set build_log=true
    )
    if /I "%%i"=="copy" (
        set build_copy=true
    )
    if /I "%%i"=="production" (
        set build_production=true
    )
    if /I "%%i"=="screen" (
        set build_screen=true
    )
    if /I "%%i"=="debug" (
        set build_debug=true
    )
    if /I "%%i"=="format" (
        pushd %TOOLS_FOLDER%
        echo format file...
        call %ASTYLE_PATH%
        popd
    )
    if /I "%%i"=="release" (
        set build_release=true
    )
    if /I "%%i"=="rebuild" (
        set build_rebuild=true
    )
    if /I "%%i"=="btc_only" (
        set build_btc_only=true
    )
    if /I "%%i"=="simulator" (
        set build_simulator=true
    )
    if /I "%%i"=="language" (
        set build_language=true
    )
)

if "%build_rebuild%"=="true" (
    rd /s /q %BUILD_FOLDER%
) 

if not exist %BUILD_FOLDER% (
    mkdir %BUILD_FOLDER%
)

if not exist %BUILD_FOLDER%\padding_bin_file.py (
    copy %TOOLS_FOLDER%\padding_bin_file\padding_bin_file.py %BUILD_FOLDER%\padding_bin_file.py /Y
)

if "%build_copy%"=="true" (
    del %BUILD_FOLDER%\mh1903.elf
    del %BUILD_FOLDER%\mh1903.map
    del %BUILD_FOLDER%\mh1903.hex
    del %BUILD_FOLDER%\mh1903.bin
)

set cmake_parm=
if "%build_language%"=="true" (
    pushd %LANGUAGE_PATH%
    %LANGUAGE_SCRIPT% --zh --ru --ko
    popd  
)

if "%build_production%"=="true" (
    set cmake_parm=%cmake_parm% -DBUILD_PRODUCTION=true
)
if "%build_btc_only%"=="true" (
    set cmake_parm=%cmake_parm% -DBTC_ONLY=true
)
if "%build_screen%"=="true" (
    set cmake_parm=%cmake_parm% -DENABLE_SCREEN_SHOT=true
)
if "%build_debug%"=="true" (
    set cmake_parm=%cmake_parm% -DDEBUG_MEMORY=true
)
if "%build_simulator%"=="true" (
    if not exist %BUILD_SIMULATOR_FOLDER% (
        mkdir %BUILD_SIMULATOR_FOLDER%
    )

    pushd %BUILD_SIMULATOR_FOLDER%
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Simulator %cmake_parm% .. 
    make -j16
    popd
) else (
    echo %cmake_parm%
    pushd %BUILD_FOLDER%
    cmake -G "Unix Makefiles" %cmake_parm% ..

    if "%build_log%"=="true" (
        make -j16 > makefile.log 2>&1
    ) else (
        make -j16 | stdbuf -oL tr '\n' '\n'
    )
    python3 .\padding_bin_file.py .\mh1903.bin
    popd
)

if "%build_copy%"=="true" (
    pushd %MAKE_OAT_FILE_PATH%
    echo generating pillar.bin file...
    call make_ota_file.bat %CD%\build\pillar.bin
    call make_ota_file.bat %CD%\build\keystone3.bin
    call make_ota_file.bat d:\pillar.bin
    popd
) else if "%build_release%"=="true" (
    pushd %MAKE_OAT_FILE_PATH%
    echo generating pillar.bin file...
    call make_ota_file.bat %CD%\build\pillar.bin
    call make_ota_file.bat %CD%\build\keystone3.bin
    popd
) else if "%build_simulator%"=="true" (
    .\build\simulator.exe
)