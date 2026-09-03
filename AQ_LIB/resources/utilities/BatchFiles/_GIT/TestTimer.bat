@ECHO OFF

SET start="12:00:00"
SET end="12:01:05"

CALL "%MLIBQ%\resource\utilities\BatchFiles\_GIT\Timer.bat" %start%, %end%, testprocess
ECHO %TIMER_RESULT%

PAUSE

CALL "%MLIBQ%\resource\utilities\BatchFiles\_GIT\Timer.bat" %start%, %end%, ""
ECHO %TIMER_RESULT%

PAUSE