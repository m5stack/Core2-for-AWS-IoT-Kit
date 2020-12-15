@ECHO off
::cd Tools\image_builder\debug\

:: Test path length.
if NOT "%CD:~230,1%"=="" (
	echo.
	echo [ERROR] File path is too long. Please move firmware update tool at the root of your hard drive and try again.
	echo.
	pause
	exit
)

echo.
echo Applying modification for 3A0 image:
echo.
Tools\image_builder_downloader\image_tool -http_modify provisioning_webpage/ -fw_path ota_firmware/m2m_ota_3a0.bin -c flash_image.config -no_wait
pause