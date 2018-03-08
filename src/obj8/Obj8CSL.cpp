//
// Created by kuroneko on 2/03/2018.
//
#include <string>
#include <deque>
#include <queue>
#include <utility>
#include <XPLMScenery.h>
#include <XPLMPlanes.h>
#include <CullInfo.h>
#include <XPMPMultiplayerVars.h>
#include <XPLMDataAccess.h>

#include "Obj8CSL.h"
#include "XPMPMultiplayerObj8.h"
#include "XUtils.h"

using namespace std;

std::queue<Obj8Attachment *>	Obj8Attachment::loadQueue;

static const char * dref_names[] = {
	"libxplanemp/controls/gear_ratio",
	"libxplanemp/controls/flap_ratio",
	"libxplanemp/controls/spoiler_ratio",
	"libxplanemp/controls/speed_brake_ratio",
	"libxplanemp/controls/slat_ratio",
	"libxplanemp/controls/wing_sweep_ratio",
	"libxplanemp/controls/thrust_ratio",
	"libxplanemp/controls/yoke_pitch_ratio",
	"libxplanemp/controls/yoke_heading_ratio",
	"libxplanemp/controls/yoke_roll_ratio",
	"libxplanemp/controls/thrust_revers",
	"libxplanemp/controls/taxi_lites_on",
	"libxplanemp/controls/landing_lites_on",
	"libxplanemp/controls/beacon_lites_on",
	"libxplanemp/controls/strobe_lites_on",
	"libxplanemp/controls/nav_lites_on",
	nullptr
};

std::vector<float> Obj8CSL::dref_values;

typedef void (* XPLMSetDataf_f)(
	void *               inRefcon,
	float                inValue);
typedef float (* XPLMGetDataf_f)(
	void *               inRefcon);

static float
obj8_dref_read(void *inRefcon)
{
	intptr_t idx = reinterpret_cast<intptr_t>(inRefcon);
	if (idx > static_cast<intptr_t>(Obj8CSL::dref_values.size()) || idx < 0) {
		return 0.0f;
	}
	return Obj8CSL::dref_values[idx];
}

static void
obj8_dref_write(void *inRefcon, float inValue)
{
	intptr_t idx = reinterpret_cast<intptr_t>(inRefcon);
	if (idx > static_cast<intptr_t>(Obj8CSL::dref_values.size()) || idx < 0) {
		return;
	}
	Obj8CSL::dref_values[idx] = inValue;
}

void
Obj8CSL::Init()
{
	// set up the animation datarefs
	for (intptr_t n = 0; dref_names[n] != nullptr; n++) {
		XPLMRegisterDataAccessor(dref_names[n],
			xplmType_Float,
			true,  // writable
			nullptr, nullptr, // int
			obj8_dref_read, obj8_dref_write, // float
			nullptr, nullptr, // double
			nullptr, nullptr, // int array
			nullptr, nullptr, // float array
			nullptr, nullptr, // data
			reinterpret_cast<void *>(n), // read refcon
			reinterpret_cast<void *>(n)	 // write refcon
		);
		Obj8CSL::dref_values.emplace_back(0.0);
	}
}

void
Obj8Attachment::loadCallback(XPLMObjectRef inObject, void *inRefcon)
{
	auto *sThis = reinterpret_cast<Obj8Attachment *>(inRefcon);

	sThis->mHandle = inObject;
	if (nullptr == inObject) {
		sThis->mLoadState = Obj8LoadState::Failed;
		XPLMDump() << XPMP_CLIENT_NAME << " failed to load obj8: " << sThis->mFile << "\n";
	} else {
		XPLMDump() << XPMP_CLIENT_NAME << " did load obj8: " << sThis->mFile << "\n";
		sThis->mLoadState = Obj8LoadState::Loaded;
	}

	if (!loadQueue.empty()) {
		Obj8Attachment *nextAtt = nullptr;
		nextAtt = loadQueue.front();
		loadQueue.pop();
		if (nextAtt) {
			XPLMLoadObjectAsync(nextAtt->mFile.c_str(), &Obj8Attachment::loadCallback, reinterpret_cast<void *>(nextAtt));
		}
	}
}

void
Obj8Attachment::enqueueLoad()
{
	if (mLoadState != Obj8LoadState::None) {
		return;
	}
	if (mFile.empty()) {
		return;
	}
	mLoadState = Obj8LoadState::Loading;
	if (loadQueue.empty()) {
		XPLMLoadObjectAsync(mFile.c_str(), &Obj8Attachment::loadCallback, reinterpret_cast<void *>(this));
	} else {
		loadQueue.push(this);
	}
}

Obj8Attachment::Obj8Attachment(std::string fileName, Obj8DrawType drawType) :
	mFile(fileName), mDrawType(drawType)
{
	mHandle = nullptr;
	mLoadState = Obj8LoadState::None;
}

Obj8Attachment::Obj8Attachment(const Obj8Attachment &copySrc) :
	mFile(copySrc.mFile), mDrawType(copySrc.mDrawType)
{
	mHandle = nullptr;
	mLoadState = Obj8LoadState::None;
}

