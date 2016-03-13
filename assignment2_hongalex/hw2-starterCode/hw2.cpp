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
#include <vector>
#include <cmath>

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

struct Vector3 {
  float x;
  float y;
  float z;
};

struct Vector4 {
  float r;
  float g;
  float b;
  float a;
};

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
GLuint vao;

GLuint cubeBuffer;
GLuint cubeVAO;

BasicPipelineProgram * pipelineProgram;
GLuint program;

BasicPipelineProgram* cubePipelineProgram;
GLuint cubeProgram;

int imageHeight;
int imageWidth;

Vector3* positions;
Vector3* points;
Vector4* colors;

//Unit vectors at each point
Vector3* tangents;
Vector3* normals;
Vector3* binormals;


int numberOfVertices;
int positionSize, colorSize, pointsSize;

GLfloat theta[3] = {0.0,0.0,0.0};
GLfloat delta = 2.0;
GLint axis = 2;

bool stop = false;
float factor = 1.0f;

int screenshotNum = 1;

vector<float> cube_pos;
vector<float> cube_uvs;

vector<Vector3> cubeData;

//Min and max height of the skybox + ground texture
float maxHeight = 100.0f;
float minHeight = -1.0f;

int currentPosition = 0; //position of the roler coaster we are at 

float alpha = 0.05f; //alpha controls the cross section width and height
float speed = 1500; //roller coaster speed

/*===================Utility functions==================*/
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

/**
 * Calculates the cross product of two vectors
 *
 * @param first- The first vector to cross
 * @param second-The second vector to cross 
 *
 *
 * @return Vector3-  the result of the cross product 
 */
Vector3 crossProduct(Vector3 first, Vector3 second) {
  Vector3* result = new Vector3();
  result->x = first.y * second.z - first.z * second.y;
  result->y = first.z * second.x - first.x * second.z;
  result->z = first.x * second.y - first.y * second.x;
  return *result;
}

/**
 * Calculates unit vector given any vector
 *
 * @param vec- The vector to normalize
 *
 *
 * @return Vector3- resultant vector
 */
Vector3 normalize(Vector3 vec) {
  Vector3* result = new Vector3();
  float length = sqrt(square(vec.x)+square(vec.y)+square(vec.z));

  result->x = vec.x/length;
  result->y = vec.y/length;
  result->z = vec.z/length;
  return *result;

}


/**
 * Calculates the value of adding two vectors multiplied by alpha
 *
 * @param first- The first vector to add
 * @param second-The second vector to add
 *
 *
 * @return Vector3- resultant vector
 */
Vector3 vectorAdd(Vector3 first, Vector3 second) {
  Vector3* result = new Vector3();

  result->x = (first.x + second.x);
  result->y = (first.y + second.y);
  result->z = (first.z + second.z);
  return *result;
}

/**
 * Calculates the value of a vector multiplied by a scalar
 *
 * @param vec- The vector to multiply by a scalar
 *
 * @return Vector3- resultant vector
 */
Vector3 vectorScalar(Vector3 vec, float scalar) {
  Vector3* result = new Vector3();

  result->x = scalar*vec.x;
  result->y = scalar*vec.y;
  result->z = scalar*vec.z;
  return *result;


}
/*===================End utility functions==============*/

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

