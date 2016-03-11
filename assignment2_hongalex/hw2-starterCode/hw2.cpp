/*
  CSCI 420 Computer Graphics, USC
  Assignment 2: Roller Coaster
  C++ starter code

  Student username: <hongalex>
*/
#include <iostream>
#include <cstring>
#include <sstream>
#include <String>

#include "openGLHeader.h"
#include "glutHeader.h"

#include <stdio.h>
#include <stdlib.h>
#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#ifdef WIN32
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

// represents one control point along the spline 
struct Point 
{
  double x;
  double y;
  double z;
};

// spline struct 
// contains how many control points the spline has, and an array of control points 
struct Spline 
{
  int numControlPoints;
  Point * points;
};

// the spline array 
Spline * splines;
// total number of splines 
int numSplines;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

OpenGLMatrix* matrix;

//Texture images for skybox and ground 
GLuint skyTexHandle;
GLuint groundTexHandle;

GLuint buffer;
GLuint program;;
GLuint vao;

GLuint cubeBuffer;
GLuint cubeVAO;


BasicPipelineProgram * pipelineProgram;

int imageHeight;
int imageWidth;

float* positions;
float* colors;

int numberOfVertices;
int positionSize, colorSize;

GLfloat theta[3] = {0.0,0.0,0.0};
GLfloat delta = 2.0;
GLint axis = 2;

bool stop = false;
float factor = 1.0f;

int screenshotNum = 1;

static const GLfloat g_vertex_buffer_data[] = {
      -50.0f,-50.0f,-50.0f, // triangle 50 : begin
      -50.0f,-50.0f, 50.0f,
      -50.0f, 50.0f, 50.0f, // triangle 50 : end
      50.0f, 50.0f,-50.0f, // triangle 2 : begin
      -50.0f,-50.0f,-50.0f,
      -50.0f, 50.0f,-50.0f, // triangle 2 : end
      50.0f,-50.0f, 50.0f,
       -50.0f,-50.0f,-50.0f,
       50.0f,-50.0f,-50.0f,
       50.0f, 50.0f,-50.0f,
       50.0f,-50.0f,-50.0f,
       -50.0f,-50.0f,-50.0f,
       -50.0f,-50.0f,-50.0f,
       -50.0f, 50.0f, 50.0f,
       -50.0f, 50.0f,-50.0f,
       50.0f,-50.0f, 50.0f,
       -50.0f,-50.0f, 50.0f,
       -50.0f,-50.0f,-50.0f,
       -50.0f, 50.0f, 50.0f,
       -50.0f,-50.0f, 50.0f,
       50.0f,-50.0f, 50.0f,
       50.0f, 50.0f, 50.0f,
       50.0f,-50.0f,-50.0f,
       50.0f, 50.0f,-50.0f,
       50.0f,-50.0f,-50.0f,
       50.0f, 50.0f, 50.0f,
       50.0f,-50.0f, 50.0f,
       50.0f, 50.0f, 50.0f,
       50.0f, 50.0f,-50.0f,
       -50.0f, 50.0f,-50.0f,
       50.0f, 50.0f, 50.0f,
       -50.0f, 50.0f,-50.0f,
       -50.0f, 50.0f, 50.0f,
       50.0f, 50.0f, 50.0f,
       -50.0f, 50.0f, 50.0f,
       50.0f,-50.0f, 50.0f
};

 // One color for each vertex. They were generated randomly.
