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

#include <cstdlib>
#include <cstdio>
#include <set>
#include <cassert>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <utility>

#include <XPLMUtilities.h>
#include <XPLMProcessing.h>
#include <XPLMPlanes.h>
#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>
#include <XPLMPlugin.h>
#include <XPLMUtilities.h>
#include <XPMPMultiplayer.h>

#include "XPMPMultiplayer.h"
#include "XPMPMultiplayerVars.h"
#include "TCASHack.h"
#include "CSLLibrary.h"
#include "XUtils.h"
#include "Renderer.h"
#include "obj8/Obj8CSL.h"


// This prints debug info on our process of loading Austin's planes.
#define	DEBUG_MANUAL_LOADING	0

static XPMPPlanePtr
XPMPPlaneFromID(XPMPPlaneID inID, XPMPPlaneVector::iterator * outIter=nullptr)
{
	assert(inID);
	if (outIter) {
		*outIter = std::find_if(gPlanes.begin(), gPlanes.end(), [inID] (const auto &p) {
			return p.get() == inID;
		});
		assert(*outIter != gPlanes.end());
	}
	return static_cast<XPMPPlanePtr>(inID);
}

/********************************************************************************
 * SETUP
 ********************************************************************************/

const char *
XPMPMultiplayerInit(XPMPConfiguration_t *inConfiguration,
                    const char *inRelated,
                    const char *inDoc8643)
{
	if (nullptr != inConfiguration) {
		memcpy(&gConfiguration, inConfiguration, sizeof(gConfiguration));
	}

	// set up OBJ8 support
	Obj8CSL::Init();

	bool	problem = false;

	if (!CSL_LoadData(inRelated, inDoc8643)) {
		problem = true;
	}
	Renderer_Init();

	if (problem)		return "There were problems initializing " XPMP_CLIENT_LONGNAME ".  Please examine X-Plane's log.txt file for detailed information.";
	else 				return "";
}

void
XPMPSetConfiguration(XPMPConfiguration_t *inConfig)
{
	memcpy(&gConfiguration, inConfig, sizeof(gConfiguration));
}

void
XPMPGetConfiguration(XPMPConfiguration_t *outConfig)
{
	memcpy(outConfig, &gConfiguration, sizeof(gConfiguration));
}

const char *	XPMPMultiplayerLoadCSLPackages(const char * inPackagePath) {
	bool problem = !CSL_LoadCSL(inPackagePath);

	if (problem) return "There was a problem loading CSLs for " XPMP_CLIENT_LONGNAME ". Please examine X-Plane's Log.txt file for detailed information.";
	else         return "";
}

void XPMPMultiplayerCleanup()
{
	Renderer_Detach_Callbacks();
}


const  char *
XPMPMultiplayerEnable()
{
	// Attempt to grab multiplayer planes, then analyze.
	int	result = XPLMAcquirePlanes(nullptr, nullptr, nullptr);
	if (result)
		XPLMSetActiveAircraftCount(1);
	else
		XPLMDebugString("WARNING: " XPMP_CLIENT_LONGNAME " did not acquire multiplayer planes!!\n");

	// put in the rendering hook now
	//Renderer_Attach_Callbacks();

	return "";
}

void
XPMPMultiplayerDisable(void)
{
	gPlanes.clear();
	Renderer_Detach_Callbacks();	
	XPLMReleasePlanes();
}


const char *
XPMPLoadCSLPackages(const char * inCSLFolder)
{
	bool	problem = false;

	if (!CSL_LoadCSL(inCSLFolder))
		problem = true;

	if (problem)		return "There were problems initializing " XPMP_CLIENT_LONGNAME ".  Please examine X-Plane's log.txt file for detailed information.";
	else 				return "";
}

int
XPMPGetNumberOfInstalledModels(void)
{
	size_t number = 0;
	for (const auto& package : gPackages)
	{
		number += package.planes.size();
	}
	return static_cast<int>(number);
}

void
XPMPGetModelInfo(int inIndex, const char** outModelName, const char** outIcao, const char** outAirline, const char** outLivery)
{
	int counter = 0;
	for (const auto& package : gPackages)
	{
		if (counter + static_cast<int>(package.planes.size()) < inIndex + 1) {
			counter += static_cast<int>(package.planes.size());
			continue;
		}

		int positionInPackage =  inIndex - counter;
		if (outModelName)
			*outModelName = package.planes[positionInPackage]->getModelName().c_str();
		if (outIcao)
			*outIcao = package.planes[positionInPackage]->getICAO().c_str();
		if (outAirline)
			*outAirline = package.planes[positionInPackage]->getAirline().c_str();
		if (outLivery)
			*outLivery = package.planes[positionInPackage]->getLivery().c_str();
		break;
	}
}

/********************************************************************************
 * PLANE OBJECT SUPPORT
 ********************************************************************************/

