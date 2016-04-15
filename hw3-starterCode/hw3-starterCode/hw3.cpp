/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: Alexander Hong
 * *************************
*/

#ifdef WIN32
	#include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
	#include <GL/gl.h>
	#include <GL/glut.h>
#elif defined(__APPLE__)
	#include <OpenGL/gl.h>
	#include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <String>
#ifdef WIN32
	#define strcasecmp _stricmp
#endif

#include <imageIO.h>

#define PI 3.14159265

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480
const float ASPECT = (float)WIDTH/HEIGHT;
//the field of view of the camera
#define fov 60.0

using namespace std;
using namespace glm;

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double normal[3];
	double shininess;
};

struct Triangle
{
	Vertex v[3];
};

struct Sphere
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double shininess;
	double radius;
};

struct Light
{
	double position[3];
	double color[3];
};

struct Object {
	Object() { objectNum=-1; };
	string objectType;
	int objectNum;
	float tvalue;
	glm::dvec3 intersection;
};

struct Ray {
	glm::dvec3 origin;
	glm::dvec3 direction;
	Ray() {};
	Ray(double ox, double oy, double oz, double dx, double dy, double dz) {
		origin.x = ox;
		origin.y = oy;
		origin.z = oz;
		direction.x = dx;
		direction.y = dy;
		direction.z = dz;
		cout << direction.x << endl;
		direction = glm::normalize(direction);
		cout << direction.y << endl;
	};
	Object closestObject;
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
Ray** rays;
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

//The amount to increment the pixels X and Y values for the rays
float deltaX, deltaY;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

/*==============UTILITY FUNCTIONS============*/

double square (float num) {
	return num*num;
}

//
/**
 *Takes in a, b, and c, solves the quadratic function, and returns the minimum positive solution
 *
 * @param a, b, c- constants in the quadratic function 
 *
 *
 * @return double- the minimum non-zero solution to the quadratic equation. -1 if both solutions are negative or 0 
 */
double quadraticMinimum(double a, double b, double c) {
	double t0 = (-b + sqrt(b*b-4*a*c))/2;
	double t1 = (-b - sqrt(b*b-4*a*c))/2;
	if(t0 > t1) {
		if(t1 > 0) {
			return t1;
		} else if(t0 > 0) {
			return t0;
		} else {
			return -1;
		}
	} else {
		if(t0 > 0) {
			return t0;
		} else if(t1 > 0) {
			return t1;
		} else {
			return -1;
		}
	}
}

/*=============END UTILITY FUNCTION=========*/

void calculateRaySphereIntersection(Ray ray) {
	for(int i=0; i< num_spheres; i++) {
		double radius = spheres[i].radius;
		double xc = spheres[i].position[0];
		double yc = spheres[i].position[1];
		double zc = spheres[i].position[2];

		double xd = ray.direction.x;
		double yd = ray.direction.y;
		double zd = ray.direction.z;

		double b = 2*(xd * -xc + yd * -yc + zd * -zc);
		double c = square(xc) + square(yc) + square(zc) - square(radius);
		double result = quadraticMinimum(1, b, c);
		if(result > 0) {
			if(ray.closestObject.objectNum == -1 || ray.closestObject.tvalue > result) {
				Object newObject;
				newObject.objectType = "SPHERE";
				newObject.objectNum = i;
				newObject.tvalue = result;
				newObject.intersection = result*ray.direction;
				ray.closestObject = newObject;
			} 
		}

	}

}

//MODIFY THIS FUNCTION
void draw_scene()
{
	//a simple test output
	glPointSize(2.0);	
	glBegin(GL_POINTS);

	for(unsigned int x=0; x<WIDTH; x++)
	{
		for(unsigned int y=0; y<HEIGHT; y++)
		{
			calculateRaySphereIntersection(rays[x][y]);
			plot_pixel(x, y, x % 256, y % 256, (x+y) % 256);
		}
	}
	glEnd();
	glFlush();

	printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
	glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	buffer[y][x][0] = r;
	buffer[y][x][1] = g;
	buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	plot_pixel_display(x,y,r,g,b);
	if(mode == MODE_JPEG)
		plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
	printf("Saving JPEG file: %s\n", filename);

	ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
	if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
		printf("Error in Saving\n");
	else 
		printf("File saved Successfully\n");
}

void parse_check(const char *expected, char *found)
{
	if(strcasecmp(expected,found))
	{
		printf("Expected '%s ' found '%s '\n", expected, found);
		printf("Parse error, abnormal abortion\n");
		exit(0);
	}
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
	char str[100];
	fscanf(file,"%s",str);
	parse_check(check,str);
	fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
	printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
	char str[100];
	fscanf(file,"%s",str);
	parse_check("rad:",str);
	fscanf(file,"%lf",r);
	printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
	char s[100];
	fscanf(file,"%s",s);
	parse_check("shi:",s);
	fscanf(file,"%lf",shi);
	printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
	FILE * file = fopen(argv,"r");
	int number_of_objects;
	char type[50];
	Triangle t;
	Sphere s;
	Light l;
	fscanf(file,"%i", &number_of_objects);

	printf("number of objects: %i\n",number_of_objects);

	parse_doubles(file,"amb:",ambient_light);

	for(int i=0; i<number_of_objects; i++)
	{
		fscanf(file,"%s\n",type);
		printf("%s\n",type);
		if(strcasecmp(type,"triangle")==0)
		{
			printf("found triangle\n");
			for(int j=0;j < 3;j++)
			{
				parse_doubles(file,"pos:",t.v[j].position);
				parse_doubles(file,"nor:",t.v[j].normal);
				parse_doubles(file,"dif:",t.v[j].color_diffuse);
				parse_doubles(file,"spe:",t.v[j].color_specular);
				parse_shi(file,&t.v[j].shininess);
			}

			if(num_triangles == MAX_TRIANGLES)
			{
				printf("too many triangles, you should increase MAX_TRIANGLES!\n");
				exit(0);
			}
			triangles[num_triangles++] = t;
		}
		else if(strcasecmp(type,"sphere")==0)
		{
			printf("found sphere\n");

			parse_doubles(file,"pos:",s.position);
			parse_rad(file,&s.radius);
			parse_doubles(file,"dif:",s.color_diffuse);
			parse_doubles(file,"spe:",s.color_specular);
			parse_shi(file,&s.shininess);	

			if(num_spheres == MAX_SPHERES)
			{
				printf("too many spheres, you should increase MAX_SPHERES!\n");
				exit(0);
			}
			spheres[num_spheres++] = s;
		}
		else if(strcasecmp(type,"light")==0)
		{
			printf("found light\n");
			parse_doubles(file,"pos:",l.position);
			parse_doubles(file,"col:",l.color);

			if(num_lights == MAX_LIGHTS)
			{
				printf("too many lights, you should increase MAX_LIGHTS!\n");
				exit(0);
			}
			lights[num_lights++] = l;
		}
		else
		{
			printf("unknown type in scene description:\n%s\n",type);
			exit(0);
		}
	}
	return 0;
}

void display()
{
}

void init()
{
	glMatrixMode(GL_PROJECTION);
	glOrtho(0,WIDTH,0,HEIGHT,1,-1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);

	//Draw Rays
	rays = new Ray*[WIDTH];
	for(int i=0; i< WIDTH; i++) {
		rays[i] = new Ray[HEIGHT];
	}

	//define the four corners
	float tangentValue = tan(fov*PI/180.0/2);
	float x_max = ASPECT*tangentValue;
	float x_min = -1*ASPECT*tangentValue;
	float y_max = tangentValue;
	float y_min = -1*tangentValue;

	cout << x_max << endl;
	cout << y_max << endl;

	rays[0][0] = 				Ray(0, 0 ,0, x_min, y_min, -1);
	rays[WIDTH-1][0] = 			Ray(0, 0 ,0, x_max, y_min, -1);
	rays[0][HEIGHT-1] = 		Ray(0, 0 ,0, x_min, y_max, -1);
	rays[WIDTH-1][HEIGHT-1] = 	Ray(0, 0 ,0, x_max, y_max, -1);

	//set up increment values
	deltaX = (x_max - x_min)/WIDTH;
	deltaY = (y_max - y_min)/HEIGHT;

	float x_count = x_min;
	float y_count = y_min;
	//create the remaining rays
	for(int i=0; i < WIDTH; i++) {
		for(int j=0; j < HEIGHT; j++) {
			rays[i][j] = Ray(0, 0, 0, x_count, y_count, -1);
			x_count += deltaX;
		}
		x_count = x_min;
		y_count += deltaY;
	}


}

void idle()
{
	//hack to make it only draw once
	static int once=0;
	if(!once)
	{
		draw_scene();
		if(mode == MODE_JPEG)
			save_jpg();
	}
	once=1;
}

int main(int argc, char ** argv)
{
	if ((argc < 2) || (argc > 3))
	{	
		printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
		exit(0);
	}
	if(argc == 3)
	{
		mode = MODE_JPEG;
		filename = argv[2];
	}
	else if(argc == 2)
		mode = MODE_DISPLAY;

	glutInit(&argc,argv);
	loadScene(argv[1]);

	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(WIDTH,HEIGHT);
	int window = glutCreateWindow("Ray Tracer");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	init();

	glutMainLoop();
}

