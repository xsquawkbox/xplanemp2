/*
 * Copyright (c) 2004, Ben Supnik and Chris Serio.
 * Copyright (c) 2018, Chris Collins.
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

#ifndef XPMP_TCASHACK_H
#define XPMP_TCASHACK_H

#include <vector>
#include <map>

#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>

/* Maximum altitude difference in feet for TCAS blips */
#define		MAX_TCAS_ALTDIFF		10000


class TCAS {
private:
	static std::vector<XPLMDataRef>			gMultiRef_X;
	static std::vector<XPLMDataRef>			gMultiRef_Y;
	static std::vector<XPLMDataRef>			gMultiRef_Z;

	static int 								gEnableCount; // Hack - see TCAS support

	static bool								gTCASHooksRegistered;

	static int ControlPlaneCount(XPLMDrawingPhase, int, void *);

	struct plane_record {
		float x;
		float y;
		float z;
	};
	typedef std::multimap<float, struct plane_record>	TCASMap;

	static TCASMap 							gTCASPlanes;
	static int								gMaxTCASItems;

public:
	static XPLMDataRef						gAltitudeRef; // Current aircraft altitude (for TCAS)

	static void Init();
	static void EnableHooks();
	static void DisableHooks();

	static void cleanFrame();

	/** adds a plane to the list of aircraft we're going to report on */
	static void addPlane(float distanceSqr, float x, float y, float z, bool isReportingAltitude);
};

#endif //XPMP_TCASHACK_H
