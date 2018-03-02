#if IBM
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#elif APL
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <XPLMUtilities.h>

#include "XPMPMultiplayer.h"
#include "XPMPMultiplayerCSL.h"
#include "XPMPMultiplayerObj.h"
#include "XOGLUtils.h"
#include "GlDebug.h"
#include "XUtils.h"
#include "LegacyCSL.h"
#include "XObjReadWrite.h"

bool
LegacyCSL_Init(const char *inTexturePath)
{

	// Set up OpenGL for our drawing callbacks
	OGL_UtilsInit();

#ifdef DEBUG_GL
	XPLMDebugString(XPMP_CLIENT_NAME ": WARNING: This build includes OpenGL Debugging\n");
	XPLMDebugString("    OpenGL Debugging induces a large overhead and produces large logfiles.\n");
	XPLMDebugString("    Please do not use this build other than as directed.\n");
#endif

	OGLDEBUG(XPLMDebugString(XPMP_CLIENT_NAME " - GL supports debugging\n"));
	OGLDEBUG(glEnable(GL_DEBUG_OUTPUT));
	OGLDEBUG(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
	OGLDEBUG(XPMPSetupGLDebug());

	xpmp_tex_useAnisotropy = OGL_HasExtension("GL_EXT_texture_filter_anisotropic");
	if (xpmp_tex_useAnisotropy) {
		GLfloat maxAnisoLevel;

		XPLMDebugString(XPMP_CLIENT_NAME " - GL supports anisoptropic filtering.\n");

		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisoLevel);
		xpmp_tex_maxAnisotropy = maxAnisoLevel;
	}
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &xpmp_tex_maxSize);

	obj_init();
	bool ok = OBJ_Init(inTexturePath);
	if (!ok)
		XPLMDump() << XPMP_CLIENT_NAME " WARNING: we failed to find xpmp's custom lighting texture at " << inTexturePath << ".\n";
	return ok;
}

void
LegacyCSL_DeInit()
{
	obj_deinit();
}

LegacyCSL::LegacyCSL(
	std::vector<std::string> dirNames,
	std::string objFileName,
	std::string	fullPath,
	std::string textureName) :
	CSL(dirNames),
	mObjectName(objFileName),
	mFilePath(fullPath),
	mTextureName(textureName)
{
}

void
LegacyCSL::setTexture(std::string textureFileName, std::string texturePath, std::string textureLitPath)
{
	mTextureName = textureFileName;
	mTexturePath = texturePath;
	mTextureLitPath = textureLitPath;
}

string
LegacyCSL::getModelName() const
{
	string modelName = "";
	for (const auto &dir: mDirNames) {
		modelName += dir;
		modelName += ' ';
	}
	modelName += mObjectName;
	if (!mTextureName.empty()) {
		modelName += ' ';
		modelName += mTextureName;
	}
	return modelName;
}

bool
LegacyCSL::isUsable() const
{
	return obj_idx != -1;
}

virtual bool
LegacyCSL::needsRenderCallback()
{
	// legacy CSL aircraft are always rendered by us.
	return true;
}


bool
LegacyCSL::getResources()
{
	if (!mObjHandle) {
		mObjHandle = gObjManager.get(mFilePath, &mObjState);
		if (mObjHandle && mObjHandle->loadStatus == Failed) {
			// Failed to load
			XPLMDebugString("Skipping ");
			XPLMDebugString(getModelName().c_str());
			XPLMDebugString(" since object could not be loaded.");
			XPLMDebugString("\n");
		}
	}
	if (!mObjHandle || mObjHandle->loadStatus == Failed) {
		return;
	}

	// Try to load a texture if not yet done. If one can't be loaded continue without texture
	if (!mTexHandle) {
		string texturePath = mTexturePath;
		if (mTexturePath.empty()) {
			mTexturePath = mObjHandle->defaultTexture;
		}
		mTexHandle = gTextureManager.get(texturePath, &mTexState);

		// Async loading completed with failure
		if (mTexHandle && mTexHandle->loadStatus == Failed) {
			// Failed to load
			XPLMDebugString("Texture for ");
			XPLMDebugString(getModelName().c_str());
			XPLMDebugString(" cannot be loaded.");
			XPLMDebugString("\n");
		}
	}

	auto model = plane->model;
	// Try to load a texture if not yet done. If one can't be loaded continue without texture
	if (! plane->texLitHandle)
	{
		string texturePath = model->textureLitPath;
		if (texturePath.empty()) { texturePath = plane->objHandle->defaultLitTexture; }
		plane->texLitHandle = gTextureManager.get(texturePath, &plane->texLitState);
	}

	if (plane->texHandle && plane->texHandle->loadStatus == Succeeded && !plane->texHandle->id)
	{
		plane->texHandle->id = xpmpGetGlTextureId();
		LoadTextureFromMemory(plane->texHandle->im, true, false, true, plane->texHandle->id);

#if DEBUG_RESOURCE_CACHE
		XPLMDebugString(XPMP_CLIENT_NAME ": Finished loading of texture id=");
		char buf[32];
		sprintf(buf,"%d", plane->texHandle->id);
		XPLMDebugString(buf);
		XPLMDebugString("\n");
#endif
	}

	if (plane->texLitHandle && plane->texLitHandle->loadStatus == Succeeded && !plane->texLitHandle->id)
	{
		plane->texLitHandle->id = xpmpGetGlTextureId();
		LoadTextureFromMemory(plane->texLitHandle->im, true, false, true, plane->texLitHandle->id);

#if DEBUG_RESOURCE_CACHE
		XPLMDebugString(XPMP_CLIENT_NAME ": Finished loading of texture id=");
		char buf[32];
		sprintf(buf,"%d", plane->texLitHandle->id);
		XPLMDebugString(buf);
		XPLMDebugString("\n");
#endif
	}

}


void
LegacyCSL::drawPlane(
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
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
	glRotatef(static_cast<GLfloat>(heading), 0.0, -1.0, 0.0);
	glRotatef(static_cast<GLfloat>(pitch), 01.0, 0.0, 0.0);
	glRotatef(static_cast<GLfloat>(roll), 0.0, 0.0, -1.0);

	_PlotModel(full?distance:max(distance, 10000.0f));

	glPopMatrix();
};