/*static const GLfloat g_color_buffer_data[] = {
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
};*/

 static const GLfloat g_color_buffer_data[] = {
      0.583f,  0.771f,  0.014f, 1.0f,
      0.609f,  0.115f,  0.436f, 1.0f,
      0.327f,  0.483f,  0.844f, 1.0f,
      0.822f,  0.569f,  0.201f, 1.0f,
      0.435f,  0.602f,  0.223f, 1.0f,
      0.310f,  0.747f,  0.185f, 1.0f,
      0.597f,  0.770f,  0.761f, 1.0f,
     0.559f,  0.436f,  0.730f, 1.0f,
     0.359f,  0.583f,  0.152f, 1.0f,
     0.483f,  0.596f,  0.789f, 1.0f,
     0.559f,  0.861f,  0.639f, 1.0f,
     0.195f,  0.548f,  0.859f, 1.0f,
     0.014f,  0.184f,  0.576f, 1.0f,
     0.771f,  0.328f,  0.970f, 1.0f,
     0.406f,  0.615f,  0.116f, 1.0f,
     0.676f,  0.977f,  0.133f, 1.0f,
     0.971f,  0.572f,  0.833f, 1.0f,
     0.140f,  0.616f,  0.489f, 1.0f,
     0.997f,  0.513f,  0.064f, 1.0f,
     0.945f,  0.719f,  0.592f, 1.0f,
     0.543f,  0.021f,  0.978f, 1.0f,
     0.279f,  0.317f,  0.505f, 1.0f,
     0.167f,  0.620f,  0.077f, 1.0f,
     0.347f,  0.857f,  0.137f, 1.0f,
     0.055f,  0.953f,  0.042f, 1.0f,
     0.714f,  0.505f,  0.345f, 1.0f,
     0.783f,  0.290f,  0.734f, 1.0f,
     0.722f,  0.645f,  0.174f, 1.0f,
     0.302f,  0.455f,  0.848f, 1.0f,
     0.225f,  0.587f,  0.040f, 1.0f,
     0.517f,  0.713f,  0.338f, 1.0f,
     0.053f,  0.959f,  0.120f, 1.0f,
     0.393f,  0.621f,  0.362f, 1.0f,
     0.673f,  0.211f,  0.457f, 1.0f,
     0.820f,  0.883f,  0.371f, 1.0f,
     0.982f,  0.099f,  0.879f, 1.0f
};


//Utility functions 
string StringToInt(int i) {
  ostringstream oss;
  oss << i;
  return oss.str();
}

float cube(float x) {
	return x*x*x;
}

float square(float x) {
	return x*x;
}
//End Utility Functions

int loadSplines(char * argv) 
{
  char * cName = (char *) malloc(128 * sizeof(char));
  FILE * fileList;
  FILE * fileSpline;
  int iType, i = 0, j, iLength;

  // load the track file 
  fileList = fopen(argv, "r");
  if (fileList == NULL) 
  {
    printf ("can't open file\n");
    exit(1);
  }
  
  // stores the number of splines in a global variable 
  fscanf(fileList, "%d", &numSplines);

  splines = (Spline*) malloc(numSplines * sizeof(Spline));

  // reads through the spline files 
  for (j = 0; j < numSplines; j++) 
  {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) 
    {
      printf ("can't open file\n");
      exit(1);
    }
      
    // gets length for spline file
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    // allocate memory for all the points
    splines[j].points = (Point *)malloc(iLength * sizeof(Point));
    splines[j].numControlPoints = iLength;

    // saves the data to the struct
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &splines[j].points[i].x, 
	   &splines[j].points[i].y, 
	   &splines[j].points[i].z) != EOF) 
    {
      i++;
    }
  }

  free(cName);

  return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle)
{
  // read the texture image
  ImageIO img;
  ImageIO::fileFormatType imgFormat;
  ImageIO::errorType err = img.load(imageFilename, &imgFormat);

  if (err != ImageIO::OK) 
  {
    printf("Loading texture from %s failed.\n", imageFilename);
    return -1;
  }

  // check that the number of bytes is a multiple of 4
  if (img.getWidth() * img.getBytesPerPixel() % 4) 
  {
    printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
    return -1;
  }

  // allocate space for an array of pixels
  int width = img.getWidth();
  int height = img.getHeight();
  unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++)
    for (int w = 0; w < width; w++) 
    {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
      pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

      // set the RGBA channels, based on the loaded image
      int numChannels = img.getBytesPerPixel();
      for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
        pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
    }

  // bind the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  // initialize the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

  // generate the mipmaps for this texture
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // query support for anisotropic texture filtering
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  printf("Max available anisotropic samples: %f\n", fLargest);
  // set anisotropic texture filtering
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

  // query for any errors
  GLenum errCode = glGetError();
  if (errCode != 0) 
  {
    printf("Texture initialization error. Error code: %d.\n", errCode);
    return -1;
  }
  
  // de-allocate the pixel array -- it is no longer needed
  delete [] pixelsRGBA;

  return 0;
}


// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}


