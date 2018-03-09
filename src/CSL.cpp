//
// Created by kuroneko on 2/03/2018.
//

#include <string>
#include <XPLMDataAccess.h>
#include <XPLMScenery.h>

#include "CSL.h"
#include "CullInfo.h"
#include "XPMPMultiplayerVars.h"
#include "Renderer.h"
#include "TCASHack.h"

using namespace std;

void
RenderedCSLInstanceData::updateInstance(
	CSL *csl,
	double x,
	double y,
	double z,
	double pitch,
	double roll,
	double heading,
	xpmp_LightStatus lights,
	XPLMPlaneDrawState_t *state)
{
	mX = x;
	mY = y;
	mZ = z;
	mPitch = pitch;
	mRoll = roll;
	mHeading = heading;
	mLights = lights;
	mFull = true;
	if (mDistanceSqr > (Render_FullPlaneDistance * Render_FullPlaneDistance)) {
		mFull = false;
	}

	memcpy(&mState, state, sizeof(mState));
};

CSL::CSL()
{
	mOffsetSource = VerticalOffsetSource::None;
	mMovingGear = true;
}

CSL::CSL(std::vector<std::string> dirNames) :
	mDirNames(dirNames)
{
	mMovingGear = true;
	mOffsetSource = VerticalOffsetSource::None;
}

double
CSL::getVertOffset() const
{
	switch (mOffsetSource) {
	case VerticalOffsetSource::None:
		return 0.0;
	case VerticalOffsetSource::Model:
		return mModelVertOffset;
	case VerticalOffsetSource::Mtl:
		return mMtlVertOffset;
	case VerticalOffsetSource::Preference:
		return mPreferencesVertOffset;
	}
	return 0.0;
}

VerticalOffsetSource
CSL::getVertOffsetSource() const
{
	return mOffsetSource;
}

void
CSL::setVertOffsetSource(VerticalOffsetSource offsetSource)
{
	mOffsetSource = offsetSource;
}

void
CSL::setVerticalOffset(VerticalOffsetSource src, double offset)
{
	switch(src) {
	case VerticalOffsetSource::None:
		break;
	case VerticalOffsetSource::Model:
		mModelVertOffset = offset;
		break;
	case VerticalOffsetSource::Mtl:
		mMtlVertOffset = offset;
		break;
	case VerticalOffsetSource::Preference:
		mPreferencesVertOffset = offset;
		break;
	}
	if (src > mOffsetSource) {
		mOffsetSource = src;
	}
}

void
CSL::setMovingGear(bool movingGear)
{
	mMovingGear = movingGear;
}

bool
CSL::getMovingGear() const
{
	return mMovingGear;
}

void
CSL::setICAO(std::string icaoCode)
{
	mICAO = icaoCode;
}

void
CSL::setAirline(std::string icaoCode, std::string airline)
{
	setICAO(icaoCode);
	mAirline = airline;
}

void
CSL::setLivery(std::string icaoCode, std::string airline, std::string livery)
{
	setAirline(icaoCode, airline);
	mLivery = livery;
}

std::string
CSL::getICAO() const {
	return mICAO;
}

std::string
CSL::getAirline() const {
	return mAirline;
}

std::string
CSL::getLivery() const {
	return mLivery;
}

bool
CSL::isUsable() const {
	return true;
}

void
CSL::drawPlane(CSLInstanceData *instanceData, bool is_blend, int data) const
{
}

void
CSL::newInstanceData(CSLInstanceData *&newInstanceData) const
{
	newInstanceData = new RenderedCSLInstanceData();
}

void
CSL::updateInstance(
	const CullInfo &cullInfo,
	double &x,
	double &y,
	double &z,
	double roll,
	double heading,
	double pitch,
	XPLMPlaneDrawState_t *state,
	xpmp_LightStatus lights,
	CSLInstanceData *&instanceData)
{
	if (instanceData == nullptr) {
		newInstanceData(instanceData);
	}
	if (instanceData == nullptr) {
		return;
	}

	// clamp to the surface if enabled
	if (gConfiguration.enableSurfaceClamping) {
		XPLMProbeInfo_t	probeResult = {
			sizeof(XPLMProbeInfo_t),
		};
		XPLMProbeResult r = XPLMProbeTerrainXYZ(gTerrainProbe, x, y, z, &probeResult);
		if (r == xplm_ProbeHitTerrain) {
			float minY = probeResult.locationY + getVertOffset();
			if (y < minY) {
				y = minY;
				instanceData->mClamped = true;
			} else {
				instanceData->mClamped = false;
			}
		}
	}
	instanceData->mDistanceSqr = cullInfo.SphereDistanceSqr(x, y, z);

	// TCAS checks.
	instanceData->mTCAS = true;
	// If the plane is farther than our TCAS range, it's just not visible.  Drop it!
	if (instanceData->mDistanceSqr> (kMaxDistTCAS * kMaxDistTCAS)) {
		instanceData->mTCAS = false;
	}

	// we need to assess cull state so we can work out if we need to render labels or not
	instanceData->mCulled = false;
	// cull if the aircraft is not visible due to poor horizontal visibility
	if (gVisDataRef) {
		float horizVis = XPLMGetDataf(gVisDataRef);
		if (instanceData->mDistanceSqr > horizVis*horizVis) {
			instanceData->mCulled = true;
		}
	}
	instanceData->updateInstance(this, x, y, z, pitch, roll, heading, lights, state);
}