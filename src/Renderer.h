/*
 * Copyright (c) 2004, Ben Supnik and Chris Serio.
 * Copyright (c) 2018, 2020, Chris Collins.
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

#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <vector>

#include <XPLMScenery.h>
#include <XPLMDataAccess.h>

extern XPLMDataRef		gVisDataRef;		// Current air visiblity for culling.
extern XPLMProbeRef		gTerrainProbe;

extern double	Render_FullPlaneDistance;

struct Label {
	double		x;
	double		y;
	float 		distSqr;
	std::string	labelText;
};

void	Renderer_Init();
void	Renderer_Attach_Callbacks();
void	Renderer_Detach_Callbacks();


#endif //RENDERER_H
