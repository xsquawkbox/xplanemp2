/*
 * Copyright (c) 2004, Ben Supnik and Chris Serio.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "TCASHack.h"
#include "XPMPPlaneRenderer.h"
#include "XPMPMultiplayer.h"
#include "CSLLibrary.h"
#include "XPMPMultiplayerCSLOffset.h"
#include "XPMPMultiplayerVars.h"
#include "legacycsl/LegacyObj.h"
#include "obj8/XPMPMultiplayerObj8.h"

#include "XPLMGraphics.h"
#include "XPLMDisplay.h"
#include "XPLMCamera.h"
#include "XPLMPlanes.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "CullInfo.h"

#include <stdio.h>
#include <math.h>
#include <algorithm>

#if IBM
#include <GL/gl.h>
#elif APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vector>
#include <string>
#include <set>
#include <map>

// Turn this on to get a lot of diagnostic info on who's visible, etc.
#define 	DEBUG_RENDERER 0
// Turn this on to put rendering stats in datarefs for realtime observatoin.
#define		RENDERER_STATS 0

// Even in good weather we don't want labels on things
// that we can barely see.  Cut labels at 5 km.
#define		MAX_LABEL_DIST			5000.0


bool gDrawLabels = true;



#if RENDERER_STATS

static int		GetRendererStat(void * inRefcon)
{
	return *((int *) inRefcon);
}

#endif

static	int		gTotPlanes = 0;			// Counters
static	int		gACFPlanes = 0;			// Number of Austin's planes we drew in full
static	int		gNavPlanes = 0;			// Number of Austin's planes we drew with lights only
static	int		gOBJPlanes = 0;			// Number of our OBJ planes we drew in full


static XPLMProbeRef terrainProbe = NULL; // Probe to probe where the ground is for clamping


void
XPMPInitDefaultPlaneRenderer()
{
	XPLMDestroyProbe(terrainProbe);
	terrainProbe = XPLMCreateProbe(xplm_ProbeY);
	
	// SETUP - mostly just fetch datarefs.

	gVisDataRef = XPLMFindDataRef("sim/graphics/view/visibility_effective_m");
	if (gVisDataRef == NULL) gVisDataRef = XPLMFindDataRef("sim/weather/visibility_effective_m");
	if (gVisDataRef == NULL)
		XPLMDebugString("WARNING: Default renderer could not find effective visibility in the sim.\n");

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

	TCAS_Init();
}

void XPMPDeinitDefaultPlaneRenderer() {
	XPLMDestroyProbe(terrainProbe);
}

double getCorrectYValue(double inX, double inY, double inZ, double inModelYOffset, bool inIsClampingOn) {
	if (!inIsClampingOn) {
		return inY;
	}
	XPLMProbeInfo_t info;
	info.structSize = sizeof(XPLMProbeInfo_t);
	XPLMProbeResult res = XPLMProbeTerrainXYZ(terrainProbe, inX, inY, inZ, &info);
	if (res != xplm_ProbeHitTerrain) {
		return inY;
	}
	double minY = info.locationY + inModelYOffset;
	return (inY < minY) ? minY : inY;
}

// PlaneToRender struct: we prioritize planes radially by distance, so...
// we use this struct to remember one visible plane.  Once we've
// found all visible planes, we draw the closest ones.

struct	PlaneToRender_t {
	float					x;			// Positional info
	float					y;
	float					z;
	XPMPPlanePtr			plane;
	bool					full;		// Do we need to draw the full plane or just lites?
	bool					cull;		// Are we visible on screen?
	bool					tcas;		// Are we visible on TCAS?
	XPLMPlaneDrawState_t	state;		// Flaps, gear, etc.
	float					dist;
};
// typedef	std::map<float, PlaneToRender_t>	RenderMap;


void			XPMPDefaultPlaneRenderer(int is_blend)
{
	long	planeCount = XPMPCountPlanes();
#if DEBUG_RENDERER
	char	buf[50];
	sprintf(buf,"Renderer Planes: %d\n", planeCount);
	XPLMDebugString(buf);
#endif
	if (planeCount == 0)		// Quick exit if no one's around.
	{
		if (gDumpOneRenderCycle)
		{
			gDumpOneRenderCycle = false;
			XPLMDebugString("No planes this cycle.\n");
		}
		return;
	}
	
	CullInfo			gl_camera;

	XPLMCameraPosition_t x_camera;

	XPLMReadCameraPosition(&x_camera);	// only for zoom!

	// Culling - read the camera pos«and figure out what's visible.

	double	maxDist = XPLMGetDataf(gVisDataRef);
	double  labelDist = min(maxDist, MAX_LABEL_DIST) * x_camera.zoom;		// Labels get easier to see when users zooms.
	double	fullPlaneDist = x_camera.zoom * (5280.0 / 3.2) * (gFloatPrefsFunc ? gFloatPrefsFunc("planes","full_distance", 3.0) : 3.0);	// Only draw planes fully within 3 miles.
	int		maxFullPlanes = gIntPrefsFunc ? gIntPrefsFunc("planes","max_full_count", 100) : 100;						// Draw no more than 100 full planes!

	gTotPlanes = planeCount;
	gNavPlanes = gACFPlanes = gOBJPlanes = 0;

	int modelCount, active, plugin, tcas;
	XPLMCountAircraft(&modelCount, &active, &plugin);
	tcas = modelCount - 1;	// This is how many tcas blips we can have!

	RenderMap						myPlanes;		// Planes - sorted by distance so we can do the closest N and bail
	
	/************************************************************************************
	 * CULLING AND STATE CALCULATION LOOP
	 ************************************************************************************/
	
	if (gDumpOneRenderCycle)
	{
		XPLMDebugString("Dumping one cycle map of planes.\n");
		char	fname[256], bigbuf[1024], foo[32];
		for (int n = 1; n < modelCount; ++n)
		{
			XPLMGetNthAircraftModel(n, fname, bigbuf);
			sprintf(foo, " [%d] - ", n);
			XPLMDebugString(foo);
			XPLMDebugString(fname);
			XPLMDebugString(" - ");
			XPLMDebugString(bigbuf);
			XPLMDebugString("\n");
		}
	}
	
	// Go through every plane.  We're going to figure out if it is visible and if so remember it for drawing later.
	for (long index = 0; index < planeCount; ++index)
	{
		XPMPPlaneID id = XPMPGetNthPlane(index);
		
		XPMPPlanePosition_t	pos;
		pos.size = sizeof(pos);
		pos.label[0] = 0;

		if (XPMPGetPlaneData(id, xpmpDataType_Position, &pos) != xpmpData_Unavailable)
		{
			// First figure out where the plane is!

			double	x,y,z;
			XPLMWorldToLocal(pos.lat, pos.lon, pos.elevation * kFtToMeters, &x, &y, &z);
			
			float distMeters = sqrt(gl_camera.SphereDistanceSqr(static_cast<float>(x),
														static_cast<float>(y),
														static_cast<float>(z)));
			
			// If the plane is farther than our TCAS range, it's just not visible.  Drop it!
			if (distMeters > kMaxDistTCAS)
				continue;

			// Only draw if it's in range.
			bool cull = (distMeters > maxDist);
			
			XPMPPlaneSurveillance_t radar;
			radar.size = sizeof(radar);
			bool tcas = true;
			if (XPMPGetPlaneData(id, xpmpDataType_Radar, &radar) != xpmpData_Unavailable)
				if (radar.mode == xpmpTransponderMode_Standby)
					tcas = false;

			// check for altitude - if difference exceeds a preconfigured limit, don't show
			double acft_alt = XPLMGetDatad(gAltitudeRef) / kFtToMeters;
			double alt_diff = pos.elevation - acft_alt;
			if(alt_diff < 0) alt_diff *= -1;
			if(alt_diff > MAX_TCAS_ALTDIFF) tcas = false;

			// Calculate the heading from the camera to the target (hor, vert).
			// Calculate the angles between the camera angles and the real angles.
			// Cull if we exceed half the FOV.
			
			if(!cull && !gl_camera.SphereIsVisible(static_cast<float>(x),
										   static_cast<float>(y),
										   static_cast<float>(z), 50.0))
			{
				cull = true;
			}
			
			// Full plane or lites based on distance.
			bool	drawFullPlane = (distMeters < fullPlaneDist);
			
#if DEBUG_RENDERER

			char	icao[128], livery[128];
			char	debug[512];

			XPMPGetPlaneICAOAndLivery(id, icao, livery);
			sprintf(debug,"Queueing plane %d (%s/%s) at lle %f, %f, %f (xyz=%f, %f, %f) pitch=%f,roll=%f,heading=%f,model=1.\n", index, icao, livery,
					pos.lat, pos.lon, pos.elevation,
					x, y, z, pos.pitch, pos.roll, pos.heading);
			XPLMDebugString(debug);
#endif

			// Stash one render record with the plane's position, etc.
			{
				PlaneToRender_t		renderRecord;
				renderRecord.x = static_cast<float>(x);
				renderRecord.y = static_cast<float>(y);
				renderRecord.z = static_cast<float>(z);
				renderRecord.plane = static_cast<XPMPPlanePtr>(id);
				renderRecord.cull = cull;						// NO other planes.  Doing so causes a lot of things to go nuts!
				renderRecord.tcas = tcas;

				XPMPPlaneSurfaces_t	surfaces;
				surfaces.size = sizeof(surfaces);
				if (XPMPGetPlaneData(id, xpmpDataType_Surfaces, &surfaces) != xpmpData_Unavailable)
				{
					renderRecord.state.structSize = sizeof(renderRecord.state);
					renderRecord.state.gearPosition 	= surfaces.gearPosition 	;
					renderRecord.state.flapRatio 		= surfaces.flapRatio 		;
					renderRecord.state.spoilerRatio 	= surfaces.spoilerRatio 	;
					renderRecord.state.speedBrakeRatio 	= surfaces.speedBrakeRatio 	;
					renderRecord.state.slatRatio 		= surfaces.slatRatio 		;
					renderRecord.state.wingSweep 		= surfaces.wingSweep 		;
					renderRecord.state.thrust 			= surfaces.thrust 			;
					renderRecord.state.yokePitch 		= surfaces.yokePitch 		;
					renderRecord.state.yokeHeading 		= surfaces.yokeHeading 		;
					renderRecord.state.yokeRoll 		= surfaces.yokeRoll 		;
				} else {
					renderRecord.state.structSize = sizeof(renderRecord.state);
					renderRecord.state.gearPosition = (pos.elevation < 70) ?  1.0f : 0.0f;
					renderRecord.state.flapRatio = (pos.elevation < 70) ? 1.0f : 0.0f;
					renderRecord.state.spoilerRatio = renderRecord.state.speedBrakeRatio = renderRecord.state.slatRatio = renderRecord.state.wingSweep = 0.0;
					renderRecord.state.thrust = (pos.pitch > 30) ? 1.0f : 0.6f;
					renderRecord.state.yokePitch = pos.pitch / 90.0f;
					renderRecord.state.yokeHeading = pos.heading / 180.0f;
					renderRecord.state.yokeRoll = pos.roll / 90.0f;

					// use some smart defaults
					renderRecord.plane->surface.lights.bcnLights = 1;
					renderRecord.plane->surface.lights.navLights = 1;
				}
				if (renderRecord.plane->model && !renderRecord.plane->model->mMovingGear)
					renderRecord.plane->surface.gearPosition = 1.0;
				renderRecord.full = drawFullPlane;
				renderRecord.dist = distMeters;
				myPlanes.insert(RenderMap::value_type(distMeters, renderRecord));

			} // State calculation
			
		} // Plane has data available
		
	} // Per-plane loop

	if (gDumpOneRenderCycle)
		XPLMDebugString("End of cycle dump.\n");
	
	/************************************************************************************
	 * ACTUAL RENDERING LOOP
	 ************************************************************************************/
	
	// We're going to go in and render the first N planes in full, and the rest as lites.
	// We're also going to put the x-plane multiplayer vars in place for the first N
	// TCAS-visible planes, so they show up on our moving map.
	// We do this in two stages: building up what to do, then doing it in the optimal
	// OGL order.
	
	size_t	renderedCounter = 0;

	vector<PlaneToRender_t *>			planes_obj_lites;
	multimap<int, PlaneToRender_t *>	planes_austin;
	vector<PlaneToRender_t *>			planes_obj;
	vector<PlaneToRender_t *>			planes_obj8;

	vector<PlaneToRender_t *>::iterator			planeIter;
	multimap<int, PlaneToRender_t *>::iterator	planeMapIter;

	// In our first iteration pass we'll go through all planes and handle TCAS, draw planes that have no
	// CSL model, and put CSL planes in the right 'bucket'.

	for (RenderMap::iterator iter = myPlanes.begin(); iter != myPlanes.end(); ++iter)
	{
		// This is the case where we draw a real plane.
		if (!iter->second.cull)
		{
			// Max plane enforcement - once we run out of the max number of full planes the
			// user allows, force only lites for framerate
			if (gACFPlanes >= maxFullPlanes)
				iter->second.full = false;

#if DEBUG_RENDERER
			char	debug[512];
			sprintf(debug,"Drawing plane: %s at %f,%f,%f (%fx%fx%f full=%d\n",
					iter->second.plane->model ? iter->second.plane->model->file_path.c_str() : "<none>", iter->second.x, iter->second.y, iter->second.z,
					iter->second.plane->pos.pitch, iter->second.plane->pos.roll, iter->second.plane->pos.heading, iter->second.full ? 1 : 0);
			XPLMDebugString(debug);
#endif

			if (iter->second.plane->model)
			{
				//find or update the actual vert offset in the csl model data
				cslVertOffsetCalc.findOrUpdateActualVertOffset(*iter->second.plane->model);
				//correct y value by real terrain elevation
				bool isClampingOn = (gIntPrefsFunc("PREFERENCES", "CLAMPING", true > 0) ? true : false);
				iter->second.y = getCorrectYValue(iter->second.x, iter->second.y, iter->second.z, iter->second.plane->model->actualVertOffset, isClampingOn);
				if (iter->second.plane->model->plane_type == plane_Austin)
				{
					planes_austin.insert(multimap<int, PlaneToRender_t *>::value_type(CSL_GetOGLIndex(iter->second.plane->model), &iter->second));
				}
				else if (iter->second.plane->model->plane_type == plane_Obj)
				{
					planes_obj.push_back(&iter->second);
					planes_obj_lites.push_back(&iter->second);
				}
				else if(iter->second.plane->model->plane_type == plane_Obj8)
				{
					planes_obj8.push_back(&iter->second);
				}

			} else {
				// If it's time to draw austin's planes but this one
				// doesn't have a model, we draw anything.
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glTranslatef(iter->second.x, iter->second.y, iter->second.z);
				glRotatef(iter->second.plane->pos.heading, 0.0, -1.0, 0.0);
				glRotatef(iter->second.plane->pos.pitch, 01.0, 0.0, 0.0);
				glRotatef(iter->second.plane->pos.roll, 0.0, 0.0, -1.0);

				// Safety check - if plane 1 isn't even loaded do NOT draw, do NOT draw plane 0.
				// Using the user's planes can cause the internal flight model to get f-cked up.
				// Using a non-loaded plane can trigger internal asserts in x-plane.
				if (modelCount > 1)
					if(!is_blend)
						XPLMDrawAircraft(1,
										 (float) iter->second.x, (float) iter->second.y, (float) iter->second.z,
										 iter->second.plane->pos.pitch, iter->second.plane->pos.roll, iter->second.plane->pos.heading,
										 iter->second.full ? 1 : 0, &iter->second.state);

				glPopMatrix();
			}

		}

		// TCAS handling - if the plane needs to be drawn on TCAS and we haven't yet, move one of Austin's planes.
		if (iter->second.tcas && renderedCounter < gMultiRef_X.size())
		{
			XPLMSetDataf(gMultiRef_X[renderedCounter], iter->second.x);
			XPLMSetDataf(gMultiRef_Y[renderedCounter], iter->second.y);
			XPLMSetDataf(gMultiRef_Z[renderedCounter], iter->second.z);
			++renderedCounter;
		}
	}
	
	// PASS 1 - draw Austin's planes.

	if(!is_blend)
		for (planeMapIter = planes_austin.begin(); planeMapIter != planes_austin.end(); ++planeMapIter)
		{
			CSL_DrawObject(	planeMapIter->second->plane,
							planeMapIter->second->dist,
							planeMapIter->second->x,
							planeMapIter->second->y,
							planeMapIter->second->z,
							planeMapIter->second->plane->pos.pitch,
							planeMapIter->second->plane->pos.roll,
							planeMapIter->second->plane->pos.heading,
							plane_Austin,
							planeMapIter->second->full ? 1 : 0,
							planeMapIter->second->plane->surface.lights,
							&planeMapIter->second->state);

			if (planeMapIter->second->full)
				++gACFPlanes;
			else
				++gNavPlanes;
		}
	
	// PASS 2 - draw OBJs
	// Blend for solid OBJ7s?  YES!  First, in HDR mode, they DO NOT draw to the gbuffer properly -
	// they splat their livery into the normal map, which is terrifying and stupid.  Then they are also
	// pre-lit...the net result is surprisingly not much worse than regular rendering considering how many
	// bad things have happened, but for all I know we're getting NaNs somewhere.
	//
	// Blending isn't going to hurt things in NON-HDR because our rendering is so stupid for old objs - there's
	// pretty much never translucency so we aren't going to get Z-order fails.  So f--- it...always draw blend.<
	if(is_blend)
		for (const auto &plane_obj : planes_obj)
		{
			CSL_DrawObject(
						plane_obj->plane,
						plane_obj->dist,
						plane_obj->x,
						plane_obj->y,
						plane_obj->z,
						plane_obj->plane->pos.pitch,
						plane_obj->plane->pos.roll,
						plane_obj->plane->pos.heading,
						plane_Obj,
						plane_obj->full ? 1 : 0,
						plane_obj->plane->surface.lights,
						&plane_obj->state);
			++gOBJPlanes;
		}

	for(planeIter = planes_obj8.begin(); planeIter != planes_obj8.end(); ++planeIter)
	{
		CSL_DrawObject( (*planeIter)->plane,
						(*planeIter)->dist,
						(*planeIter)->x,
						(*planeIter)->y,
						(*planeIter)->z,
						(*planeIter)->plane->pos.pitch,
						(*planeIter)->plane->pos.roll,
						(*planeIter)->plane->pos.heading,
						plane_Obj8,
						(*planeIter)->full ? 1 : 0,
						(*planeIter)->plane->surface.lights,
						&(*planeIter)->state);
	}

	if(!is_blend)
		obj_draw_solid();

	// PASS 3 - draw OBJ lights.

	if(is_blend)
		if (!planes_obj_lites.empty())
		{
			OBJ_BeginLightDrawing();
			for (planeIter = planes_obj_lites.begin(); planeIter != planes_obj_lites.end(); ++planeIter)
			{
				// this thing draws the lights of a model
				CSL_DrawObject( (*planeIter)->plane,
								(*planeIter)->dist,
								(*planeIter)->x,
								(*planeIter)->y,
								(*planeIter)->z,
								(*planeIter)->plane->pos.pitch,
								(*planeIter)->plane->pos.roll,
								(*planeIter)->plane->pos.heading,
								plane_Lights,
								(*planeIter)->full ? 1 : 0,
								(*planeIter)->plane->surface.lights,
								&(*planeIter)->state);
			}
		}
	
	if(is_blend)
		obj_draw_translucent();
	obj_draw_done();
	
	// PASS 4 - Labels
	if(is_blend)
		if ( gDrawLabels )
		{
			GLfloat	vp[4];
			glGetFloatv(GL_VIEWPORT,vp);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, vp[2], 0, vp[3], -1, 1);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			float c[4] = { 1, 1, 0, 1 };


			for (RenderMap::iterator iter = myPlanes.begin(); iter != myPlanes.end(); ++iter)
				if(iter->first < labelDist)
					if(!iter->second.cull)		// IMPORTANT - airplane BEHIND us still maps XY onto screen...so we get 180 degree reflections.  But behind us acf are culled, so that's good.
					{
						float x, y;
						gl_camera.ConvertTo2D(vp, iter->second.x, iter->second.y, iter->second.z, 1.0, &x, &y);

						float rat = 1.0f - (iter->first / static_cast<float>(labelDist));
						c[0] = c[1] = 0.5f + 0.5f * rat;
						c[2] = 0.5f - 0.5f * rat;		// gray -> yellow - no alpha in the SDK - foo!

						XPLMDrawString(c, static_cast<int>(x), static_cast<int>(y)+10, (char *) iter->second.plane->pos.label, NULL, xplmFont_Basic);
					}

			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();

		}

	
	// Final hack - leave a note to ourselves for how many of Austin's planes we relocated to do TCAS.
	if (tcas > static_cast<int>(renderedCounter))
		tcas = static_cast<int>(renderedCounter);
	gEnableCount = (tcas+1);
	
	gDumpOneRenderCycle = 0;

	// finally, cleanup textures.
	OBJ_MaintainTextures();
}

void XPMPEnableAircraftLabels()
{
	gDrawLabels = true;
}

void XPMPDisableAircraftLabels()
{
	gDrawLabels = false;
}

bool XPMPDrawingAircraftLabels()
{
	return gDrawLabels;
}

