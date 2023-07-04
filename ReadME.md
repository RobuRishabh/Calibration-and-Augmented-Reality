# Calibration_and_Augmented_Reality
In this project our task is to develop a program that can calibrate a camera and use the calibration data to generate virtual objects in a scene. The camera calibration involves accurately determining the internal parameters of the camera, such as focal length and distortion coefficients, by capturing images of a known pattern, For our project we will use checkerboard image for calibration. Once the camera is calibrated, the program should be able to detect the target (checkerboard) in real-time using the camera feed.

Using the calibrated camera, the program should then be able to place virtual objects in the scene relative to the detected target. These virtual objects should align and orient themselves correctly with respect to the target, taking into account any motion of the camera or the target itself.

Overall, the goal is to create a system that can accurately track the target and overlay virtual objects seamlessly into the real-world scene, accounting for any changes in camera or target position.

## Camera calibration With OpenCV 
For the distortion OpenCV takes into account the radial and tangential factors.
- The presence of the radial distortion manifests in form of the "barrel" or "fish-eye" effect.
- Tangential distortion occurs because the image taking lenses are not perfectly parallel to the imaging plane.
So we have 5 distortion parameters which in OpenCV are presented as one row matrix with 5 columns:
```
	distortion_coefficients=[k1 k2 p1 p2 k3]
```
Now for the unit conversion we use the following formula:

```
	[x]   [fx  0   cx] [X]
	[y] = [0   fy  cy] [Y]
	[w]   [0   0    1] [Z]
```
Here the presence of w is explained by the use of homography coordinate system (and w=Z). The unknown parameters are fx and fy (camera focal lengths) and (cx,cy) which are the optical centers expressed in pixels coordinates. If for both axes a common focal length is used with a given aspect ratio (usually 1), then fy=fx∗a and in the upper formula we will have a single focal length f. The matrix containing these four parameters is referred to as the camera matrix. While the distortion coefficients are the same regardless of the camera resolutions used, these should be scaled along with the current resolution from the calibrated resolution.

In order to calibrate the camera, you will need to take pictures of the checkerboard pattern from different angles using your camera. The goal is for the camera to "find" the pattern in these pictures automatically using OpenCV. Each time the pattern is found in a picture, it provides valuable information that can be used to calculate the camera's internal parameters.

To accurately calculate these parameters, you need a certain number of pictures of the pattern taken from different positions. The exact number depends on the type of pattern used. For the chessboard pattern, you would typically need at least 10 good pictures taken from various angles. This is because the input images often contain some noise or imperfections, and having multiple pictures helps to get more accurate results.

By collecting enough pictures of the pattern and allowing OpenCV to detect and analyze them, you can solve a set of equations that will give you the necessary camera calibration data. This data includes important information about the camera's focal length and distortion coefficients, which are essential for accurately placing virtual objects in the scene.

## Task - 1 : Detect and Extract Chessboard Corners
Before moving on to the camera calibration process. Our task is to develop a system that can detect the target (checkerboard) in the video and extract the corners of the checkerboard accurately.

To achieve this, we can use the functions provided by OpenCV: **findChessboardCorners**, **cornerSubPix**, and **drawChessboardCorners**

- findChessboardCorners() : It helps in locating the corners of the checkerboard pattern in the image or video frame. It returns a set of detected corners. These corners can be stored in a std::vector called corner_set, where each entry represents a corner and can be accessed using corner_set[i].x and corner_set[i].y to get the coordinates of the corner.
```
bool cv::findChessboardCorners(InputArray image, Size patternSize, 
				OutputArray corners, int flags = CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE);
```
	
Once the corners are found, we can use the 
- drawChessboardCorners(): It visually marks the detected corners on the image or video frame. This will help in verifying the accuracy of the corner detection.

- cornerSubPix(): It refines the corner positions for better accuracy. This function further refines the detected corners to sub-pixel accuracy, improving the precision of the corner coordinates.
```
void cv::cornerSubPix(InputArray image, InputOutputArray corners, 
			Size winSize, Size zeroZone, TermCriteria criteria); 	
```
In order to test the system, you can display the image or video with the detected corners marked and also print out the number of corners found along with the coordinates of the first corner. Typically, the first corner will be located in the upper-left of the checkerboard as seen in the image or video.

