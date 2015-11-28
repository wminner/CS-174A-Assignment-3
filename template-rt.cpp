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
	float Ka;
	float Kd;
	float Ks;
	float Kr;
	float n;
};

struct Light
{
	string name;
	vec4 origin;
	vec3 rgb;
};

// Resolution and pixel colors
int g_width;
int g_height;
vector<vec4> g_colors;

// Other colors
vec3 background;
vec3 ambient;

// Frustrum
float g_left;
float g_right;
float g_top;
float g_bottom;
float g_near;

// Output filename
char *outName;

// Spheres
Sphere spheres[5];
int sphereIndex = 0;

// Lights
Light lights[5];
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
			spheres[sphereIndex].name = vs[1];
			spheres[sphereIndex].origin = toVec4(vs[2], vs[3], vs[4]);
			spheres[sphereIndex].scale = Scale(vec3(toFloat(vs[5]), toFloat(vs[6]), toFloat(vs[7])));
			spheres[sphereIndex].rgb = vec3(toFloat(vs[8]), toFloat(vs[9]), toFloat(vs[10]));
			spheres[sphereIndex].Ka = toFloat(vs[11]);
			spheres[sphereIndex].Kd = toFloat(vs[12]);
			spheres[sphereIndex].Ks = toFloat(vs[13]);
			spheres[sphereIndex].Kr = toFloat(vs[14]);
			spheres[sphereIndex].n = toFloat(vs[15]);

			sphereIndex++;
			break;
		case 7:		// LIGHT
			lights[lightIndex].name = vs[1];
			lights[lightIndex].origin = toVec4(vs[2], vs[3], vs[4]);
			lights[lightIndex].rgb = vec3(toFloat(vs[5]), toFloat(vs[6]), toFloat(vs[7]));

			lightIndex++;
			break;
		case 8:		// BACK
			background = vec3(toFloat(vs[1]), toFloat(vs[2]), toFloat(vs[3]));
			break;
		case 9:		// AMBIENT
			ambient = vec3(toFloat(vs[1]), toFloat(vs[2]), toFloat(vs[3]));
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

// TODO: add your ray-sphere intersection routine here.


// -------------------------------------------------------------------
// Ray tracing

vec4 trace(const Ray& ray)
{
    // TODO: implement your ray tracing routine here.
    return vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

vec4 getDir(int ix, int iy)
{
    // TODO: modify this. This should return the direction from the origin
    // to pixel (ix, iy), normalized.
    vec4 dir;
    dir = vec4(0.0f, 0.0f, -1.0f, 0.0f);
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
	// The following is DEPRICATED in order to use the output name given in the file
	//int len = strlen(inName);
	//char *outName = (char*) malloc(len+1);
	//strcpy(outName, inName);
	//outName[len-3] = 'p';
	//outName[len-2] = 'p';
	//outName[len-1] = 'm';
	//outName[len] = '\0';
	
	// Convert color components from floats to unsigned chars.
    // TODO: clamp values if out of range.
    unsigned char* buf = new unsigned char[g_width * g_height * 3];
    for (int y = 0; y < g_height; y++)
        for (int x = 0; x < g_width; x++)
            for (int i = 0; i < 3; i++)
                buf[y*g_width*3+x*3+i] = (unsigned char)(((float*)g_colors[y*g_width+x])[i] * 255.9f);
    
    // DONE: change file name based on input file name.
    //savePPM(g_width, g_height, "output.ppm", buf);
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
	//cout << argv[0] << endl;
	//cout << argv[1] << endl;
    loadFile(argv[1]);
    render();
    saveFile(argv[1]);

	//for (int i = 0; i < sphereIndex; i++) {
	//	delete &spheres[i];
	//}
	//for (int j = 0; j < lightIndex; j++) {
	//	delete &lights[j];
	//}
	//delete[] spheres;
	//delete[] lights;

	return 0;
}

