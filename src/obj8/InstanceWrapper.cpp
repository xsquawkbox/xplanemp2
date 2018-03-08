//
// Created by kuroneko on 2/03/2018.
//

#include <XPLMPlugin.h>
#include <XPLMUtilities.h>

#include "InstanceCompat.h"
#include "InstanceWrapper.h"

static bool iwIsCompatibility = false;

xp11InstanceRef     (*xp11CreateInstance)(
	XPLMObjectRef        obj,
	const char **        datarefs);
void                 	(*xp11DestroyInstance)(
	xp11InstanceRef      instance);
void                 (*xp11InstanceSetPosition)(
	xp11InstanceRef      instance,
	const XPLMDrawInfo_t * new_position,
	const float *        data);


static void
InstanceWrapper_connectLegacy()
{
	iwIsCompatibility = true;
	xp11CreateInstance = &compat::XPLMCreateInstance;
	xp11DestroyInstance = &compat::XPLMDestroyInstance;
	xp11InstanceSetPosition = &compat::XPLMInstanceSetPosition;
}

void
InstanceCompat_Init()
{
	xp11CreateInstance = reinterpret_cast<xp11InstanceRef (*)(XPLMObjectRef, const char **)>(XPLMFindSymbol("XPLMCreateInstance"));
	xp11DestroyInstance = reinterpret_cast<void (*)(xp11InstanceRef)>(XPLMFindSymbol("XPLMDestroyInstance"));
	xp11InstanceSetPosition = reinterpret_cast<void (*)(xp11InstanceRef, const XPLMDrawInfo_t *, const float *)>(XPLMFindSymbol("XPLMInstanceSetPosition"));

	if (xp11CreateInstance == nullptr ||
		xp11DestroyInstance == nullptr ||
		xp11InstanceSetPosition == nullptr) {
		InstanceWrapper_connectLegacy();
	}
}

void
InstanceCompat_Start()
{
	if (iwIsCompatibility) {
		compat::XPLMInstanceInit();
	}
}

void
InstanceCompat_Stop()
{
	if (iwIsCompatibility) {
		compat::XPLMInstanceCleanup();
	}

}
