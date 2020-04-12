/*
 * Copyright (c) 2005, Ben Supnik and Chris Serio.
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

#ifndef XPLMMULTIPLAYERVARS_H
#define XPLMMULTIPLAYERVARS_H

/*
 * This file contains the various internal definitions
 * and globals for the model rendering code.
 *
 */

#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <memory>

#include "XPMPMultiplayer.h"

#include "CSL.h"
#include "PlaneType.h"

const	double	kFtToMeters = 0.3048;
const	double	kMaxDistTCAS = 40.0 * 6080.0 * kFtToMeters;

/****************** MODEL MATCHING CRAP ***************/

// These enums define the eight levels of matching we might possibly
// make.  For each level of matching, we use a single string as a key.
// (The string's contents vary with model - examples are shown.)
enum {
	match_icao_airline_livery = 0,		//	B738 SWA SHAMU
	match_icao_airline,					//	B738 SWA
	match_group_airline_livery,			//	B731 B732 B733 B734 B735 B736 B737 B738 B739 SWA SHAMU
	match_group_airline,				//	B731 B732 B733 B734 B735 B736 B737 B738 B739 SWA
	match_icao_livery,					//  B738 SHAMU
	match_icao,							//	B738
	match_group_livery,					//  B731 B732 B733 B734 B735 B736 B737 B738 B739 SHAMU
	match_group, 						//	B731 B732 B733 B734 B735 B736 B737 B738 B739
	match_count
};

enum {
	match_fallback_wtc_fullconfig = 0,
	match_fallback_wtc_engines_enginetype,
	match_fallback_wtc_engines,
	match_fallback_wtc_enginetype,
	match_fallback_wtc,
	match_fallback_count
};


// A CSL package - a vector of planes and six maps from the above matching 
// keys to the internal index of the plane.
struct	CSLPackage_t {

	bool hasValidHeader() const
	{
		return !name.empty() && !path.empty();
	}

	std::string					name;
	std::string					path;
	std::vector<CSL *>			planes;
	std::unordered_map<std::string, int>	matches[match_count];
};

extern std::vector<CSLPackage_t>		gPackages;

extern std::unordered_map<std::string, std::string>		gGroupings;

/**************** Model matching using ICAO doc 8643
		(http://www.icao.int/anb/ais/TxtFiles/Doc8643.txt) ***********/

struct CSLAircraftCode_t {
	std::string			icao;		// aircraft ICAO code
    std::string			equip;		// equipment code (L1T, L2J etc)
	char				category;	// L, M, H, V (vertical = helo)
};

extern std::unordered_map<std::string, CSLAircraftCode_t>	gAircraftCodes;

/**************** PLANE OBJECTS ********************/

#include "XPMPPlane.h"

typedef	XPMPPlane *								XPMPPlanePtr;
typedef	std::unordered_map<void *, std::unique_ptr<XPMPPlane>>		XPMPPlaneMap;

extern XPMPConfiguration_t				gConfiguration;
extern PlaneType						gDefaultPlane;

extern XPMPPlaneMap					gPlanes;				// All planes

#endif