Obj8Attachment::Obj8Attachment(Obj8Attachment &&moveSrc) :
	mFile(std::move(moveSrc.mFile)),
	mDrawType(moveSrc.mDrawType)
{
	mHandle = moveSrc.mHandle;
	moveSrc.mHandle = nullptr;

	mLoadState = moveSrc.mLoadState;
	moveSrc.mLoadState = Obj8LoadState::None;
}

Obj8Attachment::~Obj8Attachment()
{
	if (mHandle != nullptr) {
		XPLMUnloadObject(mHandle);
		mLoadState = Obj8LoadState::None;
	}
}

XPLMObjectRef
Obj8Attachment::getObjectHandle()
{
	switch (mLoadState) {
	case Obj8LoadState::None:
		enqueueLoad();
		return nullptr;
	case Obj8LoadState::Loaded:
		return mHandle;
	case Obj8LoadState::Failed:
	case Obj8LoadState::Loading:
		return nullptr;
	}
	return nullptr;
}

Obj8InstanceData::Obj8InstanceData() {
	mainInstance = nullptr;
	mainType = Obj8DrawType::None;
}

Obj8InstanceData::~Obj8InstanceData() {
	resetModel();
}

void
Obj8InstanceData::resetModel()
{
	if (mainInstance) {
		(*xp11DestroyInstance)(mainInstance);
		mainInstance = nullptr;
	}
	mainType = Obj8DrawType::None;
}

void
Obj8InstanceData::updateInstance(
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
	Obj8CSL *myCSL = reinterpret_cast<Obj8CSL *>(csl);

	// determine which instance type we want.
	//FIXME: use lowlod + lights as appropriate.
	Obj8DrawType desiredObj = Obj8DrawType::Solid;
	if (mDistanceSqr > (gConfiguration.maxFullAircraftRenderingDistance * gConfiguration.maxFullAircraftRenderingDistance)) {
		desiredObj = Obj8DrawType::LightsOnly;
		if (!myCSL->getAttachmentFor(desiredObj)) {
			desiredObj = Obj8DrawType::LowLevelOfDetail;
			if (!myCSL->getAttachmentFor(desiredObj)) {
				desiredObj = Obj8DrawType::Solid;
			}
		}
	}
	if (mainType != desiredObj) {
		auto att = myCSL->getAttachmentFor(desiredObj);
		if (att != nullptr) {
			auto objHandle = att->getObjectHandle();
			if (objHandle != nullptr) {
				// unload the old object (if applicable) and create the new one.
				resetModel();
				mainInstance = (*xp11CreateInstance)(objHandle, dref_names);
				mainType = att->mDrawType;
			}
		}
	}

	// build the state objects.
	XPLMDrawInfo_t	objPosition = {};

	objPosition.structSize = sizeof(objPosition);
	objPosition.x = x;
	objPosition.y = y;
	objPosition.z = z;
	objPosition.heading = heading;
	objPosition.pitch = pitch;
	objPosition.roll = roll;

	// these must be in the same order as defined by dref_names
	float		dataRefValues[] = {
		state->gearPosition,
		state->flapRatio,
		state->spoilerRatio,
		state->speedBrakeRatio,
		state->slatRatio,
		state->wingSweep,
		state->thrust,
		state->yokePitch,
		state->yokeHeading,
		state->yokeRoll,
		static_cast<float>((state->thrust < 0.0) ? 1.0 : 0.0),
		static_cast<float>(lights.taxiLights),
		static_cast<float>(lights.landLights),
		static_cast<float>(lights.bcnLights),
		static_cast<float>(lights.strbLights),
		static_cast<float>(lights.navLights)
	};
	if (mainInstance) {
		(*xp11InstanceSetPosition)(mainInstance, &objPosition, dataRefValues);
	}
}

Obj8CSL::Obj8CSL(std::vector<std::string> dirNames, std::string objectName) :
	CSL(dirNames),
	mObjectName(objectName)
{
}

void
Obj8CSL::addAttachment(Obj8Attachment &att)
{
	mAttachments.push_back(att);
}

void
Obj8CSL::addAttachment(Obj8Attachment &&att)
{
	mAttachments.push_back(std::move(att));
}

string
Obj8CSL::getModelName() const
{
	string modelName = "";
	for (const auto &dir: mDirNames) {
		modelName += dir;
		modelName += ' ';
	}
	modelName += mObjectName;
	return modelName;
}

std::string
Obj8CSL::getModelType() const
{
	return "Obj8";
}


Obj8Attachment *
Obj8CSL::getAttachmentFor(Obj8DrawType drawType)
{
	for (auto &att: mAttachments) {
		if (att.mDrawType == drawType) {
			return &att;
		}
	}
	return nullptr;
}

void
Obj8CSL::newInstanceData(CSLInstanceData *&newInstanceData) const
{
	newInstanceData = new Obj8InstanceData();
}

