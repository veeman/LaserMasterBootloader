@setlocal
@echo off
cd /d "%~dp0"

set FULLFILENAME="%1"
set FILEPATH=%~dp1
set FILENAME=%~n1
set FILEEXT=%~x1

if %FULLFILENAME%=="" (
    echo No binary firmware file specified
    goto usage
)

if NOT "%FILEEXT%"==".bin" (
    echo This is not a binary file
    goto usage
)

srec_cat.exe "%FULLFILENAME%" -Binary ^
-offset 0x08004000 ^
-fill 0xFF 0x08004000 0x0800FFFC ^
-crop 0x08004000 0x0800FFFC ^
-CRC32_Big_Endian 0x800FFFC ^
-o "%FULLFILENAME%.hex" -Intel

echo Done


goto end
:usage
echo:
echo Usage: bin2hex_with_crc32_48k.bat firmware.bin
echo:
:end
PAUSE