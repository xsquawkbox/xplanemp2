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

#ifndef CULLINFO_H
#define CULLINFO_H

#if IBM
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <GL/gl.h>
#elif APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <XPLMDataAccess.h>

// This struct has everything we need to cull fast!
class CullInfo
{
private:
	static XPLMDataRef	projectionMatrixRef;
	static XPLMDataRef	modelviewMatrixRef;
	static XPLMDataRef	viewportRef;
	static bool 		useGlFallback;

	static void multMatrixVec4f(float dst[4], const float m[16], const float v[4]);
	static void normalizeMatrix(float vec[4]);
	static bool	checkClip(const float eye[4], const float clip[4], float r);

protected:
	float model_view[16];	// The model view matrix, to get from local OpenGL to eye coordinates.
	float proj[16];			// Proj matrix - this is just a hack to use for gluProject.
	float nea_clip[4];		// Four clip planes in the form of Ax + By + Cz + D = 0 (ABCD are in the array.)
	float far_clip[4];		// They are oriented so the positive side of the clip plane is INSIDE the view volume.
	float lft_clip[4];
	float rgt_clip[4];
	float bot_clip[4];
	float top_clip[4];

public:
	static void init();

	CullInfo();
	CullInfo(const CullInfo &src);

	static void GetCurrentViewport(GLfloat vp[]);

	bool SphereIsVisible(float x, float y, float z, float r) const;

	float SphereDistanceSqr(float x, float y, float z) const;

	void ConvertTo2D(float x, float y, float z, float w, float *out_x, float *out_y) const;
	static void ProjectToViewport(const float *vp, float x, float y, float *out_x, float *out_y);
};


#endif //CULLINFO_H
