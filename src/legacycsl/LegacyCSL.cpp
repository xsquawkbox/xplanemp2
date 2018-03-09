#if IBM
#define WIN32_MEAN_AND_LEAN
#include <Windows.h> 
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
#include <queue>
#include <cmath>

#include <XPLMGraphics.h>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include "XPMPMultiplayer.h"
#include "CSLLibrary.h"
#include "LegacyObj.h"
#include "XOGLUtils.h"
#include "GlDebug.h"
#include "XUtils.h"
#include "LegacyCSL.h"
#include "XObjReadWrite.h"

using namespace std;

const	double	kMetersToNM = 0.000539956803;
// Some color constants
//const	float	kNavLightRed[] = {1.0, 0.0, 0.2, 0.6};
//const	float	kNavLightGreen[] = {0.0, 1.0, 0.3, 0.6};
//const	float	kLandingLight[]	= {1.0, 1.0, 0.7, 0.6};
//const	float	kStrobeLight[]	= {1.0, 1.0, 1.0, 0.4};
const	float	kNavLightRed[] = { 1.0f, 0.0f, 0.2f, 0.5f };
const	float	kNavLightGreen[] = { 0.0f, 1.0f, 0.3f, 0.5f };
const	float	kLandingLight[] = { 1.0f, 1.0f, 0.7f, 0.6f };
const	float	kStrobeLight[] = { 1.0f, 1.0f, 1.0f, 0.7f };

LegacyCSL::render_list_type	LegacyCSL::gRenderList;

bool LegacyCSL::triedInit = false;

bool
LegacyCSL::Init(const char *inTexturePath)
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

	if (!triedInit) {
		sLightTexture = OBJ_LoadLightTexture(inTexturePath, true);
		triedInit = true;
	}
	bool ok = (sLightTexture != 0);


	if (!ok)
		XPLMDump() << XPMP_CLIENT_NAME " WARNING: we failed to find the LegacyCSL lighting texture at " << inTexturePath << ".\n";
	return ok;
}