void renderImage() {
  glBindVertexArray(vao);
  GLint first = 0;
  GLsizei count = numberOfVertices;
  glDrawArrays(GL_LINE_STRIP,first,count);

  //CUBE RENDERING
  glBindVertexArray(cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares*/

  //unbind the VAOs
  glBindVertexArray(0);

}

void displayFunc()
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
  matrix->LoadIdentity();
  matrix->LookAt(0, 0, 0, 1, 0, 0, 0, 0, 1); // default camera
  //matrix->LookAt(0,100,200,0,0,0,0,1,0);
  //matrix->Translate(-128,0,120);

  //do some scaling
  matrix->Scale(landScale[0],landScale[1],landScale[2]);

  //do rotations using landRotate
  matrix->Rotate(landRotate[0]/2.0,1.0,0.0,0.0);  
  matrix->Rotate(landRotate[1]/2.0,0.0,1.0,0.0);
  matrix->Rotate(landRotate[2]/2.0,0.0,0.0,1.0);

  //do translation using landTranslate 
  matrix->Translate(landTranslate[0],landTranslate[1],landTranslate[2]);
  //matrix->Translate(0.0,landTranslate[1],0.0);
  //matrix->Translate(0.0,0.0,landTranslate[2]);



  //pipelineProgram->Bind(); optional

  //Uploads information to openGL
  GLint h_modelViewMatrix=glGetUniformLocation(program,"modelViewMatrix");
  float m[16];
  matrix->GetMatrix(m);
  GLboolean isRowMajor = GL_FALSE;
  glUniformMatrix4fv(h_modelViewMatrix,1,isRowMajor,m);

  //Change to projection mode
  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  GLint h_projectionMatrix = glGetUniformLocation(program,"projectionMatrix");
  float p[16];
  matrix->GetMatrix(p);
  glUniformMatrix4fv(h_projectionMatrix,1,isRowMajor,p);

  renderImage();

  //swap frame buffers
  glutSwapBuffers();

}


void idleFunc()
{
  // display result (do not forget this!)
  glutPostRedisplay(); 

}

void reshapeFunc(int w, int h)
{
  GLfloat aspect = (GLfloat)w/(GLfloat)h;
  glViewport(0, 0, w, h);
  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  matrix->LoadIdentity();
  //matrix->Ortho(-2.0,2.0,-2.0/aspect,2.0/aspect,0.0,10.0);
  matrix->Perspective(60.0,aspect,0.01,1000.0);
  matrix->SetMatrixMode(OpenGLMatrix::ModelView);

  // setup perspective matrix...
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[1] -= mousePosDelta[1];
        landRotate[2] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[0] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
      if (key == ' ')
        stop = !stop;
      if (stop)
        glutIdleFunc(NULL);
      else
        glutIdleFunc(idleFunc); 
    break;

    case 'q':
      landScale[0]*=0.75;
      landScale[1]*=0.75;
      landScale[2]*=0.75;

      break;

    case 'e':
      landScale[0]/=0.75;
      landScale[1]/=0.75;
      landScale[2]/=0.75;
      break;

    case 'w':
      landTranslate[1]-=factor;
      break;

    case 'a':
      landTranslate[0]+=factor;
      break;

    case 's':
      landTranslate[1]+=factor;
      break;

    case 'd':
      landTranslate[0]-=factor;
      break;

    case 'z':
      landTranslate[2]+=factor;
      break;

    case 'x':
      landTranslate[2]-=factor;
      break;

    case 't':
      factor*=2;
      break;

    case 'y':
      factor/=2;
      break;

    case 'c':
      // take a screenshot
      string filename = "screenshots/";
      if(screenshotNum<10) {
        filename+="00"+StringToInt(screenshotNum)+".jpg";
        saveScreenshot(filename.c_str());
      }
      else if(screenshotNum < 100) {
        filename+="0"+StringToInt(screenshotNum)+".jpg";
        saveScreenshot(filename.c_str());
      } else {
        filename+=StringToInt(screenshotNum)+".jpg";
        saveScreenshot(filename.c_str());
      }

      screenshotNum++;
    break;
  }
}

