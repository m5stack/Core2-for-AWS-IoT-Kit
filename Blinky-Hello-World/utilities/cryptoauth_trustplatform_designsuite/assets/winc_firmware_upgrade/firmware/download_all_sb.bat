@echo off
setlocal  EnableDelayedExpansion

if /I a%1a==aa (
	echo Usage egs: 
	echo   download_all_sb 
	echo       edbg 
	echo       [mcu eg ATSAMD21J18A]
	echo       [path to at cmd app or serial bridge elf file]
	echo       [3A0 or 3400]
	echo       [SamD2x SNo [eg ATML2130021800006127] or 0]
	echo       [com port or 0]
)

set TOOL=%1
set MCU=%2
set IMAGE_FILE=%3
set TGTIN=%4
set HOSTSNIN=%5
set PRTIN=%6
::set varPath=%PROGRAMFILES%
set varPath=%PROGRAMFILES(X86)%

set TGTCHIP=xx
if /I a%TGTIN%a==a3A0a (
  set TGTCHIP=3A0
)
if /I a%TGTIN%a==a3400a (
  set TGTCHIP=3400
)
if /I %TGTCHIP%==xx (
  echo Need to specify target chip 3400 or 3A0
  goto failout
)

if a%HOSTSNIN%a==aa (
  set D21SN=0
) else (
  set D21SN=%HOSTSNIN%
)
if /I %TOOL%==edbg (
  set TOOLSN=-s %D21SN%
) else (
  set TOOLSN=
)

if /I "%TOOLSN%"=="-s 0" (
  set TOOLSN=
)

if a%PRTIN%a==aa (
  set PRGPORTNUM=0
) else (
  set PRGPORTNUM=%PRTIN%
)

:gotparms

echo Checking for Python support.
where /q python.exe 
if ERRORLEVEL 1 GOTO NOPYTHON  
goto :HASPYTHON  
:NOPYTHON
echo Require Python v3.x
exit /b 2
:HASPYTHON 
echo OK
echo.

Echo Checking Atmel Studio Installation
python handler_search.py atsln atmelstudio > atmelstudiopath.txt
if ERRORLEVEL 1 GOTO NOAS2  
goto :HASAS2  
:NOAS2
echo Require Atmel Studio v7.0
exit /b 2
:HASAS2
set /p ASFULLPATH=<atmelstudiopath.txt
REM Typically "C:\Program Files x86\Atmel\Studio\7.0\atmelstudio.exe"
For %%A in (%ASFULLPATH%) do (
    Set ASPATH=%%~dpA
    Set ASEXE=%%~nxA
)
echo Found: %ASPATH%%ASEXE%  
echo.
set atprogram="%ASPATH%atbackend\atprogram.exe"

rem ######## Info #########################################################
echo Tool:    %atprogram%
echo Devices:
%atprogram% list
echo.

rem ######## Instruction #########################################################
echo Please connect %TOOL% and power up the board.
echo.
echo Calling...
echo %atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% chiperase
echo %atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% program ^^
echo     -f %IMAGE_FILE%
echo %atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% reset
echo download_all.bat UART %GAIN_MODE% %TGTCHIP% %AARDVARKSN% %PRGPORTNUM%
echo.

rem ######## Program loop #########################################################
FOR /L %%H IN (1,1,1) DO (
	echo ** Programming attempt %%H of 5...

	echo Erasing host...
	%atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% chiperase
	echo Programming host...
	%atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% program -f %IMAGE_FILE%
	if a%MCU%a==aATSAM4SD32Ca (
		echo Setting Boot from Flash...
		%atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% write -fs -o 0 --values 0x02
	)
	echo Resetting host...
	%atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% reset
	PING 127.0.0.1 -n 2 >NUL

	echo Verifying com port was enumerated...
	call test_edbg.cmd comport.txt
	if errorlevel 1 (
		echo Resetting host...
		%atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% reset
		PING 127.0.0.1 -n 2 >NUL
	)
	
	if %PRGPORTNUM% EQU 0 (
		set /p PRGPORTNUM=<comport.txt
		echo Setting port number to !PRGPORTNUM!
	)

	echo Downloading FW...
	echo call download_all.bat UART %TGTCHIP% 0 !PRGPORTNUM!
	call download_all.bat UART %TGTCHIP% 0 !PRGPORTNUM! && goto okout
)

echo Downloading failed
goto failout


:okout
echo OK
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
echo Downloading ends sucessfully
echo Resetting host...
%atprogram% -t %TOOL% %TOOLSN% -i SWD -d %MCU% reset
exit /b 0


:failout
echo Fail
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
exit /b 1
)
