//
// Created by kuroneko on 3/03/2018.
//

#include <map>
#include <utility>
#include <algorithm>

#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <XPLMProcessing.h>
#include <XPLMInstance.h>
#include <XPLMCamera.h>

#include "Renderer.h"

#include "XPMPMultiplayerVars.h"
#include "TCASHack.h"

using namespace std;

XPLMDataRef			gVisDataRef = nullptr;	// Current air visiblity for culling.
XPLMProbeRef		gTerrainProbe = nullptr;

void
Renderer_Init()
{
	// SETUP - mostly just fetch datarefs.
	gVisDataRef = XPLMFindDataRef("sim/graphics/view/visibility_effective_m");
	if (gVisDataRef == nullptr)
		gVisDataRef = XPLMFindDataRef("sim/weather/visibility_effective_m");
	if (gVisDataRef == nullptr)
		XPLMDebugString("WARNING: Default renderer could not find effective visibility in the sim.\n");

	gTerrainProbe = XPLMCreateProbe(xplm_ProbeY);
	CullInfo::init();
	TCAS::Init();

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

double	Render_FullPlaneDistance = 0.0;

void
Render_PrepLists()
{
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
	Render_FullPlaneDistance = x_camera.zoom * (5280.0 / 3.2) * gConfiguration.maxFullAircraftRenderingDistance;	// Only draw planes fully within 3 miles.

	for (auto &plane: gPlanes) {
		plane->doInstanceUpdate(gl_camera);
	}
}

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
	return 1;
}

void
Renderer_Attach_Callbacks()
{
	XPLMRegisterDrawCallback(&XPMP_RenderCallback_Aircraft,
		xplm_Phase_Airplanes, 0, nullptr);

	TCAS::EnableHooks();
}

void
Renderer_Detach_Callbacks()
{
	TCAS::DisableHooks();

	XPLMUnregisterDrawCallback(&XPMP_RenderCallback_Aircraft,
		xplm_Phase_Airplanes, 0, nullptr);
}
