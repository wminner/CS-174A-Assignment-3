// Wesley Minner
// 703549234
// CS 174A, Dis 1B, Fall 15

//
// template-rt.cpp
//

#define _CRT_SECURE_NO_WARNINGS
#include "matm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

struct Ray
{
    vec4 origin;
    vec4 dir;
};

// TODO: add structs for spheres, lights and anything else you may need.
struct Sphere
{
	string name;
	vec4 origin;
	mat4 scale;
	vec3 rgb;
	float Ka;				// Ambient coeffecient
	float Kd;				// Diffuse coeffecient
	float Ks;				// Specular coeffecient
	float Kr;				// Reflective coeffecient
	float n;				// Shininess
	mat4 sphereTrans;
	mat4 invSphereTrans;
};

struct Light
{
	string name;
	vec4 origin;
	vec3 rgb;
};

struct Intersect
{
	vec4 pos;		// Point position of intersect
	vec4 norm;		// Normal vector at intersect
	float dist;		// Distance of intersect point from camera
	int sphereNum;	// Index of sphere that intersect occurred on
};

// Resolution and pixel colors
int g_width;
int g_height;
vector<vec4> g_colors;

// Other colors
vec3 g_background;
vec3 g_ambient;

// Frustrum
float g_left;
float g_right;
float g_top;
float g_bottom;
float g_near;

// Output filename
char *outName;

// Spheres
vector<Sphere> g_spheres;
int sphereIndex = 0;

// Lights
vector<Light> g_lights;
int lightIndex = 0;

// -------------------------------------------------------------------
// Input file parsing

inline vec3 toVec3(vec4 in)
{
	return vec3(in[0], in[1], in[2]);
}

vec4 toVec4(const string& s1, const string& s2, const string& s3)
{
    stringstream ss(s1 + " " + s2 + " " + s3);
    vec4 result;
    ss >> result.x >> result.y >> result.z;
    result.w = 1.0f;
    return result;
}

float toFloat(const string& s)
{
    stringstream ss(s);
    float f;
    ss >> f;
    return f;
}

void parseLine(const vector<string>& vs)
{
    //DONE: add parsing of NEAR, LEFT, RIGHT, BOTTOM, TOP, SPHERE, LIGHT, BACK, AMBIENT, OUTPUT.
	const int num_labels = 11;	//0		 1		 2		   3	   4	  5		  6			7		8		  9			10
	const string labels[] = { "NEAR", "LEFT", "RIGHT", "BOTTOM", "TOP", "RES", "SPHERE", "LIGHT", "BACK", "AMBIENT", "OUTPUT" };
	unsigned label_id = find( labels, labels + num_labels, vs[0] ) - labels;

	switch (label_id) {
		case 0:		// NEAR
			g_near = toFloat(vs[1]);
			break;
		case 1:		// LEFT
			g_left = toFloat(vs[1]);
			break;
		case 2:		// RIGHT
			g_right = toFloat(vs[1]);
			break;
		case 3:		// BOTTOM
			g_bottom = toFloat(vs[1]);
			break;
		case 4:		// TOP
			g_top = toFloat(vs[1]);
			break;
		case 5:		// RES
			g_width = (int)toFloat(vs[1]);
			g_height = (int)toFloat(vs[2]);
			g_colors.resize(g_width * g_height);
			break;
		case 6:		// SPHERE
			g_spheres.push_back(Sphere());
			g_spheres[sphereIndex].name = vs[1];
			g_spheres[sphereIndex].origin = toVec4(vs[2], vs[3], vs[4]);
			g_spheres[sphereIndex].scale = Scale(vec3(toFloat(vs[5]), toFloat(vs[6]), toFloat(vs[7])));
			g_spheres[sphereIndex].rgb = vec3(toFloat(vs[8]), toFloat(vs[9]), toFloat(vs[10]));
			g_spheres[sphereIndex].Ka = toFloat(vs[11]);
			g_spheres[sphereIndex].Kd = toFloat(vs[12]);
			g_spheres[sphereIndex].Ks = toFloat(vs[13]);
			g_spheres[sphereIndex].Kr = toFloat(vs[14]);
			g_spheres[sphereIndex].n = toFloat(vs[15]);

			// Find sphere transform
			g_spheres[sphereIndex].sphereTrans = Translate(g_spheres[sphereIndex].origin) * g_spheres[sphereIndex].scale;

			// Find inverse sphere transform
			InvertMatrix(g_spheres[sphereIndex].sphereTrans, g_spheres[sphereIndex].invSphereTrans);

			sphereIndex++;
			break;
		case 7:		// LIGHT
			g_lights.push_back(Light());
			g_lights[lightIndex].name = vs[1];
			g_lights[lightIndex].origin = toVec4(vs[2], vs[3], vs[4]);
			g_lights[lightIndex].rgb = vec3(toFloat(vs[5]), toFloat(vs[6]), toFloat(vs[7]));

			lightIndex++;
			break;
		case 8:		// BACK
			g_background = vec3(toFloat(vs[1]), toFloat(vs[2]), toFloat(vs[3]));
			break;
		case 9:		// AMBIENT
			g_ambient = vec3(toFloat(vs[1]), toFloat(vs[2]), toFloat(vs[3]));
			break;
		case 10:	// OUTPUT
			int len = vs[1].length();
			outName = (char*)malloc(len+1);
			for (int i = 0; i < len; i++) {
				outName[i] = vs[1][i];
			}
			outName[len] = '\0';
			break;
	}
}

