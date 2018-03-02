//
// Created by kuroneko on 2/03/2018.
//

#ifndef LEGACYCSL_H
#define LEGACYCSL_H

#include "CSL.h"

bool		LegacyCSL_Init(const char *inTexturePath);
void		LegacyCSL_DeInit();

class LegacyCSL : public CSL {
private:
	void 	_PlotModel(float inDistance);

public:
	LegacyCSL(
		std::vector<std::string> dirNames,
		std::string objFileName,
		std::string	fullPath,
		std::string textureName);


	void setTexture(std::string textureFileName, std::string texturePath, std::string textureLitPath);

	string	getModelName() const override;
	virtual bool	isUsable() const override;

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

protected:
	// plane_Obj
	int obj_idx;
	int texID;            // can be 0 for no customization
	int texLitID;        // can be 0 for no customization

	std::string 		mTexturePath;    // Full path to the planes texture
	std::string 		mTextureLitPath; // Full path to the planes lit texture

	std::string 		mObjectName;	// Basename of the object file
	std::string 		mTextureName;   // Basename of the texture file
	std::string 		mFilePath;		// Where do we load from (oz and obj, debug-use-only for OBJ8)

	OBJ7Handle          mObjHandle;
	TextureHandle       mTexHandle;
	TextureHandle       mTexLitHandle

	ObjManager::TransientState      mObjState;
	TextureManager::TransientState  mTexState;
	TextureManager::TransientState  mTexLitState;
};

#endif //LEGACYCSL_H
