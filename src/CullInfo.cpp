/*
 * Copyright (c) 2004, Ben Supnik and Chris Serio.
 * Copyright (c) 2018, Chris Collins.
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

#include "CullInfo.h"
#include <XPLMDataAccess.h>

#include "XUtils.h"


XPLMDataRef		CullInfo::projectionMatrixRef = nullptr;
XPLMDataRef		CullInfo::modelviewMatrixRef = nullptr;

void
CullInfo::init()
{
	if (modelviewMatrixRef == nullptr) {
		modelviewMatrixRef = XPLMFindDataRef("sim/graphics/view/modelview_matrix");
	} else {
	    XPLMDump() << "Was unable to get the modelview matrix from the simulator.  plane rendering may not work correctly.";
	}
	if (projectionMatrixRef == nullptr) {
		projectionMatrixRef = XPLMFindDataRef("sim/graphics/view/projection_matrix");
	} else {
        XPLMDump() << "Was unable to get the projection matrix from the simulator.  plane rendering may not work correctly.";
    }
}

void 
CullInfo::multMatrixVec4f(float dst[4], const float m[16], const float v[4])
{
	dst[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
	dst[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
	dst[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
	dst[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];
}

void
CullInfo::normalizeMatrix(float vec[4])
{
	if (vec[3] != 0.0) {
		float w = 1.0f / vec[3];
		vec[0] *= w;
		vec[1] *= w;
		vec[2] *= w;
		vec[3] = 1.0;
	}
}


CullInfo::CullInfo()
{
	// First, just read out the current OpenGL matrices...do this once at setup because it's not the fastest thing to do.
	if (modelviewMatrixRef && projectionMatrixRef) {
		XPLMGetDatavf(modelviewMatrixRef, model_view, 0, 16);
		XPLMGetDatavf(projectionMatrixRef, proj, 0, 16);
	}

	// Now...what the heck is this?  Here's the deal: the clip planes have values in "clip" coordinates of: Left = (1,0,0,1)
	// Right = (-1,0,0,1), Bottom = (0,1,0,1), etc.  (Clip coordinates are coordinates from -1 to 1 in XYZ that the driver
	// uses.  The projection matrix converts from eye to clip coordinates.)
	//
	// How do we convert a plane backward from clip to eye coordinates?  Well, we need the transpose of the inverse of the
	// inverse of the projection matrix.  (Transpose of the inverse is needed to transform a plane, and the inverse of the
	// projection is the matrix that goes clip -> eye.)  Well, that cancels out to the transpose of the projection matrix,
	// which is nice because it means we don't need a matrix inversion in this bit of sample code.

	// So this nightmare down here is simply:
	// clip plane * transpose (proj_matrix)
	// worked out for all six clip planes.  If you squint you can see the patterns:
	// L:  1  0 0 1
	// R: -1  0 0 1
	// B:  0  1 0 1
	// T:  0 -1 0 1
	// etc.

	lft_clip[0] = proj[0]+proj[3];	lft_clip[1] = proj[4]+proj[7];	lft_clip[2] = proj[8]+proj[11];	lft_clip[3] = proj[12]+proj[15];
	rgt_clip[0] =-proj[0]+proj[3];	rgt_clip[1] =-proj[4]+proj[7];	rgt_clip[2] =-proj[8]+proj[11];	rgt_clip[3] =-proj[12]+proj[15];

	bot_clip[0] = proj[1]+proj[3];	bot_clip[1] = proj[5]+proj[7];	bot_clip[2] = proj[9]+proj[11];	bot_clip[3] = proj[13]+proj[15];
	top_clip[0] =-proj[1]+proj[3];	top_clip[1] =-proj[5]+proj[7];	top_clip[2] =-proj[9]+proj[11];	top_clip[3] =-proj[13]+proj[15];

	nea_clip[0] = proj[2]+proj[3];	nea_clip[1] = proj[6]+proj[7];	nea_clip[2] = proj[10]+proj[11];nea_clip[3] = proj[14]+proj[15];
	far_clip[0] =-proj[2]+proj[3];	far_clip[1] =-proj[6]+proj[7];	far_clip[2] =-proj[10]+proj[11];far_clip[3] =-proj[14]+proj[15];
}

CullInfo::CullInfo(const CullInfo &src)
{
	for (int c = 0; c < 16; c++) {
		proj[c] = src.proj[c];
		model_view[c] = src.model_view[c];
	}
	for (int c = 0; c < 4; c++) {
		lft_clip[c] = src.lft_clip[c];
		rgt_clip[c] = src.rgt_clip[c];
		bot_clip[c] = src.bot_clip[c];
		top_clip[c] = src.top_clip[c];
		nea_clip[c] = src.nea_clip[c];
		far_clip[c] = src.far_clip[c];
	}
}

bool
CullInfo::checkClip(const float eye[4], const float clip[4], float r)
{
	return ((eye[0] * clip[0] + eye[1] * clip[1] + eye[2] * clip[2] + clip[3] + r) < 0);
}

bool
CullInfo::SphereIsVisible(float x, float y, float z, float r) const
{
	float	objCoord[4] = { x, y, z, 1.0f };
	float	objProj[4];

	// First: we transform our coordinate into eye coordinates from model-view.
	multMatrixVec4f(objProj, model_view, objCoord);
	normalizeMatrix(objProj);

	// Now - we apply the "plane equation" of each clip plane to see how far from the clip plane our point is.
	// The clip planes are directed: positive number distances mean we are INSIDE our viewing area by some distance;
	// negative means outside.  So ... if we are outside by less than -r, the ENTIRE sphere is out of bounds.
	// We are not visible!  We do the near clip plane, then sides, then far, in an attempt to try the planes
	// that will eliminate the most geometry first...half the world is behind the near clip plane, but not much is
	// behind the far clip plane on sunny day.
	if (checkClip(objProj, nea_clip, r)) return false;
	if (checkClip(objProj, bot_clip, r)) return false;
	if (checkClip(objProj, top_clip, r)) return false;
	if (checkClip(objProj, lft_clip, r)) return false;
	if (checkClip(objProj, rgt_clip, r)) return false;
	if (checkClip(objProj, far_clip, r)) return false;
	return true;
}

float
CullInfo::SphereDistanceSqr(float x, float y, float z) const
{
	float xp = x * model_view[0] + y * model_view[4] + z * model_view[ 8] + model_view[12];
	float yp = x * model_view[1] + y * model_view[5] + z * model_view[ 9] + model_view[13];
	float zp = x * model_view[2] + y * model_view[6] + z * model_view[10] + model_view[14];
	return xp*xp+yp*yp+zp*zp;
}

void
CullInfo::ConvertTo2D(float x, float y, float z, float w, float * out_x, float * out_y) const
{
	float	mvVec[4] = {x, y, z, w};
	float	eye[4];
	float	screen[4];

	multMatrixVec4f(eye, model_view, mvVec);
	multMatrixVec4f(screen, proj, eye);
	normalizeMatrix(screen);

	*out_x = screen[0];
	*out_y = screen[1];
}
