#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#include "udray.h"
#include "glm.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

extern Camera *ray_cam;       // camera info
extern int image_i, image_j;  // current pixel being shaded
extern bool wrote_image;      // has the last pixel been shaded?

// reflection/refraction recursion control

extern int maxlevel;          // maximum depth of ray recursion 
extern double minweight;      // minimum fractional contribution to color

// these describe the scene

extern vector < GLMmodel * > model_list;
extern vector < Sphere * > sphere_list;
extern vector < Light * > light_list;


float minDepth; 
float maxDepth;
int shaded, empty = 0;
bool verbose = false;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// intersect a ray with the entire scene (.obj models + spheres)

// x, y are in pixel coordinates with (0, 0) the upper-left hand corner of the image.
// color variable is result of this function--it carries back info on how to draw the pixel
void shade_ray_depth(Ray *ray, Intersection *inter, Vect color, float minimumDetph, float maximumDepth);
void shade_ray_shadows(Ray *ray, Intersection *inter, Vect color, int level);
void trace_ray(int level, double weight, Ray *ray, Vect color)
{
	Intersection *nearest_inter = NULL;
	Intersection *inter = NULL;
	int i;
	if (level > 100){
		int hmmm = 0;
	}

	// test for intersection with all .obj models

	for (i = 0; i < model_list.size(); i++) {
		inter = intersect_ray_glm_object(ray, model_list[i]);
		update_nearest_intersection(&inter, &nearest_inter);
	}

	// test for intersection with all spheres

	for (i = 0; i < sphere_list.size(); i++) {
		inter = intersect_ray_sphere(ray, sphere_list[i]);
		update_nearest_intersection(&inter, &nearest_inter);
	}

	// "color" the ray according to intersecting surface properties

	// choose one of the simpler options below to debug or preview your scene more quickly.
	// another way to render faster is to decrease the image size.

	if (nearest_inter) {
		//shade_ray_false_color_normal(nearest_inter, color);
		//    shade_ray_intersection_mask(color); 
		//shade_ray_depth(ray, nearest_inter, color, 1.5, 2.0);
		//shade_ray_diffuse(ray, nearest_inter, color);
		ray->occluded = true;
		if (level < 100)
			shade_ray_local(ray, nearest_inter, color);
		else{
			//We occluded!!!!!!
			int hmm = 0;
		}
		
		//shade_ray_shadows(ray, nearest_inter, color, 0);
		//   shade_ray_recursive(level, weight, ray, nearest_inter, color);
	}

	// color the ray using a default

	else{
		if (level < 100)
			shade_ray_background(ray, color); 
		ray->occluded = false;
	}
}

//----------------------------------------------------------------------------

// test for ray-sphere intersection; return details of intersection if true

Intersection *intersect_ray_sphere(Ray *ray, Sphere *S)
{
	// FILL IN CODE (line below says "no" for all spheres, so replace it)
	//(S-P)(S-P) - r^2 = 0
	Vect center;
	VectCopy(center, S->P);
	double sphereRadius = S->radius;
	
	Vect lineOrigin;
	VectCopy(lineOrigin, ray->orig);
	Vect lineDirection;
	VectCopy(lineDirection, ray->dir);
	Vect originMinusCenter;
	VectSub(lineOrigin, center, originMinusCenter);
	double t = -(VectDotProd(lineDirection, originMinusCenter));

	double underSqrt = pow(VectDotProd(lineDirection, originMinusCenter),2);
	underSqrt -= pow(VectMag(originMinusCenter),2);
	underSqrt += pow(sphereRadius,2);
	//No intersection
	if (underSqrt < 0){
		return NULL;
	}
	//If more than one intersection, pick closest Intersection
	else if (underSqrt > 0){
		underSqrt = sqrt(underSqrt);
		double plusUnderSqrt = t + underSqrt;
		double minusUnderSqrt = t - underSqrt;

		if (plusUnderSqrt < 0 && minusUnderSqrt < 0){
			return NULL;
		}
		else if (plusUnderSqrt){
			if (minusUnderSqrt){
				t = plusUnderSqrt < minusUnderSqrt ? plusUnderSqrt : minusUnderSqrt;
			}
			else{
				t = plusUnderSqrt;
			}
		}
		else{
			t = minusUnderSqrt;
		}
	}
	Vect pointOfIntersection;
	pointOfIntersection[X] = lineOrigin[X] + (t*lineDirection[X]);
	pointOfIntersection[Y] = lineOrigin[Y] + (t*lineDirection[Y]);
	pointOfIntersection[Z] = lineOrigin[Z] + (t*lineDirection[Z]);

	Intersection * result = make_intersection();
	VectCopy(result->P, pointOfIntersection);
	Vect normal;
	VectSub(pointOfIntersection, center, normal);
	VectUnit(normal);
	VectCopy(result->N, normal);
	result->surf = S->surf;
	result->t = t;
	return result;
	
}

//----------------------------------------------------------------------------



