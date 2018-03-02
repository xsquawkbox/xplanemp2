//
// Created by kuroneko on 2/03/2018.
//
#include <string>
#include <deque>
#include <queue>
#include <utility>

#include "Obj8CSL.h"
#include "XPMPMultiplayerObj8.h"
#include "XUtils.h"

using namespace std;


std::queue<Obj8Attachment *>	Obj8Attachment::loadQueue;


static void
Obj8Attachment::loadCallback(XPLMObjectRef inObject, void *inRefcon)
{
	auto *sThis = reinterpret_cast<Obj8Attachment *>(inRefcon);

	sThis->mHandle = inObject;
	if (nullptr == inObject) {
		sThis->mLoadState = Obj8LoadState::Failed;
		XPLMDump() << XPMP_CLIENT_NAME << " failed to load obj8: " << sThis->mFile << "\n";
	} else {
		sThis->mLoadState = Obj8LoadState::Loaded;
	}

	if (!loadQueue.empty()) {
		Obj8Attachment *nextAtt = nullptr;
		nextAtt = loadQueue.front();
		loadQueue.pop();
		if (nextAtt) {
			XPLMLoadObjectAsync(nextAtt->mFile.c_str(), &Obj8Attachment::loadCallback, reinterpret_cast<void *>(next));
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
		XPLMLoadObjectAsync(mFile.c_str(), &Obj8Attachment::loadCallback, reinterpret_cast<void *>(next));
	} else {
		loadQueue.push(this);
	}
}

Obj8Attachment::Obj8Attachment(std::string fileName, bool needsAnimation, Obj8DrawType drawType) :
	mFile(fileName), mNeedsAnimation(needsAnimation), mDrawType(drawType)
{
	mHandle = nullptr;
	mLoadState = Obj8LoadState::None;
}

Obj8Attachment::Obj8Attachment(Obj8Attachment &copySrc) :
	mFile(copySrc.mFile), mNeedsAnimation(copySrc.mNeedsAnimation), mDrawType(copySrc.mDrawType)
{

}

Obj8Attachment::Obj8Attachment(Obj8Attachment &&moveSrc) :
	mFile(std::move(moveSrc.mFile)),
	mNeedsAnimation(moveSrc.mNeedsAnimation),
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
	mAttachments.push_back(att);
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

virtual bool needsRenderCallback() override;
/* drawPlane renders the plane when called from within the rendering callback */

void
Obj8CSL::drawPlane(
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
	// prod the attachments to make them load.
	for (auto &att: mAttachments) {
		att.getObjectHandle();
	}


	obj_schedule_one_aircraft(this, x, y, z, pitch, roll, heading, full, lights, state);
}