void
LegacyCSL::DeInit()
{
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

std::string 
LegacyCSL::getModelType() const
{
	return "Legacy";
}


bool
LegacyCSL::isUsable() const
{
	return obj_idx != -1;
}

enum LegacyCSLDrawPhase_e {
	LegacyCSL_Models,
	LegacyCSL_Lights,
};

void
LegacyCSL::drawPlane(CSLInstanceData *instanceData, bool is_blend, int data) const
{
	auto *i = reinterpret_cast<RenderedCSLInstanceData *>(instanceData);
	if (nullptr == i) {
		return;
	}
	switch(data) {
	case LegacyCSL_Models:
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(static_cast<GLfloat>(i->mX), static_cast<GLfloat>(i->mY), static_cast<GLfloat>(i->mZ));
		glRotatef(static_cast<GLfloat>(i->mHeading), 0.0, -1.0, 0.0);
		glRotatef(static_cast<GLfloat>(i->mPitch), 01.0, 0.0, 0.0);
		glRotatef(static_cast<GLfloat>(i->mRoll), 0.0, 0.0, -1.0);

		_PlotModel(instanceData);
		glPopMatrix();
		break;
	case LegacyCSL_Lights:
		_PlotLights(instanceData);
		break;
	}
}

void
LegacyCSL::doRender(int isBlend)
{
	if (isBlend) {
		CullInfo camera;
		for (const auto &p: gRenderList) {
			if (camera.SphereIsVisible(
				static_cast<float>(p.second.instanceData->mX),
				static_cast<float>(p.second.instanceData->mY),
				static_cast<float>(p.second.instanceData->mZ),
				50.0)) {
				p.second.csl->drawPlane(p.second.instanceData, isBlend, LegacyCSL_Models);
			}
		}
		_BeginLightDrawing();
		for (const auto &p: gRenderList) {
			if (camera.SphereIsVisible(
				static_cast<float>(p.second.instanceData->mX),
				static_cast<float>(p.second.instanceData->mY),
				static_cast<float>(p.second.instanceData->mZ),
				50.0)) {
				p.second.csl->drawPlane(p.second.instanceData, isBlend, LegacyCSL_Lights);
			}
		}
	}
}

void
LegacyCSL::updateInstance(
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
	CSL::updateInstance(cullInfo, x, y, z, roll, heading, pitch, state, lights, instanceData);
	if (nullptr != instanceData && !instanceData->mCulled) {
		// if we are to render this CSL, make sure the resources are ready.
		if (getResources()) {
			gRenderList.emplace(instanceData->mDistanceSqr, render_list_item{this, reinterpret_cast<RenderedCSLInstanceData*>(instanceData)});
		}
	}
}

void
LegacyCSL::cleanFrame()
{
	_MaintainTextures();
	gRenderList.clear();
}


int 					LegacyCSL::gSpareTexHandleDecayFrames = -1;
std::queue<GLuint>		LegacyCSL::gFreedTextures;


/* _MaintainTextures should be called every frame we render.  It cleans up the
 * spare texture pool so it doesn't eat texture memory unnecessarily.
 */
void
LegacyCSL::_MaintainTextures() {
	if (gSpareTexHandleDecayFrames >= 0) {
		if (--gSpareTexHandleDecayFrames <= 0) {
			GLuint textures[] = { GetGlTextureId() };
			if (textures[0] != 0) {
				glDeleteTextures(1, textures);
			}
			// we don't need to reset the timer ourselves, using xpmpGetGlTextureId has done that for us.
		}
	}
}


/*
 * GetGlTextureId tries to return a spare texture handle from our pool of
 * unused GL texture handles.
 *
 * if we're out, return 0.  LoadTextureFromMemory knows that a textureid of 0 means
 * that it has to get a new handle from the GL layer anyway.
 *
 * If we have any textures left after this operation, we bump the timer for texture handle release.
 */
GLuint 
LegacyCSL::GetGlTextureId() {
	GLuint	retHandle = 0;
	if (!gFreedTextures.empty()) {
		retHandle = gFreedTextures.front();
		gFreedTextures.pop();
	}
	if (gFreedTextures.empty()) {
		gSpareTexHandleDecayFrames = -1;
	} else {
		gSpareTexHandleDecayFrames = SPARE_TEXHANDLES_DECAY_FRAMES;
	}
	return retHandle;
}

void
LegacyCSL::DeleteTexture(CSLTexture_t* texture)
{
#if DEBUG_RESOURCE_CACHE
	XPLMDebugString(XPMP_CLIENT_NAME ": Released texture id=");
	char buf[32];
	sprintf(buf,"%d", texture->id);
	XPLMDebugString(buf);
	XPLMDebugString(" (");
	XPLMDebugString(texture->path.c_str());
	XPLMDebugString(")\n");
#endif
	if (gFreedTextures.size() >= MAX_SPARE_TEXHANDLES) {
		GLuint textures[] = { texture->id };
		glDeleteTextures(1, textures);
	} else {
		gFreedTextures.push(texture->id);
	}
	delete texture;
}

/*****************************************************
			Aircraft Model Drawing
******************************************************/

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
		return false;
	}

	// Try to load a texture if not yet done. If one can't be loaded continue without texture
	if (!mTexHandle) {
		string texturePath = mTexturePath;
		if (mTexturePath.empty()) {
			mTexturePath = mObjHandle->defaultTexture;
		}
		mTexHandle = gTextureManager.get(texturePath, &mTexState);
#if 0
		// Async loading completed with failure
		if (mTexHandle && mTexHandle->loadStatus == Failed) {
			// Failed to load
			XPLMDebugString("Texture for ");
			XPLMDebugString(getModelName().c_str());
			XPLMDebugString(" cannot be loaded.");
			XPLMDebugString("\n");
			return false;
		}
#endif
	}

	// Try to load a texture if not yet done. If one can't be loaded continue without texture
	if (!mTexLitHandle)	{
		string texturePath = mTextureLitPath;
		if (texturePath.empty()) {
			texturePath = mObjHandle->defaultLitTexture;
		}
		mTexLitHandle = gTextureManager.get(texturePath, &mTexLitState);
	}

	// now that we have the textures, load them into memory.
	if (mTexHandle && mTexHandle->loadStatus == Succeeded && !mTexHandle->id) {
		mTexHandle->id = GetGlTextureId();
		LoadTextureFromMemory(mTexHandle->im, true, false, true, mTexHandle->id);
#if DEBUG_RESOURCE_CACHE
		XPLMDebugString(XPMP_CLIENT_NAME ": Finished loading of texture id=");
		char buf[32];
		sprintf(buf,"%d", mTexHandle->id);
		XPLMDebugString(buf);
		XPLMDebugString("\n");
#endif
	}

	if (mTexLitHandle && mTexLitHandle->loadStatus == Succeeded && !mTexLitHandle->id) {
		mTexLitHandle->id = GetGlTextureId();
		LoadTextureFromMemory(mTexLitHandle->im, true, false, true, mTexLitHandle->id);
#if DEBUG_RESOURCE_CACHE
		XPLMDebugString(XPMP_CLIENT_NAME ": Finished loading of texture id=");
		char buf[32];
		sprintf(buf,"%d", mTexLitHandle->id);
		XPLMDebugString(buf);
		XPLMDebugString("\n");
#endif
	}
	if (!mTexHandle->id) {
		return false;
	}
	return true;
}

