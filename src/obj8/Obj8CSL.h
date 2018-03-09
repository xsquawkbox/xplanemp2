//
// Created by kuroneko on 2/03/2018.
//

#ifndef OBJ8CSL_H
#define OBJ8CSL_H

#include <queue>

#include <XPLMScenery.h>

#include "CullInfo.h"
#include "CSL.h"
#include "obj8/InstanceWrapper.h"

enum class Obj8DrawType {
	None,
	LightsOnly,
	LowLevelOfDetail,
	Solid
};

enum class Obj8LoadState {
	None = 0,		// not loaded, no attempt yet.
	Loading,		// async load requested.
	Loaded,			// (a)sync load complete
	Failed			// (a)sync load failed
};

class Obj8Attachment {
private:
	static void	loadCallback(XPLMObjectRef inObject, void *inRefcon);
	static std::queue<Obj8Attachment *>	loadQueue;

	void enqueueLoad();
public:
	Obj8Attachment(std::string fileName, Obj8DrawType drawType);
	Obj8Attachment(const Obj8Attachment &copySrc);
	Obj8Attachment(Obj8Attachment &&moveSrc);

	virtual ~Obj8Attachment();

	/** try to get the object handle.  Queue it for loading if it's not available.
	 * @returns The XPLMObjectRef for this attachment
	 */
	XPLMObjectRef	getObjectHandle();

	Obj8DrawType		mDrawType;

protected:
	std::string			mFile;
	XPLMObjectRef		mHandle;
	Obj8LoadState		mLoadState;
};

class Obj8InstanceData : public CSLInstanceData {
public:
	xp11InstanceRef		mainInstance;
	Obj8DrawType 		mainType;

	Obj8InstanceData();
	~Obj8InstanceData();

	friend class Obj8CSL;

protected:
	virtual void updateInstance(
		CSL *csl,
		double x,
		double y,
		double z,
		double pitch,
		double roll,
		double heading,
		xpmp_LightStatus lights,
		XPLMPlaneDrawState_t *state) override;

private:
	void resetModel();
};

class Obj8CSL : public CSL
{
protected:
	std::string 					mObjectName;     // Basename of the object file
	std::vector<Obj8Attachment>		mAttachments;
public:
	static std::vector<float> 		dref_values; // from Obj8CSL.cpp

	virtual void newInstanceData(CSLInstanceData *&newInstanceData) const override;

	Obj8CSL(std::vector<std::string> dirNames, std::string objectName);
	void addAttachment(Obj8Attachment &att);
	void addAttachment(Obj8Attachment &&att);

	Obj8Attachment *	getAttachmentFor(Obj8DrawType drawType);

	std::string	getModelName() const override;
	std::string getModelType() const override;

	static void Init();
};


#endif //XSQUAWKBOX_VATSIM_OBJ8CSL_H
