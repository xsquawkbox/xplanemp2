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

#include <map>
#include <vector>
#include <cmath>
#include <cstdio>
#include <queue>
#include <fstream>

#include <XPLMGraphics.h>
#include <XPLMUtilities.h>
#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>

#include "XPMPMultiplayer.h"

#include "XPMPMultiplayerVars.h"
#include "XUtils.h"
#include "legacycsl/LegacyCSL.h"
#include "legacycsl/LegacyObj.h"
#include "legacycsl/TexUtils.h"
#include "legacycsl/XObjReadWrite.h"


#define DEBUG_NORMALS 0
#define	DISABLE_SHARING 0
#define BLEND_NORMALS 1

#ifdef IBM
#define snprintf _snprintf
#endif

// Set this to 1 to get resource loading and unloading diagnostics
#define DEBUG_RESOURCE_CACHE 0


using namespace std;

static void MakePartialPathNativeObj(string& io_str)
{
	//	char sep = *XPLMGetDirectorySeparator();
	for(size_t i = 0; i < io_str.size(); ++i)
		if(io_str[i] == '/' || io_str[i] == ':' || io_str[i] == '\\')
			io_str[i] = '/';
}


bool 	NormalizeVec(float vec[3])
{
	float	len=sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
	if (len>0.0)
	{
		len = 1.0f / len;
		vec[0] *= len;
		vec[1] *= len;
		vec[2] *= len;
		return true;
	}
	return false;
}

void	CrossVec(float a[3], float b[3], float dst[3])
{
	dst[0] = a[1] * b[2] - a[2] * b[1] ;
	dst[1] = a[2] * b[0] - a[0] * b[2] ;
	dst[2] = a[0] * b[1] - a[1] * b[0] ;
}

// Adds a point to our pool and returns it's index.
// If one already exists in the same location, we
// just return that index
int	OBJ_PointPool::AddPoint(float xyz[3], float st[2])
{
#if !DISABLE_SHARING
	// Use x as the key...see if we can find it
	for(size_t n = 0; n < mPointPool.size(); n += 8)
	{
		if((xyz[0] == mPointPool[n]) &&
				(xyz[1] == mPointPool[n+1]) &&
				(xyz[2] == mPointPool[n+2]) &&
				(st[0] == mPointPool[n+3]) &&
				(st[1] == mPointPool[n+4]))
			return static_cast<int>(n/8);	// Clients care about point # not array index
	}
#endif	

	// If we're here, no match was found so we add it to the pool
	// Add XYZ
	mPointPool.push_back(xyz[0]);	mPointPool.push_back(xyz[1]);	mPointPool.push_back(xyz[2]);
	// Add ST
	mPointPool.push_back(st[0]); mPointPool.push_back(st[1]);
	// Allocate some space for the normal later
	mPointPool.push_back(0.0); mPointPool.push_back(0.0); mPointPool.push_back(0.0);
	return (static_cast<int>(mPointPool.size())/8)-1;
}

// This function sets up OpenGL for our point pool
void OBJ_PointPool::PreparePoolToDraw()
{
	// Setup our triangle data (20 represents 5 elements of 4 bytes each
	// namely s,t,xn,yn,zn)
	glVertexPointer(3, GL_FLOAT, 32, &(*mPointPool.begin()));
	// Set our texture data (24 represents 6 elements of 4 bytes each
	// namely xn, yn, zn, x, y, z. We start 3 from the beginning to skip
	// over x, y, z initially.
	glClientActiveTextureARB(GL_TEXTURE1);
	glTexCoordPointer(2, GL_FLOAT, 32, &(*(mPointPool.begin() + 3)));
	glClientActiveTextureARB(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, 32, &(*(mPointPool.begin() + 3)));
	// Set our normal data...
	glNormalPointer(GL_FLOAT, 32, &(*(mPointPool.begin() + 5)));
}

