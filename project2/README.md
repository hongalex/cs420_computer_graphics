Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: < Alexander Hong >
USC ID 		: < 6404600535 >

Description: In this assignment, we use Catmull-Rom splines along with OpenGL texture mapping to create a roller coaster simulation.

Core Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Uses OpenGL core profile, version 3.2 or higher - Y

2. Completed all Levels:
  Level 1 : - Y
  level 2 : - Y
  Level 3 : - Y
  Level 4 : - Y
  Level 5 : - Y

3. Used Catmull-Rom Splines to render the Track - Y
 
4. Rendered a Rail Cross Section - Y

5. Rendered the camera at a reasonable speed in a continuous path/orientation - Y

6. Run at interactive frame rate (>15fps at 1280 x 720) - Y

7. Understandably written, well commented code - Y

8. Attached an Animation folder containing not more than 1000 screenshots - Y inside hw2-starter-code/screenshots

9. Attached this ReadMe File - Y inside main directory

Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section - N

2. Render a Double Rail - Y

3. Made the track circular and closed it with C1 continuity - N

4. Added OpenGl lighting - N

5. Any Additional Scene Elements? (list them here) N

6. Generate track from several sequences of splines - N

7. Draw splines using recursive subdivision - N

8. Modify velocity with which the camera moves - N

9. Create tracks that mimic a real world coaster - N

10. Render environment in a better manner - N

Additional Features: (Please document any additional features you may have implemented other than the ones described above)
1. 
2.

Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)
1.
2.

Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)
3. N increases the movement speed
4. M decreases the movement speed
5. Main mouse rotates the world left to right when not in roller coaster

Names of the .cpp files you made changes to:
1. hw2.cpp
2.

Comments : (If any)
1. Currently I am using the goodRide.sp in track.txt

2. Up vectors are calculated using the method described in the assignment website, that is calculating normals and binormals

3. However, some parts of the ride may appear tilted due to the nature of the curve. I have confirmed with other students that have had a similar issue.

4. To compile, simply use "make" and then "./hw2 track.txt"



