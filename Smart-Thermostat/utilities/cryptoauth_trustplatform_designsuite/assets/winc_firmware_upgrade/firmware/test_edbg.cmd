@echo off
rem return 1 if any edbg port is working
set /a PORTCOUNT=0

C:\windows\System32\Wbem\wmic path CIM_LogicalDevice where "Description like 'Curiosity Virtual COM Port%'" get /value > tempdbSSS.txt
C:\windows\System32\find.exe /c "DeviceID=COM" < tempdbSSS.txt > tmpportcountSSS.txt
C:\windows\System32\find.exe    "DeviceID=COM" < tempdbSSS.txt > edbgportSSS.txt
set /p PORTCOUNT=<tmpportcountSSS.txt
del tmpportcountSSS.txt
del tempdbSSS.txt
if %PORTCOUNT% GEQ 2 goto errexit
if %PORTCOUNT% EQU 0 goto errexit

:: Must be just one EDBG device
set /p EDBGPORTSSS=<edbgportSSS.txt
set EDBGPORT=%EDBGPORTSSS:~12%
echo Found: COM%EDBGPORT%
del edbgportSSS.txt
if a%1a==aa goto okexit
echo %EDBGPORT% > %1

:okexit
exit /B 0

:errexit
Echo Error: This script will only work with one EDBG device installed.
exit /B 1