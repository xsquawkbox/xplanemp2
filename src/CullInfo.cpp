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

#if IBM
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#elif APL
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <XPLMDataAccess.h>

#include "CullInfo.h"

XPLMDataRef		CullInfo::projectionMatrixRef = nullptr;
XPLMDataRef		CullInfo::modelviewMatrixRef = nullptr;
XPLMDataRef		CullInfo::viewportRef = nullptr;

void
CullInfo::init()
{
	if (modelviewMatrixRef == nullptr) {
		modelviewMatrixRef = XPLMFindDataRef("sim/graphics/view/modelview_matrix");
	}
	if (projectionMatrixRef == nullptr) {
		projectionMatrixRef = XPLMFindDataRef("sim/graphics/view/projection_matrix");
	}
	if (viewportRef == nullptr) {
		viewportRef = XPLMFindDataRef("sim/graphics/view/viewport");
	}
}

CullInfo::CullInfo()
{
	// First, just read out the current OpenGL matrices...do this once at setup because it's not the fastest thing to do.
	if (!modelviewMatrixRef || !projectionMatrixRef) {
		glGetFloatv(GL_MODELVIEW_MATRIX, model_view);
		glGetFloatv(GL_PROJECTION_MATRIX, proj);
	} else {
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

void
CullInfo::GetCurrentViewport(GLfloat vp[])
{
	if (viewportRef) {
		int tempvp[4];
		XPLMGetDatavi(viewportRef, tempvp, 0, 4);
		vp[0] = static_cast<GLfloat>(tempvp[0]); // left
		vp[1] = static_cast<GLfloat>(tempvp[1]); // bottom
		vp[2] = static_cast<GLfloat>(tempvp[2] - tempvp[0]);
		vp[3] = static_cast<GLfloat>(tempvp[3] - tempvp[1]);
	} else {
		glGetFloatv(GL_VIEWPORT, vp);
	}
}

bool
CullInfo::SphereIsVisible(float x, float y, float z, float r) const
{
	// First: we transform our coordinate into eye coordinates from model-view.
	float xp = x * model_view[0] + y * model_view[4] + z * model_view[ 8] + model_view[12];
	float yp = x * model_view[1] + y * model_view[5] + z * model_view[ 9] + model_view[13];
	float zp = x * model_view[2] + y * model_view[6] + z * model_view[10] + model_view[14];

	// Now - we apply the "plane equation" of each clip plane to see how far from the clip plane our point is.
	// The clip planes are directed: positive number distances mean we are INSIDE our viewing area by some distance;
	// negative means outside.  So ... if we are outside by less than -r, the ENTIRE sphere is out of bounds.
	// We are not visible!  We do the near clip plane, then sides, then far, in an attempt to try the planes
	// that will eliminate the most geometry first...half the world is behind the near clip plane, but not much is
	// behind the far clip plane on sunny day.
	if ((xp * nea_clip[0] + yp * nea_clip[1] + zp * nea_clip[2] + nea_clip[3] + r) < 0)	return false;
	if ((xp * bot_clip[0] + yp * bot_clip[1] + zp * bot_clip[2] + bot_clip[3] + r) < 0)	return false;
	if ((xp * top_clip[0] + yp * top_clip[1] + zp * top_clip[2] + top_clip[3] + r) < 0)	return false;
	if ((xp * lft_clip[0] + yp * lft_clip[1] + zp * lft_clip[2] + lft_clip[3] + r) < 0)	return false;
	if ((xp * rgt_clip[0] + yp * rgt_clip[1] + zp * rgt_clip[2] + rgt_clip[3] + r) < 0)	return false;
	if ((xp * far_clip[0] + yp * far_clip[1] + zp * far_clip[2] + far_clip[3] + r) < 0)	return false;
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
CullInfo::ConvertTo2D(const float * vp, float x, float y, float z, float w, float * out_x, float * out_y) const
{
	float xe = x * model_view[0] + y * model_view[4] + z * model_view[ 8] + w * model_view[12];
	float ye = x * model_view[1] + y * model_view[5] + z * model_view[ 9] + w * model_view[13];
	float ze = x * model_view[2] + y * model_view[6] + z * model_view[10] + w * model_view[14];
	float we = x * model_view[3] + y * model_view[7] + z * model_view[11] + w * model_view[15];

	float xc = xe * proj[0] + ye * proj[4] + ze * proj[ 8] + we * proj[12];
	float yc = xe * proj[1] + ye * proj[5] + ze * proj[ 9] + we * proj[13];
	//	float zc = xe * proj[2] + ye * proj[6] + ze * proj[10] + we * proj[14];
	float wc = xe * proj[3] + ye * proj[7] + ze * proj[11] + we * proj[15];

	xc /= wc;
	yc /= wc;
	//	zc /= wc;

	*out_x = vp[0] + (1.0f + xc) * vp[2] / 2.0f;
	*out_y = vp[1] + (1.0f + yc) * vp[3] / 2.0f;
}
