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

#include <XPLMDataAccess.h>

// This struct has everything we need to cull fast!
class CullInfo
{
public:
    /** Prepares the static data required for cullInfo, such as dataref
     * handles.  Must be called (once!) before attempting to instantiate a
     * CulLInfo.
     */
    static void init();

    /** Creates a new CullInfo, copying the current projection and modelview
     * matrices from the simulator.
     */
    CullInfo();

    CullInfo(const CullInfo &src);

    /** SphereIsVisible performs a visibility check at the location (in view
     * coordinates) x,y,z to determine if a sphere of radius r would be visible.
     */
    bool SphereIsVisible(float x, float y, float z, float r) const;


    /** SphereDistanceSqr returns the square of the distance from the camera to
     * provided world position.
     *
     * @param x
     * @param y
     * @param z
     *
     * @return distance squared in world units.
     */
    float SphereDistanceSqr(float x, float y, float z) const;

    /** ConvertTo2D projects the provided world coordinates into screen
     * coordinates using the projection & modelview matrices in the CullInfo
     * object
     */
    void ConvertTo2D(float x, float y, float z, float w, float *out_x, float *out_y) const;

protected:
    float model_view[16];	// The model view matrix, to get from local OpenGL to eye coordinates.
    float proj[16];			// Proj matrix - this is just a hack to use for gluProject.
    float nea_clip[4];		// Four clip planes in the form of Ax + By + Cz + D = 0 (ABCD are in the array.)
    float far_clip[4];		// They are oriented so the positive side of the clip plane is INSIDE the view volume.
    float lft_clip[4];
    float rgt_clip[4];
    float bot_clip[4];
    float top_clip[4];

private:
	static XPLMDataRef	projectionMatrixRef;
	static XPLMDataRef	modelviewMatrixRef;

	static void multMatrixVec4f(float dst[4], const float m[16], const float v[4]);
	static void normalizeMatrix(float vec[4]);
	static bool	checkClip(const float eye[4], const float clip[4], float r);
};


#endif //CULLINFO_H
