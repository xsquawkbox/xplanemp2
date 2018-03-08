//
// Created by kuroneko on 2/03/2018.
//

#ifndef XPMP_TCASHACK_H
#define XPMP_TCASHACK_H

#include <vector>

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