// Note that texID and litTexID are OPTIONAL! They will only be filled
// in if the user wants to override the default texture specified by the
// obj file
void
LegacyCSL::_PlotModel(const CSLInstanceData *instanceData) const
{
	auto *i = reinterpret_cast<const RenderedCSLInstanceData *>(instanceData);

	if (!mObjHandle || mObjHandle->loadStatus == Failed) {
		return;
	}
	if (!mTexHandle->id) {
		return;
	}

	auto obj = mObjHandle.get();

	// Find out what LOD we need to draw
	int lodIdx = -1;
	for(size_t n = 0; n < obj->lods.size(); n++) {
		if((i->mDistanceSqr >= (obj->lods[n].nearDist * obj->lods[n].nearDist))
			&& (i->mDistanceSqr <= (obj->lods[n].farDist * obj->lods[n].farDist))) {
			lodIdx = static_cast<int>(n);
			break;
		}
	}

	// If we didn't find a good LOD bin, we don't draw!
	if(lodIdx == -1)
		return;

	// pointPool is and always was empty! returning early
	if(obj->lods[lodIdx].pointPool.Size()==0 && obj->lods[lodIdx].dl == 0)
		return;

	static XPLMDataRef	night_lighting_ref = XPLMFindDataRef("sim/graphics/scenery/percent_lights_on");
	bool	use_night = XPLMGetDataf(night_lighting_ref) > 0.25;

	int tex = 0;
	int lit = 0;
	auto texture = mTexHandle.get();
	if(texture && texture->id) {
		tex = texture->id;
	}

	auto litTexure = mTexLitHandle.get();
	if (litTexure && litTexure->id) {
		lit = litTexure->id;
	}

	if (!use_night)	lit = 0;
	if (tex == 0) lit = 0;
	XPLMSetGraphicsState(1, (tex != 0) + (lit != 0), 1, 1, 1, 1, 1);
	if (tex != 0)	XPLMBindTexture2d(tex, 0);
	if (lit != 0)	XPLMBindTexture2d(lit, 1);

	if (tex) { glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); }
	if (lit) { glActiveTextureARB(GL_TEXTURE1); glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD); glActiveTextureARB(GL_TEXTURE0); }

	if (obj->lods[lodIdx].dl == 0)
	{
		obj->lods[lodIdx].dl = glGenLists(1);

		GLint xpBuffer = 0;

		//FIXME: is half of this necessary now?  see https://developer.x-plane.com/?article=openglstate
		// See if the card even has VBO. If it does, save xplane's pointer
		// and bind to 0 for us.
#if IBM
		if(glBindBufferARB)
#endif
		{
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING_ARB, &xpBuffer);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
		// Save XPlanes OpenGL state
		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
		// Setup OpenGL pointers to our pool
		obj->lods[lodIdx].pointPool.PreparePoolToDraw();
		// Enable vertex data sucking
		glEnableClientState(GL_VERTEX_ARRAY);
		// Enable normal array sucking
		glEnableClientState(GL_NORMAL_ARRAY);
		// Enable texture coordinate data sucking
		glClientActiveTextureARB(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTextureARB(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		// Disable colors - maybe x-plane left it around.
		glDisableClientState(GL_COLOR_ARRAY);

		glNewList(obj->lods[lodIdx].dl, GL_COMPILE);
		// Kick OpenGL and draw baby!
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(obj->lods[lodIdx].triangleList.size()),
			GL_UNSIGNED_INT, &(*obj->lods[lodIdx].triangleList.begin()));

#if DEBUG_NORMALS
		obj->lods[lodIdx].pointPool.DebugDrawNormals();
		XPLMSetGraphicsState(1, (tex != 0) + (lit != 0), 1, 1, 1, 1, 1);
#endif

		glEndList();

		// Disable vertex data sucking
		glDisableClientState(GL_VERTEX_ARRAY);
		// Disable texture coordinate data sucking
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		// Disable normal array sucking
		glDisableClientState(GL_NORMAL_ARRAY);

		// Restore Xplane's OpenGL State
		glPopClientAttrib();

		// If we bound before, we need to put xplane back where it was
#if IBM
		if(glBindBufferARB)
#endif
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, xpBuffer);

		obj->lods[lodIdx].triangleList.clear();
		obj->lods[lodIdx].pointPool.Purge();
	}
	glCallList(obj->lods[lodIdx].dl);
}

