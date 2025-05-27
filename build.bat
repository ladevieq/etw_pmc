@ECHO OFF

set BUILD_DIR=build

IF EXIST %BUILD_DIR% rmdir %BUILD_DIR%\ /S /Q

mkdir build

:: set C_FLAGS=/nologo /W4 /WX /Zi /GS- /GR- /Gs1000000000 /Fo:%BUILD_DIR%\ /Iinclude /Isrc /std:c17 /c /Tc
:: set C_FLAGS=/nologo /W4 /WX /Zi /GS- /GR- /Gs1000000000 /Fo:%BUILD_DIR%\ /Iinclude /Isrc /std:c17 /D_CRT_SECURE_NO_WARNINGS= /Tc
:: set L_FLAGS=/WX /SUBSYSTEM:CONSOLE /NODEFAULTLIB /stack:0x100000,100000
:: set L_FLAGS=/WX /SUBSYSTEM:CONSOLE /stack:0x100000,100000

:: cl /Fd:%BUILD_DIR%\main.pdb %C_FLAGS% src\main.c /link %L_FLAGS% user32.lib advapi32.lib libucrtd.lib
cl.exe /Zi /Od /Fd:%BUILD_DIR%\main.pdb /Fo:%BUILD_DIR%\main.obj src\main.c /link /SUBSYSTEM:CONSOLE /OUT:%BUILD_DIR%\main.exe user32.lib advapi32.lib libucrtd.lib