void shade_ray_depth(Ray *ray, Intersection *inter, Vect color, float minimumDepth, float maximumDepth)
{
	Vect L;
	double diff_factor;

	// iterate over lights

	for (int i = 0; i < light_list.size(); i++) {

		// AMBIENT
		float range = maximumDepth - minimumDepth;
		double depth = static_cast<double>(inter->t);

		color[R] = color[G] = color[B] = (range - (depth - minimumDepth))/range;

	}

	// clamp color to [0, 1]

	VectClamp(color, 0, 1);
}


//----------------------------------------------------------------------------



void shade_ray_shadows(Ray *ray, Intersection *inter, Vect color, int level)
{
	Vect L;
	double diff_factor;

	// iterate over lights

	for (int i = 0; i < light_list.size(); i++) {
		
		// AMBIENT
		Light * currentLight = light_list[i];
		color[R] += inter->surf->amb[R] * currentLight->amb[R];
		color[G] += inter->surf->amb[G] * currentLight->amb[G]; 
		color[B] += inter->surf->amb[B] * currentLight->amb[B];
		Vect directionOfLight; 
		VectSub( inter->P, currentLight->P ,directionOfLight);
		VectUnit(directionOfLight);
		// DIFFUSE 
		Vect incremented;
		incremented[X] = inter->P[X] + 0.001*directionOfLight[X];
		incremented[Y] = inter->P[Y] + 0.001*directionOfLight[Y];
		incremented[Z] = inter->P[Z] + 0.001*directionOfLight[Z];
		Ray * shadowRay = make_ray(incremented, directionOfLight);
		trace_ray(101, 1.0,  shadowRay, color);
		
		if (!shadowRay->occluded ){
			
		
			double dotProduct = VectDotProd(directionOfLight, inter->N);
			dotProduct = dotProduct < 0 ? 0 : dotProduct;

			//printf("intensity: %f\n",dotProduct);

			color[R] += inter->surf->diff[R] * currentLight->diff[R] * dotProduct;
			color[G] += inter->surf->diff[G] * currentLight->diff[G] * dotProduct;
			color[B] += inter->surf->diff[B] * currentLight->diff[B] * dotProduct;
		}
		

	}

	// clamp color to [0, 1]

	VectClamp(color, 0, 1);
}

//----------------------------------------------------------------------------

// only local, ambient + diffuse lighting (no specular, shadows, reflections, or refractions)

void shade_ray_diffuse(Ray *ray, Intersection *inter, Vect color)
{
	Vect L;
	double diff_factor;

	// iterate over lights

	for (int i = 0; i < light_list.size(); i++) {

		// AMBIENT
		Light * currentLight = light_list[i];
		color[R] += inter->surf->amb[R] * currentLight->amb[R];
		color[G] += inter->surf->amb[G] * currentLight->amb[G]; 
		color[B] += inter->surf->amb[B] * currentLight->amb[B];
	
		// DIFFUSE 

		Vect directionOfLight; 
		VectSub( inter->P, currentLight->P ,directionOfLight);
		VectUnit(directionOfLight);
		
		double dotProduct = VectDotProd(directionOfLight, inter->N);
		dotProduct = dotProduct < 0 ? 0 : dotProduct;

		//printf("intensity: %f\n",dotProduct);

		color[R] += inter->surf->diff[R] * currentLight->diff[R] * dotProduct;
		color[G] += inter->surf->diff[G] * currentLight->diff[G] * dotProduct;
		color[B] += inter->surf->diff[B] * currentLight->diff[B] * dotProduct;

	}

	// clamp color to [0, 1]

	VectClamp(color, 0, 1);
}

// same as shade_ray_diffuse(), but add specular lighting + shadow rays (i.e., full Phong illumination model)

