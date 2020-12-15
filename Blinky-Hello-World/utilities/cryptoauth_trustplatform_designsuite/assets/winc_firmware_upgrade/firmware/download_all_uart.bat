@echo off
echo Usage: %0 [optional com port]
echo Host must be running a serial bridge or ATcmd app

set CHPFAM=error
if exist firmware\m2m_image_3400.bin (
	set CHPFAM=3400
	set TGTCHIP=3400
)
if exist firmware\m2m_image_3A0.bin (
	set CHPFAM=3A0
	set TGTCHIP=3A0
)

if %CHPFAM%==error (
	echo Cannot find firmware to program
	echo Try running Prepare_image.cmd
	exit /b 1
)

set PRTIN=%1
if a%PRTIN%a==aa (
  call test_edbg.cmd comport.txt
  set /p PRGPORTNUM=<comport.txt
) else (
  set PRGPORTNUM=%PRTIN%
)
echo Using COM port %PRGPORTNUM%

rem ######## Instruction #########################################################
echo Please connect and power up the board.
echo.

rem ######## Program loop #########################################################
FOR /L %%H IN (1,1,5) DO (
	echo ** Programming attempt %%H of 5...
	echo call download_all.bat UART %TGTCHIP% 0 %PRGPORTNUM%
	call download_all.bat UART %TGTCHIP% 0 %PRGPORTNUM% && goto okout
)
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
echo Downloading failed
exit /b 1
)
