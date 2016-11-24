/*
 * Copyright (c) 2005, Ben Supnik and Chris Serio.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef XPLMMULTIPLAYEROBJ_H
#define XPLMMULTIPLAYEROBJ_H

#include "XPMPMultiplayer.h"	// for light status
#include "XObjDefs.h"
#include "XPLMCamera.h"
#include "XOGLUtils.h"

struct XPMPPlane_t;

/*****************************************************
			Ben's Crazy Point Pool Class
******************************************************/

class	OBJ_PointPool {
public:
	OBJ_PointPool(){}
	~OBJ_PointPool(){}

	int AddPoint(float xyz[3], float st[2]);
	void PreparePoolToDraw();
	void CalcTriNormal(int idx1, int idx2, int idx3);
	void NormalizeNormals(void);
	void DebugDrawNormals();
	void Purge() { mPointPool.clear(); }
	int Size() { return static_cast<int>(mPointPool.size()); }
private:
	vector<float>	mPointPool;
};

/*****************************************************
			Object and Struct Definitions
******************************************************/

struct	LightInfo_t {
	float			xyz[3];
	int				rgb[3];
};

// One of these structs per LOD read from the OBJ file
struct	LODObjInfo_t {

	float					nearDist;	// The visible range
	float					farDist;	// of this LOD
	vector<int>				triangleList;
	vector<LightInfo_t>		lights;
	OBJ_PointPool			pointPool;
	GLuint					dl;
};

// One of these structs per OBJ file
struct	ObjInfo_t {

	string					path;
	string                  defaultTexture;
	int						texnum;
	int						texnum_lit;
	XObj					obj;
	vector<LODObjInfo_t>	lods;
};

bool	OBJ_Init(const char * inTexturePath);

// Load one model - return -1 if it can't be loaded.
int		OBJ_LoadModel(const char * inFilePath);

// Get name of objects default model
std::string OBJ_DefaultModel(int model);

// MODEL DRAWING
// Note that texID and litTexID are OPTIONAL! They will only be filled
// in if the user wants to override the default texture specified by the
// obj file
void	OBJ_PlotModel(int model, int texID, int litTexID, float inDistance, double inX, double inY, 
					  double inZ, double inPitch, double inRoll, double inHeading);

// TEXTURED LIGHTS DRAWING
void	OBJ_BeginLightDrawing();
void	OBJ_DrawLights(int model, float inDistance, double inX, double inY,
					   double inZ, double inPitch, double inRoll, double inHeading,
					   xpmp_LightStatus lights);

// Texture loading
int		OBJ_LoadTexture(const char * inFilePath, bool inForceMaxRes);
int		OBJ_GetModelTexID(int model);


#endif
