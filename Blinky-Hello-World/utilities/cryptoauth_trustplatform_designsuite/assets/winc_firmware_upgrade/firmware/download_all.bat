@ECHO off
setlocal EnableDelayedExpansion

:: %1 type of bus
:: %2 (3A0 or 1500 or 3400) 
:: %3 (aardvark_serial_number or 0)
:: %4 com port for edbg usb device or 0

set TOOL=%1
set TGTIN=%2
set AADVSNIN=%3
set PRTIN=%4

echo Mode %TOOL%
if /I "%TOOL%" == "UART"  Goto contine_UART
if /I "%TOOL%" == "I2C"   Goto contine_I2C

goto usage

:contine_I2C
Set  FMode=debug_i2c
set Mode=I2C
set ptool=winc_programmer_i2c.exe
goto START

:contine_UART
Set  FMode=debug_uart
set Mode=UART
set ptool=winc_programmer_uart.exe  -p \\.\COM%PRTIN%
goto START

:START

if /I "a%AADVSNIN%a" == "aa" (
	set AARDVARK=0
) else (
	set AARDVARK=%AADVSNIN%
)

if /I "a%5a" == "aa" (
  set COMPORT=
) else (
  if /I "a%5a" == "a0a" (
  	set COMPORT=
  ) else (
	set COMPORT=-port %5
  )
)

set COMPORT=-port %PRTIN%

echo Chip %TGTIN%
if /I "%TGTIN%" == "3400"  Goto chip3400
if /I "%TGTIN%" == "3A0"   Goto chip3A0
if /I "%TGTIN%" == "1500"  Goto chip3A0

:USAGE
echo Usage %0 (I2C-UART) (3A0 or 1500 or 3400) (aardvark_serial_number or 0) (comport or 0 for I2C)
@ECHO %CMDCMDLINE% | FIND /I /C "/C" > NUL && PAUSE
exit /b 2

:chip3A0
set VARIANT=3A0
set CHPFAM=1500
set PFW=programmer_release_text.bin
goto parmsok

:chip3400
set VARIANT=3400
set CHPFAM=3400
set PFW=programmer_firmware.bin
goto parmsok

:parmsok

:: pushd firmware

echo Tools\image_builder_downloader\image_tool.exe -c flash_image.config -c Tools\gain_builder\gain_sheets\new_gain.config -o firmware\m2m_image_%VARIANT%.bin -of prog
Tools\image_builder_downloader\image_tool.exe -c flash_image.config -c Tools\gain_builder\gain_sheets\new_gain.config -o firmware\m2m_image_%TGT_CHIP%.bin -of prog


pushd Tools\image_builder_downloader
echo Downloading Image...

echo Call %ptool% -d winc%CHPFAM% -e -i ..\..\firmware\m2m_image_%VARIANT%.bin -if prog -w -r

echo      -pfw ..\programmer_firmware\release%VARIANT%\%PFW%
%ptool% -d winc%CHPFAM% -i ..\..\firmware\m2m_image_%VARIANT%.bin -if prog -e -w -r -pfw ..\..\programmer_firmware\release%VARIANT%\%PFW%
if %ERRORLEVEL% NEQ 0 GOTO FAILED  
popd

pushd Tools\root_certificate_downloader\%FMode%

echo Tools\root_certificate_downloader\%FMode%
echo Downloading root certificates...
set /a c = 0
set seq=
for %%X in (..\binary\*.cer) do (
	set /a c+=1
	@set seq=!seq! %%X
)
echo root_certificate_downloader.exe ^
	-n %c% %seq% ^
	-no_wait ^
	-aardvark %AARDVARK% %COMPORT% ^
	-vflash_path ../../../vflash_root_certificate_downloader.bin ^
	-bf_bin  ../../../boot_firmware/release%VARIANT%/boot_firmware.bin ^
	-pf_bin  ../../../programmer_firmware/release%VARIANT%/programmer_firmware.bin ^
	-df_bin  ../../../downloader_firmware/release%VARIANT%/downloader_firmware.bin ^
	-e
root_certificate_downloader.exe ^
	-n %c% %seq% ^
	-no_wait ^
	-aardvark %AARDVARK% %COMPORT% ^
	-vflash_path ../../../vflash_root_certificate_downloader.bin ^
	-bf_bin  ../../../boot_firmware/release%VARIANT%/boot_firmware.bin ^
	-pf_bin  ../../../programmer_firmware/release%VARIANT%/programmer_firmware.bin ^
	-df_bin  ../../../downloader_firmware/release%VARIANT%/downloader_firmware.bin ^
	-e
IF ERRORLEVEL 1 goto FAILED
popd
goto SUCCESS

timeout 5

:FAILED
popd
echo *************** Failed to download *****************
@ECHO %CMDCMDLINE% | FIND /I /C "/C" > NUL && PAUSE
exit /b 1

:SUCCESS
popd
echo *************** Download worked *****************
@ECHO %CMDCMDLINE% | FIND /I /C "/C" > NUL && PAUSE
exit /b 0