## Task - 2 : Select Calibration Images
In this task, we want to allow the user to choose a specific image for calibration. We will ask the user to press the 's' key when they want to save the corners found in the last successful attempt to find a chessboard pattern in the image.

First, we need to define some variables to store the corner locations and the corresponding 3D world points. We will use the following definitions:
- corner_list: A list that stores the corner locations for each image where a chessboard pattern is found.
- point_list: A list that stores the 3D world points corresponding to the corner locations.
- point_set: A set of 3D positions that will remain the same regardless of the chessboard orientation.

When the user presses 's', we will save the corner locations from the last successful attempt into the corner_list. We will also create a point_set that specifies the 3D position of the corners in world coordinates. The point_set will have the same number of points as the corner_list.

- To create the point_set, we can either measure the size of a checkerboard square in millimeters and assign specific coordinates, or we can use a simpler approach where we measure the world in terms of checkerboard squares. For example, we can assign (0, 0, 0) to the upper left corner and (1, 0, 0) to the next corner on the same row. If the Z-axis comes towards the viewer, the first point on the next row will be (0, -1, 0).

It's important to note that the number of points in the point_set should match the number of corners in the corner_set. Similarly, the number of point_sets in the point_list should match the number of corner_sets in the corner_list.

Lastly, we can store the images themselves that are being used for calibration. It would be helpful to highlight the chessboard corners in these images for better visualization.

Remember to include a calibration image with highlighted chessboard corners in your project report.

## Task - 3 : Calibrate the Camera
In this task, we want to allow the user to perform camera calibration using a set of calibration images. The user should select at least 5 calibration frames before running the calibration process.

To perform the calibration, we will use the * **cv::calibrateCamera()** from OpenCV. This function requires several parameters, including point_list and corner_list that we have already generated. Additionally, we need to provide the size of the calibration images, the camera matrix, distortion coefficients, rotations, and translations.

Before running the calibration, we will initialize the camera matrix as a 3x3 matrix using the CV_64FC1 data type. We can set the initial values of the matrix to [1, 0, frame.cols/2], [0, 1, frame.rows/2], [0, 0, 1]. These values assume that the pixels are square.

After performing the calibration, we will print out the camera matrix, distortion coefficients, and the final reprojection error. The camera matrix should have equal focal lengths, and the values for u0 and v0 should be close to the center of the image. The reprojection error should ideally be less than half a pixel, but for larger images or cell phone cameras, it may be around 2-3 pixels.

The * **cv::calibrateCamera()** also returns the rotations and translations associated with each calibration image. If you have saved the calibration images, you might want to save these rotations and translations along with them.

Additionally, we will enable the user to write out the intrinsic parameters (camera matrix and distortion coefficients) to a file.

## Task - 4 : Calculate Current Position of the Camera
In this task, we will write a program that reads camera calibration parameters from a file and then starts a video loop. For each frame captured by the camera, the program will attempt to detect a chessboard pattern. If the pattern is found, it will determine the locations of the corners and then use the solvePNP function to calculate the pose of the chessboard (rotation and translation) in real time.

To accomplish this, the program will first load the camera calibration parameters from a file. These parameters include the camera matrix and distortion coefficients that were previously obtained during the camera calibration process.

Next, the program will enter a video loop where it continuously captures frames from the camera. For each frame, it will try to detect the chessboard pattern using the * **findChessboardCorners()**

If the chessboard pattern is successfully detected, the program will retrieve the corner locations. Then, using the * **solvePNP()**, it will calculate the pose of the chessboard by estimating its rotation and translation in relation to the camera.

During the video loop, as the program detects the chessboard pattern and calculates its pose, it will print out the rotation and translation data in real time. This will allow you to observe and verify the accuracy of the pose estimation.

## Task - 5 : Project Outside Corners or 3D Axes
In this task, we will enhance our program to project the 3D points corresponding to the four outside corners of the chessboard onto the image plane in real time. This projection will be done using the * **projectPoints()**.

As the chessboard or camera moves, the program will continuously estimate the pose of the chessboard. With the estimated pose, we can project the 3D points of the four outside corners onto the image plane, allowing us to visualize their positions in the image.

Alternatively, instead of projecting the outside corners, we can attach 3D axes to the chessboard's origin. This will provide a visual representation of the three spatial axes (X, Y, and Z) aligned with the chessboard.