XPMPPlaneID
XPMPCreatePlane(
		const char *			inICAOCode,
		const char *			inAirline,
		const char *			inLivery)
{
	auto plane = std::make_unique<XPMPPlane>();
	plane->setType(PlaneType(inICAOCode, inAirline, inLivery));
	plane->updateCSL();

	gPlanes.push_back(std::move(plane));
	
	XPMPPlanePtr planePtr = gPlanes.back().get();
	if (gPlanes.size() == 1) {
		Renderer_Attach_Callbacks();
	}
	return planePtr;
}

XPMPPlaneID
XPMPCreatePlaneWithModelName(
	const char *inModelName,
	const char *inICAOCode,
	const char *inAirline,
	const char *inLivery)
{
	auto plane = std::make_unique<XPMPPlane>();
	plane->setType(PlaneType(inICAOCode, inAirline, inLivery));

	// Find the model
	bool found = false;
	for (const auto &package : gPackages) {
		auto cslPlane = std::find_if(package.planes.begin(), package.planes.end(), [inModelName](CSL* p) { return p->getModelName() == inModelName; });
		if (cslPlane != package.planes.end()) {
			plane->setCSL(*cslPlane);
			found = true;
		}
	}

	if (!found)	{
		XPLMDebugString("Requested model ");
		XPLMDebugString(inModelName);
		XPLMDebugString(" is unknown! Falling back to own model matching.");
		XPLMDebugString("\n");
		plane->updateCSL();
	}

	gPlanes.push_back(std::move(plane));

	XPMPPlanePtr planePtr = gPlanes.back().get();
	if (gPlanes.size() == 1) {
		Renderer_Attach_Callbacks();
	}
	return planePtr;
}

void
XPMPDestroyPlane(XPMPPlaneID inID)
{
	XPMPPlaneVector::iterator iter;
	XPMPPlanePtr plane = XPMPPlaneFromID(inID, &iter);

	gPlanes.erase(iter);
	if (gPlanes.size() == 0) {
		Renderer_Detach_Callbacks();
	}

}

int
XPMPChangePlaneModel(
	XPMPPlaneID				inPlaneID,
	const char *			inICAOCode,
	const char *			inAirline,
	const char *			inLivery,
	int						force_change)
{
	PlaneType newType(inICAOCode, inAirline, inLivery);

	XPMPPlanePtr plane = XPMPPlaneFromID(inPlaneID);
	if (force_change) {
		plane->setType(newType);
		plane->updateCSL();
	} else {
		if (plane->upgradeCSL(newType)) {
			plane->setType(newType);
		}
	}
	return plane->getMatchQuality();
}	

void	XPMPSetDefaultPlaneICAO(
		const char *			inICAO)
{
	gDefaultPlane.mICAO = inICAO;
}						

long			XPMPCountPlanes(void)
{
	return static_cast<long>(gPlanes.size());
}

XPMPPlaneID		XPMPGetNthPlane(
		long 					index)
{
	if ((index < 0) || (index >= static_cast<long>(gPlanes.size())))
		return NULL;

	return gPlanes[index].get();
}							

bool			XPMPIsICAOValid(
		const char *				inICAO)
{
	return CSL_MatchPlane(PlaneType(inICAO, "", ""), nullptr, false) != nullptr;
}

int 		XPMPGetPlaneModelQuality(
		XPMPPlaneID 				inPlane)
{
	XPMPPlanePtr thisPlane = XPMPPlaneFromID(inPlane);

	return thisPlane->getMatchQuality();
}

int			XPMPModelMatchQuality(
		const char *				inICAO,
		const char *				inAirline,
		const char *				inLivery)
{
	int 	matchQuality = -1;
	CSL_MatchPlane(PlaneType(inICAO, inAirline, inLivery), &matchQuality, false);

	return matchQuality;
}

void		XPMPDumpOneCycle(void)
{
	CSL_Dump();
}

void		XPMPUpdatePlanes(
		XPMPUpdate_t *				inUpdates,
		size_t						inUpdateSize,
		size_t						inCount)
{
	auto *ptr = reinterpret_cast<uint8_t *>(inUpdates);

	for (int idx = 0; idx < inCount; idx++) {
		auto *thisUpdate = reinterpret_cast<XPMPUpdate_t *>(ptr+(idx * inUpdateSize));

		// our default structure is 4 pointers long.
		if (inUpdateSize < (sizeof(void *) * 4))
			continue;

		auto *plane = XPMPPlaneFromID(thisUpdate->plane);

		if (thisUpdate->position) {
			plane->updatePosition(*thisUpdate->position);
		}
		if (thisUpdate->surfaces) {
			plane->updateSurfaces(*thisUpdate->surfaces);
		}
		if (thisUpdate->surveillance) {
			plane->updateSurveillance(*thisUpdate->surveillance);
		}

		// guards against new struct members should begin below.
	}
}