void OBJ_PointPool::CalcTriNormal(int idx1, int idx2, int idx3)
{
	if (mPointPool[idx1*8  ]==mPointPool[idx2*8  ]&&
			mPointPool[idx1*8+1]==mPointPool[idx2*8+1]&&
			mPointPool[idx1*8+2]==mPointPool[idx2*8+2])		return;
	if (mPointPool[idx1*8  ]==mPointPool[idx3*8  ]&&
			mPointPool[idx1*8+1]==mPointPool[idx3*8+1]&&
			mPointPool[idx1*8+2]==mPointPool[idx3*8+2])		return;
	if (mPointPool[idx2*8  ]==mPointPool[idx3*8  ]&&
			mPointPool[idx2*8+1]==mPointPool[idx3*8+1]&&
			mPointPool[idx2*8+2]==mPointPool[idx3*8+2])		return;

	// idx2->idx1 cross idx1->idx3 = normal product
	float	v1[3], v2[3], n[3];
	v1[0] = mPointPool[idx2*8  ] - mPointPool[idx1*8  ];
	v1[1] = mPointPool[idx2*8+1] - mPointPool[idx1*8+1];
	v1[2] = mPointPool[idx2*8+2] - mPointPool[idx1*8+2];

	v2[0] = mPointPool[idx2*8  ] - mPointPool[idx3*8  ];
	v2[1] = mPointPool[idx2*8+1] - mPointPool[idx3*8+1];
	v2[2] = mPointPool[idx2*8+2] - mPointPool[idx3*8+2];
	
	// We do NOT normalize the cross product; we want larger triangles
	// to make bigger normals.  When we blend them, bigger sides will
	// contribute more to the normals.  We'll normalize the normals
	// after the blend is done.
	CrossVec(v1, v2, n);
	mPointPool[idx1*8+5] += n[0];
	mPointPool[idx1*8+6] += n[1];
	mPointPool[idx1*8+7] += n[2];

	mPointPool[idx2*8+5] += n[0];
	mPointPool[idx2*8+6] += n[1];
	mPointPool[idx2*8+7] += n[2];

	mPointPool[idx3*8+5] += n[0];
	mPointPool[idx3*8+6] += n[1];
	mPointPool[idx3*8+7] += n[2];
}

inline void swapped_add(float& a, float& b)
{
	float a_c = a;
	float b_c = b;
	a += b_c;
	b += a_c;
}

void OBJ_PointPool::NormalizeNormals()
{
	// Average all normals of same-point, different texture points?  Why is this needed?
	// Well...the problem is that we get non-blended normals around the 'seam' where the ACF fuselage
	// is put together...at this point the separate top and bottom texes touch.  Their color will
	// be the same but the S&T coords won't.  If we have slightly different normals and the sun is making
	// shiney specular hilites, the discontinuity is real noticiable.
#if BLEND_NORMALS
	for (size_t n = 0; n < mPointPool.size(); n += 8)
	{
		for (size_t m = 0; m < mPointPool.size(); m += 8)
			if (mPointPool[n  ]==mPointPool[m  ] &&
					mPointPool[n+1]==mPointPool[m+1] &&
					mPointPool[n+2]==mPointPool[m+2] &&
					m != n)
			{
				swapped_add(mPointPool[n+5], mPointPool[m+5]);
				swapped_add(mPointPool[n+6], mPointPool[m+6]);
				swapped_add(mPointPool[n+7], mPointPool[m+7]);
			}
	}
#endif	
	for (size_t n = 5; n < mPointPool.size(); n += 8)
	{
		NormalizeVec(&mPointPool[n]);
	}
}

// This is a debug routine that will draw each vertex's normals
void OBJ_PointPool::DebugDrawNormals()
{
	XPLMSetGraphicsState(0, 0, 0, 0, 0, 1, 0);
	glColor3f(1.0, 0.0, 1.0);
	glBegin(GL_LINES);
	for(size_t n = 0; n < mPointPool.size(); n+=8)
	{
		glVertex3f(mPointPool[n], mPointPool[n+1], mPointPool[n+2]);
		glVertex3f(mPointPool[n] + mPointPool[n+5], mPointPool[n+1] + mPointPool[n+1+5],
				mPointPool[n+2] + mPointPool[n+2+5]);
	}
	glEnd();
}

//FIXME: sTexes only appears to be used for global textures (ie: the lights texture) - either broaden the use, or eliminate it.
static	map<string, int>	sTexes;

ObjManager 		gObjManager(OBJ_LoadModelAsync);
TextureManager	gTextureManager(OBJ_LoadTexture);

/*****************************************************
		Utility functions to handle OBJ stuff
******************************************************/

int OBJ_LoadLightTexture(const string &inFilePath, bool inForceMaxTex)
{
	string	path(inFilePath);
	if (sTexes.count(path) > 0)
		return sTexes[path];

	int derez = 5 - gConfiguration.legacyCslOptions.maxResolution;
	if (inForceMaxTex)
		derez = 0;

	GLuint texNum = 0;
	bool ok = LoadTextureFromFile(path, true, false, true, derez, &texNum, NULL, NULL);
	if (!ok) return 0;

	sTexes[path] = texNum;
	return texNum;
}

