/**
 * This is a backward-compatibility wrapper for the new XPLM300
 * instancing APIs.
 *
 * It can be used to provide compatibility with plugins that support
 * both X-Plane 10 and 11. X-Plane 10 plugins will not see the performance
 * benefit, of course, but this saves you from needing two separate
 * implementations.
 */

#ifndef instancecompat_h
#define instancecompat_h

#include <XPLMScenery.h>

namespace compat {
	typedef void *	XPLMInstanceRef;

	XPLMInstanceRef XPLMCreateInstance(XPLMObjectRef obj, const char * drefs[]);
	void			XPLMDestroyInstance(XPLMInstanceRef inst);

	// Data is consecutive floats, one for each dataref
	void			XPLMInstanceSetPosition(
						XPLMInstanceRef			inst,
						const XPLMDrawInfo_t *	position,
						const float *			data);

	// These are boiler plate needed only for the 'lib' implementation of the above APIs.
	void			XPLMInstanceInit();
	void			XPLMInstanceCleanup();
};


#endif /* instancecompat_h */