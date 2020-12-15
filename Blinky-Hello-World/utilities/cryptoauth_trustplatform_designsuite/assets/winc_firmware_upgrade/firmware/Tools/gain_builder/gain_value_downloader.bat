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
set ptool=Tools\image_builder_downloader\winc_programmer_uart.exe  -p \\.\COM%PRTIN%
goto START

:START

if /I "a%AADVSNIN%a" == "aa" (
	set AARDVARK=0
) else (
	set AARDVARK=%AADVSNIN%
)


echo Chip %TGTIN%
if /I "%TGTIN%" == "3400"  Goto chip3400
if /I "%TGTIN%" == "3A0"   Goto chip3A0
if /I "%TGTIN%" == "1500"  Goto chip3A0

:USAGE
echo Usage %0 (I2C-UART) (3400 or 1500) (aardvark_serial_number or 0) (comport or 0 for I2C)
@ECHO %CMDCMDLINE% | FIND /I /C "/C" > NUL && PAUSE
exit /b 2

:chip3A0
set VARIANT=3A0
set CHPFAM=1500
set PFW=programmer_firmware\release3A0\programmer_release_text.bin
goto parmsok

:chip3400
set VARIANT=3400
set CHPFAM=3400
set PFW=programmer_firmware\release3400\programmer_firmware.bin
goto parmsok

:parmsok

cd ..\..\
REM echo %cd%
echo Downloading Image...

:: winc_programmer_uart.exe -d wincXX00 -i m2m_image_XX00.bin -if prog -w -r -p \\.\COM73 -pfw ..\programmer_firmware\releaseXX00\programmer_firmware.bin
echo Building flash images (prog format)
echo Tools\image_builder_downloader\image_tool.exe -c Tools\gain_builder\gain_val_image.config -o Tools\gain_builder\m2m_gain_value_image_%VARIANT%.bin -of prog
Tools\image_builder_downloader\image_tool.exe -c Tools\gain_builder\gain_val_image.config -o Tools\gain_builder\m2m_gain_value_image_%VARIANT%.bin -of prog
echo Call %ptool% -d winc%CHPFAM% -i Tools\gain_builder\m2m_gain_value_image_%VARIANT%.bin -if prog -w -r -pfw programmer_firmware\release%VARIANT%\%PFW%
%ptool% -rs 0x004000 -re 0x005000 -d winc%CHPFAM% -i Tools\gain_builder\m2m_gain_value_image_%VARIANT%.bin -if prog -w -r -pfw %PFW%
if %ERRORLEVEL% NEQ 0 GOTO FAILED  
goto SUCCESS

:FAILED
echo            *************** Failed to download *****************
echo     #######################################################################
echo     ##                                                                   ##
echo     ##                    ########    ###     ####  ##                   ##
echo     ##                    ##         ## ##     ##   ##                   ##
echo     ##                    ##        ##   ##    ##   ##                   ##
echo     ##                    ######   ##     ##   ##   ##                   ##
echo     ##                    ##       #########   ##   ##                   ##
echo     ##                    ##       ##     ##   ##   ##                   ##
echo     ##                    ##       ##     ##  ####  ########             ##
echo     ##                                                                   ##
echo     #######################################################################
pause
exit /b 1

:SUCCESS
echo             *************** Download worked *****************
echo     #######################################################################
echo     ##                                                                   ##
echo     ##                 ########     ###     ######   ######              ##
echo     ##                 ##     ##   ## ##   ##    ## ##    ##             ##
echo     ##                 ##     ##  ##   ##  ##       ##                   ##
echo     ##                 ########  ##     ##  ######   ######              ##
echo     ##                 ##        #########       ##       ##             ##
echo     ##                 ##        ##     ## ##    ## ##    ##             ##
echo     ##                 ##        ##     ##  ######   ######              ##
echo     ##                                                                   ##
echo     #######################################################################
pause
exit /b 0