void setTextureUnit(GLint unit)
{
 glActiveTexture(unit); // select the active texture unit
 // get a handle to the “textureImage” shader variable
 GLint h_textureImage = glGetUniformLocation(cubeProgram, "textureImage");
 // deem the shader variable “textureImage” to read from texture unit “unit”
 glUniform1i(h_textureImage, unit - GL_TEXTURE0);
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

void displayFunc()
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
  matrix->LoadIdentity();

  //matrix->LookAt(0, 0, 0, 1, 0, 0, 0, 0, 1); // default camera
  Vector3 p0 = positions[currentPosition];
  float ex = vectorAdd(p0, vectorScalar(binormals[currentPosition],-3*alpha)).x; 
  float ey = vectorAdd(p0, vectorScalar(binormals[currentPosition],-3*alpha)).y;
  float ez = vectorAdd(p0, vectorScalar(binormals[currentPosition],-3*alpha)).z;

  float fx = ex + tangents[currentPosition].x;
  float fy = ey + tangents[currentPosition].y;
  float fz = ez + tangents[currentPosition].z;

  float ux = -1*binormals[currentPosition].x;
  float uy = -1*binormals[currentPosition].y;
  float uz = -1*binormals[currentPosition].z;

  matrix->LookAt(ex, ey, ez, fx, fy, fz, ux, uy, uz);
  //matrix->LookAt(0,0,0,1,0,0,0,0,1);

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

  pipelineProgram->Bind(); 

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

  //Draw the main roller coaster
  glBindVertexArray(vao);
  GLint first = 0;
  //36 is 6 faces * 2 triangles * 3 vertices
  GLsizei count = numberOfVertices*36*2;
  glDrawArrays(GL_TRIANGLES,first,count);
  glBindVertexArray(0);

  cubePipelineProgram->Bind();

  //Uploads information to openGL
  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
  h_modelViewMatrix=glGetUniformLocation(cubeProgram,"modelViewMatrix");
  matrix->GetMatrix(m);
  glUniformMatrix4fv(h_modelViewMatrix,1,GL_FALSE,m);

  //Change to projection mode
  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  h_projectionMatrix = glGetUniformLocation(cubeProgram,"projectionMatrix");
  matrix->GetMatrix(p);
  glUniformMatrix4fv(h_projectionMatrix,1,GL_FALSE,p);
  
  setTextureUnit(GL_TEXTURE0);

  //Draw the ground textures as a cube

  glBindTexture(GL_TEXTURE_2D,groundTexHandle);
  glBindVertexArray(cubeVAO);

  glDrawArrays(GL_TRIANGLES,0,6);

  //Draw the sky textures around the cube

  glBindTexture(GL_TEXTURE_2D,skyTexHandle);
  glDrawArrays(GL_TRIANGLES,6,30);

  glBindVertexArray(0);

  //swap frame buffers
  glutSwapBuffers();

}