During the execution of the program, as the chessboard or camera moves, the projected 3D points or the 3D axes will be displayed in real time on the image. This will enable us to observe and analyze the spatial relationships between the chessboard and the camera.

## Task - 6 : Create a Virtual Object
In this task, we will create a virtual 3D object made out of lines and project it onto the image captured by the camera. As the camera moves around, the virtual object will maintain its correct orientation relative to the camera.

To achieve this, we will construct a 3D object in the world space using lines. The object can be more complex than a simple cube and can have an asymmetrical shape to aid in debugging and testing.

As the camera moves, we will continuously estimate its pose. Using the estimated pose, we will project the 3D coordinates of the lines of the virtual object onto the image plane. This projection will allow us to draw the lines of the virtual object in the image, giving the illusion that the virtual object is present in the real world.

It's important to ensure that the virtual object maintains its correct orientation relative to the camera as it moves. This will provide a realistic and visually accurate representation of the virtual object in the captured images.

By implementing this task, we can visualize and analyze how the virtual object aligns with the real-world scene, allowing us to assess the accuracy and effectiveness of our program.

## Task - 7 : Detect Robust Features
In this task, we will select a feature detection method, such as Harris corners or SURF features, and develop a separate program to detect and visualize these features in a video stream. We will create a pattern of our choice and display the detected feature points on that pattern.

The program will continuously analyze the frames from the video stream, identifying the selected features in each frame. The detected feature points will be highlighted or marked on the pattern to indicate their locations.

By accurately detecting and tracking these features in real time, we can establish correspondences between the real-world environment and virtual objects. This enables us to overlay virtual content onto the video stream, creating an augmented reality experience.

The detected feature points act as reference points that can be used for tracking and aligning virtual objects within the captured images. This alignment allows us to place virtual objects in the scene in a visually consistent and accurate manner.

## Summary 

- Command-Line Arguments: The program expects two command-line arguments: <target> and <obj_name>. 
	<target> should be either "chessboard" or "circlesgrid," indicating the type of target pattern. 
	<obj_name> refers to the name of the 3D object file located in the "objs" folder.

- Blob Detector Initialization: The program initializes a * **cv::SimpleBlobDetector** using predefined parameters (params). This detector is used for detecting circular grid corners in case of the "circlesgrid" target.

- Target Setup: Based on the <target> value, the program sets the dimensions of the target pattern (points_per_row and points_per_column). For the "chessboard" target, it sets points_per_row to 9 and points_per_column to 6. For the "circlesgrid" target, it sets points_per_row to 4 and points_per_column to 11.

- Object Loading: The program reads the 3D object file (<obj_name>.obj) corresponding to the target pattern. It populates the vertices and faces vectors with the object's vertex coordinates and face indices.

- Video Capture Setup: The program initializes a video capture device (capdev) to read frames from the camera. It also obtains the width and height of the video frames (refS) for further processing.

- Camera Calibration Parameters: The program reads camera intrinsic parameters (camera_matrix and distortion_coefficients) from a CSV file based on the target value. These parameters are used in the camera calibration process.

- Frame Processing Loop: The program enters a loop to process each frame from the video stream. The following steps are performed for each frame:

    a. Frame Preprocessing: The frame is converted to grayscale for corner detection.

    b. Corner Detection: Using OpenCV's findChessboardCorners or findCirclesGrid functions, corners are detected in the target pattern based on the selected target type. Detected corners are refined using cornerSubPix for better accuracy.

    c. 3D Point Generation: Depending on the target type, a set of 3D points (point_set) is generated to represent the target pattern's corners. The z-coordinate is set to 0 since the pattern is assumed to lie on a flat surface.

    d. Pose Estimation: The program calls solvePnP to estimate the pose (rotation and translation) of the object in the scene using the 2D-3D correspondences (corner positions and 3D points). Camera intrinsic parameters and distortion coefficients are provided as inputs.

    e. Visualization: The program visualizes the detected corners on the frame and draws coordinate axes representing the object's pose using the drawChessboardCorners and draw_axes functions.

    f. User Interaction: The program waits for keystrokes from the user. Pressing 'q' quits the program, 'a' shows only the axes, 'w' shows both axes and the 3D object. Pressing 's' saves the current frame as an image.

- Cleanup and Exit: Once the loop ends, the program releases the video capture device and terminates.




















