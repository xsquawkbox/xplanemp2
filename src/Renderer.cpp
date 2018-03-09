//
// Created by kuroneko on 3/03/2018.
//

#include <map>
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

#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <XPLMProcessing.h>

#include "obj8/InstanceWrapper.h"
#include "Renderer.h"

#include "XPMPMultiplayerVars.h"
#include "CullInfo.h"
#include "TCASHack.h"

#include "legacycsl/LegacyCSL.h"

XPLMDataRef			gVisDataRef = nullptr;	// Current air visiblity for culling.
XPLMProbeRef		gTerrainProbe = nullptr;

static XPLMDataRef	gWorldRenderTypeRef = nullptr;
static XPLMDataRef	gPlaneRenderTypeRef = nullptr;

bool				gMSAAHackInitialised = false;
static XPLMDataRef  gMSAAXRatioRef = nullptr;
static XPLMDataRef	gMSAAYRatioRef = nullptr;

void
Renderer_Init()
{
	// SETUP - mostly just fetch datarefs.
	gWorldRenderTypeRef = XPLMFindDataRef("sim/graphics/view/world_render_type");
	gPlaneRenderTypeRef = XPLMFindDataRef("sim/graphics/view/plane_render_type");
	gVisDataRef = XPLMFindDataRef("sim/graphics/view/visibility_effective_m");
	if (gVisDataRef == nullptr)
		gVisDataRef = XPLMFindDataRef("sim/weather/visibility_effective_m");
	if (gVisDataRef == nullptr)
		XPLMDebugString("WARNING: Default renderer could not find effective visibility in the sim.\n");

	gTerrainProbe = XPLMCreateProbe(xplm_ProbeY);
	CullInfo::init();
	TCAS::Init();
	InstanceCompat_Init();

#if RENDERER_STATS
	XPLMRegisterDataAccessor("hack/renderer/planes", xplmType_Int, 0, GetRendererStat, NULL,
							 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
							 &gTotPlanes, NULL);
	XPLMRegisterDataAccessor("hack/renderer/navlites", xplmType_Int, 0, GetRendererStat, NULL,
							 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
							 &gNavPlanes, NULL);
	XPLMRegisterDataAccessor("hack/renderer/objects", xplmType_Int, 0, GetRendererStat, NULL,
							 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
							 &gOBJPlanes, NULL);
	XPLMRegisterDataAccessor("hack/renderer/acfs", xplmType_Int, 0, GetRendererStat, NULL,
							 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
							 &gACFPlanes, NULL);
#endif

}

double	Render_LabelDistance = 0.0;
double	Render_FullPlaneDistance = 0.0;

static void
Render_PrepLists()
{
	gLabelList.clear();
	LegacyCSL::cleanFrame();
	TCAS::cleanFrame();

	if (gPlanes.empty()) {
		return;
	}

	// get our view information
	CullInfo			gl_camera;
	XPLMCameraPosition_t x_camera;
	XPLMReadCameraPosition(&x_camera);	// only for zoom!

	// Culling - read the camera pos and figure out what's visible.
	double	maxDist = XPLMGetDataf(gVisDataRef);
	Render_LabelDistance = min(maxDist, gConfiguration.maxLabelDistance) * x_camera.zoom;		// Labels get easier to see when users zooms.
	Render_FullPlaneDistance = x_camera.zoom * (5280.0 / 3.2) * gConfiguration.maxFullAircraftRenderingDistance;	// Only draw planes fully within 3 miles.

	for (auto &plane: gPlanes) {
		plane->doInstanceUpdate(gl_camera);
	}
}

vector<Label>	gLabelList;

/*
 * RenderingCallback
 *
 * Originally we had nicely split up elegance.
 *
 * Alas, it was not to to be - in order to render labels at all, we needed to do our 2d render in the 3d view phases
 * with bad hacks to maintain compatibility with FSAA
 */

static int rendLastCycle = -1;

static int
XPMP_RenderCallback_Aircraft(XPLMDrawingPhase, int, void *)
{
	// if we haven't yet done our rendering setup, do it now.
	int thisCycle = XPLMGetCycleNumber();
	if (thisCycle != rendLastCycle) {
		Render_PrepLists();
		rendLastCycle = thisCycle;
	}
	bool is_blend = false;
	bool is_shadow = (gWorldRenderTypeRef && XPLMGetDatai(gWorldRenderTypeRef));
	if (gPlaneRenderTypeRef) {
		is_blend = (XPLMGetDatai(gPlaneRenderTypeRef) == 2);
	}

	LegacyCSL::doRender(is_blend);

	if (is_blend && !is_shadow) {
		CullInfo	glCamera;

		gLabelList.clear();
		for (auto &plane : gPlanes) {
			plane->queueLabel(glCamera);
		}

		if (!gLabelList.empty()) {
			GLfloat		vp[4];

			CullInfo::GetCurrentViewport(vp);

			XPLMCameraPosition_t x_camera;
			XPLMReadCameraPosition(&x_camera);	// only for zoom!
			double	maxDist = XPLMGetDataf(gVisDataRef);
			double  labelDist = min(maxDist, gConfiguration.maxLabelDistance) * x_camera.zoom;		// Labels get easier to see when users zooms.

			double	x_scale = 1.0;
			double	y_scale = 1.0;
			if (gMSAAXRatioRef) {
				x_scale = XPLMGetDataf(gMSAAXRatioRef);
			}
			if (gMSAAYRatioRef) {
				y_scale = XPLMGetDataf(gMSAAYRatioRef);
			}
			
			/* setup an ortho projection */
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, vp[2], 0, vp[3], -1, 1);

			/* and load the identity modelview matrix */
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			if (x_scale > 1.0 || y_scale > 1.0) {
				glScalef(x_scale, y_scale, 1.0);
			}

			float c[4] = { 1, 1, 0, 1 };
			for (const auto &label : gLabelList) {
				float rat = 1.0f - (label.distSqr / static_cast<float>(labelDist * labelDist));
				c[0] = c[1] = 0.5f + 0.5f * rat;
				c[2] = 0.5f - 0.5f * rat;		// gray -> yellow - no alpha in the SDK - foo!

				float lx, ly;
				CullInfo::ProjectToViewport(vp, label.x, label.y, &lx, &ly);

				XPLMDrawString(c, lx / x_scale, (ly / y_scale) + 10, (char *)label.labelText.c_str(), nullptr, xplmFont_Basic);
			}

			/* restore the projection and modelview matrixes */
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
	}
	
	return 1;
}

void
Renderer_Attach_Callbacks()
{
	// don't look for the MSAA datarefs until we've attached the rendering hooks for the first time - for whatever reason, they don't exist during early sim-start.
	if (!gMSAAHackInitialised) {
		gMSAAHackInitialised = true;
		gMSAAXRatioRef = XPLMFindDataRef("sim/private/controls/hdr/fsaa_ratio_x");
		gMSAAYRatioRef = XPLMFindDataRef("sim/private/controls/hdr/fsaa_ratio_y");
	}


	XPLMRegisterDrawCallback(&XPMP_RenderCallback_Aircraft,
		xplm_Phase_Airplanes, 0, nullptr);

	InstanceCompat_Start();
	TCAS::EnableHooks();
}

void
Renderer_Detach_Callbacks()
{
	InstanceCompat_Stop();
	TCAS::DisableHooks();

	XPLMUnregisterDrawCallback(&XPMP_RenderCallback_Aircraft,
		xplm_Phase_Airplanes, 0, nullptr);
}