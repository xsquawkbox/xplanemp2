//
// Created by kuroneko on 2/03/2018.
//

#ifndef INSTANCEWRAPPER_H
#define INSTANCEWRAPPER_H

#include <XPLMScenery.h>

typedef void * xp11InstanceRef;

extern xp11InstanceRef     (*xp11CreateInstance)(
	XPLMObjectRef        obj,
	const char **        datarefs);
extern void                 	(*xp11DestroyInstance)(
	xp11InstanceRef      instance);
extern void                 (*xp11InstanceSetPosition)(
	xp11InstanceRef      instance,
	const XPLMDrawInfo_t * new_position,
	const float *        data);

/** InstanceCompat_Init() probes X-Plane to see if the Instance API is available, and if it isn't, hooks up the
 * backwards compatibility API instead.
 */

extern void		InstanceCompat_Init();
extern void		InstanceCompat_Start();
extern void		InstanceCompat_Stop();



#endif //INSTANCEWRAPPER_H
