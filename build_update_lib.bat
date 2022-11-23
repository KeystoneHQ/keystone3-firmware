
SET BUILD_FOLDER=%CD%\build
SET TOOLS_FOLDER=%CD%\tools
SET PADDING_PY=%TOOLS_FOLDER%\padding_bin_file\padding_bin_file.py

if not exist %BUILD_FOLDER% (
	mkdir %BUILD_FOLDER%
)

if not exist %BUILD_FOLDER%\padding_bin_file.py (
	copy %PADDING_PY% %BUILD_FOLDER%\padding_bin_file.py /Y
)

pushd build

cmake -G "Unix Makefiles" .. -DLIB_SIGNER=ON
make -j16
padding_bin_file.py mh1903.bin
popd

