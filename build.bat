@echo off
SETLOCAL ENABLEEXTENSIONS

SET BUILD_FOLDER=%CD%\build
SET BUILD_SIMULATOR_FOLDER=%CD%\build_simulator
SET TOOLS_FOLDER=%CD%\tools
SET MAKE_OAT_FILE_PATH=%TOOLS_FOLDER%\ota_file_maker
SET MAKE_PADDING_FILE_PATH=%TOOLS_FOLDER%\padding_bin_file
SET ASTYLE_PATH=%TOOLS_FOLDER%\AStyle.bat
SET PACK_PATH=%CD%\pack.bat
SET LANGUAGE_PATH=%CD%\src\ui\lv_i18n
SET LANGUAGE_SCRIPT=py data_loader.py
SET RUST_C_PATH=%CD%\rust\rust_c

SET "build_options=log copy production screen debug format release rebuild btc_only simulator language clean"
FOR %%O IN (%build_options%) DO SET "build_%%O=false"

FOR %%i in (%*) DO (
    IF /I "%%i"=="format" (
        pushd %TOOLS_FOLDER%
        echo format file...
        call %ASTYLE_PATH%
        popd
    ) ELSE (
        SET "build_%%i=true"
    )
)

IF "%build_rebuild%"=="true" (
    IF EXIST %BUILD_FOLDER% rd /s /q %BUILD_FOLDER%
    pushd %RUST_C_PATH%
    cargo clean
    popd  
)

IF NOT EXIST %BUILD_FOLDER% mkdir %BUILD_FOLDER%

IF NOT EXIST %BUILD_FOLDER%\padding_bin_file.py (
    copy %TOOLS_FOLDER%\padding_bin_file\padding_bin_file.py %BUILD_FOLDER%\padding_bin_file.py /Y
)

CALL :EXECUTE_BUILD

ENDLOCAL
GOTO :EOF

:EXECUTE_BUILD
IF "%build_language%"=="true" (
    pushd %LANGUAGE_PATH%
    %LANGUAGE_SCRIPT%
    popd  
)

SET cmake_parm=
IF "%build_production%"=="true" SET "cmake_parm=%cmake_parm% -DBUILD_PRODUCTION=true"
IF "%build_btc_only%"=="true" SET "cmake_parm=%cmake_parm% -DBTC_ONLY=true"
IF "%build_screen%"=="true" SET "cmake_parm=%cmake_parm% -DENABLE_SCREEN_SHOT=true"
IF "%build_debug%"=="true" SET "cmake_parm=%cmake_parm% -DDEBUG_MEMORY=true"

IF "%build_simulator%"=="true" (
    IF NOT EXIST %BUILD_SIMULATOR_FOLDER% mkdir %BUILD_SIMULATOR_FOLDER%
    pushd %BUILD_SIMULATOR_FOLDER%
    cmake -G "Unix Makefiles" -DBUILD_TYPE=Simulator %cmake_parm% .. 
    make -j16
    popd
) ELSE (
    pushd %BUILD_FOLDER%
    cmake -G "Unix Makefiles" %cmake_parm% ..
    IF "%build_log%"=="true" (
        make -j16 > makefile.log 2>&1
    ) ELSE (
        make -j16
    )
    python3 padding_bin_file.py mh1903.bin
    popd
)

IF "%build_copy%"=="true" (
    echo generating pillar.bin file...
    pushd %MAKE_OAT_FILE_PATH%
    echo generating pillar.bin file...
    call make_ota_file.bat %CD%\build\pillar.bin
    call make_ota_file.bat %CD%\build\keystone3.bin
    call make_ota_file.bat d:\pillar.bin
    popd
) ELSE IF "%build_release%"=="true" (
    pushd %MAKE_OAT_FILE_PATH%
    echo generating pillar.bin file...
    call make_ota_file.bat %CD%\build\pillar.bin
    call make_ota_file.bat %CD%\build\keystone3.bin
    popd
) ELSE IF "%build_simulator%"=="true" (
    .\build\simulator.exe
)

GOTO :EOF