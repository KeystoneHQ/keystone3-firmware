set(CMAKE_SYSTEM_NAME Generic)
cmake_minimum_required(VERSION 3.10)
project(demo)

set(trezor-crypto-src
	trezor-crypto/memzero.c 
	trezor-crypto/pbkdf2.c 
	trezor-crypto/sha2.c 
	trezor-crypto/hmac.c 
	trezor-crypto/rand.c
)

include_directories(trezor-crypto ../../mid/log/ ../../core/)

set(objsrc slip39_demo.c slip39.c gf256.c rs1024.c ../../mid/log/log_print.c ${trezor-crypto-src})

add_executable(demo ${objsrc})
