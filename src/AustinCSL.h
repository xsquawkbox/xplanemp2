//
// Created by kuroneko on 2/03/2018.
//

#ifndef XSQUAWKBOX_VATSIM_AUSTINCSL_H
#define XSQUAWKBOX_VATSIM_AUSTINCSL_H

#include "CSL.h"

class AustinCSL : public CSL {
protected:
	// plane_Austin
	int 		mPlaneIndex;
	std::string mFilePath;		// Where do we load from (oz and obj, debug-use-only for OBJ8)

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

public:
	AustinCSL(std::string absolutePath);

	std::string	getModelName() const override;
	virtual bool	isUsable() const override;
};


#endif //XSQUAWKBOX_VATSIM_AUSTINCSL_H