void idleFunc()
{
  currentPosition+=numberOfVertices/speed;
  if(currentPosition>=numberOfVertices) {
    currentPosition = 0;
  }

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
      if (rightMouseButton)
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
      if (rightMouseButton)
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
      if (rightMouseButton)
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

    case 'p':
      if(currentPosition+numberOfVertices/speed <= numberOfVertices) {
        currentPosition+=numberOfVertices/speed;
      } else {
        currentPosition = 0;
      }
      break;

    case 'o':
      if(currentPosition-numberOfVertices/speed > 0) {
        currentPosition-=numberOfVertices/speed;
      }      
      break;

    case 'm':
      speed/=0.75;
      break;
    case 'n':
      speed*=0.75;
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

void createCubeData(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4, Vector3 v5, Vector3 v6, Vector3 v7) {
  cubeData.push_back(v0); cubeData.push_back(v1); cubeData.push_back(v2); //first triangle
  cubeData.push_back(v2); cubeData.push_back(v3); cubeData.push_back(v0); //second triangle

  cubeData.push_back(v2); cubeData.push_back(v1); cubeData.push_back(v5);
  cubeData.push_back(v5); cubeData.push_back(v6); cubeData.push_back(v2);

  cubeData.push_back(v0); cubeData.push_back(v4); cubeData.push_back(v5);
  cubeData.push_back(v5); cubeData.push_back(v1); cubeData.push_back(v0);

  cubeData.push_back(v3); cubeData.push_back(v7); cubeData.push_back(v6);
  cubeData.push_back(v6); cubeData.push_back(v2); cubeData.push_back(v3);

  cubeData.push_back(v3); cubeData.push_back(v0); cubeData.push_back(v4);
  cubeData.push_back(v4); cubeData.push_back(v7); cubeData.push_back(v3);

  cubeData.push_back(v7); cubeData.push_back(v4); cubeData.push_back(v5);
  cubeData.push_back(v5); cubeData.push_back(v6); cubeData.push_back(v7);


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

	positions = new Vector3[numberOfVertices];
	colors = new Vector4[numberOfVertices*36*2];
	positionSize = numberOfVertices*3*sizeof(float); 
	colorSize = numberOfVertices*36*4*2*sizeof(float);

  tangents = new Vector3[numberOfVertices];
  normals = new Vector3[numberOfVertices];
  binormals = new Vector3[numberOfVertices];


	int index = 0;
	//fill positions array and also calculate for tangents and normals
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


        positions[index] = Vector3(); 
        positions[index].x = t0*x0 + t1*x1 + t2*x2 + t3*x3;
        positions[index].y = t0*y0 + t1*y1 + t2*y2 + t3*y3;
        positions[index].z = t0*z0 + t1*z1 + t2*z2 + t3*z3;

        //temp0 to temp 3 represent temporary values from multiplying [3u^2 2u 1 0][M] derivative of u and the basis matrix
        u3 = 3 * u2;
        u2 = 2 * k;

        float temp0 = u3*-1*s + u2*2*s - s;
        float temp1 = u3*(2-s) + u2*(s-3);
        float temp2 = u3*(s-2) + u2*(3-2*s) + s;
        float temp3 = u3*s - u2*s;

        tangents[index] = Vector3(); 
        tangents[index].x = temp0*x0 + temp1*x1 + temp2*x2 + temp3*x3;
        tangents[index].y = temp0*y0 + temp1*y1 + temp2*y2 + temp3*y3;
        tangents[index].z = temp0*z0 + temp1*z1 + temp2*z2 + temp3*z3;
        tangents[index] = normalize(tangents[index]);

        index++;
			}
		}
	}


  //calculate the normal and binormal vectors
  Vector3 arb = Vector3();
  arb.x = 0.0f; arb.y =0.0f; arb.z = 1.0f;
  normals[0] = normalize(crossProduct(tangents[0],arb));
  binormals[0] = normalize(crossProduct(tangents[0],normals[0]));
  for(int i=1; i < numberOfVertices; i++) {
    normals[i] = normalize(crossProduct(binormals[i-1],tangents[i]));
    binormals[i] = normalize(crossProduct(tangents[i],normals[i]));
  }

  points = new Vector3[numberOfVertices*4];
  pointsSize = 4*numberOfVertices*3*sizeof(float);

  index = 0;
  for(int i=0; i<numberOfVertices; i++) {
    Vector3 pos0 = positions[i];
    Vector3 n0 = normals[i];
    Vector3 b0 = binormals[i];
    Vector3 n0_negative = vectorScalar(n0,-1.0);
    Vector3 b0_negative = vectorScalar(b0,-1.0);


    points[index] = vectorAdd(pos0,vectorScalar(vectorAdd(n0_negative,b0),alpha));
    index++;
    points[index] = vectorAdd(pos0,vectorScalar(vectorAdd(n0,b0),alpha));
    index++;
    points[index] = vectorAdd(pos0,vectorScalar(vectorAdd(n0,b0_negative),alpha));
    index++;
    points[index] = vectorAdd(pos0,vectorScalar(vectorAdd(n0_negative,b0_negative),alpha));
    index++;
  }

  //fill in the color values
  for(int i=0; i<numberOfVertices*36*2; i++) {
    colors[i].r = 0.5f;
    colors[i].g = 0.5f;
    colors[i].b = 0.5f;
    colors[i].a = 0.5f;
  }

  for(int i=0; i<4*(numberOfVertices-1); i+=4) {
    Vector3 p0 = points[i];
    Vector3 p1 = points[i+1];
    Vector3 p2 = points[i+2];
    Vector3 p3 = points[i+3];
    Vector3 p4 = points[i+4];
    Vector3 p5 = points[i+5];
    Vector3 p6 = points[i+6];
    Vector3 p7 = points[i+7];

    createCubeData(p0, p1, p2, p3, p4, p5, p6, p7);

    Vector3 alphaScalar = vectorScalar(normals[i/4],4*alpha);
    p0 = vectorAdd(p0, alphaScalar);
    p1 = vectorAdd(p1, alphaScalar);
    p2 = vectorAdd(p2, alphaScalar);
    p3 = vectorAdd(p3, alphaScalar);
    p4 = vectorAdd(p4, alphaScalar);
    p5 = vectorAdd(p5, alphaScalar);
    p6 = vectorAdd(p6, alphaScalar);
    p7 = vectorAdd(p7, alphaScalar);

    createCubeData(p0, p1, p2, p3, p4, p5, p6, p7);



  }
}

void initVBO() {
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, cubeData.size() * 3* sizeof(float) + colorSize,
     NULL, GL_STATIC_DRAW); // init buffer’s size, but don’t assign any
                          // data to it

  // upload position data
  int loc = 0;
  glBufferSubData(GL_ARRAY_BUFFER, loc, cubeData.size()*3*sizeof(float), cubeData.data());

  // upload color data
  loc = cubeData.size()*3*sizeof(float);
  glBufferSubData(GL_ARRAY_BUFFER, loc, colorSize, colors);

}

void initPipelineProgram() {
  pipelineProgram = new BasicPipelineProgram();
  pipelineProgram->Init("../openGLHelper-starterCode");
  program = pipelineProgram->GetProgramHandle();
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
  offset = (void *)(uintptr_t)(cubeData.size() *3* sizeof(float)); 
  // set the layout of the “color” attribute data
  glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);
  glBindVertexArray(0); // unbind the VAO
}