void shade_ray_local(Ray *ray, Intersection *inter, Vect color)
{
	Vect L;
	double diff_factor;
	Vect tempColor;
	// iterate over lights
	int x;
	for (x= 0; x < light_list.size(); x++) {

		// AMBIENT
		Light * currentLight = light_list[x];
		//printf("light: %d of %d \n",x+1,light_list.size());
		color[R] += inter->surf->amb[R] * currentLight->amb[R];
		color[G] += inter->surf->amb[G] * currentLight->amb[G]; 
		color[B] += inter->surf->amb[B] * currentLight->amb[B];
		if (verbose){
			printf("intersects object at %f, %f, %f\n ", inter->P[X], inter->P[Y], inter->P[Z]); 
			//printf("light at %f, %f, %f\n ", currentLight->P[Y],currentLight->P[Z]);
		}
		//Calculate the direction of the light, relative to the point of intersection
		Vect directionOfLight; 
		VectSub( currentLight->P , inter->P, directionOfLight);
		VectUnit(directionOfLight);
		
		Vect incremented;
		incremented[X] = inter->P[X] + 0.001*directionOfLight[X];
		incremented[Y] = inter->P[Y] + 0.001*directionOfLight[Y];
		incremented[Z] = inter->P[Z] + 0.001*directionOfLight[Z];
		
		Ray * shadowRay = make_ray(incremented, directionOfLight);
		
		trace_ray( 101, 1.0,  shadowRay, color);
		//printf("1 r: %f, g: %f, b: %f\n", color[R], color[G], color[B]);fflush(stdout);
		if (!shadowRay->occluded){
			shaded++;
			double dotProduct = VectDotProd(directionOfLight,inter->N);
			dotProduct = dotProduct < 0 ? 0 : dotProduct;

			// DIFFUSE 
			color[R] += inter->surf->diff[R] * currentLight->diff[R] * dotProduct;
			color[G] += inter->surf->diff[G] * currentLight->diff[G] * dotProduct;
			color[B] += inter->surf->diff[B] * currentLight->diff[B] * dotProduct;
			
			//SPECULAR
			Vect viewDirection;
			VectCopy(ray->dir, viewDirection);
			VectNegate(viewDirection, viewDirection);
			Vect halfVector;
			halfVector[X] = viewDirection[X] + directionOfLight[X];
			halfVector[Y] = viewDirection[Y] + directionOfLight[X];
			halfVector[Z] = viewDirection[X] + directionOfLight[Z];
			halfVector[X] = halfVector[X]/2;
			halfVector[Y] = halfVector[Y]/2;
			halfVector[Z] = halfVector[Z]/2;

			VectUnit(halfVector);
			dotProduct = VectDotProd(inter->N, halfVector);
			color[R] += inter->surf->spec[R] * currentLight->spec[R] * pow(dotProduct,inter->surf->spec_exp);
			color[G] += inter->surf->spec[G] * currentLight->spec[G] * pow(dotProduct,inter->surf->spec_exp);
			color[B] += inter->surf->spec[B] * currentLight->spec[B] * pow(dotProduct,inter->surf->spec_exp);
			//printf("1 r: %f, g: %f, b: %f\n", color[R], color[G], color[B]);
			fflush(stdout);
			VectClamp(color, 0.0, 1.0);
			if (x = light_list.size()){
				VectCopy( tempColor, color);
			}
		}
		else{
			/*if (verbose)
				printf("did not shade occluded pixel\n");*/
			empty++;
		}
	}
	//if (color[0] != tempColor[0] || color[1] != tempColor[1] || color[2] != tempColor[2])
		//printf("diffeerence detected!\n");
	// clamp color to [0, 1]
	//printf("2 r: %f, g: %f, b: %f\n", color[R], color[G], color[B]);fflush(stdout);
	VectClamp(color, 0.0, 1.0);
	//printf("3 r: %f, g: %f, b: %f\n", color[R], color[G], color[B]);fflush(stdout);
}

//----------------------------------------------------------------------------

// full shading model: ambient/diffuse/specular lighting, shadow rays, recursion for reflection, refraction

// level = recursion level (only used for reflection/refraction)

void shade_ray_recursive(int level, double weight, Ray *ray, Intersection *inter, Vect color)
{
	Surface *surf;
	int i;

	// initialize color to Phong reflectance model

	shade_ray_local(ray, inter, color);

	// if not too deep, recurse

	if (level + 1 < maxlevel) {

		// add reflection component to color

		if (surf->reflectivity * weight > minweight) {

			// FILL IN CODE

		}

		// add refraction component to color

		if (surf->transparency * weight > minweight) {

			// GRAD STUDENTS -- FILL IN CODE

		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// ray trace another pixel if the image isn't finished yet

void idle()
{
	if (image_j < ray_cam->im->h) {

		raytrace_one_pixel(image_i, image_j);

		image_i++;

		if (image_i == ray_cam->im->w) {
			image_i = 0;
			image_j++;
			glutPostRedisplay();
		}    
	}

	// write rendered image to file when done

	else if (!wrote_image) {
		printf("minDepth and maxDepth:< %f, %f >\n",minDepth, maxDepth);
		write_PPM("output.ppm", ray_cam->im);
		printf("shaded: %d\n occluded: %d\n", shaded, empty);
		verbose = true;
		wrote_image = true;
	}

	
}

//----------------------------------------------------------------------------

// show the image so far

void display(void)
{
	// draw it!

	glPixelZoom(1, -1);
	glRasterPos2i(0, ray_cam->im->h);

	glDrawPixels(ray_cam->im->w, ray_cam->im->h, GL_RGBA, GL_FLOAT, ray_cam->im->data);

	glFlush ();
}

//----------------------------------------------------------------------------

void init()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, ray_cam->im->w, 0.0, ray_cam->im->h);
}

//----------------------------------------------------------------------------
//Clicking a pixel starts raytracing for that pixel
void onMouseButton(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ){
		printf("clicked pixel <%d, %d>\n", x, y);
		raytrace_one_pixel(x, y);
		glutPostRedisplay();

	}
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	// initialize scene (must be done before scene file is parsed)

	init_raytracing();

	if (argc == 2)
		parse_scene_file(argv[1], ray_cam);
	else {
		printf("missing .scene file. Loading default\n");
		parse_scene_file("test.scene", ray_cam);
		//exit(1);
	}

	// opengl business

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(ray_cam->im->w, ray_cam->im->h);
	glutInitWindowPosition(500, 300);
	
	glutCreateWindow("hw3");
	glutMouseFunc(onMouseButton);
	init();

	glutDisplayFunc(display); 
	glutIdleFunc(idle);

	glutMainLoop();

	return 0; 
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
