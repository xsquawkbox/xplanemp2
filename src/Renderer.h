//
// Created by kuroneko on 3/03/2018.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <vector>

#include "XPLMScenery.h"

extern XPLMDataRef		gVisDataRef;		// Current air visiblity for culling.
extern XPLMProbeRef		gTerrainProbe;

extern double	Render_FullPlaneDistance;

struct Label {
	double		x;
	double		y;
	float 		distSqr;
	std::string	labelText;
};

void	Renderer_Init();
void	Renderer_Attach_Callbacks();
void	Renderer_Detach_Callbacks();


#endif //RENDERER_H