void loadFile(const char* filename)
{
    ifstream is(filename);
    if (is.fail())
    {
        cout << "Could not open file " << filename << endl;
        exit(1);
    }
    string s;
    vector<string> vs;
    while(!is.eof())
    {
        vs.clear();
        getline(is, s);
        istringstream iss(s);
        while (!iss.eof())
        {
            string sub;
            iss >> sub;
            vs.push_back(sub);
        }
        parseLine(vs);
    }
}


// -------------------------------------------------------------------
// Utilities

void setColor(int ix, int iy, const vec4& color)
{
    int iy2 = g_height - iy - 1; // Invert iy coordinate.
    g_colors[iy2 * g_width + ix] = color;
}


// -------------------------------------------------------------------
// Intersection routine
bool IntersectRay(const Ray &ray, Intersect &intersect)
{
	// TODO: add your ray-sphere intersection routine here.
	bool intersectFound = false;
	Ray rayPrime;
	float determ;
	float solution[2] = { 0, 0 };
	float minDist[2] = { 0, 0 };
	vector<Intersect> isectList;
	int isectNum = 0;
	
	// For each sphere in scene...
	for (int i = 0; i < sphereIndex; i++) {
		// Find inverse transform ray
		rayPrime.origin = normalize(g_spheres[i].invSphereTrans * ray.origin);	// Need to normalize? TODO
		rayPrime.dir = normalize(g_spheres[i].invSphereTrans * ray.dir);		// Need to normalize? TODO
		
		// Find intersection of inverse transformed ray with unit sphere at origin
		// Use quadratic to find determinant
		determ = pow(dot(rayPrime.origin, rayPrime.dir), 2) - pow(length(rayPrime.dir), 2) * (pow(length(rayPrime.origin), 2) - 1);

		// Analyze determinant to find number of intersection points
		if (determ < 0) {  // If determinant < 0, no intersect
			;	// Do nothing
		} 
		else if (determ > 0) {  // If determinant > 0, two intersect
			// Get two solutions and find associated intersections
			solution[0] = -1 * (dot(rayPrime.origin, rayPrime.dir) / pow(length(rayPrime.dir), 2)) + (determ / pow(length(rayPrime.dir), 2));
			solution[1] = -1 * (dot(rayPrime.origin, rayPrime.dir) / pow(length(rayPrime.dir), 2)) - (determ / pow(length(rayPrime.dir), 2));
			
			// Save first intersection if within frustrum
			if ((ray.origin + solution[0] * ray.dir).z <= -1) {
				isectList.push_back(Intersect());
				isectList[isectNum].pos = ray.origin + solution[0] * ray.dir;
				isectList[isectNum].norm = isectList[isectNum].pos - g_spheres[i].origin;
				isectList[isectNum].dist = length(isectList[isectNum].pos);
				isectList[isectNum].sphereNum = i;
				isectNum++;
				intersectFound = true;
			}

			// Save second intersection if within frustrum
			if ((ray.origin + solution[1] * ray.dir).z <= -1) {
				isectList.push_back(Intersect());
				isectList[isectNum].pos = ray.origin + solution[1] * ray.dir;
				isectList[isectNum].norm = isectList[isectNum].pos - g_spheres[i].origin;
				isectList[isectNum].dist = length(isectList[isectNum].pos);
				isectList[isectNum].sphereNum = i;
				isectNum++;
				intersectFound = true;
			}
		}
		else if (determ == 0) {  // If determinant = 0, one intersect
			// Get one solution and find associated intersection
			solution[0] = -1 * (dot(rayPrime.origin, rayPrime.dir) / pow(length(rayPrime.dir), 2)) + (determ / pow(length(rayPrime.dir), 2));

			// Save only intersection if within frustrum
			if ((ray.origin + solution[0] * ray.dir).z <= -1) {
				isectList.push_back(Intersect());
				isectList[isectNum].pos = ray.origin + solution[0] * ray.dir;
				isectList[isectNum].norm = isectList[isectNum].pos - g_spheres[i].origin;
				isectList[isectNum].dist = length(isectList[isectNum].pos);
				isectList[isectNum].sphereNum = i;
				isectNum++;
				intersectFound = true;
			}
		}
	}
	// If found intersection, look for intersection with smallest dist and return true
	if (intersectFound) {
		// Find the intersection with the smallest distance
		intersect = isectList[0];
		for (int j = 1; j < isectNum; j++) {	// Start from 1 because intersect.dist populated with isectList[0].dist initially
			if (isectList[j].dist < intersect.dist) {
				intersect = isectList[j];
			}
		}
		return true;
	}
	else  // Otherwise return false
		return false;
}