XPLMDataRef			LegacyCSL::sFOVRef = XPLMFindDataRef("sim/graphics/view/field_of_view_deg");
float				LegacyCSL::sFOV = 60.0;
int					LegacyCSL::sLightTexture = -1;


/*****************************************************
			Textured Lights Drawing

 Draw one or more lights on our OBJ.
 RGB of 11,11,11 is a RED NAV light
 RGB of 22,22,22 is a GREEN NAV light
 RGB of 33,33,33 is a Red flashing BEACON light
 RGB of 44,44,44 is a White flashing STROBE light
 RGB of 55,55,55 is a landing light
******************************************************/
void
LegacyCSL::_BeginLightDrawing()
{
	sFOV = XPLMGetDataf(sFOVRef);

	// Setup OpenGL for the drawing
	XPLMSetGraphicsState(1, 1, 0,   1, 1 ,   1, 0);
	XPLMBindTexture2d(sLightTexture, 0);
}

void
LegacyCSL::_PlotLights(const CSLInstanceData *instanceData) const
{
	const auto *i = reinterpret_cast<const RenderedCSLInstanceData *>(instanceData);

	bool navLights = i->mLights.navLights == 1;
	bool bcnLights = i->mLights.bcnLights == 1;
	bool strbLights = i->mLights.strbLights == 1;
	bool landLights = i->mLights.landLights == 1;

	if (! mObjHandle) { return; }
	auto obj = mObjHandle.get();
	int offset = i->mLights.timeOffset;

	// flash frequencies
	if(bcnLights) {
		bcnLights = false;
		int x = (int)(XPLMGetElapsedTime() * 1000 + offset) % 1200;
		switch(i->mLights.flashPattern) {
		case xpmp_Lights_Pattern_EADS:
			// EADS pattern: two flashes every 1.2 seconds
			if(x < 120 || ((x > 240 && x < 360))) bcnLights = true;
			break;

		case xpmp_Lights_Pattern_GA:
			// GA pattern: 900ms / 1200ms
			if((((int)(XPLMGetElapsedTime() * 1000 + offset) % 2100) < 900)) bcnLights = true;
			break;

		case xpmp_Lights_Pattern_Default:
		default:
			// default pattern: one flash every 1.2 seconds
			if(x < 120) bcnLights = true;
			break;
		}

	}
	if(strbLights) {
		strbLights = false;
		int x = (int)(XPLMGetElapsedTime() * 1000 + offset) % 1700;
		switch(i->mLights.flashPattern) {
		case xpmp_Lights_Pattern_EADS:
			if(x < 80 || (x > 260 && x < 340)) strbLights = true;
			break;

		case xpmp_Lights_Pattern_GA:
			// similar to the others.. but a little different frequency :)
			x = (int)(XPLMGetElapsedTime() * 1000 + offset) % 1900;
			if(x < 100) strbLights = true;
			break;

		case xpmp_Lights_Pattern_Default:
		default:
			if(x < 80) strbLights = true;
			break;
		}
	}

	// Find out what LOD we need to draw
	int lodIdx = -1;
	for(size_t n = 0; n < obj->lods.size(); n++)
	{
		if((i->mDistanceSqr >= (obj->lods[n].nearDist * obj->lods[n].nearDist)) &&
			(i->mDistanceSqr <= (obj->lods[n].farDist * obj->lods[n].farDist)))
		{
			lodIdx = static_cast<int>(n);
			break;
		}
	}
	// If we didn't find a good LOD bin, we don't draw!
	if(lodIdx == -1)
		return;

	GLfloat size;
	// Where are we looking?
	XPLMCameraPosition_t cameraPos;
	XPLMReadCameraPosition(&cameraPos);

	// We can have 1 or more lights on each aircraft
	for(size_t n = 0; n < obj->lods[lodIdx].lights.size(); n++)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		// First we translate to our coordinate system and move the origin
		// to the center of our lights.
		glTranslatef(obj->lods[lodIdx].lights[n].xyz[0],
			obj->lods[lodIdx].lights[n].xyz[1],
			obj->lods[lodIdx].lights[n].xyz[2]);

		// Now we undo the rotation of the plane
		glRotated(-i->mRoll, 0.0, 0.0, -1.0);
		glRotated(-i->mPitch, 1.0, 0.0, 0.0);
		glRotated(-i->mHeading, 0.0, -1.0, 0.0);

		// Now we undo the rotation of the camera
		// NOTE: The order and sign of the camera is backwards
		// from what we'd expect (the plane rotations) because
		// the camera works backwards. If you pan right, everything
		// else moves left!
		glRotated(cameraPos.heading, 0.0, -1.0, 0.0);
		glRotated(cameraPos.pitch, 1.0, 0.0, 0.0);
		glRotated(cameraPos.roll, 0.0, 0.0, -1.0);

		// Find our distance from the camera
		float dx = cameraPos.x - static_cast<float>(i->mX);
		float dy = cameraPos.y - static_cast<float>(i->mY);
		float dz = cameraPos.z - static_cast<float>(i->mZ);
		double distance = sqrt((dx * dx) + (dy * dy) + (dz * dz));

		// Convert to NM
		distance *= kMetersToNM;

		// Scale based on our FOV and Zoom. I did my initial
		// light adjustments at a FOV of 60 so thats why
		// I divide our current FOV by 60 to scale it appropriately.
		distance *= sFOV/60.0;
		distance /= cameraPos.zoom;

		// Calculate our light size. This is piecewise linear. I noticed
		// that light size changed more rapidly when closer than 3nm so
		// I have a separate equation for that.
		if(distance <= 3.6)
			size = (10.0f * static_cast<GLfloat>(distance)) + 1.0f;
		else
			size = (6.7f * static_cast<GLfloat>(distance)) + 12.0f;

		// Finally we can draw our lights
		// Red Nav
		glBegin(GL_QUADS);
		if((obj->lods[lodIdx].lights[n].rgb[0] == 11) &&
			(obj->lods[lodIdx].lights[n].rgb[1] == 11) &&
			(obj->lods[lodIdx].lights[n].rgb[2] == 11))
		{
			if(navLights) {
				glColor4fv(kNavLightRed);
				glTexCoord2f(0.0f, 0.5f); glVertex2f(-(size/2.0f), -(size/2.0f));
				glTexCoord2f(0.0f, 1.0f); glVertex2f(-(size/2.0f), (size/2.0f));
				glTexCoord2f(0.25f, 1.0f); glVertex2f((size/2.0f), (size/2.0f));
				glTexCoord2f(0.25f, 0.5f); glVertex2f((size/2.0f), -(size/2.0f));
			}
		}
			// Green Nav
		else if((obj->lods[lodIdx].lights[n].rgb[0] == 22) &&
				(obj->lods[lodIdx].lights[n].rgb[1] == 22) &&
				(obj->lods[lodIdx].lights[n].rgb[2] == 22))
			{
				if(navLights) {
					glColor4fv(kNavLightGreen);
					glTexCoord2f(0.0f, 0.5f); glVertex2f(-(size/2.0f), -(size/2.0f));
					glTexCoord2f(0.0f, 1.0f); glVertex2f(-(size/2.0f), (size/2.0f));
					glTexCoord2f(0.25f, 1.0f); glVertex2f((size/2.0f), (size/2.0f));
					glTexCoord2f(0.25f, 0.5f); glVertex2f((size/2.0f), -(size/2.0f));
				}
			}
				// Beacon
			else if((obj->lods[lodIdx].lights[n].rgb[0] == 33) &&
					(obj->lods[lodIdx].lights[n].rgb[1] == 33) &&
					(obj->lods[lodIdx].lights[n].rgb[2] == 33))
				{
					if(bcnLights)
					{
						glColor4fv(kNavLightRed);
						glTexCoord2f(0.0f, 0.5f); glVertex2f(-(size/2.0f), -(size/2.0f));
						glTexCoord2f(0.0f, 1.0f); glVertex2f(-(size/2.0f), (size/2.0f));
						glTexCoord2f(0.25f, 1.0f); glVertex2f((size/2.0f), (size/2.0f));
						glTexCoord2f(0.25f, 0.5f); glVertex2f((size/2.0f), -(size/2.0f));
					}
				}
					// Strobes
				else if((obj->lods[lodIdx].lights[n].rgb[0] == 44) &&
						(obj->lods[lodIdx].lights[n].rgb[1] == 44) &&
						(obj->lods[lodIdx].lights[n].rgb[2] == 44))
					{
						if(strbLights)
						{
							glColor4fv(kStrobeLight);
							glTexCoord2f(0.25f, 0.0f); glVertex2f(-(size/1.5f), -(size/1.5f));
							glTexCoord2f(0.25f, 0.5f); glVertex2f(-(size/1.5f), (size/1.5f));
							glTexCoord2f(0.50f, 0.5f); glVertex2f((size/1.5f), (size/1.5f));
							glTexCoord2f(0.50f, 0.0f); glVertex2f((size/1.5f), -(size/1.5f));
						}
					}
						// Landing Lights
					else if((obj->lods[lodIdx].lights[n].rgb[0] == 55) &&
							(obj->lods[lodIdx].lights[n].rgb[1] == 55) &&
							(obj->lods[lodIdx].lights[n].rgb[2] == 55))
						{
							if(landLights) {
								// BEN SEZ: modulate the _alpha to make this dark, not
								// the light color.  Otherwise if the sky is fairly light the light
								// will be darker than the sky, which looks f---ed during the day.
								float color[4];
								color[0] = kLandingLight[0];
								if(color[0] < 0.0) color[0] = 0.0;
								color[1] = kLandingLight[1];
								if(color[0] < 0.0) color[0] = 0.0;
								color[2] = kLandingLight[2];
								if(color[0] < 0.0) color[0] = 0.0;
								color[3] = kLandingLight[3] * ((static_cast<float>(distance) * -0.05882f) + 1.1764f);
								glColor4fv(color);
								glTexCoord2f(0.25f, 0.0f); glVertex2f(-(size/2.0f), -(size/2.0f));
								glTexCoord2f(0.25f, 0.5f); glVertex2f(-(size/2.0f), (size/2.0f));
								glTexCoord2f(0.50f, 0.5f); glVertex2f((size/2.0f), (size/2.0f));
								glTexCoord2f(0.50f, 0.0f); glVertex2f((size/2.0f), -(size/2.0f));
							}
						} else {
							// rear nav light and others? I guess...
							if(navLights) {
								glColor3f(
									obj->lods[lodIdx].lights[n].rgb[0] * 0.1f,
									obj->lods[lodIdx].lights[n].rgb[1] * 0.1f,
									obj->lods[lodIdx].lights[n].rgb[2] * 0.1f);
								glTexCoord2f(0.0f, 0.5f); glVertex2f(-(size/2.0f), -(size/2.0f));
								glTexCoord2f(0.0f, 1.0f); glVertex2f(-(size/2.0f), (size/2.0f));
								glTexCoord2f(0.25f, 1.0f); glVertex2f((size/2.0f), (size/2.0f));
								glTexCoord2f(0.25f, 0.5f); glVertex2f((size/2.0f), -(size/2.0f));
							}
						}
		glEnd();
		// Put OpenGL back how we found it
		glPopMatrix();
	}
}
