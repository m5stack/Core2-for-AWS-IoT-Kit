@ECHO Off
cd firmware
for /f "usebackq" %%B in (`wmic path Win32_SerialPort Where "Caption LIKE '%%Curiosity%%'" Get DeviceID ^| FIND "COM"`) do set comport=%%B

download_all_sb.bat nedbg ATSAMD21E18A Tools\serial_bridge\winc_serial_bridge.elf  3A0 0 0 %comport:~3%
pause