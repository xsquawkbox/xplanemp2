//
// Created by kuroneko on 2/03/2018.
//

#ifndef LEGACYCSL_LEGACYCSL_H
#define LEGACYCSL_LEGACYCSL_H

#include <map>
#include <queue>

#include "CSL.h"
#include "XPMPPlane.h"
#include "legacycsl/LegacyObj.h"

#define MAX_SPARE_TEXHANDLES			4
#define SPARE_TEXHANDLES_DECAY_FRAMES	120

class LegacyCSL : public CSL {
private:
	bool 	getResources();
	void 	_PlotModel(const CSLInstanceData *instanceData) const;
	void 	_PlotLights(const CSLInstanceData *instanceData) const;
	static bool		triedInit;


public:
	static bool Init(const char *inTexturePath);
	static void DeInit();

	LegacyCSL(
		std::vector<std::string> dirNames,
		std::string objFileName,
		std::string	fullPath,
		std::string textureName);


	void setTexture(std::string textureFileName, std::string texturePath, std::string textureLitPath);

	std::string		getModelName() const override;
	std::string		getModelType() const;
	virtual bool	isUsable() const override;

	/* drawPlane renders the plane when called from within the rendering callback */
	virtual void	drawPlane(CSLInstanceData *instanceData, bool is_blend, int data) const override;

	virtual void 	updateInstance(
		const CullInfo &cullInfo,
		double &x,
		double &y,
		double &z,
		double roll,
		double heading,
		double pitch,
		XPLMPlaneDrawState_t *state,
		xpmp_LightStatus lights,
		CSLInstanceData *&instanceData) override;

protected:
	// plane_Obj
	int obj_idx;

	std::string 		mTexturePath;    // Full path to the planes texture
	std::string 		mTextureLitPath; // Full path to the planes lit texture

	std::string 		mObjectName;	// Basename of the object file
	std::string 		mTextureName;   // Basename of the texture file
	std::string 		mFilePath;		// Where do we load from (oz and obj, debug-use-only for OBJ8)

	OBJ7Handle          mObjHandle;
	TextureHandle       mTexHandle;
	TextureHandle       mTexLitHandle;

	ObjManager::TransientState      mObjState;
	TextureManager::TransientState  mTexState;
	TextureManager::TransientState  mTexLitState;

	/* --- Global Rendering parts of LegacyCSL --- */

	/* --- Internal Renderer Resources --- */
private:
	static int 					gSpareTexHandleDecayFrames;
	static std::queue<GLuint>	gFreedTextures;
	static void					_MaintainTextures();
public:
	static GLuint 				GetGlTextureId();
	static void					DeleteTexture(CSLTexture_t* texture);

	/* --- rendering --- */
private:
	static	XPLMDataRef			sFOVRef;
	static	float				sFOV;
	static	int					sLightTexture;

	struct render_list_item {
		LegacyCSL		*csl;
		RenderedCSLInstanceData *instanceData;
	};

	typedef std::multimap<float, render_list_item>	render_list_type;
	static	render_list_type gRenderList;

	static void		_BeginLightDrawing();
public:

	static void 	cleanFrame();
	static void 	doRender(int isBlend);

};

#endif //LEGACYCSL_LEGACYCSL_H