TextureManager::Future OBJ_LoadTexture(const string &path)
{
	return std::async(std::launch::async, [path]
	{
#if DEBUG_RESOURCE_CACHE
		XPLMDebugString(XPMP_CLIENT_NAME ": Loading texture ");
		XPLMDebugString("(");
		XPLMDebugString(path.c_str());
		XPLMDebugString(")\n");
#endif

		int derez = 5 - gConfiguration.legacyCslOptions.maxResolution;
		ImageInfo im;
		CSLTexture_t texture;
		texture.id = 0;
		if (!LoadImageFromFile(path, true, derez, im, NULL, NULL))
		{
			XPLMDebugString(XPMP_CLIENT_NAME ": WARNING: ");
			XPLMDebugString(path.c_str());
			XPLMDebugString(" failed to load.");
			XPLMDebugString("\n");
			texture.loadStatus = Failed;
			return std::make_shared<CSLTexture_t>(texture);
		}
		if (!VerifyTextureImage(path, im)) {
			// VTI reports it's own errors.
			texture.loadStatus = Failed;
			return std::make_shared<CSLTexture_t>(texture);			
		}
		texture.path = path;
		texture.im = im;
		texture.loadStatus = Succeeded;
		return TextureManager::ResourceHandle(new CSLTexture_t(texture), LegacyCSL::DeleteTexture);
	});
}

void DeleteObjInfo(ObjInfo_t* objInfo)
{
#if DEBUG_RESOURCE_CACHE
	XPLMDebugString(XPMP_CLIENT_NAME ": Released OBJ ");
	XPLMDebugString("(");
	XPLMDebugString(objInfo->path.c_str());
	XPLMDebugString(")\n");
#endif

	delete objInfo;
}

