/*
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
#ifndef XPMPPLANE_H
#define XPMPPLANE_H

#include "XPMPMultiplayerVars.h"
#include "PlaneType.h"
#include "CullInfo.h"

class XPMPMapRendering;

class XPMPPlane {
private:
	// world state
	PlaneType			mPlaneType;

	XPMPPlanePosition_t	mPosition;
	XPMPPlaneSurfaces_t	mSurface;
	XPMPPlaneSurveillance_t	mSurveillance;

	// rendering data
	CSL *				mCSL;
	int					mMatchQuality;

	friend void Render_PrepLists();
	friend class XPMPMapRendering;
public:
	XPMPPlane();
	virtual ~XPMPPlane();

	void setType(const PlaneType &type);
	void setCSL(CSL *csl);
	void setCSL(const PlaneType &type);
	void updateCSL();
	/** upgradeCSL works mostly like setCSL, only it only takes hold if the new
	 * CSL is a higher quality match than the old one.
	 *
	 * @param type
	 * @return true if the type was changed, false otherwise.
	 */
	bool upgradeCSL(const PlaneType &type);
	int  getMatchQuality();

	void updatePosition(const XPMPPlanePosition_t &newPosition);
	void updateSurfaces(const XPMPPlaneSurfaces_t &newSurfaces);
	void updateSurveillance(const XPMPPlaneSurveillance_t &newSurveillance);

	/** Updates the specific plane's instance data and prepares it's tcas
	 * (and culling flags for selfrendered models)
	 *
	 * @param gl_camera the CullInfo from the rendering loop
	 * @returns the square of the distance from the camera
	 */
	float doInstanceUpdate(const CullInfo &gl_camera);

	// instanceData is public for the convenience of the main render loop only.
	CSLInstanceData *	mInstanceData;
};


#endif //XPMPPLANE_H
