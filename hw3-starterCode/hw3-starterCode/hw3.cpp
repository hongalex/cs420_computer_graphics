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
#include <math.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
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
const double ASPECT = (double)WIDTH/HEIGHT;
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

	dvec3 calculateUnitNormal(dvec3 point) {
		dvec3 center = dvec3(position[0], position[1], position[2]);
		return normalize(point - center); 
	}
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
	double tvalue;
	glm::dvec3 intersection;
};

struct Ray {

	glm::dvec3 origin;
	glm::dvec3 direction;

	Object closestObject;
	glm::dvec3 color;

	Ray() {}; //default constructor
	//pass in points individually
	Ray(double ox, double oy, double oz, double dx, double dy, double dz) {
		origin.x = ox;
		origin.y = oy;
		origin.z = oz;
		direction.x = dx;
		direction.y = dy;
		direction.z = dz;
		direction = glm::normalize(direction);	
		color.x = 0;
		color.y = 0;
		color.z = 0;
	};
	//pass in points by vec3's
	Ray(dvec3 origin, dvec3 direction) {
		this->origin = origin;
		this->direction = direction;
		direction = glm::normalize(direction);	
		color.x = 50;
		color.y = 50;
		color.z = 50;
	}
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
double deltaX, deltaY;
int shadowsCount = 0, outerloop = 0, elseloop = 0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

/*==============UTILITY FUNCTIONS============*/

double square (double num) {
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

void printVector(vec3 vec) {
	cout << vec.x << " , " << vec.y << " , " << vec.z << endl;
}

//taken from GLM website
double clamp(double x, double minVal, double maxVal) {
	return std::min(std::max(x, minVal), maxVal);
}

dvec3 toVec3(double* array) {
	dvec3 result;
	result.x = array[0];
	result.y = array[1];
	result.z = array[2];
	return result;
}

/**
 * Calculate barycentric coordinates using a point inside a triangle, and the vertices of a triangle (a, b, and c)
 *
 * @return dvec3- a vec3 containing the values of Alpha, Beta, and Gamma
 */
dvec3 calcBarycentric(dvec3 point, dvec3 a, dvec3 b, dvec3 c) {
	dvec3 v0 = b - a;
	dvec3 v1 = c - a;
	dvec3 v2 = point - a;

	double d00 = dot(v0, v0);
	double d01 = dot(v0, v1);
	double d11 = dot(v1, v1);
	double d20 = dot(v2, v0);
	double d21 = dot(v2, v1);

	double result = d00*d11 - d01 *d01;
	dvec3 vec_result;
	vec_result.x = (d11*d20 - d01*d21)/result;
	vec_result.y = (d00*d21 - d01*d20)/result;
	vec_result.z = 1.0 - vec_result.x - vec_result.y;
	return vec_result;
}

/*=============END UTILITY FUNCTION=========*/

void calculateRaySphereIntersection(Ray &ray, int num) {
	for(int i=0; i< num_spheres; i++) {
		if(i!=num) {
			double radius = spheres[i].radius;
			double xc = spheres[i].position[0];
			double yc = spheres[i].position[1];
			double zc = spheres[i].position[2];

			double xd = ray.direction.x;
			double yd = ray.direction.y;
			double zd = ray.direction.z;

			double x0 = ray.origin.x;
			double y0 = ray.origin.y;
			double z0 = ray.origin.z;

			double b = 2*(xd *(x0-xc) + yd *(y0-yc) + zd *(z0 -zc));
			double c = square(x0-xc) + square(y0-yc) + square(z0-zc) - square(radius);
			double result = quadraticMinimum(1, b, c);
			if(result > 0) {
				if(ray.closestObject.objectNum == -1 || ray.closestObject.tvalue > result) {
					Object newObject;
					newObject.objectType = "SPHERE";
					newObject.objectNum = i;
					newObject.tvalue = result;
					newObject.intersection = ray.origin + result*ray.direction;
					ray.closestObject = newObject;
				} 
			}
		} 
	}
}

void calculateRayTriangleIntersection(Ray &ray, int num) {
	for(int i=0; i< num_triangles; i++) {
		if(i!=num) {
			Triangle triangle = triangles[i];
			dvec3 pointA = vec3(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]); 
			dvec3 pointB = vec3(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]); 
			dvec3 pointC = vec3(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]); 

			dvec3 n = cross((pointB - pointA), (pointC-pointA));
			n = normalize(n);
			if(dot(n,ray.direction)!=0) {

				double t = dot(n,pointA - ray.origin)/(dot(n,ray.direction));


				if(t > 0) {
					if(ray.closestObject.objectNum == -1 || ray.closestObject.tvalue > t) {
						//Check if the intersection point is inside the triangle. 
						//Pseudocode borrowed from http://www.blackpawn.com/texts/pointinpoly/
						// Compute vectors        
						dvec3 v0 = pointC - pointA;
						dvec3 v1 = pointB - pointA;
						dvec3 v2 = ray.origin + t*ray.direction - pointA;

						// Compute dot products
						double dot00 = dot(v0, v0);
						double dot01 = dot(v0, v1);
						double dot02 = dot(v0, v2);
						double dot11 = dot(v1, v1);
						double dot12 = dot(v1, v2);

						// Compute barycentric coordinates
						double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
						double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
						double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

						// Check if point is in triangle
						if((u >= 0) && (v >= 0) && (u + v < 1)) {

							Object newObject;
							newObject.objectType = "TRIANGLE";
							newObject.objectNum = i;
							newObject.tvalue = t;
							newObject.intersection = ray.origin + t*ray.direction;
							ray.closestObject = newObject;
						} 
					}
				}
			}
		} 
	}
}

