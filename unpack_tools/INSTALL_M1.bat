;Very simple script to create a single
@ECHO OFF
cls
ECHO.
ECHO Magden USB memory stick creator.
ECHO.
SET /P drive=Enter drive letter of USB stick with no colon. (Example E, D, F): 
ECHO.
ECHO WARNING: All data on drive %drive% will be erased.
SET /P yesno=To continue type yes: 
IF NOT %yesno%==yes exit
ECHO.
ECHO Formatting drive %drive%:
ECHO yes | FORMAT /FS:FAT32 /V:M1Install /Q %drive%: > log.txt
ECHO Copying data to drive %drive%:
copy image\*.* %drive%: >> log.txt
syslinux %drive%: >> log.txt
ECHO Done.
ECHO.
ECHO To install the M1, please do the following
ECHO 1. Remove the USB memory stick from this computer.
ECHO 2. Ensure that the M1 unit is turned off.
ECHO 3. Insert the USB memory stick in the M1 unit. (Any USB port works.)
ECHO 4. Power on the M1 unit and follow the instructions on the unit screen.
ECHO.
ECHO You can close this window at any time.
pause >> log.txt
