@echo off
ED.EXE
IF ERRORLEVEL 1 GOTO FAIL       ; Any non-zero = fail
echo PASS: Return code 0 verified
GOTO END
:FAIL
echo FAIL: Expected 0
:END
