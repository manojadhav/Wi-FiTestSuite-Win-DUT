
@echo off
setlocal enableextensions enabledelayedexpansion

set EXPPATH="C:\Program Files\Intel\WiFi\bin\P2PConsole.exe"

call :getargc argc %*

set INTERFACE=%1
set INTERFACEVALUE=%2

set GROUPID=%3

set BSSID=%4
echo The GO MACis %BSSID%

set SSID=%5
echo The Group SSID is %SSID%

SET GROUPIDVALUE="%BSSID% %SSID%"
echo The group id is %GROUPIDVALUE%


set CTWINDOW=%6
set CTWINDOWVALUE=%7

SET ARGUMENTCOUNT=%argc%


::////////////////////////////////////////////Let's do it Dans way/////////////////////////



		echo %EXPPATH% sta_set_opportunistic_ps %INTERFACE% %INTERFACEVALUE% %GROUPID% %GROUPIDVALUE% %CTWINDOW% %CTWINDOWVALUE%
		%EXPPATH% sta_set_opportunistic_ps %INTERFACE% %INTERFACEVALUE% %GROUPID% %GROUPIDVALUE% %CTWINDOW% %CTWINDOWVALUE%
		endlocal
goto :eof


:getargc
		set getargc_v0=%1
		set /a "%getargc_v0%=0"
		:getargc_l0
			if not x%2x==xx (
						shift
						set /a "%getargc_v0%=%getargc_v0% + 1"
						goto :getargc_l0
					)
		set getargc_v0=
		goto :eof