//Add a triangle to the world texture 
void addTriangle(float posA[3], float posB[3], float posC[3],
 float uvA[2], float uvB[2], float uvC[2]) {
   cube_pos.push_back(posA[0]); cube_pos.push_back(posA[1]); cube_pos.push_back(posA[2]);
   cube_pos.push_back(posB[0]); cube_pos.push_back(posB[1]); cube_pos.push_back(posB[2]);
   cube_pos.push_back(posC[0]); cube_pos.push_back(posC[1]); cube_pos.push_back(posC[2]);
   cube_uvs.push_back(uvA[0]); cube_uvs.push_back(uvA[1]);
   cube_uvs.push_back(uvB[0]); cube_uvs.push_back(uvB[1]);
   cube_uvs.push_back(uvC[0]); cube_uvs.push_back(uvC[1]);
} 

void fillTextureData() {
  //Load the texture data from files first and bind them to a handle
  glGenTextures(1, &groundTexHandle);
  glGenTextures(1, &skyTexHandle);
  initTexture("textures/wood.jpg",groundTexHandle);
  initTexture("textures/sky.jpg",skyTexHandle);

  //Creates the ground 
  float posA0[] = {-maxHeight, -maxHeight, minHeight};
  float posB0[] = {-maxHeight, maxHeight, minHeight};
  float posC0[] = {maxHeight, -maxHeight, minHeight};

  float uvA0[] = {0.0f, 1.0f};
  float uvB0[] = {0.0f, 0.0f};
  float uvC0[] = {1.0f, 1.0f};

  addTriangle(posA0, posB0, posC0, uvA0, uvB0, uvC0);

  float posA1[] = {maxHeight, -maxHeight, minHeight};
  float posB1[] = {maxHeight, maxHeight, minHeight};
  float posC1[] = {-maxHeight, maxHeight, minHeight};

  float uvA1[] = {1.0f, 1.0f};
  float uvB1[] = {1.0f, 0.0f};
  float uvC1[] = {0.0f, 0.0f};

  addTriangle(posA1, posB1, posC1, uvA1, uvB1, uvC1);


  //Creates the other 5 walls of the skybox
  //Left wall
  float posA2[] = {-maxHeight, -maxHeight, minHeight};
  float posB2[] = {-maxHeight, -maxHeight, maxHeight};
  float posC2[] = {-maxHeight, maxHeight, maxHeight};

  float uvA2[] = {1.0f, 0.0f};
  float uvB2[] = {1.0f, 1.0f};
  float uvC2[] = {0.0f, 1.0f};

  addTriangle(posA2, posB2, posC2, uvA2, uvB2, uvC2);

  float posA3[] = {-maxHeight, maxHeight, minHeight};
  float posB3[] = {-maxHeight, -maxHeight, minHeight};
  float posC3[] = {-maxHeight, maxHeight, maxHeight};

  float uvA3[] = {0.0f, 0.0f};
  float uvB3[] = {1.0f, 0.0f};
  float uvC3[] = {0.0f, 1.0f};

  addTriangle(posA3, posB3, posC3, uvA3, uvB3, uvC3);

  //Top wall
  float posA4[] = {-maxHeight, -maxHeight, maxHeight};
  float posB4[] = {-maxHeight, maxHeight, maxHeight};
  float posC4[] = {maxHeight, -maxHeight, maxHeight};

  float uvA4[] = {0.0f, 1.0f};
  float uvB4[] = {0.0f, 0.0f};
  float uvC4[] = {1.0f, 1.0f};

  addTriangle(posA4, posB4, posC4, uvA4, uvB4, uvC4);

  float posA5[] = {maxHeight, -maxHeight, maxHeight};
  float posB5[] = {maxHeight, maxHeight, maxHeight};
  float posC5[] = {-maxHeight, maxHeight, maxHeight};

  float uvA5[] = {1.0f, 1.0f};
  float uvB5[] = {1.0f, 0.0f};
  float uvC5[] = {0.0f, 0.0f};

  addTriangle(posA5, posB5, posC5, uvA5, uvB5, uvC5);

  //Right wall
  float posA6[] = {maxHeight, -maxHeight, minHeight};
  float posB6[] = {maxHeight, -maxHeight, maxHeight};
  float posC6[] = {maxHeight, maxHeight, maxHeight};

  float uvA6[] = {1.0f, 0.0f};
  float uvB6[] = {1.0f, 1.0f};
  float uvC6[] = {0.0f, 1.0f};

  addTriangle(posA6, posB6, posC6, uvA6, uvB6, uvC6);

  float posA7[] = {maxHeight, maxHeight, minHeight};
  float posB7[] = {maxHeight, -maxHeight, minHeight};
  float posC7[] = {maxHeight, maxHeight, maxHeight};

  float uvA7[] = {0.0f, 0.0f};
  float uvB7[] = {1.0f, 0.0f};
  float uvC7[] = {0.0f, 1.0f};

  addTriangle(posA7, posB7, posC7, uvA7, uvB7, uvC7);

  //Back wall
  float posA8[] = {-maxHeight, -maxHeight, maxHeight};
  float posB8[] = {maxHeight, -maxHeight, maxHeight};
  float posC8[] = {-maxHeight, -maxHeight, minHeight};

  float uvA8[] = {0.0f, 1.0f};
  float uvB8[] = {1.0f, 1.0f};
  float uvC8[] = {0.0f, 0.0f};

  addTriangle(posA8, posB8, posC8, uvA8, uvB8, uvC8);

  float posA9[] = {-maxHeight, -maxHeight, minHeight};
  float posB9[] = {maxHeight, -maxHeight, minHeight};
  float posC9[] = {maxHeight, -maxHeight, maxHeight};

  float uvA9[] = {0.0f, 0.0f};
  float uvB9[] = {1.0f, 0.0f};
  float uvC9[] = {1.0f, 1.0f};

  addTriangle(posA9, posB9, posC9, uvA9, uvB9, uvC9);


  //Front wall
  float posA10[] = {-maxHeight, maxHeight, maxHeight};
  float posB10[] = {maxHeight, maxHeight, maxHeight};
  float posC10[] = {-maxHeight, maxHeight, minHeight};

  float uvA10[] = {0.0f, 1.0f};
  float uvB10[] = {1.0f, 1.0f};
  float uvC10[] = {0.0f, 0.0f};

  addTriangle(posA10, posB10, posC10, uvA10, uvB10, uvC10);

  float posA11[] = {-maxHeight, maxHeight, minHeight};
  float posB11[] = {maxHeight, maxHeight, minHeight};
  float posC11[] = {maxHeight, maxHeight, maxHeight};

  float uvA11[] = {0.0f, 0.0f};
  float uvB11[] = {1.0f, 0.0f};
  float uvC11[] = {1.0f, 1.0f};

  addTriangle(posA11, posB11, posC11, uvA11, uvB11, uvC11);


}
//Creates the VBO, initializes the program, and VAO for the sky and ground textures
void initCube() {

  glGenBuffers(1, &cubeBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, cubeBuffer);
  glBufferData(GL_ARRAY_BUFFER, (cube_pos.size() + cube_uvs.size())*sizeof(float), NULL, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, 0, cube_pos.size() * sizeof(float), cube_pos.data());
  
  // upload uv data
  glBufferSubData(GL_ARRAY_BUFFER, cube_pos.size() * sizeof(float), cube_uvs.size() * sizeof(float), cube_uvs.data()); 


  cubePipelineProgram = new BasicPipelineProgram();
  cubePipelineProgram->BuildShadersFromFiles("../openGLHelper-starterCode", "texture.vertexShader.glsl", "texture.fragmentShader.glsl");
  cout << "Successfully built the texture pipeline program." << endl;

  cubeProgram = cubePipelineProgram->GetProgramHandle();
  cubePipelineProgram->Bind();

  glGenVertexArrays(1,&cubeVAO);
  glBindVertexArray(cubeVAO);

  // get location index of the “position” shader variable
  GLuint loc = glGetAttribLocation(cubeProgram, "position");
  glEnableVertexAttribArray(loc); // enable the “position” attribute
  const void * offset = 0; GLsizei stride = 0;
  GLboolean normalized = GL_FALSE;
  // set the layout of the “position” attribute data
  glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

  loc = glGetAttribLocation(cubeProgram, "texCoord");
  glEnableVertexAttribArray(loc); // enable the "texCoord" attribute
  offset = (void *)(cube_pos.size()*sizeof(float)); 
  // set the layout of the "texCoord" attribute data
  glVertexAttribPointer(loc, 2, GL_FLOAT, normalized, stride, offset);

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


  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  // do additional initialization here...
  glEnable(GL_DEPTH_TEST);
  matrix = new OpenGLMatrix();

  //call with u = 0.001 and s = 0.5 
  fillSplineData(0.001, 0.5);

  initVBO();
  initPipelineProgram();
  initVAO();

  fillTextureData();
  initCube();

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

