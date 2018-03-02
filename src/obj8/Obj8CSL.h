//
// Created by kuroneko on 2/03/2018.
//

#ifndef OBJ8CSL_H
#define OBJ8CSL_H

#include <XPLMScenery.h>
#include "CSL.h"

enum class Obj8DrawType {
	LightsOnly = 0,
	LowLevelOfDetail,
	Solid,
	Glass
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
	Obj8Attachment(std::string fileName, bool needsAnimation, Obj8DrawType drawType=Obj8DrawType::Solid);
	Obj8Attachment(Obj8Attachment &copySrc);
	Obj8Attachment(Obj8Attachment &&moveSrc);

	virtual ~Obj8Attachment();

	/** try to get the object handle.  Queue it for loading if it's not available.
	 * @returns The XPLMObjectRef for this attachment
	 */
	XPLMObjectRef	getObjectHandle();

protected:
	std::string			mFile;
	XPLMObjectRef		mHandle;
	Obj8DrawType		mDrawType;
	Obj8LoadState		mLoadState;
	bool				mNeedsAnimation;
};


class Obj8CSL : public CSL
{
protected:
	std::string 					mObjectName;     // Basename of the object file
	std::vector<Obj8Attachment>		mAttachments;
public:
	Obj8CSL(std::vector<std::string> dirNames, std::string objectName);
	void addAttachment(Obj8Attachment &att);
	void addAttachment(Obj8Attachment &&att);

	std::string	getModelName() const override;

	virtual bool needsRenderCallback() override;
	/* drawPlane renders the plane when called from within the rendering callback */
	virtual void drawPlane(
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
		void *&instanceData) override;

};


#endif //XSQUAWKBOX_VATSIM_OBJ8CSL_H