/**
 * Fire a shadow ray, and then calculate the color using Phong illumination model if there is no intersection
 * If the shadow ray is obstructed, then return black
 *
 *
 *
 */
void calculateShadowRay(Ray &ray) {
	//Fire a shadow ray first for each light source
	for(int i=0; i<num_lights; i++) {
		Light light = lights[i];

		//If the ray actually intersected with something, fire the shadow ray
		if(ray.closestObject.objectNum != -1) {
			dvec3 lightVec = dvec3(light.position[0], light.position[1], light.position[2]);
			lightVec -= ray.closestObject.intersection;
			lightVec = normalize(lightVec);
			Ray shadowRay = Ray(ray.closestObject.intersection, lightVec);
			if(ray.closestObject.objectType == "SPHERE") {
				calculateRaySphereIntersection(shadowRay, ray.closestObject.objectNum);
				calculateRayTriangleIntersection(shadowRay, -1);
			} else {
				calculateRaySphereIntersection(shadowRay, -1);
				calculateRayTriangleIntersection(shadowRay, ray.closestObject.objectNum);
			}
			
			//if there is no intersection, calculate color using Phong Illumination model with respect to that light
			if(shadowRay.closestObject.objectNum == -1) {
			//if(true) {
				dvec3 kd, ks;
				double alpha; //diffuse, specular, and alpha (shininess)

				dvec3 l, n, r, v, L; //Light vector, normal vector, reflect vector, vector to image plane, Light color
				L = dvec3(light.color[0], light.color[1], light.color[2]);
				v = -ray.direction;
				l = normalize(shadowRay.direction);

				//Calculate lighting for Spheres
				if(ray.closestObject.objectType== "SPHERE") {
					Sphere s = spheres[ray.closestObject.objectNum];
					n = s.calculateUnitNormal(ray.closestObject.intersection);

					kd = dvec3(s.color_diffuse[0], s.color_diffuse[1], s.color_diffuse[2]);
					ks = dvec3(s.color_specular[0], s.color_specular[1], s.color_specular[2]);
					alpha = s.shininess;

				}

				//Calculate lighting for triangles
				else if(ray.closestObject.objectType == "TRIANGLE") {
					Triangle t = triangles[ray.closestObject.objectNum];
					Vertex a = t.v[0], b = t.v[1], c = t.v[2];
					dvec3 bary = calcBarycentric(ray.closestObject.intersection, toVec3(a.position), toVec3(b.position), toVec3(c.position));
					n = normalize(toVec3(a.normal)*bary.x + toVec3(b.normal)*bary.y + toVec3(c.normal)*bary.z);
					
					kd = toVec3(a.color_diffuse)*bary.x + toVec3(b.color_diffuse)*bary.y + toVec3(c.color_diffuse)*bary.z;
					ks = toVec3(a.color_specular)*bary.x + toVec3(b.color_specular)*bary.y + toVec3(c.color_specular)*bary.z;
					alpha = a.shininess*bary.x + b.shininess*bary.y + c.shininess*bary.z;
				}
				
				double ln = dot(l, n);
				if(ln < 0) ln = 0;

				r = 2*(ln)*n - l;
				double rv = dot(r,v);
				if(rv < 0) rv = 0;


				//kd(l*n) + ks(r*v)^a
				double red = L.x*(kd.x*(ln) + ks.x*pow(rv, alpha))*255;
				double green = L.y*(kd.y*(ln) + ks.y*pow(rv, alpha))*255;
				double blue = L.z*(kd.z*(ln) + ks.z*pow(rv, alpha))*255;

				ray.color.x += red;
				ray.color.y += green;
				ray.color.z += blue;

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
			calculateRayTriangleIntersection(rays[x][y], -1);
			calculateRaySphereIntersection(rays[x][y], -1);
			calculateShadowRay(rays[x][y]);

			//if you can't find the intersection, plot a white color
			if(rays[x][y].closestObject.objectNum == -1) {
				plot_pixel(x, y, 255, 255, 255);
			}

			//plot the actual color of the ray intersection
			else {
				double red = clamp(rays[x][y].color.x + 255*ambient_light[0], 0, 255);
				double green = clamp(rays[x][y].color.y + 255*ambient_light[1], 0, 255);
				double blue = clamp(rays[x][y].color.z + 255*ambient_light[2], 0, 255);

				plot_pixel(x, y, red, green, blue);
			}
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
	double tangentValue = tan(fov*PI/180.0/2);
	double x_max = ASPECT*tangentValue;
	double x_min = -1*ASPECT*tangentValue;
	double y_max = tangentValue;
	double y_min = -1*tangentValue;

	rays[0][0] = 				Ray(0, 0 ,0, x_min, y_min, -1);
	rays[WIDTH-1][0] = 			Ray(0, 0 ,0, x_max, y_min, -1);
	rays[0][HEIGHT-1] = 		Ray(0, 0 ,0, x_min, y_max, -1);
	rays[WIDTH-1][HEIGHT-1] = 	Ray(0, 0 ,0, x_max, y_max, -1);

	//set up increment values
	deltaX = (x_max - x_min)/WIDTH;
	deltaY = (y_max - y_min)/HEIGHT;

	double x_count = x_min;
	double y_count = y_min;
	//create the remaining rays
	for(int i=0; i < WIDTH; i++) {
		for(int j=0; j < HEIGHT; j++) {
			rays[i][j] = Ray(0, 0, 0, x_min + i*deltaX, y_min + j*deltaY, -1);
			y_count += deltaY;
		}
		y_count = y_min;
		x_count += deltaX;
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

