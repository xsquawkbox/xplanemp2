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

#include <memory>
#include <XPLMCamera.h>

#include "XPMPMultiplayer.h"	// for light status
#include "legacycsl/ResourceManager.h"
#include "legacycsl/XObjDefs.h"
#include "XOGLUtils.h"
#include "legacycsl/TexUtils.h"

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

enum LoadStatus { Succeeded, Failed };

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
	string					defaultLitTexture;
	int						texnum;
	int						texnum_lit;
	XObj					obj;
	vector<LODObjInfo_t>	lods;
	LoadStatus loadStatus;
};
using ObjManager = ResourceManager<ObjInfo_t>;
using OBJ7Handle = ObjManager::ResourceHandle;

struct CSLTexture_t
{
	std::string		path;
	ImageInfo		im;
	GLuint			id;
	LoadStatus		loadStatus;
};
using TextureManager = ResourceManager<CSLTexture_t>;
using TextureHandle = TextureManager::ResourceHandle;

bool	OBJ_Init(const char * inTexturePath);

ObjManager::ResourceHandle OBJ_LoadModel(const std::string &inFilePath);
ObjManager::Future OBJ_LoadModelAsync(const std::string &inFilePath);

// Get name of objects default model
std::string OBJ_DefaultModel(const std::string &path);

// Texture loading
int		OBJ_LoadLightTexture(const std::string &inFilePath, bool inForceMaxTex);
TextureManager::Future OBJ_LoadTexture(const std::string &path);

std::string OBJ_GetLitTextureByTexture(const std::string &texturePath);

extern ObjManager 		gObjManager;
extern TextureManager	gTextureManager;

#endif