// -------------------------------------------------------------------
// Ray tracing

vec4 trace(const Ray& ray, int recurseDepth = 0)
{
    // TODO: implement your ray tracing routine here.
	Intersect intersect;
	Sphere *targetSphere;
	Ray reflectRay;
	vec4 light_dir;
	vec4 color_total;
	vec3 color_ambient = vec3();
	vec3 color_diffuse = vec3();
	vec3 color_specular = vec3();
	vec3 color_local = vec3();
	vec4 color_reflected = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	

	// Find intersection of ray for each sphere and return closest one to camera
	bool isectFound = IntersectRay(ray, intersect);
	if (!isectFound) {  // If no intersection then assign background color to pixel
		return vec4(g_background);
	}
	else {
		// Find local color, which depends on each light source and shadow rays (TODO implement shadow consideration)
		// color_shadow = Sum(shadowRays(P,Lighti))
		// If dot product of surface normal with light direction is negative, then object is shadowing itself, no need to trace ray
		targetSphere = &g_spheres[intersect.sphereNum];

		// Find outgoing (reflected) ray based off incoming ray and intersect normal
		reflectRay.origin = intersect.pos;
		reflectRay.dir = normalize(ray.dir - 2 * dot(ray.dir, intersect.norm) * intersect.norm);		// Need to normalize? TODO

		for (int p = 0; p < lightIndex; p++) {
			// Get light direction from intersect point (normalized)
			light_dir = normalize(g_lights[p].origin - intersect.pos);

			// Sum up diffuse and specular components for each point light
			color_diffuse += targetSphere->Kd * g_lights[p].rgb * dot(intersect.norm, light_dir) * targetSphere->rgb;
			color_specular += targetSphere->Ks * g_lights[p].rgb * pow(dot(reflectRay.dir, -intersect.pos), targetSphere->n);
		}
		color_ambient = targetSphere->Ka * (targetSphere->rgb + g_ambient);
		color_local = color_ambient + color_diffuse + color_specular;

		// Recursive call trace to find color_reflected
		if (recurseDepth > 0) {
			color_reflected = trace(reflectRay, recurseDepth - 1);
		}
	}	

	// Add up colors and scale color_reflected by sphere's Kr
	color_total = vec4(color_local) + targetSphere->Kr * color_reflected;
	return color_total;
}

vec4 getDir(int ix, int iy)
{
    // TODO: modify this. This should return the direction from the origin
    // to pixel (ix, iy), normalized. (DONE but unverified)
    vec4 dir;
	float px;
	float py;
	float pz;
	px = g_left + 2 * g_right*((float)ix / (g_width-1));	// Need (g_width-1) because we iterate from pixel 0 to res_x-1
	py = g_top + 2 * g_bottom*((float)iy / (g_height-1));	// Need (g_height-1) because we iterate from pixel 0 to res_y-1
	pz = -g_near;

	dir = normalize(vec4(px, py, pz, 0.0f));
    return dir;
}

void renderPixel(int ix, int iy)
{
    Ray ray;
    ray.origin = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ray.dir = getDir(ix, iy);
    vec4 color = trace(ray);
    setColor(ix, iy, color);
}

void render()
{
    for (int iy = 0; iy < g_height; iy++)
        for (int ix = 0; ix < g_width; ix++)
            renderPixel(ix, iy);
}


// -------------------------------------------------------------------
// PPM saving

void savePPM(int Width, int Height, char* fname, unsigned char* pixels) 
{
    FILE *fp;
    const int maxVal=255;

    printf("Saving image %s: %d x %d\n", fname, Width, Height);
    fp = fopen(fname,"wb");
    if (!fp) {
        printf("Unable to open file '%s'\n", fname);
        return;
    }
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", Width, Height);
    fprintf(fp, "%d\n", maxVal);

    for(int j = 0; j < Height; j++) {
        fwrite(&pixels[j*Width*3], 3, Width, fp);
    }

    fclose(fp);
}

void saveFile(char *inName)
{
	unsigned char temp;
	// Convert color components from floats to unsigned chars.
    // DONE: clamp values if out of range
    unsigned char* buf = new unsigned char[g_width * g_height * 3];
    for (int y = 0; y < g_height; y++)
        for (int x = 0; x < g_width; x++)
			for (int i = 0; i < 3; i++) {		// Go through r, g, b values (skip alpha channel)
				temp = ((float*)g_colors[y*g_width + x])[i];
				temp = (temp > 1 ? 1 : temp);
				temp = (unsigned char)(temp * 255.9f);
				buf[y*g_width * 3 + x * 3 + i] = temp;
			}
    // DONE: change file name based on input file name.
	savePPM(g_width, g_height, outName, buf);
    delete[] buf;
	free(outName);
}


// -------------------------------------------------------------------
// Main

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: template-rt <input_file.txt>" << endl;
        exit(1);
    }
    loadFile(argv[1]);
    render();
    saveFile(argv[1]);
	return 0;
}

