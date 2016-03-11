/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: <type your USC username here>
*/

#include <iostream>
#include <cstring>
#include <sstream>
#include "openGLHeader.h"
#include "glutHeader.h"

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
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

OpenGLMatrix* matrix;

GLuint buffer;
GLuint program;;
GLuint vao;

BasicPipelineProgram * pipelineProgram;

int imageHeight;
int imageWidth;

float** points;
float** positions;
float** colors;

int positionSize, colorSize;

GLfloat theta[3] = {0.0,0.0,0.0};
GLfloat delta = 2.0;
GLint axis = 2;

bool stop = false;

int screenshotNum = 1;

typedef enum { TYPE_POINT, TYPE_LINE, TYPE_TRIANGLE } DRAW_TYPE;
DRAW_TYPE drawType = TYPE_POINT;


string StringToInt(int i) {
	ostringstream oss;
	oss << i;
	return oss.str();
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
	GLsizei count = 2*2*(imageWidth-1)*(imageHeight-1);

	switch(drawType) {
		case TYPE_POINT:
			glDrawArrays(GL_POINTS,first,count);
			break;
		case TYPE_TRIANGLE:
			for(int i=0; i<imageHeight-1; i++) {
				glDrawArrays(GL_TRIANGLE_STRIP,first,4*(imageWidth-1));
				first+=4*(imageWidth-1);
			}
			break;
		case TYPE_LINE:
			glDrawArrays(GL_LINES,first,count);

	}

  glBindVertexArray(0);

}

void displayFunc()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
  matrix->LoadIdentity();
  matrix->LookAt(1, 0, 2, 0, 0, -1, 0, 1, 0); // default camera
  //matrix->LookAt(0,100,200,0,0,0,0,1,0);
  //matrix->Translate(-128,0,120);

  //do translation using landTranslate 
	matrix->Translate(landTranslate[0],0.0,0.0);
	matrix->Translate(0.0,landTranslate[1],0.0);
	matrix->Translate(0.0,0.0,landTranslate[2]);

	//do rotations using landRotate
	matrix->Rotate(landRotate[0],1.0,0.0,0.0);	
	matrix->Rotate(landRotate[1],0.0,1.0,0.0);
	matrix->Rotate(landRotate[2],0.0,0.0,1.0);

	//do some scaling
	matrix->Scale(landScale[0],landScale[1],landScale[2]);

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
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
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
 /*if ((button==GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
 axis = 0;
          if ((button==GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN))
 axis = 1;
 if ((button==GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
 axis = 2;*/
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
    	landScale[0]*=0.9;
     	landScale[1]*=0.9;
    	landScale[2]*=0.9;

    	break;

    case 'e':
    	landScale[0]/=0.9;
     	landScale[1]/=0.9;
    	landScale[2]/=0.9;
    	break;

    case 'w':
    	landTranslate[1]-=0.03;
    	break;

    case 'a':
    	landTranslate[0]+=0.03;
    	break;

    case 's':
    	landTranslate[1]+=0.03;
    	break;

    case 'd':
    	landTranslate[0]-=0.03;
    	break;

    case 'z':
    	landTranslate[2]+=0.03;
    	break;

		case 'x':
			landTranslate[2]-=0.03;
			break;

		case 't':
			drawType = TYPE_TRIANGLE;
			break;

		case 'l':
			drawType = TYPE_LINE;
			break;

		case 'p':
			drawType = TYPE_POINT;
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

void fillData() {
	positionSize = 2*2*3 * sizeof(float) * (imageWidth-1) * (imageHeight-1);
	colorSize = 2*2*4 * sizeof(float) * (imageWidth-1) * (imageHeight-1);

	points = new float*[imageWidth*imageHeight];
	for(int i=0; i<imageWidth*imageHeight; i++) {
		points[i] = new float[4];
	}
	for(int i=0; i<imageHeight; i++) {
		for(int j=0; j<imageWidth; j++) {
			points[i*imageWidth+j][0]= i/(float)imageWidth;
			points[i*imageWidth+j][1]= j/(float)imageHeight;
			points[i*imageWidth+j][2]= heightmapImage->getPixel(i,j,0)/255.0f/4;
			points[i*imageWidth+j][3]= heightmapImage->getPixel(i,j,0)/255.0f;
		}
	}


	positions = new float*[2*2*(imageWidth-1)*(imageHeight-1)];
	for(int i=0; i<2*2*(imageWidth-1)*(imageHeight-1); i++) {
		positions[i] = new float[3];
	}

	colors = new float*[2*2*(imageWidth-1)*(imageHeight-1)];
	for(int i=0; i<2*2*(imageWidth-1)*(imageHeight-1); i++) {
		colors[i] = new float[4];
	}


	//render data 
	int index = 0;
	for(int i=0; i<imageHeight-1; i++) {
		for(int j=0; j<imageWidth-1; j++) {
			int point1 = i*imageWidth+j;
			int point2 = i*imageWidth+j+1;
			int point3 = (i+1)*imageWidth+j;
			int point4 = (i+1)*imageWidth+j+1;

			positions[index][0]= points[point1][0];
			positions[index][1]= points[point1][1];
			positions[index][2]= points[point1][2];		
			colors[index][0] = points[point1][3];
			colors[index][1] = points[point1][3];
			colors[index][2] = points[point1][3];
			colors[index][3] = 1.0;
			index++;

			positions[index][0]= points[point2][0];
			positions[index][1]= points[point2][1];
			positions[index][2]= points[point2][2];		
			colors[index][0] = points[point2][3];
			colors[index][1] = points[point2][3];
			colors[index][2] = points[point2][3];
			colors[index][3] = 1.0;
			index++;

			positions[index][0]= points[point3][0];
			positions[index][1]= points[point3][1];
			positions[index][2]= points[point3][2];		
			colors[index][0] = points[point3][3];
			colors[index][1] = points[point3][3];
			colors[index][2] = points[point3][3];
			colors[index][3] = 1.0;
			index++;

			positions[index][0]= points[point4][0];
			positions[index][1]= points[point4][1];
			positions[index][2]= points[point4][2];		
			colors[index][0] = points[point4][3];
			colors[index][1] = points[point4][3];
			colors[index][2] = points[point4][3];
			colors[index][3] = 1.0;
			index++;
		}
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
  for(int i=0; i < 2*2*(imageWidth-1)*(imageHeight-1); i++) {
      glBufferSubData(GL_ARRAY_BUFFER, loc, sizeof(float)*3, positions[i]);
      loc+=sizeof(float)*3;
  }

	// upload color data
  loc = positionSize;
  for(int i=0; i < 2*2*(imageWidth-1)*(imageHeight-1); i++) {
      glBufferSubData(GL_ARRAY_BUFFER, loc, sizeof(float)*4, colors[i]);
      loc+=sizeof(float)*4;
  }

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
void initScene(int argc, char *argv[])
{
  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  glClearColor(0.0f, 0.0f,0.0f, 0.0f);

  // do additional initialization here...
  glEnable(GL_DEPTH_TEST);
  matrix = new OpenGLMatrix();

  imageHeight = heightmapImage->getHeight();
  imageWidth = heightmapImage->getWidth();
  fillData();

  initVBO();
  initPipelineProgram();
  initVAO();

}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
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
}