//fill data with Catmull-Rom Spline formula
void fillSplineData(float u, float s) {
	numberOfVertices=0;

	//calculate how many vertices need to be rendered
	for(int i=0; i < numSplines; i++) {
		for(int j=0; j < (splines[i].numControlPoints)-3; j++) {
			for(float k=0.0; k<=1.0; k+=u) {
				numberOfVertices++;
			}
		}
	}

	positions = new float[numberOfVertices*3];
	colors = new float[numberOfVertices*4];
	positionSize = numberOfVertices*3*sizeof(float); 
	colorSize = numberOfVertices*4*sizeof(float);


	int index = 0;
	//fill positions array
	for(int i=0; i < numSplines; i++) {
		for(int j=0; j < (splines[i].numControlPoints)-3; j++) {
			for(float k=0.0; k<=1.0; k+=u) {

				//Perform initial calculations
				float u2 = square(k);
				float u3 = cube(k);

				//t0 to t3 represent temporary values from multiplying [u3 u2 u1 1][M] (u and basis matrix);
				float t0 = u3*-1*s + u2*2*s - k*s;
				float t1 = u3*(2-s) + u2*(s-3)+1;
				float t2 = u3*(s-2) + u2*(3-2*s) + k*s;
				float t3 = u3*s - u2*s;

				float x0 = splines[i].points[j].x;
				float x1 = splines[i].points[j+1].x;
				float x2 = splines[i].points[j+2].x;
				float x3 = splines[i].points[j+3].x;

				float y0 = splines[i].points[j].y;
				float y1 = splines[i].points[j+1].y;
				float y2 = splines[i].points[j+2].y;
				float y3 = splines[i].points[j+3].y;

				float z0 = splines[i].points[j].z;
				float z1 = splines[i].points[j+1].z;
				float z2 = splines[i].points[j+2].z;
				float z3 = splines[i].points[j+3].z;

				positions[index] = t0*x0 + t1*x1 + t2*x2 + t3*x3;
				index++; 
				positions[index] = t0*y0 + t1*y1 + t2*y2 + t3*y3;
				index++;
				positions[index] = t0*z0 + t1*z1 + t2*z2 + t3*z3;
				index++;
			}
		}
	}

	for(int i=0; i<numberOfVertices*4; i++) {
		colors[i] = 1.0;
	}

}

void initVBO() {
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, positionSize + colorSize,
     NULL, GL_STATIC_DRAW); // init buffer’s size, but don’t assign any
                          // data to it

  // upload position data
  int loc = 0;
  glBufferSubData(GL_ARRAY_BUFFER, loc, positionSize, positions);

  // upload color data
  loc = positionSize;
  glBufferSubData(GL_ARRAY_BUFFER, loc, colorSize, colors);



}

void initPipelineProgram() {
  pipelineProgram = new BasicPipelineProgram();
  pipelineProgram->Init("../openGLHelper-starterCode");
  program = pipelineProgram->GetProgramHandle();
  pipelineProgram->Bind();
}

void initVAO() {

  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);

  // get location index of the “position” shader variable
  GLuint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc); // enable the “position” attribute
  const void * offset = 0; GLsizei stride = 0;
  GLboolean normalized = GL_FALSE;
  // set the layout of the “position” attribute data
  glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

  // get the location index of the “color” shader variable
  loc = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(loc); // enable the “color” attribute
  offset = (void *)(uintptr_t)positionSize; 
  // set the layout of the “color” attribute data
  glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);
  glBindVertexArray(0); // unbind the VAO
}

void initCube() {
  glGenBuffers(1, &cubeBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, cubeBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data) + sizeof(g_color_buffer_data), NULL, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), sizeof(g_color_buffer_data), g_color_buffer_data);

  program = pipelineProgram->GetProgramHandle();
  pipelineProgram->Bind();

  glGenVertexArrays(1,&cubeVAO);
  glBindVertexArray(cubeVAO);

  // get location index of the “position” shader variable
  GLuint loc = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(loc); // enable the “position” attribute
  const void * offset = 0; GLsizei stride = 0;
  GLboolean normalized = GL_FALSE;
  // set the layout of the “position” attribute data
  glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

  // get the location index of the “color” shader variable
  loc = glGetAttribLocation(program, "color");
  glEnableVertexAttribArray(loc); // enable the “color” attribute
  offset = (void *)sizeof(g_vertex_buffer_data); 
  // set the layout of the “color” attribute data
  glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);
  glBindVertexArray(0); // unbind the VAO
}

void initScene(int argc, char *argv[])
{
  // load the splines from the provided filename
  loadSplines(argv[1]);

  printf("Loaded %d spline(s).\n", numSplines);
  for(int i=0; i<numSplines; i++)
    printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);

   // load the image from a jpeg disk file to main memory
  initTexture("textures/wood.jpg",groundTexHandle);
  initTexture("textures/sky.jpg",skyTexHandle);


  glClearColor(0.0f, 0.0f,0.0f, 0.0f);

  // do additional initialization here...
  glEnable(GL_DEPTH_TEST);
  matrix = new OpenGLMatrix();

  //call with u = 0.001 and s = 0.5 
  fillSplineData(0.001, 0.5);


  initVBO();
  initPipelineProgram();
  initVAO();

  initCube();

  //initTextureVBO();
  //initTextureVAO();

}

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
    printf ("usage: %s <trackfile>\n", argv[0]);
    exit(0);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();

  return 0;
}