// Load one model - returns nullptr handle if it can't be loaded.
ObjManager::ResourceHandle OBJ_LoadModel(const string &inFilePath)
{
#if DEBUG_RESOURCE_CACHE
		XPLMDebugString(XPMP_CLIENT_NAME ": Loading OBJ ");
		XPLMDebugString("(");
		XPLMDebugString(inFilePath.c_str());
		XPLMDebugString(")\n");
#endif

	ObjInfo_t objInfo;
	string path(inFilePath);

	bool ok = XObjReadWrite::read(path, objInfo.obj);
	if (!ok)
	{
		XPLMDebugString(XPMP_CLIENT_NAME ": WARNING: ");
		XPLMDebugString(path.c_str());
		XPLMDebugString(" failed to load.");
		XPLMDebugString("\n");
		objInfo.loadStatus = Failed;
		return ObjManager::ResourceHandle(new ObjInfo_t(objInfo), DeleteObjInfo);
	}

	MakePartialPathNativeObj(objInfo.obj.texture);
	objInfo.path = path;
	string tex_path(path);
	string::size_type p = tex_path.find_last_of("\\:/");//XPLMGetDirectorySeparator());
	tex_path.erase(p+1);
	tex_path += objInfo.obj.texture;
	tex_path += ".png";

	objInfo.defaultTexture = tex_path;
	// fixme: needed?
	objInfo.texnum = -1;
	objInfo.texnum_lit = -1;
	objInfo.defaultLitTexture = OBJ_GetLitTextureByTexture(objInfo.defaultTexture);

	// We prescan all of the commands to see if there's ANY LOD. If there's
	// not then we need to add one ourselves. If there is, we will find it
	// naturally later.
	bool foundLOD = false;
	for (const auto &cmd : objInfo.obj.cmds)
	{
		if((cmd.cmdType == type_Attr) && (cmd.cmdID == attr_LOD))
			foundLOD = true;
	}
	if(foundLOD == false)
	{
		objInfo.lods.push_back(LODObjInfo_t());
		objInfo.lods.back().nearDist = 0;
		objInfo.lods.back().farDist = 40000;
	}

	// Go through all of the commands for this object and filter out the polys
	// and the lights.
	for (const auto &cmd : objInfo.obj.cmds)
	{
		switch(cmd.cmdType) {
		case type_Attr:
			if(cmd.cmdID == attr_LOD)
			{
				// We've found a new LOD section so save this
				// information in a new struct. From now on and until
				// we hit this again, all data is for THIS LOD instance.
				objInfo.lods.push_back(LODObjInfo_t());
				// Save our visible LOD range
				objInfo.lods.back().nearDist = cmd.attributes[0];
				objInfo.lods.back().farDist = cmd.attributes[1];
			}
			break;
		case type_PtLine:
			if(cmd.cmdID == obj_Light)
			{
				// For each light we've found, copy the data into our
				// own light vector
				for(size_t n = 0; n < cmd.rgb.size(); n++)
				{
					objInfo.lods.back().lights.push_back(LightInfo_t());
					objInfo.lods.back().lights.back().xyz[0] = cmd.rgb[n].v[0];
					objInfo.lods.back().lights.back().xyz[1] = cmd.rgb[n].v[1];
					objInfo.lods.back().lights.back().xyz[2] = cmd.rgb[n].v[2];
					objInfo.lods.back().lights.back().rgb[0] = static_cast<int>(cmd.rgb[n].rgb[0]);
					objInfo.lods.back().lights.back().rgb[1] = static_cast<int>(cmd.rgb[n].rgb[1]);
					objInfo.lods.back().lights.back().rgb[2] = static_cast<int>(cmd.rgb[n].rgb[2]);
				}
			}
			break;
		case type_Poly:
		{
			vector<int> indexes;
			// First get our point pool setup with all verticies
			for(size_t n = 0; n < cmd.st.size(); n++)
			{
				float xyz[3], st[2];
				int index;

				xyz[0] = cmd.st[n].v[0];
				xyz[1] = cmd.st[n].v[1];
				xyz[2] = cmd.st[n].v[2];
				st[0]  = cmd.st[n].st[0];
				st[1]  = cmd.st[n].st[1];
				index = objInfo.lods.back().pointPool.AddPoint(xyz, st);
				indexes.push_back(index);
			}

			switch(cmd.cmdID) {
			case obj_Tri:
				for(size_t n = 0; n < indexes.size(); ++n)
				{
					objInfo.lods.back().triangleList.push_back(indexes[n]);
				}
				break;
			case obj_Tri_Fan:
				for(size_t n = 2; n < indexes.size(); n++)
				{
					objInfo.lods.back().triangleList.push_back(indexes[0  ]);
					objInfo.lods.back().triangleList.push_back(indexes[n-1]);
					objInfo.lods.back().triangleList.push_back(indexes[n  ]);
				}
				break;
			case obj_Tri_Strip:
			case obj_Quad_Strip:
				for(size_t n = 2; n < indexes.size(); n++)
				{
					if((n % 2) == 1)
					{
						objInfo.lods.back().triangleList.push_back(indexes[n - 2]);
						objInfo.lods.back().triangleList.push_back(indexes[n]);
						objInfo.lods.back().triangleList.push_back(indexes[n - 1]);
					}
					else
					{
						objInfo.lods.back().triangleList.push_back(indexes[n - 2]);
						objInfo.lods.back().triangleList.push_back(indexes[n - 1]);
						objInfo.lods.back().triangleList.push_back(indexes[n]);
					}
				}
				break;
			case obj_Quad:
				for(size_t n = 3; n < indexes.size(); n += 4)
				{
					objInfo.lods.back().triangleList.push_back(indexes[n-3]);
					objInfo.lods.back().triangleList.push_back(indexes[n-2]);
					objInfo.lods.back().triangleList.push_back(indexes[n-1]);
					objInfo.lods.back().triangleList.push_back(indexes[n-3]);
					objInfo.lods.back().triangleList.push_back(indexes[n-1]);
					objInfo.lods.back().triangleList.push_back(indexes[n  ]);
				}
				break;
			}
		}
			break;
		}
	}

	// Calculate our normals for all LOD's
	for (size_t i = 0; i < objInfo.lods.size(); i++)
	{
		for (size_t n = 0; n < objInfo.lods[i].triangleList.size(); n += 3)
		{
			objInfo.lods[i].pointPool.CalcTriNormal(
						objInfo.lods[i].triangleList[n],
						objInfo.lods[i].triangleList[n+1],
					objInfo.lods[i].triangleList[n+2]);
		}
		objInfo.lods[i].pointPool.NormalizeNormals();
		objInfo.lods[i].dl = 0;
	}
	objInfo.obj.cmds.clear();
	objInfo.loadStatus = Succeeded;
	return ObjManager::ResourceHandle(new ObjInfo_t(objInfo), DeleteObjInfo);
}

ObjManager::Future OBJ_LoadModelAsync(const string &inFilePath)
{
	return std::async(std::launch::async, [inFilePath]
	{
		return OBJ_LoadModel(inFilePath);
	});
}

std::string OBJ_DefaultModel(const string &path)
{
	XObj xobj;
	int version;
	XObjReadWrite::readHeader(path, version, xobj);
	return xobj.texture;
}

string OBJ_GetLitTextureByTexture(const std::string &texturePath)
{
	static const vector<string> extensions =
	{
		"LIT"
	};
	static const string defaultExtension("_LIT");

	auto position = texturePath.find_last_of('.');
	if(position == std::string::npos) { return {}; }

	for (const auto &extension : extensions)
	{
		string textureLitPath = texturePath;
		textureLitPath.insert(position, extension);

		// Does the file exist?
		if(DoesFileExist(textureLitPath)) { return textureLitPath; }
	}

	// If none of them exist, we return the default "_LIT" without testing.
	// If loading fails later, the user will be properly informed.
	string textureLitPath = texturePath;
	textureLitPath.insert(position, defaultExtension);
	return textureLitPath;
}
