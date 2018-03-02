//
// Created by kuroneko on 2/03/2018.
//

#include <string>
#include "AustinCSL.h"

using namespace std;


AustinCSL::AustinCSL(std::string absolutePath) :
	CSL(),
	mFilePath(absolutePath),
	mPlaneIndex(-1)
{
}

string
AustinCSL::getModelName() const
{
	return mFilePath;
}

bool
AustinCSL::isUsable() const
{
	return mPlaneIndex != -1;
}


bool
AustinCSL::needsRenderCallback()
{
	return true;
}

void
AustinCSL::drawPlane(
	float distance,
	double x,
	double y,
	double z,
	double pitch,
	double roll,
	double heading,
	int full,
	xpmp_LightStatus lights,
	XPLMPlaneDrawState_t *state,
	void *&instanceData)
{
	XPLMPluginID who;
	int total, active;
	XPLMCountAircraft(&total, &active, &who);

	if (mPlaneIndex > 0 && mPlaneIndex < active) {
		XPLMDrawAircraft(
			mPlaneIndex ,
			static_cast<GLfloat>(x),
			static_cast<GLfloat>(y),
			static_cast<GLfloat>(z),
			static_cast<GLfloat>(pitch),
			static_cast<GLfloat>(roll),
			static_cast<GLfloat>(heading),
			full,
			state);
	}
}
