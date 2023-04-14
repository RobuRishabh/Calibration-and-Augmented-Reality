
We used visualstudio IDE, for the project. for running the code, I just click on build the whole assignment and then click on the
"Local Windows Debugger".



----------------------------------------------------------------------------------------
File Strcuture

keep the following files in same folder

-main.cpp
-harris_corner.cpp
-aug_real.cpp
-util.cpp
-util.h
-teapot.obj

----------------------------------------------------------------------------------------
three programs for diffrent tasks (change if values to run diffrent codes)

./main (for camera calibration) (set pre processor directives 0 or 1)
./harris_corner (for harris corner detection) (set pre processor directives if 0 or 1)
./aug_real (for display virtual object) (set pre processor directives if 0 or 1)

---------------------------------------------------------------------------------------

for main code

press 's' save current frame for calibration (min 5 times)
press 'c' which will give calibration matrix and save the parametrs values to csv file

--------------------------------------------------------------------------------------

for aug_real (run maincode first and calibrate)

press 'a' to display 3D axes (By default)
press 'w' to display virtual object

--------------------------------------------------------------------------------------

for harris_corner

by default it will show harris corners only.

press 's' to save image
