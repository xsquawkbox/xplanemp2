#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>
#include <vector>
#include <algorithm>
#include <cstring>

#include "Obj8CSL.h"
#include "InstanceCompat.h"

using namespace std;

struct xplm_instance_t {
	XPLMObjectRef				model;
	vector<XPLMDataRef>			datarefs;

	XPLMDrawInfo_t				position;
	vector<float>				data;
};

static vector<xplm_instance_t *>	s_instances;

compat::XPLMInstanceRef
compat::XPLMCreateInstance(XPLMObjectRef obj, const char * drefs[])
{
	xplm_instance_t * i = new xplm_instance_t;
	i->model = obj;
	if(drefs)
	{
		while(*drefs)
		{
			XPLMDataRef	r = XPLMFindDataRef(*drefs);
			i->datarefs.push_back(r);
			i->data.push_back(0);

			++drefs;
		}
	}
	memset(&i->position,0,sizeof(i->position));
	s_instances.push_back(i);
	return i;
}

void
compat::XPLMDestroyInstance(XPLMInstanceRef i)
{
	xplm_instance_t * ii = (xplm_instance_t *) i;

	vector<xplm_instance_t *>::iterator it = find(s_instances.begin(), s_instances.end(), ii);
	if(it != s_instances.end())
		s_instances.erase(it);
	delete ii;
}

void
compat::XPLMInstanceSetPosition(
	compat::XPLMInstanceRef			instance,
	const XPLMDrawInfo_t *	position,
	const float *			data)
{
	xplm_instance_t * i = reinterpret_cast<xplm_instance_t *>(instance);
	if(data && !i->data.empty())
	{
		memcpy(&i->data[0], data, i->data.size() * sizeof(float));
	}
	memcpy(&i->position, position, sizeof(i->position));
}

static int draw_cb(
	XPLMDrawingPhase     inPhase,
	int                  inIsBefore,
	void *               inRefcon)
{
	static XPLMDataRef draw_type = XPLMFindDataRef("sim/graphics/view/plane_render_type");

	if(!draw_type || XPLMGetDatai(draw_type) == 1) {
		for (const auto &i: s_instances) {
#ifndef DONT_USE_DATAREF_HACK
			// HACK - because the only consumer of the instancing wrapper is the
			// Obj8 code, we'll splat the data into the obj8 dataref's
			// internally and without boundscheck rather than poking at the
			// XP dataref API

			for(int d = 0; d < i->data.size(); ++d)
				Obj8CSL::dref_values[d] = i->data[d];
#else
			for(int d = 0; d < i->data.size(); ++d)
				XPLMSetDataf(i->datarefs[d],i->data[d]);
#endif
			XPLMDrawObjects(i->model, 1, &i->position, 1, 0);
		}
	}
	return 1;
}

void
compat::XPLMInstanceInit()
{
	XPLMRegisterDrawCallback(draw_cb, xplm_Phase_Airplanes, 0, nullptr);
}

void
compat::XPLMInstanceCleanup()
{
	XPLMUnregisterDrawCallback(draw_cb, xplm_Phase_Airplanes, 0, nullptr);
}
